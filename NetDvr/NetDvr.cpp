
#include "netdvrprivate.h"
#include <io.h>
#include <fcntl.h>

#ifdef WIN32
//#include "winsock2.h"
#include <sys/timeb.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

extern struct NETDVR_DVR_POOL g_dvr_pool;
extern MutexHandle g_pool_lock;

//csp modify
#define NETSDK_VERSION "0.2.4.6"

/************************************************************************/
//deinterlace

#define MAX_NEG_CROP 1024

typedef unsigned char uint8_t;

uint8_t ff_cropTbl[256 + 2 * MAX_NEG_CROP] = {0, };

/* init static data */
static void dsputil_static_init(void)
{
    int i;
	
    for(i=0;i<256;i++) ff_cropTbl[i + MAX_NEG_CROP] = i;
    for(i=0;i<MAX_NEG_CROP;i++) {
        ff_cropTbl[i] = 0;
        ff_cropTbl[i + MAX_NEG_CROP + 256] = 255;
    }
}

static void deinterlace_line_inplace(uint8_t *lum_m4, 
									 uint8_t *lum_m3, 
									 uint8_t *lum_m2, 
									 uint8_t *lum_m1, 
									 uint8_t *lum,
									 int size)
{
//#if !HAVE_MMX
#if 1
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;
    int sum;
	
    for(;size > 0;size--) {
        sum = -lum_m4[0];
        sum += lum_m3[0] << 2;
        sum += lum_m2[0] << 1;
        lum_m4[0]=lum_m2[0];
        sum += lum_m1[0] << 2;
        sum += -lum[0];
        lum_m2[0] = cm[(sum + 4) >> 3];
        lum_m4++;
        lum_m3++;
        lum_m2++;
        lum_m1++;
        lum++;
    }
#else
    {
        pxor_r2r(mm7,mm7);
        movq_m2r(ff_pw_4,mm6);
    }
    for (;size > 3; size-=4) {
        DEINT_INPLACE_LINE_LUM
			lum_m4+=4;
        lum_m3+=4;
        lum_m2+=4;
        lum_m1+=4;
        lum+=4;
    }
#endif
}

static void deinterlace_bottom_field_inplace(uint8_t *src1, 
											 int src_wrap,
                                             int width, 
											 int height)
{
	static int initflag = 0;
	if(!initflag)
	{
		dsputil_static_init();
		initflag = 1;
	}
	
    uint8_t *src_m1, *src_0, *src_p1, *src_p2;
    int y;
    uint8_t *buf = NULL;
	
	unsigned char linebuf[704];
	
	if(width > 704)
    {
		buf = (uint8_t*)malloc(width);
	}
	else
	{
		buf = linebuf;
	}
	
    src_m1 = src1;
    memcpy(buf,src_m1,width);
    src_0=&src_m1[src_wrap];
    src_p1=&src_0[src_wrap];
    src_p2=&src_p1[src_wrap];
    for(y=0;y<(height-2);y+=2) {
        deinterlace_line_inplace(buf,src_m1,src_0,src_p1,src_p2,width);
        src_m1 = src_p1;
        src_0 = src_p2;
        src_p1 += 2*src_wrap;
        src_p2 += 2*src_wrap;
    }
    /* do last line */
    deinterlace_line_inplace(buf,src_m1,src_0,src_0,src_0,width);
	
	if(width > 704)
	{
		free(buf);
	}
}

/* filter parameters: [-1 4 2 4 -1] // 8 */
static void deinterlace_line(uint8_t *dst,
                             const uint8_t *lum_m4, 
							 const uint8_t *lum_m3,
                             const uint8_t *lum_m2, 
							 const uint8_t *lum_m1,
                             const uint8_t *lum,
                             int size)
{
	//#if !HAVE_MMX
#if 1
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;
    int sum;
	
    for(;size > 0;size--) {
        sum = -lum_m4[0];
        sum += lum_m3[0] << 2;
        sum += lum_m2[0] << 1;
        sum += lum_m1[0] << 2;
        sum += -lum[0];
        dst[0] = cm[(sum + 4) >> 3];
        lum_m4++;
        lum_m3++;
        lum_m2++;
        lum_m1++;
        lum++;
        dst++;
    }
#else
    {
        pxor_r2r(mm7,mm7);
        movq_m2r(ff_pw_4,mm6);
    }
    for (;size > 3; size-=4) {
        DEINT_LINE_LUM
			lum_m4+=4;
        lum_m3+=4;
        lum_m2+=4;
        lum_m1+=4;
        lum+=4;
        dst+=4;
    }
#endif
}

/* deinterlacing : 2 temporal taps, 3 spatial taps linear filter. The
top field is copied as is, but the bottom field is deinterlaced
against the top field. */
static void deinterlace_bottom_field(uint8_t *dst, 
									 int dst_wrap,
									 const uint8_t *src1, 
									 int src_wrap,
									 int width, 
									 int height)
{
	static int initflag = 0;
	if(!initflag)
	{
		dsputil_static_init();
		initflag = 1;
	}
	
    const uint8_t *src_m2, *src_m1, *src_0, *src_p1, *src_p2;
    int y;
	
    src_m2 = src1;
    src_m1 = src1;
    src_0=&src_m1[src_wrap];
    src_p1=&src_0[src_wrap];
    src_p2=&src_p1[src_wrap];
    for(y=0;y<(height-2);y+=2) {
        memcpy(dst,src_m1,width);
        dst += dst_wrap;
        deinterlace_line(dst,src_m2,src_m1,src_0,src_p1,src_p2,width);
        src_m2 = src_0;
        src_m1 = src_p1;
        src_0 = src_p2;
        src_p1 += 2*src_wrap;
        src_p2 += 2*src_wrap;
        dst += dst_wrap;
    }
    memcpy(dst,src_m1,width);
    dst += dst_wrap;
    /* do last line */
    deinterlace_line(dst,src_m2,src_m1,src_0,src_0,src_0,width);
}

/************************************************************************/
static BOOL g_bLibInit = FALSE;

int __stdcall NETDVR_startup(void)
{
	if (g_bLibInit)
	{
		return NETDVR_SUCCESS;
	}
	//yaogang modify 201401028
#if 1
	AllocConsole();
	int nRet = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	FILE* fp = _fdopen(nRet, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("控制台输出\n");
#endif
	CreateMutexHandle(&g_pool_lock);

	LockMutex(g_pool_lock);
	
	memset(&g_dvr_pool, 0 , sizeof(g_dvr_pool));
	CreateMutexHandle(&g_dvr_pool.port_lock);
	for (int i = 0; i < MAX_DVR_NUM; i++)
	{
		CreateMutexHandle(&g_dvr_pool.dvr_lock[i]);
	}

	UnlockMutex(g_pool_lock);
	
	memset(&g_decmsgQ, 0, sizeof(g_decmsgQ));

	BOOL bRet = initMsgQ(&g_decmsgQ, MAX_SAVE_FRM, sizeof(FrameMsg_t));
	if (!bRet)
	{
		return NETDVR_ERR_OUTOFMEMORY;
	}

	g_bDecVideoThreadRun = TRUE;
	g_hDecThreadEvent = CreateEvent(NULL,FALSE/*自动恢复为无信号状态*/,FALSE/*FALSE表示默认为无信号状态*/,NULL);
	g_hDecVideoThread=CreateThread(NULL, 0, DecVideoFrameThread, NULL, NULL, NULL);
	if (!g_hDecVideoThread)
	{
		return NETDVR_ERR_OUTOFMEMORY;
	}
//	SetThreadPriority(g_hDecVideoThread, THREAD_PRIORITY_HIGHEST);

	g_bReConnectThreadRun = TRUE;
	g_hReConnectThreadEvent = CreateEvent(NULL,FALSE/*自动恢复为无信号状态*/,FALSE/*FALSE表示默认为无信号状态*/,NULL);
	g_hReConnectThread = CreateThread(NULL, 0, ReConnectThread, NULL, NULL, NULL);
	if (!g_hReConnectThread)
	{
		return NETDVR_ERR_OUTOFMEMORY;
	}

	SetThreadPriority(g_hReConnectThread, THREAD_PRIORITY_LOWEST);

	//::MessageBox(NULL, "123", NULL, MB_OK);
#ifdef USE_CONNMSG_THREAD
	g_bConnMsgThreadRun = TRUE;
	//g_hConnMsgThreadEvent = CreateEvent(NULL,FALSE/*自动恢复为无信号状态*/,FALSE/*FALSE表示默认为无信号状态*/,NULL);
	g_hConnMsgThread = CreateThread(NULL, 0, DealConnectMsgThread, NULL, NULL, &g_dwConnMsgThreadID);
	if (!g_hConnMsgThread)
	{
		//::MessageBox(NULL, "456", NULL, MB_OK);
		return NETDVR_ERR_OUTOFMEMORY;
	}	
#endif
	//::MessageBox(NULL, "789", NULL, MB_OK);

	memset(g_dvrExist, 0, sizeof(g_dvrExist));
	WORD port = GetOneUnUsingPort(ACKSEARCHPORT, UDPPORT);
	SetCliAckPort(port);
	CPLibInit(0);	

	g_bLibInit = TRUE;
	return NETDVR_SUCCESS; 
}

//#define CLEANDUR

int __stdcall NETDVR_cleanup(void)
{
	if (!g_bLibInit)
	{
		return NETDVR_SUCCESS;
	}
	
#ifdef CLEANDUR
	DWORD dwTotalStart = GetTickCount();
	
	FILE* fp = fopen("c:\\cleanup.txt", "at");
	if (fp)
	{
		fprintf(fp, "*********************%d\r\n",g_dvr_pool.count);
		fflush(fp);
	}
#endif	
	
	unsigned int i;
	for (i = 0; i < g_dvr_pool.count; i++)
	{
		NETDVR_destroyDVR((int)g_dvr_pool.p_dvr[i]);
	}
	
#ifdef CLEANDUR
	if (fp)
	{
		fprintf(fp, "destroyDVR %d\r\n", GetTickCount()-dwTotalStart);
		fflush(fp);
	}
#endif
	
	if (g_hDecVideoThread)
	{
		g_bDecVideoThreadRun = FALSE;
		WaitForSingleObject (g_hDecThreadEvent, INFINITE);
// 		WaitForSingleObject (g_hDecVideoThread, 100/*INFINITE*/) ;
//		TerminateThread(g_hDecVideoThread,0);
		CloseHandle(g_hDecVideoThread);
		g_hDecVideoThread=NULL;

		CloseHandle(g_hDecThreadEvent);
		g_hDecThreadEvent = NULL;
		destroyMsgQ(&g_decmsgQ);
	}

#ifdef CLEANDUR
	if (fp)
	{
		fprintf(fp, "quitdecthread %d\r\n", GetTickCount()-dwTotalStart);
		fflush(fp);
	}
#endif

	if (g_hReConnectThread)
	{
		g_bReConnectThreadRun = FALSE;
		WaitForSingleObject (g_hReConnectThreadEvent, INFINITE);
		CloseHandle(g_hReConnectThread);
		g_hReConnectThread=NULL;
		
		CloseHandle(g_hReConnectThreadEvent);
		g_hReConnectThreadEvent = NULL;
	}

#ifdef CLEANDUR
	if (fp)
	{
		fprintf(fp, "quitreconnthread %d\r\n", GetTickCount()-dwTotalStart);
		fflush(fp);
	}
#endif

#ifdef USE_CONNMSG_THREAD
	if (g_hConnMsgThread)
	{
		PostThreadMessage(g_dwConnMsgThreadID, THREAD_QUIT, NULL, NULL);

#ifdef CLEANDUR
		if (fp)
		{
			fprintf(fp, "WaitForSingleObject111\r\n");
			fflush(fp);
		}
#endif
		g_bConnMsgThreadRun = FALSE;
		//WaitForSingleObject(g_hConnMsgThreadEvent, INFINITE);

#ifdef CLEANDUR
		if (fp)
		{
			fprintf(fp, "WaitForSingleObject222\r\n");
			fflush(fp);
		}
#endif
		CloseHandle(g_hConnMsgThread);
		g_hConnMsgThread=NULL;
		g_dwConnMsgThreadID = 0;

		//CloseHandle(g_hConnMsgThreadEvent);
		g_hConnMsgThreadEvent = NULL;
	}	
#endif

#ifdef CLEANDUR
	if (fp)
	{
		fprintf(fp, "quitConnMsgThread %d\r\n", GetTickCount()-dwTotalStart);
		fflush(fp);
	}
#endif

	LockMutex(g_pool_lock);
	
	CloseMutexHandle(g_dvr_pool.port_lock);
	g_dvr_pool.port_lock = NULL;
	for (i = 0; i < MAX_DVR_NUM; i++)
	{
		CloseMutexHandle(g_dvr_pool.dvr_lock[i]);
		g_dvr_pool.dvr_lock[i] =NULL;
	}

	UnlockMutex(g_pool_lock);
	
	CloseMutexHandle(g_pool_lock);
	g_pool_lock= NULL;

#ifdef CLEANDUR
	if (fp)
	{
		fprintf(fp, "closelockmutex %d\r\n", GetTickCount()-dwTotalStart);
		fflush(fp);
	}
#endif

	CPLibCleanup(); 

#ifdef CLEANDUR
	if (fp)
	{
		fprintf(fp, "CPLibCleanup %d\r\n", GetTickCount()-dwTotalStart);
		fflush(fp);
	}
#endif

#ifdef _CRTDBG_MAP_ALLOC
	_CrtDumpMemoryLeaks();
#endif

 	memset(g_dvrExist, 0, sizeof(g_dvrExist));

#ifdef CLEANDUR
	if (fp)
	{
		fprintf(fp, "endover %d\r\n", GetTickCount()-dwTotalStart);
		fflush(fp);
		fclose(fp);
	}
#endif
	
	g_bLibInit = FALSE;
	
	return NETDVR_SUCCESS; 
} 

int __stdcall NETDVR_createDVRbyDomain(int *p_handle, char *pDomain, unsigned short serverport)
{

	if (!p_handle || !pDomain)
	{
		return NETDVR_ERR_PARAM;
	}

	u32 dwServerip = Domain2IP(pDomain);

	int ret = NETDVR_createDVR(p_handle,dwServerip,serverport);
	if (ret == NETDVR_SUCCESS)
	{
		struct NETDVR_INNER_t *p;
		p = (struct NETDVR_INNER_t*)(*p_handle);
		strncpy(p->SvrDomain,pDomain,min(sizeof(p->SvrDomain),strlen(pDomain)));
	}
	return ret;
}

int __stdcall NETDVR_createDVR(int *p_handle,unsigned int serverip, unsigned short serverport)
{
	struct NETDVR_INNER_t *p;
	int i;
	
	if (!p_handle)
	{
		return NETDVR_ERR_PARAM;
	}

 	LockMutex(g_pool_lock);
	if (g_dvr_pool.count >= MAX_DVR_NUM)
	{
		UnlockMutex(g_pool_lock);
		*p_handle = 0;
		return NETDVR_ERROR;
	}
	
	/* create handle  */
	p = (struct NETDVR_INNER_t *)malloc(sizeof(struct NETDVR_INNER_t)); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		*p_handle = 0;
		return NETDVR_ERR_OUTOFMEMORY;
	}

	/* init handle */
	memset(p, 0, sizeof(struct NETDVR_INNER_t));


	/* set server info */

	SetMsgCallBack((pMESSAGE_CB)DealNotify, NULL);
	
	//
	UnlockMutex(g_pool_lock);
	CPHandle tmpcph = CPConnect(serverip, serverport, g_connect_timeout, NULL);
	//p->cph = CPConnect(serverip, serverport, g_connect_timeout, NULL);
	LockMutex(g_pool_lock);
	p->cph = tmpcph;
	//modify by cj 20110817

	if (p->cph == NULL)
	{
		free(p);
		UnlockMutex(g_pool_lock);
		*p_handle = 0;
		return NETDVR_ERR_NEW_CONNECTION;
	}

	int ret = NETDVR_GetDeviceInfo((int)p, &p->si);
	if (ret)
	{
		free(p);
		UnlockMutex(g_pool_lock);
		*p_handle = 0;
		return ret;
	}

	p->si.deviceIP = serverip;
	p->si.devicePort = serverport;
	ret = NETDVR_GetVideoProperty((int)p, &p->video_property);
	if (ret)
	{
		free(p);
		UnlockMutex(g_pool_lock);
		*p_handle = 0;
		return ret;
	}

	ret = NETDVR_GetAudioProperty((int)p, &p->audio_property);
	if (ret)
	{
		free(p);
		UnlockMutex(g_pool_lock);
		*p_handle = 0;
		return ret;
	}

	ret = NETDVR_GetVoipProperty((int)p, &p->voip_property);
	if (ret)
	{
		free(p);
		UnlockMutex(g_pool_lock);
		*p_handle = 0;
		return ret;
	}
	
	for (i = 0; i< MAX_DVR_NUM; i++)
	{
		if (0 == g_dvrExist[i])
		{
			p->dvr_id = i;
			g_dvrExist[i] = 1;
			break;
		}
	}

// 	for (i = 0; i < p->si.maxChnNum; i++)
// 	{
// 		CreateMutexHandle(&p->record_para[i].cb_rec_lock);
// 		CreateMutexHandle(&p->record_para[i].cb_filename_lock);
// 
// 		CreateMutexHandle(&p->record_para2[i].cb_rec_lock);
// 		CreateMutexHandle(&p->record_para2[i].cb_filename_lock);
// 
// 		CreateMutexHandle(&p->snap_shot[i].snap_lock);
// 	}
// 
// 	for (i = 0; i < p->si.maxSubstreamNum; i++)
// 	{
// 		CreateMutexHandle(&p->sub_record_para[i].cb_rec_lock);
// 		CreateMutexHandle(&p->sub_record_para[i].cb_filename_lock);
// 		
// 		CreateMutexHandle(&p->sub_snap_shot[i].snap_lock);
// 	}

// 	for (i = 0; i < MAX_PLAYER_NUM; i++)
// 	{
// 
// 		CreateMutexHandle(&p->player_lock[i]);
// 	}
// 	
// 	for (i = 0; i < MAX_PLAYER_NUM; i++)
// 	{
// 		CreateMutexHandle(&p->getfileframe_lock[i]);
// 	}

	CreateMutexHandle(&p->file_reciever.reciever_lock);
	CreateMutexHandle(&p->update.update_lock);

	/* ====================  record handle ======================= */
	*p_handle = (int)p;

	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	g_dvr_pool.p_dvr[p->dvr_id] = p;
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	g_dvr_pool.count++;

	for (i=0; i<MEDIA_LINK_CLIENT; i++ )
	{
		p->m_mediarcv[i].sockfd = INVALID_SOCKET;
	}

	p->bRcvThreadState = TRUE;
	p->hRcvEvent = CreateEvent(NULL,FALSE/*自动恢复为无信号状态*/,FALSE/*FALSE表示默认为无信号状态*/,NULL);
	//ResetEvent(p->hEvent);
	::InitializeCriticalSection(&p->m_hcs);

	p->hRcvThread = CreateThread(NULL,0,RcvTcpFrameThread,p,0,NULL);

//	SetThreadPriority(p->hRcvThread, THREAD_PRIORITY_LOWEST);

	p->bEnableRecon = FALSE/*TRUE*/;
	p->b_cmdConnectLost = FALSE;
	UnlockMutex(g_pool_lock);

	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_destroyDVR(int Handle)
{
	struct NETDVR_INNER_t *p;
	int i;
	u16 dvr_id;
	
	//FILE* fp = fopen("c:\\destroyDVR.txt", "ab+");
	//fprintf(fp, "NETDVR_destroyDVR - 1, (0x%08x)\r\n",Handle);
	//fflush(fp);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	//csp modify
	NETDVR_logoutServer(Handle);
	
	//csp modify
	if (p->hRcvThread)
	{
		//fprintf(fp, "NETDVR_destroyDVR - 1.1, (0x%08x,0x%08x,0x%08x,%d)\r\n",p,p->hRcvThread,p->hRcvEvent,p->bRcvThreadState);
		//fflush(fp);
		
		p->bRcvThreadState = FALSE;
		WaitForSingleObject(p->hRcvEvent,INFINITE);
		
		//fprintf(fp, "NETDVR_destroyDVR - 1.2, (0x%08x,0x%08x,0x%08x,%d)\r\n",p,p->hRcvThread,p->hRcvEvent,p->bRcvThreadState);
		//fflush(fp);
		
		CloseHandle(p->hRcvThread);
		p->hRcvThread = NULL;
		
		CloseHandle(p->hRcvEvent);
		p->hRcvEvent = NULL;
	}
	
	if (g_m_hwo)
	{
		waveOutSetVolume(g_m_hwo,0xffffffff);
	}
	
// 	close_audio_out();
	
	//fprintf(fp, "NETDVR_destroyDVR - 2\r\n");
	//fflush(fp);
	
	NETDVR_regCBMsgConnLost(Handle, NULL, NULL);
	NETDVR_regCBAlarmState(Handle, NULL, NULL);
	
	//fprintf(fp, "NETDVR_destroyDVR - 3\r\n");
	//fflush(fp);
	
	NETDVR_SetAlarmUpload(Handle, 0);
	
	//fprintf(fp, "NETDVR_destroyDVR - 4\r\n");
	//fflush(fp);
	
	ifly_monitor_t* pMonitor = p->m_pMonitor;
	ifly_monitor_t* pTmp = NULL;
	while(pMonitor)
	{
		pTmp = pMonitor->pNext;
		NETDVR_StopRealPlay((int)pMonitor);
		pMonitor = pTmp;
	}
	
	//fprintf(fp, "NETDVR_destroyDVR - 5\r\n");
	//fflush(fp);
	
	ifly_playback_t* pPlayBack = p->m_pPlayBack;
	ifly_playback_t* pTmp2 = NULL;
	while(pPlayBack)
	{
		pTmp2 = pPlayBack->pNext;
		NETDVR_stopPlayBack((int)pPlayBack);
		pPlayBack = pTmp2;
	}
	
	//fprintf(fp, "NETDVR_destroyDVR - 6\r\n");
	//fflush(fp);
	
	PRI_MotionFind_t* pMDFind = p->m_pMDList;
	PRI_MotionFind_t* pTmp3 = NULL;
	while(pMDFind)
	{
		pTmp3 = pMDFind->pNext;
		NETDVR_FindMotionClose((int)pMDFind);
		pMDFind = pTmp3;
	}
	
	//fprintf(fp, "NETDVR_destroyDVR - 7\r\n");
	//fflush(fp);
	
	for (i = 0; i < MAX_VOIP_NUM; i++)
	{
		NETDVR_stopVOIP(Handle, i);
	}
	
	//fprintf(fp, "NETDVR_destroyDVR - 8\r\n");
	//fflush(fp);
	
	for (i = 0; i < MAX_SERIALPORT; i++)
	{
		NETDVR_SerialStop(Handle, i+1);
	}
	
	//fprintf(fp, "NETDVR_destroyDVR - 9\r\n");
	//fflush(fp);
	
	NETDVR_stopFileDownload(Handle);
	
	//fprintf(fp, "NETDVR_destroyDVR - 10\r\n");
	//fflush(fp);
	
	NETDVR_stopUpdate(Handle);
	
	//fprintf(fp, "NETDVR_destroyDVR - 11\r\n");
	//fflush(fp);
	
	//csp modify
	//NETDVR_logoutServer(Handle);
	
	//fprintf(fp, "NETDVR_destroyDVR - 12\r\n");
	//fflush(fp);
	
	LockMutex(g_pool_lock);
	
	if (0 == g_dvr_pool.count)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	dvr_id = p->dvr_id;
 	g_dvrExist[p->dvr_id] = 0;
	
	CloseMutexHandle(p->file_reciever.reciever_lock);
	p->file_reciever.reciever_lock = NULL;
	CloseMutexHandle(p->update.update_lock);
	p->update.update_lock = NULL;
	
	//fprintf(fp, "NETDVR_destroyDVR - 13\r\n");
	//fflush(fp);
	
	for (i=0; i<MEDIA_LINK_CLIENT; i++ )
	{
		if (p->m_mediarcv[i].sockfd != INVALID_SOCKET)
		{
			closesocket(p->m_mediarcv[i].sockfd);
			p->m_mediarcv[i].sockfd = INVALID_SOCKET;
		}
	}
	
	//csp modify
	/*if (p->hRcvThread)
	{
		p->bRcvThreadState = FALSE;
		WaitForSingleObject(p->hRcvEvent,INFINITE);
		
		CloseHandle(p->hRcvThread);
		p->hRcvThread = NULL;
		
		CloseHandle(p->hRcvEvent);
		p->hRcvEvent = NULL;
	}*/
	
	if (p->cph)
	{
		CleanCPHandle(p->cph);	// close socket and reset to 0; 
		p->cph = NULL;
	}
	p->b_cmdConnectLost = FALSE;
	
	/* free dvr */
	g_dvr_pool.p_dvr[dvr_id] = NULL;
	g_dvr_pool.count--;
	
	//fprintf(fp, "NETDVR_destroyDVR - 14\r\n");
	//fflush(fp);
	
	::DeleteCriticalSection(&p->m_hcs);
	
	free(p);
	p = NULL;
	
	UnlockMutex(g_dvr_pool.dvr_lock[dvr_id]);
	UnlockMutex(g_pool_lock);
	
	//fprintf(fp, "NETDVR_destroyDVR - 15\r\n");
	//fflush(fp);
	
	return NETDVR_SUCCESS; 
}

int __stdcall NETDVR_regCBMsgConnLost(int Handle, PFUN_MSG_T p_cb_func, unsigned int dwContent)
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);

	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}

	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);

	UnlockMutex(g_pool_lock);

	p->p_cb_connlost = p_cb_func;
	p->dwContentConnlost = dwContent;

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);

	return NETDVR_SUCCESS; 
}

int __stdcall NETDVR_loginServer(int Handle, const struct NETDVR_loginInfo_t *pLoginInfo)
{
	ifly_loginpara_t login;
	struct NETDVR_INNER_t *p;
	int ret;
	char buf[2048] = {0};

	LockMutex(g_pool_lock);
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}

	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);

	UnlockMutex(g_pool_lock);

	if (NULL == p->cph)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERROR;
	}

	/* set login info */
	set_logininfo(p, pLoginInfo);//fill p->li

	login.ipAddr = p->li.ipAddr;
	strcpy(login.loginpass, p->li.loginpass);
	strcpy(login.macAddr, p->li.macAddr);
	strcpy(login.username, p->li.username);

	//::MessageBox(NULL, login.username, NULL, MB_OK);
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_LOGIN, &login, sizeof(login), buf, sizeof(buf), g_connect_timeout);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
#ifdef _DEBUG
	ret = NETDVR_SUCCESS;
#endif

	if (NETDVR_SUCCESS == ret)
	{
		p->b_login = TRUE;
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);	

	return ret;
}

int __stdcall NETDVR_logoutServer(int Handle)
{
	int ret; 
	ifly_loginpara_t login;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	login.ipAddr = p->li.ipAddr; 
	strcpy(login.loginpass, p->li.loginpass); 
	strcpy(login.macAddr, p->li.macAddr); 
	strcpy(login.username, p->li.username);
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_LOGOFF, &login, sizeof(login), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);	
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
// 	if (ret == CTRL_SUCCESS)
	{
		p->b_login = FALSE;
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret;
}

//csp modify 20130519
static BOOL IsBigEndian()
{
	const int n = 1;
	if(*(char *)&n)
	{
		return FALSE;
	}
	return TRUE;
}
#define swap64(val) ( \
	(((val) >> 56) & 0xff) |\
	(((val) & 0x00ff000000000000) >> 40) |\
	(((val) & 0x0000ff0000000000) >> 24) |\
	(((val) & 0x000000ff00000000) >> 8)  |\
	(((val) & 0x00000000ff000000) << 8)  |\
	(((val) & 0x0000000000ff0000) << 24) |\
	(((val) & 0x000000000000ff00) << 40) |\
	(((val) & 0x00000000000000ff) << 56) )
#define hton64(val) IsBigEndian()?(val):swap64(val)
#define ntoh64(val) hton64(val)
int __stdcall NETDVR_GetAdvPrivilege(int Handle, const struct NETDVR_loginInfo_t *pLoginInfo, struct NETDVR_AdvPrivilege_t *pAdvPrivilege)
{
	ifly_loginpara_t login;
	struct NETDVR_INNER_t *p;
	int ret;
	char buf[2048] = {0};
	
	LockMutex(g_pool_lock);
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (NULL == p->cph/* || !p->b_login*/)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERROR;
	}
	
	/* set login info */
	//set_logininfo(p, pLoginInfo);
	
	//login.ipAddr = p->li.ipAddr;
	//strcpy(login.loginpass, p->li.loginpass);
	//strcpy(login.macAddr, p->li.macAddr);
	//strcpy(login.username, p->li.username);
	
	login.ipAddr = pLoginInfo->ipAddr;
	strcpy(login.loginpass, pLoginInfo->loginpass);
	strcpy(login.macAddr, pLoginInfo->macAddr);
	strcpy(login.username, pLoginInfo->username);
	
	//::MessageBox(NULL, login.username, NULL, MB_OK);
	
	if (!p->b_cmdConnectLost)
	{
		ret = NETDVR_ERR_SEND;
		//ret = send_command(p->cph, CTRL_CMD_GETADVPRIVILEGE, &login, sizeof(login), buf, sizeof(buf), g_connect_timeout);
	}
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_AdvPrivilege_t para_info;
		memset(&para_info,0,sizeof(para_info));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		
		memset(pAdvPrivilege,0,sizeof(*pAdvPrivilege));
		strcpy(pAdvPrivilege->username,para_info.username);
		pAdvPrivilege->nRemoteView[0] = ntoh64(para_info.nRemoteView[0]);
	}
	else
	{
		memset(pAdvPrivilege,0,sizeof(*pAdvPrivilege));
		strcpy(pAdvPrivilege->username,pLoginInfo->username);
		pAdvPrivilege->nRemoteView[0] = (u64)(-1);
	}
	
	//if (NETDVR_SUCCESS == ret)
	//{
	//	p->b_login = TRUE;
	//}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret;
}

int __stdcall NETDVR_GetDeviceInfo(int Handle, struct NETDVR_DeviceInfo_t *pDeviceInfo)
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (NULL == p->cph)
	{
		return NETDVR_ERROR;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETDEVICEINFO, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);	
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_DeviceInfo_t para_info;
		memset(&para_info,0,sizeof(ifly_DeviceInfo_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_DeviceInfo_t));
		pDeviceInfo->deviceIP = para_info.deviceIP;
		pDeviceInfo->devicePort = ntohs(para_info.devicePort);
		memcpy(pDeviceInfo->device_name, para_info.device_name,sizeof(para_info.device_name));
		memcpy(pDeviceInfo->device_mode, para_info.device_mode,sizeof(para_info.device_mode));
		pDeviceInfo->maxChnNum = para_info.maxChnNum;
		pDeviceInfo->maxAudioNum = para_info.maxAduioNum;
		pDeviceInfo->maxSubstreamNum = para_info.maxSubstreamNum;
		pDeviceInfo->maxPlaybackNum = para_info.maxPlaybackNum;
		pDeviceInfo->maxAlarmInNum = para_info.maxAlarmInNum;
		pDeviceInfo->maxAlarmOutNum = para_info.maxAlarmOutNum;
		pDeviceInfo->maxHddNum = para_info.maxHddNum;
		pDeviceInfo->nNVROrDecoder = para_info.nNVROrDecoder;
	}
	
	return ret;
}

int __stdcall NETDVR_GetVideoProperty(int Handle, struct NETDVR_VideoProperty_t *pVideoPro)
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (NULL == p->cph)
	{
		return NETDVR_ERROR;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETVIDEOPROPERTY, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_Video_Property_t para_info;
		memset(&para_info,0,sizeof(ifly_Video_Property_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_Video_Property_t));
		pVideoPro->videoEncType = para_info.videoEncType;
		pVideoPro->max_videowidth = ntohs(para_info.max_videowidth);
		pVideoPro->max_videoheight = ntohs(para_info.max_videoheight);
	}
	
	return ret;
}

int __stdcall NETDVR_GetAudioProperty(int Handle, struct NETDVR_AudioProperty_t *pAudioPro)
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (NULL == p->cph)
	{
		return NETDVR_ERROR;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETAUDIOPROPERTY, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_Audio_Property_t para_info;
		memset(&para_info,0,sizeof(ifly_Audio_Property_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_Audio_Property_t));
		pAudioPro->audioBitPerSample = para_info.audioBitPerSample;
		pAudioPro->audioEnctype = para_info.audioEnctype;
		pAudioPro->audioSamplePerSec = ntohs(para_info.audioSamplePerSec);
		pAudioPro->audioFrameSize = ntohs(para_info.audioFrameSize);
		pAudioPro->audioFrameDurTime = ntohs(para_info.audioFrameDurTime);
	}
	
	return ret;
}

int __stdcall NETDVR_GetVoipProperty(int Handle, struct NETDVR_VOIPProperty_t *pVoipPro)
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (NULL == p->cph)
	{
		return NETDVR_ERROR;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETVOIPPROPERTY, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_VOIP_Property_t para_info;
		memset(&para_info,0,sizeof(ifly_VOIP_Property_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_VOIP_Property_t));
		pVoipPro->VOIPBitPerSample = para_info.VOIPBitPerSample;
		pVoipPro->VOIPFrameSize = ntohs(para_info.VOIPFrameSize);
		pVoipPro->VOIPSamplePerSec = ntohs(para_info.VOIPSamplePerSec);
	}
	
	return ret;
}

int __stdcall NETDVR_GetMDProperty(int Handle, struct NETDVR_MDProperty_t *pMDPro)
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (NULL == p->cph)
	{
		return NETDVR_ERROR;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETMDPROPERTY, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_MD_Property_t para_info;
		memset(&para_info,0,sizeof(ifly_MD_Property_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_MD_Property_t));
		pMDPro->MDCol = para_info.MDCol;
		pMDPro->MDRow = para_info.MDRow;
	}
	
	return ret;
}


int __stdcall NETDVR_getSystemParams(int Handle, struct NETDVR_systemParam_t *pSysPara)
{
	int ret = 0; 
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETSYSPARAM, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_SysParam_t desc; 
		memset(&desc, 0, sizeof(desc));
		memcpy(&desc,buf+sizeof(ifly_cp_header_t),sizeof(ifly_SysParam_t));
		pSysPara->device_id = ntohs(desc.device_id);
		pSysPara->flag_overlap = (enum NETDVR_OVERLAP)desc.flag_overwrite;
		pSysPara->flag_statusdisp = (enum NETDVR_DVRSTATUS)desc.flag_statusdisp;
		pSysPara->languageindex = desc.language;
		pSysPara->lock_time = (enum NETDVR_LOCKTIME)(ntohs(desc.lock_time));
		pSysPara->disable_pw = desc.disable_pw;
		pSysPara->switch_time = (enum NETDVR_SWITCHTIME)(ntohs(desc.switch_time));
		pSysPara->transparency = (enum NETDVR_TRANSPARENCY)desc.transparency;
		pSysPara->vga_solution = desc.vga_solution;
		pSysPara->video_format = (enum NETDVR_VIDEOFORMAT)desc.video_format;
		pSysPara->flag_bestrow = desc.flag_bestrow;
		memcpy(pSysPara->device_name,desc.device_name,sizeof(desc.device_name));
		memcpy(pSysPara->reserved, desc.reserved, sizeof(desc.reserved));
	}
	
	return ret; 

}

int __stdcall NETDVR_setSystemParams(int Handle, const struct NETDVR_systemParam_t *pSysPara)
{
	int ret = 0; 
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_SysParam_t desc;
	memset(&desc, 0, sizeof(desc));
	desc.device_id = htons(pSysPara->device_id);
	memcpy(desc.device_name,pSysPara->device_name,sizeof(desc.device_name));
	desc.disable_pw = pSysPara->disable_pw;
	desc.flag_overwrite = pSysPara->flag_overlap;
	desc.flag_statusdisp = pSysPara->flag_statusdisp;
	desc.language = pSysPara->languageindex;
	desc.lock_time = htons(pSysPara->lock_time);
	desc.switch_time = htons(pSysPara->switch_time);
	desc.transparency = pSysPara->transparency;
	desc.vga_solution = pSysPara->vga_solution;
	desc.video_format = pSysPara->video_format;
	desc.flag_bestrow = pSysPara->flag_bestrow;
	memcpy(desc.reserved, pSysPara->reserved, sizeof(desc.reserved));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETSYSPARAM, &desc, sizeof(desc), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_setPtzParams(int Handle, const struct NETDVR_ptzParam_t *p_ptz_param)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[1024];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_ptz_param)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_PTZParam_t ptzinfo;
	memset(&ptzinfo, 0, sizeof(ptzinfo));
	ptzinfo.chn = p_ptz_param->chn;
	ptzinfo.address = htons(p_ptz_param->address);
	ptzinfo.baud_ratesel = p_ptz_param->baud_ratesel;
	ptzinfo.copy2Chnmask = htonl(p_ptz_param->copy2Chnmask);
	ptzinfo.crccheck = p_ptz_param->check_type;
	ptzinfo.data_bitsel = p_ptz_param->data_bitsel;
	ptzinfo.flow_control = p_ptz_param->flow_control;
	ptzinfo.protocol = p_ptz_param->protocol;
	ptzinfo.stop_bitsel = p_ptz_param->stop_bitsel;
	ptzinfo.comstyle = p_ptz_param->comstyle;
	
	//csp modify
	ptzinfo.enableptz = p_ptz_param->enableptz;
	
	//char szTmp[32];
	//sprintf(szTmp,"NETDVR_setPtzParams-enableptz=%d\n",ptzinfo.enableptz);
	//::MessageBox(NULL,szTmp,NULL,MB_OK);
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETPTZPARAM, &ptzinfo, sizeof(ptzinfo), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_getPtzParams(int Handle, unsigned char chn, struct NETDVR_ptzParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	
	char buf[2048] = {0};
	
	ifly_PTZParam_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETPTZPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->address = ntohs(para_info.address);
		p_para->baud_ratesel = (enum NETDVR_BAUDRATE)para_info.baud_ratesel;
		p_para->chn = para_info.chn;
		p_para->check_type = (enum NETDVR_CHECK_TYPE)para_info.crccheck;
		p_para->data_bitsel = (enum NETDVR_DATABITSEL)para_info.data_bitsel;
		p_para->flow_control = (enum NETDVR_FLOWCONTROL)para_info.flow_control;
		p_para->protocol = (enum NETDVR_PROTOCOLTYPE)para_info.protocol;
		p_para->stop_bitsel = (enum NETDVR_STOPBITSEL)para_info.stop_bitsel;
		p_para->comstyle = para_info.comstyle;
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);
		
		//csp modify
		p_para->enableptz = para_info.enableptz;
	}
	
	return ret;
}

int __stdcall NETDVR_PtzControl(int Handle, const struct NETDVR_PtzCtrl_t *p_para)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[1024] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (p_para->chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_PtzCtrl_t ptzctrl;
	memset(&ptzctrl, 0, sizeof(ptzctrl));
// 	ptzctrl.chn = chn;
// 	ptzctrl.cmd = ptz_cmd;
//	memcpy(&ptzctrl,p_para,sizeof(NETDVR_PtzCtrl_t));
 	ptzctrl.chn = p_para->chn;
 	ptzctrl.cmd = p_para->cmd;
 	ptzctrl.reserved = p_para->aux;

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_PTZCTRL, &ptzctrl, sizeof(ptzctrl), buf,sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;

}

int __stdcall NETDVR_SetYTPresetPoint(int Handle, unsigned char chn, unsigned char preset_pos, enum NETDVR_YTPRESETPOINTCONTROL yt_com)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (0==preset_pos || preset_pos > MAX_PRESET_NUM) //1-128
	{
		return NETDVR_ERR_PARAM;
	}
	
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_PtzPresetCtr_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	para_info.chn = chn;
	para_info.presetpoint = preset_pos;
	para_info.option = yt_com;
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETPRESET, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;	
}




int __stdcall NETDVR_SetYTTrack(int Handle, unsigned char chn, enum NETDVR_YTTRACKCONTROL yt_cmd)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	ifly_TrackCtr_t para_info;
	memset(&para_info,0,sizeof(ifly_TrackCtr_t));
	para_info.chn = chn;
	para_info.flagoption = yt_cmd;

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_CTRLPTZTRACK, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;

}

int __stdcall NETDVR_SetCruiseParam(int Handle, const struct NETDVR_cruisePath_t *p_para)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_PtzCruisePathParam_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	para_info.chn = p_para->chn;
	para_info.cruise_path = p_para->path_no;
	memcpy(para_info.Cruise_point,p_para->cruise_pos,sizeof(para_info.Cruise_point));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETCRUISEPARAM, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;
}

int __stdcall NETDVR_GetCruiseParam(int Handle, unsigned char chn, unsigned char pathnum, struct NETDVR_cruisePath_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};

	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	ifly_PtzCruisePathParam_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	char tempbuf[2] = {0};
	tempbuf[0] = chn;
	tempbuf[1] = pathnum;
// 	memcpy(tempbuf,&chn,sizeof(chn));
// 	memcpy(tempbuf + 1,&pathnum,sizeof(pathnum));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETCRUISEPARAM, tempbuf, sizeof(tempbuf), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->chn = para_info.chn;
		p_para->path_no = para_info.cruise_path;
		memcpy(p_para->cruise_pos,para_info.Cruise_point,sizeof(para_info.Cruise_point));

	}
	
	return ret;
}



int __stdcall NETDVR_startYuntaiCruise(int Handle, unsigned char chn, unsigned char path_no)
{
	int ret;
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (0==path_no || path_no > MAX_CRUISE_PATH_NUM)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_CruisePathCtr_t para_info;
	memset(&para_info,0,sizeof(para_info));
	para_info.chn = chn;
	para_info.cruisepath = path_no;
	para_info.flagoption = 1;

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_CTRLCRUISEPATH,&para_info,sizeof(para_info),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	


	return ret;

}

int __stdcall NETDVR_stopYuntaiCruise (int Handle, unsigned char chn, unsigned char path_no)
{
	int ret;
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (0==path_no || path_no > MAX_CRUISE_PATH_NUM)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_CruisePathCtr_t para_info;
	memset(&para_info,0,sizeof(para_info));
	para_info.chn = chn;
	para_info.cruisepath = path_no;
	para_info.flagoption = 0;
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_CTRLCRUISEPATH,&para_info,sizeof(para_info),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	
	return ret;

}

int __stdcall NETDVR_startVOIP(int Handle, int voipindex)
{	
	struct NETDVR_INNER_t *p;
	int ret;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (voipindex >= MAX_VOIP_NUM)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}
	
	if (p->b_voip&(1<<voipindex))
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}
	
	if (p->voip_rcv[voipindex].p_frmdecBuf)
	{
		free(p->voip_rcv[voipindex].p_frmdecBuf);
		p->voip_rcv[voipindex].p_frmdecBuf = NULL;
	}
	
	if (p->voip_rcv[voipindex].p_frmdecBuf)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_RERECIVE;
	}
	
	if (p->voip_rcv[voipindex].bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_BUSY;
	}

	p->voip_rcv[voipindex].rcv_type = NETDVR_RCV_VOIP;
	/*voip reciever*/

// 	waveOutSetVolume(g_m_hwo,0xffffffff);
// 	close_audio_out();
	
	p->voip_rcv[voipindex].pAudioPlay = new CAudioPlay;
	if (p->voip_rcv[voipindex].pAudioPlay)
	{
		if(!p->voip_rcv[voipindex].pAudioPlay->Init(p->voip_property.VOIPBitPerSample, p->voip_property.VOIPSamplePerSec))
		{
// 			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
// 			return NETDVR_ERROR;
		}
		g_currAudioPlay = p->voip_rcv[voipindex].pAudioPlay;
	}

// 	if (!open_audio_out(p->voip_property.VOIPBitPerSample, p->voip_property.VOIPSamplePerSec))
// 	{
// 		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
// 		return NETDVR_ERROR;
// 	}

	p->voip_rcv[voipindex].p_frmdecBuf = (unsigned char *)malloc(MAX_AUDIO_DECLEN);
	if (NULL == p->voip_rcv[voipindex].p_frmdecBuf)
	{
		if (p->voip_rcv[voipindex].pAudioPlay)
		{
			delete p->voip_rcv[voipindex].pAudioPlay;
			p->voip_rcv[voipindex].pAudioPlay = NULL;
		}
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_OUTOFMEMORY;
	}
	memset(p->voip_rcv[voipindex].p_frmdecBuf, 0, MAX_AUDIO_DECLEN);

	ret = open_reciever(&p->voip_rcv[voipindex], NULL, 0, 0);

	if (NETDVR_SUCCESS != ret)
	{
		if (p->voip_rcv[voipindex].p_frmdecBuf)
		{
			free(p->voip_rcv[voipindex].p_frmdecBuf);
			p->voip_rcv[voipindex].p_frmdecBuf = NULL;
		}

		if (p->voip_rcv[voipindex].pAudioPlay)
		{
			delete p->voip_rcv[voipindex].pAudioPlay;
			p->voip_rcv[voipindex].pAudioPlay = NULL;
		}
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return ret;
	}

	p->voip_rcv[voipindex].req.command = 5;
	p->voip_rcv[voipindex].req.voipreserved = 0;
	p->voip_rcv[voipindex].p_audio_property = &p->audio_property;
	//if (0==p->voip_snd[voipindex].sndmode)
	{
		BOOL bret = SetRcvTcpFrame(p, &p->voip_rcv[voipindex].prcv_t, p->voip_rcv[voipindex].req, TRUE);
		if (!bret)
		{
			if (p->voip_rcv[voipindex].p_frmdecBuf)
			{
				free(p->voip_rcv[voipindex].p_frmdecBuf);
				p->voip_rcv[voipindex].p_frmdecBuf = NULL;
			}
	
			close_reciever(&p->voip_rcv[voipindex]);
			if (p->voip_rcv[voipindex].pAudioPlay)
			{
				delete p->voip_rcv[voipindex].pAudioPlay;
				p->voip_rcv[voipindex].pAudioPlay = NULL;
			}
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_SOCKET;
		}
	}
	
	/*voip sender*/
	p->voip_snd[voipindex].psnd_t = p->voip_rcv[voipindex].prcv_t;
	p->voip_snd[voipindex].m_bInterPhoneEnding = FALSE;
	p->voip_snd[voipindex].p_voip_property = &p->voip_property;

	if (S_OK != open_voip_sender(&p->voip_snd[voipindex]))
	{
		if (p->voip_rcv[voipindex].p_frmdecBuf)
		{
			free(p->voip_rcv[voipindex].p_frmdecBuf);
			p->voip_rcv[voipindex].p_frmdecBuf = NULL;
		}

		close_reciever(&p->voip_rcv[voipindex]);
// 		close_audio_out();
// 		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
// 		return NETDVR_ERROR;
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);

	p->b_voip |= (1 << voipindex);
	p->voip_rcv[voipindex].bOpened = 1;
//	Sleep(500);

	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_stopVOIP(int Handle, int voipindex)
{
	struct NETDVR_INNER_t *p;
	int ret;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (voipindex >= MAX_VOIP_NUM)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}
	
	if (!(p->b_voip&(1<<voipindex)))
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}

	
	if (!p->voip_rcv[voipindex].bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}

	char buf[2048] = {0};
	if (p->voip_rcv[voipindex].prcv_t)
	{
		u32 id = htonl(p->voip_rcv[voipindex].prcv_t->linkid);

		if (!p->b_cmdConnectLost)
		{
			ret = send_command(p->cph, CTRL_CMD_STOPVOIP, &id, sizeof(id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
		} 
		else
		{
			ret = NETDVR_ERR_SEND;
		}
		
	}
	
	SetRcvTcpFrame(p, &p->voip_rcv[voipindex].prcv_t, p->voip_rcv[voipindex].req, FALSE);

	//把系统波形调至为0,为了避免缓冲数据放出
	//waveOutSetVolume(g_m_hwo,0);
	
	close_reciever(&p->voip_rcv[voipindex]);

	//不关闭waveoutclose是因为频繁open或close会造成关闭不完全致使声音采集失败
// 	close_audio_out();
	if (p->voip_rcv[voipindex].pAudioPlay)
	{
		delete p->voip_rcv[voipindex].pAudioPlay;
		p->voip_rcv[voipindex].pAudioPlay = NULL;
	}
	//stop sending voip

	p->voip_snd[voipindex].m_bInterPhoneEnding = TRUE;
	if (p->voip_snd[voipindex].m_hwi)
	{
		waveInUnprepareHeader(p->voip_snd[voipindex].m_hwi, &p->voip_snd[voipindex].m_wh1, sizeof(p->voip_snd[voipindex].m_wh1));
		waveInUnprepareHeader(p->voip_snd[voipindex].m_hwi, &p->voip_snd[voipindex].m_wh2, sizeof(p->voip_snd[voipindex].m_wh2));
		waveInStop(p->voip_snd[voipindex].m_hwi);
		waveInClose(p->voip_snd[voipindex].m_hwi);
		p->voip_snd[voipindex].m_hwi=NULL;
	}
	


	p->b_voip &= ~(1 << voipindex);

	p->voip_rcv[voipindex].bOpened = 0;
	if (p->voip_rcv[voipindex].p_frmdecBuf)
	{
		free(p->voip_rcv[voipindex].p_frmdecBuf);
		p->voip_rcv[voipindex].p_frmdecBuf = NULL;
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	Sleep(500);
	
	return ret;
}

int __stdcall NETDVR_logSearch(int Handle, struct NETDVR_logSearchCondition_t *p_condition, struct NETDVR_logSearchResult_t *p_result)
{
	struct NETDVR_INNER_t *p;
	int ret = NETDVR_SUCCESS, i;
	
	char buf[2048]={0};
	
	ifly_search_desc_t desc; 
	ifly_logInfo_t     logback; 
	struct NETDVR_logInfo_t *pi, *qi; 
	ifly_search_log_t searchLog; 
	
	char *ptmp = NULL;
	if (!p_condition || !p_result)
	{
		return NETDVR_ERR_PARAM;
	}
	
	if ((p_condition->startID < 1) || (p_condition->max_return >52))
	{
		return NETDVR_ERR_PARAM;
	}
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	searchLog.query_mode = p_condition->query_mode;
	searchLog.main_type = p_condition->main_type;
	searchLog.slave_type = p_condition->slave_type;
	searchLog.start_time = htonl(p_condition->start_time);
	searchLog.end_time = htonl(p_condition->end_time);
	searchLog.max_return = htons(p_condition->max_return);
	searchLog.startID = htons(p_condition->startID);
	
	//csp modify for NVR
	struct timeb tb;
	ftime(&tb);
	searchLog.start_time = htonl(p_condition->start_time+tb.timezone*60);
	searchLog.end_time = htonl(p_condition->end_time+tb.timezone*60);
	
	memset(p_result, 0, sizeof(NETDVR_logSearchResult_t));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_LOGSEARCH, &searchLog, sizeof(ifly_search_log_t), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		ptmp = buf + sizeof(ifly_cp_header_t); 
		memcpy(&desc, ptmp, sizeof(ifly_search_desc_t)); 
		
		p_result->startID = ntohs(desc.startID); 
		p_result->endID = ntohs(desc.endID); 
		p_result->sum = ntohs(desc.sum); 
		
		ptmp += sizeof(ifly_search_desc_t); 
		p_result->pH = NULL; 
		
		for (i = 0; i < min(ntohs(searchLog.max_return), p_result->sum); i++)
		{	
			memcpy(&logback, ptmp, sizeof(ifly_logInfo_t));
			
			ptmp += sizeof(ifly_logInfo_t); 
			pi = (struct NETDVR_logInfo_t *) malloc(sizeof(struct NETDVR_logInfo_t)); 
			if (pi==NULL)
			{
				return NETDVR_ERR_OUTOFMEMORY; 
			}
			memset(pi, 0, sizeof(struct NETDVR_logInfo_t));
			memcpy(pi->loginfo, logback.loginfo, sizeof(logback.loginfo));
			memcpy(&pi->main, &logback.main_type, sizeof(logback.main_type));
			memcpy(&pi->slave, &logback.slave_type, sizeof(logback.slave_type));
			pi->startTime = ntohl(logback.startTime);
			
			//csp modify for NVR
			struct timeb tb;
			ftime(&tb);
			pi->startTime -= tb.timezone*60;
			
			pi->pnext = NULL;
			
			// add pi; 
			if (!p_result->pH)
			{
				p_result->pH = pi;
			}
			else 
			{
				qi->pnext = pi; 
			}
			qi = pi;
		}
	}

	return ret;
}

int __stdcall NETDVR_logSearchClean(struct NETDVR_logSearchResult_t *p_result)
{
	if (!p_result)
	{
		return NETDVR_ERR_PARAM;
	}

	struct NETDVR_logInfo_t *pi;
	
	while(p_result->sum > 0)
	{
		pi = p_result->pH; 
		if (pi)
		{
			p_result->pH = pi->pnext; 
			pi->pnext = NULL; 
			free(pi); 
			pi=NULL; 
		}
		p_result->sum--; 
	}
	
	p_result->pH = NULL; 
	p_result->startID = 0; 
	p_result->endID = 0; 
	p_result->sum = 0; 
	
	return NETDVR_SUCCESS; 
}


int __stdcall NETDVR_seachIPCList(int Handle,struct NETDVR_ipcSeachCondition *ipcS,struct NETDVR_ipcSearch *p_para)
{
#if 1

	return CTRL_FAILED_COMMAND;

#else

	struct NETDVR_INNER_t *p;
	int ret = NETDVR_SUCCESS, i , j;
	
	int num = 0 ,num1 = 0;
	char buf[4096]={0};
	ifly_search_desc_t desc; 
	ifly_search_ipc ipcback;
	struct NETDVR_ipcInfo *pi, *qi; 
	NETDVR_ipcSeachCondition searchIpc;

	printf("yg NETDVR_seachIPCList start\n");
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	char *ptmp = NULL;
	if (!ipcS || !p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	printf("yg search protocol_type: 0x%x\n", ipcS->protocol_type);

	searchIpc.protocol_type = htonl(ipcS->protocol_type);
	searchIpc.max_return = htons(ipcS->max_return);
	
	memset(p_para,0,sizeof(NETDVR_ipcSearch));

	if (!p->b_cmdConnectLost)
	{	//搜索时超时时间要设置得长一些
		ret = send_command(p->cph, CTRL_CMD_GETSEACHIPCLIST, &searchIpc, sizeof(NETDVR_ipcSeachCondition), buf, sizeof(buf), g_connect_timeout*10/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	printf("yg NETDVR_seachIPCList start 2 ret: %d\n", ret);
	if (NETDVR_SUCCESS == ret)
	{
		printf("yg NETDVR_seachIPCList start 3\n");
		ptmp = buf + sizeof(ifly_cp_header_t); 
		memcpy(&desc, ptmp, sizeof(ifly_search_desc_t)); 

		p_para->startID = ntohs(desc.startID); 
		p_para->endID = ntohs(desc.endID); 
		p_para->sum = ntohs(desc.sum); 

		ptmp += sizeof(ifly_search_desc_t); 
		p_para->pIPC = NULL; 
		if (p_para->sum > 0)
		{
			num = ( p_para->sum - 1 ) / ipcS->max_return;
			printf("get seachIPCList times = %d\n", num);
			printf("\t ipc startID: %d\n", p_para->startID);
			printf("\t ipc endID: %d\n", p_para->endID);
			printf("\t ipc sum: %d\n", p_para->sum);
		}

		
		for ( j = 0 ;j < num+1;j++)
		{
			if (j>0)
			{
				ret = send_command(p->cph, CTRL_CMD_GETTHEOTHER, &searchIpc, sizeof(NETDVR_ipcSeachCondition), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
				
				ptmp = buf + sizeof(ifly_cp_header_t); 
				memcpy(&desc, ptmp, sizeof(ifly_search_desc_t)); 

				p_para->startID = ntohs(desc.startID); 
				p_para->endID = ntohs(desc.endID); 
				p_para->sum = ntohs(desc.sum); 

				printf("get other: \n");
				printf("\t ipc startID: %d\n", p_para->startID);
				printf("\t ipc endID: %d\n", p_para->endID);
				printf("\t ipc sum: %d\n", p_para->sum);

				ptmp += sizeof(ifly_search_desc_t);
			}
			if(j == num)
			{
				num1 = p_para->sum - num * ipcS->max_return;
			}
			else
			{
				num1 = ipcS->max_return;
			}
			printf("other num1: %d\n", num1);
			for (i = 0; i < num1 ; i++)//p_para->sum
			{	
				memcpy(&ipcback, ptmp, sizeof(ifly_search_ipc));

				ptmp += sizeof(ifly_search_ipc); 
				pi = (struct NETDVR_ipcInfo *) malloc(sizeof(struct NETDVR_ipcInfo)); 
				if (pi==NULL)
				{
					return NETDVR_ERR_OUTOFMEMORY; 
				}
				memset(pi, 0, sizeof(struct NETDVR_logInfo_t));
				memcpy(&pi->channel_no, &ipcback.channel_no, sizeof(ipcback.channel_no)); 
				memcpy(&pi->enable, &ipcback.enable, sizeof(ipcback.enable)); 
				memcpy(&pi->stream_type, &ipcback.stream_type, sizeof(ipcback.stream_type));
				memcpy(&pi->trans_type, &ipcback.trans_type, sizeof(ipcback.trans_type));
				memcpy(&pi->protocol_type, &ipcback.protocol_type, sizeof(ipcback.protocol_type));
				memcpy(&pi->dwIp, &ipcback.dwIp, sizeof(ipcback.dwIp));
				memcpy(&pi->wPort, &ipcback.wPort, sizeof(ipcback.wPort));
				memcpy(&pi->force_fps, &ipcback.force_fps, sizeof(ipcback.force_fps));
				memcpy(&pi->frame_rate, &ipcback.frame_rate, sizeof(ipcback.frame_rate));
				memcpy(pi->user, ipcback.user, sizeof(ipcback.user)); 
				memcpy(pi->pwd, ipcback.pwd, sizeof(ipcback.pwd)); 
				memcpy(pi->name, ipcback.name, sizeof(ipcback.name)); 
				memcpy(pi->uuid, ipcback.uuid, sizeof(ipcback.uuid)); 
				memcpy(pi->address, ipcback.address, sizeof(ipcback.address));
				memcpy(&pi->ipc_type, &ipcback.ipc_type, sizeof(ipcback.ipc_type));
				memcpy(pi->reserved, ipcback.reserved, sizeof(ipcback.reserved));
				memcpy(&pi->net_mask, &ipcback.net_mask, sizeof(ipcback.net_mask));
				memcpy(&pi->net_gateway, &ipcback.net_gateway, sizeof(ipcback.net_gateway));
				memcpy(&pi->dns1, &ipcback.dns1, sizeof(ipcback.dns1));
				memcpy(&pi->dns2, &ipcback.dns2, sizeof(ipcback.dns2));

				pi->pnext = NULL;

				struct in_addr host;
				host.s_addr = pi->dwIp;//ipcback.dwIp;
				printf("%s\n",inet_ntoa(host));
				// add pi; 
				if (!p_para->pIPC)
				{
					p_para->pIPC = pi;
				}
				else 
				{
					qi->pnext = pi; 
				}
				qi = pi;
			}
		}
	}
	return ret;
#endif
}

 
int __stdcall NETDVR_getAddIPCList(int Handle,NETDVR_ipcSeachCondition *ipcS,NETDVR_ipcSearch *p_para)
{
#if 1
	struct NETDVR_INNER_t *p = NULL;
	struct NETDVR_ipcInfo **ppipc = NULL;
	struct NETDVR_ipcInfo *ptmp = NULL;
	ifly_search_ipc *pipc_info = NULL;
	int i = 0;
	
	char buf[4096]={0};
	int ret = NETDVR_SUCCESS;
	ifly_search_desc_t req_desc, ret_desc;
	struct in_addr in;
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	memset(p_para, 0, sizeof(NETDVR_ipcSearch));
	ppipc = &(p_para->pIPC);
	p_para->startID = 1;
	
	memset(&req_desc, 0, sizeof(ifly_search_desc_t));
	memset(&ret_desc, 0, sizeof(ifly_search_desc_t));
	
	if (!p->b_cmdConnectLost)
	{
		do{	
			req_desc.startID = ret_desc.endID+1;//从第一个开始
			//printf("req startID: %d\n", req_desc.startID);
			req_desc.startID = htons(req_desc.startID);
			
			ret = send_command(p->cph, CTRL_CMD_GETADDIPCLIST, &req_desc, sizeof(ifly_search_desc_t), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
			if (NETDVR_SUCCESS != ret)
			{
				printf("ret: %d\n", ret);
				goto fail;
			}

			memcpy(&ret_desc, buf+sizeof(ifly_cp_header_t), sizeof(ifly_search_desc_t));
			ret_desc.startID = ntohs(ret_desc.startID);
			ret_desc.endID = ntohs(ret_desc.endID);
			ret_desc.sum = ntohs(ret_desc.sum);

			printf("ret sum: %d, startID: %d, endID: %d\n", 
						ret_desc.sum, ret_desc.startID, ret_desc.endID);

			pipc_info = (ifly_search_ipc *)(buf + sizeof(ifly_cp_header_t) + sizeof(ifly_search_desc_t));			
			for (i=ret_desc.startID-1; i<ret_desc.endID; ++i, ++pipc_info)
			{
				ptmp = NULL;
				ptmp = (struct NETDVR_ipcInfo *) malloc(sizeof(struct NETDVR_ipcInfo)); 
				if (NULL == ptmp)
				{
					printf("malloc NETDVR_ipcInfo failed\n");
					ret = NETDVR_ERR_OUTOFMEMORY; 
					goto fail;
				}
				memset(ptmp, 0, sizeof(struct NETDVR_ipcInfo));

				ptmp->channel_no = pipc_info->channel_no;
				ptmp->enable = pipc_info->enable;
				ptmp->stream_type = pipc_info->stream_type;
				ptmp->trans_type = pipc_info->trans_type;
				ptmp->protocol_type = pipc_info->protocol_type;
				ptmp->dwIp = pipc_info->dwIp;
				ptmp->wPort = pipc_info->wPort;
				ptmp->force_fps = pipc_info->force_fps;
				ptmp->frame_rate = pipc_info->frame_rate;
				memcpy(ptmp->user, pipc_info->user, sizeof(pipc_info->user)); 
				memcpy(ptmp->pwd, pipc_info->pwd, sizeof(pipc_info->pwd)); 
				memcpy(ptmp->name, pipc_info->name, sizeof(pipc_info->name)); 
				memcpy(ptmp->uuid, pipc_info->uuid, sizeof(pipc_info->uuid)); 
				memcpy(ptmp->address, pipc_info->address, sizeof(pipc_info->address));
				ptmp->ipc_type = pipc_info->ipc_type;
				ptmp->max_nvr_chn = pipc_info->max_nvr_chn;
				ptmp->req_nvr_chn = pipc_info->req_nvr_chn;
				ptmp->net_mask = pipc_info->net_mask;
				ptmp->net_gateway = pipc_info->net_gateway;
				ptmp->dns1 = pipc_info->dns1;
				ptmp->dns2 = pipc_info->dns2;
				ptmp->pnext = NULL;

				*ppipc = ptmp;
				ppipc = &(ptmp->pnext);

				in.s_addr = ptmp->dwIp;
				printf("ipc ip: %s\n", inet_ntoa(in));
			}
			
			
		} while(ret_desc.endID < ret_desc.sum);
	} 
	else
	{
		printf("ConnectLost\n");
		return NETDVR_ERR_SEND;
	}

	p_para->endID = ret_desc.endID;
	p_para->sum = ret_desc.sum;

	return NETDVR_SUCCESS;
	
fail:

	ppipc = &(p_para->pIPC);
	while (*ppipc)
	{
		ptmp = *ppipc;
		*ppipc = ptmp->pnext;
		
		free(ptmp);
	}

	memset(p_para, 0, sizeof(NETDVR_ipcSearch));
	
	return ret;
#else

	struct NETDVR_INNER_t *p;
	int ret = NETDVR_SUCCESS, i ,j;
	struct NETDVR_ipcInfo *addhead; 
	struct NETDVR_ipcInfo *pi, *qi;
	
	int num = 0 ,num1 = 0;
	char buf[4096]={0};
	ifly_search_desc_t desc; 
	ifly_search_ipc ipcback;
	
	NETDVR_ipcSeachCondition searchIpc;

	printf("yg NETDVR_getAddIPCList start\n");
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	char *ptmp = NULL;
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	searchIpc.protocol_type = htonl(ipcS->protocol_type);
	searchIpc.max_return = htons(ipcS->max_return);

	memset(p_para,0,sizeof(NETDVR_ipcSearch));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETADDIPCLIST, &searchIpc, sizeof(NETDVR_ipcSeachCondition), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		ptmp = buf + sizeof(ifly_cp_header_t); 
		memcpy(&desc, ptmp, sizeof(ifly_search_desc_t)); 

		p_para->startID = ntohs(desc.startID); 
		p_para->endID = ntohs(desc.endID); 
		p_para->sum = ntohs(desc.sum); 

		ptmp += sizeof(ifly_search_desc_t); 
		p_para->pIPC = NULL;
		if (p_para->sum > 0)
		{
			num = ( p_para->sum - 1 ) / ipcS->max_return;
			printf("get AddIpcList times = %d\n", num);
			printf("\t ipc startID: %d\n", p_para->startID);
			printf("\t ipc endID: %d\n", p_para->endID);
			printf("\t ipc sum: %d\n", p_para->sum);
		}

		for ( j = 0 ;j < num+1;j++)
		{
			if (j>0)
			{
				ret = send_command(p->cph, CTRL_CMD_GETTHEOTHER, &searchIpc, sizeof(NETDVR_ipcSeachCondition), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );

				ptmp = buf + sizeof(ifly_cp_header_t); 
				memcpy(&desc, ptmp, sizeof(ifly_search_desc_t)); 

				p_para->startID = ntohs(desc.startID); 
				p_para->endID = ntohs(desc.endID); 
				p_para->sum = ntohs(desc.sum); 
				printf("get other: \n");
				printf("\t ipc startID: %d\n", p_para->startID);
				printf("\t ipc endID: %d\n", p_para->endID);
				printf("\t ipc sum: %d\n", p_para->sum);

				ptmp += sizeof(ifly_search_desc_t);
				//printf(" p_para->sum = %d \n",p_para->sum);
			}
			//printf(" p_para->startID = %d ,p_para->endID = %d \n",p_para->startID,p_para->endID);
			if(j == num)
			{
				num1 = p_para->sum - num * ipcS->max_return;
			}
			else
			{
				num1 = ipcS->max_return;
			}
			printf("other num1: %d\n", num1);
			for (i = 0; i < num1 ; i++)//p_para->sum
			{	
				memcpy(&ipcback, ptmp, sizeof(ifly_search_ipc));

				ptmp += sizeof(ifly_search_ipc); 
				pi = (struct NETDVR_ipcInfo *) malloc(sizeof(struct NETDVR_ipcInfo)); 
				if (pi==NULL)
				{
					return NETDVR_ERR_OUTOFMEMORY; 
				}
				memset(pi, 0, sizeof(struct NETDVR_logInfo_t));
				memcpy(&pi->channel_no, &ipcback.channel_no, sizeof(ipcback.channel_no)); 
				memcpy(&pi->enable, &ipcback.enable, sizeof(ipcback.enable)); 
				memcpy(&pi->stream_type, &ipcback.stream_type, sizeof(ipcback.stream_type));
				memcpy(&pi->trans_type, &ipcback.trans_type, sizeof(ipcback.trans_type));
				memcpy(&pi->protocol_type, &ipcback.protocol_type, sizeof(ipcback.protocol_type));
				memcpy(&pi->dwIp, &ipcback.dwIp, sizeof(ipcback.dwIp));
				memcpy(&pi->wPort, &ipcback.wPort, sizeof(ipcback.wPort));
				memcpy(&pi->force_fps, &ipcback.force_fps, sizeof(ipcback.force_fps));
				memcpy(&pi->frame_rate, &ipcback.frame_rate, sizeof(ipcback.frame_rate));
				memcpy(pi->user, ipcback.user, sizeof(ipcback.user)); 
				memcpy(pi->pwd, ipcback.pwd, sizeof(ipcback.pwd)); 
				memcpy(pi->name, ipcback.name, sizeof(ipcback.name)); 
				memcpy(pi->uuid, ipcback.uuid, sizeof(ipcback.uuid)); 
				memcpy(pi->address, ipcback.address, sizeof(ipcback.address));
				memcpy(&pi->ipc_type, &ipcback.ipc_type, sizeof(ipcback.ipc_type));
				memcpy(pi->reserved, ipcback.reserved, sizeof(ipcback.reserved));
				memcpy(&pi->net_mask, &ipcback.net_mask, sizeof(ipcback.net_mask));
				memcpy(&pi->net_gateway, &ipcback.net_gateway, sizeof(ipcback.net_gateway));
				memcpy(&pi->dns1, &ipcback.dns1, sizeof(ipcback.dns1));
				memcpy(&pi->dns2, &ipcback.dns2, sizeof(ipcback.dns2));

				pi->pnext = NULL;

				struct in_addr host;
				host.s_addr = pi->dwIp;//ipcback.dwIp;
				printf(" %s\n",inet_ntoa(host));
				// add pi; 
				if (!p_para->pIPC)
				{
					p_para->pIPC = pi;
				}
				else 
				{
					qi->pnext = pi; 
				}
				qi = pi;
			}
		}
	}
	
	return ret;
#endif
}
//yaogang modify 20141025
int __stdcall NETDVR_destoryIPCList(struct NETDVR_ipcSearch *plist)
{
#if 0

	return CTRL_FAILED_COMMAND;

#else
	
	struct NETDVR_ipcInfo *tmp = NULL;

	if (NULL == plist)
	{
		printf("destoryIPCList param invaild!");
		return 1;
	}
	plist->startID = 0;
	plist->endID = 0;
	plist->sum = 0;

	while(plist->pIPC != NULL)
	{
		tmp = plist->pIPC;
		plist->pIPC = tmp->pnext;
		free(tmp);
	}
	printf("yg destory ipclist\n");
	return 0;
#endif
}

int __stdcall NETDVR_setIPC(int Handle,NETDVR_ipcInfo *pIPC)
{
	int ret = NETDVR_SUCCESS, i;
	struct NETDVR_INNER_t *p;
	char buf[4096] = {0};
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	NETDVR_ipcInfo pIpc;
	memset(&pIpc,0,sizeof(NETDVR_ipcInfo));
	pIPC->dwIp = ntohs(pIPC->dwIp);
	pIPC->protocol_type = ntohl(pIPC->protocol_type);
	pIPC->net_mask = ntohl(pIPC->net_mask);
	pIPC->net_gateway = ntohl(pIPC->net_gateway);
	pIPC->dns1 = ntohl(pIPC->dns1);
	pIPC->dns2 = ntohl(pIPC->dns2);

	memcpy(&pIpc,pIPC,sizeof(NETDVR_ipcInfo));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETIPC , &pIpc, sizeof(NETDVR_ipcInfo), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	return ret;
}

//pIPC->channel_no 取值在[1-16]，则添加到指定通道
//= 0，则添加到最小未使用的通道
int __stdcall NETDVR_addIPC(int Handle,struct NETDVR_ipcInfo *pIPC)
{
	int ret = NETDVR_SUCCESS, i;
	struct NETDVR_INNER_t *p;
	char buf[4096] = {0};
	NETDVR_ipcInfo pIpc;
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	/*
	if (!pIPC || pIPC->channel_no > 16)
	{
		return NETDVR_ERR_PARAM;
	}
	*/

	memset(&pIpc,0,sizeof(NETDVR_ipcInfo));
	memcpy(&pIpc,pIPC,sizeof(NETDVR_ipcInfo));

	struct in_addr host;
	host.s_addr = pIpc.dwIp;//ipcback.dwIp;
	printf("yg addipc %s\n",inet_ntoa(host));
	
	pIpc.protocol_type = htonl(pIpc.protocol_type);
	pIpc.dwIp = htonl(pIpc.dwIp);
	pIpc.wPort = htons(pIpc.wPort);
	pIpc.net_mask = htonl(pIpc.net_mask);
	pIpc.net_gateway = htonl(pIpc.net_gateway);
	pIpc.dns1 = htonl(pIpc.dns1);
	pIpc.dns2 = htonl(pIpc.dns2);
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_ADDIPC , &pIpc, sizeof(NETDVR_ipcInfo), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	return ret;
}

int __stdcall NETDVR_deleteIPC(int Handle,struct NETDVR_ipcInfo *pIPC)
{
	int ret = NETDVR_SUCCESS, i;
	struct NETDVR_INNER_t *p;
	char buf[4096] = {0};
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	/*
	if (!pIPC || pIPC->channel_no > 16)
	{
		return NETDVR_ERR_PARAM;
	}
	*/
	NETDVR_ipcInfo pIpc;
	memset(&pIpc,0,sizeof(NETDVR_ipcInfo));
	memcpy(&pIpc,pIPC,sizeof(NETDVR_ipcInfo));

	struct in_addr host;
	host.s_addr = pIpc.dwIp;//ipcback.dwIp;
	printf("yg del ipc %s\n",inet_ntoa(host));
	
	pIpc.protocol_type = htonl(pIpc.protocol_type);
	pIpc.dwIp = htonl(pIpc.dwIp);
	pIpc.wPort = htons(pIpc.wPort);
	pIpc.net_mask = htonl(pIpc.net_mask);
	pIpc.net_gateway = htonl(pIpc.net_gateway);
	pIpc.dns1 = htonl(pIpc.dns1);
	pIpc.dns2 = htonl(pIpc.dns2);

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_DELETEIPC , &pIpc, sizeof(NETDVR_ipcInfo), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	return ret;
}

//yaogang modify 20141030
int __stdcall NETDVR_GetPatrolPara(int Handle, struct NETDVR_PatrolPara *p_para, int *psize)
{
	struct NETDVR_INNER_t *p;
	int ret = NETDVR_SUCCESS;
	
	char buf[1024]={0};

	printf("yg NETDVR_GetPatrolPara start\n");
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	char *ptmp = NULL;
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	memset(p_para, 0, sizeof(NETDVR_PatrolPara));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GET_PATROL_PARA, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}

	if (NETDVR_SUCCESS == ret)
	{
		ptmp = buf + sizeof(ifly_cp_header_t); 
		memcpy(p_para, ptmp, sizeof(NETDVR_PatrolPara)); 
		
		int num = sizeof(NETDVR_PatrolPara)+p_para->nInterval_num \
				+ p_para->nPatrolMode_num - 1;//nInterval_num+nPatrolMode_num 为value[]数组大小

		//printf("para size: %d, num: %d\n", *psize, num);
		if (num > *psize)
		{
			*psize = num;
			ret =  NETDVR_ERR_OUTOFMEMORY;
		}
		else
		{
			memcpy(p_para, ptmp, num); 
		}
	}

	return ret;
}
int __stdcall NETDVR_SetPatrolPara(int Handle, struct NETDVR_PatrolPara *p_para)
{
	int ret = NETDVR_SUCCESS;
	struct NETDVR_INNER_t *p;
	char buf[1024] = {0};

	printf("yg NETDVR_SetPatrolPara start\n");
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SET_PATROL_PARA , p_para, sizeof(NETDVR_PatrolPara), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	return ret;
}
int __stdcall NETDVR_GetPreviewPara(int Handle, struct NETDVR_PreviewPara *p_para)
{
	struct NETDVR_INNER_t *p;
	int ret = NETDVR_SUCCESS;
	
	char buf[1024]={0};

	//printf("yg NETDVR_GetPreviewPara start\n");
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	char *ptmp = NULL;
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	memset(p_para, 0, sizeof(NETDVR_PreviewPara));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GET_PREVIEW_PARA, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}

	if (NETDVR_SUCCESS == ret)
	{
		ptmp = buf + sizeof(ifly_cp_header_t); 
		memcpy(p_para, ptmp, sizeof(NETDVR_PreviewPara)); 
	}

	return ret;
}
int __stdcall NETDVR_SetPreviewPara(int Handle, struct NETDVR_PreviewPara *p_para)
{
	int ret = NETDVR_SUCCESS;
	struct NETDVR_INNER_t *p;
	char buf[1024] = {0};

	//printf("yg NETDVR_SetPreviewPara start\n");
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SET_PREVIEW_PARA , p_para, sizeof(NETDVR_PreviewPara), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	return ret;
}

//通道属性
int __stdcall NETDVR_GetChnPara(int Handle, struct NETDVR_ChnPara * p_para)
{
	struct NETDVR_INNER_t *p;
	int ret = NETDVR_SUCCESS;
	
	char buf[1024]={0};

	printf("yg NETDVR_GetChnPara start\n");
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	char *ptmp = NULL;
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GET_CHN_PARA, p_para, sizeof(NETDVR_ChnPara), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}

	if (NETDVR_SUCCESS == ret)
	{
		ptmp = buf + sizeof(ifly_cp_header_t); 
		memcpy(p_para, ptmp, sizeof(NETDVR_ChnPara)); 
		
		p_para->para.sChnNamePos.x = ntohl(p_para->para.sChnNamePos.x);
		p_para->para.sChnNamePos.y = ntohl(p_para->para.sChnNamePos.y);

		p_para->para.sEncChnNamePos.x = ntohl(p_para->para.sEncChnNamePos.x);
		p_para->para.sEncChnNamePos.y = ntohl(p_para->para.sEncChnNamePos.y);

		p_para->para.sEncTimePos.x = ntohl(p_para->para.sEncTimePos.x);
		p_para->para.sEncTimePos.y = ntohl(p_para->para.sEncTimePos.y);
		/*
		printf("nShowChnName: %u\n", p_para->para.nShowChnName);
		printf("strChnName: %s\n", p_para->para.strChnName);
		printf("nEncShowTime: %u\n", p_para->para.nEncShowTime);
		*/
	}

	return ret;
}
int __stdcall NETDVR_SetChnPara(int Handle, struct NETDVR_ChnPara *p_para)
{
	int ret = NETDVR_SUCCESS;
	struct NETDVR_INNER_t *p = NULL;
	char buf[1024] = {0};
	struct NETDVR_ChnPara tmp;

	printf("yg NETDVR_SetChnPara start\n");
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		memcpy(&tmp, p_para, sizeof(NETDVR_ChnPara));
		tmp.para.sChnNamePos.x = htonl(p_para->para.sChnNamePos.x);
		tmp.para.sChnNamePos.y = htonl(p_para->para.sChnNamePos.y);

		tmp.para.sEncChnNamePos.x = htonl(p_para->para.sEncChnNamePos.x);
		tmp.para.sEncChnNamePos.y = htonl(p_para->para.sEncChnNamePos.y);

		tmp.para.sEncTimePos.x = htonl(p_para->para.sEncTimePos.x);
		tmp.para.sEncTimePos.y = htonl(p_para->para.sEncTimePos.y);
		ret = send_command(p->cph, CTRL_CMD_SET_CHN_PARA , &tmp, sizeof(NETDVR_ChnPara), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	return ret;
}

//清除桌面报警三角标（注意只有在当前没有报警源存在时，才会清除）
int __stdcall NETDVR_CleanDesktopAlarmIcon(int Handle)
{
	int ret = 0;
	char buf[2048] = {0};

	struct NETDVR_INNER_t *p = NULL;

	printf("yg NETDVR_CleanDesktopAlarmIcon start\n");
	
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_CLEAN_ALARM_ICON, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

//跃天解码板CMS remote CTRL
int __stdcall NETDVR_CLOSE_GUIDE(int Handle)
{
	int ret = 0;
	char buf[2048] = {0};

	struct NETDVR_INNER_t *p = NULL;

	printf("yg NETDVR_CleanDesktopAlarmIcon start\n");
	
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_CLOSE_GUIDE, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

//yaogang modify 20160122
//获取IPC通道连接状态(一个IPC有两个通道，主、子码流)
int __stdcall NETDVR_GetIPCChnLinkStatus(int Handle, struct NETDVR_IPCChnStatus * p_para)
{
	int ret = NETDVR_SUCCESS, i;
	struct NETDVR_INNER_t *p;
	char buf[4096] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	memset(p_para, 0, sizeof(NETDVR_IPCChnStatus));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GET_IPCCHN_LINKSTATUS, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}

	if (NETDVR_SUCCESS == ret)
	{
		char *ptmp = buf + sizeof(ifly_cp_header_t); 
		memcpy(p_para, ptmp, sizeof(NETDVR_IPCChnStatus));
	#if 0
		printf("SDK max_chn_num: %d\n", p_para->max_chn_num);
		for (i=0; i<p_para->max_chn_num/8 +1; i++)
		{
			printf("SDK chn_status[%d]: 0x%x\n", i, p_para->chn_status[i]);
		}
	#endif
	}
	
	return ret;
}



int __stdcall NETDVR_setVideoParams(int Handle, const struct NETDVR_videoParam_t *p_para)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_ImgParam_t image_info;
	memset(&image_info,0,sizeof(ifly_ImgParam_t));
	memcpy(image_info.channelname,p_para->channelname,sizeof(image_info.channelname));
	image_info.channel_no = p_para->channel_no;
	image_info.chnpos_x = htons(p_para->chnpos_x);
	image_info.chnpos_y = htons(p_para->chnpos_y);
	image_info.copy2chnmask = htonl(p_para->copy2chnmask);
	image_info.flag_mask = p_para->flag_mask;
	image_info.flag_name = p_para->flag_name;
	image_info.flag_time = p_para->flag_time;
	image_info.timepos_x = htons(p_para->timepos_x);
	image_info.timepos_y = htons(p_para->timepos_y);
	image_info.flag_safechn = p_para->flag_safechn;
	for (int i = 0; i<4; i++)
	{
		image_info.MaskInfo[i].x = htons(p_para->maskinfo[i].x);
		image_info.MaskInfo[i].y = htons(p_para->maskinfo[i].y);
		image_info.MaskInfo[i].height = htons(p_para->maskinfo[i].height);
		image_info.MaskInfo[i].width = htons(p_para->maskinfo[i].width);
	}
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETIMGPARAM, &image_info, sizeof(ifly_ImgParam_t), buf,sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;

}

int __stdcall NETDVR_getVideoParams(int Handle, unsigned char chn, struct NETDVR_videoParam_t *p_para)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[2048];



	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_ImgParam_t para_info;
	memset(&para_info,0,sizeof(ifly_ImgParam_t));	

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETIMGPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->channel_no = para_info.channel_no;
		memcpy(p_para->channelname,para_info.channelname,sizeof(para_info.channelname));
		p_para->chnpos_x = ntohs(para_info.chnpos_x);
		p_para->chnpos_y = ntohs(para_info.chnpos_y);
		p_para->copy2chnmask = ntohl(para_info.copy2chnmask);
		p_para->flag_mask = para_info.flag_mask;
		p_para->flag_name = para_info.flag_name;
		p_para->flag_safechn = para_info.flag_safechn;
		p_para->flag_time = para_info.flag_time;
		for (int j = 0; j<4; j++)
		{
			p_para->maskinfo[j].x = ntohs(para_info.MaskInfo[j].x);
			p_para->maskinfo[j].y = ntohs(para_info.MaskInfo[j].y);
			p_para->maskinfo[j].height = ntohs(para_info.MaskInfo[j].height);
			p_para->maskinfo[j].width = ntohs(para_info.MaskInfo[j].width);
		}
		p_para->timepos_x = ntohs(para_info.timepos_x);
		p_para->timepos_y = ntohs(para_info.timepos_y);

	}
	
	return ret;
}
//yaogang modify 20170715 简易设置通道名的接口
int __stdcall NETDVR_SetChnName(int Handle, unsigned char chn, const char *pname, int len)
{
	int ret;
	char buf[1024] = {0};
	struct NETDVR_INNER_t *p;
	ifly_ImgParam_t para_info;
	memset(&para_info, 0, sizeof(ifly_ImgParam_t));
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!pname)
	{
		return NETDVR_ERR_PARAM;
	}
	
	if (len <= 0 || len+1 > sizeof(para_info.channelname))
	{
		return NETDVR_ERR_PARAM;
	}
	
	para_info.channel_no = chn;
	memcpy(para_info.channelname, pname, len+1);	
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SET_CHN_NAME, &para_info, sizeof(ifly_ImgParam_t), buf,sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}	
	
	return ret;
}
int __stdcall NETDVR_GetChnName(int Handle, unsigned char chn, char *pname, int size)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[1024] = {0};
	ifly_ImgParam_t para_info;
	memset(&para_info,0,sizeof(ifly_ImgParam_t));

	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	
	if (!pname)
	{
		return NETDVR_ERR_PARAM;
	}	

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GET_CHN_NAME, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));

		if (size < sizeof(para_info.channelname))
		{
			return NETDVR_ERR_PARAM;
		}
		strcpy(pname, para_info.channelname);
	}
	
	return ret;
}

int __stdcall NETDVR_setRecordParams(int Handle, const struct NETDVR_recordParam_t *p_para)
{

	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_RecordParam_t para_info;
	memset(&para_info,0,sizeof(ifly_RecordParam_t));
	para_info.channelno = p_para->channelno;
	para_info.bit_max = htonl(p_para->bit_max);
	para_info.bit_type = p_para->bit_type;
	para_info.code_type = p_para->code_type;
	para_info.frame_rate = p_para->frame_rate;
	para_info.post_record = htons(p_para->post_record);
	para_info.pre_record = htons(p_para->pre_record);
	para_info.quality = p_para->quality;
	para_info.copy2chnmask = htonl(p_para->copy2chnmask);
	para_info.intraRate = htons(p_para->intraRate);
	para_info.maxQ = p_para->maxQ;
	para_info.minQ = p_para->minQ;
	para_info.qi = p_para->qi;
	para_info.supportdeinter = p_para->supportdeinter;			
	para_info.deinterval = p_para->deinterval;				
	para_info.supportResolu = p_para->supportResolu;		
	para_info.resolutionpos = p_para->resolutionpos;
	memcpy(para_info.reserved,p_para->reserved1,sizeof(p_para->reserved1));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETRECPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;

}

int __stdcall NETDVR_getRecordParams(int Handle, unsigned char chn, struct NETDVR_recordParam_t *p_para)
{

	int ret;
	struct NETDVR_INNER_t *p;
	char buf[2048];


	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_RecordParam_t para_info;
	memset(&para_info,0,sizeof(ifly_RecordParam_t));
				
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETRECPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->bit_max = (enum NETDVR_BITRATE)ntohl(para_info.bit_max);
		p_para->bit_type = (enum NETDVR_BITRATE_TYPE)para_info.bit_type;
		p_para->channelno = para_info.channelno;
		p_para->code_type = (enum NETDVR_FLOW_TYPE)para_info.code_type;
		p_para->copy2chnmask = ntohl(para_info.copy2chnmask);
		p_para->frame_rate = (enum NETDVR_FRAMERATE)para_info.frame_rate;
		p_para->post_record = (enum NETDVR_POSTRECORD_TIME)ntohs(para_info.post_record);
		p_para->pre_record = (enum NETDVR_PRERECORD_TIME)ntohs(para_info.pre_record);
		p_para->quality = (enum NETDVR_VIDEO_QUALITY)para_info.quality;
		p_para->intraRate = ntohs(para_info.intraRate);
		p_para->maxQ = para_info.maxQ;
		p_para->minQ = para_info.minQ;
		p_para->qi = para_info.qi;
		p_para->supportdeinter = para_info.supportdeinter;			
		p_para->deinterval = para_info.deinterval;				
		p_para->supportResolu = para_info.supportResolu;		
		p_para->resolutionpos = para_info.resolutionpos;
		memcpy(p_para->reserved1,para_info.reserved,sizeof(para_info.reserved));
	}
	return ret;

}


int __stdcall NETDVR_setAlarmInParams(int Handle, const struct NETDVR_alarmInParam_t *p_para)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_AlarmInParam_t aipt;
	memset(&aipt,0,sizeof(ifly_AlarmInParam_t));
	aipt.inid = p_para->inid;
	for (int i = 0; i<32;i++)
	{
		aipt.AlarmInPtz[i].cruise = p_para->alarmptz[i].cruise;
		aipt.AlarmInPtz[i].flag_cruise = p_para->alarmptz[i].flag_cruise;
		aipt.AlarmInPtz[i].flag_preset = p_para->alarmptz[i].flag_preset;
		aipt.AlarmInPtz[i].flag_track = p_para->alarmptz[i].flag_track;
		aipt.AlarmInPtz[i].preset = p_para->alarmptz[i].preset;
	}
	aipt.copy2AlarmInmask = htonl(p_para->copy2AlarmInmask);
	aipt.delay = htons(p_para->delay);
	aipt.flag_buzz = p_para->flag_buzz;
	aipt.flag_deal = p_para->flag_deal;
	aipt.flag_email = p_para->flag_email;
	aipt.flag_mobile = p_para->flag_mobile;
	aipt.triAlarmoutid = htonl(p_para->triAlarmoutid);
	aipt.triRecChn = htonl(p_para->triRecChn);
	aipt.typein = p_para->typein;
	aipt.flag_enablealarm = p_para->flag_enablealarm;
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETALARMINPARAM, &aipt, sizeof(ifly_AlarmInParam_t), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_getAlarmInParams(int Handle, unsigned char in_id, struct NETDVR_alarmInParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};

	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (in_id >= p->si.maxAlarmInNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_AlarmInParam_t para_info;
	memset(&para_info,0,sizeof(ifly_AlarmInParam_t));	

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETALARMINPARAM, &in_id, sizeof(in_id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		for (int i = 0; i<32;i++)
		{
			p_para->alarmptz[i].cruise = para_info.AlarmInPtz[i].cruise;
			p_para->alarmptz[i].flag_cruise = para_info.AlarmInPtz[i].flag_cruise;
			p_para->alarmptz[i].flag_preset = para_info.AlarmInPtz[i].flag_preset;
			p_para->alarmptz[i].flag_track = para_info.AlarmInPtz[i].flag_track;
			p_para->alarmptz[i].preset = para_info.AlarmInPtz[i].preset;
		}
		p_para->copy2AlarmInmask = ntohl(para_info.copy2AlarmInmask);
		p_para->delay = (enum NETDVR_DELAY_TIME)(ntohs(para_info.delay));
		p_para->flag_buzz = para_info.flag_buzz;
		p_para->flag_deal = para_info.flag_deal;
		p_para->flag_email = para_info.flag_email;
		p_para->flag_mobile = para_info.flag_mobile;
		p_para->inid = para_info.inid;
		p_para->triAlarmoutid = ntohl(para_info.triAlarmoutid);
		p_para->triRecChn = ntohl(para_info.triRecChn);
		p_para->typein = (enum NETDVR_ALARMINTYPE)para_info.typein;
		p_para->flag_enablealarm = para_info.flag_enablealarm;
	}
	return ret;
}

int __stdcall NETDVR_setAlarmOutParams(int Handle, const struct NETDVR_alarmOutParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_AlarmOutParam_t para_info;
	memset(&para_info,0,sizeof(ifly_AlarmOutParam_t));
	
	para_info.outid = p_para->outid;
	para_info.copy2AlarmOutmask = htonl(p_para->copy2AlarmOutmask);
	para_info.typeout = p_para->typeout;
	
	//csp modify
	para_info.alarmoutdelay = htons((u16)p_para->alarmoutdelay);
	para_info.flag_buzz = p_para->flag_buzz;
	para_info.buzzdelay = htons((u16)p_para->buzzdelay);
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETALARMOUTPARAM, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_getAlarmOutParams(int Handle, u8 out_id, struct NETDVR_alarmOutParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (out_id >= p->si.maxAlarmOutNum)
	{
		return NETDVR_ERR_PARAM;
	}
	ifly_AlarmOutParam_t para_info;
	memset(&para_info,0,sizeof(ifly_AlarmOutParam_t));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETALARMOUTPARAM, &out_id, sizeof(out_id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->copy2AlarmOutmask = ntohl(para_info.copy2AlarmOutmask);
		p_para->outid = para_info.outid;
		p_para->typeout = (enum NETDVR_ALARMOUTYPE)para_info.typeout;
		
		//csp modify
		p_para->alarmoutdelay = (delay_t)ntohs(para_info.alarmoutdelay);
		p_para->flag_buzz = para_info.flag_buzz;
		p_para->buzzdelay = (delay_t)ntohs(para_info.buzzdelay);
		
		//char szTmp[32];
		//sprintf(szTmp,"NETDVR_getAlarmOutParams-alarmoutdelay=(%d,%d)\n",p_para->alarmoutdelay,para_info.alarmoutdelay);
		//MessageBox(NULL,szTmp,NULL,MB_OK);
	}
	
	return ret;
}

int __stdcall NETDVR_clearAlarms(int Handle)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	char buf[2048] = {0};

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_CLEARALARM,NULL,0,buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;

}

int __stdcall NETDVR_getNetworkParams(int Handle, struct NETDVR_networkParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETNETWORK, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_NetWork_t para_info;
		memset(&para_info,0,sizeof(ifly_NetWork_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_NetWork_t));
		p_para->centerhost_address = para_info.centerhost_address;
		p_para->centerhost_port = ntohs(para_info.centerhost_port);
		memcpy(p_para->ddns_domain,para_info.ddns_domain,sizeof(para_info.ddns_domain));
		memcpy(p_para->ddns_passwd,para_info.ddns_passwd,sizeof(para_info.ddns_passwd));
		memcpy(p_para->ddns_user,para_info.ddns_user,sizeof(para_info.ddns_user));
		p_para->dns = para_info.dns;
		p_para->flag_ddns = para_info.flag_ddns;
		p_para->flag_dhcp = para_info.flag_dhcp;
		p_para->flag_multicast = para_info.flag_multicast;
		p_para->flag_pppoe = para_info.flag_pppoe;
		p_para->http_port = ntohs(para_info.http_port);
		p_para->ip_address = para_info.ip_address;
		memcpy(p_para->mac_address,para_info.mac_address,sizeof(para_info.mac_address));
		p_para->multicast_address = para_info.multicast_address;
		p_para->net_gateway = para_info.net_gateway;
		p_para->net_mask = para_info.net_mask;
		memcpy(p_para->pppoe_passwd,para_info.pppoe_passwd,sizeof(para_info.pppoe_passwd));
		memcpy(p_para->pppoe_user_name,para_info.pppoe_user_name,sizeof(para_info.pppoe_user_name));
		p_para->server_port = ntohs(para_info.server_port);
		p_para->ddnsserver = para_info.ddnsserver;
		p_para->mobile_port = ntohs(para_info.mobile_port);
		memcpy(p_para->hwid,para_info.hwid,sizeof(para_info.hwid));
	}
	return ret;

}

int __stdcall NETDVR_setNetworkParams(int Handle, const struct NETDVR_networkParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	ifly_NetWork_t para_info;
	memset(&para_info,0,sizeof(ifly_NetWork_t));

	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	para_info.centerhost_address = p_para->centerhost_address;
	para_info.centerhost_port = htons(p_para->centerhost_port);
	memcpy(para_info.ddns_domain,p_para->ddns_domain,sizeof(p_para->ddns_domain));
	memcpy(para_info.ddns_passwd,p_para->ddns_passwd,sizeof(p_para->ddns_passwd));
	memcpy(para_info.ddns_user,p_para->ddns_user,sizeof(p_para->ddns_user));
	para_info.dns = (p_para->dns);
	para_info.flag_ddns = p_para->flag_ddns;
	para_info.flag_dhcp = p_para->flag_dhcp;
	para_info.flag_multicast = p_para->flag_multicast;
	para_info.flag_pppoe = p_para->flag_pppoe;
	para_info.http_port = htons(p_para->http_port);
	para_info.ip_address = p_para->ip_address;
	memcpy(para_info.mac_address,p_para->mac_address,sizeof(p_para->mac_address));
	para_info.multicast_address = (p_para->multicast_address);
	para_info.net_gateway = (p_para->net_gateway);
	para_info.net_mask = (p_para->net_mask);
	memcpy(para_info.pppoe_passwd,p_para->pppoe_passwd,sizeof(p_para->pppoe_passwd));
	memcpy(para_info.pppoe_user_name,p_para->pppoe_user_name,sizeof(p_para->pppoe_user_name));
	para_info.server_port = htons(p_para->server_port);
	para_info.ddnsserver = p_para->ddnsserver;
	para_info.mobile_port = htons(p_para->mobile_port);
	memcpy(para_info.hwid,p_para->hwid,sizeof(p_para->hwid));
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETNETWORK, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_recFilesSearch(int Handle, const struct NETDVR_fileSearchCondition_t *prfs, struct NETDVR_recFileSearchResult_t *pdesc)
{
	struct NETDVR_INNER_t *p;
	int ret = NETDVR_SUCCESS;
	char buf[4096] = {0};
	struct NETDVR_recFileInfo_t *pi = NULL, *qi=NULL; 
	ifly_recsearch_param_t searchpara;
	memset(&searchpara,0,sizeof(ifly_recsearch_param_t));
	ifly_recfileinfo_t fileinfo;
	memset(&fileinfo,0,sizeof(ifly_recfileinfo_t));
	ifly_search_desc_t searchdesc;
	memset(&searchdesc,0,sizeof(ifly_search_desc_t));
	int i = 0;
	char *ptmp = NULL;
	
	if (!prfs || !pdesc)
	{
		return NETDVR_ERR_PARAM;
	}
	
	if ((prfs->startID < 1) || (prfs->max_return > 24/*2000*/))
	{
		return NETDVR_ERR_PARAM;
	}
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	searchpara.channel_mask = ntohs(prfs->chn);
	searchpara.chn17to32mask = ntohs(prfs->chn17to32);
	searchpara.end_time = ntohl(prfs->end_time);
	searchpara.max_return = ntohs(prfs->max_return);
	searchpara.start_time = ntohl(prfs->start_time);
	searchpara.startID = ntohs(prfs->startID);
	searchpara.type_mask = ntohs(prfs->type);
	memcpy(searchpara.bankcardno, prfs->bankcardno, sizeof(prfs->bankcardno));
	
	//csp modify for NVR
	struct timeb tb;
	ftime(&tb);
	searchpara.start_time = htonl(prfs->start_time+tb.timezone*60);//上层应用之前-tb.timezone*60
	searchpara.end_time = htonl(prfs->end_time+tb.timezone*60);
	
	memset(pdesc, 0, sizeof(NETDVR_recFileSearchResult_t));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_RECFILESEARCH, &searchpara, sizeof(ifly_recsearch_param_t), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	}
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		ptmp = buf+sizeof(ifly_cp_header_t);
		memcpy(&searchdesc,ptmp,sizeof(ifly_search_desc_t));
		pdesc->endID = ntohs(searchdesc.endID);
		pdesc->startID = ntohs(searchdesc.startID);
		pdesc->sum = ntohs(searchdesc.sum);
		pdesc->precInfo = NULL;
		ptmp += sizeof(ifly_search_desc_t);
		
		for (i = 0; i < min(ntohs(searchpara.max_return), pdesc->sum); i++)
		{
			memcpy(&fileinfo,ptmp,sizeof(ifly_recfileinfo_t));
			ptmp += sizeof(ifly_recfileinfo_t);
			pi = (NETDVR_recFileInfo_t *)malloc(sizeof(NETDVR_recFileInfo_t));
			if (pi==NULL)
			{
				return NETDVR_ERR_OUTOFMEMORY;
			}
			memset(pi, 0, sizeof(NETDVR_recFileInfo_t));
			pi->channel_no = fileinfo.channel_no;
			pi->end_time = ntohl(fileinfo.end_time);
			pi->image_format = fileinfo.image_format;
			pi->offset = ntohl(fileinfo.offset);
			pi->size = ntohl(fileinfo.size);
			pi->start_time = ntohl(fileinfo.start_time);
			pi->stream_flag	= fileinfo.stream_flag;
			pi->type = fileinfo.type;
			strcpy(pi->filename, fileinfo.filename);
			
			//csp modify for NVR
			struct timeb tb;
			ftime(&tb);
			pi->start_time -= tb.timezone*60;
			pi->end_time -= tb.timezone*60;
			
			pi->pnext = NULL;
			
			if (!pdesc->precInfo)
			{
				pdesc->precInfo = pi;
			}
			else
			{
				qi->pnext = pi;
			}
			qi = pi;
		}
	}
	
	return ret;
}

int __stdcall NETDVR_recFilesSearchClean(struct NETDVR_recFileSearchResult_t *pdesc)
{
	struct NETDVR_recFileInfo_t *pi;
	
	while(pdesc->sum > 0)
	{
		pi = pdesc->precInfo; 
		if (pi)
		{
			pdesc->precInfo = pi->pnext; 
			pi->pnext = NULL; 
			free(pi); 
			pi=NULL; 
		}
		pdesc->sum--; 
	}
	
	pdesc->precInfo = NULL; 
	pdesc->startID = 0; 
	pdesc->endID = 0; 
	pdesc->sum = 0; 
	
	return NETDVR_SUCCESS; 
}



int __stdcall NETDVR_regCBFileRecieverProgress(int Handle, PFUN_PROGRESS_T p_cb_progress, unsigned int dwContent)
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(p->file_reciever.reciever_lock);

	p->file_reciever.p_cb_progress = p_cb_progress;
	p->file_reciever.dwContentProgress = dwContent;
	
	UnlockMutex(p->file_reciever.reciever_lock);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);

	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_regCBFileRecieverError(int Handle, PFUN_ERROR_T p_cb_err, unsigned int dwContent)
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(p->file_reciever.reciever_lock);
	
	p->file_reciever.p_cb_err = p_cb_err;
	p->file_reciever.dwContentErr = dwContent;
	
	UnlockMutex(p->file_reciever.reciever_lock);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}


int __stdcall NETDVR_startFileDownload(int Handle, char *s_save_dir, char *s_save_filename, /*unsigned int rcv_ip, unsigned short rcv_port,*/ const struct NETDVR_recFileInfo_t *pFileInfo)
{
	struct NETDVR_INNER_t *p;
	int ret = 0;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	
	if (p->file_reciever.bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_BUSY;
	}

	LockMutex(p->file_reciever.reciever_lock);

	memset(p->file_reciever.save_path,0,sizeof(p->file_reciever.save_path));
	memset(p->file_reciever.save_filename,0,sizeof(p->file_reciever.save_filename));
	
	ret = set_out_path(p->file_reciever.save_path, s_save_dir);
	if (ret != NETDVR_SUCCESS)
	{
		UnlockMutex(p->file_reciever.reciever_lock);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return ret;
	}
	
	if (s_save_filename)
	{
#ifdef UNICODE
		UINT tmp_len=wcslen(s_save_filename);
#else
		UINT tmp_len=strlen(s_save_filename);
#endif
		if (tmp_len >= FILENAME_LEN_MAX || 0 == tmp_len)
		{
			UnlockMutex(p->file_reciever.reciever_lock);
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_PARAM;
		}
		
		strcpy(p->file_reciever.save_filename, s_save_filename);
	}
	else
	{
		UnlockMutex(p->file_reciever.reciever_lock);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}
	
	p->file_reciever.rcv_size = pFileInfo->size;
	p->file_reciever.recieved_size = 0;
	
	p->file_reciever.req.command = 3;
	strcpy(p->file_reciever.req.FileDownLoad_t.filename, pFileInfo->filename);
	p->file_reciever.req.FileDownLoad_t.offset = htonl(pFileInfo->offset);
	p->file_reciever.req.FileDownLoad_t.size = htonl(pFileInfo->size);
	
	p->file_reciever.p_audio_property = &p->audio_property;
	
	if (SetRcvTcpFrame(p, &p->file_reciever.prcv_t, p->file_reciever.req,TRUE))
	{
		p->file_reciever.reciever_handle = CreateThread(NULL, 0, DownLoadThread, &p->file_reciever, 0 , NULL);
		if (!p->file_reciever.reciever_handle)
		{
			ret = NETDVR_ERR_OUTOFMEMORY;
		}
		else
		{
			p->file_reciever.bOpened = 1;
			ret = NETDVR_SUCCESS;
		}
	} 
	else
	{
		ret = NETDVR_ERR_CONNECT;
	}

	UnlockMutex(p->file_reciever.reciever_lock);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	return ret;
}



int __stdcall NETDVR_stopFileDownload(int Handle)
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p->file_reciever.bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}

	LockMutex(p->file_reciever.reciever_lock);

	if (p->file_reciever.prcv_t)
	{
		char buf[2048] = {0};
		u32 id = htonl(p->file_reciever.prcv_t->linkid);
		if (!p->b_cmdConnectLost)
		{
			send_command(p->cph, CTRL_CMD_STOPDOWNLOAD, &id, sizeof(id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
		} 
		else
		{
			int ret = NETDVR_ERR_SEND;
		}
		
	}
	p->file_reciever.rcv_size = 0;
	p->file_reciever.recieved_size = 0;

	SetRcvTcpFrame(p, &p->file_reciever.prcv_t, p->file_reciever.req, FALSE);
	if (p->file_reciever.reciever_handle)
	{
		WaitForSingleObject(p->file_reciever.reciever_handle, INFINITE);
		CloseHandle(p->file_reciever.reciever_handle);
		p->file_reciever.reciever_handle = NULL;
	}
	p->file_reciever.bOpened = 0;
	UnlockMutex(p->file_reciever.reciever_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);

	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_regCBUpdateProgress(int Handle, PFUN_PROGRESS_T p_cb_func, unsigned int dwContent)
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(p->update.update_lock);

	p->update.p_cb_progress = p_cb_func;
	p->update.dwContentUpdate = dwContent;

	UnlockMutex(p->update.update_lock);

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_regCBUpdateError( int Handle, PFUN_ERROR_T p_cb_err, unsigned int dwContent )
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(p->update.update_lock);
	
	p->update.p_cb_err = p_cb_err;
	p->update.dwContentErr = dwContent;
	
	UnlockMutex(p->update.update_lock);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_startUpdate(int Handle, const struct NETDVR_updateParam_t *p_update_para, /*unsigned short update_port, */char *s_update_dir, char *s_update_filename)
{
	struct NETDVR_INNER_t *p;
	int ret;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);

	UnlockMutex(g_pool_lock);
	
	LockMutex(p->update.update_lock);

	if (p->update.b_updating)
	{
		UnlockMutex(p->update.update_lock);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERROR;
	}
	
	memset(p->update.update_path,0,sizeof(p->update.update_path));
	memset(p->update.update_filename,0,sizeof(p->update.update_filename));
	ret = set_out_path(p->update.update_path, s_update_dir);
	if (NETDVR_SUCCESS != ret)
	{
		UnlockMutex(p->update.update_lock);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERROR;
	}

	if (s_update_filename)
	{
#ifdef UNICODE
		UINT tmp_len=wcslen(s_update_filename);
#else
		UINT tmp_len=strlen(s_update_filename);
#endif
		if (tmp_len >= FILENAME_LEN_MAX || 0 == tmp_len)
		{
			UnlockMutex(p->update.update_lock);
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_PARAM;
		}
		
		strcpy(p->update.update_filename, s_update_filename);
	}
	else
	{
		UnlockMutex(p->update.update_lock);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}

	p->update.b_updating = 1;
	p->update.status = NETDVR_UPDATE_RUN;
	p->update.file_size = p_update_para->size;

	p->update.req.command = 4;
	p->update.req.Update_t.size = htonl(p_update_para->size);
	p->update.req.Update_t.verify = htonl(p_update_para->verify);
	p->update.req.Update_t.updateType = htons(p_update_para->reserved);
	p->update.req.Update_t.version = htons(p_update_para->version);

	ret = NETDVR_ERR_CONNECT;
	unsigned int errcode = 0;
	if (SetRcvTcpFrame(p,&p->update.prcv_t,p->update.req, TRUE, NULL, &errcode))
	{
		if (!p->update.update_handle)
		{
			p->update.update_handle = CreateThread(NULL, 0, UpdateThread, (LPVOID)&p->update, 0, NULL);
			if (!p->update.update_handle)
			{
				p->update.b_updating = 0;
				p->update.status = NETDVR_UPDATE_STOPPED;
				ret = NETDVR_ERR_OUTOFMEMORY;
			}
			else
			{
				ret = NETDVR_SUCCESS;
			}
		}				
	}
	if (CTRL_FAILED_CONFLICT == errcode)
	{
		p->update.b_updating = 0;
		ret = NETDVR_ERR_BUSY;		
	}
	
	UnlockMutex(p->update.update_lock);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret;
}

int __stdcall NETDVR_stopUpdate(int Handle)
{
	//char szTmp[32];
	//sprintf(szTmp, "NETDVR_stopUpdate\n");
	//MessageBox(NULL,szTmp,NULL,MB_OK);
	
	struct NETDVR_INNER_t *p;
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (p->update.update_handle)
	{
		if (p->update.fp)
		{
			fflush(p->update.fp);
			fclose(p->update.fp);
			p->update.fp = NULL;
		}
		SetRcvTcpFrame(p,&p->update.prcv_t,p->update.req, FALSE);
		WaitForSingleObject(p->update.update_handle, 1000);
		CloseHandle(p->update.update_handle);
		p->update.update_handle = NULL;
		p->update.b_updating = 0;
		p->update.status = NETDVR_UPDATE_STOPPED; 
	}
	
	return NETDVR_SUCCESS; 
}

int __stdcall NETDVR_restoreFactorySettings(int Handle)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_SETRESTORE,NULL,0,buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
		if (ret == NETDVR_SUCCESS || ret == NETDVR_ERR_SEND)
		{
			p->b_cmdConnectLost = TRUE;
		} 
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;

}

int __stdcall NETDVR_reboot(int Handle)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_REBOOT,NULL,0,NULL,0,g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
		if (ret == NETDVR_SUCCESS || ret == NETDVR_ERR_SEND)
		{
			p->b_cmdConnectLost = TRUE;
		} 
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_shutdown(int Handle)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_SHUTDOWN,NULL,0,NULL,0,g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
		if (ret == NETDVR_SUCCESS || ret == NETDVR_ERR_SEND)
		{
			p->b_cmdConnectLost = TRUE;
		} 
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_getSystemTime(int Handle, struct NETDVR_SysTime_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_sysTime_t para_info;
	memset(&para_info,0,sizeof(para_info));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_GETSYSTIME,NULL,0,buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	}
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_sysTime_t));
		p_para->flag_time = para_info.flag_time;
		p_para->format = (enum NETDVR_TimeFormat_T)para_info.format;
		p_para->systemtime = ntohl(para_info.systemtime);
		p_para->timepos_x = ntohs(para_info.timepos_x);
		p_para->timepos_y = ntohs(para_info.timepos_y);
		
		//csp modify for NVR
		struct timeb tb;
		ftime(&tb);
		p_para->systemtime -= tb.timezone*60;
	}
	
	return ret;
}

int __stdcall NETDVR_setSystemTime(int Handle, const struct NETDVR_SysTime_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	//csp modify for NVR
	struct timeb tb;
	ftime(&tb);
	
	ifly_sysTime_t para_info;
	memset(&para_info,0,sizeof(para_info));
	para_info.flag_time = p_para->flag_time;
	para_info.format = p_para->format;
	para_info.systemtime = htonl(p_para->systemtime + tb.timezone*60);//csp modify for NVR
	para_info.timepos_x = htons(p_para->timepos_x);
	para_info.timepos_y = htons(p_para->timepos_y);
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_SETSYSTIME,&para_info,sizeof(para_info),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_getMotionDection(int Handle, unsigned char chn, struct NETDVR_motionDetection_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};

	
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_MDParam_t para_info;
	memset(&para_info,0,sizeof(ifly_MDParam_t));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETMDPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(p_para->block,para_info.block,sizeof(para_info.block));
		p_para->chn = para_info.chn;
		p_para->delay = (enum NETDVR_DELAY_TIME)ntohs(para_info.delay);
		p_para->flag_buzz = para_info.flag_buzz;
		p_para->flag_email = para_info.flag_email;
		p_para->flag_mobile = para_info.flag_mobile;
		p_para->sense = para_info.sense;
		p_para->trigAlarmOut = ntohl(para_info.trigAlarmOut);
		p_para->trigRecChn = ntohl(para_info.trigRecChn);
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);

	}
	
	return ret;

}

int __stdcall NETDVR_setMotionDection(int Handle, const struct NETDVR_motionDetection_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_MDParam_t para_info;
	memset(&para_info,0,sizeof(ifly_MDParam_t));
	para_info.chn = p_para->chn;
	memcpy(para_info.block,p_para->block,sizeof(p_para->block));
	para_info.delay = htons(p_para->delay);
	para_info.flag_buzz = p_para->flag_buzz;
	para_info.flag_email = p_para->flag_email;
	para_info.flag_mobile = p_para->flag_mobile;
	para_info.sense = p_para->sense;
	para_info.trigAlarmOut = htonl(p_para->trigAlarmOut);
	para_info.trigRecChn = htonl(p_para->trigRecChn);
	para_info.copy2Chnmask = htonl(p_para->copy2Chnmask);

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETMDPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;

}

int __stdcall NETDVR_getVideoLost(int Handle, unsigned char chn, struct NETDVR_VideoLostParam_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};


	struct NETDVR_INNER_t *p;
	ifly_VideoLostParam_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	ifly_cp_header_t cphead;
	memset(&cphead, 0, sizeof(cphead));

	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
		
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETVIDEOLOSTPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->chn = para_info.chn;
		p_para->delay = (enum NETDVR_DELAY_TIME)ntohs(para_info.delay);
		p_para->flag_buzz = para_info.flag_buzz;
		p_para->flag_email = para_info.flag_email;
		p_para->flag_mobile = para_info.flag_mobile;
		p_para->trigAlarmOut = ntohl(para_info.trigAlarmOut);
		p_para->trigRecChn = ntohl(para_info.trigRecChn);
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);

	}


	return ret;

}

int __stdcall NETDVR_setVideoLost(int Handle, const struct NETDVR_VideoLostParam_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	ifly_VideoLostParam_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	struct NETDVR_INNER_t *p;

	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	para_info.chn = p_para->chn;
	para_info.delay = htons(p_para->delay);
	para_info.flag_buzz = p_para->flag_buzz;
	para_info.flag_email = p_para->flag_email;
	para_info.flag_mobile = p_para->flag_mobile;
	para_info.trigAlarmOut = htonl(p_para->trigAlarmOut);
	para_info.trigRecChn = htonl(p_para->trigRecChn);
	para_info.copy2Chnmask = htonl(p_para->copy2Chnmask);

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETVIDEOLOSTPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	

	return ret;

}

int __stdcall NETDVR_GetVideoBlockParam(int Handle, unsigned char chn, struct NETDVR_VideoBlockParam_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};

	
	struct NETDVR_INNER_t *p;
	ifly_VideoBlockParam_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	ifly_cp_header_t cphead;
	memset(&cphead, 0, sizeof(cphead));
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETVIDEOBLOCKPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->chn = para_info.chn;
		p_para->delay = (enum NETDVR_DELAY_TIME)ntohs(para_info.delay);
		p_para->flag_buzz = para_info.flag_buzz;
		p_para->flag_email = para_info.flag_email;
		p_para->flag_mobile = para_info.flag_mobile;
		p_para->trigAlarmOut = ntohl(para_info.trigAlarmOut);
		p_para->trigRecChn = ntohl(para_info.trigRecChn);
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);

	}
	return ret;
}

int __stdcall NETDVR_SetVideoBlockParam(int Handle, const struct NETDVR_VideoBlockParam_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	ifly_VideoBlockParam_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	struct NETDVR_INNER_t *p;
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	para_info.chn = p_para->chn;
	para_info.delay = htons(p_para->delay);
	para_info.flag_buzz = p_para->flag_buzz;
	para_info.flag_email = p_para->flag_email;
	para_info.flag_mobile = p_para->flag_mobile;
	para_info.trigAlarmOut = htonl(p_para->trigAlarmOut);
	para_info.trigRecChn = htonl(p_para->trigRecChn);
	para_info.copy2Chnmask = htonl(p_para->copy2Chnmask);
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETVIDEOBLOCKPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;		
}



int __stdcall NETDVR_GetUserInfo(int Handle, struct NETDVR_UserNumber_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	int para_len = 0;
	int i = 0;
	int chnnumber = 0;
	ifly_cp_header_t cphead;

	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	ifly_userNumber_t para_info;
	memset(&para_info,0,sizeof(ifly_userNumber_t));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_GETUSERINFO,NULL,0,buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&cphead,buf,sizeof(ifly_cp_header_t));
		para_len = ntohl(cphead.length) - sizeof(ifly_cp_header_t);
		if (para_len <= 0)
		{
			return NETDVR_ERROR;
		}
		if (0 == para_len%sizeof(ifly_user_t))
		{
			chnnumber = para_len/sizeof(ifly_user_t);
			for (i = 0;i<chnnumber;i++)
			{
				memcpy(&para_info.userNum[i],&buf[sizeof(ifly_cp_header_t) + i * sizeof(ifly_user_t)],sizeof(ifly_user_t));
				memcpy(&p_para->userinfo[i],&para_info.userNum[i],sizeof(ifly_user_t));
			}
		}
	}

	return ret;
}

int __stdcall NETDVR_AddUserInfo(int Handle, const struct NETDVR_userInfo_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;

	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_userMgr_t para_info;
	memset(&para_info,0,sizeof(ifly_userMgr_t));

	para_info.flagOption = 0;
	memcpy(&para_info.userInfo,p_para,sizeof(NETDVR_userInfo_t));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_SETUSERINFO,&para_info,sizeof(ifly_userMgr_t),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;
}

int __stdcall NETDVR_EditUserInfo(int Handle, const struct NETDVR_userInfo_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_userMgr_t para_info;
	memset(&para_info,0,sizeof(ifly_userMgr_t));

	para_info.flagOption = 1;
	memcpy(&para_info.userInfo,p_para,sizeof(NETDVR_userInfo_t));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_SETUSERINFO,&para_info,sizeof(ifly_userMgr_t),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;	
}

int __stdcall NETDVR_DelUserInfo(int Handle, const char *username)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	ifly_userMgr_t para_info;
	memset(&para_info,0,sizeof(ifly_userMgr_t));

	para_info.flagOption = 2;
	memcpy(para_info.userInfo.name, username, strlen(username));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_SETUSERINFO,&para_info,sizeof(ifly_userMgr_t),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;
}




int __stdcall NETDVR_remoteGetHddInfo(int Handle, unsigned char hddindex, struct NETDVR_hddInfo_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];

	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	if (hddindex >= p->si.maxHddNum)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_GETHDDINFO,&hddindex,sizeof(hddindex),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		ifly_hddInfo_t para_info;
		memset(&para_info,0,sizeof(ifly_hddInfo_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_hddInfo_t));
		p_para->capability = ntohl(para_info.capability);
		p_para->freesize = ntohl(para_info.freesize);
		p_para->hdd_exist = para_info.hdd_exist;
		p_para->hdd_index = para_info.hdd_index;
	}
	return ret;

}

int __stdcall NETDVR_remoteGetSysVerInfo(int Handle, struct NETDVR_SysVerInfo_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_sysinfo_t para_info;
	memset(&para_info,0,sizeof(ifly_sysinfo_t));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_GETSYSINFO,NULL,0,buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf + sizeof(ifly_cp_header_t),sizeof(ifly_sysinfo_t));
		memcpy(p_para,&para_info,sizeof(ifly_sysinfo_t));
	}
	
	return ret;

}

int __stdcall NETDVR_sdkCheckout(int Handle, char* chkstring)
{
	NETDVR_SysVerInfo_t param_ret;

	int ret;
	
	if ((!chkstring )|| (strlen(chkstring)<1))
	{
		return NETDVR_ERR_PARAM;
	}
	
	memset(&param_ret, 0, sizeof(param_ret));
	ret = NETDVR_remoteGetSysVerInfo(Handle, &param_ret);
	
	if (NETDVR_SUCCESS == ret)
	{
		
		if (strcmp(param_ret.devicemodel, chkstring))
		{
			return NETDVR_ERR_SDK_CHECKFAILED;
		}

	}
	
	return ret;

}

int __stdcall NETDVR_setConnectTimeout(unsigned int millisecond)
{
	g_connect_timeout = millisecond;
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_getConnectTimeout(unsigned int *pMillisecond)
{
	if (NULL==pMillisecond)
	{
		return NETDVR_ERR_PARAM;
	}
	*pMillisecond = g_connect_timeout;
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_sendExtendCmd( int Handle, unsigned short wCommand, const void *pInData, int nInDataLen, void* pOutData, int nMaxOutDatalen )
{

	BYTE ackbuf[2048] = {0};

	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	int ret = 0;
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, wCommand, pInData, nInDataLen, ackbuf, sizeof(ackbuf), g_connect_timeout);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (pOutData)
	{
		memcpy(pOutData, ackbuf+sizeof(ifly_cp_header_t), min(nMaxOutDatalen, /*sizeof(ackbuf)*/(sizeof(ackbuf)-sizeof(ifly_cp_header_t))));
	}

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret; 	
}

int __stdcall NETDVR_setPicAdjust( int Handle, const struct NETDVR_PICADJUST_T *p_para )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_PicAdjust_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	para_info.channel_no = p_para->channel_no;
	para_info.copy2chnmask = htonl(p_para->copy2chnmask);
	para_info.flag = p_para->flag;
	para_info.val = p_para->val;
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETPICADJ, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;

}

int __stdcall NETDVR_getPicAdjust( int Handle, unsigned char chn, enum NETDVR_PICADJUST type, struct NETDVR_PICADJUST_T *p_para )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};


	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	ifly_PicAdjust_t para_info;
	memset(&para_info, 0, sizeof(para_info));	

	char tempBuf[2] = {0};
	tempBuf[0] = chn;
	tempBuf[1] = type;
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETPICADJ, tempBuf, sizeof(tempBuf), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->channel_no = para_info.channel_no;
		p_para->copy2chnmask = htonl(para_info.copy2chnmask);
		p_para->flag = (enum NETDVR_PICADJUST)para_info.flag;
		p_para->val = para_info.val;

	}
	return ret;

}

int __stdcall NETDVR_GetVGAsoluton(int Handle, struct NETDVR_VGARESOLLIST *p_para)
{
	int ret = 0; 
	char buf[2048] = {0};
	
	struct NETDVR_INNER_t *p;	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETVGASOLLIST, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_VGA_Solution pvga;

		memset(&pvga,0,sizeof(ifly_VGA_Solution));
		memcpy(&pvga,buf+sizeof(ifly_cp_header_t),sizeof(ifly_VGA_Solution));
		for (int i = 0; i < 16; i++)
		{
			p_para->vgapro[i].flashrate = pvga.vgapro[i].flashrate;
			p_para->vgapro[i].height = ntohs(pvga.vgapro[i].height);
			p_para->vgapro[i].width = ntohs(pvga.vgapro[i].width);
		}	
	}
	
	return ret; 
}


int __stdcall NETDVR_GetSubStreamParam(int Handle, unsigned char chn, struct NETDVR_SubStreamParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];

	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	if (chn >= p->si.maxSubstreamNum)
	{
		return NETDVR_ERR_PARAM;
	}
	ifly_SubStreamParam_t para_info;
	memset(&para_info,0,sizeof(ifly_SubStreamParam_t));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETSUBSTREAMPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->chn = para_info.channelno;
		p_para->copy2chnmask = ntohl(para_info.copy2chnmask);
		p_para->sub_bit_type = (enum NETDVR_SUBBITTYPE)para_info.sub_bit_type;
		p_para->sub_bitrate = (enum NETDVR_SUBBITRATE)ntohl(para_info.sub_bitrate);
		p_para->sub_flag = (enum NETDVR_SUBFLAG)para_info.sub_flag;
		p_para->sub_framerate = (enum NETDVR_SUBFRAMERATE)para_info.sub_framerate;
		p_para->sub_intraRate = ntohs(para_info.sub_intraRate);
		p_para->sub_maxQ = para_info.sub_maxQ;
		p_para->sub_minQ = para_info.sub_minQ;
		p_para->sub_qi = para_info.sub_qi;
		p_para->sub_quality = (enum NETDVR_SUBVIDEOQUALITY)para_info.sub_quality;

	}

	return ret;
}

int __stdcall NETDVR_SetSubStreamParam(int Handle, const struct NETDVR_SubStreamParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_SubStreamParam_t para_info;
	memset(&para_info,0,sizeof(ifly_SubStreamParam_t));
	para_info.channelno = p_para->chn;
	para_info.copy2chnmask = htonl(p_para->copy2chnmask);
	para_info.sub_bit_type = p_para->sub_bit_type;
	para_info.sub_bitrate = htonl(p_para->sub_bitrate);
	para_info.sub_flag = p_para->sub_flag;
	para_info.sub_framerate = p_para->sub_framerate;
	para_info.sub_intraRate = htons(p_para->sub_intraRate);
	para_info.sub_maxQ = p_para->sub_maxQ;
	para_info.sub_minQ = p_para->sub_minQ;
	para_info.sub_qi = p_para->sub_qi;
	para_info.sub_quality = p_para->sub_quality;
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETSUBSTREAMPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	

	return ret;
}

int __stdcall NETDVR_GetAlarmNoticeParam(int Handle, struct NETDVR_AlarmNoticeParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];

	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETALARMNOTICYPARAM, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_AlarmNoticeParam_t para_info;
		memset(&para_info,0,sizeof(ifly_AlarmNoticeParam_t));
		memcpy(&para_info,buf + sizeof(ifly_cp_header_t),sizeof(ifly_AlarmNoticeParam_t));
		memcpy(p_para->alarm_email,para_info.alarm_email,sizeof(para_info.alarm_email));
		memcpy(p_para->alarm_mobile,para_info.alarm_mobile,sizeof(para_info.alarm_mobile));
	}
	return ret;
}

int __stdcall NETDVR_SetAlarmNoticeParam(int Handle, const struct NETDVR_AlarmNoticeParam_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	ifly_AlarmNoticeParam_t para_info;
	memset(&para_info,0,sizeof(ifly_AlarmNoticeParam_t));
	memcpy(para_info.alarm_email,p_para->alarm_email,sizeof(p_para->alarm_email));
	memcpy(para_info.alarm_mobile,p_para->alarm_mobile,sizeof(p_para->alarm_mobile));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETALARMNOTICYPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;
}

int __stdcall NETDVR_GetRecordSCH(int Handle, unsigned char chn, enum NETDVR_WEEKDAY day, struct NETDVR_RecordSCH_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};

	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_RecordSCH_t para_info;
	memset(&para_info,0,sizeof(ifly_RecordSCH_t));

	char tempBuf[2] = {0};
	tempBuf[0] = chn;
	tempBuf[1] = day;
// 	memcpy(tempBuf,&chn,sizeof(chn));
// 	memcpy(tempBuf+1,&day,sizeof(day));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETRECSCHPARAM, tempBuf, sizeof(tempBuf), buf, sizeof(buf), CTRL_PROTOCOL_CONNECT_DEFAULT * 5);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->chn = para_info.chn;
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);
		p_para->copy2Weekday = para_info.copy2Weekday;
		for (int j = 0; j < 4; j++)
		{
			p_para->recTimeFieldt[j].endtime = ntohl(para_info.TimeFiled[j].endtime);
			p_para->recTimeFieldt[j].starttime = ntohl(para_info.TimeFiled[j].starttime);
			p_para->recTimeFieldt[j].flag_alarm = para_info.TimeFiled[j].flag_alarm;
			p_para->recTimeFieldt[j].flag_md = para_info.TimeFiled[j].flag_md;
			p_para->recTimeFieldt[j].flag_sch = para_info.TimeFiled[j].flag_sch;
		}
		p_para->weekday = (enum NETDVR_WEEKDAY)para_info.weekday;

	}

	return ret;
}

int __stdcall NETDVR_SetRecordSCH(int Handle, const struct NETDVR_RecordSCH_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	ifly_RecordSCH_t para_info;
	memset(&para_info,0,sizeof(ifly_RecordSCH_t));
	para_info.chn =p_para->chn;
	para_info.copy2Chnmask = htonl(p_para->copy2Chnmask);
	para_info.copy2Weekday = p_para->copy2Weekday;
	for (int i = 0;i<4;i++)
	{
		para_info.TimeFiled[i].endtime = htonl(p_para->recTimeFieldt[i].endtime);
		para_info.TimeFiled[i].starttime = htonl(p_para->recTimeFieldt[i].starttime);
		para_info.TimeFiled[i].flag_alarm = p_para->recTimeFieldt[i].flag_alarm;
		para_info.TimeFiled[i].flag_md = p_para->recTimeFieldt[i].flag_md;
		para_info.TimeFiled[i].flag_sch = p_para->recTimeFieldt[i].flag_sch;
	}
	para_info.weekday = p_para->weekday;

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETRECSCHPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	

	return ret;
}

int __stdcall NETDVR_GetRecordState(int Handle, struct NETDVR_ManualRecord_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_ManualRecord_t para_info;
	memset(&para_info,0,sizeof(ifly_ManualRecord_t));

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_GETMANUALREC,NULL,0,buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_ManualRecord_t));
		p_para->chnRecState = ntohl(para_info.chnRecState);
	}

	return ret;
}

int __stdcall NETDVR_SetRecordState(int Handle, const struct NETDVR_ManualRecord_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;

	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_ManualRecord_t para_info;
	memset(&para_info,0,sizeof(ifly_ManualRecord_t));
	para_info.chnRecState = htonl(p_para->chnRecState); 

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_SETMANUALREC,&para_info,sizeof(para_info),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

static PFUN_SearchDevice_CB pFindCB = NULL;

void exchangeFindEquipment(NETDVR_DeviceInfo_t dev, void* pContext)
{
	//	vector<DevEquipments_s>::iterator it = CPageDeviceMgrSet::m_vDevEquipments.end();
	
	pFindCB(dev, pContext);
	//	m_vDeviceInfo.push_back(dev);
}

int __stdcall NETDVR_RegCallBackSearchDevice(PFUN_SearchDevice_CB p_cb_func, void* pContext)
{
	pFindCB = p_cb_func;
	SetFindDeviceCB(pFindDeivce_CB(exchangeFindEquipment), pContext);

	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_SearchDevice()
{
	SearchDevice();

	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_PatrolSync() //广播设备，同步轮巡
{
	printf("%s\n", "NETDVR_PatrolSync");
	PatrolSync();

	return NETDVR_SUCCESS;
}


int __stdcall NETDVR_regCBAlarmState(int Handle, PFUN_ALARMSTATE_T p_cb_func, unsigned int dwContent)
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	
	p->p_cb_alarmstate = p_cb_func;
	p->dwAlarmStateContent = dwContent;
	
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_SetAlarmUpload(int Handle, const unsigned char uploadflag)
{
	int ret = 0;
	char buf[2048] = {0};

	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	p->byAlarmUploadFlag = uploadflag;

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_ALARMUPLOADCENTER,&uploadflag,sizeof(uploadflag),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_DeinterlaceVideoFrame( pFrameHeadrDec pFrmHdrDec )
{
	if (NULL == pFrmHdrDec)
	{
		return NETDVR_ERR_PARAM;
	}
	
	if (MEDIA_TYPE_H264 != pFrmHdrDec->mediaType || NULL == pFrmHdrDec->data)
	{
		return NETDVR_ERR_PARAM;
	}
	
	if (pFrmHdrDec->reserved1[0])
	{
		//csp modify
		return NETDVR_SUCCESS;
		
		//csp modify
		//deinterlace_bottom_field_inplace((u8 *)pFrmHdrDec->data,
		//	pFrmHdrDec->video_param.width,
		//	pFrmHdrDec->video_param.width,
		//	pFrmHdrDec->video_param.height);						
	}
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_VOIPRegRcvCB( int Handle, int voipindex, pDecFrameCallBack rcvfunc, unsigned int dwContext )
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	
	LockMutex(p->voip_rcv[voipindex].cb_dec_lock);
	if (rcvfunc)
	{
		p->voip_rcv[voipindex].pDecFrameCB = rcvfunc;
		p->voip_rcv[voipindex].dwContent = dwContext;
	}
	
	UnlockMutex(p->voip_rcv[voipindex].cb_dec_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_VOIPRegCaptureCB( int Handle, int voipindex, pDecFrameCallBack getcapturefunc, unsigned int dwContext )
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	
	if (getcapturefunc)
	{
		p->voip_snd[voipindex].pDecFrameCB = getcapturefunc;
		p->voip_snd[voipindex].dwContent = dwContext;
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;	
}

int __stdcall NETDVR_VOIPSetSendMode( int Handle, int voipindex, unsigned char flagmode )
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	p->voip_snd[voipindex].sndmode = flagmode;

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;	
}

int __stdcall NETDVR_VOIPSendData( int Handle, int voipindex, FrameHeadrDec voipdata )
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	
	if (p->voip_snd[voipindex].sndmode)
	{
		p->voip_snd[voipindex].m_InterPhoneFrmHdr.m_pData= (BYTE*)voipdata.data;
		p->voip_snd[voipindex].m_InterPhoneFrmHdr.m_dwDataSize = voipdata.data_size;

		if (p->voip_snd[voipindex].p_voip_property->VOIPBitPerSample == 8 )
		{
			
			for(DWORD i=0;i<voipdata.data_size;i++)
			{   
				p->voip_snd[voipindex].m_InterPhoneFrmHdr.m_pData[i] -= 0x80;   // unsigned 2 signed
			}
			
		}

		ifly_MediaFRAMEHDR_t hdr;
		memset(&hdr, 0, sizeof(hdr));
		hdr.m_byMediaType = MEDIA_TYPE_PCMU;
		hdr.m_dwTimeStamp = htonl(GetTickCount());
		hdr.m_dwDataSize = htonl(p->voip_snd[voipindex].m_InterPhoneFrmHdr.m_dwDataSize);
		
		int ret = 0;
		ret=loopsend(p->voip_snd[voipindex].psnd_t->sockfd,(char *)&hdr,sizeof(ifly_MediaFRAMEHDR_t));
		if(ret <= 0)
		{
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_SEND;
		}

		ret=loopsend(p->voip_snd[voipindex].psnd_t->sockfd,
			(char *)p->voip_snd[voipindex].m_InterPhoneFrmHdr.m_pData,
			p->voip_snd[voipindex].m_InterPhoneFrmHdr.m_dwDataSize);

		if(ret <= 0)
		{
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_SEND;
		}
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;	
}

int __stdcall NETDVR_CtrlCDROM(int Handle,unsigned char ctrlflag)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	char buf[2048] = {0};
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_CDROM,&ctrlflag,sizeof(unsigned char),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;
}

int __stdcall NETDVR_GetComStyle(int Handle,int *p_nStyle)
{
	int ret = 0;

	return ret;
}

int __stdcall NETDVR_SetComStyle(int Handle,int nStyle)
{
	int ret = 0;
	
	return ret;
}

int __stdcall NETDVR_GetComParam(int Handle, unsigned char serialport, struct NETDVR_ComParam_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_com_param_t para_info;
	memset(&para_info,0,sizeof(ifly_com_param_t));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_COM_PROTOCOL_GET,&serialport,sizeof(serialport),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_com_param_t));
		p_para->com_baudrate = ntohl(para_info.com_baudrate);
		p_para->com_checkbit = (enum NETDVR_YTCOM_CheckBit_t)para_info.com_checkbit;
		p_para->com_databit = (enum NETDVR_YTCOM_DataBit_t)para_info.com_databit;
		p_para->com_protocol = (enum NETDVR_YTCOM_Protocol_t)para_info.com_protocol;
		p_para->com_stopbit = (enum NETDVR_YTCOM_StopBit_t)para_info.com_stopbit;
		p_para->serialport = para_info.serialport;
		memcpy(p_para->reserved,para_info.reserved,sizeof(para_info.reserved));
	}
	
	return ret;
}

int __stdcall NETDVR_SetComParam(int Handle, const struct NETDVR_ComParam_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_com_param_t para_info;
	memset(&para_info,0,sizeof(ifly_com_param_t));
	para_info.com_baudrate = htonl(p_para->com_baudrate);
	para_info.com_checkbit = (enum NETDVR_YTCOM_CheckBit_t)p_para->com_checkbit;
	para_info.com_databit = (enum NETDVR_YTCOM_DataBit_t)p_para->com_databit;
	para_info.com_protocol = (enum NETDVR_YTCOM_Protocol_t)p_para->com_protocol;
	para_info.com_stopbit = (enum NETDVR_YTCOM_StopBit_t)p_para->com_stopbit;
	para_info.serialport = p_para->serialport;
	memcpy(para_info.reserved,p_para->reserved,sizeof(p_para->reserved));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_COM_PROTOCOL_SET,&para_info,sizeof(para_info),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}


int __stdcall NETDVR_SerialStart( int Handle, int lSerialPort, pSerialDataCallBack cbSerialDataCallBack, unsigned int dwContent )
{
	if (lSerialPort <1 || lSerialPort >2)
	{
		return NETDVR_ERR_PARAM;
	}

	struct NETDVR_INNER_t *p;

	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}

	if (p->serial[lSerialPort-1].bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_SERIAL_REOPEN;
	}
	
	switch (lSerialPort)
	{
	case 1:
		p->serial[lSerialPort-1].serialport = SERIAL_232;
		break;
	case 2:
		p->serial[lSerialPort-1].serialport = SERIAL_485;
		break;
	}

	p->serial[lSerialPort-1].req.command = 8;
	p->serial[lSerialPort-1].req.SerialPort = (u8)lSerialPort;
	p->serial[lSerialPort-1].cbSerialDataCallBack = cbSerialDataCallBack;
	p->serial[lSerialPort-1].dwContent = dwContent;

	int ret = NETDVR_ERR_CONNECT;

	if (SetRcvTcpFrame(p, &p->serial[lSerialPort-1].prcv_t, p->serial[lSerialPort-1].req, TRUE))
	{
		p->serial[lSerialPort-1].hRcvthread = CreateThread(NULL, 0, SerialRcvThread, &p->serial[lSerialPort-1], 0 , NULL);
		if (!p->serial[lSerialPort-1].hRcvthread)
		{
			ret = NETDVR_ERR_OUTOFMEMORY;
		}
		else
		{
			p->serial[lSerialPort-1].bOpened = 1;
			ret = NETDVR_SUCCESS;
		}
		
	}

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret;	
}

int __stdcall NETDVR_SerialSend( int Handle, int lSerialPort, unsigned char bySerialChannel, char* pSendBuf, unsigned int dwBufSize )
{
	if (lSerialPort <1 || lSerialPort >2 || dwBufSize > 4096)
	{
		return NETDVR_ERR_PARAM;
	}
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	if(p->serial[lSerialPort-1].prcv_t)
	{
		ifly_serial_hdr hdr;
		memset(&hdr, 0, sizeof(hdr));
		hdr.byChannel = bySerialChannel;
		hdr.dwDataSize = htonl(dwBufSize);
		
	
		int ret = 0;
		ret = loopsend(p->serial[lSerialPort-1].prcv_t->sockfd,(char *)&hdr,sizeof(ifly_serial_hdr));
		if(ret <= 0)
		{
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_SEND;
		}

		ret=loopsend(p->serial[lSerialPort-1].prcv_t->sockfd,(char *)pSendBuf,dwBufSize);
		if(ret <= 0)
		{
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_SEND;
		}
	}

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;

}

int __stdcall NETDVR_SerialStop( int Handle, int lSerialPort )
{
	if (lSerialPort <1 || lSerialPort >2)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (p->serial[lSerialPort-1].prcv_t)
	{
		char buf[2048] = {0};
		u32 id = htonl(p->serial[lSerialPort-1].prcv_t->linkid);
		if (!p->b_cmdConnectLost)
		{
			send_command(p->cph, CTRL_CMD_SERIALSTOP, &id, sizeof(id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
		} 
		else
		{
			int ret = NETDVR_ERR_SEND;
		}
		
	}

	
	SetRcvTcpFrame(p, &p->serial[lSerialPort-1].prcv_t, p->serial[lSerialPort-1].req, FALSE);
	if (p->serial[lSerialPort-1].hRcvthread)
	{
		WaitForSingleObject(p->serial[lSerialPort-1].hRcvthread, INFINITE);
		CloseHandle(p->serial[lSerialPort-1].hRcvthread);
		p->serial[lSerialPort-1].hRcvthread = NULL;
	}
		
	p->serial[lSerialPort-1].bOpened = 0;
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;		
}

int __stdcall NETDVR_setReconnectTime( unsigned int millisecond )
{
	g_dwReconnectTime = millisecond;
	return NETDVR_SUCCESS;	
}

int __stdcall NETDVR_regCBMsgReconnect( int Handle, pCBReconnMsg p_cb_func, unsigned int dwContent )
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	p->p_cb_recconn = p_cb_func;
	p->dwContentReconn = dwContent;
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS; 
}

int __stdcall NETDVR_setReconnectFlag( int Handle, unsigned char reconflag )
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	p->bEnableRecon = reconflag;
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS; 
}

int __stdcall NETDVR_getSysLangList( int Handle, NETDVR_SysLangList_t* p_para )
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}

	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETSYSLANGLIST, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	}
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_SysLangList_t para_info;
		memset(&para_info,0,sizeof(ifly_SysLangList_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_SysLangList_t));
		p_para->max_langnum = para_info.max_langnum;
		for (int i = 0; i< 32; i++)
		{
			p_para->langlist[i] = (NETDVR_SYS_LANGUAGE)para_info.langlist[i];
		}
	}
	
	return ret;	
}

int __stdcall NETDVR_getBitRateList( int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_bitRateList_t* p_para )
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		char sendbuf[2] = {0};
		sendbuf[0] = chn;
		sendbuf[1] = vidoetype;
		ret = send_command( p->cph, CTRL_CMD_GETBITRATELIST, sendbuf, sizeof(sendbuf), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_bitRateList_t para_info;
		memset(&para_info,0,sizeof(ifly_bitRateList_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_bitRateList_t));
		p_para->chn = para_info.chn;
		p_para->videotype = para_info.videotype;
		for (int i = 0; i< 16; i++)
		{
			p_para->bitratelist[i] = para_info.bitratelist[i];
		}
	}
	
	return ret;		
}

int __stdcall NETDVR_getxwServerParams(int Handle, struct NETDVR_xwServer_t *pxwServ)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	ret = send_command(p->cph, CTRL_CMD_XWSERVER_GET, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);
	if(NETDVR_SUCCESS == ret)
	{
		ifly_pingtai_xingwang_t para_info;
		memset(&para_info,0,sizeof(para_info));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		pxwServ->flag_server = para_info.flag_pingtai;
		pxwServ->flag_verifytime = para_info.flag_verifytime;
		pxwServ->ip_server = para_info.ip_pingtai;
		pxwServ->port_server = ntohs(para_info.port_pingtai);
		pxwServ->port_download = ntohs(para_info.port_download);
		memcpy(pxwServ->device_serial,para_info.device_name,sizeof(pxwServ->device_serial));
		memcpy(pxwServ->device_passwd,para_info.device_passwd,sizeof(pxwServ->device_passwd));
	}
	
	return ret;
}

int __stdcall NETDVR_setxwServerParams(int Handle, const struct NETDVR_xwServer_t *pxwServ)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	ifly_pingtai_xingwang_t para_info;
	memset(&para_info,0,sizeof(para_info));
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	para_info.flag_pingtai = pxwServ->flag_server;
	para_info.flag_verifytime = pxwServ->flag_verifytime;
	para_info.ip_pingtai = pxwServ->ip_server;
	para_info.port_pingtai = htons(pxwServ->port_server);
	para_info.port_download = htons(pxwServ->port_download);
	memcpy(para_info.device_name,pxwServ->device_serial,sizeof(para_info.device_name));
	memcpy(para_info.device_passwd,pxwServ->device_passwd,sizeof(para_info.device_passwd));
	
	ret = send_command(p->cph, CTRL_CMD_XWSERVER_SET, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);
	
	return ret;
}

int __stdcall NETDVR_getSMTPServerParams(int Handle, struct NETDVR_SMTPServer_t *pSMTPServ)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	ret = send_command(p->cph, CTRL_CMD_GETEMAILSMTP, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);
	if(NETDVR_SUCCESS == ret)
	{
		ifly_AlarmEmail_SMTP_t para_info;
		memset(&para_info,0,sizeof(para_info));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(pSMTPServ->alarm_email,para_info.alarm_email,sizeof(pSMTPServ->alarm_email));
		memcpy(pSMTPServ->reserved,para_info.reserved,sizeof(pSMTPServ->reserved));
		memcpy(pSMTPServ->SMTP_svr,para_info.SMTP_svr,sizeof(pSMTPServ->SMTP_svr));
		memcpy(pSMTPServ->username,para_info.username,sizeof(pSMTPServ->username));
		memcpy(pSMTPServ->userpw,para_info.userpw,sizeof(pSMTPServ->userpw));
		pSMTPServ->SMTPport = ntohs(para_info.SMTPport);
		pSMTPServ->flag_ssl = para_info.flag_ssl;
	}
	
	return ret;
}

int __stdcall NETDVR_setSMTPServerParams(int Handle, const struct NETDVR_SMTPServer_t *pSMTPServ)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	ifly_AlarmEmail_SMTP_t para_info;
	memset(&para_info,0,sizeof(para_info));
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	memcpy(para_info.alarm_email,pSMTPServ->alarm_email,sizeof(para_info.alarm_email));
	memcpy(para_info.reserved,pSMTPServ->reserved,sizeof(para_info.reserved));
	memcpy(para_info.SMTP_svr,pSMTPServ->SMTP_svr,sizeof(para_info.SMTP_svr));
	memcpy(para_info.username,pSMTPServ->username,sizeof(para_info.username));
	memcpy(para_info.userpw,pSMTPServ->userpw,sizeof(para_info.userpw));
	para_info.SMTPport = htons(pSMTPServ->SMTPport);
	para_info.flag_ssl = pSMTPServ->flag_ssl;
	ret = send_command(p->cph, CTRL_CMD_SETEMAILSMTP, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);
	
	return ret;
}

int __stdcall NETDVR_getLASAFQQInfo(int Handle, struct NETDVR_LASServer_t *pLASServ)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	ret = send_command(p->cph, CTRL_CMD_LASAFQQ_GETINFO, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);
	if(NETDVR_SUCCESS == ret)
	{
		ifly_las_afqq_info para_info;
		memset(&para_info,0,sizeof(para_info));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(pLASServ->sn,para_info.sn,sizeof(pLASServ->sn));
		memcpy(pLASServ->productcode,para_info.productcode,sizeof(pLASServ->productcode));
		memcpy(pLASServ->macaddr,para_info.macaddr,sizeof(pLASServ->macaddr));
		memcpy(pLASServ->reserved,para_info.reserved,sizeof(pLASServ->reserved));
	}
	
	return ret;
}

int __stdcall NETDVR_setLASAFQQInfo(int Handle, const struct NETDVR_LASServer_t *pLASServ)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	ifly_las_afqq_info para_info;
	memset(&para_info,0,sizeof(para_info));
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	memcpy(para_info.sn,pLASServ->sn,sizeof(para_info.sn));
	memcpy(para_info.productcode,pLASServ->productcode,sizeof(para_info.productcode));
	memcpy(para_info.macaddr,pLASServ->macaddr,sizeof(para_info.macaddr));
	memcpy(para_info.reserved,pLASServ->reserved,sizeof(para_info.reserved));
	ret = send_command(p->cph, CTRL_CMD_LASAFQQ_SETINFO, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);

	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_formatHdd( int Handle, unsigned char hddindex, pFormatProgressCallBack pCBFunc, unsigned int dwContent )
{
	struct NETDVR_INNER_t *p;

	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}


	if (hddindex >= p->si.maxHddNum)
	{
		return NETDVR_ERR_PARAM;
	}

	if (p->fmt_hdd[hddindex].bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_RERECIVE;
	}
	

	ifly_TCP_Stream_Req req;
	memset(&req, 0, sizeof(req));

	req.command = 9;
	req.formatHddIndex = (u8)hddindex;

	p->fmt_hdd[hddindex].pCBFmtProgress = pCBFunc;
	p->fmt_hdd[hddindex].dwContent = dwContent;

	int ret = NETDVR_ERR_CONNECT;
	unsigned int errcode = 0;  
	if (SetRcvTcpFrame(p, &p->fmt_hdd[hddindex].prcv_t, req, TRUE,NULL,&errcode))
	{
		p->fmt_hdd[hddindex].hRcvthread = CreateThread(NULL, 0, FormatHddThread, &p->fmt_hdd[hddindex], 0 , NULL);
		if (!p->fmt_hdd[hddindex].hRcvthread)
		{
			ret = NETDVR_ERR_OUTOFMEMORY;
		}
		else
		{
			p->fmt_hdd[hddindex].bOpened = 1;
			ret = NETDVR_SUCCESS;
		}
		
	}
	if (CTRL_FAILED_BUSY == errcode)
	{
		ret = NETDVR_ERR_BUSY;		
	}

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret;	
}

int __stdcall NETDVR_remoteSnap( int Handle, unsigned char chn, char* filename )
{
	struct NETDVR_INNER_t *p;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	
	if (chn >= p->si.maxChnNum)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}
	
	
	ifly_TCP_Stream_Req req;
	memset(&req, 0, sizeof(req));
	
	req.command = 10;


	int ret = 0;

	//没建立连接就先连接
	if (NULL == p->remotesnap.prcv_t)
	{
		if (!SetRcvTcpFrame(p, &p->remotesnap.prcv_t, req, TRUE))
		{
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_CONNECT;
		}
	}

	if (filename)
	{
		strcpy(p->remotesnap.filename,  filename);
	}
	else
	{
		SYSTEMTIME SystemTime;
		GetLocalTime(&SystemTime);
		wsprintf(p->remotesnap.filename, TEXT("c:\\temp\\Chn%d%04d%02d%02d%02d%02d%02d.jpg"), chn+1, SystemTime.wYear, SystemTime.wMonth
		, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);		
	}
	
	ifly_snapreq_t imgreq;
	memset(&imgreq, 0, sizeof(imgreq));
	imgreq.chn = chn;

	ret = loopsend(p->remotesnap.prcv_t->sockfd,(char *)&imgreq,sizeof(imgreq));
	if(ret <= 0)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_SEND;
	}

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	Sleep(100);

	return NETDVR_SUCCESS;		
}

int __stdcall NETDVR_RegDecCB(int nRealPlayHandle, pDecFrameCallBack pCBFun, unsigned int dwContent)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	reg_rcvcb_dec(&pMonitor->video_rcv, pCBFun, dwContent);
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_StartRealPlay( int Handle, const struct RealPlayClientInfo_t* pClientinfo, int* pRealPlayHandle )
{
	//参数判断
	if (!Handle || !pClientinfo || !pRealPlayHandle)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	int ret;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		*pRealPlayHandle = 0;
		UnlockMutex(g_pool_lock);
		
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	u8 chnlimit = 0;
	if (pClientinfo->streamtype == 0)
	{
		chnlimit = p->si.maxChnNum;
	} 
	else if (pClientinfo->streamtype == 1)
	{
		chnlimit = p->si.maxSubstreamNum;
	}

	if (pClientinfo->rcv_chn >= chnlimit)
	{
		*pRealPlayHandle = 0;
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}
	
	//分配预览handle内存空间
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)malloc(sizeof(ifly_monitor_t));
	if (NULL == pMonitor)
	{
		*pRealPlayHandle = 0;
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_OUTOFMEMORY;
	}

	//初始化预览handle参数，分配解码buf空间
	memset(&pMonitor->video_rcv, 0, sizeof(IFLY_MediaRcvPara_t));
	memset(&pMonitor->audio_rcv, 0, sizeof(IFLY_MediaRcvPara_t));
	memset(&pMonitor->record_para, 0, sizeof(IFLY_RecordPara_t));
	memset(&pMonitor->record_para2, 0, sizeof(IFLY_RecordPara_t));
	memset(&pMonitor->snap_shot, 0, sizeof(IFLY_Snapshot_t));
	pMonitor->pNext = NULL;
	pMonitor->channel = pClientinfo->rcv_chn;	
	pMonitor->pDeviceHandle = p;
	
	//插到前面
	pMonitor->pNext = p->m_pMonitor;
	p->m_pMonitor = pMonitor;
	
	if (pClientinfo->streamtype == 0)
	{
		pMonitor->video_rcv.rcv_type = NETDVR_RCV_VIDEO;
	}
	else if (pClientinfo->streamtype == 1)
	{
		pMonitor->video_rcv.rcv_type = NETDVR_RCV_SUBVIDEO;
	}
	else
	{
		*pRealPlayHandle = 0;
		p->m_pMonitor=p->m_pMonitor->pNext;
		free(pMonitor);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}
	
	CreateMutexHandle(&pMonitor->record_para.cb_rec_lock);
	CreateMutexHandle(&pMonitor->record_para.cb_filename_lock);
	CreateMutexHandle(&pMonitor->record_para2.cb_rec_lock);
	CreateMutexHandle(&pMonitor->record_para2.cb_filename_lock);
	CreateMutexHandle(&pMonitor->record_para.rec_lock);
	CreateMutexHandle(&pMonitor->record_para2.rec_lock);
	CreateMutexHandle(&pMonitor->snap_shot.snap_lock);
	
	pMonitor->video_rcv.p_record_para = &pMonitor->record_para;
	pMonitor->video_rcv.p_record_para2 = &pMonitor->record_para2;
	pMonitor->video_rcv.p_snapshot = &pMonitor->snap_shot;
	
	//csp modify
	//int nMalloclen = p->video_property.max_videowidth*p->video_property.max_videoheight*2;//MAX_VIDEODECLEN;
	int nMalloclen = ((p->video_property.max_videowidth+15)/16*16) * ((p->video_property.max_videoheight+15)/16*16) * 3 / 2;
	
	//char showmsg[256];
	//sprintf(showmsg,"img=%dx%dx2=%d",p->video_property.max_videowidth,p->video_property.max_videoheight,nMalloclen);
	//::MessageBox(NULL,showmsg,NULL,MB_OK);
	
	pMonitor->video_rcv.p_frmdecBuf = (unsigned char *)malloc(nMalloclen);
	if (NULL == pMonitor->video_rcv.p_frmdecBuf)
	{
		*pRealPlayHandle = 0;
		p->m_pMonitor=p->m_pMonitor->pNext;
		free(pMonitor);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_OUTOFMEMORY;
	}
	memset(pMonitor->video_rcv.p_frmdecBuf, 0, nMalloclen);
	
	ret = open_reciever(&pMonitor->video_rcv, pClientinfo->pEncFrameCBFunc, pClientinfo->dwEncFrameContent, p->si.deviceIP);
	
	//创建&设置解码器
	//printf("NETDVR_StartRealPlay 1\n");
	set_rcvvideo_decoder_fmt(&pMonitor->video_rcv, NETDVR_FMT_YV12);
	reg_rcvcb_dec(&pMonitor->video_rcv, pClientinfo->pDecFrameCBFunc, pClientinfo->dwDecFrameContent);
	
	if(pClientinfo->pDecFrameCBFunc != NULL)//csp modify
	{
		create_rcvvideo_decoder(&pMonitor->video_rcv, &p->video_property);
		if (0 != pMonitor->video_rcv.fmt)
		{
			set_rcvvideo_decoder_fmt(&pMonitor->video_rcv, pMonitor->video_rcv.fmt);
		}
	}
	
#if 0
	if (pMonitor->video_rcv.bOpened)
	{
		*pRealPlayHandle = 0;
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_BUSY;
	}
#endif
	
	//开始传输码流
	pMonitor->video_rcv.req.command = 0;
	pMonitor->video_rcv.req.Monitor_t.chn = pClientinfo->rcv_chn;
	
	if (pClientinfo->streamtype == 0)
	{
		pMonitor->video_rcv.req.Monitor_t.type = 0;
		
		//csp modify 20130423
		pMonitor->video_rcv.req.reserved[0] = pClientinfo->wnd_num;
		pMonitor->video_rcv.req.reserved[1] = pClientinfo->wnd_index;
		pMonitor->video_rcv.req.reserved[6] = 0x5A;
		//if(pClientinfo->reserved[0] == 0x5A)
		//{
		//	pMonitor->video_rcv.req.reserved[0] = pClientinfo->wnd_num;
		//	pMonitor->video_rcv.req.reserved[1] = pClientinfo->wnd_index;
		//	pMonitor->video_rcv.req.reserved[6] = 0x5A;
		//}
		//else
		//{
		//	pMonitor->video_rcv.req.reserved[0] = 0;
		//	pMonitor->video_rcv.req.reserved[1] = 0;
		//	pMonitor->video_rcv.req.reserved[6] = 0;
		//}
	}
	else if (pClientinfo->streamtype == 1)
	{
		pMonitor->video_rcv.req.Monitor_t.type = 2;
		
		//csp modify 20130423
		pMonitor->video_rcv.req.reserved[0] = 0;
		pMonitor->video_rcv.req.reserved[1] = 0;
		pMonitor->video_rcv.req.reserved[6] = 0;
	}
	
	pMonitor->video_rcv.p_video_property = &p->video_property;
	
#ifdef USE_CONNMSG_THREAD
	u32 command = MAKELONG(0, p->dvr_id);
	if(PostThreadMessage(g_dwConnMsgThreadID, STARTCONNECTMSG, (WPARAM)command, (LPARAM)&pMonitor->video_rcv))
	{
		*pRealPlayHandle = (int)pMonitor;
		ret = NETDVR_SUCCESS;
	}
#else
	if(SetRcvTcpFrame(p, &pMonitor->video_rcv.prcv_t, pMonitor->video_rcv.req, TRUE, &pMonitor->video_rcv))
	{
		pMonitor->video_rcv.bOpened = 1;
		*pRealPlayHandle = (int)pMonitor;
		ret = NETDVR_SUCCESS;
	}
#endif
	else
	{
		unreg_rcvcb_dec(&pMonitor->video_rcv);
		destroy_rcvvideo_decoder(&pMonitor->video_rcv);
		*pRealPlayHandle = 0;
		p->m_pMonitor=p->m_pMonitor->pNext;
		free(pMonitor->video_rcv.p_frmdecBuf);
		free(pMonitor);
		ret = NETDVR_ERR_CONNECT;
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);		
	
	return ret;
}

//csp modify 20130423
int __stdcall NETDVR_SetMonitorInfo(int nRealPlayHandle, unsigned char chn, unsigned char wnd_num, unsigned char wnd_index)
{
	//参数判断
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret = NETDVR_SUCCESS;
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	if (!pMonitor->video_rcv.bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}
	
	if (pMonitor->video_rcv.prcv_t)
	{
		u32 id = htonl(pMonitor->video_rcv.prcv_t->linkid);
		if (id == 0)
		{
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_PARAM;
		}
		if (!p->b_cmdConnectLost)
		{
			ifly_wndinfo_t wndinfo;
			memset(&wndinfo,0,sizeof(wndinfo));
			wndinfo.id = id;
			wndinfo.chn = chn;
			wndinfo.wnd_num = wnd_num;
			wndinfo.wnd_index = wnd_index;
			char buf[2048] = {0};
			ret = send_command(p->cph, CTRL_CMD_SETMONITORINFO, &wndinfo, sizeof(wndinfo), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
		}
		else
		{
			ret = NETDVR_ERR_SEND;
		}
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret;
}

int __stdcall NETDVR_StopRealPlay( int nRealPlayHandle )
{
	//参数判断
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	int ret;
	//停止录像
	NETDVR_stopRecord(nRealPlayHandle);
	//停止音频
	NETDVR_CloseRealAudio(nRealPlayHandle);
	//以下停止视频
	//....
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	UnlockMutex(g_pool_lock);
	if (!pMonitor->video_rcv.bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}
	//清除解码器
	unreg_rcvcb_dec(&pMonitor->video_rcv);
	destroy_rcvvideo_decoder(&pMonitor->video_rcv);
	//停止码流
	if (pMonitor->video_rcv.prcv_t)
	{
		u32 id = htonl(pMonitor->video_rcv.prcv_t->linkid);
		if (id == 0)
		{
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_PARAM;
		}
		if (!p->b_cmdConnectLost)
		{
#ifndef USE_CONNMSG_THREAD		//by cj@20100415
			char buf[2048] = {0};
			ret = send_command(p->cph, CTRL_CMD_STOPVIDEOMONITOR, &id, sizeof(id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
#else
			u32 command = MAKELONG(CTRL_CMD_STOPVIDEOMONITOR, p->dvr_id);
			if(PostThreadMessage(g_dwConnMsgThreadID, STOPCONNECTMSG, (WPARAM)command, (LPARAM)id))
			{
				ret = NETDVR_SUCCESS;
			}
			else
			{
				ret = NETDVR_ERR_SEND;
			}
#endif
		} 
		else
		{
			ret = NETDVR_ERR_SEND;
		}
	}
	EnterCriticalSection(&p->m_hcs);
	SetRcvTcpFrame(p, &pMonitor->video_rcv.prcv_t, pMonitor->video_rcv.req, FALSE);	
	close_reciever(&pMonitor->video_rcv);	
	if (pMonitor->video_rcv.p_frmdecBuf)
	{
		free(pMonitor->video_rcv.p_frmdecBuf);
		pMonitor->video_rcv.p_frmdecBuf = NULL;
	}
	
#if 0
	if (!pMonitor->video_rcv.bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}
#endif

	pMonitor->video_rcv.bOpened = 0;

	CloseMutexHandle(pMonitor->record_para.cb_rec_lock);
	CloseMutexHandle(pMonitor->record_para.cb_filename_lock);
	CloseMutexHandle(pMonitor->record_para2.cb_rec_lock);
	CloseMutexHandle(pMonitor->record_para2.cb_filename_lock);
	CloseMutexHandle(pMonitor->record_para.rec_lock);
	CloseMutexHandle(pMonitor->record_para2.rec_lock);
	CloseMutexHandle(pMonitor->snap_shot.snap_lock);

	pMonitor->record_para.cb_rec_lock = NULL;
	pMonitor->record_para.cb_filename_lock = NULL;
	pMonitor->record_para2.cb_rec_lock = NULL;
	pMonitor->record_para2.cb_filename_lock = NULL;
	pMonitor->record_para.rec_lock = NULL;
	pMonitor->record_para2.rec_lock = NULL;
	pMonitor->snap_shot.snap_lock = NULL;

	//最后：释放pMonitor;
	CleanMsgQ(&g_decmsgQ, &pMonitor->video_rcv);

	ifly_monitor_t* pTmp = p->m_pMonitor;
	ifly_monitor_t* pTmp2 = NULL;
	while(pTmp)
	{
		if(pTmp==pMonitor)
		{
			if(pTmp==p->m_pMonitor)
			{
				p->m_pMonitor = pTmp->pNext;
			}
			else
			{
				pTmp2->pNext = pTmp->pNext;
			}
			free(pTmp);
			break;
		}
		pTmp2 = pTmp;
		pTmp  = pTmp->pNext;
	}
	LeaveCriticalSection(&p->m_hcs);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	return ret/*0*/;
}

int __stdcall NETDVR_SetVideoDecFlag( int nRealPlayHandle, unsigned char bDec )
{
	//参数判断
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	pMonitor->video_rcv.bVideoDecFlag = bDec;
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_OpenRealAudio( int nRealPlayHandle )
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	if (pMonitor->audio_rcv.bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_BUSY;
	}
	int ret;
	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);

// 	waveOutSetVolume(g_m_hwo,0xffffffff);
// 	close_audio_out();
	
	pMonitor->audio_rcv.rcv_type = NETDVR_RCV_AUDIO;
	pMonitor->audio_rcv.p_record_para = &pMonitor->record_para;
	pMonitor->audio_rcv.p_record_para2 = &pMonitor->record_para2;
	pMonitor->audio_rcv.p_frmdecBuf = (unsigned char *)malloc(MAX_AUDIO_DECLEN);
	
	if (NULL == pMonitor->audio_rcv.p_frmdecBuf)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_OUTOFMEMORY;
	}
	
	memset(pMonitor->audio_rcv.p_frmdecBuf, 0, MAX_AUDIO_DECLEN);
	
	u8 audiomode = p->audio_property.audioBitPerSample;
	u16 samplepersec = p->audio_property.audioSamplePerSec;
	pMonitor->audio_rcv.pAudioPlay = new CAudioPlay;
	if (pMonitor->audio_rcv.pAudioPlay)
	{
		if(!pMonitor->audio_rcv.pAudioPlay->Init(audiomode, samplepersec))
		{
// 			delete pMonitor->audio_rcv.pAudioPlay;
// 			pMonitor->audio_rcv.pAudioPlay = NULL;
// 			free(pMonitor->audio_rcv.p_frmdecBuf);
// 			pMonitor->audio_rcv.p_frmdecBuf = NULL;
// 			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
// 			return NETDVR_ERROR;
		}
		
		g_currAudioPlay = pMonitor->audio_rcv.pAudioPlay;
	}
// 	if (!pMonitor->audio_rcv.bPreviewAudioMute)
// 	{
// 		if (!open_audio_out(audiomode, samplepersec))
// 		{
// 			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
// 			return NETDVR_ERROR;
// 		}
// 	}
	
	ret = open_reciever(&pMonitor->audio_rcv,pMonitor->video_rcv.pFrameCB, pMonitor->video_rcv.dwContentRaw, 0);
	if (NETDVR_SUCCESS != ret)
	{
		if (pMonitor->audio_rcv.pAudioPlay)
		{
			delete pMonitor->audio_rcv.pAudioPlay;
			pMonitor->audio_rcv.pAudioPlay = NULL;
		}
// 		close_audio_out();
	}
	
	reg_rcvcb_dec(&pMonitor->audio_rcv, pMonitor->video_rcv.pDecFrameCB, pMonitor->video_rcv.dwContent);

	//开始传输
	
	pMonitor->audio_rcv.req.command = 0;
	pMonitor->audio_rcv.req.Monitor_t.chn = pMonitor->channel;
	pMonitor->audio_rcv.req.Monitor_t.type = 1;
	pMonitor->audio_rcv.p_audio_property = &p->audio_property;
	
	if (p->audio_property.audioEnctype == MEDIA_TYPE_ADPCM_HISI)
	{
		HI_VOICE_DecReset(&(pMonitor->audio_rcv.decaudio_hdr),ADPCM_IMA);
	}
	
#ifdef USE_CONNMSG_THREAD
	u32 command = MAKELONG(0, p->dvr_id);
	if(PostThreadMessage(g_dwConnMsgThreadID, STARTCONNECTMSG, (WPARAM)command, (LPARAM)&pMonitor->audio_rcv))
	{              
		ret = NETDVR_SUCCESS;
	} 
#else
	if (SetRcvTcpFrame(p, &pMonitor->audio_rcv.prcv_t, pMonitor->audio_rcv.req, TRUE, &pMonitor->audio_rcv))
	{
		ret = NETDVR_SUCCESS;
		pMonitor->audio_rcv.bOpened = 1;
	} 
#endif
	
	else
	{
		if (pMonitor->audio_rcv.pAudioPlay)
		{
			delete pMonitor->audio_rcv.pAudioPlay;
			pMonitor->audio_rcv.pAudioPlay = NULL;
		}
		free(pMonitor->audio_rcv.p_frmdecBuf);
		pMonitor->audio_rcv.p_frmdecBuf = NULL;
		ret = NETDVR_ERR_CONNECT;
	}

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	return ret;
}

int __stdcall NETDVR_CloseRealAudio( int nRealPlayHandle )
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret;
	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);

	if (!pMonitor->audio_rcv.bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}
	
	if (pMonitor->audio_rcv.prcv_t)
	{
		u32 id = htonl(pMonitor->audio_rcv.prcv_t->linkid);
		if (!p->b_cmdConnectLost)
		{
#ifndef USE_CONNMSG_THREAD
			char buf[2048]={0};
			ret = send_command(p->cph, CTRL_CMD_STOPAUDIOMONITOR, &id, sizeof(id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
#else
			u32 command = MAKELONG(CTRL_CMD_STOPAUDIOMONITOR, p->dvr_id);
			if(PostThreadMessage(g_dwConnMsgThreadID, STOPCONNECTMSG, (WPARAM)command, (LPARAM)id))
			{
				ret = NETDVR_SUCCESS;
			}
			else
			{
				ret = NETDVR_ERR_SEND;
			}
#endif	
		} 
		else
		{
			ret = NETDVR_ERR_SEND;
		}
	}
	EnterCriticalSection(&p->m_hcs);
	SetRcvTcpFrame(p, &pMonitor->audio_rcv.prcv_t, pMonitor->audio_rcv.req, FALSE);
	pMonitor->audio_rcv.bOpened = 0;

	//清除资源
	ret = close_reciever(&pMonitor->audio_rcv);
// 	close_audio_out();
	if (pMonitor->audio_rcv.pAudioPlay)
	{
		pMonitor->audio_rcv.pAudioPlay->Release();
		delete pMonitor->audio_rcv.pAudioPlay;
		pMonitor->audio_rcv.pAudioPlay = NULL;
	}
	if (pMonitor->audio_rcv.p_frmdecBuf)
	{
		free(pMonitor->audio_rcv.p_frmdecBuf);
		pMonitor->audio_rcv.p_frmdecBuf = NULL;
	}
	LeaveCriticalSection(&p->m_hcs);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret;
}

int __stdcall NETDVR_MuteRealAudio( int nRealPlayHandle, bool bMute )
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	pMonitor->audio_rcv.bPreviewAudioMute = bMute;
	if (!bMute)
	{
		g_currAudioPlay = pMonitor->audio_rcv.pAudioPlay;
	}
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_snapshot( int nRealPlayHandle, char *path, char *filename )
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	LockMutex(g_pool_lock);
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (NULL == pMonitor->video_rcv.p_frmdecBuf)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_OUTOFMEMORY;
	}
	
	LockMutex(pMonitor->snap_shot.snap_lock);
	
	set_default_snap(&pMonitor->snap_shot, p->dvr_id, pMonitor->channel);
	
	/*deal with snapshot directory path*/
	UINT tmp_len;
	if (path)
	{
#ifdef UNICODE
		tmp_len=wcslen(path);
#else
		tmp_len=strlen(path);
#endif
		if (tmp_len && tmp_len < PATH_LEN_MAX)
		{
			if (create_directory(path))
			{
				strcpy(pMonitor->snap_shot.path, path);
			}
		}
	}
	
	if (!create_directory(pMonitor->snap_shot.path))
	{
		UnlockMutex(pMonitor->snap_shot.snap_lock);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}
	
	/*deal with snapshot filename*/
	if (filename)
	{
#ifdef UNICODE
		tmp_len=wcslen(filename);
#else
		tmp_len=strlen(filename);
#endif
		if (tmp_len > 4 && tmp_len <= FILENAME_LEN_MAX)
		{
			char suffix[5];
			memset(suffix, 0, 5);
			memcpy(suffix, filename + tmp_len - 4, 4);
			
			if (0 == _stricmp(suffix, ".bmp"))
			{
				strcpy(pMonitor->snap_shot.filename, filename);
			}
			else if (0 == _stricmp(suffix, ".jpg"))
			{
				pMonitor->snap_shot.pictype = PIC_TYPE_JPG;
				
				strcpy(pMonitor->snap_shot.filename, filename);
			}
		}			
	}
	
	pMonitor->snap_shot.b_snap_on = 1;
	
#if 1  //20100108 cj 抓图移至调用接口处,实时抓图
	if (pMonitor->video_rcv.bVideoDecFlag)
	{
		LockMutex(pMonitor->video_rcv.dec_lock);//csp modify
		
		if(pMonitor->video_rcv.p_frmdecBuf != NULL)
		{
			FrameHeadrDec framehdrdec;
			framehdrdec.mediaType = MEDIA_TYPE_H264;
			framehdrdec.data = (void *)pMonitor->video_rcv.p_frmdecBuf;
			framehdrdec.video_param.fmt = pMonitor->video_rcv.fmt;
			framehdrdec.video_param.width = pMonitor->video_rcv.wCurrFrmWidth;
			framehdrdec.video_param.height = pMonitor->video_rcv.wCurrFrmHeight;
			framehdrdec.data_size = framehdrdec.video_param.width*framehdrdec.video_param.height*3/2;
			
			framehdrdec.reserved1[0] = pMonitor->video_rcv.byDeinterlacing;
			
			if (framehdrdec.reserved1[0])
			{
				//csp modify
				//deinterlace_bottom_field_inplace(pMonitor->video_rcv.p_frmdecBuf,
				//	pMonitor->video_rcv.wCurrFrmWidth,
				//	pMonitor->video_rcv.wCurrFrmWidth,
				//	pMonitor->video_rcv.wCurrFrmHeight);						
			}
			
			do_snapshot(&framehdrdec, &pMonitor->snap_shot);
		}
		else
		{
			UnlockMutex(pMonitor->video_rcv.dec_lock);//csp modify
			
			pMonitor->snap_shot.b_snap_on = 0;
			
			UnlockMutex(pMonitor->snap_shot.snap_lock);
			
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			
			return NETDVR_ERR_OUTOFMEMORY;
		}
		
		UnlockMutex(pMonitor->video_rcv.dec_lock);//csp modify
	}
	
	pMonitor->snap_shot.b_snap_on = 0;
#endif
	
	UnlockMutex(pMonitor->snap_shot.snap_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_startRecord(int nRealPlayHandle, char *p_dir_path, u32 file_max_len)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret;
	
	LockMutex(g_pool_lock);
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	/*deal with record directory path*/
	memset(pMonitor->record_para.path,0,sizeof(pMonitor->record_para.path));
	ret = set_out_path(pMonitor->record_para.path, p_dir_path);
	if (NETDVR_SUCCESS != ret)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return ret;
	}
	
	/*deal with record max file length*/
	if ((file_max_len < REC_FILE_LEN_MIN) || (file_max_len > REC_FILE_LEN_MAX))
	{
		file_max_len = REC_FILE_LEN_MAX;
	}
	

	pMonitor->record_para.max_length = file_max_len;
	pMonitor->record_para.chn = pMonitor->channel;

		
	LockMutex(pMonitor->record_para.cb_rec_lock);
	pMonitor->record_para.p_audio_property = &p->audio_property;

	if (!pMonitor->record_para.p_rec_cb)
	{
		pMonitor->record_para.p_rec_cb = deal_frame_record;
		pMonitor->record_para.dwContentRec = (u32)&pMonitor->record_para;
	}


	UnlockMutex(pMonitor->record_para.cb_rec_lock);

	LockMutex(pMonitor->record_para.cb_filename_lock);
	if (!pMonitor->record_para.p_filename_cb)
	{
		pMonitor->record_para.p_filename_cb = deal_record_filename;
		pMonitor->record_para.dwContentFilename = (u32)&pMonitor->record_para;
	}
	UnlockMutex(pMonitor->record_para.cb_filename_lock);
	
	pMonitor->record_para.b_record_on = 1;
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_stopRecord(int nRealPlayHandle)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(pMonitor->record_para.rec_lock);	
	if (pMonitor->record_para.b_record_on)
	{
		pMonitor->record_para.b_record_on = 0;

		if (pMonitor->record_para.file)
		{
			custommp4_close(pMonitor->record_para.file);
			pMonitor->record_para.file = NULL;
		}
	}
	UnlockMutex(pMonitor->record_para.rec_lock);

	LockMutex(pMonitor->record_para.cb_rec_lock);
	pMonitor->record_para.p_rec_cb = NULL;
	pMonitor->record_para.dwContentRec = 0;
	UnlockMutex(pMonitor->record_para.cb_rec_lock);
	
	LockMutex(pMonitor->record_para.cb_filename_lock);
	pMonitor->record_para.p_filename_cb = NULL;
	pMonitor->record_para.dwContentFilename = 0;
	UnlockMutex(pMonitor->record_para.cb_filename_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_setRecordCB(int nRealPlayHandle, pFrameCallBack pRecordCBFun, u32 dwContent)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(pMonitor->record_para.cb_rec_lock);
	pMonitor->record_para.p_audio_property = &p->audio_property;
	if (pRecordCBFun)
	{
		pMonitor->record_para.p_rec_cb = pRecordCBFun;
		pMonitor->record_para.dwContentRec = dwContent;
	}
	else
	{
		pMonitor->record_para.p_rec_cb = deal_frame_record;
		pMonitor->record_para.dwContentRec = (u32)&pMonitor->record_para;
	}
	
	
	UnlockMutex(pMonitor->record_para.cb_rec_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_setRecordFileNameCB(int nRealPlayHandle, pRecFilenameCallBack pRecFilenameCBFun, unsigned int dwContent)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(pMonitor->record_para.cb_filename_lock);
	if (pRecFilenameCBFun)
	{
		pMonitor->record_para.p_filename_cb = pRecFilenameCBFun;
		pMonitor->record_para.dwContentFilename = dwContent;
	}
	else
	{
		pMonitor->record_para.p_filename_cb = deal_record_filename;
		pMonitor->record_para.dwContentFilename = (u32)&pMonitor->record_para;
	}
	UnlockMutex(pMonitor->record_para.cb_filename_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_startPlayByFile( int Handle, const struct NETDVR_recFileInfo_t *pFileInfo, const struct PlayBackClientInfo_t* pClientinfo, int* pPlayHandle )
{
	//参数判断
	if (!Handle || !pClientinfo || !pPlayHandle)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	int ret;
// 	FILE *fp = fopen("C:\\Startplayfile.txt","ab+");
// 	fprintf(fp,"begin\r\n");
// 	fclose(fp);
	LockMutex(g_pool_lock);
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		*pPlayHandle = 0;
		UnlockMutex(g_pool_lock);
		
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	//分配预览handle内存空间
	ifly_playback_t* pPlayBack = (ifly_playback_t*)malloc(sizeof(ifly_playback_t));
	if (NULL == pPlayBack)
	{
		*pPlayHandle = 0;
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_OUTOFMEMORY;
	}

	//初始化预览handle参数，分配解码buf空间
	memset(pPlayBack, 0, sizeof(ifly_playback_t));
	pPlayBack->pDeviceHandle = p;
	pPlayBack->playtype = 0;

	//插到前面
	pPlayBack->pNext = p->m_pPlayBack;
	p->m_pPlayBack = pPlayBack;

	if (pClientinfo->sendmode == 0)
	{
		pPlayBack->player.pb_rcv.req.command = 1;
	} 
	else if (pClientinfo->sendmode == 1)
	{
		pPlayBack->player.pb_rcv.req.command = 6;
	}
	else
	{
		*pPlayHandle = 0;
		p->m_pPlayBack = p->m_pPlayBack->pNext;
		free(pPlayBack);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}

	CreateMutexHandle(&pPlayBack->player_lock);

	LockMutex(pPlayBack->player_lock);

	open_reciever(&pPlayBack->player.pb_rcv, pClientinfo->pEncFrameCBFunc, pClientinfo->dwEncFrameContent, p->si.deviceIP);

	pPlayBack->player.pb_rcv.rcv_type = NETDVR_RCV_PLAYCB_VIDEO;
	pPlayBack->player.p_cb_playover = pClientinfo->pPlayOverCBFunc;
	pPlayBack->player.dwContentPlayover = pClientinfo->dwPlayOverContent;
	pPlayBack->player.p_cb_progress = pClientinfo->pProgressCBFunc;
	pPlayBack->player.dwContentProgress = pClientinfo->dwProgressContent;
	
	if (pClientinfo->pDecFrameCBFunc)  //需要解码
	{
// 		waveOutSetVolume(g_m_hwo,0xffffffff);
// 		close_audio_out();
		
		//csp modify
		//int nMalloclen = p->video_property.max_videowidth*p->video_property.max_videoheight*2;//MAX_VIDEODECLEN;
		int nMalloclen = ((p->video_property.max_videowidth+15)/16*16) * ((p->video_property.max_videoheight+15)/16*16) * 3 / 2;
		
		pPlayBack->player.pb_rcv.p_frmdecBuf = (unsigned char *)malloc(nMalloclen);
		if (NULL == pPlayBack->player.pb_rcv.p_frmdecBuf)
		{
			*pPlayHandle=0;
			p->m_pPlayBack = p->m_pPlayBack->pNext;
		    
			UnlockMutex(pPlayBack->player_lock);
			free(pPlayBack);
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_OUTOFMEMORY;
		}
		memset(pPlayBack->player.pb_rcv.p_frmdecBuf, 0, nMalloclen);
		
		if (p->audio_property.audioEnctype == MEDIA_TYPE_ADPCM_HISI)
		{
			HI_VOICE_DecReset(&(pPlayBack->player.pb_rcv.decaudio_hdr),ADPCM_IMA);
		}
		
		reg_rcvcb_dec(&pPlayBack->player.pb_rcv, pClientinfo->pDecFrameCBFunc, pClientinfo->dwDecFrameContent);
		
		create_rcvvideo_decoder(&pPlayBack->player.pb_rcv, &p->video_property);
		
		set_rcvvideo_decoder_fmt(&pPlayBack->player.pb_rcv, NETDVR_FMT_YV12);
		
		pPlayBack->player.pb_rcv.bVideoDecFlag = TRUE;
	}
	
	//传输码流
	strcpy(pPlayBack->player.pb_rcv.req.FilePlayBack_t.filename, pFileInfo->filename);
	pPlayBack->player.pb_rcv.req.FilePlayBack_t.offset = htonl(pFileInfo->offset);
	pPlayBack->player.pb_rcv.p_video_property = &p->video_property;
	pPlayBack->player.pb_rcv.p_audio_property = &p->audio_property;
	
	if (SetRcvTcpFrame(p, &pPlayBack->player.pb_rcv.prcv_t, pPlayBack->player.pb_rcv.req, TRUE, &pPlayBack->player.pb_rcv))
	{
		pPlayBack->player.pb_rcv.bOpened = 1;
		pPlayBack->player.m_PlayId = htonl(pPlayBack->player.pb_rcv.prcv_t->linkid);
		//接收进度信息
		u8 flagSend = 1;
		ctrl_player(Handle, pPlayBack->player.m_PlayId, CTRL_CMD_PLAYPROGRESS, (char *)&flagSend, sizeof(flagSend));
		ret = NETDVR_SUCCESS;
		*pPlayHandle = (int)pPlayBack;
	}
	else
	{
		ret = NETDVR_ERR_SEND;
		if (pClientinfo->pDecFrameCBFunc)
		{
			unreg_rcvcb_dec(&pPlayBack->player.pb_rcv);	
			destroy_rcvvideo_decoder(&pPlayBack->player.pb_rcv);
		}
		*pPlayHandle = 0;
		p->m_pPlayBack = p->m_pPlayBack->pNext;
		if (pPlayBack->player.pb_rcv.p_frmdecBuf)
		{
			free(pPlayBack->player.pb_rcv.p_frmdecBuf);
			pPlayBack->player.pb_rcv.p_frmdecBuf = NULL;
		}
	}
	
	UnlockMutex(pPlayBack->player_lock);
	if (ret)
	{
		free(pPlayBack);
	}
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
// 	fp = fopen("C:\\Startplayfile.txt","ab+");
// 	fprintf(fp,"end\r\n");
// 	fclose(fp);
	return ret;
}

int __stdcall NETDVR_stopPlayBack( int nPlayBackHandle )
{
// 	FILE *fp = fopen("C:\\Stopplayfile.txt","ab+");
// 	fprintf(fp,"begin\r\n");
// 	fclose(fp);
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);	
	UnlockMutex(g_pool_lock);
	LockMutex(pPlayBack->player_lock);	
	if (!pPlayBack->player.pb_rcv.bOpened)
	{
		UnlockMutex(pPlayBack->player_lock);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}

	u16 cmd = 0;
	switch (pPlayBack->playtype)
	{
	case 0:
		cmd = CTRL_CMD_STOPFILEPLAY;
		break;
	case 1:
		cmd = CTRL_CMD_STOPTIMEPLAY;
		break;
	}

	int ret = ctrl_player((int)p, pPlayBack->player.m_PlayId, cmd);

	EnterCriticalSection(&p->m_hcs);
	SetRcvTcpFrame(p, &pPlayBack->player.pb_rcv.prcv_t, pPlayBack->player.pb_rcv.req, FALSE);	
	pPlayBack->player.pb_rcv.bOpened = 0;	

	unreg_rcvcb_dec(&pPlayBack->player.pb_rcv);	
	destroy_rcvvideo_decoder(&pPlayBack->player.pb_rcv);	
	close_reciever(&pPlayBack->player.pb_rcv);	
	if (pPlayBack->player.pb_rcv.p_frmdecBuf)
	{
		free(pPlayBack->player.pb_rcv.p_frmdecBuf);
		pPlayBack->player.pb_rcv.p_frmdecBuf = NULL;
	}
	
	UnlockMutex(pPlayBack->player_lock);
	CloseMutexHandle(pPlayBack->player_lock);

	//最后：释放pPlayBack;
	ifly_playback_t* pTmp = p->m_pPlayBack;
	ifly_playback_t* pTmp2 = NULL;
	
	while(pTmp)
	{
		if(pTmp==pPlayBack)
		{
			if(pTmp==p->m_pPlayBack)
			{
				p->m_pPlayBack = pTmp->pNext;
			}
			else
			{
				pTmp2->pNext = pTmp->pNext;
			}
			free(pTmp);
			break;
		}
		pTmp2 = pTmp;
		pTmp  = pTmp->pNext;
	}
	LeaveCriticalSection(&p->m_hcs);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
// 	fp = fopen("C:\\Stopplayfile.txt","ab+");
// 	fprintf(fp,"end\r\n");
// 	fclose(fp);
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_setPlayCBAudioMute( int nPlayBackHandle, bool bMute )
{
	
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(pPlayBack->player_lock);
	
	pPlayBack->player.pb_rcv.bPreviewAudioMute = bMute;
	if (!bMute)
	{
		g_currAudioPlay = pPlayBack->player.pb_rcv.pAudioPlay;
	}
	UnlockMutex(pPlayBack->player_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}




int __stdcall NETDVR_pausePlay(int nPlayBackHandle)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
 	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_PAUSEPLAY);
}

int __stdcall NETDVR_resumePlay(int nPlayBackHandle)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
 	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_RESUMEPLAY);
}

int __stdcall NETDVR_singleFramePlay(int nPlayBackHandle)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
 	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_SINGLEPLAY);
}

int __stdcall NETDVR_fastPlay(int nPlayBackHandle)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
 	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_FASTPLAY);	
}

int __stdcall NETDVR_slowPlay(int nPlayBackHandle)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
 	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_SLOWPLAY);	
}

int __stdcall NETDVR_setPlayRate(int nPlayBackHandle, int play_rate)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	int nRate = play_rate;
	nRate = htonl(nRate);

	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_SETPLAYRATE, (char *)&nRate, sizeof(nRate));
}

int __stdcall NETDVR_playPrevious(int nPlayBackHandle)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
 	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_PLAYPREV);	
}

int __stdcall NETDVR_playNext(int nPlayBackHandle)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
 	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_PLAYNEXT);	
}

int __stdcall NETDVR_playSeek(int nPlayBackHandle, unsigned int new_time)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	printf("seek time: %d\n", new_time);
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	u32 seektime = new_time;
	seektime = htonl(seektime);
	
	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_PLAYSEEK, (char *)&seektime, sizeof(seektime));
}

int __stdcall NETDVR_playMute(int nPlayBackHandle, BOOL b_mute)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	u8 muteFlag = b_mute;
	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_PLAYMUTE, (char *)&muteFlag, sizeof(muteFlag));
}

int __stdcall NETDVR_playProgress(int nPlayBackHandle, BOOL b_send_progress)
{
	if (!ChkPlayBackHandle(nPlayBackHandle)/*NULL == nPlayBackHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	u8 flagSend = b_send_progress;
	return ctrl_player((int)p, pPlayBack->player.m_PlayId, CTRL_CMD_PLAYPROGRESS, (char *)&flagSend, sizeof(flagSend));
}

int __stdcall NETDVR_startPlayByTime( int Handle, const struct NETDVR_TimePlayCond_t *pTimePlayInfo, const struct PlayBackClientInfo_t* pClientinfo, int* pPlayHandle )
{
	//参数判断
	if (!Handle || !pTimePlayInfo || !pPlayHandle)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	int ret;
	
	LockMutex(g_pool_lock);
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		*pPlayHandle = 0;
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);	
	UnlockMutex(g_pool_lock);	
	//分配预览handle内存空间
	ifly_playback_t* pPlayBack = (ifly_playback_t*)malloc(sizeof(ifly_playback_t));
	if (NULL == pPlayBack)
	{
		*pPlayHandle = 0;
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_OUTOFMEMORY;
	}


	//初始化预览handle参数，分配解码buf空间
	memset(pPlayBack, 0, sizeof(ifly_playback_t));
	pPlayBack->pDeviceHandle = p;
	pPlayBack->playtype = 1;

	//插到前面
	pPlayBack->pNext = p->m_pPlayBack;
	p->m_pPlayBack = pPlayBack;

	if (pClientinfo->sendmode == 0)
	{
			pPlayBack->player.pb_rcv.req.command = 2;
	} 
	else if (pClientinfo->sendmode == 1)
	{
		pPlayBack->player.pb_rcv.req.command = 7;
	}
	else
	{
		*pPlayHandle = 0;
		p->m_pPlayBack = p->m_pPlayBack->pNext;
		free(pPlayBack);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_PARAM;
	}
	
	CreateMutexHandle(&pPlayBack->player_lock);
	LockMutex(pPlayBack->player_lock);
	open_reciever(&pPlayBack->player.pb_rcv, pClientinfo->pEncFrameCBFunc, pClientinfo->dwEncFrameContent, p->si.deviceIP);
	pPlayBack->player.p_cb_playover = pClientinfo->pPlayOverCBFunc;
	pPlayBack->player.dwContentPlayover = pClientinfo->dwPlayOverContent;
	pPlayBack->player.p_cb_progress = pClientinfo->pProgressCBFunc;
	pPlayBack->player.dwContentProgress = pClientinfo->dwProgressContent;
	
	if (pClientinfo->pDecFrameCBFunc)  //需要解码
	{
// 		waveOutSetVolume(g_m_hwo,0xffffffff);
// 		close_audio_out();
		
		//csp modify
		//int nMalloclen = p->video_property.max_videowidth*p->video_property.max_videoheight*2;//MAX_VIDEODECLEN;
		int nMalloclen = ((p->video_property.max_videowidth+15)/16*16) * ((p->video_property.max_videoheight+15)/16*16) * 3 / 2;
		
		pPlayBack->player.pb_rcv.p_frmdecBuf = (unsigned char *)malloc(nMalloclen);
		if (NULL == pPlayBack->player.pb_rcv.p_frmdecBuf)
		{
			*pPlayHandle=0;
			p->m_pPlayBack = p->m_pPlayBack->pNext;
			
			UnlockMutex(pPlayBack->player_lock);
			free(pPlayBack);	
			UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
			return NETDVR_ERR_OUTOFMEMORY;
		}
		memset(pPlayBack->player.pb_rcv.p_frmdecBuf, 0, nMalloclen);
		
		if (p->audio_property.audioEnctype == MEDIA_TYPE_ADPCM_HISI)
		{
			HI_VOICE_DecReset(&(pPlayBack->player.pb_rcv.decaudio_hdr),ADPCM_IMA);
		}
		
		reg_rcvcb_dec(&pPlayBack->player.pb_rcv, pClientinfo->pDecFrameCBFunc, pClientinfo->dwDecFrameContent);		
		create_rcvvideo_decoder(&pPlayBack->player.pb_rcv, &p->video_property);	
		set_rcvvideo_decoder_fmt(&pPlayBack->player.pb_rcv, NETDVR_FMT_YV12);
		pPlayBack->player.pb_rcv.bVideoDecFlag = TRUE;
	}
	
	//传输码流
	pPlayBack->player.pb_rcv.req.TimePlayBack_t.channel = pTimePlayInfo->chn;
	pPlayBack->player.pb_rcv.req.TimePlayBack_t.type = htons(pTimePlayInfo->type);
	pPlayBack->player.pb_rcv.req.TimePlayBack_t.start_time = htonl(pTimePlayInfo->start_time);
	pPlayBack->player.pb_rcv.req.TimePlayBack_t.end_time = htonl(pTimePlayInfo->end_time);
	
	//csp modify for NVR
	struct timeb tb;
	ftime(&tb);
	pPlayBack->player.pb_rcv.req.TimePlayBack_t.start_time = htonl(pTimePlayInfo->start_time+tb.timezone*60);
	pPlayBack->player.pb_rcv.req.TimePlayBack_t.end_time = htonl(pTimePlayInfo->end_time+tb.timezone*60);
	
	pPlayBack->player.pb_rcv.rcv_type = NETDVR_RCV_PLAYCB_VIDEO;
	pPlayBack->player.pb_rcv.p_video_property = &p->video_property;
	pPlayBack->player.pb_rcv.p_audio_property = &p->audio_property;
	
	if (SetRcvTcpFrame(p, &pPlayBack->player.pb_rcv.prcv_t, pPlayBack->player.pb_rcv.req, TRUE, &pPlayBack->player.pb_rcv))
	{
		pPlayBack->player.pb_rcv.bOpened = 1;
		pPlayBack->player.m_PlayId = htonl(pPlayBack->player.pb_rcv.prcv_t->linkid);
		u8 flagSend = 1;
		ctrl_player(Handle, pPlayBack->player.m_PlayId, CTRL_CMD_PLAYPROGRESS, (char *)&flagSend, sizeof(flagSend));
		ret = NETDVR_SUCCESS;
		*pPlayHandle = (int)pPlayBack;
	}
	else
	{
		ret = NETDVR_ERR_SEND;
		if (pClientinfo->pDecFrameCBFunc)
		{
			unreg_rcvcb_dec(&pPlayBack->player.pb_rcv);	
			destroy_rcvvideo_decoder(&pPlayBack->player.pb_rcv);
		}
		*pPlayHandle = 0;
		p->m_pPlayBack = p->m_pPlayBack->pNext;
		if (pPlayBack->player.pb_rcv.p_frmdecBuf)
		{
			free(pPlayBack->player.pb_rcv.p_frmdecBuf);
			pPlayBack->player.pb_rcv.p_frmdecBuf = NULL;
		}
	}

	UnlockMutex(pPlayBack->player_lock);
	if (ret)
	{
		free(pPlayBack);
	}
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);		
	return ret;	
}

int __stdcall NETDVR_IsUseUTCTime( int Handle, unsigned char *pFlag )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ret = send_command(p->cph, CTRL_CMD_USEUTCTIME, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);
	if(NETDVR_SUCCESS == ret)
	{
		ifly_utctime_t para_info;
		memset(&para_info,0,sizeof(para_info));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		*pFlag = para_info.bUseUTC;	
	}
	else
	{
		*pFlag = 0;
	}
	return ret;
}

int __stdcall NETDVR_getPresetList( int Handle, unsigned char chn, struct NETDVR_PresetList_t* pList )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};

	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (chn >= p->si.maxChnNum || !pList)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	ifly_preset_list_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETPRESETLIST, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));

		memcpy(pList,&para_info,sizeof(ifly_preset_list_t));
	}
	
	return ret;
}

int __stdcall NETDVR_AddPresetByName( int Handle, const struct NETDVR_PresetName_t *p_para )
{
	int ret = 0;
	char buf[2048] = {0};
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)Handle;
	
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	if (p_para->chn >= p->si.maxChnNum ||p_para->preset >128 ||p_para->preset <1)
	{
		return NETDVR_ERR_PARAM;
	}
	

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_ADDPRESET_BYNAME,p_para,sizeof(NETDVR_PresetName_t),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;	
}

int __stdcall NETDVR_getPTZRate( int Handle, unsigned char chn, struct NETDVR_PTZRate_t* p_para )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (chn >= p->si.maxChnNum || !p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_ptz_rate_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETPTZRATE, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		
		memcpy(p_para,&para_info,sizeof(ifly_ptz_rate_t));
	}
	
	return ret;	
}

int __stdcall NETDVR_resetPicAdjust( int Handle, const struct NETDVR_Reset_Picadjust_t *pPara )
{
	int ret = 0;
	char buf[2048] = {0};
	if (!pPara)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	if (pPara->chn >= p->si.maxChnNum || !pPara)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_reset_picadjust_t para_info;
	memcpy(&para_info, pPara, sizeof(ifly_reset_picadjust_t));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_RESETPICADJUST,&para_info,sizeof(para_info),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;	
}

int __stdcall NETDVR_getFrameRateList( int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_Framerate_list_t* pList )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (chn >= p->si.maxChnNum || !pList /*|| vidoetype>1*/)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_framerate_list_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	u8 tmp[2] = {0};
	tmp[0] = chn;
	tmp[1] = vidoetype;
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETFRAMERATELIST, tmp, sizeof(tmp), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(pList,&para_info,sizeof(ifly_framerate_list_t));
	}
	
	return ret;
}

int __stdcall NETDVR_getVideoResoluList( int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_VideoResolu_list_t* pList )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (chn >= p->si.maxChnNum || !pList || vidoetype>1)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_videoresolu_list_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	u8 tmp[2] = {0};
	tmp[0] = chn;
	tmp[1] = vidoetype;
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_GET_RESOLUTION_LIST, tmp, sizeof(tmp), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(pList,&para_info,sizeof(ifly_videoresolu_list_t));
	}
	
	return ret;	
}

int __stdcall NETDVR_getMaxIMGMaskNum( int Handle, unsigned char* pNum )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!pNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETMAX_IMGMASKNUM, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(pNum,buf+sizeof(ifly_cp_header_t),sizeof(BYTE));
	}
	
	return ret;	
	
}

int __stdcall NETDVR_setNRServer(int Handle, const struct NETDVR_NRServer_t *p_para)
{
	
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_nrserver_t para_info;
	memset(&para_info,0,sizeof(ifly_nrserver_t));
	para_info.nrserverip = p_para->nrserverip;
	para_info.serverport = htons(p_para->serverport);
	para_info.databindport=htons(p_para->databindport);
	memcpy(para_info.reserved,p_para->reserved,sizeof(p_para->reserved));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_NRSERVER_SET, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;	
}

int __stdcall NETDVR_getNRServer( int Handle, NETDVR_NRServer_t * p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_nrserver_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_NRSERVER_GET, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->nrserverip	= para_info.nrserverip;
		p_para->serverport	= ntohs(para_info.serverport);
		p_para->databindport= ntohs(para_info.databindport);
		memcpy(p_para->reserved,para_info.reserved,sizeof(para_info.reserved));
	}
	
	return ret;	
}

//报警
int __stdcall NETDVR_setAlarmOutVal(int Handle, const struct NETDVR_AlarmVal_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para || p_para->alarmid >= p->si.maxAlarmOutNum)
	{
//		printf("p->si.maxAlarmOutNum = %d,p_para->alarmid = %d\n",p->si.maxAlarmOutNum,p_para->alarmid);
		return NETDVR_ERR_PARAM;
	}
	
//	printf("p->si.maxAlarmOutNum = %d,p_para->alarmid = %d\n",p->si.maxAlarmOutNum,p_para->alarmid);
	ifly_alarm_val_t para_info;
	memset(&para_info,0,sizeof(ifly_alarm_val_t));
	para_info.alarmid = p_para->alarmid;
	para_info.val = p_para->val;
	memcpy(para_info.reserved,p_para->reserved,sizeof(p_para->reserved));	
	
	ret = send_command(p->cph, CTRL_CMD_SET_ALARMOUT_VAL, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	
	return  ret;
}

int __stdcall NETDVR_getAlarmInVal( int Handle, unsigned char alarm_in_id,struct NETDVR_AlarmVal_t * p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (!p_para || alarm_in_id >= p->si.maxAlarmInNum)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_alarm_val_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	ret = send_command(p->cph, CTRL_CMD_GET_ALARMIN_VAL, &alarm_in_id, sizeof(alarm_in_id), buf, sizeof(buf), 		g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->alarmid	= para_info.alarmid;
		p_para->val	= para_info.val;
		memcpy(p_para->reserved,para_info.reserved,sizeof(para_info.reserved));
	}
	
	return ret;
}

int __stdcall NETDVR_startTimeDownload(int Handle, const TimeDownloadInfo_t *pDownloadInfo, int* pDownloadHandle)
{
	if (!Handle || !pDownloadInfo || !pDownloadHandle)
	{
		return NETDVR_ERR_PARAM;
	}

	struct NETDVR_INNER_t *p;
	int ret = 0;

	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		*pDownloadHandle=0;
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		*pDownloadHandle=0;
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}

	IFLY_FileReciever_t *filereciever=(IFLY_FileReciever_t*)malloc(sizeof(IFLY_FileReciever_t));
	if (NULL==filereciever)
	{
		*pDownloadHandle = 0;
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_OUTOFMEMORY;
	}

	memset(filereciever,0,sizeof(IFLY_FileReciever_t));
	filereciever->pDeviceHandle=p;

	filereciever->pNext = p->filereciever;
	p->filereciever = filereciever;

    CreateMutexHandle(&filereciever->reciever_lock);
	LockMutex(filereciever->reciever_lock);	

	filereciever->p_cb_err=pDownloadInfo->p_cb_err;
	filereciever->dwContentErr=pDownloadInfo->dwErrContent;
	filereciever->p_cb_timedlprogress = pDownloadInfo->p_cb_progress;
	filereciever->dwContentProgress = pDownloadInfo->dwProgressContent;
	filereciever->p_cb_save=pDownloadInfo->p_cb_save;
	filereciever->dwContentSav=pDownloadInfo->dwSaveContent;
	
	if (filereciever->bOpened)
	{
		*pDownloadHandle=0;
		p->filereciever=p->filereciever->pNext;
		free(filereciever);
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_BUSY;
	}
	
	filereciever->req.command = 12;
	filereciever->req.MultiTimePlayBack_t.chnMask=htons(pDownloadInfo->rcv_chn);
	filereciever->req.MultiTimePlayBack_t.chn17to32Mask=htons(pDownloadInfo->rcv_chn17to32);
	filereciever->req.MultiTimePlayBack_t.type=htons(pDownloadInfo->streamtype);
	filereciever->req.MultiTimePlayBack_t.start_time=htonl(pDownloadInfo->startTime);
	filereciever->req.MultiTimePlayBack_t.end_time=htonl(pDownloadInfo->endTime);
	
	//csp modify for NVR
	struct timeb tb;
	ftime(&tb);
	filereciever->req.MultiTimePlayBack_t.start_time = htonl(pDownloadInfo->startTime+tb.timezone*60);
	filereciever->req.MultiTimePlayBack_t.end_time = htonl(pDownloadInfo->endTime+tb.timezone*60);
	
	filereciever->p_audio_property = &p->audio_property;
	
	if (SetRcvTcpFrame(p, &filereciever->prcv_t, filereciever->req,TRUE))
	{
		filereciever->reciever_handle = CreateThread(NULL, 0, TimeDownLoadThread, filereciever, 0 , NULL);
		if (!filereciever->reciever_handle)
		{
			*pDownloadHandle=0;
			p->filereciever=p->filereciever->pNext;
			free(filereciever);
			ret = NETDVR_ERR_OUTOFMEMORY;
		}
		else
		{
			filereciever->bOpened = 1;
			ret = NETDVR_SUCCESS;
			*pDownloadHandle=(int)filereciever;
		}
	} 
	else
	{
		ret = NETDVR_ERR_CONNECT;
		*pDownloadHandle = 0;
		p->filereciever=p->filereciever->pNext;
		free(filereciever);
	}

	UnlockMutex(filereciever->reciever_lock);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	return ret;
}

int __stdcall NETDVR_stopTimeDownload(int nDownloadHandle)
{
	if (NULL==nDownloadHandle)
	{
		return NETDVR_ERR_PARAM;
	}
	IFLY_FileReciever_t* filereciever = (IFLY_FileReciever_t*)nDownloadHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)filereciever->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	LockMutex(g_pool_lock);		
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!filereciever->bOpened)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_SUCCESS;
	}
	
	LockMutex(filereciever->reciever_lock);
	
	if (filereciever->prcv_t)
	{
		char buf[2048] = {0};
		u32 id = htonl(filereciever->prcv_t->linkid);
		if (!p->b_cmdConnectLost)
		{
			send_command(p->cph, CTRL_CMD_STOPGETFILEBYTIME, &id, sizeof(id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
		} 
		else
		{
			int ret = NETDVR_ERR_SEND;
		}
		
	}
	filereciever->rcv_size = 0;
	filereciever->recieved_size = 0;
	
	SetRcvTcpFrame(p, &filereciever->prcv_t, filereciever->req, FALSE);
	if (filereciever->reciever_handle)
	{
		WaitForSingleObject(filereciever->reciever_handle, INFINITE);
		CloseHandle(filereciever->reciever_handle);
		filereciever->reciever_handle = NULL;
	}
	filereciever->bOpened = 0;
	UnlockMutex(filereciever->reciever_lock);
	CloseMutexHandle(filereciever->reciever_lock);
	//最后：释放pPlayBack;
	IFLY_FileReciever_t*  pTmp = p->filereciever;
	IFLY_FileReciever_t* pTmp2 = NULL;
	
	while(pTmp)
	{
		if(pTmp==filereciever)
		{
			if(pTmp==p->filereciever)
			{
				p->filereciever = pTmp->pNext;
			}
			else
			{
				pTmp2->pNext = pTmp->pNext;
			}
			free(pTmp);
			break;
		}
		pTmp2 = pTmp;
		pTmp  = pTmp->pNext;
	}
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
	
}

int __stdcall NETDVR_getAlarmUploadState( int Handle, unsigned char id, unsigned char type, NETDVR_AlarmUploadState_t* pPara )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (/*id >= p->si.maxChnNum ||*/ !pPara || type>8)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_alarmuploadstate_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	u8 tmp[2] = {0};
	tmp[0] = id;
	tmp[1] = type;
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETALARMULSTATE, tmp, sizeof(tmp), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		pPara->id = para_info.id;
		pPara->state = para_info.state;
		pPara->type = para_info.type;
		pPara->reserved1 = ntohs(para_info.reserved1);
		pPara->reserved2 = ntohl(para_info.reserved2);
	}
	
	return ret;
}

int __stdcall NETDVR_getSpecialinfo_t(int Handle, NETDVR_specialinfo_t* pPara)
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (NULL == p->cph)
	{
		return NETDVR_ERROR;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GET_SPECIALDEVICEINFO, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);	
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_specialinfo_t para_info;
		memset(&para_info,0,sizeof(ifly_specialinfo_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_specialinfo_t));
		pPara->characterset = para_info.characterset;
		pPara->mainbordmode = para_info.mainbordmode;
		memcpy(pPara->reserved, para_info.reserved,sizeof(para_info.reserved));
	}
	
	return ret;
}

int __stdcall NETDVR_FindMotion_Info( int Handle, const NETDVR_MOTIONCOND* pMDCond, int* lpFindHandle )
{
	if (!pMDCond || !lpFindHandle)
	{
		return NETDVR_ERR_PARAM;
	}

	if (pMDCond->dwstopTime < pMDCond->dwstartTime)
	{
		*lpFindHandle = 0;
		return NETDVR_ERR_PARAM;
	}

	struct NETDVR_INNER_t *p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		*lpFindHandle = 0;
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		*lpFindHandle = 0;
		return NETDVR_ERR_NOLOGIN;
	}

	if (p->b_cmdConnectLost)
	{
		*lpFindHandle = 0;
		return NETDVR_ERR_SEND;
	}
	
	//去掉高位的非法值
	u8 offset = 32 - p->si.maxChnNum;
	if (offset <0)
	{
		//....
		//目前暂不支持32路以上的
		*lpFindHandle = 0;
		return NETDVR_ERR_UNKNOWN;
	}

	u32 chn32Mask = (pMDCond->channelmask<<offset)>>offset;

	if (0 == chn32Mask)
	{
		*lpFindHandle = 0;
		return NETDVR_ERR_PARAM;
	}

	char ackbuf[2048] = {0};
	ifly_recsearch_param_t searchpara;
	memset(&searchpara,0,sizeof(ifly_recsearch_param_t));

	searchpara.channel_mask = htons(LOWORD(chn32Mask));
	searchpara.chn17to32mask = htons(HIWORD(chn32Mask));
	searchpara.startID = htons(1);
	searchpara.max_return = htons(MAX_RECFIND);
	
	searchpara.start_time = htonl(pMDCond->dwstartTime);
	searchpara.end_time = htonl(pMDCond->dwstopTime);
	
	searchpara.type_mask = htons(NETDVR_REC_INDEX_MD);
	
	int ret = send_command(p->cph, CTRL_CMD_RECFILESEARCH, &searchpara, sizeof(ifly_recsearch_param_t), ackbuf, sizeof(ackbuf), g_connect_timeout);
	if (NETDVR_SUCCESS == ret)
	{
		struct PRI_MotionFind_t *pMDFindHandle = NULL;
		pMDFindHandle = (PRI_MotionFind_t *)malloc(sizeof(PRI_MotionFind_t));
		if (NULL == pMDFindHandle)
		{
			*lpFindHandle = 0;
			return NETDVR_ERR_OUTOFMEMORY;
		}

		memset(pMDFindHandle, 0, sizeof(PRI_MotionFind_t));

		strcpy(pMDFindHandle->flag, "mdfind");
		strcpy(pMDFindHandle->tRecInfo.flag, "recfind");
		pMDFindHandle->tRecInfo.pDeviceHandle = p;

		memcpy(&pMDFindHandle->tRecInfo.findcond, &searchpara, sizeof(ifly_recsearch_param_t));
		
		ifly_search_desc_t searchdesc;
		memset(&searchdesc,0,sizeof(ifly_search_desc_t));

		char *ptmp = ackbuf+sizeof(ifly_cp_header_t);
		memcpy(&searchdesc,ptmp,sizeof(ifly_search_desc_t));
		pMDFindHandle->tRecInfo.endID = ntohs(searchdesc.endID);
		pMDFindHandle->tRecInfo.startID = ntohs(searchdesc.startID);
		pMDFindHandle->tRecInfo.sum = ntohs(searchdesc.sum);
		
		ptmp += sizeof(ifly_search_desc_t);
		
		for (int i = 0; i < min(ntohs(searchpara.max_return), pMDFindHandle->tRecInfo.sum); i++)
		{
			ifly_recfileinfo_t fileinfo;
			memset(&fileinfo,0,sizeof(ifly_recfileinfo_t));

			memcpy(&fileinfo,ptmp,sizeof(ifly_recfileinfo_t));
			ptmp += sizeof(ifly_recfileinfo_t);
			
			pMDFindHandle->tRecInfo.finddatabuf[i].channel_no = fileinfo.channel_no; 
					
			pMDFindHandle->tRecInfo.finddatabuf[i].start_time = ntohl(fileinfo.start_time); 
			
			pMDFindHandle->tRecInfo.finddatabuf[i].end_time = ntohl(fileinfo.end_time);
			
			pMDFindHandle->tRecInfo.finddatabuf[i].offset = ntohl(fileinfo.offset); 
			pMDFindHandle->tRecInfo.finddatabuf[i].size = ntohl(fileinfo.size); 
			pMDFindHandle->tRecInfo.finddatabuf[i].type = fileinfo.type; 
			strcpy(pMDFindHandle->tRecInfo.finddatabuf[i].filename, fileinfo.filename); 
		}
		*lpFindHandle = (int)pMDFindHandle;
		
		//插到前面
		pMDFindHandle->pNext = p->m_pMDList;
		p->m_pMDList = pMDFindHandle;

		return NETDVR_SUCCESS;
	}
	else
	{
		*lpFindHandle = 0;
		return ret;
	}
}

int __stdcall NETDVR_GetNextMotionInfo( int lFindHandle, NETDVR_MOTION_DATA* lpMotiondData )
{
	if (!lFindHandle || !lpMotiondData)
	{
		return NETDVR_ERR_PARAM;
	}

	//check lFindHandle flag
	char tmp[8] = {0};
	strncpy(tmp, (char *)lFindHandle, 7);
	if (strcmp(tmp, "mdfind"))
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct PRI_MotionFind_t *pMDFindHandle = (PRI_MotionFind_t *)lFindHandle;
	
	int i=0;
	for (i=0; i< g_dvr_pool.count; i++)
	{
		if (pMDFindHandle->tRecInfo.pDeviceHandle == g_dvr_pool.p_dvr[i])
		{
			break;
		}
	}
	
	if (i == g_dvr_pool.count)
	{
		memset(lpMotiondData, 0, sizeof(NETDVR_MOTION_DATA));
		return NETDVR_ERR_NOINIT;
	}

	if (pMDFindHandle->tRecInfo.pDeviceHandle->b_cmdConnectLost)
	{
		memset(lpMotiondData, 0, sizeof(NETDVR_MOTION_DATA));
		return NETDVR_ERR_SEND;
	}

	if (pMDFindHandle->tRecInfo.sum == 0)
	{
		memset(lpMotiondData, 0, sizeof(NETDVR_MOTION_DATA));
		return NETDVR_SUCCESS;
	}
	
	if (pMDFindHandle->tRecInfo.currentfileindex >= pMDFindHandle->tRecInfo.sum)
	{
		memset(lpMotiondData, 0, sizeof(NETDVR_MOTION_DATA));
		return NETDVR_SUCCESS;
	}
	
	int nCurrfile = pMDFindHandle->tRecInfo.currentfileindex%MAX_RECFIND;

	if (0 == pMDFindHandle->dwCurrFileMotionTime)
	{
		pMDFindHandle->dwCurrFileMotionTime = pMDFindHandle->tRecInfo.finddatabuf[nCurrfile].start_time;
	}

	unsigned int currTime = pMDFindHandle->dwCurrFileMotionTime;

	//先看是否在当前文件里
	if (currTime < pMDFindHandle->tRecInfo.finddatabuf[nCurrfile].end_time)
	{
		lpMotiondData->dwSecondNum  = min(MAX_SECONDS, (pMDFindHandle->tRecInfo.finddatabuf[nCurrfile].end_time - currTime));
		
		for (int i = 0; i< lpMotiondData->dwSecondNum; i++)
		{
			lpMotiondData->tMotionData[i].byChn =  pMDFindHandle->tRecInfo.finddatabuf[nCurrfile].channel_no;
			lpMotiondData->tMotionData[i].dwMotionNum = 1;
			lpMotiondData->tMotionData[i].dwMotionTime = pMDFindHandle->dwCurrFileMotionTime+i+1;
		}

		pMDFindHandle->dwCurrFileMotionTime += lpMotiondData->dwSecondNum;
		return NETDVR_SUCCESS;
	}

	//找下一个文件，如果此次搜的MAX_RECFIND个用完了，再搜一次
	if (pMDFindHandle->tRecInfo.currentfileindex < pMDFindHandle->tRecInfo.endID)
	{
		pMDFindHandle->tRecInfo.currentfileindex++;
	}
	else
	{
		char ackbuf[2048] = {0};
	
		//其余条件不变，只更新startid
		pMDFindHandle->tRecInfo.findcond.startID = ntohs(pMDFindHandle->tRecInfo.currentfileindex+1);
	
		int ret = send_command(pMDFindHandle->tRecInfo.pDeviceHandle->cph, CTRL_CMD_RECFILESEARCH, &pMDFindHandle->tRecInfo.findcond, sizeof(ifly_recsearch_param_t), ackbuf, sizeof(ackbuf), g_connect_timeout);
		if (NETDVR_SUCCESS == ret)
		{
			ifly_search_desc_t searchdesc;
			memset(&searchdesc,0,sizeof(ifly_search_desc_t));

			char *ptmp = ackbuf+sizeof(ifly_cp_header_t);
			memcpy(&searchdesc,ptmp,sizeof(ifly_search_desc_t));

			pMDFindHandle->tRecInfo.endID = ntohs(searchdesc.endID);
			pMDFindHandle->tRecInfo.startID = ntohs(searchdesc.startID);
			pMDFindHandle->tRecInfo.sum = ntohs(searchdesc.sum);
			
			ptmp += sizeof(ifly_search_desc_t);
			
			for (int i = (pMDFindHandle->tRecInfo.currentfileindex%MAX_RECFIND); 
				i < min(ntohs(pMDFindHandle->tRecInfo.findcond.max_return), (pMDFindHandle->tRecInfo.sum - pMDFindHandle->tRecInfo.currentfileindex)); 
				i++)
			{
				ifly_recfileinfo_t fileinfo;
				memset(&fileinfo,0,sizeof(ifly_recfileinfo_t));
				memcpy(&fileinfo,ptmp,sizeof(ifly_recfileinfo_t));
				ptmp += sizeof(ifly_recfileinfo_t);
				
				pMDFindHandle->tRecInfo.finddatabuf[i].channel_no = fileinfo.channel_no; 
				
				pMDFindHandle->tRecInfo.finddatabuf[i].start_time = ntohl(fileinfo.start_time); 				
				pMDFindHandle->tRecInfo.finddatabuf[i].end_time = ntohl(fileinfo.end_time);				
				pMDFindHandle->tRecInfo.finddatabuf[i].offset = ntohl(fileinfo.offset); 
				pMDFindHandle->tRecInfo.finddatabuf[i].size = ntohl(fileinfo.size); 
				pMDFindHandle->tRecInfo.finddatabuf[i].type = fileinfo.type; 
				strcpy(pMDFindHandle->tRecInfo.finddatabuf[i].filename, fileinfo.filename); 
			}

		}
		else
		{
			memset(lpMotiondData, 0, sizeof(NETDVR_MOTION_DATA));
			return ret;
		}
		
	}
	
	//在更新的文件中查找motion
	nCurrfile = pMDFindHandle->tRecInfo.currentfileindex%MAX_RECFIND;
	pMDFindHandle->dwCurrFileMotionTime = pMDFindHandle->tRecInfo.finddatabuf[nCurrfile].start_time;
	lpMotiondData->dwSecondNum  = min(MAX_SECONDS, (pMDFindHandle->tRecInfo.finddatabuf[nCurrfile].end_time - pMDFindHandle->tRecInfo.finddatabuf[nCurrfile].start_time));
	
	for (i = 0; i< lpMotiondData->dwSecondNum; i++)
	{
		lpMotiondData->tMotionData[i].byChn =  pMDFindHandle->tRecInfo.finddatabuf[nCurrfile].channel_no;
		lpMotiondData->tMotionData[i].dwMotionNum = 1;
		lpMotiondData->tMotionData[i].dwMotionTime = pMDFindHandle->dwCurrFileMotionTime+i+1;
	}
	
	pMDFindHandle->dwCurrFileMotionTime += lpMotiondData->dwSecondNum;

	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_FindMotionClose( int lFindHandle )
{
	if (!lFindHandle)
	{
		return NETDVR_ERR_PARAM;
	}
	
	//check lFindHandle flag
	char tmp[8] = {0};
	strncpy(tmp, (char *)lFindHandle, 7);
	if (strcmp(tmp, "mdfind"))
	{
		return NETDVR_ERR_PARAM;
	}

	struct PRI_MotionFind_t *pMDFindHandle = (PRI_MotionFind_t *)lFindHandle;

	PRI_MotionFind_t* pTmp = pMDFindHandle->tRecInfo.pDeviceHandle->m_pMDList;
	PRI_MotionFind_t* pTmp2 = NULL;
	while(pTmp)
	{
		if(pTmp==pMDFindHandle)
		{
			if(pTmp==pMDFindHandle->tRecInfo.pDeviceHandle->m_pMDList)
			{
				pMDFindHandle->tRecInfo.pDeviceHandle->m_pMDList = pTmp->pNext;
			}
			else
			{
				pTmp2->pNext = pTmp->pNext;
			}
			free(pMDFindHandle);
			break;
		}
		pTmp2 = pTmp;
		pTmp  = pTmp->pNext;
	}
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_MakeKeyFrame(int Handle, NETDVR_Makekeyframe_t* pPara)
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (NULL == p->cph)
	{
		return NETDVR_ERROR;
	}
	
	if (pPara->chn>= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}

	int ret = 0;
	char buf[2048] = {0};
	ifly_makekeyframe_t para_info;
	memset(&para_info,0,sizeof(ifly_makekeyframe_t));
	para_info.chn = pPara->chn;
	para_info.type = pPara->type;
	memcpy(para_info.reserved,pPara->reserved,sizeof(pPara->reserved));
	if (!p->b_cmdConnectLost)
	{
		ret =send_command(p->cph, CTRL_CMD_MAKE_KEYFRAME, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

//获得创世平台信息
int __stdcall NETDVR_getCrearSvrInfo(int Handle, struct NETDVR_CrearSvr_Info_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_crearo_svr_info para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	ret = send_command(p->cph, CTRL_CMD_GET_CREAROSVR_INFO,NULL,0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->flagPlatform	= para_info.flagPlatform;
		p_para->ipPlatform	= para_info.ipPlatform;
		p_para->portPlatform	= ntohs(para_info.portPlatform);
		memcpy(p_para->PUID, para_info.PUID,sizeof(para_info.PUID));
		memcpy(p_para->passwd, para_info.passwd,sizeof(para_info.passwd));
		memcpy(p_para->reserved,para_info.reserved,sizeof(para_info.reserved));
	}
	
	return ret;	
}

//设置创世平台信息
int __stdcall NETDVR_setCrearSvrInfo(int Handle, const struct NETDVR_CrearSvr_Info_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_crearo_svr_info para_info;
	memset(&para_info,0,sizeof(ifly_crearo_svr_info));
	para_info.flagPlatform = p_para->flagPlatform;
	para_info.ipPlatform = htonl(p_para->ipPlatform);
	para_info.portPlatform = htons(p_para->portPlatform);
	memcpy(para_info.PUID,p_para->PUID,sizeof(p_para->PUID));	
	memcpy(para_info.passwd,p_para->passwd,sizeof(p_para->passwd));	
	memcpy(para_info.reserved,p_para->reserved,sizeof(p_para->reserved));	
	
	ret = send_command(p->cph, CTRL_CMD_SET_CREAROSVR_INFO, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	
	return  ret;
}

int __stdcall NETDVR_startRecord2(int nRealPlayHandle, char *p_dir_path, u32 file_max_len)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret;
	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);

	/*deal with record directory path*/
	memset(pMonitor->record_para2.path,0,sizeof(pMonitor->record_para2.path));
	ret = set_out_path(pMonitor->record_para2.path, p_dir_path);
	if (NETDVR_SUCCESS != ret)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return ret;
	}
	
	/*deal with record max file length*/
	if ((file_max_len < REC_FILE_LEN_MIN) || (file_max_len > REC_FILE_LEN_MAX))
	{
		file_max_len = REC_FILE_LEN_MAX;
	}
	

	pMonitor->record_para2.max_length = file_max_len;
	pMonitor->record_para2.chn = pMonitor->channel;

		
	LockMutex(pMonitor->record_para2.cb_rec_lock);
	pMonitor->record_para2.p_audio_property = &p->audio_property;

	if (!pMonitor->record_para2.p_rec_cb)
	{
		pMonitor->record_para2.p_rec_cb = deal_frame_record;
		pMonitor->record_para2.dwContentRec = (u32)&pMonitor->record_para2;
	}


	UnlockMutex(pMonitor->record_para2.cb_rec_lock);

	LockMutex(pMonitor->record_para2.cb_filename_lock);
	if (!pMonitor->record_para2.p_filename_cb)
	{
		pMonitor->record_para2.p_filename_cb = deal_record_filename;
		pMonitor->record_para2.dwContentFilename = (u32)&pMonitor->record_para;
	}
	UnlockMutex(pMonitor->record_para2.cb_filename_lock);

	pMonitor->record_para2.b_record_on = 1;
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_stopRecord2(int nRealPlayHandle)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(pMonitor->record_para2.rec_lock);	
	if (pMonitor->record_para2.b_record_on)
	{
		pMonitor->record_para2.b_record_on = 0;

		if (pMonitor->record_para2.file)
		{
			custommp4_close(pMonitor->record_para2.file);
			pMonitor->record_para2.file = NULL;
		}
	}
	UnlockMutex(pMonitor->record_para2.rec_lock);

	LockMutex(pMonitor->record_para2.cb_rec_lock);
	pMonitor->record_para2.p_rec_cb = NULL;
	pMonitor->record_para2.dwContentRec = 0;
	UnlockMutex(pMonitor->record_para2.cb_rec_lock);
	
	LockMutex(pMonitor->record_para2.cb_filename_lock);
	pMonitor->record_para2.p_filename_cb = NULL;
	pMonitor->record_para2.dwContentFilename = 0;
	UnlockMutex(pMonitor->record_para2.cb_filename_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_setRecord2CB(int nRealPlayHandle, pFrameCallBack pRecordCBFun, u32 dwContent)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}

	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(pMonitor->record_para2.cb_rec_lock);
	pMonitor->record_para2.p_audio_property = &p->audio_property;
	if (pRecordCBFun)
	{
		pMonitor->record_para2.p_rec_cb = pRecordCBFun;
		pMonitor->record_para2.dwContentRec = dwContent;
	}
	else
	{
		pMonitor->record_para2.p_rec_cb = deal_frame_record;
		pMonitor->record_para2.dwContentRec = (u32)&pMonitor->record_para;
	}
	
	
	UnlockMutex(pMonitor->record_para2.cb_rec_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_setRecord2FileNameCB(int nRealPlayHandle, pRecFilenameCallBack pRecFilenameCBFun, unsigned int dwContent)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	LockMutex(g_pool_lock);
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	LockMutex(pMonitor->record_para2.cb_filename_lock);
	if (pRecFilenameCBFun)
	{
		pMonitor->record_para2.p_filename_cb = pRecFilenameCBFun;
		pMonitor->record_para2.dwContentFilename = dwContent;
	}
	else
	{
		pMonitor->record_para2.p_filename_cb = deal_record_filename;
		pMonitor->record_para2.dwContentFilename = (u32)&pMonitor->record_para;
	}
	UnlockMutex(pMonitor->record_para2.cb_filename_lock);
	
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;
}

int __stdcall NETDVR_GetRecord2State(int nRealPlayHandle, unsigned char* pState)
{
	if (!ChkRealHandle(nRealPlayHandle)/*NULL == nRealPlayHandle*/)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealPlayHandle;
	
	NETDVR_INNER_t* p = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == p)
	{
		return NETDVR_ERR_PARAM;
	}
	
	LockMutex(g_pool_lock);
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (!p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	LockMutex(pMonitor->record_para2.rec_lock);	
	
	*pState	= pMonitor->record_para2.b_record_on;
	
	UnlockMutex(pMonitor->record_para2.rec_lock);
	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return NETDVR_SUCCESS;		
}

int __stdcall NETDVR_RegistDdns( int Handle, const struct NETDVR_DDNSinfo_t *pPara )
{
	int ret = 0;
	char buf[2048] = {0};
	if (!pPara)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_ddnsinfo_t para_info;
	memcpy(&para_info, pPara, sizeof(ifly_ddnsinfo_t));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_REGISTDDNS,&para_info,sizeof(para_info),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;	
}

int __stdcall NETDVR_CancelDdns( int Handle, const struct NETDVR_DDNSinfo_t *pPara )
{
	int ret = 0;
	char buf[2048] = {0};
	if (!pPara)
	{
		return NETDVR_ERR_PARAM;
	}
	
	struct NETDVR_INNER_t *p;
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_ddnsinfo_t para_info;
	memcpy(&para_info, pPara, sizeof(ifly_ddnsinfo_t));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph,CTRL_CMD_CANCELDDNS,&para_info,sizeof(para_info),buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;	
}

int __stdcall NETDVR_GetAlarmSCH(int Handle, unsigned char chn, enum NETDVR_WEEKDAY day, struct NETDVR_AlarmSCH_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};

	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_AlarmSCH_t para_info;
	memset(&para_info,0,sizeof(ifly_AlarmSCH_t));

	char tempBuf[2] = {0};
	tempBuf[0] = chn;
	tempBuf[1] = day;

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETALARMSCHPARAM, tempBuf, sizeof(tempBuf), buf, sizeof(buf), CTRL_PROTOCOL_CONNECT_DEFAULT * 5);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->chn = para_info.chn;
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);
		p_para->copy2Weekday = para_info.copy2Weekday;
		for (int j = 0; j < 4; j++)
		{
			p_para->alarmTimeFieldt[j].endtime = ntohl(para_info.TimeFiled[j].endtime);
			p_para->alarmTimeFieldt[j].starttime = ntohl(para_info.TimeFiled[j].starttime);
			p_para->alarmTimeFieldt[j].flag_alarm = para_info.TimeFiled[j].flag_alarm;
		}
		p_para->weekday = (enum NETDVR_WEEKDAY)para_info.weekday;

	}

	return ret;
}

int __stdcall NETDVR_SetAlarmSCH(int Handle, const struct NETDVR_AlarmSCH_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	ifly_AlarmSCH_t para_info;
	memset(&para_info,0,sizeof(ifly_AlarmSCH_t));
	para_info.chn =p_para->chn;
	para_info.copy2Chnmask = htonl(p_para->copy2Chnmask);
	para_info.copy2Weekday = p_para->copy2Weekday;
	for (int i = 0;i<4;i++)
	{
		para_info.TimeFiled[i].endtime = htonl(p_para->alarmTimeFieldt[i].endtime);
		para_info.TimeFiled[i].starttime = htonl(p_para->alarmTimeFieldt[i].starttime);
		para_info.TimeFiled[i].flag_alarm = p_para->alarmTimeFieldt[i].flag_alarm;
	}
	para_info.weekday = p_para->weekday;

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETALARMSCHPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	

	return ret;
} 

int __stdcall NETDVR_getMDSenseList( int Handle, NETDVR_MDSenselist_t* p_para )
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETMDSENSELIST, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_MDSenselist_t para_info;
		memset(&para_info,0,sizeof(ifly_MDSenselist_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_MDSenselist_t));
		memcpy(p_para, &para_info, sizeof(ifly_MDSenselist_t));
	}
	
	return ret;		
}

int __stdcall NETDVR_getMDAlarmDelayList( int Handle, NETDVR_MDAlarmDelaylist_t* p_para )
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETMDALARMDELAYLIST, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_MDAlarmDelaylist_t para_info;
		memset(&para_info,0,sizeof(ifly_MDAlarmDelaylist_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_MDAlarmDelaylist_t));
		for (int i = 0; i< 16; i++)
		{
			p_para->mdalarmdelaylist[i] = ntohs(para_info.mdalarmdelaylist[i]);//csp modify
		}
	}
	
	return ret;
}

int __stdcall NETDVR_getBaudRateList( int Handle, NETDVR_BaudRateList_t* p_para )
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETBAUDRATELIST, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_BaudRateList_t para_info;
		memset(&para_info,0,sizeof(ifly_BaudRateList_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_BaudRateList_t));
		for (int i = 0; i< 16; i++)
		{
			p_para->baudratelist[i] = ntohl(para_info.baudratelist[i]);//csp modify
		}
	}
	
	return ret;		
}

int __stdcall NETDVR_getPTZProtocolList( int Handle, NETDVR_PTZProtocolList_t* p_para )
{
	struct NETDVR_INNER_t *p;
	p = (struct NETDVR_INNER_t *)(Handle); 
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	int ret = 0;
	char buf[2048] = {0};
	if (!p->b_cmdConnectLost)
	{
		ret = send_command( p->cph, CTRL_CMD_GETPTZPROTOCOLLIST, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		ifly_PTZProtocolList_t para_info;
		memset(&para_info,0,sizeof(ifly_PTZProtocolList_t));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(ifly_PTZProtocolList_t));
		memcpy(p_para, &para_info, sizeof(ifly_PTZProtocolList_t));
	}
	
	return ret;
}

#if 0
int __stdcall NETDVR_getFrameRateListByresolution( int Handle, NETDVR_FramerateListByResolution_t* pList )
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (pList->chn >= p->si.maxChnNum || !pList || pList->frametype > 1)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	ifly_frameratelistbyresolution_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	para_info.chn = pList->chn;
	para_info.resolutiontype = pList->resolutiontype;
	para_info.frametype = pList->frametype;
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_GETFRAMERATELIST_BYRESOLUTION, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(pList,&para_info,sizeof(ifly_frameratelistbyresolution_t));
	}
	
	return ret;
}
#endif

int __stdcall NETDVR_GetRecordSCHByType(int Handle, unsigned char chn, unsigned char SchType, enum NETDVR_WEEKDAY day, struct NETDVR_RecSchTime_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_RecSchTime_t para_info;
	memset(&para_info,0,sizeof(ifly_RecSchTime_t));
	
	char tempBuf[3] = {0};
	tempBuf[0] = chn;
	tempBuf[1] = day;
	tempBuf[2] = SchType;
	// 	memcpy(tempBuf,&chn,sizeof(chn));
	// 	memcpy(tempBuf+1,&day,sizeof(day));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETRECSCHPARAMBYTYPE, tempBuf, sizeof(tempBuf), buf, sizeof(buf), CTRL_PROTOCOL_CONNECT_DEFAULT * 5);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->chn = para_info.chn;
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);
		p_para->copy2Weekday = para_info.copy2Weekday;
		p_para->type = para_info.type;
		for (int j = 0; j < 24; j++)
		{
			p_para->TimeFiled[j].endtime = ntohl(para_info.TimeFiled[j].endtime);
			p_para->TimeFiled[j].starttime = ntohl(para_info.TimeFiled[j].starttime);
		}
		p_para->weekday = (enum NETDVR_WEEKDAY)para_info.weekday;
		
	}
	
	return ret;
}

int __stdcall NETDVR_SetRecordSCHByType(int Handle, const struct NETDVR_RecSchTime_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	ifly_RecSchTime_t para_info;
	memset(&para_info,0,sizeof(ifly_RecSchTime_t));
	para_info.chn =p_para->chn;
	para_info.type = p_para->type;
	para_info.copy2Chnmask = htonl(p_para->copy2Chnmask);
	para_info.copy2Weekday = p_para->copy2Weekday;
	for (int i = 0;i<24;i++)
	{
		para_info.TimeFiled[i].endtime = htonl(p_para->TimeFiled[i].endtime);
		para_info.TimeFiled[i].starttime = htonl(p_para->TimeFiled[i].starttime);
	}
	para_info.weekday = p_para->weekday;
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETRECSCHPARAMBYTYPE, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;
}

int __stdcall NETDVR_getHuiNaInfo(int Handle, struct NETDVR_HuiNaInfo_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}

	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_HuiNaInfo_t para_info;
	memset(&para_info,0,sizeof(ifly_HuiNaInfo_t));
	
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETHUINAINFO, NULL, 0, buf, sizeof(buf), g_connect_timeout);
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->admin_host = ntohl(para_info.admin_host);
		p_para->ctrl_port = ntohs(para_info.ctrl_port);
		p_para->data_port = ntohs(para_info.data_port);
		p_para->heartbeat_time = ntohs(para_info.heartbeat_time);
		p_para->server_enable_flag = para_info.server_enable_flag;
		memcpy(p_para->device_flag, para_info.device_flag, sizeof(p_para->device_flag));
		memcpy(p_para->huina_http_ulr, para_info.huina_http_ulr, sizeof(p_para->huina_http_ulr));
		memcpy(p_para->shop_num, para_info.shop_num, sizeof(p_para->shop_num));
	}
	
	return ret;
}

int __stdcall NETDVR_setHuiNaInfo(int Handle, const struct NETDVR_HuiNaInfo_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	ifly_HuiNaInfo_t para_info;
	memset(&para_info,0,sizeof(ifly_HuiNaInfo_t));
	para_info.admin_host = htonl(p_para->admin_host);
	para_info.ctrl_port = htons(p_para->ctrl_port);
	para_info.data_port = htons(p_para->data_port);
	para_info.heartbeat_time = htons(p_para->heartbeat_time);
	para_info.server_enable_flag = p_para->server_enable_flag;
	memcpy(para_info.device_flag, p_para->device_flag, sizeof(p_para->device_flag));
	memcpy(para_info.huina_http_ulr, p_para->huina_http_ulr, sizeof(p_para->huina_http_ulr));
	memcpy(para_info.shop_num, p_para->shop_num, sizeof(p_para->shop_num));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETHUINAINFO, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;
}

//for2.0
int __stdcall NETDVR_getMotionDectionfor2_0(int Handle, unsigned char chn, struct NETDVR_motionDetectionfor2_0_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};

	
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_MDParamfor2_0_t para_info;
	memset(&para_info,0,sizeof(ifly_MDParamfor2_0_t));
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETMDPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(p_para->block,para_info.block,sizeof(para_info.block));
		p_para->chn = para_info.chn;
		p_para->delay = (enum NETDVR_DELAY_TIME)ntohs(para_info.delay);
		p_para->flag_buzz = para_info.flag_buzz;
		p_para->flag_email = para_info.flag_email;
		p_para->flag_mobile = para_info.flag_mobile;
		p_para->sense = para_info.sense;
		p_para->trigAlarmOut = ntohl(para_info.trigAlarmOut);
		p_para->trigRecChn = ntohl(para_info.trigRecChn);
		for (int i = 0; i<32;i++)
		{
			p_para->alarmptz[i].cruise = para_info.MDPtz[i].cruise;
			p_para->alarmptz[i].flag_cruise = para_info.MDPtz[i].flag_cruise;
			p_para->alarmptz[i].flag_preset = para_info.MDPtz[i].flag_preset;
			p_para->alarmptz[i].flag_track = para_info.MDPtz[i].flag_track;
			p_para->alarmptz[i].preset = para_info.MDPtz[i].preset;
		}
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);
		p_para->flag_enablealarm = para_info.flag_enablealarm;
	}
	
	return ret;

}

int __stdcall NETDVR_setMotionDectionfor2_0(int Handle, const struct NETDVR_motionDetectionfor2_0_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	p = (struct NETDVR_INNER_t *)Handle;
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}

	ifly_MDParamfor2_0_t para_info;
	memset(&para_info,0,sizeof(ifly_MDParamfor2_0_t));
	para_info.chn = p_para->chn;
	memcpy(para_info.block,p_para->block,sizeof(p_para->block));
	para_info.delay = htons(p_para->delay);
	para_info.flag_buzz = p_para->flag_buzz;
	para_info.flag_email = p_para->flag_email;
	para_info.flag_mobile = p_para->flag_mobile;
	para_info.sense = p_para->sense;
	para_info.trigAlarmOut = htonl(p_para->trigAlarmOut);
	para_info.trigRecChn = htonl(p_para->trigRecChn);
	for (int i = 0; i<32;i++)
	{
		para_info.MDPtz[i].cruise = p_para->alarmptz[i].cruise;
		para_info.MDPtz[i].flag_cruise = p_para->alarmptz[i].flag_cruise;
		para_info.MDPtz[i].flag_preset = p_para->alarmptz[i].flag_preset;
		para_info.MDPtz[i].flag_track = p_para->alarmptz[i].flag_track;
		para_info.MDPtz[i].preset = p_para->alarmptz[i].preset;
	}
	para_info.copy2Chnmask = htonl(p_para->copy2Chnmask);
	para_info.flag_enablealarm = p_para->flag_enablealarm;

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETMDPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	
	return ret;

}

int __stdcall NETDVR_getVideoLostfor2_0(int Handle, unsigned char chn, struct NETDVR_VideoLostParamfor2_0_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};


	struct NETDVR_INNER_t *p;
	ifly_VideoLostParamfor2_0_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	ifly_cp_header_t cphead;
	memset(&cphead, 0, sizeof(cphead));

	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
		
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (chn >= p->si.maxChnNum)
	{
		return NETDVR_ERR_PARAM;
	}

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_GETVIDEOLOSTPARAM, &chn, sizeof(chn), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	if(NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->chn = para_info.chn;
		p_para->delay = (enum NETDVR_DELAY_TIME)ntohs(para_info.delay);
		p_para->flag_buzz = para_info.flag_buzz;
		p_para->flag_email = para_info.flag_email;
		p_para->flag_mobile = para_info.flag_mobile;
		p_para->trigAlarmOut = ntohl(para_info.trigAlarmOut);
		p_para->trigRecChn = ntohl(para_info.trigRecChn);
		for (int i = 0; i<32;i++)
		{
			p_para->alarmptz[i].cruise = para_info.VideoLostPtz[i].cruise;
			p_para->alarmptz[i].flag_cruise = para_info.VideoLostPtz[i].flag_cruise;
			p_para->alarmptz[i].flag_preset = para_info.VideoLostPtz[i].flag_preset;
			p_para->alarmptz[i].flag_track = para_info.VideoLostPtz[i].flag_track;
			p_para->alarmptz[i].preset = para_info.VideoLostPtz[i].preset;
		}
		p_para->copy2Chnmask = ntohl(para_info.copy2Chnmask);

	}


	return ret;

}

int __stdcall NETDVR_setVideoLostfor2_0(int Handle, const struct NETDVR_VideoLostParamfor2_0_t *p_para)
{
	int ret = 0;
	char buf[2048] = {0};
	ifly_VideoLostParamfor2_0_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	struct NETDVR_INNER_t *p;

	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	para_info.chn = p_para->chn;
	para_info.delay = htons(p_para->delay);
	para_info.flag_buzz = p_para->flag_buzz;
	para_info.flag_email = p_para->flag_email;
	para_info.flag_mobile = p_para->flag_mobile;
	para_info.trigAlarmOut = htonl(p_para->trigAlarmOut);
	para_info.trigRecChn = htonl(p_para->trigRecChn);
	for (int i = 0; i<32;i++)
	{
		para_info.VideoLostPtz[i].cruise = p_para->alarmptz[i].cruise;
		para_info.VideoLostPtz[i].flag_cruise = p_para->alarmptz[i].flag_cruise;
		para_info.VideoLostPtz[i].flag_preset = p_para->alarmptz[i].flag_preset;
		para_info.VideoLostPtz[i].flag_track = p_para->alarmptz[i].flag_track;
		para_info.VideoLostPtz[i].preset = p_para->alarmptz[i].preset;
	}
	para_info.copy2Chnmask = htonl(p_para->copy2Chnmask);

	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETVIDEOLOSTPARAM, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	

	return ret;

}

int __stdcall NETDVR_getSMTPServerParamsfor2_0(int Handle, struct NETDVR_SMTPServerfor2_0_t *pSMTPServ)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	ret = send_command(p->cph, CTRL_CMD_GETEMAILSMTP, NULL, 0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);
	if(NETDVR_SUCCESS == ret)
	{
		ifly_AlarmSMTPfo2_0_t para_info;
		memset(&para_info,0,sizeof(para_info));
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(pSMTPServ->alarm_email1,para_info.alarm_email1,sizeof(pSMTPServ->alarm_email1));
		memcpy(pSMTPServ->alarm_email2,para_info.alarm_email2,sizeof(pSMTPServ->alarm_email2));
		memcpy(pSMTPServ->alarm_email3,para_info.alarm_email3,sizeof(pSMTPServ->alarm_email3));
		memcpy(pSMTPServ->reserved,para_info.reserved,sizeof(pSMTPServ->reserved));
		memcpy(pSMTPServ->SMTP_svr,para_info.SMTP_svr,sizeof(pSMTPServ->SMTP_svr));
		memcpy(pSMTPServ->username,para_info.username,sizeof(pSMTPServ->username));
		memcpy(pSMTPServ->userpw,para_info.userpw,sizeof(pSMTPServ->userpw));
		pSMTPServ->SMTPport = ntohs(para_info.SMTPport);
		pSMTPServ->flag_ssl = para_info.flag_ssl;
	}
	
	return ret;
}

int __stdcall NETDVR_setSMTPServerParamsfor2_0(int Handle, const struct NETDVR_SMTPServerfor2_0_t *pSMTPServ)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	ifly_AlarmSMTPfo2_0_t para_info;
	memset(&para_info,0,sizeof(para_info));
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	memcpy(para_info.alarm_email1,pSMTPServ->alarm_email1,sizeof(para_info.alarm_email1));
	memcpy(para_info.alarm_email2,pSMTPServ->alarm_email2,sizeof(para_info.alarm_email2));
	memcpy(para_info.alarm_email3,pSMTPServ->alarm_email3,sizeof(para_info.alarm_email3));
	memcpy(para_info.reserved,pSMTPServ->reserved,sizeof(para_info.reserved));
	memcpy(para_info.SMTP_svr,pSMTPServ->SMTP_svr,sizeof(para_info.SMTP_svr));
	memcpy(para_info.username,pSMTPServ->username,sizeof(para_info.username));
	memcpy(para_info.userpw,pSMTPServ->userpw,sizeof(para_info.userpw));
	para_info.SMTPport = htons(pSMTPServ->SMTPport);
	para_info.flag_ssl = pSMTPServ->flag_ssl;
	ret = send_command(p->cph, CTRL_CMD_SETEMAILSMTP, &para_info, sizeof(para_info), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ * 2);
	
	return ret;
}

//csp modify
//获得重启时间
int __stdcall NETDVR_getRebootTime(int Handle, struct NETDVR_reboottime_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_reboottime_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	ret = send_command(p->cph, CTRL_CMD_GETREBOOTTIME,NULL,0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->reboot_time	= ntohl(para_info.reboot_time);
	}
	
	return ret;	
}

//csp modify
int __stdcall NETDVR_getSDKVersion(char* szVersion)
{
	if (!szVersion)
	{
		return NETDVR_ERR_PARAM;
	}
	
	sprintf(szVersion, NETSDK_VERSION);
	return NETDVR_SUCCESS;
}

//csp modify
int __stdcall NETDVR_getAlarmTimeInterval(int Handle, struct NETDVR_alarmtimeinterval_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_uploadalarmtimeinterval_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	ret = send_command(p->cph, CTRL_CMD_GETUPLOADALARMTIME,NULL,0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		p_para->alarm_timeinterval	= ntohs(para_info.alarm_timeinterval);
	}
	
	return ret;	
}

//csp modify
int __stdcall NETDVR_setAlarmTimeInterval(int Handle, const struct NETDVR_alarmtimeinterval_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048];
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_uploadalarmtimeinterval_t para_info;
	para_info.alarm_timeinterval = htons(p_para->alarm_timeinterval);
	
	if (!p->b_cmdConnectLost)
	{
		ret = send_command(p->cph, CTRL_CMD_SETUPLOADALARMTIME, &para_info, sizeof(para_info), buf,sizeof(buf),g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/ );
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	
	return ret;
}

int __stdcall NETDVR_getDDNSList(int Handle, struct NETDVR_DDNSList_t *p_para)
{
	int ret = 0;
	struct NETDVR_INNER_t *p;
	char buf[2048] = {0};
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		return NETDVR_ERR_NOINIT;
	}
	if (!p->b_login)
	{
		return NETDVR_ERR_NOLOGIN;
	}
	if (!p_para)
	{
		return NETDVR_ERR_PARAM;
	}
	
	ifly_DDNSList_t para_info;
	memset(&para_info, 0, sizeof(para_info));
	
	ret = send_command(p->cph, CTRL_CMD_GETDDNSLIST,NULL,0, buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
	
	if (NETDVR_SUCCESS == ret)
	{
		memcpy(&para_info,buf+sizeof(ifly_cp_header_t),sizeof(para_info));
		memcpy(p_para, &para_info, sizeof(para_info));
	}
	
	return ret;	
}
