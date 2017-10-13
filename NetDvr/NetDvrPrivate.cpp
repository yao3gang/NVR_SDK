#include "NetDvrPrivate.h"
#include "TLFileLib.h"

struct NETDVR_DVR_POOL g_dvr_pool;
MutexHandle g_pool_lock;
unsigned int g_connect_timeout = CTRL_PROTOCOL_CONNECT_DEFAULT;
ifly_msgQ_t g_decmsgQ;
HANDLE g_hDecVideoThread = NULL;
BOOL g_bDecVideoThreadRun = FALSE;
HANDLE g_hDecThreadEvent = NULL;
BYTE g_dvrExist[MAX_DVR_NUM] = {0};

HANDLE g_hReConnectThread = NULL;
BOOL g_bReConnectThreadRun = FALSE;
HANDLE g_hReConnectThreadEvent = NULL;

#ifdef USE_CONNMSG_THREAD
HANDLE g_hConnMsgThread = NULL;
DWORD g_dwConnMsgThreadID = 0;
BOOL g_bConnMsgThreadRun = FALSE;
HANDLE g_hConnMsgThreadEvent = NULL;
#endif

DWORD g_dwReconnectTime = 10*1000; //10s

CAudioPlay* g_currAudioPlay = NULL;

//csp modify
static int looprecv(SOCKET s, char * buf, unsigned int rcvsize)
{
	u32 remian = rcvsize;
	u32 recvlen = 0;
	int ret = 0;

	while(remian > 0)
	{
		ret = recv(s,buf+recvlen,remian,0);
		if(ret <= 0)
		{
			//LPVOID lpMsgBuf;
			//FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			//	NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			//	(LPTSTR) &lpMsgBuf, 0, NULL);
			//MessageBox(NULL,(char *)lpMsgBuf,NULL,MB_OK);
			
			return ret;
		}
		
		recvlen += ret;
		remian -= ret;
	}
	
	return recvlen;
}

int loopsend(SOCKET s, char * buf, unsigned int sndsize)
{
	int remian = sndsize;
	int sendlen = 0;
	int ret = 0;
	while(remian > 0)
	{
		ret=send(s,buf+sendlen,remian,0);
		if(ret <= 0)
		{
			return ret;
		}
		
		sendlen += ret;
		remian -= ret;
	}
	
	return sndsize;
}

BOOL CreateMutexHandle(MutexHandle *pMutex)
{
//	return TRUE;
	BOOL bRet = TRUE;
#ifdef WIN32
	*pMutex = CreateMutex(NULL, FALSE, NULL);
	if(*pMutex == NULL) bRet = FALSE;
#else
	bRet = (0 == pthread_mutex_init(pMutex,NULL));
#endif
	return bRet;
}

BOOL CloseMutexHandle(MutexHandle hMutex)
{
//	return TRUE;
	if (!hMutex)
	{
		return FALSE;
	}
#ifdef WIN32
	return CloseHandle(hMutex);
#else
	return (0 == pthread_mutex_destroy(&hMutex) );
#endif
}
BOOL LockMutex(MutexHandle hMutex)
{
//	return TRUE;
	if (!hMutex)
	{
		return FALSE;
	}
#ifdef WIN32
	return ( WAIT_FAILED != WaitForSingleObject(hMutex,INFINITE) );
#else
	return ( 0 == pthread_mutex_lock(&hMutex) );
#endif
}

BOOL UnlockMutex(MutexHandle hMutex)
{
//	return TRUE;
	if (!hMutex)
	{
		return FALSE;
	}
#ifdef WIN32
	return ReleaseMutex(hMutex);
#else
	return ( 0 == pthread_mutex_unlock(&hMutex) );
#endif
}

void adpcm_decoder(char indata[],short outdata[],int len,adpcm_state *state)
{
    signed char *inp;	/* Input buffer pointer */
    short *outp;		/* output buffer pointer */
    int sign;			/* Current adpcm sign bit */
    int delta;			/* Current adpcm output value */
    int step;			/* Stepsize */
    int valpred;		/* Predicted value */
    int vpdiff;			/* Current change to valpred */
    int index;			/* Current step change index */
    int inputbuffer;	/* place to keep next 4-bit value */
    int bufferstep;		/* toggle between inputbuffer/input */
	
	//	state->index = 0;
	//	state->valprev = 0;
    outp = outdata;
    inp = (signed char *)indata;
	
    valpred = state->valprev;
    index = state->index;
    step = stepsizeTable[index];
	
    bufferstep = 0;
    
    for ( ; len > 0 ; len-- ) {
		
		/* Step 1 - get the delta value */
		if ( bufferstep ) {
			delta = inputbuffer & 0xf;
		} else {
			inputbuffer = *inp++;
			delta = (inputbuffer >> 4) & 0xf;
		}
		bufferstep = !bufferstep;
		
		/* Step 2 - Find new index value (for later) */
		index += indexTable[delta];
		if ( index < 0 ) index = 0;
		if ( index > 88 ) index = 88;
		
		/* Step 3 - Separate sign and magnitude */
		sign = delta & 8;
		delta = delta & 7;
		
		/* Step 4 - Compute difference and new predicted value */
		/*
		** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
		** in adpcm_coder.
		*/
		vpdiff = step >> 3;
		if ( delta & 4 ) vpdiff += step;
		if ( delta & 2 ) vpdiff += step>>1;
		if ( delta & 1 ) vpdiff += step>>2;
		
		if ( sign )
			valpred -= vpdiff;
		else
			valpred += vpdiff;
		
		/* Step 5 - clamp output value */
		if ( valpred > 32767 )
			valpred = 32767;
		else if ( valpred < -32768 )
			valpred = -32768;
		
		/* Step 6 - Update step value */
		step = stepsizeTable[index];
		
		/* Step 7 - Output value */
		*outp++ = valpred;
    }
	
    state->valprev = valpred;
    state->index = index;
}

int audio_dec(PFRAMEHDR p_framehdr, unsigned char *p_buf)
{
#ifdef USE_ADPCM
		adpcm_state state;
		int raw_len;
		raw_len = p_framehdr->m_dwDataSize - sizeof(adpcm_state);
		memcpy(&state, p_framehdr->m_pData + raw_len, sizeof(adpcm_state));
		adpcm_decoder((char *)p_framehdr->m_pData, (short *)p_buf, raw_len << 1, &state);
		return (raw_len << 2);
#endif
	return 0;
}

void deal_audio_rcv(PFRAMEHDR p_framehdr, struct IFLY_MediaRcvPara_t* p_MediaRcvPara, unsigned char *p_buf)
{
	if (!p_MediaRcvPara)
	{
		return;
	}

	if (p_framehdr->m_byMediaType == MEDIA_TYPE_ADPCM)
	{
		adpcm_state * pstate = (adpcm_state *)(p_framehdr->m_pData + p_framehdr->m_dwDataSize - sizeof(adpcm_state));
		if (pstate)
		{
			pstate->index = ntohs(pstate->index);
			pstate->valprev = ntohs(pstate->valprev);
		}
	}
	
	//csp modify 20131001
	//::MessageBox(NULL, "NetDvrSDK Audio Data", NULL, MB_OK);
	
	LockMutex(p_MediaRcvPara->cb_raw_lock);
	if (p_MediaRcvPara->pFrameCB)
	{
		p_MediaRcvPara->pFrameCB((pFrameHeadr)p_framehdr, p_MediaRcvPara->dwContentRaw);
	}
	UnlockMutex(p_MediaRcvPara->cb_raw_lock);

#if 0 //cj@20101218
	if (p_MediaRcvPara->p_record_para)
	{
		LockMutex(p_MediaRcvPara->p_record_para->cb_rec_lock);
		if (p_MediaRcvPara->p_record_para->p_rec_cb)
		{
			p_MediaRcvPara->p_record_para->p_rec_cb((pFrameHeadr)p_framehdr, p_MediaRcvPara->p_record_para->dwContentRec);
		}
		UnlockMutex(p_MediaRcvPara->p_record_para->cb_rec_lock);
	}

	if (p_MediaRcvPara->p_record_para2)
	{
		LockMutex(p_MediaRcvPara->p_record_para2->cb_rec_lock);
		if (p_MediaRcvPara->p_record_para2->p_rec_cb)
		{
			p_MediaRcvPara->p_record_para2->p_rec_cb((pFrameHeadr)p_framehdr, p_MediaRcvPara->p_record_para2->dwContentRec);
		}
		UnlockMutex(p_MediaRcvPara->p_record_para2->cb_rec_lock);
	}
#endif

	if (p_buf)
	{
		FrameHeadrDec framehdrdec;
		int dec_size;
		
		memset(&framehdrdec, 0, sizeof(FrameHeadrDec));

		if (p_framehdr->m_byMediaType == MEDIA_TYPE_ADPCM)
		{
			dec_size = audio_dec(p_framehdr, p_buf);	
		} 
		else if(p_framehdr->m_byMediaType == MEDIA_TYPE_ADPCM_HISI)
		{
			HI_S16 len=0;
			if (HI_VOICE_DecodeFrame(&(p_MediaRcvPara->decaudio_hdr),(HI_S16 *)(p_framehdr->m_pData),(HI_S16 *)p_buf,&len)!=0)
			{
				return;
			}
			dec_size = len*sizeof(HI_S16);
		}
		else if(p_framehdr->m_byMediaType == MEDIA_TYPE_PCMU)
		{
			if (p_MediaRcvPara->p_audio_property && p_MediaRcvPara->p_audio_property->audioBitPerSample == 8)
			{
				for(DWORD i=0;i<p_framehdr->m_dwDataSize;i++)
				{   
					p_framehdr->m_pData[i]+=0x80;   // signed 2 unsigned
				}
			}
			dec_size = p_framehdr->m_dwDataSize;
			p_buf = p_framehdr->m_pData;
		}

		if (dec_size > 0)
		{
			framehdrdec.mediaType = MEDIA_TYPE_PCMU;
			framehdrdec.data = (void *)p_buf;
			framehdrdec.data_size = dec_size;
			framehdrdec.audio_mode = p_framehdr->m_byAudioMode;
			
			//csp modify 20131001
			//::MessageBox(NULL, "NetDvrSDK Audio Play???", NULL, MB_OK);
			
			if (!p_MediaRcvPara->bPreviewAudioMute)
			{
				//csp modify 20131001
				//::MessageBox(NULL, "NetDvrSDK Audio Play!!!", NULL, MB_OK);
				
				deal_play_audio(p_MediaRcvPara, &framehdrdec);
			}
			
			LockMutex(p_MediaRcvPara->cb_dec_lock);
			if (p_MediaRcvPara->pDecFrameCB)
			{
				p_MediaRcvPara->pDecFrameCB(&framehdrdec, p_MediaRcvPara->dwContent);
			}
			UnlockMutex(p_MediaRcvPara->cb_dec_lock);
		}
	}
}

void deal_playcb_audio_rcv(PFRAMEHDR p_framehdr,  struct IFLY_MediaRcvPara_t* p_MediaRcvPara, unsigned char *p_buf)
{
	deal_audio_rcv(p_framehdr, p_MediaRcvPara, p_buf);
}

int deal_voip_rcv(PFRAMEHDR p_framehdr, struct IFLY_MediaRcvPara_t* p_MediaRcvPara, unsigned char *p_buf)
{	
	if (!p_buf || !p_MediaRcvPara)
	{
		return 0;
	}
	
	FrameHeadrDec framehdrdec;

	if (p_MediaRcvPara->p_audio_property->audioBitPerSample == 8)
	{
		for(DWORD i=0;i<p_framehdr->m_dwDataSize;i++)
		{
			p_framehdr->m_pData[i]+=0x80;   // signed 2 unsigned
		}
	}

	memcpy(p_buf, p_framehdr->m_pData, p_framehdr->m_dwDataSize);
	framehdrdec.data = (void *)p_buf;
	framehdrdec.data_size = p_framehdr->m_dwDataSize;
	framehdrdec.audio_mode = p_framehdr->m_byAudioMode;
	
	deal_play_audio(p_MediaRcvPara, &framehdrdec);

	if (p_MediaRcvPara->pDecFrameCB)
	{
		p_MediaRcvPara->pDecFrameCB(&framehdrdec, p_MediaRcvPara->dwContentRaw);
	}
	return 0;
}

#if 0
int config_port_free(unsigned short *port, int flag)
{
	static unsigned short port_used[RCV_PORT_NUM];
	static unsigned short count = 0;
	int i = count;

	LockMutex(g_dvr_pool.port_lock);
	if (*port != 0)
	{
		for (; i < count; i++)
		{
			if (*port == port_used[i])
				break;
		}
	}
	
	if (NETDVR_PORT_GET_FREE == flag)
	{
		if (i < count)
		{
			UnlockMutex(g_dvr_pool.port_lock);
			return FALSE;
		}

		if (count >= RCV_PORT_NUM)
		{
			UnlockMutex(g_dvr_pool.port_lock);
			return FALSE;
		}
		
		if (0 == *port)
		{
			*port = 20000;//RCV_PORT_BASE;

			unsigned char b_find = TRUE;
			while (b_find)
			{
				b_find = FALSE;
				for (i = 0; i < RCV_PORT_NUM; i++)
				{
					if (*port == port_used[i])
					{
						b_find = TRUE;
						*port += 1;
						break;
					}
				}

				if (*port >= RCV_PORT_BASE + RCV_PORT_NUM)
					break;
			}

			if (b_find)
			{
				*port = 0;
				UnlockMutex(g_dvr_pool.port_lock);
				return FALSE;
			}
			port_used[count] = *port;
		}
		else
		{
			port_used[count] = *port;
		}
		count++;
	}
	else
	{
		if (i < count)
		{
			count--;
			port_used[i] = port_used[count];
			port_used[count] = 0;
			*port = 0;
		}
	}

	UnlockMutex(g_dvr_pool.port_lock);
	
	return TRUE;
}
#endif

int get_local_ip(CPHandle cph, struct sockaddr_in *p_addr)
{
	if (cph)
	{
		return GetCPHandleLocalAddr(cph, p_addr);
	}
	else
	{
		char szHostName[64];
		struct hostent* pHostLocal;
		int i;
		
		gethostname(szHostName, sizeof(szHostName));
		pHostLocal = gethostbyname(szHostName);
		
		for(i=0;;i++)
		{
			if(pHostLocal->h_addr_list[i] + pHostLocal->h_length >= pHostLocal->h_name)
				break;
		}
		
		p_addr->sin_addr = *(struct in_addr *)pHostLocal->h_addr_list[0];
		
		return TRUE;
	}
}

#define CTRL_UPDATING 0xffff
#define	NETDVR_IS_UPDATING 2043

int iflydvr_get_error_code(int event)
{
	switch(event)
	{
	case CTRL_SUCCESS:
		return NETDVR_SUCCESS; 
	case CTRL_FAILED_LINKLIMIT:						//已达到连接上限
		return NETDVR_ERR_LINKLIMIT; 
	case CTRL_FAILED_DATABASE: 
		return NETDVR_ERR_DATABASE; 
	case CTRL_FAILED_USER: 
		return NETDVR_ERR_NOUSER; 
	case CTRL_FAILED_PASSWORD: 
		return NETDVR_ERR_PASSWD; 
	case CTRL_FAILED_COMMAND: 
		return NETDVR_ERR_COMMAND; 
	case CTRL_FAILED_MACADDR:							//远程访问的pc mac地址错误
		return NETDVR_ERR_MACADDR; 
	case CTRL_FAILED_RELOGIN: 
		return NETDVR_ERR_RELOGIN; 
	case CTRL_FAILED_PARAM: 
		return NETDVR_ERR_RET_PARAM;
	case CTRL_FAILED_OUTOFMEMORY: 
		return NETDVR_ERR_OUTOFMEMORY;
	case CTRL_FAILED_RESOURCE: 
		return NETDVR_ERR_RESOURCE; 
	case CTRL_FAILED_FILENOTEXIST: 
		return NETDVR_ERR_FILENOTEXIST; 
	case CTRL_FAILED_BAUDLIMIT: 
		return NETDVR_ERR_BAUDLIMIT; 
	case CTRL_FAILED_CREATESOCKET: 
		return NETDVR_ERR_CREATESOCKET; 
	case CTRL_FAILED_CONNECT: 
		return NETDVR_ERR_CONNECT; 
	case CTRL_FAILED_BIND: 
		return NETDVR_ERR_BIND;		
	case CTRL_FAILED_LISTEN:					//侦听失败
		return NETDVR_ERR_LISTEN;			
	case CTRL_FAILED_NETSND:					//网络发送出错
		return NETDVR_ERR_NETSND;		
	case CTRL_FAILED_NETRCV:					//网络接收出错
		return NETDVR_ERR_NETRCV;		
	case CTRL_FAILED_TIMEOUT:					//网络连接超时
		return NETDVR_ERR_TIMEOUT;		
	case CTRL_FAILED_CHNERROR:					//超出通道限制
		return NETDVR_ERR_CHNERROR;		
	case CTRL_FAILED_DEVICEBUSY:				//设备正在忙
		return NETDVR_ERR_DEVICEBUSY;		
	case CTRL_FAILED_WRITEFLASH:				//烧写flash出错
		return NETDVR_ERR_WRITEFLASH;		
	case CTRL_FAILED_VERIFY:					//校验错
		return NETDVR_ERR_VERIFY;		
	case CTRL_FAILED_CONFLICT:					//系统资源冲突
		return NETDVR_ERR_CONFLICT;		
	case CTRL_FAILED_BUSY:						//系统正在忙
		return NETDVR_ERR_BUSY;		
	case CTRL_FAILED_USER_SAME:					//用户名相同07-08-02
		return NETDVR_ERR_USER_SAME;		
	case CTRL_FAILED_NOINIT:					//模块未初始化
		return NETDVR_ERR_NOINIT;		
	case CTRL_FAILED_UNKNOWN:					//未知错误
		return NETDVR_ERR_UNKNOWN;
	case CTRL_UPDATING:
		return NETDVR_IS_UPDATING;
	default: 
		return NETDVR_ERR_UNKNOWN; 
	}
}

void CALLBACK deal_record_filename(char *p_filename, u32 dwContent)
{
	struct IFLY_RecordPara_t* p_record_para;
	
	p_record_para = (struct IFLY_RecordPara_t *)dwContent;

	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime);
	wsprintf(p_filename, TEXT("Chn%02d%04d%02d%02d%02d%02d%02d.ifv"), p_record_para->chn + 1, SystemTime.wYear, SystemTime.wMonth
		, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);		
}

BOOL is_video_frame(u8 media_type)
{
	switch (media_type)
	{
	case MEDIA_TYPE_H264:
		return TRUE;
	case MEDIA_TYPE_MP4:
		return TRUE;
	case MEDIA_TYPE_H261:
		return TRUE;
	case MEDIA_TYPE_H263:
		return TRUE;
	case MEDIA_TYPE_MJPEG:
		return TRUE;
	case MEDIA_TYPE_MP2:
		return TRUE;
	}
	
	return FALSE;
}

void CALLBACK deal_frame_record(pFrameHeadr pFrmHdr, u32 dwContent)
{
	struct IFLY_RecordPara_t* p_record_para;
	u8 update, b_video;	

	p_record_para = (struct IFLY_RecordPara_t *)dwContent;

	if (!p_record_para)
	{
		return;
	}

	LockMutex(p_record_para->rec_lock);
	if (p_record_para->b_record_on)
	{
		b_video = is_video_frame(pFrmHdr->mediaType);
		
		if (b_video && p_record_para->file)
		{
			if ((custommp4_video_height(p_record_para->file) != pFrmHdr->videoParam.videoHeight)||(custommp4_video_width(p_record_para->file) != pFrmHdr->videoParam.videoWidth))
			{
				custommp4_close(p_record_para->file);
				p_record_para->file = NULL;
			}
		}//by binyang 20110415
		if (p_record_para->file)
		{		
			if ((/*p_record_para->cur_length*/custommp4_end_position(p_record_para->file) + pFrmHdr->dataSize) > p_record_para->max_length)
			{
				custommp4_close(p_record_para->file);
				p_record_para->file = NULL;
			}
		}

		//to open record file
		if (NULL == p_record_para->file)
		{
			if (!b_video)
			{
				UnlockMutex(p_record_para->rec_lock);
				return;
			}

			char record_file[PATH_LEN_MAX + FILENAME_LEN_MAX];
			
			strcpy(record_file, p_record_para->path);

			LockMutex(p_record_para->cb_filename_lock);
			p_record_para->p_filename_cb(p_record_para->filename, p_record_para->dwContentFilename);
			UnlockMutex(p_record_para->cb_filename_lock);

			strcat(record_file, "\\");
			strcat(record_file, p_record_para->filename);

			p_record_para->file = custommp4_open(record_file, O_W_CREAT, 0, 0);
			if (NULL == p_record_para->file)
			{
				UnlockMutex(p_record_para->rec_lock);
				return;
			}

			{
				if (p_record_para->p_audio_property)
				{
					u16 bits = p_record_para->p_audio_property->audioBitPerSample;
					u32 samplepersec = p_record_para->p_audio_property->audioSamplePerSec;
					u32 size = p_record_para->p_audio_property->audioFrameSize;
					u32 dur = p_record_para->p_audio_property->audioFrameDurTime;

					char encodername[5] = {0};
					switch (p_record_para->p_audio_property->audioEnctype)
					{
					case MEDIA_TYPE_PCMU:
						strcpy(encodername, "GRAW");
						break;
					case MEDIA_TYPE_ADPCM:
						strcpy(encodername, "ADPA");
						break;
					case MEDIA_TYPE_ADPCM_HISI:
						strcpy(encodername, "ADPB");
						break;
					}
					custommp4_set_audio(p_record_para->file, 1000, 1, bits, samplepersec, str2uint(encodername), size, dur);
				}

				custommp4_set_video(p_record_para->file, 
									1000, 
									pFrmHdr->videoParam.videoWidth, 
									pFrmHdr->videoParam.videoHeight, 
									pFrmHdr->frameRate, 
									512 * 1024, 
									str2uint("H264"), 
									0x18);

// 				if (p_record_para->m_sps.buflen)
// 				{
//  					custommp4_write_video_frame(p_record_para->file, p_record_para->m_sps.buf, p_record_para->m_sps.buflen, 0, 1, &update);
// 				}
// 				
// 				if (p_record_para->m_pps.buflen)
// 				{
//  					custommp4_write_video_frame(p_record_para->file, p_record_para->m_pps.buf, p_record_para->m_pps.buflen, 0, 2, &update);
// 				}
 				
			}


			p_record_para->first_stamp = pFrmHdr->timeStamp;
//			p_record_para->cur_length = 0;
			p_record_para->b_wait_key = 1;
		}
		
		if (b_video)
		{
// 			if (!pFrmHdr->videoParam.keyFrame)//if not a key frame
// 			{
// 				if (p_record_para->b_wait_key)//if waiting a key frame
// 				{
// 					UnlockMutex(p_record_para->rec_lock);
// 					return;
// 				}
// 				else if ((p_record_para->last_frame_id + 1) != pFrmHdr->frameID)//check if a frame is lost
// 				{
// 					p_record_para->b_wait_key = 1;
// 					UnlockMutex(p_record_para->rec_lock);
// 					return;
// 				}
// 			}

			custommp4_write_video_frame(p_record_para->file, 
										pFrmHdr->pData, 
										pFrmHdr->dataSize, 
										pFrmHdr->timeStamp - p_record_para->first_stamp, 
										(UINT8)pFrmHdr->videoParam.keyFrame,
										pFrmHdr->frameID,
										&update);
			
			p_record_para->last_frame_id = pFrmHdr->frameID;
			p_record_para->b_wait_key = 0;
			p_record_para->firstvideo = 1;
		}
		else
		{
// 			if (!p_record_para->firstvideo)
// 			{
// 				UnlockMutex(p_record_para->rec_lock);
// 				return;
// 			}
			custommp4_write_audio_frame(p_record_para->file, 
										pFrmHdr->pData, 
										pFrmHdr->dataSize, 
										pFrmHdr->timeStamp - p_record_para->first_stamp, 
										&update);
		}

//		p_record_para->cur_length += pFrmHdr->dataSize;
	} 

	UnlockMutex(p_record_para->rec_lock);
}

void snapshot_rgb_2bmp(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot, u8 bit_count)
{
	FILE* fp_rgb = NULL;
	char s_filepath[PATH_LEN_MAX + FILENAME_LEN_MAX];

	strcpy(s_filepath, p_snapshot->path);
	strcat(s_filepath, "\\");
	strcat(s_filepath, p_snapshot->filename);

	fp_rgb = fopen(s_filepath, "wb");
	if (fp_rgb)
	{
		BITMAPFILEHEADER hdr;
		BITMAPINFOHEADER *bih;
		bih = (BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));
		
		bih->biSize = sizeof(BITMAPINFOHEADER);
		bih->biWidth  = p_framehdrdec->video_param.width;
		bih->biHeight = p_framehdrdec->video_param.height;  //坐标不同，需要颠倒
		bih->biPlanes = 1;
		bih->biCompression = BI_RGB;
		bih->biBitCount = bit_count;
		bih->biSizeImage = p_framehdrdec->data_size;
		bih->biXPelsPerMeter = 0;
		bih->biYPelsPerMeter = 0;
		bih->biClrUsed = 0;
		bih->biClrImportant = 0;
		
		hdr.bfType = ((WORD)('M'<<8) | 'B');
		hdr.bfSize = p_framehdrdec->data_size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		hdr.bfReserved1 = 0;
		hdr.bfReserved2 = 0;
		hdr.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) );
		
		fwrite(&hdr, 1, sizeof(BITMAPFILEHEADER), fp_rgb);
		fflush(fp_rgb);
		fwrite(bih, 1, sizeof(BITMAPINFOHEADER), fp_rgb);
		fflush(fp_rgb);
		fwrite(p_framehdrdec->data, 1, p_framehdrdec->data_size, fp_rgb);
		fflush(fp_rgb);
		free(bih);
		
		fclose(fp_rgb);
	}
}

void snapshot_yuv_2bmp(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot, u8 yuv_type)
{
	unsigned char *bufy , *bufu , *bufv;
	short int r, c,R, G, B, y, u, v; 
	BYTE* yuvBuf;
	short int m_iMaxW, m_iMaxH;
	BYTE *pYUVbuf, *pRGBbuf;
	long iIndex;
	FrameHeadrDec rgb;

	m_iMaxW = p_framehdrdec->video_param.width;
	m_iMaxH = p_framehdrdec->video_param.height;
	pYUVbuf = (BYTE *)p_framehdrdec->data;
	pRGBbuf = (BYTE *)malloc(m_iMaxW * m_iMaxH * 3);

	if (!pRGBbuf)
	{
		return;
	}
	
	yuvBuf=pYUVbuf;

	bufy = yuvBuf;
	if (0 == yuv_type)
	{
		bufv = yuvBuf + m_iMaxW * m_iMaxH;
		bufu = yuvBuf + m_iMaxW * m_iMaxH * 5/4;
	}
	else
	{
		bufu = yuvBuf + m_iMaxW * m_iMaxH;
		bufv = yuvBuf + m_iMaxW * m_iMaxH * 5/4;
	}
	
	BYTE b_upside = 0;
	if (0 == b_upside)
	{
		iIndex=3*m_iMaxW*(m_iMaxH-1);
	} 
	else
	{
		iIndex=0;
	}
	
	
	for (r = 0; r < m_iMaxH; r++) 
	{
		for (c = 0; c < m_iMaxW; c++) 
		{
			y = bufy [c];
			u = bufu [c >> 1];
			
			v = bufv [c >> 1];
			
			R = short(1.164 * (y - 16) + 1.159 * (v - 128));
			G = short(1.164 * (y - 16) - 0.38 * (u - 128) - 0.813 * (v - 128));
			B = short(1.164 * (y - 16) + 2.018 * (u - 128));
			R = max (0, min (255, R));
			G = max (0, min (255, G));
			B = max (0, min (255, B));
			
			pRGBbuf[iIndex + 3 * c] = (u8)B;
			pRGBbuf[iIndex + 1 + 3 * c] = (u8)G;
			pRGBbuf[iIndex + 2 + 3 * c] = (u8)R;
		}
		
		if (0 == b_upside)
		{
 			iIndex -= 3 * m_iMaxW;
		} 
		else
		{
			iIndex += 3 * m_iMaxW;
		}
		
		bufy += m_iMaxW;
		
		if (r % 2)
		{
			bufu += m_iMaxW / 2;
			bufv += m_iMaxW / 2;
		}
	}

	memcpy(&rgb, p_framehdrdec, sizeof(rgb));
	rgb.data = pRGBbuf;
	rgb.data_size = m_iMaxW * m_iMaxH * 3;

	snapshot_rgb_2bmp(&rgb, p_snapshot, 24);

	free(pRGBbuf);
}

void snapshot_rgb_2jpg(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot, u8 factor)
{
	BYTE *bufforjpeg;
	FILE *outfile = NULL;
	BYTE *image_buffer;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */
	char s_filepath[PATH_LEN_MAX + FILENAME_LEN_MAX];
	short int m_iMaxW, m_iMaxH;
	int i;

	strcpy(s_filepath, p_snapshot->path);
	strcat(s_filepath, "\\");
	strcat(s_filepath, p_snapshot->filename);

	if (NULL == (outfile = fopen(s_filepath, "wb")))
	{
		char sztemp[512];
		wsprintf(sztemp, TEXT("Can not open %s !"), s_filepath);
		return;
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	
	m_iMaxW = p_framehdrdec->video_param.width;
	m_iMaxH = p_framehdrdec->video_param.height;

	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = m_iMaxW; 	/* image width and height, in pixels */
	cinfo.image_height = m_iMaxH;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 100, TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = m_iMaxW * 3;	/* JSAMPLEs per row in image_buffer */
	
	bufforjpeg = (BYTE *)p_framehdrdec->data;

	image_buffer = (BYTE *)malloc(m_iMaxW * m_iMaxH * 3);
	
	for (i = 0;  i< m_iMaxW * m_iMaxH; i++)
	{
		image_buffer[i * 3] = bufforjpeg[i * factor + 2];
		image_buffer[i * 3 + 1] = bufforjpeg[i * factor + 1];
		image_buffer[i * 3 + 2] = bufforjpeg[i * factor + 0];
	}

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	
	jpeg_finish_compress(&cinfo);
	
	fclose(outfile);
	free(image_buffer);
	jpeg_destroy_compress(&cinfo);
}

void snapshot_yuv_2jpg(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot, u8 yuv_type)
{
	BYTE *sampleBuffer;
	short int m_iMaxW, m_iMaxH;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile; /* target file */
	JSAMPIMAGE buffer;
	int band, i, buf_width[3], buf_height[3];
	char s_filepath[PATH_LEN_MAX + FILENAME_LEN_MAX];
	int counter;
	u8 *yuvdata[3];

	strcpy(s_filepath, p_snapshot->path);
	strcat(s_filepath, "\\");
	strcat(s_filepath, p_snapshot->filename);

	
	if (NULL == (outfile = fopen(s_filepath, "wb")))
	{
		char sztemp[512];
		wsprintf(sztemp, TEXT("Can not open %s !"), s_filepath);
		return;
	}

	m_iMaxW = p_framehdrdec->video_param.width;
	m_iMaxH = p_framehdrdec->video_param.height;

	sampleBuffer = (u8 *)p_framehdrdec->data;

	yuvdata[0] = (u8 *)sampleBuffer;
	if (0 == yuv_type)//NETDVR_FMT_YV12
	{
		yuvdata[1] = (u8 *)sampleBuffer + m_iMaxW * m_iMaxH * 5 / 4;
		yuvdata[2] = (u8 *)sampleBuffer + m_iMaxW * m_iMaxH;
	}
	else//NETDVR_FMT_I420
	{
		yuvdata[2] = (u8 *)sampleBuffer + m_iMaxW * m_iMaxH * 5 / 4;
		yuvdata[1] = (u8 *)sampleBuffer + m_iMaxW * m_iMaxH;
	}
	
	cinfo.err = jpeg_std_error(&jerr);
	
	jpeg_create_compress(&cinfo);

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = m_iMaxW; /* image width and height, in pixels */
	cinfo.image_height = m_iMaxH;
	cinfo.input_components = 3; /* # of color components per pixel */
	cinfo.in_color_space = JCS_YCbCr;//JCS_RGB; /* colorspace of input image */
	
	jpeg_set_defaults(&cinfo);
	
	jpeg_set_quality(&cinfo, 100, TRUE );
	
	//////////////////////////////
	cinfo.raw_data_in = TRUE;
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 2;
	/////////////////////////
	
	jpeg_start_compress(&cinfo, TRUE);
	
	buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY)); 
	for (band = 0; band < 3; band++)
	{
		buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
		buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
		buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, buf_width[band], buf_height[band]);
	} 
	
	int max_line = cinfo.max_v_samp_factor*DCTSIZE; 
	for (counter = 0; cinfo.next_scanline < cinfo.image_height; counter++)
	{ 
		//buffer image copy.
		for(band=0; band < 3; band++)
		{
			int mem_size = buf_width[band];
			BYTE * pDst = (BYTE *) buffer[band][0];
			BYTE * pSrc = (BYTE *) yuvdata[band] + counter*buf_height[band] * buf_width[band];//yuv.data[band]分别表示YUV起始地址
			
			for(i=0; i<buf_height[band]; i++)
			{
				memcpy(pDst, pSrc, mem_size);
				pSrc += buf_width[band];
				pDst += buf_width[band];
			}
		}
		jpeg_write_raw_data(&cinfo, buffer, max_line);
	}
	
	jpeg_finish_compress(&cinfo);
	
	fclose(outfile);
	
	jpeg_destroy_compress(&cinfo);
}

void do_snapshot(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot)
{
	switch (p_snapshot->pictype)
	{
	case PIC_TYPE_BMP:
		switch (p_framehdrdec->video_param.fmt)
		{
		case NETDVR_FMT_RGB24:
			snapshot_rgb_2bmp(p_framehdrdec, p_snapshot, 24);
			break;
		case NETDVR_FMT_RGB32:
			snapshot_rgb_2bmp(p_framehdrdec, p_snapshot, 32);
			break;
		case NETDVR_FMT_YV12:
			snapshot_yuv_2bmp(p_framehdrdec, p_snapshot, 0);
			break;
		case NETDVR_FMT_YUY2:
			break;
		default://NETDVR_FMT_YV12
			snapshot_yuv_2bmp(p_framehdrdec, p_snapshot, 1);
		}
		break;
	case PIC_TYPE_JPG:
		switch (p_framehdrdec->video_param.fmt)
		{
		case NETDVR_FMT_RGB24:
			snapshot_rgb_2jpg(p_framehdrdec, p_snapshot, 24 >> 3);
			break;
		case NETDVR_FMT_RGB32:
			snapshot_rgb_2jpg(p_framehdrdec, p_snapshot, 32 >> 3);
			break;
		case NETDVR_FMT_YV12:
			snapshot_yuv_2jpg(p_framehdrdec, p_snapshot, 0);
			break;
		case NETDVR_FMT_YUY2:
			break;
		default://NETDVR_FMT_YV12
			snapshot_yuv_2jpg(p_framehdrdec, p_snapshot, 1);
			return;
		}
		break;
	default:
			return;
	}
}

#define MEDIA_TYPE_H264_FPGA  109
void deal_video_rcv(PFRAMEHDR pFrmHdr, struct IFLY_MediaRcvPara_t *p_MediaRcvPara, u8 *p_buf)
{
	//char showmsg[256];
	//sprintf(showmsg,"deal_video_rcv1:pic=%dx%d enc=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
	//::MessageBox(NULL,showmsg,NULL,MB_OK);
	
	if (!p_MediaRcvPara)
	{
		return;
	}
	
// 	char bufname[256] = {0};
// 	sprintf(bufname, "E:\\Deal_RCBFram.txt");
// 	FILE* fp = NULL;
// 	fp = fopen(bufname,"at");
// 	fprintf(fp,"begin\r\n");
// 	fclose(fp);
	
	if(p_MediaRcvPara->bInterrupted && pFrmHdr->m_tVideoParam.m_bKeyFrame)
	{
		p_MediaRcvPara->bInterrupted = FALSE;
	}
	
	if (p_MediaRcvPara->bInterrupted)
	{
// 		fp = fopen(bufname,"at");
// 		fprintf(fp,"return1\r\n");
// 		fclose(fp);
		return;
	}
	
	//sprintf(showmsg,"deal_video_rcv2:pic=%dx%d enc=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
	//::MessageBox(NULL,showmsg,NULL,MB_OK);
	
	UINT rtFrameId = pFrmHdr->m_dwFrameID;
	if((p_MediaRcvPara->dwOldVideoFrameId!=0) && ((rtFrameId-p_MediaRcvPara->dwOldVideoFrameId)!=1)	&& (!pFrmHdr->m_tVideoParam.m_bKeyFrame))
	{
#if 0
		char bufname[256] = {0};
		sprintf(bufname, "c:\\framelost%02d.txt", p_MediaRcvPara->req.Monitor_t.chn);
		
		FILE* fp = fopen(bufname, "at");
		if (fp)
		{
			fprintf(fp, "lost: id = %d, oldid=%d\n", rtFrameId, p_MediaRcvPara->dwOldVideoFrameId);
			fclose(fp);
		}
#endif
		
		p_MediaRcvPara->dwOldVideoFrameId=0;
		p_MediaRcvPara->bInterrupted=TRUE;
// 		fp = fopen(bufname,"at");
// 		fprintf(fp,"return2\r\n");
// 		fclose(fp);
		return;
	}
	else
	{
		p_MediaRcvPara->dwOldVideoFrameId=rtFrameId;
	}

	BOOL bLoopDec = TRUE;
	if (pFrmHdr->m_byMediaType == MEDIA_TYPE_H264_FPGA)
	{
		bLoopDec = FALSE;
	}
	
	pFrmHdr->m_byMediaType = MEDIA_TYPE_H264;
	
	//sprintf(showmsg,"deal_video_rcv3:pic=%dx%d enc=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
	//::MessageBox(NULL,showmsg,NULL,MB_OK);
	
	LockMutex(p_MediaRcvPara->cb_raw_lock);
	if (p_MediaRcvPara->pFrameCB)
	{
// 		fp = fopen(bufname,"at");
// 		fprintf(fp,"enccallbegin\r\n");
// 		fclose(fp);
		p_MediaRcvPara->pFrameCB((pFrameHeadr)pFrmHdr, p_MediaRcvPara->dwContentRaw);
// 		fp = fopen(bufname,"at");
// 		fprintf(fp,"enccallend\r\n");
// 		fclose(fp);
	}
	UnlockMutex(p_MediaRcvPara->cb_raw_lock);
	
	//sprintf(showmsg,"deal_video_rcv4:pic=%dx%d enc=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
	//::MessageBox(NULL,showmsg,NULL,MB_OK);
	
#if 0  //20100108 cj 录像移到接收码流处，这样丢帧不会影响录像
	if (p_MediaRcvPara->p_record_para)
	{
		LockMutex(p_MediaRcvPara->p_record_para->cb_rec_lock);
		if (p_MediaRcvPara->spspps.spslen)
		{
			memcpy(&p_MediaRcvPara->p_record_para->m_sps.buf, p_MediaRcvPara->spspps.spsbuf, p_MediaRcvPara->spspps.spslen);
			p_MediaRcvPara->p_record_para->m_sps.buflen = p_MediaRcvPara->spspps.spslen;
		}
		
		if (p_MediaRcvPara->spspps.ppslen)
		{
			memcpy(&p_MediaRcvPara->p_record_para->m_pps.buf, p_MediaRcvPara->spspps.ppsbuf, p_MediaRcvPara->spspps.ppslen);
			p_MediaRcvPara->p_record_para->m_pps.buflen = p_MediaRcvPara->spspps.ppslen;
		}

		if (p_MediaRcvPara->p_record_para->p_rec_cb)
		{
			p_MediaRcvPara->p_record_para->p_rec_cb((pFrameHeadr)pFrmHdr, p_MediaRcvPara->p_record_para->dwContentRec);
		}
		UnlockMutex(p_MediaRcvPara->p_record_para->cb_rec_lock);
	}

	if (p_MediaRcvPara->p_record_para2)
	{
		LockMutex(p_MediaRcvPara->p_record_para2->cb_rec_lock);
		if (p_MediaRcvPara->spspps.spslen)
		{
			memcpy(&p_MediaRcvPara->p_record_para2->m_sps.buf, p_MediaRcvPara->spspps.spsbuf, p_MediaRcvPara->spspps.spslen);
			p_MediaRcvPara->p_record_para2->m_sps.buflen = p_MediaRcvPara->spspps.spslen;
		}
		
		if (p_MediaRcvPara->spspps.ppslen)
		{
			memcpy(&p_MediaRcvPara->p_record_para2->m_pps.buf, p_MediaRcvPara->spspps.ppsbuf, p_MediaRcvPara->spspps.ppslen);
			p_MediaRcvPara->p_record_para2->m_pps.buflen = p_MediaRcvPara->spspps.ppslen;
		}

		if (p_MediaRcvPara->p_record_para2->p_rec_cb)
		{
			p_MediaRcvPara->p_record_para2->p_rec_cb((pFrameHeadr)pFrmHdr, p_MediaRcvPara->p_record_para2->dwContentRec);
		}
		UnlockMutex(p_MediaRcvPara->p_record_para2->cb_rec_lock);
	}
#endif
	
	LockMutex(p_MediaRcvPara->dec_lock);
	if (!p_MediaRcvPara->bVideoDecFlag)
	{
		p_MediaRcvPara->bDecKeyFrameFlg = TRUE;
	}
	
	//char showmsg[256];
	//sprintf(showmsg,"deal_video_rcv5:(0x%08x,0x%08x,%d,0x%08x)",p_MediaRcvPara->decoder_hdr,p_buf,p_MediaRcvPara->bVideoDecFlag,p_MediaRcvPara->pDecFrameCB);
	//::MessageBox(NULL,showmsg,NULL,MB_OK);
	
	if (p_MediaRcvPara->decoder_hdr && p_buf && p_MediaRcvPara->bVideoDecFlag && p_MediaRcvPara->pDecFrameCB)
	{
		//char showmsg[256];
		//sprintf(showmsg,"deal_video_rcv6:pic=%dx%d enc=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
		//::MessageBox(NULL,showmsg,NULL,MB_OK);
		
		if (p_MediaRcvPara->bDecKeyFrameFlg)
		{
			if (pFrmHdr->m_tVideoParam.m_bKeyFrame)
			{
				p_MediaRcvPara->bDecKeyFrameFlg = FALSE;
			}
			else
			{
				UnlockMutex(p_MediaRcvPara->dec_lock);
				return; 
			}
		}
		
		//sprintf(showmsg,"deal_video_rcv7:pic=%dx%d enc=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
		//::MessageBox(NULL,showmsg,NULL,MB_OK);
		
		FrameHeadrDec framehdrdec;
		int dec_size = 0;
		
		if (p_MediaRcvPara->p_video_property && p_MediaRcvPara->p_video_property->videoEncType == MEDIA_TYPE_H264)
		{
#ifdef _NVR_
			if(p_MediaRcvPara->decoder_hdr != NULL)
			{
				if(p_MediaRcvPara->decoder_width != pFrmHdr->m_tVideoParam.m_wVideoWidth || 
					p_MediaRcvPara->decoder_height != pFrmHdr->m_tVideoParam.m_wVideoHeight)
				{
					//char showmsg[256];
					//sprintf(showmsg,"destroy decoder-1");
					//::MessageBox(NULL,showmsg,NULL,MB_OK);
					
					Hi264DecDestroy(p_MediaRcvPara->decoder_hdr);
					p_MediaRcvPara->decoder_hdr = NULL;
					p_MediaRcvPara->decoder_width = 0;
					p_MediaRcvPara->decoder_height = 0;
					
					//sprintf(showmsg,"destroy decoder-2");
					//::MessageBox(NULL,showmsg,NULL,MB_OK);
					
					if(p_buf == p_MediaRcvPara->p_frmdecBuf)
					{
						if(p_buf != NULL)
						{
							free(p_buf);
							p_buf = NULL;
							p_MediaRcvPara->p_frmdecBuf = NULL;
						}
					}
					
					//sprintf(showmsg,"destroy decoder-3");
					//::MessageBox(NULL,showmsg,NULL,MB_OK);
				}
				if(p_MediaRcvPara->decoder_hdr == NULL)
				{
					H264_DEC_ATTR_S dec_attrbute;
					dec_attrbute.uBufNum        = (pFrmHdr->m_tVideoParam.m_wVideoWidth>720)?2:2;	//1;//16;//reference frames number: 16
//					dec_attrbute.uPicHeightInMB = (pFrmHdr->m_tVideoParam.m_wVideoHeight+15)/16;	//68;//45;//18;
//					dec_attrbute.uPicWidthInMB  = (pFrmHdr->m_tVideoParam.m_wVideoWidth+15)/16;		//120;//80;//22;
					dec_attrbute.uPicHeightInMB = (pFrmHdr->m_tVideoParam.m_wVideoHeight+15)/16;	//68;//45;//18;
					dec_attrbute.uPicWidthInMB  = ((pFrmHdr->m_tVideoParam.m_wVideoWidth+63)/64*64)/16;//120;//80;//22;//解决长视网络摄像机bug
					dec_attrbute.uStreamInType  = 0x00;												//bitstream begin with "00 00 01" or "00 00 00 01"
					dec_attrbute.uWorkMode      = 0x01;												//0x11;//不需要deinteralce
//					dec_attrbute.uWorkMode      |= 0x20;											//多线程
					
					//char showmsg[256];
					//sprintf(showmsg,"re-create decoder-1");
					//::MessageBox(NULL,showmsg,NULL,MB_OK);
					
					if ((p_MediaRcvPara->decoder_hdr = Hi264DecCreate(&dec_attrbute)) == NULL)
					{
						//sprintf(showmsg,"re-create decoder-failed");
						//::MessageBox(NULL,showmsg,NULL,MB_OK);
						
						UnlockMutex(p_MediaRcvPara->dec_lock);
						return;
					}
					
					//sprintf(showmsg,"re-create decoder-2");
					//::MessageBox(NULL,showmsg,NULL,MB_OK);
					
					p_MediaRcvPara->decoder_width = pFrmHdr->m_tVideoParam.m_wVideoWidth;
					p_MediaRcvPara->decoder_height = pFrmHdr->m_tVideoParam.m_wVideoHeight;
					
					if(p_MediaRcvPara->p_video_property->max_videowidth < pFrmHdr->m_tVideoParam.m_wVideoWidth)
					{
						p_MediaRcvPara->p_video_property->max_videowidth = pFrmHdr->m_tVideoParam.m_wVideoWidth;
					}
					if(p_MediaRcvPara->p_video_property->max_videoheight < pFrmHdr->m_tVideoParam.m_wVideoHeight)
					{
						p_MediaRcvPara->p_video_property->max_videoheight = pFrmHdr->m_tVideoParam.m_wVideoHeight;
					}
				}
				if(p_buf == NULL)
				{
					int nMalloclen = ((pFrmHdr->m_tVideoParam.m_wVideoWidth+15)/16*16) * ((pFrmHdr->m_tVideoParam.m_wVideoHeight+15)/16*16) * 3 / 2;
					p_buf = (u8 *)malloc(nMalloclen);
					if(p_buf == NULL)
					{
						UnlockMutex(p_MediaRcvPara->dec_lock);
						return;
					}
					p_MediaRcvPara->p_frmdecBuf = p_buf;
				}
			}
			
			hiH264_DEC_FRAME_S dec_frame;
			HI_S32 hisiresult;
			
			//char showmsg[256];
			//sprintf(showmsg,"before dec:pic=%dx%d encsize=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
			//::MessageBox(NULL,showmsg,NULL,MB_OK);
			
			hisiresult = Hi264DecFrame(p_MediaRcvPara->decoder_hdr, pFrmHdr->m_pData, pFrmHdr->m_dwDataSize, 0, &dec_frame, 0);
			
			//sprintf(showmsg,"hisiresult=%d",hisiresult);
			//::MessageBox(NULL,showmsg,NULL,MB_OK);
			
			while(HI_H264DEC_NEED_MORE_BITS != hisiresult)
			{
				if(HI_H264DEC_NO_PICTURE == hisiresult)   //flush over and all the remain picture are output
				{
					//char showmsg[256];
					//sprintf(showmsg,"hisiresult=HI_H264DEC_NO_PICTURE");
					//::MessageBox(NULL,showmsg,NULL,MB_OK);
					
					break;
				}
				
				if(hisiresult == HI_H264DEC_OK)
				{
					//Hi264DecImageEnhance(p_MediaRcvPara->decoder_hdr, &dec_frame, 40);
					
					const HI_U8 *pY = dec_frame.pY;
					const HI_U8 *pU = dec_frame.pU;
					const HI_U8 *pV = dec_frame.pV;
					
					int m_width = dec_frame.uWidth;
					int m_height = dec_frame.uHeight/16*16;
					if(dec_frame.uHeight == 1088)
					{
						m_height = 1080;
					}
					
					if(m_width > p_MediaRcvPara->decoder_width)
					{
						m_width = p_MediaRcvPara->decoder_width;
					}
					if(m_height > p_MediaRcvPara->decoder_height)
					{
						m_height = p_MediaRcvPara->decoder_height;
					}
					
					int pixes = m_width * m_height;
					
					memcpy(p_buf, (LPBYTE)pY, pixes);
					memcpy(p_buf+pixes, (LPBYTE)pV, pixes/4);
					memcpy(p_buf+pixes*5/4, (LPBYTE)pU, pixes/4);
					
					dec_size = pixes*3/2;
					
					//char showmsg[256];
					//sprintf(showmsg,"dec succ:pic=%dx%d encsize=%d decsize=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize,dec_size);
					//::MessageBox(NULL,showmsg,NULL,MB_OK);
				}
				
				hisiresult = Hi264DecFrame(p_MediaRcvPara->decoder_hdr, NULL, 0, 0, &dec_frame, 0);
			}
			
			//sprintf(showmsg,"after dec:pic=%dx%d encsize=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
			//::MessageBox(NULL,showmsg,NULL,MB_OK);
#endif
			
#ifdef DEC_AVCDEC
			int result;
			if (bLoopDec)
			{
				FILE * fp = NULL;
				//fp = fopen("c:\\testdec.txt", "at");
				u32 dwstart = GetTickCount();
				result = My_avcdDecodeOneNal(p_MediaRcvPara->decoder_hdr, pFrmHdr->m_pData,pFrmHdr->m_dwDataSize, &p_MediaRcvPara->m_recoBuf , &p_MediaRcvPara->m_refBuf, pFrmHdr->m_tVideoParam.m_bKeyFrame);
				if (fp)
				{
					fprintf(fp, "id = %d, dur = %d\n", pFrmHdr->m_dwFrameID, GetTickCount()-dwstart);
					fflush(fp);
					fclose(fp);
					fp = NULL;
				}
			}
			else
			{
				result = avcdDecodeOneNal(p_MediaRcvPara->decoder_hdr, pFrmHdr->m_pData,pFrmHdr->m_dwDataSize, &p_MediaRcvPara->m_recoBuf , &p_MediaRcvPara->m_refBuf);
			}
			
			if(p_MediaRcvPara->m_recoBuf.picType == AVCD_PIC_SPS)
			{
				if (p_MediaRcvPara->p_record_para)
				{
					memcpy(p_MediaRcvPara->p_record_para->m_sps.buf, pFrmHdr->m_pData, pFrmHdr->m_dwDataSize);
					p_MediaRcvPara->p_record_para->m_sps.buflen = pFrmHdr->m_dwDataSize;
				}

				if (p_MediaRcvPara->p_record_para2)
				{
					memcpy(p_MediaRcvPara->p_record_para2->m_sps.buf, pFrmHdr->m_pData, pFrmHdr->m_dwDataSize);
					p_MediaRcvPara->p_record_para2->m_sps.buflen = pFrmHdr->m_dwDataSize;
				}

				UnlockMutex(p_MediaRcvPara->dec_lock);
				return;
			}
			else if (p_MediaRcvPara->m_recoBuf.picType == AVCD_PIC_PPS)
			{
				if (p_MediaRcvPara->p_record_para)
				{
					memcpy(p_MediaRcvPara->p_record_para->m_pps.buf, pFrmHdr->m_pData, pFrmHdr->m_dwDataSize);
					p_MediaRcvPara->p_record_para->m_pps.buflen = pFrmHdr->m_dwDataSize;
				}

				if (p_MediaRcvPara->p_record_para2)
				{
					memcpy(p_MediaRcvPara->p_record_para2->m_pps.buf, pFrmHdr->m_pData, pFrmHdr->m_dwDataSize);
					p_MediaRcvPara->p_record_para2->m_pps.buflen = pFrmHdr->m_dwDataSize;
				}

				UnlockMutex(p_MediaRcvPara->dec_lock);
				return;
			}

			if(result == AVCD_OK)
			{
				pFrmHdr->m_tVideoParam.m_wVideoWidth = p_MediaRcvPara->m_recoBuf.width;
				pFrmHdr->m_tVideoParam.m_wVideoHeight = p_MediaRcvPara->m_recoBuf.height;
				memcpy(p_buf, (LPBYTE)p_MediaRcvPara->m_recoBuf.y, 
						pFrmHdr->m_tVideoParam.m_wVideoWidth*pFrmHdr->m_tVideoParam.m_wVideoHeight);
				memcpy(p_buf+pFrmHdr->m_tVideoParam.m_wVideoWidth*pFrmHdr->m_tVideoParam.m_wVideoHeight, 
						(LPBYTE)p_MediaRcvPara->m_recoBuf.v, 
						pFrmHdr->m_tVideoParam.m_wVideoWidth*pFrmHdr->m_tVideoParam.m_wVideoHeight/4);
				memcpy(p_buf+pFrmHdr->m_tVideoParam.m_wVideoWidth*pFrmHdr->m_tVideoParam.m_wVideoHeight*5/4, 
						(LPBYTE)p_MediaRcvPara->m_recoBuf.u, 
						pFrmHdr->m_tVideoParam.m_wVideoWidth*pFrmHdr->m_tVideoParam.m_wVideoHeight/4);
				
				void * tmp;
				
				tmp = p_MediaRcvPara->m_recoBuf.y;
				p_MediaRcvPara->m_recoBuf.y = p_MediaRcvPara->m_refBuf.y;
				p_MediaRcvPara->m_refBuf.y = tmp;
				
				tmp = p_MediaRcvPara->m_recoBuf.u;
				p_MediaRcvPara->m_recoBuf.u = p_MediaRcvPara->m_refBuf.u;
				p_MediaRcvPara->m_refBuf.u = tmp;
				
				tmp = p_MediaRcvPara->m_recoBuf.v;
				p_MediaRcvPara->m_recoBuf.v = p_MediaRcvPara->m_refBuf.v;
				p_MediaRcvPara->m_refBuf.v = tmp;
				
				dec_size = pFrmHdr->m_tVideoParam.m_wVideoWidth*pFrmHdr->m_tVideoParam.m_wVideoHeight*3/2;
			}
			else
			{
				UnlockMutex(p_MediaRcvPara->dec_lock);
				return;
			}
#endif
		}
		
		UnlockMutex(p_MediaRcvPara->dec_lock);
		
		if(dec_size > 0)
		{
			framehdrdec.mediaType = MEDIA_TYPE_H264;
			framehdrdec.data = (void *)p_buf;
			framehdrdec.data_size = dec_size;
			framehdrdec.video_param.fmt = p_MediaRcvPara->fmt;
#ifdef _NVR_
			framehdrdec.video_param.width = pFrmHdr->m_tVideoParam.m_wVideoWidth/*pFrmHdr->m_tVideoParam.m_wVideoWidth*/;
			framehdrdec.video_param.height = pFrmHdr->m_tVideoParam.m_wVideoHeight/*pFrmHdr->m_tVideoParam.m_wVideoHeight*/;
#endif
#ifdef DEC_AVCDEC
			framehdrdec.video_param.width = p_MediaRcvPara->m_recoBuf.width/*pFrmHdr->m_tVideoParam.m_wVideoWidth*/;
			framehdrdec.video_param.height = p_MediaRcvPara->m_recoBuf.height/*pFrmHdr->m_tVideoParam.m_wVideoHeight*/;
#endif
			if (pFrmHdr->m_tVideoParam.m_wBitRate)
			{
				framehdrdec.reserved1[0] = 0;//1;//csp modify
				p_MediaRcvPara->byDeinterlacing = 0;//1;//csp modify
			}
			else
			{
				framehdrdec.reserved1[0] = 0;
				p_MediaRcvPara->byDeinterlacing = 0;
			}
			
			p_MediaRcvPara->wCurrFrmWidth = pFrmHdr->m_tVideoParam.m_wVideoWidth;
			p_MediaRcvPara->wCurrFrmHeight = pFrmHdr->m_tVideoParam.m_wVideoHeight;
			
#if 0 //20100108 cj 抓图移至调用接口处,实时抓图
			if (p_MediaRcvPara->p_snapshot)
			{
				LockMutex(p_MediaRcvPara->p_snapshot->snap_lock);
				if (p_MediaRcvPara->p_snapshot->b_snap_on)
				{
					do_snapshot(&framehdrdec, p_MediaRcvPara->p_snapshot);
					p_MediaRcvPara->p_snapshot->b_snap_on = 0;
				}
				UnlockMutex(p_MediaRcvPara->p_snapshot->snap_lock);
			}
#endif
			
			LockMutex(p_MediaRcvPara->cb_dec_lock);
			if (p_MediaRcvPara->pDecFrameCB)
			{
// 				fp = fopen(bufname,"at");
// 				fprintf(fp,"deccallbegin\r\n");
// 				fclose(fp);
				p_MediaRcvPara->pDecFrameCB(&framehdrdec, p_MediaRcvPara->dwContent);
// 				fp = fopen(bufname,"at");
// 				fprintf(fp,"deccallend\r\n");
// 				fclose(fp);
			}
			UnlockMutex(p_MediaRcvPara->cb_dec_lock);
		}
	}
	else
	{
		UnlockMutex(p_MediaRcvPara->dec_lock);
	}	
// 	fp = fopen(bufname,"at");
// 	fprintf(fp,"end\r\n");
// 	fclose(fp);
}

void deal_playcb_video_rcv(PFRAMEHDR pFrmHdr, struct IFLY_MediaRcvPara_t *p_MediaRcvPara, u8 *p_buf)
{
	deal_video_rcv(pFrmHdr, p_MediaRcvPara, p_buf);
}



#if 1

#ifdef WAIT_WAVE_OUT_DONE
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	LPWAVEHDR pHdr = (LPWAVEHDR)dwParam1;
	static short last_v = 0;
// 	int i;

	switch (uMsg)
	{
    case MM_WOM_DONE:
		// A waveform-audio data block has been played and
		// can now be freed.

		LockMutex(g_audio_buf_lock);

		if (g_audio_buf_used > 1)
		{
			memcpy(&last_v, &g_audio_in_buf[g_audio_buf_used - 1], sizeof(short)); 
			memcpy(pHdr->lpData, g_audio_in_buf, g_audio_buf_used); 
			

			pHdr->dwBufferLength = g_audio_buf_used;

			//memset(g_audio_in_buf, 0x80, g_audio_buf_used); //cj090319
			g_audio_buf_used = 0;
		}
		else
		{
// 				for (i = 0; i < AUDIO_PLAY_INITBUF - 1; i+=2)
// 					memcpy(pHdr->lpData + i, &last_v, sizeof(short));
				
			memset((short *)pHdr->lpData, last_v, AUDIO_PLAY_INITBUF/sizeof(short));
//			memset(pHdr->lpData, 0x80, AUDIO_PLAY_INITBUF);
 			pHdr->dwBufferLength = AUDIO_PLAY_INITBUF;
//			pHdr->dwBufferLength = g_audio_buf_used;
		}

		if ((NULL != g_m_hwo))
		{
			waveOutWrite(hwo, pHdr, sizeof(WAVEHDR));
		}


		UnlockMutex(g_audio_buf_lock);
		break;
    }
}
#endif


BOOL open_audio_out(u8 audio_mode, u16 samplepersec)
{
	if(g_m_hwo == NULL)
	{
		WAVEFORMATEX wfex;
		memset(&wfex, 0, sizeof(wfex));
		wfex.wFormatTag = WAVE_FORMAT_PCM;
		wfex.nChannels = 1;
		wfex.nSamplesPerSec = samplepersec;//8000;
		wfex.wBitsPerSample = audio_mode;
		wfex.nBlockAlign = wfex.wBitsPerSample * wfex.nChannels / 8;
		wfex.nAvgBytesPerSec = wfex.nChannels * wfex.nSamplesPerSec * wfex.wBitsPerSample / 8;

		CreateMutexHandle(&g_audio_buf_lock);
		LockMutex(g_audio_buf_lock);
#ifdef WAIT_WAVE_OUT_DONE
		MMRESULT mmret = waveOutOpen(&g_m_hwo, (DWORD)(UINT)WAVE_MAPPER, &wfex, (DWORD)waveOutProc, (DWORD)0, CALLBACK_FUNCTION);
#else
		MMRESULT mmret = waveOutOpen(&g_m_hwo, WAVE_MAPPER, &wfex, (DWORD)0, (DWORD)0, CALLBACK_NULL);
#endif
		
		if(MMSYSERR_NOERROR != mmret)
		{	
//  		int err = GetLastError();
// 			char temp[10] = {0};
// 			sprintf(temp,"error = %d,ret = %d",err,mmret);
// 			MessageBox(NULL,temp,NULL,MB_OK);

			g_m_hwo = NULL;
			UnlockMutex(g_audio_buf_lock);
			CloseMutexHandle(g_audio_buf_lock);
			g_audio_buf_lock = NULL;
			return FALSE;
		}
		
		if (audio_mode == 8)
		{
			memset(g_audio_in_buf, 0x80, AUDIO_PLAY_BUFLEN);
		}
		else
		{
			memset(g_audio_in_buf, 0, AUDIO_PLAY_BUFLEN);
		}

		for (int i = 0; i < AUDIO_PLAY_BUFNUM; i++)
		{
			memset(&g_hWaveHdr[i], 0, sizeof(WAVEHDR));
			memset(g_audio_play_buf[i], 0, AUDIO_PLAY_BUFLEN);
			g_hWaveHdr[i].lpData = (LPTSTR)g_audio_play_buf[i];
			g_hWaveHdr[i].dwBufferLength = 2;
			g_hWaveHdr[i].dwLoops = 1;
			waveOutPrepareHeader(g_m_hwo, &g_hWaveHdr[i], sizeof(WAVEHDR));
			waveOutWrite(g_m_hwo, &g_hWaveHdr[i], sizeof(WAVEHDR));
		}
		UnlockMutex(g_audio_buf_lock);
		return TRUE;
	}
	return TRUE;
}

void close_audio_out(void)
{
	if (g_m_hwo)
	{
		LockMutex(g_audio_buf_lock);
		for (int i = 0; i < AUDIO_PLAY_BUFNUM; i++)
		{
			waveOutUnprepareHeader(g_m_hwo, &g_hWaveHdr[i],sizeof(WAVEHDR));
		}
		waveOutClose(g_m_hwo);
		g_audio_buf_used = 0;
		g_m_hwo = NULL;
		UnlockMutex(g_audio_buf_lock);
		CloseMutexHandle(g_audio_buf_lock);
		g_audio_buf_lock = NULL;
	}
}

void deal_play_audio(struct IFLY_MediaRcvPara_t* p_MediaRcvPara, pFrameHeadrDec pFrmHdr)
{
// 	LockMutex(g_audio_buf_lock);
// 
// 	if (NULL == g_m_hwo)
// 	{
// 		UnlockMutex(g_audio_buf_lock);
// 		return;
// 	}
// 
// 
// 	if ((g_audio_buf_used + pFrmHdr->data_size) <= AUDIO_PLAY_BUFLEN)
// 	{
// 		memcpy(g_audio_in_buf + g_audio_buf_used, pFrmHdr->data, pFrmHdr->data_size);
// 		g_audio_buf_used += pFrmHdr->data_size;
// 	}
// 
// 	UnlockMutex(g_audio_buf_lock);
	
	//csp modify 20131001
	//::MessageBox(NULL, "NetDvrSDK FillAudioFrame???", NULL, MB_OK);
	
	if (p_MediaRcvPara && p_MediaRcvPara->pAudioPlay && (g_currAudioPlay == p_MediaRcvPara->pAudioPlay))
	{
		//csp modify 20131001
		//::MessageBox(NULL, "NetDvrSDK FillAudioFrame!!!", NULL, MB_OK);
		
		p_MediaRcvPara->pAudioPlay->FillAudioFrame((u8 *)pFrmHdr->data, pFrmHdr->data_size);
	}
}
#endif



void DealMediaFrame(PFRAMEHDR pFrmHdr, unsigned int dwContext)
{
	struct IFLY_MediaRcvPara_t *p_MediaRcvPara;
	p_MediaRcvPara = (struct IFLY_MediaRcvPara_t *)dwContext;
	if (p_MediaRcvPara)
	{
		switch (p_MediaRcvPara->rcv_type)
		{
		case NETDVR_RCV_VIDEO:
			//char showmsg[256];
			//sprintf(showmsg,"DealMediaFrame:pic=%dx%d encsize=%d",pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_dwDataSize);
			//::MessageBox(NULL,showmsg,NULL,MB_OK);
			deal_video_rcv(pFrmHdr, p_MediaRcvPara, p_MediaRcvPara->p_frmdecBuf);
			break;
		case NETDVR_RCV_AUDIO:
			deal_audio_rcv(pFrmHdr, p_MediaRcvPara, p_MediaRcvPara->p_frmdecBuf);
			break;
		case NETDVR_RCV_VOIP:
			deal_voip_rcv(pFrmHdr, p_MediaRcvPara, p_MediaRcvPara->p_frmdecBuf);
			break;
		case NETDVR_RCV_PLAYCB_VIDEO:
			deal_playcb_video_rcv(pFrmHdr, p_MediaRcvPara, p_MediaRcvPara->p_frmdecBuf);
			break;
		case NETDVR_RCV_PLAYCB_AUDIO:
			deal_playcb_audio_rcv(pFrmHdr, p_MediaRcvPara, p_MediaRcvPara->p_frmdecBuf);
			break;
		case NETDVR_RCV_SUBVIDEO:
			deal_video_rcv(pFrmHdr, p_MediaRcvPara, p_MediaRcvPara->p_frmdecBuf);
			break;
		}
	}
}

int set_ip(CPHandle cph, u32 *p_set, u32 ip)
{
	if (0 == *p_set)
	{
		*p_set = ip;
	}
	else if (ip != 0 && *p_set != ip)
	{
		return NETDVR_ERR_PARAM;
	}
	
	if (0 == *p_set)
	{
		struct sockaddr_in ip_addr;
		if (get_local_ip(cph, &ip_addr))
		{
			*p_set = inet_addr(inet_ntoa(ip_addr.sin_addr));
		}
		else
		{		
			return NETDVR_ERROR;
		}
	}
	
	return NETDVR_SUCCESS;
}


#if 0
int set_port(u16 *p_set, u16 port)
{
	if (0 == *p_set)
	{
		*p_set = port;
	}
	else if (port != 0 && *p_set != port)
	{
		return NETDVR_ERR_PARAM;
	}

	config_port_free(p_set, NETDVR_PORT_GET_FREE);
	
	if (0 == *p_set)
	{
		return NETDVR_ERROR;
	}

	return NETDVR_SUCCESS;
}
#endif

int set_portEx(u16 *p_set, u16 port, u8 rcvtype, u8 chn)
{
	if (!p_set)
	{
		return NETDVR_ERR_PARAM;
	}

	WORD wBasePort = RCV_PORT_BASE;
	switch(rcvtype)
	{
	case NETDVR_RCV_VIDEO:
		wBasePort = RCV_PORT_BASE+chn*4;
		break;
	case NETDVR_RCV_AUDIO:
		wBasePort = RCV_PORT_BASE+chn*4+2;
		break;
	case NETDVR_RCV_PLAYCB_VIDEO:
		wBasePort = RCV_PORT_BASE+16;
	    break;
	case NETDVR_RCV_PLAYCB_AUDIO:
		wBasePort = RCV_PORT_BASE+18;
	    break;
	case NETDVR_RCV_VOIP:
		wBasePort = RCV_PORT_BASE+20;
		break;
	case NETDVR_RCV_FILE:
		wBasePort = RCV_PORT_BASE+200;
	    break;
	default:
	    break;
	}

	if (rcvtype != NETDVR_RCV_FILE)
	{	
		if (port == 0)
		{
			*p_set = GetOneUnUsingPort(wBasePort, UDPPORT);	
		} 
		else
		{
			*p_set = GetOneUnUsingPort(port, UDPPORT);	
		}	

	} 
	else //FILE DOWNLOAD USE TCP
	{
		if (port == 0)
		{
			*p_set = GetOneUnUsingPort(wBasePort, TCPPORT);	
		} 
		else
		{
			*p_set = GetOneUnUsingPort(port, TCPPORT);	
		}	
	}


	return NETDVR_SUCCESS;
}


BOOL IsPortUsed(WORD wPort, enum PortType byPortFlag)
{
	if (byPortFlag == UDPPORT)
	{
		MIB_UDPTABLE table;
		DWORD dwSize = 0;
		memset(&table, 0,sizeof(table));
		BOOL bUsing = FALSE;
		if(GetUdpTable(&table,&dwSize,TRUE) == ERROR_INSUFFICIENT_BUFFER)   
		{   
			LPBYTE lpBuf = new BYTE[dwSize];   
			PMIB_UDPTABLE pTable = (PMIB_UDPTABLE)lpBuf;   
			if(GetUdpTable(pTable, &dwSize, TRUE) == NO_ERROR)   
			{   
				for(DWORD i=0; i< pTable->dwNumEntries; i++)   
				{   
					if (pTable->table[i].dwLocalPort == htons(wPort))
					{
						bUsing = TRUE;
						break;
					}
					
				}   
			} 
			delete []lpBuf;
			lpBuf = NULL;
		}   
		return bUsing;
	} 
	else //TCPPORT
	{	MIB_TCPTABLE tcptable;
		DWORD dwSize = 0;
		memset(&tcptable, 0,sizeof(tcptable));
		BOOL bUsing = FALSE;
		if(GetTcpTable(&tcptable,&dwSize,TRUE) == ERROR_INSUFFICIENT_BUFFER)   
		{   
			LPBYTE lpBuf = new BYTE[dwSize];   
			PMIB_TCPTABLE pTable = (PMIB_TCPTABLE)lpBuf;   
			if (GetTcpTable(pTable, &dwSize, TRUE) == NO_ERROR)   
			{   
				for(DWORD i = 0; i < pTable->dwNumEntries; i++)   
				{   
					if (pTable->table[i].dwLocalPort == htons(wPort))
					{
						bUsing = TRUE;
						break;
					}
					
				}   
			} 
			delete []lpBuf;
			lpBuf = NULL;
		}
		return bUsing;
	}

}

WORD GetOneUnUsingPort(WORD wInPort, enum PortType byPortFlag)
{
	WORD wPort = wInPort;
	while (1)
	{
		if (IsPortUsed(wPort, byPortFlag))
		{
			//wPort++;
			wPort += 4;
			//Sleep(0);
		}
		else
		{
			break;
		}
	}
	return wPort;
}




int open_reciever(struct IFLY_MediaRcvPara_t *p_reciever, pFrameCallBack pCBFun, unsigned int dwContent, DWORD dwRtpIP)
{
	if (!p_reciever)
	{
		return NETDVR_ERR_PARAM;
	}
	
	CreateMutexHandle(&p_reciever->dec_lock);
	CreateMutexHandle(&p_reciever->cb_raw_lock);
	CreateMutexHandle(&p_reciever->cb_dec_lock);
	
	LockMutex(p_reciever->cb_raw_lock);
	p_reciever->pFrameCB = pCBFun;
	p_reciever->dwContentRaw = dwContent;
	UnlockMutex(p_reciever->cb_raw_lock);
	
	return NETDVR_SUCCESS;
}

int close_reciever(struct IFLY_MediaRcvPara_t *p_reciever)
{
	if (!p_reciever)
	{
		return NETDVR_ERR_PARAM;
	}

	
	LockMutex(p_reciever->cb_raw_lock);
	p_reciever->pFrameCB = NULL;
	p_reciever->dwContentRaw = 0;
	UnlockMutex(p_reciever->cb_raw_lock);


//	config_port_free(&p_reciever->rcv_port, NETDVR_PORT_SET_FREE);

	CloseMutexHandle(p_reciever->dec_lock);
	p_reciever->dec_lock = NULL;
	CloseMutexHandle(p_reciever->cb_raw_lock);
	p_reciever->cb_raw_lock = NULL;
	CloseMutexHandle(p_reciever->cb_dec_lock);	
	p_reciever->cb_dec_lock = NULL;

	return NETDVR_SUCCESS;
}

void set_logininfo(struct NETDVR_INNER_t *p, const struct NETDVR_loginInfo_t *pLoginInfo)
{
	struct sockaddr_in ipAddr;
	struct NETDVR_loginInfo_t *p_li = &p->li;
	
	//memset(p_li, 0, sizeof(struct NETDVR_loginInfo_t));	
	strcpy(p_li->username, pLoginInfo->username); 
	strcpy(p_li->loginpass, pLoginInfo->loginpass);
	if (0 == strcmp(pLoginInfo->macAddr, ""))
	{
		strcpy(p_li->macAddr, "00:00:00:00:00:00");
		printf("set_logininfo 1\n");
	}
	else
	{	
		strcpy(p_li->macAddr, pLoginInfo->macAddr);
	}
	
	if (0 == pLoginInfo->ipAddr)
	{
		if (get_local_ip(p->cph, &ipAddr))
		{
			p_li->ipAddr = inet_addr(inet_ntoa(ipAddr.sin_addr));
			printf("set_logininfo p_li->ipAddr: %s\n", inet_ntoa(ipAddr.sin_addr));
		}
	}
	else
	{
		p_li->ipAddr = pLoginInfo->ipAddr;
	}
}

int send_command(CPHandle cph, u16 command, const void *in_data, u32 in_length, void *out_data, u32 max_out_length, u32 timeout)
{
	int ret = CTRL_FAILED_BASE;
	ifly_cp_header_t cphead;
	
	ret = CPSend(cph, command, in_data, in_length, out_data, max_out_length, NULL, timeout);

	if (ret == CTRL_SUCCESS)
	{
		memcpy(&cphead, out_data, sizeof(cphead));
		cphead.event = ntohs(cphead.event);
		cphead.length = ntohl(cphead.length);
		if (cphead.event != CTRL_SUCCESS)
		{
// 			FILE *fp = fopen("C:\\test2007error.txt","ab+");
// 			fprintf(fp,"command %d, errorcode %d\r\n", command, cphead.event);
// 			fclose(fp);
			return iflydvr_get_error_code(cphead.event);
		}
	}
	else 
	{	
// 		FILE *fp = fopen("C:\\test2007error.txt","ab+");
// 		fprintf(fp,"command %d, errorcode %d\r\n", command, ret);
// 		fclose(fp);
		return NETDVR_ERR_SEND; 
	}
//	Sleep(100);
	return NETDVR_SUCCESS;
}

int send_command_noout(CPHandle cph, u16 command, const void *in_data, u32 in_length, u32 timeout)
{
	char buf[2048];
	
	return send_command(cph, command, in_data, in_length, buf, sizeof(buf), timeout);
}

void reg_rcvcb_dec(struct IFLY_MediaRcvPara_t *p_reciever, pDecFrameCallBack pCBFun, unsigned int dwContent)
{
	if (!p_reciever) 
	{
		return;
	}

	LockMutex(p_reciever->cb_dec_lock);
	p_reciever->pDecFrameCB = pCBFun;
	p_reciever->dwContent = dwContent;
	UnlockMutex(p_reciever->cb_dec_lock);
}

void unreg_rcvcb_dec(struct IFLY_MediaRcvPara_t *p_reciever)
{
	if (!p_reciever) 
	{
		return;
	}
	
	LockMutex(p_reciever->cb_dec_lock);
	p_reciever->pDecFrameCB = NULL;
	p_reciever->dwContent = 0;
	UnlockMutex(p_reciever->cb_dec_lock);
}

void create_rcvvideo_decoder(struct IFLY_MediaRcvPara_t *p_reciever, NETDVR_VideoProperty_t* p_video_property)
{
	if (!p_reciever || !p_video_property) 
	{
		return;
	}
	
	LockMutex(p_reciever->dec_lock);
	if (!p_reciever->p_video_property)
	{
		p_reciever->p_video_property = p_video_property;
	}
	
	if (!p_reciever->decoder_hdr)
	{
		p_reciever->bVideoDecFlag = TRUE;
		p_reciever->bDecKeyFrameFlg = FALSE;
		
		if (p_video_property->videoEncType == MEDIA_TYPE_H264)
		{
#ifdef _NVR_
			H264_DEC_ATTR_S dec_attrbute;
			dec_attrbute.uBufNum        = (p_video_property->max_videowidth>720)?2:2;	//1;//16;//reference frames number: 16
//			dec_attrbute.uPicHeightInMB = (p_video_property->max_videoheight+15)/16;	//68;//45;//18;
//			dec_attrbute.uPicWidthInMB  = (p_video_property->max_videowidth+15)/16;		//120;//80;//22;
			dec_attrbute.uPicHeightInMB = (p_video_property->max_videoheight+15)/16;	//68;//45;//18;
			dec_attrbute.uPicWidthInMB  = ((p_video_property->max_videowidth+63)/64*64)/16;//120;//80;//22;//解决长视网络摄像机bug
			dec_attrbute.uStreamInType  = 0x00;											//bitstream begin with "00 00 01" or "00 00 00 01"
			dec_attrbute.uWorkMode      = 0x01;											//0x11;//不需要deinteralce
//			dec_attrbute.uWorkMode      |= 0x20;										//多线程
			
			//char showmsg[256];
			//sprintf(showmsg,"decoder create:%dx%d",p_video_property->max_videowidth,p_video_property->max_videoheight);
			//::MessageBox(NULL,showmsg,NULL,MB_OK);
			
			if ((p_reciever->decoder_hdr = Hi264DecCreate(&dec_attrbute)) == NULL)
			{
				UnlockMutex(p_reciever->dec_lock);
				return;
			}
			
			p_reciever->decoder_width = p_video_property->max_videowidth;
			p_reciever->decoder_height = p_video_property->max_videoheight;
#endif
			
#ifdef DEC_AVCDEC
			if( ( p_reciever->decoder_hdr = avcdOpen(MsgOut , 0) ) == NULL )
			{
				UnlockMutex(p_reciever->dec_lock);
				return;
			}
			
			int size = p_video_property->max_videowidth * p_video_property->max_videoheight;
			p_reciever->m_recoBuf.width = p_video_property->max_videowidth;
			p_reciever->m_recoBuf.height = p_video_property->max_videoheight;
			p_reciever->m_refBuf.width = p_video_property->max_videowidth;
			p_reciever->m_refBuf.height = p_video_property->max_videoheight;
			p_reciever->m_recoBuf.y = my_aligned_malloc(size , 16);
			p_reciever->m_recoBuf.u = my_aligned_malloc(size >> 2 , 16);
			p_reciever->m_recoBuf.v = my_aligned_malloc(size >> 2 , 16);
			p_reciever->m_refBuf.y = my_aligned_malloc(size , 16);
			p_reciever->m_refBuf.u = my_aligned_malloc(size >> 2 , 16);
			p_reciever->m_refBuf.v = my_aligned_malloc(size >> 2 , 16);
			
			if (p_reciever->spspps.spslen)
			{
				avcdDecodeOneNal(p_reciever->decoder_hdr, p_reciever->spspps.spsbuf, p_reciever->spspps.spslen, &p_reciever->m_recoBuf , &p_reciever->m_refBuf);
			}
			if (p_reciever->spspps.ppslen)
			{
				avcdDecodeOneNal(p_reciever->decoder_hdr, p_reciever->spspps.ppsbuf, p_reciever->spspps.ppslen, &p_reciever->m_recoBuf , &p_reciever->m_refBuf);
			}
#endif
		}
	}
	UnlockMutex(p_reciever->dec_lock);
}

int set_rcvvideo_decoder_fmt(struct IFLY_MediaRcvPara_t *p_reciever, fmt_type_t fmt)
{
	Decoder_Handle dec_handle;
	
	p_reciever->fmt = fmt;
	
	LockMutex(p_reciever->dec_lock);
	dec_handle = p_reciever->decoder_hdr;
	if (NULL == dec_handle)
	{
		//printf("set_rcvvideo_decoder_fmt 1\n");
		UnlockMutex(p_reciever->dec_lock);	
		return NETDVR_ERROR;
	}
	//printf("set_rcvvideo_decoder_fmt 2\n");
	if (p_reciever->p_video_property->videoEncType == MEDIA_TYPE_H264)
	{
		if (fmt != NETDVR_FMT_YV12)
		{
			UnlockMutex(p_reciever->dec_lock);
			return NETDVR_ERR_PARAM;
		}
	}
	
	UnlockMutex(p_reciever->dec_lock);
	
	return NETDVR_SUCCESS;
}

void destroy_rcvvideo_decoder(struct IFLY_MediaRcvPara_t *p_reciever)
{
	if (!p_reciever) 
	{
		return;
	}
	
	LockMutex(p_reciever->dec_lock);
	if(p_reciever->decoder_hdr)
	{
		if(p_reciever->p_video_property->videoEncType == MEDIA_TYPE_H264)
		{
#ifdef _NVR_
			Hi264DecDestroy(p_reciever->decoder_hdr);
#endif
#ifdef DEC_AVCDEC
			avcdClose(p_reciever->decoder_hdr);
			my_aligned_free(p_reciever->m_recoBuf.y);
			my_aligned_free(p_reciever->m_recoBuf.u);
			my_aligned_free(p_reciever->m_recoBuf.v);
			my_aligned_free(p_reciever->m_refBuf.y);
			my_aligned_free(p_reciever->m_refBuf.u);
			my_aligned_free(p_reciever->m_refBuf.v);
#endif
		}
		
		p_reciever->decoder_hdr = NULL;
	}
	UnlockMutex(p_reciever->dec_lock);
}

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	PWAVEHDR pwh;
	struct IFLY_MediaSndPara_t *p_sender=(struct IFLY_MediaSndPara_t *)dwInstance;
	if(!p_sender || NULL == p_sender->m_hwi ||!p_sender->psnd_t)
	{
		return;
	}

	switch(uMsg)
	{
	case WIM_OPEN:
//		ATLTRACE(TEXT("waveInProc: WIM_OPEN\r\n"));
		p_sender->m_bInterPhoneEnding = FALSE;
		break;
	case WIM_DATA:
//		ATLTRACE(TEXT("waveInProc: WIM_DATA\r\n"));
		pwh = (PWAVEHDR)dwParam1;
		if(!pwh)
		{
			return;
		}
		
		if (p_sender->m_bInterPhoneEnding)
		{
			return;
		}

		if (p_sender->psnd_t->bStart)
		{
			p_sender->m_InterPhoneFrmHdr.m_pData= (BYTE*)pwh->lpData;
			p_sender->m_InterPhoneFrmHdr.m_dwDataSize = pwh->dwBytesRecorded;


			if (p_sender->p_voip_property->VOIPBitPerSample == 8 )
			{

				for(DWORD i=0;i!=pwh->dwBytesRecorded;i++)
				{   
					p_sender->m_InterPhoneFrmHdr.m_pData[i] -= 0x80;   // unsigned 2 signed
				}

			}

			if (p_sender->pDecFrameCB)
			{
				FrameHeadrDec frame;
				memset(&frame, 0, sizeof(frame));
				frame.data = p_sender->m_InterPhoneFrmHdr.m_pData;
				frame.data_size = p_sender->m_InterPhoneFrmHdr.m_dwDataSize;
				frame.audio_mode = p_sender->m_InterPhoneFrmHdr.m_byAudioMode;
				p_sender->pDecFrameCB(&frame, p_sender->dwContent);
			}

			if (!p_sender->sndmode)
			{
				ifly_MediaFRAMEHDR_t hdr;
				memset(&hdr, 0, sizeof(hdr));
				hdr.m_byMediaType = MEDIA_TYPE_PCMU;
				hdr.m_dwTimeStamp = htonl(GetTickCount());
				hdr.m_dwDataSize = htonl(p_sender->m_InterPhoneFrmHdr.m_dwDataSize);
				int ret = 0;
				ret = loopsend(p_sender->psnd_t->sockfd, (char *)&hdr,sizeof(ifly_MediaFRAMEHDR_t));
				if(ret <= 0)
				{
					return;
				}

				ret = loopsend(p_sender->psnd_t->sockfd, (char *)p_sender->m_InterPhoneFrmHdr.m_pData,p_sender->m_InterPhoneFrmHdr.m_dwDataSize);
				if(ret <= 0)
				{
					return;
				}
				
			}
		}
	
		pwh->dwBytesRecorded=0;
		waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));
		break;
	case WIM_CLOSE:
//		ATLTRACE(TEXT("waveInProc: WIM_CLOSE\r\n"));
		break;
	}



}	

int open_voip_sender(struct IFLY_MediaSndPara_t *p_sender)
{


	if (!p_sender || !p_sender->p_voip_property )
	{
		return  S_FALSE;
	}
	
	UINT uiDevCount = waveInGetNumDevs();
	if (p_sender->m_hwi || 0 == uiDevCount)
	{
		return S_FALSE;
	}
	
	u16 wBitsPerSample = p_sender->p_voip_property->VOIPBitPerSample;
	DWORD dwWaveInBuflen = p_sender->p_voip_property->VOIPFrameSize;

	WAVEFORMATEX wfex;
	memset(&wfex, 0, sizeof(wfex));
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = 1;
	wfex.nSamplesPerSec = p_sender->p_voip_property->VOIPSamplePerSec;
	wfex.wBitsPerSample = wBitsPerSample;
	wfex.nBlockAlign = wfex.wBitsPerSample * wfex.nChannels / 8;
	wfex.nAvgBytesPerSec = wfex.nChannels * wfex.nSamplesPerSec * wfex.wBitsPerSample / 8;	
	if (MMSYSERR_NOERROR != waveInOpen(&p_sender->m_hwi, WAVE_MAPPER, &wfex, NULL, NULL, WAVE_FORMAT_QUERY))
	{
		p_sender->m_hwi=NULL;
		return S_FALSE;
	}
	
	if (MMSYSERR_NOERROR != waveInOpen(&p_sender->m_hwi, WAVE_MAPPER, &wfex, (DWORD)waveInProc, (DWORD)p_sender, CALLBACK_FUNCTION))
	{
		p_sender->m_hwi=NULL;
		return S_FALSE;
	}

	p_sender->m_wh1.lpData = (LPSTR) p_sender->m_RecBuf1;
	p_sender->m_wh1.dwBufferLength = dwWaveInBuflen;
	p_sender->m_wh1.dwBytesRecorded = 0;
	p_sender->m_wh1.dwUser = 0;
	p_sender->m_wh1.dwFlags = 0;
	p_sender->m_wh1.dwLoops = 1;
	p_sender->m_wh1.lpNext = NULL;
	p_sender->m_wh1.reserved = 0;
	if (waveInPrepareHeader(p_sender->m_hwi, &p_sender->m_wh1, sizeof(p_sender->m_wh1)) != MMSYSERR_NOERROR)
	{
		waveInClose(p_sender->m_hwi);
		p_sender->m_hwi=NULL;
		return S_FALSE;
	}

	p_sender->m_wh2.lpData = (LPSTR)p_sender->m_RecBuf2;
	p_sender->m_wh2.dwBufferLength = dwWaveInBuflen;
	p_sender->m_wh2.dwBytesRecorded = 0;
	p_sender->m_wh2.dwUser = 0;
	p_sender->m_wh2.dwFlags = 0;
	p_sender->m_wh2.dwLoops = 1;
	p_sender->m_wh2.lpNext = NULL;
	p_sender->m_wh2.reserved = 0;
	if (waveInPrepareHeader(p_sender->m_hwi, &p_sender->m_wh2, sizeof(p_sender->m_wh2)) != MMSYSERR_NOERROR)
	{
		waveInClose(p_sender->m_hwi);
		p_sender->m_hwi=NULL;
		return S_FALSE;
	}
	
	if (waveInAddBuffer(p_sender->m_hwi, &p_sender->m_wh1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		waveInClose(p_sender->m_hwi);
		p_sender->m_hwi=NULL;
		return S_FALSE;
	}

	if (waveInAddBuffer(p_sender->m_hwi, &p_sender->m_wh2, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		waveInClose(p_sender->m_hwi);
		p_sender->m_hwi=NULL;
		return S_FALSE;
	}




	if (MMSYSERR_NOERROR != waveInStart(p_sender->m_hwi))
	{
		waveInClose(p_sender->m_hwi);
		p_sender->m_hwi=NULL;

		return S_FALSE;
	}

	return S_OK;
}

BOOL create_directory(LPTSTR lpPath)
{
	char sDirectory[PATH_LEN_MAX];
	char tDir[PATH_LEN_MAX];
	
	if (!lpPath)
		return FALSE;

#ifdef UNICODE
	UINT s_Len=wcslen(lpPath);
#else
	UINT s_Len=strlen(lpPath);
#endif
	if (0 == s_Len)
		return FALSE;
	
	tDir[0]=TEXT('\0');
#ifdef UNICODE
	wcscpy(sDirectory, lpPath);
#else
	strcpy(sDirectory, lpPath);
#endif
	
	TCHAR sSplit[]=TEXT("\\");
	TCHAR* token=NULL;
#ifdef UNICODE
	token = wcstok(sDirectory, sSplit);
#else
	token = strtok(sDirectory, sSplit);
#endif

	while (NULL != token)
	{
#ifdef UNICODE
		wcscat(tDir,token);
#else
		strcat(tDir,token);
#endif
		HANDLE hFile = CreateFile(tDir, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL
			, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
		
		if (INVALID_HANDLE_VALUE != hFile)
		{
			CloseHandle(hFile);
			hFile=NULL;
		}
		else
		{
			if (!CreateDirectory(tDir, NULL))
			{
				return FALSE;
			}
		}
#ifdef UNICODE
		token = wcstok(NULL, sSplit);
		wcscat(tDir,TEXT("\\"));
#else
		token = strtok(NULL, sSplit);
		strcat(tDir,"\\");
#endif
	}

	return TRUE;
}

void set_default_snap(struct IFLY_Snapshot_t *p_snap, int dvrid, u8 chn)
{
	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime);
	
	wsprintf(p_snap->filename, TEXT("dvr%dchn%d%04d%02d%02d%02d%02d%02d.bmp"), dvrid + 1, chn + 1, SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, 
		SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
	
	strcpy(p_snap->path, NETDVR_PATH_DEFAULT);
	
	p_snap->pictype = PIC_TYPE_BMP;
}



void get_notify_dvr(CPHandle svrcph, struct NETDVR_INNER_t **pp)
{
	u32 i;

	if (NULL == pp)
		return;

	for (i = 0; i < MAX_DVR_NUM/*g_dvr_pool.count*/; i++)
	{
		if (g_dvr_pool.p_dvr[i])
		{
			if (g_dvr_pool.p_dvr[i]->cph == svrcph)
			{
				*pp = g_dvr_pool.p_dvr[i];
				break;
			}
		}

	}
}


//处理事件通知
void DealNotify(CPHandle svrcph, u16 event, u8 *pbyMsgBuf, int msgLen, u8 *pbyAckBuf, int *pAckLen, void* pContext)
{
	struct NETDVR_INNER_t *p = NULL;
	ifly_progress_t playpro;
	ifly_play_hasaudio_t pha;
	ifly_alarmstate_t alarmstate;
//	struct NETDVR_progressParam_t progress;
//	printf("DealNotify \n");
	if (svrcph)
	{
		switch(event)
		{
		case CTRL_NOTIFY_CONNLOST:
			{
				get_notify_dvr(svrcph, &p);
				
				if (p)
				{
					g_dvrExist[p->dvr_id] = 0;
					printf("DealNotify\n");
					p->b_cmdConnectLost = TRUE;
					if (p->p_cb_connlost)
					{
						p->p_cb_connlost(p->dwContentConnlost);
					}
				}
			}
			break;
		case CTRL_NOTIFY_PLAYEND:
			{
				get_notify_dvr(svrcph, &p);
				
				u32 tmpid = 0;
				memcpy(&tmpid,pbyMsgBuf,sizeof(tmpid));
				
				ifly_playback_t* pPlayBack = p->m_pPlayBack;
				while (pPlayBack)
				{
					if (pPlayBack->player.m_PlayId == tmpid)
					{
						if (pPlayBack->player.p_cb_playover)
						{
							pPlayBack->player.p_cb_playover(pPlayBack->player.dwContentPlayover);
						}
						//break;
					}
					pPlayBack = pPlayBack->pNext;
				}
			}
			break;
		case CTRL_NOTIFY_PLAYPROGRESS:
			{
				get_notify_dvr(svrcph, &p);
				memcpy(&playpro,pbyMsgBuf,sizeof(ifly_progress_t));
				
				ifly_playback_t* pPlayBack = p->m_pPlayBack;
				while (pPlayBack)
				{
					if (pPlayBack->player.m_PlayId == playpro.id)
					{
						pPlayBack->player.progress.curr_pos = ntohl(playpro.currPos);
						pPlayBack->player.progress.total_size = ntohl(playpro.totallen);
						
						if (pPlayBack->player.p_cb_progress)
						{
							pPlayBack->player.p_cb_progress(pPlayBack->player.progress, pPlayBack->player.dwContentProgress);
						}
						//break;
					}
					pPlayBack = pPlayBack->pNext;
				}
			}
			break;
		case CTRL_NOTIFY_HASAUDIO:
			{
				get_notify_dvr(svrcph, &p);
				memcpy(&pha, pbyMsgBuf, sizeof(pha));
				{
// 					if (p)
// 					{
// 						if (p->player[0].p_cb_hasaudio)
// 						{
// 							p->player[0].p_cb_hasaudio(pha.bHasAudio, p->player[0].dwContentHasaudio);
// 						}
// 					}
				}
			}
			break;
		case CTRL_NOTIFY_ALARMINFO:
			{
				get_notify_dvr(svrcph, &p);
				memcpy(&alarmstate,pbyMsgBuf,sizeof(ifly_alarmstate_t));
				//printf("DealNotify alarm type: %d, id: %d, state: %d\n",
				//	alarmstate.type, alarmstate.id, alarmstate.state);
				if (p)
				{
					p->alarmstate.type		= alarmstate.type;
					p->alarmstate.state		= alarmstate.state;
					p->alarmstate.id		= alarmstate.id;
					p->alarmstate.reserved1 = ntohs(alarmstate.reserved1);
					p->alarmstate.reserved2 = ntohl(alarmstate.reserved2);

					if (p->p_cb_alarmstate)
					{
						p->p_cb_alarmstate(p->alarmstate, p->dwAlarmStateContent);
					}
				}
			}
			break;
		default:
			break;
		}
	}
	else
	{
		printf("svrcph null \n");
	}
}


FILE *open_update_file(struct IFLY_Update_t *p_update)
{
	char s_filepath[PATH_LEN_MAX + FILENAME_LEN_MAX];
	
	LockMutex(p_update->update_lock);
	if (0 == strlen(p_update->update_path))
	{
		set_out_path(p_update->update_path, NULL);
	}
	
	strcpy(s_filepath, p_update->update_path);
	
	UnlockMutex(p_update->update_lock);

	strcat(s_filepath, "\\");
	
	strcat(s_filepath, p_update->update_filename);
	
	return fopen(s_filepath, "rb");
}

void open_reciever_file(struct IFLY_FileReciever_t *p_reciever)
{
	char s_filepath[PATH_LEN_MAX + FILENAME_LEN_MAX +1];
	LockMutex(p_reciever->reciever_lock);
	if (0 == strlen(p_reciever->save_path))
	{
		set_out_path(p_reciever->save_path, NULL);
	}

	strcpy(s_filepath, p_reciever->save_path);
	UnlockMutex(p_reciever->reciever_lock);
	strcat(s_filepath, "\\");
	strcat(s_filepath, p_reciever->save_filename);
	p_reciever->fp = fopen(s_filepath, "wb");

}

int setSocketNoDelay(SOCKET hSock)
{
	int ret;
	
	//设置NODELAY选项
#ifdef WIN32
	char optval = 1;
#else
	int optval = 1;
#endif
	ret = setsockopt( hSock, IPPROTO_TCP/*SOL_SOCKET*/, TCP_NODELAY, (char *)&optval, sizeof(optval) );
	if( SOCKET_ERROR == ret )
	{
		printf("SetSockLinkOpt:set socket nodelay error!\n");
		return -1;
	}
	
	//设置SO_LINGER为零(亦即linger结构中的l_onoff域设为非零,但l_linger为0),便
	//不用担心closesocket调用进入“锁定”状态(等待完成),不论是否有排队数据未发
	//送或未被确认。这种关闭方式称为“强行关闭”，因为套接字的虚电路立即被复位，尚
	//未发出的所有数据都会丢失。在远端的recv()调用都会失败，并返回WSAECONNRESET错误。
	{
		struct linger m_sLinger;
		m_sLinger.l_onoff = 1;  //(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
		m_sLinger.l_linger = 0; //(容许逗留的时间为0秒)
		ret = setsockopt(hSock,SOL_SOCKET,SO_LINGER,(char*)&m_sLinger,sizeof(struct linger));
		if( SOCKET_ERROR == ret )
		{
			printf("SetSockLinkOpt:set socket linger error!\n");
			return -1;
		}
	}
	
	return 0; 
}

int set_out_path(char *p_path, const char *p_dir_path)
{
	if (!p_path)
	{
		return NETDVR_ERR_PARAM;
	}		

	if (p_dir_path)
	{
#ifdef UNICODE
		UINT tmp_len=wcslen(p_dir_path);
#else
		UINT tmp_len=strlen(p_dir_path);
#endif
		if (tmp_len >= PATH_LEN_MAX)
		{
			return NETDVR_ERR_PARAM;
		}
		
#if 0
//add by cj@20100331
		TCHAR sSplit[]=TEXT("\\");
		TCHAR* token=NULL;
#ifdef UNICODE
		token = wcstok(p_dir_path, sSplit);
#else
		token = strtok((char *)p_dir_path, sSplit);
#endif
		
		while (NULL != token)
		{
#ifdef UNICODE
			wcscat(p_path,token);
#else
			strcat(p_path,token);
#endif

#ifdef UNICODE
			token = wcstok(NULL, sSplit);
			wcscat(p_path,TEXT("\\"));
#else
			token = strtok(NULL, sSplit);
			strcat(p_path,"\\");
#endif
		}
//cj@20100331

#else
		strcpy(p_path, p_dir_path);	
#endif

	}
	else
	{
		strcpy(p_path, NETDVR_PATH_DEFAULT);
	}
	
	if (!create_directory(p_path))
	{
		return NETDVR_ERR_PARAM;
	}
	
	return NETDVR_SUCCESS;
}

#if 0
int command_server(int Handle, const struct NETDVR_Command_t *p_command)
{
	int ret;
	struct NETDVR_INNER_t *p;
	char cmd_param[2048];
	u32 cmd_param_len = 0;
	
	LockMutex(g_pool_lock);
	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
	LockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	UnlockMutex(g_pool_lock);
	
	if (p_command->b_chk_login && !p->b_login)
	{
		UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
		return NETDVR_ERR_NOLOGIN;
	}
	
	if (p_command->b_send_user)
	{
		memcpy(cmd_param, p->li.username, 12);
		cmd_param_len = 12;
	}

	if (p_command->cmd_param && p_command->cmd_param_len)
	{
		memcpy(cmd_param + cmd_param_len, p_command->cmd_param, p_command->cmd_param_len);
		cmd_param_len += p_command->cmd_param_len;
	}

	if (!p->b_cmdConnectLost)
	{
		if (p_command->cmd_ret_param)
		{
			ret = send_command(p->cph, p_command->command, cmd_param, cmd_param_len, p_command->cmd_ret_param
				, p_command->cmd_ret_maxlen, p_command->timeout);
		}
		else
		{
			ret = send_command_noout(p->cph, p_command->command, cmd_param, cmd_param_len, p_command->timeout);
		}
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}

	UnlockMutex(g_dvr_pool.dvr_lock[p->dvr_id]);
	
	return ret;
}
#endif

int GetIPGUID(u32 ip, CPGuid* guid)
{
	unsigned long umac=0,ulen=6;
	
	//发送ARP查询包获得远程MAC地址
	DWORD dwRet = SendARP(ip,0,(PULONG)guid->data,&ulen);
	if(NO_ERROR!=dwRet)
	{
		LPVOID lpMsgBuf;  
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
			);
		//MessageBox((LPCTSTR)lpMsgBuf,"Error+++",MB_OK|MB_ICONINFORMATION);
		LocalFree(lpMsgBuf);
		memset(guid, 0,sizeof(CPGuid));
		
		return -1;
	}
	
	return 0;
}





struct TCP_KEEPALIVE {
    u_long  onoff;
    u_long  keepalivetime;
    u_long  keepaliveinterval;
} ;

#define SIO_KEEPALIVE_VALS   _WSAIOW(IOC_VENDOR,4)
#define MAX_DL_BUF	4*1024

DWORD WINAPI RcvTcpFrameThread(LPVOID lpParam)
{
	NETDVR_INNER_t *p = (NETDVR_INNER_t *)lpParam;
	if(!p)
	{
		return 0;
	}
	
	int	i;
	fd_set set;
	
	struct timeval timeout;
	int ret;
	
	//csp modify
	char *frmaebuf = new char[MAX_FRAME_SIZE];
	if(frmaebuf == NULL)
	{
		return 0;
	}
	
	//FILE *fp = fopen("C:\\RcvTcpFrameThread.txt","ab+");
	
	while(1)
	{
		//fprintf(fp,"RcvTcpFrameThread-1, (0x%08x,0x%08x,0x%08x,%d)\r\n",p,p->hRcvThread,p->hRcvEvent,p->bRcvThreadState);
		//fflush(fp);
		
		if (!p->bRcvThreadState)
		{
			//csp modify
			if(frmaebuf != NULL)
			{
				delete []frmaebuf;
				frmaebuf = NULL;
			}
			
			SetEvent(p->hRcvEvent);
			
			//fprintf(fp,"RcvTcpFrameThread-2\r\n");
			//fflush(fp);
			
			break;
		}
		
		//fprintf(fp,"RcvTcpFrameThread-3, (0x%08x,0x%08x,0x%08x,%d)\r\n",p,p->hRcvThread,p->hRcvEvent,p->bRcvThreadState);
		//fflush(fp);
		
		unsigned char find = 0;
		
		FD_ZERO(&set);
		for(i=0;i<MEDIA_LINK_CLIENT;i++)
		{
			//fprintf(fp,"RcvTcpFrameThread-3.1,i=%d\r\n",i);
			//fflush(fp);
			
			if((p->m_mediarcv[i].sockfd != INVALID_SOCKET)
				&& (p->m_mediarcv[i].bStart)//csp modify
				&& (p->m_mediarcv[i].req.command!=3)
				&& (p->m_mediarcv[i].req.command!=4)
				&& (p->m_mediarcv[i].req.command!=8)
				&& (p->m_mediarcv[i].req.command!=9)
				&& (p->m_mediarcv[i].req.command!=12)
				)
			{
				//{
				//	FILE *fp = fopen("C:\\testupdate.txt","ab+");
				//	fprintf(fp,"添加套接字:(0x%08x,%d),command:%d\r\n",p,p->m_mediarcv[i].sockfd,p->m_mediarcv[i].req.command);
				//	fclose(fp);
				//}
				
				//fprintf(fp,"RcvTcpFrameThread-3.2\r\n");
				//fflush(fp);
				
				FD_SET(p->m_mediarcv[i].sockfd,&set);
				
				find = 1;
			}
			
			//fprintf(fp,"RcvTcpFrameThread-3.3,i=%d\r\n",i);
			//fflush(fp);
		}
		
		//fprintf(fp,"RcvTcpFrameThread-3.4, find=%d, (0x%08x,0x%08x,0x%08x,%d)\r\n",find,p,p->hRcvThread,p->hRcvEvent,p->bRcvThreadState);
		//fflush(fp);
		
		if(!find)
		{
			//fprintf(fp,"RcvTcpFrameThread-3.5, (0x%08x,0x%08x,0x%08x,%d)\r\n",p,p->hRcvThread,p->hRcvEvent,p->bRcvThreadState);
			//fflush(fp);
			
			Sleep(1);
			
			//fprintf(fp,"RcvTcpFrameThread-3.6, (0x%08x,0x%08x,0x%08x,%d)\r\n",p,p->hRcvThread,p->hRcvEvent,p->bRcvThreadState);
			//fflush(fp);
			
			continue;
		}
		
		//fprintf(fp,"RcvTcpFrameThread-4, (0x%08x,0x%08x,0x%08x,%d)\r\n",p,p->hRcvThread,p->hRcvEvent,p->bRcvThreadState);
		//fflush(fp);
		
		//linux平台下timeout会被修改以表示剩余时间,故每次都要重新赋值
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		ret = select(FD_SETSIZE,&set,NULL,NULL,&timeout);
		
		//fprintf(fp,"RcvTcpFrameThread-5,ret=%d\r\n",ret);
		//fflush(fp);
		
		if(ret == 0)
		{
			Sleep(1);
			
			//fprintf(fp,"RcvTcpFrameThread-5.1\r\n");
			//fflush(fp);
			
			continue;
		}
		if(ret == SOCKET_ERROR)
		{
			Sleep(100);
			
			//fprintf(fp,"RcvTcpFrameThread-5.2\r\n");
			//fflush(fp);
			
			continue;
		}
		
		//fprintf(fp,"RcvTcpFrameThread-6\r\n");
		//fflush(fp);
		
		for(i=0; i<MEDIA_LINK_CLIENT; i++)
		{
			if((p->m_mediarcv[i].sockfd != INVALID_SOCKET) && (p->m_mediarcv[i].bStart) && FD_ISSET(p->m_mediarcv[i].sockfd,&set))
			{
				//{
				//	FILE *fp = fopen("C:\\testupdate.txt","ab+");
				//	fprintf(fp,"有数据的套接字:(0x%08x,%d),command:%d\r\n",p,p->m_mediarcv[i].sockfd,p->m_mediarcv[i].req.command);
				//	fclose(fp);
				//}
				
				//fprintf(fp,"RcvTcpFrameThread-7\r\n");
				//fflush(fp);
				
				ifly_MediaFRAMEHDR_t hdr;
				int ret = 0;
				char buf[512] = {0};
				
				//csp modify
				//char frmaebuf[MAX_FRAME_SIZE] = {0};
				
				if (10==p->m_mediarcv[i].req.command) //远程抓图
				{
					ifly_snapack_t ackhead;
					ret = looprecv(p->m_mediarcv[i].sockfd,(char *)&ackhead,sizeof(ackhead));
					if (ret <= 0)
					{
						goto over;
					}
					ackhead.imglenth = ntohl(ackhead.imglenth);
					char imgbuf[64*1024] = {0};
					ret = looprecv(p->m_mediarcv[i].sockfd,imgbuf,ackhead.imglenth);
					FILE* fpImg = fopen(p->remotesnap.filename, "wb");
					if (fpImg)
					{
						fwrite(imgbuf, ackhead.imglenth, 1, fpImg);
						fflush(fpImg);
						fclose(fpImg);
						fpImg = NULL;
					}
					
					continue;
				}
				
				ret = looprecv(p->m_mediarcv[i].sockfd,buf,sizeof(ifly_MediaFRAMEHDR_t));
				if (ret <= 0)
				{
					goto over;
				}
				
				memcpy(&hdr, buf, sizeof(ifly_MediaFRAMEHDR_t));
				hdr.m_dwDataSize = ntohl(hdr.m_dwDataSize);
				hdr.m_dwFrameID = ntohl(hdr.m_dwFrameID);
				hdr.m_dwTimeStamp = ntohl(hdr.m_dwTimeStamp);
				hdr.m_nVideoHeight = ntohl(hdr.m_nVideoHeight);
				hdr.m_nVideoWidth = ntohl(hdr.m_nVideoWidth);
				
				//printf("size=%8d, id = %d, h = %d, w = %d \n", hdr.m_dwDataSize, hdr.m_dwFrameID, hdr.m_nVideoHeight, hdr.m_nVideoWidth);
				
				ret = looprecv(p->m_mediarcv[i].sockfd,frmaebuf,hdr.m_dwDataSize);
				if(ret <= 0)
				{
					goto over;
				}
				
				if(ret == hdr.m_dwDataSize)
				{
					//do deal frame here!
					FRAMEHDR frame = {0};
					frame.m_byMediaType = hdr.m_byMediaType;
					frame.m_byFrameRate = hdr.m_byFrameRate;
					frame.m_tVideoParam.m_bKeyFrame = hdr.m_bKeyFrame;
					//printf("m_bKeyFrame: %d\n", hdr.m_bKeyFrame);
					frame.m_tVideoParam.m_wVideoWidth = hdr.m_nVideoWidth;
					//frame.m_tVideoParam.m_wVideoHeight = hdr.m_nVideoHeight;
					frame.m_tVideoParam.m_wVideoHeight = hdr.m_nVideoHeight & 0x7fff;			//csp add 20091116
					frame.m_tVideoParam.m_wBitRate = ((hdr.m_nVideoHeight & 0x8000)?1:0) << 31;	//csp add 20091116
					frame.m_dwDataSize = hdr.m_dwDataSize;
					frame.m_dwFrameID = hdr.m_dwFrameID;
					frame.m_dwTimeStamp = hdr.m_dwTimeStamp;
					frame.m_pData = (BYTE *)frmaebuf;
					
					#if 0
					if (hdr.m_byMediaType == 98)
					{
						p->m_mediarcv[i].m_dwCount++;
						//p->m_mediarcv[i].m_dwCount += hdr.m_dwDataSize;
						DWORD tickTimeKnow = 0;
						tickTimeKnow = GetTickCount();
						char bufname[256] = {0};
						sprintf(bufname, "E:\\frameRcv_%d.txt",p->m_mediarcv[i].req.Monitor_t.chn);
						FILE* fp = fopen(bufname,"at");
						if (p->m_mediarcv[i].first)
						{
							p->m_mediarcv[i].time = tickTimeKnow;
							p->m_mediarcv[i].tickTimeLast = tickTimeKnow;
						}
						fprintf(fp,"type %d the framekey is %d and the id is %d time %d timestamp %u\n",frame.m_byMediaType, frame.m_tVideoParam.m_bKeyFrame,frame.m_dwFrameID,tickTimeKnow-p->m_mediarcv[i].tickTimeLast, frame.m_dwTimeStamp);
						fflush(fp);
						if (tickTimeKnow-p->m_mediarcv[i].time >= 10000)
						{
							fprintf(fp,"count %d\n",p->m_mediarcv[i].m_dwCount-1);
							fflush(fp);
							p->m_mediarcv[i].m_dwCount = 0;
							p->m_mediarcv[i].time = tickTimeKnow;
						}
						fclose(fp);
						p->m_mediarcv[i].tickTimeLast = tickTimeKnow;
						p->m_mediarcv[i].first = FALSE;
					}
					#else
					//printf("m_byMediaType: %d\n", hdr.m_byMediaType);
					/*
					if (hdr.m_byMediaType == 98)
					{
						printf("m_bKeyFrame: %d, size=%d, id = %d, h = %d, w = %d \n", 
							hdr.m_bKeyFrame, hdr.m_dwDataSize, hdr.m_dwFrameID, hdr.m_nVideoHeight, hdr.m_nVideoWidth);
				
					}
					*/
					#endif
					
					switch (p->m_mediarcv[i].req.command)
					{
					case 0:	// monitor
						{
							u8 type = p->m_mediarcv[i].req.Monitor_t.type;
							u8 chn =  p->m_mediarcv[i].req.Monitor_t.chn;
							
							EnterCriticalSection(&p->m_hcs);
							
							struct IFLY_MediaRcvPara_t *p_MediaRcvPara = (struct IFLY_MediaRcvPara_t *)p->m_mediarcv[i].pContent;
							
							#if 1 //20100108 cj 录像移到接收码流处，这样丢帧不会影响录像
							if (p_MediaRcvPara)
							{
								if (p_MediaRcvPara->p_record_para)
								{
									LockMutex(p_MediaRcvPara->p_record_para->cb_rec_lock);
									if (p_MediaRcvPara->spspps.spslen)
									{
										memcpy(&p_MediaRcvPara->p_record_para->m_sps.buf, p_MediaRcvPara->spspps.spsbuf, p_MediaRcvPara->spspps.spslen);
										p_MediaRcvPara->p_record_para->m_sps.buflen = p_MediaRcvPara->spspps.spslen;
									}
									
									if (p_MediaRcvPara->spspps.ppslen)
									{
										memcpy(&p_MediaRcvPara->p_record_para->m_pps.buf, p_MediaRcvPara->spspps.ppsbuf, p_MediaRcvPara->spspps.ppslen);
										p_MediaRcvPara->p_record_para->m_pps.buflen = p_MediaRcvPara->spspps.ppslen;
									}
									
									if (p_MediaRcvPara->p_record_para->p_rec_cb)
									{
										p_MediaRcvPara->p_record_para->p_rec_cb((pFrameHeadr)&frame, p_MediaRcvPara->p_record_para->dwContentRec);
									}
									UnlockMutex(p_MediaRcvPara->p_record_para->cb_rec_lock);
								}
								
								if (p_MediaRcvPara->p_record_para2)
								{
									LockMutex(p_MediaRcvPara->p_record_para2->cb_rec_lock);
									if (p_MediaRcvPara->spspps.spslen)
									{
										memcpy(&p_MediaRcvPara->p_record_para2->m_sps.buf, p_MediaRcvPara->spspps.spsbuf, p_MediaRcvPara->spspps.spslen);
										p_MediaRcvPara->p_record_para2->m_sps.buflen = p_MediaRcvPara->spspps.spslen;
									}
									
									if (p_MediaRcvPara->spspps.ppslen)
									{
										memcpy(&p_MediaRcvPara->p_record_para2->m_pps.buf, p_MediaRcvPara->spspps.ppsbuf, p_MediaRcvPara->spspps.ppslen);
										p_MediaRcvPara->p_record_para2->m_pps.buflen = p_MediaRcvPara->spspps.ppslen;
									}
									
									if (p_MediaRcvPara->p_record_para2->p_rec_cb)
									{
										p_MediaRcvPara->p_record_para2->p_rec_cb((pFrameHeadr)&frame, p_MediaRcvPara->p_record_para2->dwContentRec);
									}
									UnlockMutex(p_MediaRcvPara->p_record_para2->cb_rec_lock);
								}
							}
							#endif
							
							if ((hdr.m_byMediaType == MEDIA_TYPE_H264) || (hdr.m_byMediaType == MEDIA_TYPE_H264_FPGA))
							{
								u8 *pBuf = NULL;
								u32 buflenth = 0;
								BOOL bRet = GetMsgQWriteInfo(&g_decmsgQ, &pBuf,&buflenth);
								if (bRet)
								{
									FrameMsg_t* pWriteFrame = (FrameMsg_t*)pBuf;
									pWriteFrame->frame.m_byMediaType = frame.m_byMediaType;
									pWriteFrame->frame.m_dwPreBufSize = frame.m_dwPreBufSize;
									pWriteFrame->frame.m_dwDataSize = frame.m_dwDataSize;
									pWriteFrame->frame.m_byFrameRate = frame.m_byFrameRate;
									pWriteFrame->frame.m_dwFrameID = frame.m_dwFrameID;
									pWriteFrame->frame.m_dwTimeStamp = frame.m_dwTimeStamp;
									pWriteFrame->frame.m_tVideoParam.m_bKeyFrame = frame.m_tVideoParam.m_bKeyFrame;
									pWriteFrame->frame.m_tVideoParam.m_wVideoHeight = frame.m_tVideoParam.m_wVideoHeight;
									pWriteFrame->frame.m_tVideoParam.m_wVideoWidth = frame.m_tVideoParam.m_wVideoWidth;
									pWriteFrame->frame.m_tVideoParam.m_wBitRate = frame.m_tVideoParam.m_wBitRate;
									pWriteFrame->p_MediaRcvPara = p_MediaRcvPara;

									pWriteFrame->dvr_id = p->dvr_id;
									memcpy(pWriteFrame->frame.m_pData, frame.m_pData, frame.m_dwDataSize);

									skipWriteMsgQ(&g_decmsgQ);
								}
							}
							else
							{
								DealMediaFrame(&frame, (u32)p_MediaRcvPara);
							}
							LeaveCriticalSection(&p->m_hcs);
						}
						break;
					case 1: //file playback
					case 2: //time playback
					case 6: //getfileframe byfile
					case 7: //getfileframe bytime
						{
							EnterCriticalSection(&p->m_hcs);
							struct IFLY_MediaRcvPara_t *p_MediaRcvPara = NULL;
							
							p_MediaRcvPara = (struct IFLY_MediaRcvPara_t *)p->m_mediarcv[i].pContent;
							
							if (p_MediaRcvPara)
							{
// 								char bufname[256] = {0};
// 								sprintf(bufname, "E:\\frame_%d.txt",(int)p_MediaRcvPara);
// 								FILE *fp = fopen(bufname,"at");
// 								fprintf(fp,"begin\r\n");
// 								fclose(fp);
								if ((hdr.m_byMediaType == MEDIA_TYPE_H264) || (hdr.m_byMediaType == MEDIA_TYPE_H264_FPGA))
								{
									deal_playcb_video_rcv(&frame, p_MediaRcvPara, p_MediaRcvPara->p_frmdecBuf);
								} 
								else if ((hdr.m_byMediaType == MEDIA_TYPE_PCMU)
									||(hdr.m_byMediaType == MEDIA_TYPE_ADPCM)
									||(hdr.m_byMediaType == MEDIA_TYPE_ADPCM_HISI))
								{
									BYTE audiobuf[MAX_AUDIO_DECLEN] = {0};
									deal_playcb_audio_rcv(&frame, p_MediaRcvPara, audiobuf);				
								}
// 								fp = fopen(bufname,"at");
// 								fprintf(fp,"end\r\n");
// 								fclose(fp);
							}
							LeaveCriticalSection(&p->m_hcs);
						}
						break;
					case 5:	//voip
						{
							struct IFLY_MediaRcvPara_t *p_MediaRcvPara = NULL;
							
							for (int index=0; index < MAX_VOIP_NUM; index++)
							{
								if (p->voip_rcv[index].prcv_t == &p->m_mediarcv[i])
								{
									p_MediaRcvPara = &p->voip_rcv[index];
									break;
								}
							}
							DealMediaFrame(&frame, (u32)p_MediaRcvPara);
						}
						break;
					}//switch (p->m_mediarcv[i].req.command)
					
					continue;
				}//if(ret == hdr.m_dwDataSize)
				
over:
				//FILE *fp = fopen("C:\\testupdate.txt","ab+");
				//fprintf(fp,"码流线程:(0x%08x,%d)\r\n",p,p->m_mediarcv[i].sockfd);
				//fclose(fp);
				
				closesocket(p->m_mediarcv[i].sockfd);
				p->m_mediarcv[i].sockfd = INVALID_SOCKET;
				
				switch (p->m_mediarcv[i].req.command)
				{
				case 1:
				case 2:
				case 6:
				case 7:
					{
						p->m_mediarcv[i].bStart = 0;
					}
					break;
				}
			}//if((p->m_mediarcv[i].sockfd != INVALID_SOCKET) && (p->m_mediarcv[i].bStart) && FD_ISSET(p->m_mediarcv[i].sockfd,&set))
			
//			Sleep(0);//csp modify
		}//for(i=0; i<MEDIA_LINK_CLIENT; i++)
		
		//Sleep(1);
	}
	
	//csp modify
	if(frmaebuf != NULL)
	{
		delete []frmaebuf;
		frmaebuf = NULL;
	}
	
	return 0;
}

BOOL SetRcvTcpFrame(struct NETDVR_INNER_t *p, ifly_stearmrcv_t** pprcv_t, ifly_TCP_Stream_Req req, BOOL bRcv, void* pContent, u32 *pErrcode)
{
	if (!pprcv_t || !p)
	{
		return FALSE;
	}
	
	if (bRcv)
	{
		*pprcv_t = AddStreamConnect(p, req, pContent, pErrcode);
		if (*pprcv_t)
		{
			return TRUE;
		}
	} 
	else
	{
		DelStreamConnect(*pprcv_t);
		return TRUE;
	}
	return FALSE;
}

#ifdef DEC_AVCDEC
void _stdcall MsgOut(void *user , char * msg)
{

}

void *my_aligned_malloc(int size, int alignment)
{
	void* ptr = malloc(size + alignment);
	
	if(ptr)
    {
		void* aligned = (void*)(((long)ptr + alignment) & ~(alignment - 1));
		((void**)aligned)[-1] = ptr;
		
		return aligned;
    }
	else
		return NULL;
}

void *my_aligned_free(void *paligned)
{
	//delete [ ]paligned;
	
	void* ptr = ((void**)paligned)[-1];
	free(ptr);
	return ptr;
}
#endif

//#define CONNECTDEBUG

BOOL StartStreamConnect( ifly_stearmrcv_t* prcv_t, u32 ip, u16 port, ifly_TCP_Stream_Req req, u32 *pErrcode )
{
	if (!prcv_t)
	{
		return FALSE;
	}
	
	struct sockaddr_in server;
	struct TCP_KEEPALIVE klive;
	struct TCP_KEEPALIVE outKeepAlive = {0,0,0};
	unsigned long ulBytesReturn = 0;
	
	int retConnect = 0;
	int j = 0;
	int err;
	int ret;
	
	prcv_t->sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (prcv_t->sockfd == INVALID_SOCKET)
	{
#ifdef CONNECTDEBUG
		FILE * fp = fopen("c:\\testconnect.txt", "at");
		struct in_addr in;
		in.s_addr = ip;
		fprintf(fp, "socket err ip:%s, port:%d, command:%d, chn:%d \n", inet_ntoa(in), port, req.command, req.Monitor_t.chn);
		fflush(fp);
		fclose(fp);
#endif
		
		return FALSE;
	}
	
	server.sin_addr.s_addr=ip;
	server.sin_family=AF_INET;
	server.sin_port=htons(port);
	
	for (j=0; j<3; j++)
	{
		retConnect = connect(prcv_t->sockfd,(struct sockaddr*)&server,sizeof(server));
		if (retConnect == 0)
		{
			break;
		}
	}
	
	err = WSAGetLastError();
	
	if(retConnect)
	{
#ifdef CONNECTDEBUG
		FILE * fp = fopen("c:\\testconnect.txt", "at");
		struct in_addr in;
		in.s_addr = ip;
		fprintf(fp, "connect err = %d, ip:%s, port:%d, command:%d, chn:%d \n", err, inet_ntoa(in), port, req.command, req.Monitor_t.chn);
		fflush(fp);
		fclose(fp);
#endif
		closesocket(prcv_t->sockfd);
		prcv_t->sockfd = INVALID_SOCKET;
		return FALSE;
	}
	
	struct linger m_sLinger;
	m_sLinger.l_onoff = 1;  //(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	m_sLinger.l_linger = 0; //(容许逗留的时间为0秒)
	setsockopt(prcv_t->sockfd,SOL_SOCKET,SO_LINGER,(char*)&m_sLinger,sizeof(struct linger));
	
	ret = SendTcpConnHead(prcv_t->sockfd, 0x2);
	if (ret < sizeof(ifly_ProtocolHead_t))
	{
#ifdef CONNECTDEBUG
		FILE * fp = fopen("c:\\testconnect.txt", "at");
		struct in_addr in;
		in.s_addr = ip;
		fprintf(fp, "SendTcpConnHead err ip:%s, port:%d, command:%d, chn:%d \n", inet_ntoa(in), port, req.command, req.Monitor_t.chn);
		fflush(fp);
		fclose(fp);
#endif
		closesocket(prcv_t->sockfd);
		prcv_t->sockfd = INVALID_SOCKET;
		return FALSE;
	}
	
	klive.onoff = 1;//启用保活
	klive.keepalivetime = 5000;
	klive.keepaliveinterval = 1000 * 5;//重试间隔为5秒 Resend if No-Reply
	
	ret = WSAIoctl(
		prcv_t->sockfd,
		SIO_KEEPALIVE_VALS,
		&klive,
		sizeof(klive),
		&outKeepAlive,
		sizeof(outKeepAlive),
		(unsigned long *)&ulBytesReturn,
		0,
		NULL
		);
	
	ret = loopsend(prcv_t->sockfd,(char *)&req,sizeof(ifly_TCP_Stream_Req));
	if (ret > 0)
	{
		int ret = 0;
		char buf[512] = {0};
		
		ret = looprecv(prcv_t->sockfd,buf,sizeof(ifly_TCP_Stream_Ack));
		if (ret <=0)
		{
			
		}
		
		if (ret == sizeof(ifly_TCP_Stream_Ack))
		{
			ifly_TCP_Stream_Ack ack;
			memcpy(&ack, buf,sizeof(ifly_TCP_Stream_Ack));
			ack.ackid = ntohl(ack.ackid);
			ack.errcode = ntohl(ack.errcode);
			if (ack.errcode == 0)
			{
				if (req.command == 4)
				{
					int nNetTimeout=1000*5; //5s
					setsockopt(prcv_t->sockfd, SOL_SOCKET,SO_SNDTIMEO, (char *)&nNetTimeout,sizeof(int));
				}
				if (req.command == 5)
				{
					int nNetTimeout=64; //64ms
					setsockopt(prcv_t->sockfd, SOL_SOCKET,SO_SNDTIMEO, (char *)&nNetTimeout,sizeof(int));
				}
				if (req.command == 8)
				{
					int nNetTimeout=1000*5; //5s
					setsockopt(prcv_t->sockfd, SOL_SOCKET,SO_SNDTIMEO, (char *)&nNetTimeout,sizeof(int));
				}
				
				prcv_t->linkid = ack.ackid;
				memcpy(&prcv_t->req, &req,sizeof(ifly_TCP_Stream_Req));
				prcv_t->bStart = 1;
				
#ifdef CONNECTDEBUG
				FILE * fp = fopen("c:\\testconnect.txt", "at");
				struct in_addr in;
				in.s_addr = ip;
				fprintf(fp, "okok sockfd:%d, ip:%s, port:%d, command:%d, chn:%d\n", prcv_t->sockfd, inet_ntoa(in), port, req.command, req.Monitor_t.chn);
				fflush(fp);
				fclose(fp);
#endif
				return TRUE;
			}
			else
			{
#ifdef CONNECTDEBUG
				FILE * fp = fopen("c:\\testconnect.txt", "at");
				struct in_addr in;
				in.s_addr = ip;
				fprintf(fp, "ack err:%d ip:%s, port:%d, command:%d, chn:%d \n", ack.errcode, inet_ntoa(in), port, req.command, req.Monitor_t.chn);
				fflush(fp);
				fclose(fp);
#endif
				closesocket(prcv_t->sockfd);
				prcv_t->sockfd = INVALID_SOCKET;
				if (pErrcode)
				{
					*pErrcode = ack.errcode;
				}				
			}
		}
		else
		{
#ifdef CONNECTDEBUG
			FILE * fp = fopen("c:\\testconnect.txt", "at");
			struct in_addr in;
			in.s_addr = ip;
			fprintf(fp, "rcvack err ip:%s, port:%d, command:%d, chn:%d \n", inet_ntoa(in), port, req.command, req.Monitor_t.chn);
			fflush(fp);
			fclose(fp);
#endif
			closesocket(prcv_t->sockfd);
			prcv_t->sockfd = INVALID_SOCKET;
		}
	}
	else
	{
#ifdef CONNECTDEBUG
		FILE * fp = fopen("c:\\testconnect.txt", "at");
		struct in_addr in;
		in.s_addr = ip;
		fprintf(fp, "sendreq err ip:%s, port:%d, command:%d, chn:%d \n", inet_ntoa(in), port, req.command, req.Monitor_t.chn);
		fflush(fp);
		fclose(fp);
#endif
		closesocket(prcv_t->sockfd);
		prcv_t->sockfd = INVALID_SOCKET;
	}
	
	return FALSE;
}

ifly_stearmrcv_t* AddStreamConnect(struct NETDVR_INNER_t *p, ifly_TCP_Stream_Req req, void* pContent, u32 *pErrcode)
{
	if (NULL == p)
	{
		return NULL;
	}

	for (int i=0; i<MEDIA_LINK_CLIENT; i++ )
	{
		if(0 == p->m_mediarcv[i].bStart)
		{
			if (StartStreamConnect(&p->m_mediarcv[i], p->si.deviceIP, p->si.devicePort, req, pErrcode))
			{
				p->m_mediarcv[i].pContent = pContent;
				return &p->m_mediarcv[i];
			}
			break;
		}
	}

	return NULL;
}

void DelStreamConnect( ifly_stearmrcv_t* prcv_t )
{
	if (prcv_t)
	{
		//FILE *fp = fopen("C:\\testupdate.txt","ab+");
		//fprintf(fp,"^^^DelStreamConnect:(0x%08x,%d)\r\n",prcv_t,prcv_t->sockfd);
		//fclose(fp);
		
		prcv_t->bStart = 0;
		closesocket(prcv_t->sockfd);
		memset(prcv_t, 0, sizeof(ifly_stearmrcv_t));
		prcv_t->sockfd = INVALID_SOCKET;
	}
}

int GetNalheadSize(u8* pbuf)
{
	int size = 0;
	while(*pbuf == 0x00)
	{
		pbuf++;
		size++;
	}
	if(*pbuf == 0x01)
	{
		size += 1;
	}
	else
	{
		size = 0;
	}
	return size;
	
}

enum
{
	PARSE_STATE_INIT=0,
		PARSE_STATE_A,
		PARSE_STATE_B
};


typedef struct _byteStream_s 
{
	long bytesRead;               /* The number of bytes read from the file */
	unsigned char *bitbuf;        /* Buffer for stream bits */
	int bitbufLen;                /* Size of the bit buffer in bytes */
	int bitbufDataPos;
	int bitbufDataLen;            /* Length of all the data in the bit buffer in bytes */
	
	unsigned char *bitbufNalunit; /* Pointer to first NAL unit in the bitbuffer */
	int bitbufNalunitLen;         /* Length of the first NAL unit in the bit buffer in bytes,
																including variable size start code, nal head byte and the
								  RBSP payload. It will help to find the RBSP length. */
} byteStream_s;

static int findStartCode(byteStream_s *str)
{
	int numZeros;
	int startCodeFound;
	int i;
	int currByte;
	
	numZeros       = 0;
	startCodeFound = 0;
	
	i = str->bitbufDataPos;
	
	/* Find sequence of 0x00 ... 0x00 0x01 */
	
	while (i < str->bitbufDataLen) {
		currByte = str->bitbuf[i];
		i++;
		
		if (currByte > 1) /* If current byte is > 1, it cannot be part of a start code */
			{
				numZeros = 0;

				//add by cj@20100419  每段最多只找前30个字节
				if (i-str->bitbufDataPos > 64)
				{
					startCodeFound = 1;
					return -1;
				}//add by cj@20100419
			}
		else if (currByte == 0)  /* If current byte is 0, it could be part of a start code */
			{
				numZeros++;
			}
		else {                    /* currByte == 1 */
			if (numZeros > 1) {  /* currByte == 1. If numZeros > 1, we found a start code */
				startCodeFound = 1;
				return i - 3;
			}
			numZeros = 0;
		}
	}
	return -1;
}

#ifdef DEC_AVCDEC
int _stdcall My_avcdDecodeOneNal( avcdDecoder_t *dec, unsigned char *encbuf, int encdatalen, avcdYUVbuffer_s *outBuf , avcdYUVbuffer_s * refBuf, unsigned char keyflag )
{
	int result = -1;
	//if(nalUnitLen >30 && keyflag)
	if(3 == keyflag)
	{
#if 1
		int index = 0;
		byteStream_s strByte;
		int nalPos , nalIndex = 0;
		int nalPosTab[5] = {0};

		strByte.bitbuf				= encbuf;
		strByte.bytesRead           = encdatalen;
		strByte.bitbufLen           = encdatalen;
		strByte.bitbufDataLen       = strByte.bytesRead;
		strByte.bitbufDataPos       = 0;
		strByte.bitbufNalunitLen    = 0;

		FILE * fp = NULL;
		//fp = fopen("c:\\My_avcdDecodeOneNal.txt", "at");
		DWORD dwstart = GetTickCount();
		DWORD dwend = 0;
		for(;;)
		{
			nalPos = findStartCode(&strByte);
			if(nalPos < 0)
			{
				nalPosTab[nalIndex++] = strByte.bitbufDataLen;
				break;
			}
			nalPosTab[nalIndex++] = nalPos;
			strByte.bitbufDataPos = nalPos + 3;
		}

		if (fp)
		{
			dwend = GetTickCount();
			fprintf(fp, "findStartCode, dur = %d\n", dwend-dwstart);
			fflush(fp);
			dwstart = dwend;
		}

		while (1) 
		{
			if (result != AVCD_OK_STREAM_NOT_DECODED) 
			{
				strByte.bitbufNalunit    = strByte.bitbuf + nalPosTab[index];
				strByte.bitbufNalunitLen = nalPosTab[index + 1] - nalPosTab[index];
				index++;
				if(strByte.bitbufNalunitLen <= 0)
				{
					strByte.bitbufNalunit = 0;
					strByte.bitbufNalunitLen = 0;
					break;
				}
				
				if(index > 4)break;
			}
			else
			{
				// printf("!!!!!!!!!!!!!!!!!!!!!!!AVCD_OK_STREAM_NOT_DECODED\n");
			}
			
			/* Decode one NAL unit */
			result = avcdDecodeOneNal(dec, strByte.bitbufNalunit,
				strByte.bitbufNalunitLen, outBuf , refBuf);

			if (fp)
			{
				dwend = GetTickCount();
				fprintf(fp, "avcdDecodeOneNal, dur = %d\n", dwend-dwstart);
				fflush(fp);
				dwstart = dwend;
			}
			
			if(outBuf->picType == AVCD_PIC_SPS)
			{
				// printf("hehe0.1\n");
			}
			
			if(outBuf->picType == AVCD_PIC_PPS)
			{
				// printf("hehe0.2\n");
			}
			
			//printf("hehe1\n");
			if(result == AVCD_OK)
			{
				if (fp)
				{
					fclose(fp);
					fp = NULL;
				}

				return result;
			}
		}
#else		
	int nal_size = GetNalheadSize(encbuf);
	int i_data_size=encdatalen;
	int i_cur=nal_size;
	int i_cliplen=0;
	int decoffset = 0;
	int b_decode_finish=0;
	int i_parse_state;
	int b_find_nal, i_start_code_size;
	//[2] main loop
	while (!b_decode_finish)
	{
		i_parse_state=PARSE_STATE_INIT;
		
		//[2.2] extract a nal
		b_find_nal=0;
		
		while (i_cur<i_data_size && !b_find_nal)
		{
			switch(i_parse_state) 
			{
			case PARSE_STATE_INIT:
					if (0==encbuf[i_cur])
					{
						i_parse_state=PARSE_STATE_A;
						//printf("*****kakaka\n");
					}
					else
						i_parse_state=PARSE_STATE_INIT;
					break;
				case PARSE_STATE_A:
					if (0==encbuf[i_cur])
					{
						i_start_code_size=2;
						i_parse_state=PARSE_STATE_B;
					}
					else
						i_parse_state=PARSE_STATE_INIT;
					break;
				case PARSE_STATE_B:
					if (0==encbuf[i_cur])
					{
						i_start_code_size++;
						//printf("kakaka\n");
						i_parse_state=PARSE_STATE_B;
					}
					else if (1==encbuf[i_cur])
					{
						i_start_code_size++;
						b_find_nal=1;
					}
					else
					{
						//printf("hehe\n");
						i_parse_state=PARSE_STATE_INIT;
					}
					break;
				default:
					//printf("*****hehe\n");
					//can't jump here
					break;
				}
				i_cur++;//!!
			}//end of while (i_cur<i_data_size && !b_find_nal)
			
			
			
			//judge decode finish
			if (i_cur>=i_data_size)
			{
				
				b_decode_finish=1;
				
			}
			
			
			//update the data index
			i_cliplen=(int)i_cur-nal_size-decoffset;
			
			// printf("i_nal_start: %d\n ", i_cliplen);
			if (dec)
			{
				result = avcdDecodeOneNal(dec, encbuf+decoffset,i_cliplen, outBuf, refBuf);
			}

			decoffset += i_cliplen;
			
			// printf("resualt %d m_recoBuf.picType : %d\n ", result, m_recoBuf.picType);
		}
#endif
	}
	else
	{
		if(dec)
		{
			result = avcdDecodeOneNal(dec, encbuf, encdatalen, outBuf, refBuf);
		}
	}
	return result;
}
#endif

BOOL initMsgQ(ifly_msgQ_t *mq,u32 unitNum,u32 unitLen)
{
	if(mq && unitNum && unitLen)
	{
		mq->rpos    = 0;
		mq->wpos    = 0;
		mq->unitNum = unitNum;
		mq->unitLen = unitLen;
		mq->buf = (u8 *)malloc(unitNum*(sizeof(u32)+unitLen));
		if(mq->buf == NULL)
		{
			//printf("initMsgQ:malloc failed\n");
			return FALSE;
		}
//		pthread_mutex_init(&mq->lock,NULL);
		mq->lock = CreateMutex(NULL, FALSE, NULL);
//		sem_init(&mq->rsem,0,unitNum);
		mq->rsem = CreateSemaphore(NULL, 0, unitNum, NULL);	
//		sem_init(&mq->wsem,0,unitNum);
		mq->wsem = CreateSemaphore(NULL, unitNum, unitNum, NULL);	
		return TRUE;
	}
//	printf("initMsgQ:param error\n");
	return FALSE;
}

int readMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 readLen)
{
	int realLen;
	if(mq && pBuf && readLen)
	{
		//sem_wait(&mq->rsem);
		WaitForSingleObject(mq->rsem,INFINITE);

		//pthread_mutex_lock(&mq->lock);
		WaitForSingleObject(mq->lock,INFINITE);

		memcpy(&realLen,mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen),sizeof(u32));
		realLen = min(realLen,readLen);
		memcpy(pBuf,mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen)+sizeof(u32),realLen);
		
		mq->rpos = (mq->rpos + 1) % mq->unitNum;
		
		//pthread_mutex_unlock(&mq->lock);
		ReleaseMutex(mq->lock);

		//sem_post(&mq->wsem);
		ReleaseSemaphore(mq->wsem, 1, NULL);
		return realLen;
	}
//	printf("readMsgQ:param error\n");
	return -1;
}

int writeMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 writeLen)
{
	if(mq && pBuf && writeLen <= mq->unitLen)
	{
		//sem_wait(&mq->wsem);
		WaitForSingleObject(mq->wsem,INFINITE);
		//pthread_mutex_lock(&mq->lock);
		WaitForSingleObject(mq->lock,INFINITE);

		memcpy(mq->buf+mq->wpos*(sizeof(u32)+mq->unitLen),&writeLen,sizeof(u32));
		memcpy(mq->buf+mq->wpos*(sizeof(u32)+mq->unitLen)+sizeof(u32),pBuf,writeLen);
		
		mq->wpos = (mq->wpos + 1) % mq->unitNum;
		
		//sem_post(&mq->rsem);
		ReleaseSemaphore(mq->rsem, 1, NULL);

		//pthread_mutex_unlock(&mq->lock);
		ReleaseMutex(mq->lock);

		return writeLen;
	}
//	printf("writeMsgQ:param error,mq=%x,pBuf=%x,writeLen=%d,mq->unitLen=%d\n",(int)mq,(int)pBuf,writeLen,mq->unitLen);
	return -1;
}

BOOL destroyMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
		//pthread_mutex_destroy(&mq->lock);
		CloseHandle(mq->lock);
		//sem_destroy(&mq->rsem);
		CloseHandle(mq->rsem);
		//sem_destroy(&mq->wsem);
		CloseHandle(mq->wsem);
		if(mq->buf != NULL)
		{
			free(mq->buf);
			mq->buf = NULL;
		}
		return TRUE;
	}
//	printf("destroyMsgQ:param error\n");
	return FALSE;
}

#if 0
BOOL GetMsgQReadInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pReadLen)
{
	if(mq && ppBuf && pReadLen)
	{
		// sem_wait(&mq->rsem);
		WaitForSingleObject(mq->rsem,INFINITE);

		//pthread_mutex_lock(&mq->lock);
		
		memcpy(pReadLen,mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen),sizeof(u32));
		*ppBuf = mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen)+sizeof(u32);
		
		return TRUE;
	}
//	printf("GetMsgQReadInfo:param error\n");
	return FALSE;
}
#else
BOOL GetMsgQReadInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pReadLen)
{
	if(mq && ppBuf && pReadLen)
	{
		// sem_wait(&mq->rsem);
		DWORD dwRet = WaitForSingleObject(mq->rsem,/*INFINITE*/40);
		if (dwRet != WAIT_OBJECT_0)
		{
			return FALSE;
		}
		//pthread_mutex_lock(&mq->lock);
		//cj@20121224
		WaitForSingleObject(mq->lock,INFINITE);

		memcpy(pReadLen,mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen),sizeof(u32));
		*ppBuf = mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen)+sizeof(u32);
		
		return TRUE;
	}
	//	printf("GetMsgQReadInfo:param error\n");
	return FALSE;
}
#endif


BOOL skipReadMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
		mq->rpos = (mq->rpos + 1) % mq->unitNum;
		
		//pthread_mutex_unlock(&mq->lock);		
// 		sem_post(&mq->wsem);
		ReleaseSemaphore(mq->wsem, 1, NULL);
		
		//cj@20121224
		ReleaseMutex(mq->lock);

		return TRUE;
	}
// 	printf("skipReadMsgQ:param error\n");
	return FALSE;
}



#if 0
BOOL GetMsgQWriteInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pWriteLen)
{
	if(mq && ppBuf && pWriteLen)
	{
//		sem_wait(&mq->wsem);
		WaitForSingleObject(mq->wsem,INFINITE);
// 		pthread_mutex_lock(&mq->lock);
		WaitForSingleObject(mq->lock,INFINITE);
		int tmpWPos = mq->wpos;
		mq->wpos = (mq->wpos + 1) % mq->unitNum;
		
		//pthread_mutex_unlock(&mq->lock);
		
		memcpy(mq->buf+tmpWPos*(sizeof(u32)+mq->unitLen),pWriteLen,sizeof(u32));
		*ppBuf = mq->buf+tmpWPos*(sizeof(u32)+mq->unitLen)+sizeof(u32);
		
		return TRUE;
	}
// 	printf("GetMsgQWriteInfo:param error\n");
	return FALSE;
}
#else
BOOL GetMsgQWriteInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pWriteLen)
{
	if(mq && ppBuf && pWriteLen)
	{
		//		sem_wait(&mq->wsem);
		DWORD dwRet = WaitForSingleObject(mq->wsem,/*INFINITE*/0);
		if (dwRet != WAIT_OBJECT_0)
		{
			return FALSE;
		}
		// 		pthread_mutex_lock(&mq->lock);
		WaitForSingleObject(mq->lock,INFINITE);
		int tmpWPos = mq->wpos;
		mq->wpos = (mq->wpos + 1) % mq->unitNum;
		
		//pthread_mutex_unlock(&mq->lock);
		
		memcpy(mq->buf+tmpWPos*(sizeof(u32)+mq->unitLen),pWriteLen,sizeof(u32));
		*ppBuf = mq->buf+tmpWPos*(sizeof(u32)+mq->unitLen)+sizeof(u32);
		
		return TRUE;
	}
	// 	printf("GetMsgQWriteInfo:param error\n");
	return FALSE;
}
#endif


BOOL skipWriteMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
// 		sem_post(&mq->rsem);
		ReleaseSemaphore(mq->rsem, 1, NULL);
// 		pthread_mutex_unlock(&mq->lock);
		ReleaseMutex(mq->lock);
		return TRUE;
	}
// 	printf("skipWriteMsgQ:param error\n");
	return FALSE;
}

DWORD WINAPI DecVideoFrameThread( LPVOID lpParam )
{
// 	FILE *fp = fopen("c:\\decthread.txt", "at");
//	while (g_hDecVideoThreadRun)
	while(1)
	{
		if (!g_bDecVideoThreadRun)
		{
			if (g_hDecThreadEvent)
			{
				SetEvent(g_hDecThreadEvent);
			}
			break;
		}

		u8 *pBuf = NULL;
		u32 buflenth = 0;
		BOOL bRet = GetMsgQReadInfo(&g_decmsgQ, &pBuf,&buflenth);
		if (bRet)
		{
// 			if (fp)
// 			{
// 				fprintf(fp, "GetMsgQReadInfo ok \r\n");
// 				fflush(fp);
// 			}

			FrameMsg_t* pReadFrame = (FrameMsg_t*)pBuf;
			if (g_dvrExist[pReadFrame->dvr_id])
			{
				FRAMEHDR FrmHdr = {0};
				
				FrmHdr.m_byMediaType = pReadFrame->frame.m_byMediaType; 
				FrmHdr.m_dwPreBufSize = pReadFrame->frame.m_dwPreBufSize;
				FrmHdr.m_dwDataSize = pReadFrame->frame.m_dwDataSize;
				FrmHdr.m_byFrameRate = pReadFrame->frame.m_byFrameRate;
				FrmHdr.m_dwFrameID = pReadFrame->frame.m_dwFrameID;
				FrmHdr.m_dwTimeStamp = pReadFrame->frame.m_dwTimeStamp;
				memcpy(&FrmHdr.m_tVideoParam, &pReadFrame->frame.m_tVideoParam, sizeof(FrmHdr.m_tVideoParam));
				FrmHdr.m_pData = pReadFrame->frame.m_pData;
				
				IFLY_MediaRcvPara_t *p_MediaRcvPara = pReadFrame->p_MediaRcvPara;
				
				//if (ChkMediaRcvPointer(g_dvr_pool.p_dvr[pReadFrame->dvr_id], p_MediaRcvPara))
				{
					DealMediaFrame(&FrmHdr, (u32)p_MediaRcvPara);
				}
			}

			skipReadMsgQ(&g_decmsgQ);
		}
		else
		{
			// Sleep(20);
		}

		Sleep(0);
	}

// 	if (fp)
// 	{
// 		fprintf(fp, "over \r\n");
// 		fflush(fp);
// 		fclose(fp);
// 	}
	return 0;
}

DWORD WINAPI DownLoadThread( LPVOID lpParam )
{
	struct IFLY_FileReciever_t *p_file_reciever = (IFLY_FileReciever_t *)lpParam;
	if (!p_file_reciever)
	{
		return 0;
	}
	
	if (!p_file_reciever->prcv_t)
	{
		return 0;
	}
	
	if (p_file_reciever->req.command != 3)
	{
		return 0;
	}

	if((p_file_reciever->prcv_t->sockfd != INVALID_SOCKET) && (p_file_reciever->prcv_t->bStart))
	{
		char dlbuf[MAX_DL_BUF] = {0};
		int ret = 0;
		int remian = p_file_reciever->rcv_size;
		int recvlen = 0;

		char svPathFileName[PATH_LEN_MAX+FILENAME_LEN_MAX+1] = {0};
		sprintf(svPathFileName, "%s\\%s", p_file_reciever->save_path, p_file_reciever->save_filename);
		char inFile[PATH_LEN_MAX+FILENAME_LEN_MAX+1] = {0};
		char FileSufName[20] = {0};
		memcpy(FileSufName,p_file_reciever->save_filename+strlen(p_file_reciever->save_filename)-4,4);
		BOOL bIsAviFile = !(_stricmp(FileSufName,".avi"));
		if (bIsAviFile)
		{
			SYSTEMTIME sysTime;
			GetSystemTime(&sysTime);
			char timeFileName[FILENAME_LEN_MAX+1] = {0};
			sprintf(timeFileName,"%x%04d%02d%02d%02d%02d%02d%02d.ifv",p_file_reciever->serverip, sysTime.wYear,sysTime.wMonth,sysTime.wDay,sysTime.wHour,sysTime.wMinute,sysTime.wSecond,sysTime.wMilliseconds);
			
			memset(p_file_reciever->save_filename, 0, sizeof(p_file_reciever->save_filename));
			strcpy(p_file_reciever->save_filename,timeFileName);
			sprintf(inFile, "%s\\%s", p_file_reciever->save_path, p_file_reciever->save_filename);
		}

		while(remian > 0)
		{
			ret=recv(p_file_reciever->prcv_t->sockfd,dlbuf,MAX_DL_BUF,0);
			if(ret <= 0)
			{
				int err = WSAGetLastError();

				if (bIsAviFile)
				{
					DeleteFile(inFile);
				}
				else
				{
					DeleteFile(svPathFileName);
				}

				if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
				{
					p_file_reciever->p_cb_err((u16)NETDVR_ERR_DOWNLOAD, p_file_reciever->dwContentErr);
				}

				goto over;
			}
			
			if (!p_file_reciever->fp)
			{
				open_reciever_file(p_file_reciever);
			}
			
			if (!p_file_reciever->fp)
			{
				if (bIsAviFile)
				{
					DeleteFile(inFile);
				}
				else
				{
					DeleteFile(svPathFileName);
				}

				if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
				{
					p_file_reciever->p_cb_err((u16)NETDVR_ERR_DOWNLOAD, p_file_reciever->dwContentErr);
				}

				goto over;
			}
			
			p_file_reciever->recieved_size += ret;
			u32 write_size = ret;
			if (p_file_reciever->fp)
			{
				ret = fwrite(dlbuf, 1, write_size, p_file_reciever->fp);
			}
			
			if (ret <= 0)
			{
				if (bIsAviFile)
				{
					DeleteFile(inFile);
				}
				else
				{
					DeleteFile(svPathFileName);
				}

				if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
				{
					p_file_reciever->p_cb_err((u16)NETDVR_ERR_DOWNLOAD, p_file_reciever->dwContentErr);
				}

				goto over;
			}
			else
			{
				if (p_file_reciever->fp)
				{
					fflush(p_file_reciever->fp);
				}
				
				if (p_file_reciever->p_cb_progress)
				{
					struct NETDVR_progressParam_t progress;
					memset(&progress,0,sizeof(NETDVR_progressParam_t));
					progress.curr_pos = p_file_reciever->recieved_size;
					progress.total_size = p_file_reciever->rcv_size;
					p_file_reciever->p_cb_progress(progress, p_file_reciever->dwContentProgress);
				}
			}
			recvlen += ret;
			remian -= ret;
		}

		if (bIsAviFile)
		{		
			if (p_file_reciever->fp)
			{
				fflush(p_file_reciever->fp);
				fclose(p_file_reciever->fp);
				p_file_reciever->fp = NULL;
			}
			
			avi_t* pAviHandle = NULL;
			
		#ifdef _NVR_//csp modify 20121118
			unsigned char buf[512<<10] = {0};
		#else
			unsigned char buf[256<<10] = {0};
		#endif
			
			UINT dwDuration = 0;
			UINT start_time;
			BYTE key;
			int vsize = 0;
			
			TLFILE_t hfile = TL_OpenFile(inFile,TL_FILE_READ);
			if(hfile == NULL)
			{
				DeleteFile(inFile);
		
				if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
				{
					p_file_reciever->p_cb_err((u16)NETDVR_ERR_DOWNLOAD, p_file_reciever->dwContentErr);
				}
				
				goto over;
			}
			
			int width = TL_FileVideoWidth(hfile);
			int height = TL_FileVideoHeight(hfile);
			int total_frames = TL_FileVideoFrameNum(hfile)+TL_FileAudioFrameNum(hfile);

			double frame_rate = TL_FileVideoFrameNum(hfile)*1000.0/TL_FileTotalTime(hfile);

			pAviHandle = AVI_open_output_file(svPathFileName);
			if(!pAviHandle)
			{
				DeleteFile(inFile);
				TL_CloseFile(hfile);

				if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
				{
					p_file_reciever->p_cb_err((u16)NETDVR_ERR_DOWNLOAD, p_file_reciever->dwContentErr);
				}
				
				goto over;
			}
			
			AVI_set_video(pAviHandle,width,height,frame_rate,"H264");

			if (TL_FileHasAudio(hfile) && p_file_reciever->p_audio_property)
			{
				AVI_set_audio(pAviHandle,  1, p_file_reciever->p_audio_property->audioSamplePerSec, 
					p_file_reciever->p_audio_property->audioBitPerSample, WAVE_FORMAT_PCM, 0);
			}
			else
			{
				AVI_set_audio(pAviHandle, 0, 0, 0, 0, 0);
			}
			
			while (total_frames > 0)
			{	
				if (p_file_reciever->p_audio_property)
				{
					u8 mediatype = 0;
					vsize = TL_FileReadOneMediaFrame(hfile,buf,&start_time,&key,&mediatype);
					if (vsize <= 0)
					{
						break;
					}

					if (mediatype == 0) //video
					{
						ret = AVI_write_frame(pAviHandle,(char *)buf,vsize,key);
						if (ret != 0)
						{
							break;
						}
					}
					else //audio
					{
						unsigned char decbuf[1024] = {0};
						int declen = 0;
						TL_FILEDecAudioFrame(hfile, buf, vsize, decbuf, &declen);

						ret = AVI_write_audio(pAviHandle,(char *)decbuf,declen);
						if (ret != 0)
						{
							break;
						}
					}
				
				}
			
				total_frames--;
			}
			
			if (total_frames > 0)
			{
				DeleteFile(svPathFileName);
			}

			AVI_close(pAviHandle);
			TL_CloseFile(hfile);
			DeleteFile(inFile);
		}
	}
over:
	
	int err = WSAGetLastError();
	
	if (p_file_reciever->fp)
	{
		fflush(p_file_reciever->fp);
		fclose(p_file_reciever->fp);
		p_file_reciever->fp = NULL;
	}

	if (p_file_reciever->prcv_t)
	{
		closesocket(p_file_reciever->prcv_t->sockfd);
		p_file_reciever->prcv_t->sockfd = INVALID_SOCKET;
		p_file_reciever->prcv_t->bStart = 0;
	}

	return 0;
}

DWORD WINAPI UpdateThread( LPVOID lpParam )
{
	struct IFLY_Update_t *p_update = (IFLY_Update_t *)lpParam;
	if (!p_update)
	{
		return 0;
	}
	
	//FILE *fp = fopen("C:\\testupdate.txt","ab+");
	//fprintf(fp,"UpdateThread:(0x%08x,%d)\r\n",p_update,p_update->prcv_t->sockfd);
	//fclose(fp);
	
	if (!p_update->prcv_t)
	{
		if (p_update->p_cb_err)
		{
			p_update->p_cb_err(NETDVR_ERR_NETSND, p_update->dwContentErr);
		}
		
		p_update->b_updating = 0;
		p_update->status = NETDVR_UPDATE_STOPPED;
		
		return 0;
	}
	
	if (p_update->req.command != 4)
	{
		if (p_update->p_cb_err)
		{
			p_update->p_cb_err(NETDVR_ERR_NETSND, p_update->dwContentErr);
		}
		
		p_update->b_updating = 0;
		p_update->status = NETDVR_UPDATE_STOPPED;
		
		return 0;
	}


	if((p_update->prcv_t->sockfd != INVALID_SOCKET) && (p_update->prcv_t->bStart))
	{
		char upbuf[MAX_DL_BUF] = {0};
		int ret = 0;
		if (!p_update->fp)
		{
			p_update->fp = open_update_file(p_update);
		}
		
		if (!p_update->fp)
		{
			if (p_update->p_cb_err)
			{
				p_update->p_cb_err(NETDVR_ERR_NETSND, p_update->dwContentErr);
			}
			
			p_update->b_updating = 0;
			p_update->status = NETDVR_UPDATE_STOPPED;
			
			goto over;
		}
		p_update->updated_size = 0;
		int remian = p_update->file_size;
		p_update->b_updating = 1;
		//FD_SET ReadSockFd;    //读集合
	    //FD_SET WriteSockFd;	//写集合
		//struct timeval timeout={3,0}; //select等待1秒，1秒轮询，要非阻塞就置0
		while(remian > 0)
		{
			//read
			fseek(p_update->fp,p_update->updated_size,SEEK_SET);//有可能收到读信号的时候没有发送数据
			int len = loopread(p_update->fp, upbuf, min(MAX_DL_BUF, p_update->file_size-p_update->updated_size));
			if (len <= 0)
			{
				if (p_update->p_cb_err)
				{
					p_update->p_cb_err(NETDVR_ERR_NETSND, p_update->dwContentErr);
				}
				
				if (p_update->fp)
				{
					fflush(p_update->fp);
					fclose(p_update->fp);
					p_update->fp = NULL;
				}
				
				p_update->b_updating = 0;
				p_update->status = NETDVR_UPDATE_STOPPED;
				
				goto over;
			}
			
			ret = loopsend(p_update->prcv_t->sockfd,upbuf,len);
			if(ret <= 0)
			{
				if (p_update->p_cb_err)
				{
					p_update->p_cb_err(NETDVR_ERR_NETSND, p_update->dwContentErr);
				}
				
				if (p_update->fp)
				{
					fflush(p_update->fp);
					fclose(p_update->fp);
					p_update->fp = NULL;
				}
				
				p_update->b_updating = 0;
				p_update->status = NETDVR_UPDATE_STOPPED;
				
				goto over;
			}
			
			p_update->updated_size += ret;
			remian -= ret;
		}
		
		//csp modify
		while(1)
		{
			fd_set ReadSockFd;
			FD_ZERO(&ReadSockFd);
			FD_SET(p_update->prcv_t->sockfd, &ReadSockFd);
			struct timeval timeout;
			timeout.tv_sec = 30;
			timeout.tv_usec = 0;
			
			//char szTmp[32];
			//sprintf(szTmp, "升级句柄:(0x%08x,%d)\n",p_update,p_update->prcv_t->sockfd);
			//MessageBox(NULL,szTmp,NULL,MB_OK);
			
			//FILE *fp = fopen("C:\\testupdate.txt","ab+");
			//fprintf(fp,"升级句柄:(0x%08x,%d)\r\n",p_update,p_update->prcv_t->sockfd);
			//fclose(fp);
			
			int rtn = select(FD_SETSIZE, &ReadSockFd, NULL, NULL, &timeout);
			if (rtn < 0)
			{
				break;
			}
			if (rtn == 0)
			{
				break;
			}
			if (rtn > 0 && FD_ISSET(p_update->prcv_t->sockfd, &ReadSockFd))
			{
				ifly_TCP_Pos uppos;
				memset(&uppos,0,sizeof(ifly_TCP_Pos));
				
				ret = looprecv(p_update->prcv_t->sockfd,(char*)&uppos,sizeof(ifly_TCP_Pos));
				if (ret > 0)
				{
					u16 err = iflydvr_get_error_code(ntohs(uppos.errcode));
					if (err == NETDVR_IS_UPDATING)
					{
						//char szTmp[32];
						//sprintf(szTmp, "进度信息:(%d,%d)\n",ntohs(uppos.errcode),uppos.pos);
						//MessageBox(NULL,szTmp,NULL,MB_OK);
						
						//FILE *fp = fopen("C:\\testupdate.txt","ab+");
						//fprintf(fp,"进度信息:(%d,%d,%d),升级句柄:(0x%08x,%d)\r\n",ntohs(uppos.errcode),uppos.pos,err,p_update,p_update->prcv_t->sockfd);
						//fclose(fp);
						
						if (p_update->p_cb_progress)
						{
							struct NETDVR_progressParam_t progress;
							memset(&progress,0,sizeof(NETDVR_progressParam_t));
							progress.curr_pos = uppos.pos;
							p_update->p_cb_progress(progress, p_update->dwContentUpdate);
						}
					}
					else
					{
						//char szTmp[32];
						//sprintf(szTmp, "接收错误的进度信息:(%d,%d,%d)\n",ntohs(uppos.errcode),uppos.pos,err);
						//MessageBox(NULL,szTmp,NULL,MB_OK);
						
						//FILE *fp = fopen("C:\\testupdate.txt","ab+");
						//fprintf(fp,"接收错误的进度信息:(%d,%d,%d),升级句柄:(0x%08x,%d)\r\n",ntohs(uppos.errcode),uppos.pos,err,p_update,p_update->prcv_t->sockfd);
						//fclose(fp);
						
						if (p_update->p_cb_err)
						{
							p_update->p_cb_err(err, p_update->dwContentErr);
						}
						
						break;
					}
				}
				else
				{
					//LPVOID lpMsgBuf;
					//FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					//	NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
					//	(LPTSTR) &lpMsgBuf, 0, NULL);
					
					//char szTmp[256];
					//sprintf(szTmp, "接收进度信息失败,返回值:%d,失败原因:%s,sockfd:%d\n",ret,(char *)lpMsgBuf,p_update->prcv_t->sockfd);
					//MessageBox(NULL,szTmp,NULL,MB_OK);
					
					//FILE *fp = fopen("C:\\testupdate.txt","ab+");
					//fprintf(fp,"接收进度信息失败,返回值:%d,升级句柄:(0x%08x,%d)\r\n",ret,p_update,p_update->prcv_t->sockfd);
					//fclose(fp);
					
					if (p_update->p_cb_err)
					{
						p_update->p_cb_err(NETDVR_ERR_NETRCV, p_update->dwContentErr);
					}
					
					break;
				}
			}
		}
		
		if (p_update->fp)
		{
			fflush(p_update->fp);
			fclose(p_update->fp);
			p_update->fp = NULL;
		}
		
		p_update->b_updating = 0;
		p_update->status = NETDVR_UPDATE_STOPPED;
		
		goto over;
	}
	
over:
	if (p_update->prcv_t)
	{
		closesocket(p_update->prcv_t->sockfd);
		p_update->prcv_t->sockfd = INVALID_SOCKET;
		p_update->prcv_t->bStart = 0;
	}
	
	return 0;
}

DWORD WINAPI SerialRcvThread( LPVOID lpParam )
{
	struct IFLY_SERIAL_T *pSerialRcv = (IFLY_SERIAL_T *)lpParam;
	if (!pSerialRcv)
	{
		return 0;
	}
	
	if (!pSerialRcv->prcv_t)
	{
		return 0;
	}
	
	if (pSerialRcv->req.command != 8)
	{
		return 0;
	}

loop:

	while(pSerialRcv->prcv_t && pSerialRcv->prcv_t->bStart)
	{
		if (pSerialRcv->prcv_t->sockfd != INVALID_SOCKET)
		{
			ifly_serial_hdr hdr;
			int ret = 0;
			char hdrbuf[512] = {0};
			char databuf[MAX_DL_BUF] = {0};

			ret = looprecv(pSerialRcv->prcv_t->sockfd,hdrbuf,sizeof(ifly_serial_hdr));
			if(ret <= 0)
			{
				closesocket(pSerialRcv->prcv_t->sockfd);
				pSerialRcv->prcv_t->sockfd = INVALID_SOCKET;
				goto loop;
			}

			
			memcpy(&hdr, hdrbuf, sizeof(ifly_serial_hdr));
			hdr.dwDataSize = ntohl(hdr.dwDataSize);
			
			ret = looprecv(pSerialRcv->prcv_t->sockfd,databuf,hdr.dwDataSize);
			if(ret <= 0)
			{
				closesocket(pSerialRcv->prcv_t->sockfd);
				pSerialRcv->prcv_t->sockfd = INVALID_SOCKET;
				goto loop;
			}
	
			if (ret == hdr.dwDataSize)
			{
				if ( pSerialRcv->cbSerialDataCallBack )
				{
					pSerialRcv->cbSerialDataCallBack(pSerialRcv->serialport, hdr.byChannel, databuf, hdr.dwDataSize, pSerialRcv->dwContent); 
				}
			}
			else
			{
				closesocket(pSerialRcv->prcv_t->sockfd);
				pSerialRcv->prcv_t->sockfd = INVALID_SOCKET;
				goto loop;
			}
		} 
		else
		{
			Sleep(200);
		}
	}
//over:	
//	closesocket(pSerialRcv->prcv_t->sockfd);
//	pSerialRcv->prcv_t->sockfd = INVALID_SOCKET;
// 	pSerialRcv->prcv_t->bStart = 0;
	return 0;
}

DWORD WINAPI ReConnectThread( LPVOID lpParam )
{
	//printf("ReConnectThread \n");
	while (1)
	{
		if (!g_bReConnectThreadRun)
		{
			if (g_hReConnectThreadEvent)
			{
				SetEvent(g_hReConnectThreadEvent);
			}
			break;
		}

		int nCount = 0;
		DWORD dwStart = GetTickCount();
		for (int i = 0; i< MAX_DVR_NUM; i++)
		{
// 			LockMutex(g_pool_lock);
// 			LockMutex(g_dvr_pool.dvr_lock[i]);
// 			UnlockMutex(g_pool_lock);

			if (g_dvr_pool.p_dvr[i] && g_dvr_pool.p_dvr[i]->bEnableRecon && g_dvr_pool.p_dvr[i]->b_cmdConnectLost)
			{
				nCount ++;

				//信令重连
				if (g_dvr_pool.p_dvr[i]->b_cmdConnectLost && g_dvr_pool.p_dvr[i]->cph)
				{
					if (g_dvr_pool.p_dvr[i]->p_cb_recconn)
					{
						g_dvr_pool.p_dvr[i]->p_cb_recconn(RECONN_BEGIN, g_dvr_pool.p_dvr[i]->dwContentReconn);
					}

					u32 serverip = Domain2IP(g_dvr_pool.p_dvr[i]->SvrDomain);
					if (serverip)
					{
						g_dvr_pool.p_dvr[i]->si.deviceIP = serverip;
					} 
					else
					{
						serverip = g_dvr_pool.p_dvr[i]->si.deviceIP;
					}

					u16 serverport = g_dvr_pool.p_dvr[i]->si.devicePort;
					CPHandle tmpcph = CPConnect(serverip, serverport, g_connect_timeout, NULL);

					if (!g_dvr_pool.p_dvr[i])
					{
						continue;
					}

					if (tmpcph)
					{
						g_dvrExist[i] = 1;
						g_dvr_pool.p_dvr[i]->cph = tmpcph;
						g_dvr_pool.p_dvr[i]->b_cmdConnectLost = FALSE;

						// UnlockMutex(g_dvr_pool.dvr_lock[i]);

						int ret = NETDVR_GetDeviceInfo((int)g_dvr_pool.p_dvr[i], &g_dvr_pool.p_dvr[i]->si);
						
						g_dvr_pool.p_dvr[i]->si.deviceIP = serverip;
						g_dvr_pool.p_dvr[i]->si.devicePort = serverport;
						NETDVR_GetVideoProperty((int)g_dvr_pool.p_dvr[i], &g_dvr_pool.p_dvr[i]->video_property);
						NETDVR_GetAudioProperty((int)g_dvr_pool.p_dvr[i], &g_dvr_pool.p_dvr[i]->audio_property);
						NETDVR_GetVoipProperty((int)g_dvr_pool.p_dvr[i], &g_dvr_pool.p_dvr[i]->voip_property);
						
						NETDVR_loginServer((int)g_dvr_pool.p_dvr[i], &g_dvr_pool.p_dvr[i]->li);
						NETDVR_SetAlarmUpload((int)g_dvr_pool.p_dvr[i], g_dvr_pool.p_dvr[i]->byAlarmUploadFlag);

// 						LockMutex(g_pool_lock);
// 						LockMutex(g_dvr_pool.dvr_lock[i]);
// 						UnlockMutex(g_pool_lock);
#if 0
						//码流重连
						for (int k= 0; k<MEDIA_LINK_CLIENT; k++)
						{
							if (g_dvr_pool.p_dvr[i] && g_dvr_pool.p_dvr[i]->m_mediarcv[k].bStart
								&& (INVALID_SOCKET==g_dvr_pool.p_dvr[i]->m_mediarcv[k].sockfd))
							{
								u16 cmd = 0;
								switch (g_dvr_pool.p_dvr[i]->m_mediarcv[k].req.command)
								{
								case 0:
									cmd = CTRL_CMD_STOPVIDEOMONITOR;
									break;
								case 5:
									cmd = CTRL_CMD_STOPVOIP;
									break;
								case 8:
									cmd = CTRL_CMD_SERIALSTOP;
									break;
								}
								
								u32 id = htonl(g_dvr_pool.p_dvr[i]->m_mediarcv[k].linkid);
								if (id && cmd)
								{
									u8 ackbuf[2048] = {0};
									send_command(g_dvr_pool.p_dvr[i]->cph, cmd, &id, sizeof(id), ackbuf, sizeof(ackbuf), g_connect_timeout);
								}

								if (g_dvr_pool.p_dvr[i])
								{
									StartStreamConnect(&g_dvr_pool.p_dvr[i]->m_mediarcv[k], 
										g_dvr_pool.p_dvr[i]->si.deviceIP, 
										g_dvr_pool.p_dvr[i]->si.devicePort, 
										g_dvr_pool.p_dvr[i]->m_mediarcv[k].req);
								}

							}
							Sleep(1);
						}
#endif
						if (g_dvr_pool.p_dvr[i] && g_dvr_pool.p_dvr[i]->p_cb_recconn)
						{
							g_dvr_pool.p_dvr[i]->p_cb_recconn(RECONN_SUCESS, g_dvr_pool.p_dvr[i]->dwContentReconn);
						}
					}
					else
					{
						if (g_dvr_pool.p_dvr[i] && g_dvr_pool.p_dvr[i]->p_cb_recconn)
						{
							g_dvr_pool.p_dvr[i]->p_cb_recconn(RECONN_FAILED, g_dvr_pool.p_dvr[i]->dwContentReconn);
						}
						Sleep(0);
						continue;
					}
				}

#if 1
				//码流重连
				for (int k= 0; k<MEDIA_LINK_CLIENT; k++)
				{
					if (g_dvr_pool.p_dvr[i] && g_dvr_pool.p_dvr[i]->m_mediarcv[k].bStart
						&& (INVALID_SOCKET==g_dvr_pool.p_dvr[i]->m_mediarcv[k].sockfd))
					{
						u16 cmd = 0;
						switch (g_dvr_pool.p_dvr[i]->m_mediarcv[k].req.command)
						{
						case 0:
							cmd = CTRL_CMD_STOPVIDEOMONITOR;
							break;
						case 5:
							cmd = CTRL_CMD_STOPVOIP;
							break;
						case 8:
							cmd = CTRL_CMD_SERIALSTOP;
							break;
						}
						
						u32 id = htonl(g_dvr_pool.p_dvr[i]->m_mediarcv[k].linkid);
						if (id && cmd)
						{
							u8 ackbuf[2048] = {0};
							int ret = send_command(g_dvr_pool.p_dvr[i]->cph, cmd, &id, sizeof(id), ackbuf, sizeof(ackbuf), g_connect_timeout);
							if (ret)
							{
								//FILE *fp = fopen("C:\\testReconnect.txt","ab+");
								//fprintf(fp,"id %d, send_command_%d g_connect_timeout %d failed, ret %d\r\n", id, cmd, ret, g_connect_timeout);
								//fclose(fp);
							}
							else
							{
								//FILE *fp = fopen("C:\\testReconnect.txt","ab+");
								//fprintf(fp,"id %d, send_command_%d success\r\n", id, cmd);
								//fclose(fp);
							}
							if(cmd == CTRL_CMD_STOPVIDEOMONITOR)
							{
								Sleep(1000);	
							}
						}

// 						if (g_dvr_pool.p_dvr[i])
// 						{
// 							StartStreamConnect(&g_dvr_pool.p_dvr[i]->m_mediarcv[k], 
// 								g_dvr_pool.p_dvr[i]->si.deviceIP, 
// 								g_dvr_pool.p_dvr[i]->si.devicePort, 
// 								g_dvr_pool.p_dvr[i]->m_mediarcv[k].req);
// 						}
					}

					Sleep(1);
				}

				for (k= 0; k<MEDIA_LINK_CLIENT; k++)
				{
					if (g_dvr_pool.p_dvr[i] && g_dvr_pool.p_dvr[i]->m_mediarcv[k].bStart
						&& (INVALID_SOCKET==g_dvr_pool.p_dvr[i]->m_mediarcv[k].sockfd))
					{			
						if (g_dvr_pool.p_dvr[i])
						{
							StartStreamConnect(&g_dvr_pool.p_dvr[i]->m_mediarcv[k], 
								g_dvr_pool.p_dvr[i]->si.deviceIP, 
								g_dvr_pool.p_dvr[i]->si.devicePort, 
								g_dvr_pool.p_dvr[i]->m_mediarcv[k].req);
						}
					}
					
					
					Sleep(1);
				}
#endif
			}
			// UnlockMutex(g_dvr_pool.dvr_lock[i]);
			//Sleep(1);
		}

		if (nCount >0)
		{
			DWORD dwDur = GetTickCount() - dwStart;
			if (dwDur < g_dwReconnectTime)
			{
				Sleep(g_dwReconnectTime-dwDur);
			}
			else
			{
				Sleep(1000);
			}
		} 
		else
		{
			Sleep(300);
		}
	}

	return 0;
}

#ifdef USE_CONNMSG_THREAD
DWORD WINAPI DealConnectMsgThread( LPVOID lpParam )
{
	//::MessageBox(NULL, "test123", "", MB_OK);
	while (1)
	{
		if (!g_bConnMsgThreadRun)
		{
			if (g_hConnMsgThreadEvent)
			{
				//SetEvent(g_hConnMsgThreadEvent);
			}
			break;
		}

		MSG msg;
		PeekMessage(&msg, NULL, WM_USER+100, WM_USER+120, PM_NOREMOVE);
		
		if(GetMessage(&msg,0,0,0)) //get msg from message queue
		{
			switch(msg.message)
			{
			case STARTCONNECTMSG:				
				{
					LockMutex(g_pool_lock);
					u16 dvrid =  HIWORD(msg.wParam);
					if (dvrid>=MAX_DVR_NUM)
					{
						UnlockMutex(g_pool_lock);
						break;
					}
					
					NETDVR_INNER_t *p = g_dvr_pool.p_dvr[dvrid];
					if (!p)
					{
						UnlockMutex(g_pool_lock);
						break;
					}
					UnlockMutex(g_pool_lock);
					
					IFLY_MediaRcvPara_t* pMediaRcv_t = (IFLY_MediaRcvPara_t*)msg.lParam;
					if (p && pMediaRcv_t && !pMediaRcv_t->bOpened)
					{
						int ret = SetRcvTcpFrame(p, &pMediaRcv_t->prcv_t, pMediaRcv_t->req, TRUE, pMediaRcv_t);
						if (ret)
						{
							pMediaRcv_t->bOpened = 1;
						}
					}
				}
				break;
			case STOPCONNECTMSG:				
				{
					LockMutex(g_pool_lock);
					u16 dvrid =  HIWORD(msg.wParam);
					if (dvrid>=MAX_DVR_NUM)
					{
						UnlockMutex(g_pool_lock);
						break;
					}
					
					NETDVR_INNER_t *p = g_dvr_pool.p_dvr[dvrid];
					if (!p)
					{
						UnlockMutex(g_pool_lock);
						break;
					}
					UnlockMutex(g_pool_lock);

					if (!p->b_cmdConnectLost)
					{
						u16 sendcmd = LOWORD(msg.wParam);
						u32 id = msg.lParam;
						BYTE buf[2048] = {0};
						send_command(p->cph, sendcmd, &id, sizeof(id), buf, sizeof(buf), g_connect_timeout/*CTRL_PROTOCOL_CONNECT_DEFAULT*/);
					}
				}
				break;
			case THREAD_QUIT:
				{
					//::MessageBox(NULL, "quit", NULL, MB_OK);
					return 0;
				}
				break;
				
			}
		}
    
	}

	return 0;
}
#endif


DWORD Domain2IP( char *pDomain )
{
	if (!pDomain)
	{
		return 0;
	}

	if (strlen(pDomain) == 0)
	{
		return 0;
	}

	u32 dwIP = 0;
	LPHOSTENT lpHostEntry;
	struct in_addr inIPAddress;
	lpHostEntry = gethostbyname(pDomain);
	if (lpHostEntry == NULL)
	{
		dwIP = inet_addr(pDomain);
	} 
	else
	{
		char sztmp[128] = {0};
		memcpy((void*)&inIPAddress,lpHostEntry->h_addr_list[0],sizeof(inIPAddress));
		strcpy(sztmp,inet_ntoa(inIPAddress));
		dwIP = inet_addr(sztmp);
	}
	
	return dwIP;	
}

DWORD WINAPI FormatHddThread( LPVOID lpParam )
{
	struct IFLY_FORMATHDD_T *p_fmt = (IFLY_FORMATHDD_T *)lpParam;
	if (!p_fmt)
	{
		return 0;
	}
	
	if (!p_fmt->prcv_t)
	{
		if (p_fmt->pCBFmtProgress)
		{
			p_fmt->pCBFmtProgress(p_fmt->hddindex, 0, NETDVR_ERR_NETSND, p_fmt->dwContent);
		}
		
		return 0;
	}
	
	if (p_fmt->prcv_t->req.command != 9)
	{
		if (p_fmt->pCBFmtProgress)
		{
			p_fmt->pCBFmtProgress(p_fmt->hddindex, 0, NETDVR_ERR_NETSND, p_fmt->dwContent);
		}

		return 0;
	}

	if((p_fmt->prcv_t->sockfd != INVALID_SOCKET) && (p_fmt->prcv_t->bStart))
	{
		while (1)
		{
			ifly_TCP_Pos fmtpos;
			memset(&fmtpos,0,sizeof(ifly_TCP_Pos));
			u16 err = 0;
			
			int ret = looprecv(p_fmt->prcv_t->sockfd, (char *)&fmtpos, sizeof(ifly_TCP_Pos));
			if (ret > 0)
			{
				err = iflydvr_get_error_code(ntohs(fmtpos.errcode));

				p_fmt->progress.pos = fmtpos.pos;
				if (p_fmt->pCBFmtProgress)
				{
					p_fmt->pCBFmtProgress(p_fmt->hddindex, p_fmt->progress.pos, err, p_fmt->dwContent);
				}

				if (err != 0)
				{
					break;
				}

				if (100 == p_fmt->progress.pos)
				{
					break;
				}
			}
			else
			{
				if (p_fmt->pCBFmtProgress)
				{
					p_fmt->pCBFmtProgress(p_fmt->hddindex, p_fmt->progress.pos, NETDVR_ERR_NETRCV, p_fmt->dwContent);
				}
			}
		}	
	}
	
	if (p_fmt->prcv_t)
	{
		closesocket(p_fmt->prcv_t->sockfd);
		memset(p_fmt->prcv_t, 0, sizeof(ifly_stearmrcv_t));
		p_fmt->prcv_t->sockfd = INVALID_SOCKET;
		p_fmt->prcv_t->bStart = 0;
		memset(&p_fmt->progress,0,sizeof(ifly_TCP_Pos));
	}
	
	p_fmt->bOpened = 0;
	
	return 0;	
}

int loopwrite(FILE* file, char * buf, unsigned int write_size)
{
	u32 remian = write_size;
	u32 writelen = 0;
	int ret = 0;
	
	while(remian > 0)
	{
		ret=fwrite(buf+writelen, 1, remian, file);
		if(ret <= 0)
		{
			return ret;
		}
		
		writelen += ret;
		remian -= ret;
	}	
	
	return writelen;
	
}

int loopread(FILE* file, char * buf, unsigned int read_size)
{
	u32 remian = read_size;
	u32 readlen = 0;
	int ret = 0;
	
	while(remian > 0)
	{
		ret=fread(buf+readlen, 1, remian, file);
		if(ret <= 0)
		{
			int err = GetLastError();
			return ret;
		}
		
		readlen += ret;
		remian -= ret;
	}
	
	return readlen;
}

int ctrl_player( int Handle, int linkid_n, u16 command, char *pbuf /*= NULL*/, u32 para_len/*=0*/ )
{
	struct NETDVR_INNER_t *p;
	int ret;

//	LockMutex(g_pool_lock);	
	p = (struct NETDVR_INNER_t *)(Handle);
	if (!p)
	{
//		UnlockMutex(g_pool_lock);
		return NETDVR_ERR_NOINIT;
	}
	
//	UnlockMutex(g_pool_lock);	
	if (!p->b_login)
	{		
		return NETDVR_ERR_NOLOGIN;
	}

	if (!p->b_cmdConnectLost)
	{
		if (NULL==pbuf)
		{
			ret = send_command_noout(p->cph, command, &linkid_n, sizeof(linkid_n), g_connect_timeout/*1000*/);
		} 
		else
		{
			char tmpbuf[512]={0};
			memcpy(tmpbuf, &linkid_n, sizeof(linkid_n));
			memcpy(tmpbuf + sizeof(linkid_n), pbuf, para_len);
			
			ret = send_command_noout(p->cph, command, tmpbuf, sizeof(linkid_n) +para_len, g_connect_timeout/*1000*/);
		}
	} 
	else
	{
		ret = NETDVR_ERR_SEND;
	}
	return ret;
}

BOOL ChkMediaRcvPointer( NETDVR_INNER_t* pDevice, IFLY_MediaRcvPara_t *p_MediaRcvPara )
{
	//查monitor里的
	
	ifly_monitor_t* pMonitor = pDevice->m_pMonitor;
	while (pMonitor)
	{
		if (p_MediaRcvPara == &pMonitor->video_rcv
			|| p_MediaRcvPara == &pMonitor->audio_rcv)
		{
			return TRUE;
		}
		pMonitor = pMonitor->pNext;
	}

#if 0
	//查playback里的

	ifly_playback_t* pPlayBack = pDevice->m_pPlayBack;
	while (pPlayBack)
	{
		if (p_MediaRcvPara == &pPlayBack->player.pb_rcv)
		{
			return TRUE;
		}
		pPlayBack = pPlayBack->pNext;
	}
#endif

	return FALSE;

}

void CleanMsgQ( ifly_msgQ_t *mq, IFLY_MediaRcvPara_t *p_MediaRcvPara )
{
	if (mq && p_MediaRcvPara)
	{
		WaitForSingleObject(mq->lock,INFINITE);
		
		for (int i = 0; i<MAX_SAVE_FRM; i++)
		{
			FrameMsg_t* pReadFrame = (FrameMsg_t*)(mq->buf+i*(sizeof(u32)+mq->unitLen)+sizeof(u32));
			if (pReadFrame->p_MediaRcvPara == p_MediaRcvPara)
			{
				pReadFrame->p_MediaRcvPara = NULL;
			}
		}

		ReleaseMutex(mq->lock);
	}
}

DWORD WINAPI TimeDownLoadThread( LPVOID lpParam )
{
	struct IFLY_FileReciever_t *p_file_reciever = (IFLY_FileReciever_t *)lpParam;
	if (!p_file_reciever)
	{
		return 0;
	}
	
	if (!p_file_reciever->prcv_t)
	{
		return 0;
	}
	
	if (p_file_reciever->req.command != 12)
	{
		return 0;
	}
	struct NETDVR_TimeDLprogressParam_t progress;
	memset(&progress,0,sizeof(NETDVR_progressParam_t));
	progress.totalrecvsize = 0;
	__int64  totalrcvbyte = 0;
	while(1)
	{
		if((p_file_reciever->prcv_t->sockfd != INVALID_SOCKET) && (p_file_reciever->prcv_t->bStart))
		{
			char dlbuf[MAX_DL_BUF] = {0};
			char svPathFileName[PATH_LEN_MAX+FILENAME_LEN_MAX+1] = {0};
			int ret = 0;
			int remian;
			ifly_dowloadbytime_file_t timeDl;
			memset(&timeDl,0,sizeof(ifly_dowloadbytime_file_t));		
			ret = looprecv(p_file_reciever->prcv_t->sockfd,dlbuf,sizeof(ifly_dowloadbytime_file_t));
			if (ret <=0)
			{
				if (p_file_reciever->p_cb_err)
				{
					p_file_reciever->p_cb_err((u16)NETDVR_ERR_NETRCV, p_file_reciever->dwContentErr);			
					goto over;
				}
			}
			memcpy(&timeDl,dlbuf,sizeof(ifly_dowloadbytime_file_t));
			timeDl.currindex = ntohl(timeDl.currindex);
			timeDl.totalnum = ntohl(timeDl.totalnum); 
			timeDl.fileinfo.end_time = ntohl(timeDl.fileinfo.end_time); 
			timeDl.fileinfo.offset = ntohl(timeDl.fileinfo.offset); 
			timeDl.fileinfo.size = ntohl(timeDl.fileinfo.size); 
			timeDl.fileinfo.start_time = ntohl(timeDl.fileinfo.start_time); 
			timeDl.totalfilesize = ntohl(timeDl.totalfilesize);
			remian=timeDl.fileinfo.size;
			p_file_reciever->rcv_size=remian;
			p_file_reciever->recieved_size=0;
			if (p_file_reciever->p_cb_save)
			{
				NETDVR_TimeDLfilename filename;
				memset(&filename,0,sizeof(NETDVR_TimeDLfilename));
				filename.currindex = timeDl.currindex;
				filename.totalnum = timeDl.totalnum;
				filename.channel_no = timeDl.fileinfo.channel_no; 
				filename.end_time = timeDl.fileinfo.end_time; 
				filename.image_format = timeDl.fileinfo.image_format; 
				filename.offset = timeDl.fileinfo.offset; 
				filename.size = timeDl.fileinfo.size; 
				filename.start_time = timeDl.fileinfo.start_time; 
				filename.stream_flag	= timeDl.fileinfo.stream_flag; 
				filename.type = timeDl.fileinfo.type; 
				memset(filename.recvefilename,0,sizeof(filename.recvefilename));
				strcpy(filename.recvefilename, timeDl.fileinfo.filename); 
				p_file_reciever->p_cb_save(filename,svPathFileName, p_file_reciever->dwContentSav);
			}
			else
			{
				struct tm *tm1;				
				tm1 = gmtime((time_t*)&timeDl.fileinfo.start_time);
				wsprintf(svPathFileName, TEXT("D:\\NETDVR\\Chn%02d%04d%02d%02d%02d%02d%02d.ifv"), timeDl.fileinfo.channel_no+1,tm1->tm_year+1900, tm1->tm_mon+1, tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
			}	

			int len=strlen(svPathFileName)-1;
			int pos=0;
			while (svPathFileName[len]!='\\')
			{
				len--;
				pos++;
			}
			memset(p_file_reciever->save_path,0,sizeof(p_file_reciever->save_path));
			memcpy(p_file_reciever->save_path,svPathFileName,strlen(svPathFileName)-pos);
			if (!create_directory(p_file_reciever->save_path))
			{
				if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
				{
					p_file_reciever->p_cb_err((u16)NETDVR_ERR_PARAM, p_file_reciever->dwContentErr);
				}				
				goto over;
			}
			while(remian > 0)
			{
				ret=looprecv(p_file_reciever->prcv_t->sockfd,dlbuf, min(MAX_DL_BUF,remian));
				if(ret <= 0)
				{
					int err = WSAGetLastError();
					DeleteFile(svPathFileName);
					if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
					{
						p_file_reciever->p_cb_err((u16)NETDVR_ERR_DOWNLOAD, p_file_reciever->dwContentErr);
					}
					
					goto over;
				}
				
				if (!p_file_reciever->fp)
				{
					p_file_reciever->fp	= fopen(svPathFileName,"wb");
				}
				
				if (!p_file_reciever->fp)
				{
					DeleteFile(svPathFileName);
					if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
					{
						p_file_reciever->p_cb_err((u16)NETDVR_ERR_DOWNLOAD, p_file_reciever->dwContentErr);
					}
					
					goto over;
				}
				
				p_file_reciever->recieved_size += ret;
				u32 write_size = ret;
				if (p_file_reciever->fp)
				{
					write_size = loopwrite(p_file_reciever->fp, dlbuf, write_size);
				}
				
				if (write_size <= 0)
				{
					DeleteFile(svPathFileName);
					if (p_file_reciever->p_cb_err && (p_file_reciever->recieved_size < p_file_reciever->rcv_size))
					{
						p_file_reciever->p_cb_err((u16)NETDVR_ERR_DOWNLOAD, p_file_reciever->dwContentErr);
					}
					
					goto over;
				}
				else
				{
					if (p_file_reciever->fp)
					{
						fflush(p_file_reciever->fp);
					}
					
					if (p_file_reciever->p_cb_timedlprogress)
					{
						progress.currindex = timeDl.currindex;
						progress.totalnum = timeDl.totalnum;
						progress.currfile_recv = p_file_reciever->recieved_size;
						progress.currfile_size = p_file_reciever->rcv_size;
						progress.totalfilesize = timeDl.totalfilesize;
						totalrcvbyte += ret;
						progress.totalrecvsize = (totalrcvbyte>>20);
						p_file_reciever->p_cb_timedlprogress(progress, p_file_reciever->dwContentProgress);
					}
				}
				remian -= ret;
				Sleep(0);
			}
			if (p_file_reciever->fp)
			{
				fflush(p_file_reciever->fp);
				fclose(p_file_reciever->fp);
				p_file_reciever->fp = NULL;
			}
			if (timeDl.currindex==timeDl.totalnum)
			{
				goto over;
			}
		}
		else
		{
			goto over;
		}
	}
	
over:
	int err = WSAGetLastError();
	
	if (p_file_reciever->fp)
	{
		fflush(p_file_reciever->fp);
		fclose(p_file_reciever->fp);
		p_file_reciever->fp = NULL;
	}
	
	if (p_file_reciever->prcv_t)
	{
		closesocket(p_file_reciever->prcv_t->sockfd);
		p_file_reciever->prcv_t->sockfd = INVALID_SOCKET;
		p_file_reciever->prcv_t->bStart = 0;
	}
	
	return 0;
}

BOOL ChkRealHandle( int nRealHandle )
{
	ifly_monitor_t* pMonitor = (ifly_monitor_t*)nRealHandle;
	if (NULL == pMonitor)
	{
		return FALSE;
	}

	NETDVR_INNER_t* pDevice = (NETDVR_INNER_t*)pMonitor->pDeviceHandle;
	if (NULL == pDevice)
	{
		return FALSE;
	}

	//chk pDevice
	BOOL bChkDeviceHandle = FALSE;
	for (int i= 0; i<MAX_DVR_NUM; i++)
	{
		if (g_dvr_pool.p_dvr[i] && (g_dvr_pool.p_dvr[i] == pDevice))
		{
			bChkDeviceHandle = TRUE;
			break;
		}
	}

	if (!bChkDeviceHandle)
	{
		return FALSE;
	}

	//chk monitor
	ifly_monitor_t* pTmp = pDevice->m_pMonitor;
	while (pTmp)
	{
		if (pMonitor == pTmp)
		{
			return TRUE;
		}
		pTmp = pTmp->pNext;
	}

	return FALSE;
}

BOOL ChkPlayBackHandle( int nPlayBackHandle )
{
	ifly_playback_t* pPlayBack = (ifly_playback_t*)nPlayBackHandle;
	if (NULL == pPlayBack)
	{
		return FALSE;
	}
	
	NETDVR_INNER_t* pDevice = (NETDVR_INNER_t*)pPlayBack->pDeviceHandle;
	if (NULL == pDevice)
	{
		return FALSE;
	}
	
	//chk pDevice
	BOOL bChkDeviceHandle = FALSE;
	for (int i= 0; i<MAX_DVR_NUM; i++)
	{
		if (g_dvr_pool.p_dvr[i] && (g_dvr_pool.p_dvr[i] == pDevice))
		{
			bChkDeviceHandle = TRUE;
			break;
		}
	}
	
	if (!bChkDeviceHandle)
	{
		return FALSE;
	}
	
	//chk monitor
	ifly_playback_t* pTmp = pDevice->m_pPlayBack;
	while (pTmp)
	{
		if (pPlayBack == pTmp)
		{
			return TRUE;
		}
		pTmp = pTmp->pNext;
	}
	
	return FALSE;
}
