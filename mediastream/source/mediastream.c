#include "mssend.h"
#include "msrecv.h"

extern TMediaSndList	g_tMediaSndList;	//发送对象列表全局变量
extern TMediaSndList	g_tMediaSndListTmp;
extern SEMHANDLE		g_hMediaSndSem;		//发送对象列表的访问维护的信号量

extern TMediaRcvList	g_tMediaRcvList;	//接收对象列表全局变量
extern TMediaRcvList	g_tMediaRcvListTmp;
extern SEMHANDLE		g_hMediaRcvSem;		//接收对象列表的访问维护的信号量

void mediastreamver()
{
	printf(" mediastream version: %s   ", VER_MEDIASTREAM);
	printf(" compile time: %s,%s \n", __TIME__, __DATE__);
}

void mediastreamhelp()
{
	printf("\n /************************************/\n");
	printf("mediastreamver()  －－ 版本查询\n");
	printf("printsock()  －－ 收发套结子基本状态信息\n");
	printf("pbinfo(BOOL bShowRcv, BOOL bShowSnd) －－收发对象基本状态信息及统计信息\n");
	printf("pdinfo(int nShowDebugInfo) －－是否显示隐藏的一些调试状态信息\n");
	printf("\rsopen(BOOL bRcvCallback, BOOL bSelfSnd)  －－收发控制开关\n");
	printf("stest(int nSndObjIndex, int nFrameLen, int nSndNum, int nSpan)－－使用已有对象发送自测包\n");
	printf("setconfuedadjust(int nbConfuedAdjust)－－乱序调整处理的开关\n");
	printf("setrepeatsend(int nRepeatSnd)－－ 重传控制开关\n");
	printf("setdiscardspan(int nDiscardSpan) －－模拟丢弃小包的步长间隔设置 \n");
	printf("\n /************************************/\n");
}

//创建发送模块
ifly_mediasnd_t* CreateMediaSnd(u32 dwMaxFrameSize, u32 dwNetBand, u8 byFrameRate, u8 byMediaType, u32 dwSSRC)
{
	ifly_mediasnd_t *pMediaSnd;
	BOOL bFind;
	s32  nPos;
	
	pMediaSnd = (ifly_mediasnd_t *)malloc(sizeof(ifly_mediasnd_t));
	SemBCreate(&pMediaSnd->m_hSndSynSem);
	pMediaSnd->m_pcNetSnd = CreateNetSnd(dwMaxFrameSize, dwNetBand, byFrameRate, byMediaType, dwSSRC);
	
	if(pMediaSnd->m_pcNetSnd == NULL)
	{
		SemDelete(pMediaSnd->m_hSndSynSem);
		pMediaSnd->m_hSndSynSem = NULL;
		free(pMediaSnd);
		return NULL;
	}
	
	SemTake(g_hMediaSndSem);
	
	//加入发送对象链表中记录对象指针
	bFind = FALSE;
	if(g_tMediaSndList.m_nMediaSndCount < MAX_SND_NUM)
	{
		for(nPos=0; nPos<g_tMediaSndList.m_nMediaSndCount; nPos++)
		{
			if(g_tMediaSndList.m_tMediaSndUnit[nPos] == pMediaSnd)
			{
				bFind = TRUE;
				break;
			}
		}
		if(FALSE == bFind)
		{
			g_tMediaSndList.m_nMediaSndCount++;
			g_tMediaSndList.m_tMediaSndUnit[g_tMediaSndList.m_nMediaSndCount-1] = pMediaSnd;
		}
	}
	//g_tMediaSndList
	
	SemGive(g_hMediaSndSem);
	
	return pMediaSnd;
}

//删除发送模块
u16 DestroyMediaSnd(ifly_mediasnd_t* pMediaSnd)
{
	s32 i,j;
	
	if(pMediaSnd)
	{
		SemTake(g_hMediaSndSem);
		
		//从发送对象链表中删除对象指针
		memset(&g_tMediaSndListTmp, 0, sizeof(g_tMediaSndListTmp));
		for(i=0; i<g_tMediaSndList.m_nMediaSndCount; i++)
		{
			if(g_tMediaSndList.m_tMediaSndUnit[i] == pMediaSnd) continue;
			g_tMediaSndListTmp.m_tMediaSndUnit[g_tMediaSndListTmp.m_nMediaSndCount] \
				= g_tMediaSndList.m_tMediaSndUnit[i];
			g_tMediaSndListTmp.m_nMediaSndCount++;
		}
		g_tMediaSndList.m_nMediaSndCount = g_tMediaSndListTmp.m_nMediaSndCount;
		for(j=0; j<g_tMediaSndListTmp.m_nMediaSndCount; j++)
		{
			g_tMediaSndList.m_tMediaSndUnit[j] = 
				g_tMediaSndListTmp.m_tMediaSndUnit[j];
		}
		
		//g_tMediaSndList
		
		SemGive(g_hMediaSndSem);
		
		SemTake(pMediaSnd->m_hSndSynSem);
		DestroyNetSnd(pMediaSnd->m_pcNetSnd);
		pMediaSnd->m_pcNetSnd = NULL;
		SemGive(pMediaSnd->m_hSndSynSem);
		
		SemDelete(pMediaSnd->m_hSndSynSem);
		pMediaSnd->m_hSndSynSem = NULL;
		
		free(pMediaSnd);
	}
	
	return MEDIASTREAM_NO_ERROR;
}

//设置网络发送参数(进行底层套结字的创建，绑定端口,以及发送目标地址的设定等动作)
u16 SetMediaSndNetParam(ifly_mediasnd_t* pMediaSnd,TNetSndParam tNetSndParam)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = SetNetSndNetParam(pMediaSnd->m_pcNetSnd,tNetSndParam);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//移除网络发送本地地址参数(进行底层套结字的删除，释放端口等动作)
u16 RemoveMediaSndLocalNetParam(ifly_mediasnd_t* pMediaSnd)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = RemoveNetSndLocalNetParam(pMediaSnd->m_pcNetSnd);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//重置帧ID
u16 ResetMediaSndFrameId(ifly_mediasnd_t* pMediaSnd)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = ResetNetSndFrameId(pMediaSnd->m_pcNetSnd);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}
//重置同步源SSRC
u16 ResetMediaSndSSRC(ifly_mediasnd_t* pMediaSnd,u32 dwSSRC)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = ResetNetSndSSRC(pMediaSnd->m_pcNetSnd,dwSSRC);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//重置发送端对于mpeg4或者H.264采用的重传处理的开关,关闭后，将不对已经发送的数据包进行缓存
u16 ResetMediaSndRSFlag(ifly_mediasnd_t* pMediaSnd, u16 wBufTimeSpan, BOOL32 bRepeatSnd)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = ResetNetSndRSFlag(pMediaSnd->m_pcNetSnd,wBufTimeSpan,bRepeatSnd);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//设置发送选项
u16 SetMediaSndInfo(ifly_mediasnd_t* pMediaSnd,u32 dwNetBand, u8 byFrameRate)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = SetNetSndInfo(pMediaSnd->m_pcNetSnd,dwNetBand,byFrameRate);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//发送数据包
u16 SendMediaFrame(ifly_mediasnd_t* pMediaSnd,PFRAMEHDR pFrmHdr,int avgInterTime)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = SendFrame(pMediaSnd->m_pcNetSnd,pFrmHdr,avgInterTime);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//得到状态
u16 GetMediaSndStatus(ifly_mediasnd_t* pMediaSnd,TSndStatus *pSndStatus)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = GetNetSndStatus(pMediaSnd->m_pcNetSnd,pSndStatus);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//得到统计
u16 GetMediaSndStatistics(ifly_mediasnd_t* pMediaSnd,TSndStatistics *pSndStatistics)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = GetNetSndStatistics(pMediaSnd->m_pcNetSnd,pSndStatistics);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//得到发送端高级设置参数(重传等)
u16 GetMediaSndAdvancedInfo(ifly_mediasnd_t* pMediaSnd,TAdvancedSndInfo *pAdvancedSndInfo)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = GetNetSndAdvancedInfo(pMediaSnd->m_pcNetSnd,pAdvancedSndInfo);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//rtcp定时rtcp包上报, 内部使用，外部请勿调用
u16 DealMediaSndRtcpTimer(ifly_mediasnd_t* pMediaSnd)
{
	u16 wRet = ERROR_SND_MEMORY;
	
	SemTake(pMediaSnd->m_hSndSynSem);
	if(pMediaSnd->m_pcNetSnd != NULL)
		wRet = DealNetSndRtcpTimer(pMediaSnd->m_pcNetSnd);
	SemGive(pMediaSnd->m_hSndSynSem);
	
	return wRet;
}

//创建接收模块
ifly_mediarcv_t* CreateMediaRcv(u32 dwMaxFrameSize, PFRAMEPROC pFrameCallBackProc, u32 dwContext, u32 dwSSRC)
{
	ifly_mediarcv_t* pMediaRcv;
	BOOL bFind;
	s32  nPos;
	
	pMediaRcv = (ifly_mediarcv_t *)malloc(sizeof(ifly_mediarcv_t));
	SemBCreate(&pMediaRcv->m_hRcvSynSem);
	pMediaRcv->m_pcNetRcv = CreateNetRcv(dwMaxFrameSize,pFrameCallBackProc,dwContext,dwSSRC);
	if(pMediaRcv->m_pcNetRcv == NULL)
	{
		SemDelete(pMediaRcv->m_hRcvSynSem);
		pMediaRcv->m_hRcvSynSem = NULL;
		free(pMediaRcv);
		return NULL;
	}
	
	SemTake(g_hMediaRcvSem);
	
	//加入接收对象链表中记录对象指针
	bFind = FALSE;
	if(g_tMediaRcvList.m_nMediaRcvCount < FD_SETSIZE)
	{
		for(nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
		{
			if(g_tMediaRcvList.m_tMediaRcvUnit[nPos] == pMediaRcv)
			{
				bFind = TRUE;
				break;
			}
		}
		if(FALSE == bFind)
		{
			g_tMediaRcvList.m_nMediaRcvCount++;
			g_tMediaRcvList.m_tMediaRcvUnit[g_tMediaRcvList.m_nMediaRcvCount-1] = pMediaRcv;
		}
	}
	//g_tMediaRcvList
	
	SemGive(g_hMediaRcvSem);
	
	return pMediaRcv;
}

ifly_mediarcv_t* CreateMediaRcvRtp(u32 dwMaxFrameSize, PRTPCALLBACK PRtpCallBackProc, u32 dwContext, u32 dwSSRC)
{
	ifly_mediarcv_t* pMediaRcv;
	BOOL bFind;
	s32  nPos;
	
	pMediaRcv = (ifly_mediarcv_t *)malloc(sizeof(ifly_mediarcv_t));
	SemBCreate(&pMediaRcv->m_hRcvSynSem);
	pMediaRcv->m_pcNetRcv = CreateNetRcvRtp(dwMaxFrameSize,PRtpCallBackProc,dwContext,dwSSRC);
	if(pMediaRcv->m_pcNetRcv == NULL)
	{
		SemDelete(pMediaRcv->m_hRcvSynSem);
		pMediaRcv->m_hRcvSynSem = NULL;
		free(pMediaRcv);
		return NULL;
	}
	
	SemTake(g_hMediaRcvSem);
	
	//加入接收对象链表中记录对象指针
	bFind = FALSE;
	if(g_tMediaRcvList.m_nMediaRcvCount < FD_SETSIZE)
	{
		for(nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
		{
			if(g_tMediaRcvList.m_tMediaRcvUnit[nPos] == pMediaRcv)
			{
				bFind = TRUE;
				break;
			}
		}
		if(FALSE == bFind)
		{
			g_tMediaRcvList.m_nMediaRcvCount++;
			g_tMediaRcvList.m_tMediaRcvUnit[g_tMediaRcvList.m_nMediaRcvCount-1] = pMediaRcv;
		}
	}
	//g_tMediaRcvList
	
	SemGive(g_hMediaRcvSem);
	
	return pMediaRcv;
}

//设置接收地址参数(进行底层套结子的创建，绑定端口等动作)
u16 SetMediaRcvLocalParam(ifly_mediarcv_t* pMediaRcv,TLocalNetParam tLocalNetParam)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;
	
    if(NULL != pMediaRcv && NULL != pMediaRcv->m_hRcvSynSem) 
	{
		SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv != NULL)
			wRet = SetNetRcvLocalParam(pMediaRcv->m_pcNetRcv,tLocalNetParam);
		SemGive(pMediaRcv->m_hRcvSynSem);
	}
	
    return wRet;
}

//移除接收地址参数(进行底层套结子的删除，释放端口等动作)
u16 RemoveMediaRcvLocalParam(ifly_mediarcv_t* pMediaRcv)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;
	
    if(NULL != pMediaRcv && NULL != pMediaRcv->m_hRcvSynSem) 
	{
		SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv != NULL)
			wRet = RemoveNetRcvLocalParam(pMediaRcv->m_pcNetRcv);
		SemGive(pMediaRcv->m_hRcvSynSem);
	}
	
    return wRet;
}

//重置接收端对于mpeg4或者H.264采用的重传处理的开关,关闭后，将不发送重传请求
u16 ResetMediaRcvRSFlag(ifly_mediarcv_t* pMediaRcv,TRSParam tRSParam, BOOL bRepeatSnd)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;
	
    if(NULL != pMediaRcv && NULL != pMediaRcv->m_hRcvSynSem) 
	{
		SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv != NULL)
			wRet = ResetNetRcvRSFlag(pMediaRcv->m_pcNetRcv,tRSParam,bRepeatSnd);
		SemGive(pMediaRcv->m_hRcvSynSem);
	}
	
    return wRet;
}

//开始接收
u16 StartMediaRcv(ifly_mediarcv_t* pMediaRcv)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;
	
    if(NULL != pMediaRcv && NULL != pMediaRcv->m_hRcvSynSem) 
	{
		SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv != NULL)
			wRet = StartNetRcv(pMediaRcv->m_pcNetRcv);
		SemGive(pMediaRcv->m_hRcvSynSem);
	}
	
    return wRet;
}

//停止接收
u16 StopMediaRcv(ifly_mediarcv_t* pMediaRcv)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;
	
    if(NULL != pMediaRcv && NULL != pMediaRcv->m_hRcvSynSem) 
	{
		SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv != NULL)
			wRet = StopNetRcv(pMediaRcv->m_pcNetRcv);
		SemGive(pMediaRcv->m_hRcvSynSem);
	}
	
    return wRet;
}

//得到状态
u16 GetMediaRcvStatus(ifly_mediarcv_t* pMediaRcv,TRcvStatus *pRcvStatus)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;
	
    if(NULL != pMediaRcv && NULL != pMediaRcv->m_hRcvSynSem) 
	{
		SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv != NULL)
			wRet = GetNetRcvStatus(pMediaRcv->m_pcNetRcv,pRcvStatus);
		SemGive(pMediaRcv->m_hRcvSynSem);
	}
	
    return wRet;
}

//得到统计
u16 GetMediaRcvStatistics(ifly_mediarcv_t* pMediaRcv,TRcvStatistics *pRcvStatistics)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;
	
    if(NULL != pMediaRcv && NULL != pMediaRcv->m_hRcvSynSem) 
	{
		SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv != NULL)
			wRet = GetNetRcvStatistics(pMediaRcv->m_pcNetRcv,pRcvStatistics);
		SemGive(pMediaRcv->m_hRcvSynSem);
	}
	
    return wRet;
}

u16 DestroyMediaRcv(ifly_mediarcv_t* pMediaRcv)
{
	s32 i,j;
	
	if(pMediaRcv)
	{
		SemTake(g_hMediaRcvSem);
		
		//从接收对象链表中删除对象指针
		memset(&g_tMediaRcvListTmp, 0, sizeof(g_tMediaRcvListTmp));
		for(i=0; i<g_tMediaRcvList.m_nMediaRcvCount; i++)
		{
			if(g_tMediaRcvList.m_tMediaRcvUnit[i] == pMediaRcv) continue;
			g_tMediaRcvListTmp.m_tMediaRcvUnit[g_tMediaRcvListTmp.m_nMediaRcvCount] \
				= g_tMediaRcvList.m_tMediaRcvUnit[i];
			g_tMediaRcvListTmp.m_nMediaRcvCount++;
		}
		g_tMediaRcvList.m_nMediaRcvCount = g_tMediaRcvListTmp.m_nMediaRcvCount;
		for(j=0; j<g_tMediaRcvListTmp.m_nMediaRcvCount; j++)
		{
			g_tMediaRcvList.m_tMediaRcvUnit[j] = 
				g_tMediaRcvListTmp.m_tMediaRcvUnit[j];
		}
		//g_tMediaRcvList
		
		SemGive(g_hMediaRcvSem);
		
		if(pMediaRcv->m_hRcvSynSem) SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv)   DestroyNetRcv(pMediaRcv->m_pcNetRcv);
		if(pMediaRcv->m_hRcvSynSem) SemGive(pMediaRcv->m_hRcvSynSem);
		
		if(pMediaRcv->m_hRcvSynSem) SemDelete(pMediaRcv->m_hRcvSynSem);
		pMediaRcv->m_hRcvSynSem = NULL;
		
		free(pMediaRcv);
	}
	return MEDIASTREAM_NO_ERROR;
}

u16 DealMediaRcvRtcpTimer(ifly_mediarcv_t* pMediaRcv)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;
	
	if(NULL != pMediaRcv && NULL != pMediaRcv->m_hRcvSynSem)
	{
		SemTake(pMediaRcv->m_hRcvSynSem);
		if(pMediaRcv->m_pcNetRcv != NULL)
			wRet = DealNetRcvRtcpTimer(pMediaRcv->m_pcNetRcv);
		SemGive(pMediaRcv->m_hRcvSynSem);
	}
	
    return wRet;
}
