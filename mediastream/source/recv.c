#include "msrecv.h"

extern s32 g_nRepeatSnd;
extern s32 g_nDiscardSpan;
extern s32 g_nShowDebugInfo;

void InitNetRcv(ifly_netrcv_t *pRcv)
{
	s32 i;
	
	if(pRcv)
	{
		pRcv->m_pcRtp					= NULL;
		
		pRcv->m_pcRtcp					= NULL;//csp add 2008-11-20
		
		pRcv->m_pFrameBuf				= NULL;
		
		pRcv->m_dwFrameId				= 0;
		pRcv->m_dwTimeStamp				= 0;
		pRcv->m_dwMaxFrameSize			= 0;
		pRcv->m_pFrameCallBackHandler	= NULL;
		pRcv->m_pRtpCallBackHandler		= NULL;
		pRcv->m_dwContext				= 0;
		pRcv->m_dwLastSSRC				= 0;
		pRcv->m_bRcvStart				= FALSE;
		
		pRcv->m_bConfuedAdjust			= FALSE;
		
		pRcv->m_dwQueneNum				= 0;
		pRcv->m_bRepeatSend				= FALSE;
		pRcv->m_dwMaxQueneNum			= 0;
		pRcv->m_dwMinTimeStamp			= 0;
		pRcv->m_dwMaxTimeStamp			= 0;
		pRcv->m_dwLastTimeStamp			= 0;
		
		memset(&pRcv->m_tRSParam, 0, sizeof(TRSParam));
		
		pRcv->m_hSem					= NULL;
		
		for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
		{
			pRcv->m_ptPackInfo[i]		= NULL;
			pRcv->m_ptCBPackInfo[i*2]	= NULL;
			pRcv->m_ptCBPackInfo[i*2+1] = NULL;
		}	
		
		pRcv->m_ptAudioInfo = NULL;
		memset(&pRcv->m_tRcvStatus, 0, sizeof(pRcv->m_tRcvStatus));
		memset(&pRcv->m_tRcvStatistics, 0, sizeof(pRcv->m_tRcvStatistics));
		memset(&pRcv->m_tLocalNetParam, 0, sizeof(pRcv->m_tLocalNetParam));
		memset(&pRcv->m_tLastInfo, 0, sizeof(pRcv->m_tLastInfo));
		memset(&pRcv->m_FrmHdr, 0, sizeof(pRcv->m_FrmHdr));
		memset(&pRcv->m_tMp4Header, 0, sizeof(pRcv->m_tMp4Header));
		
		//csp add 2008-11-28
		memset(pRcv->m_ptPackInfo,0,sizeof(pRcv->m_ptPackInfo));
		memset(pRcv->m_ptCBPackInfo,0,sizeof(pRcv->m_ptCBPackInfo));
	}
}

ifly_netrcv_t* CreateNetRcv(u32 dwMaxFrameSize, PFRAMEPROC pFrameCallBackProc, u32 dwContext, u32 dwSSRC)
{
	ifly_netrcv_t *pRcv = NULL;
	TRSParam tRSParam;
	
	if( dwMaxFrameSize == 0 || dwMaxFrameSize > MAX_FRAME_SIZE || pFrameCallBackProc == NULL )
	{
		return NULL;
	}
	
	pRcv = (ifly_netrcv_t *)malloc(sizeof(ifly_netrcv_t));
	InitNetRcv(pRcv);
	
	pRcv->m_pFrameCallBackHandler	= pFrameCallBackProc;
	pRcv->m_dwContext				= dwContext;
	pRcv->m_dwMaxFrameSize			= dwMaxFrameSize;
	
	if(0 == dwSSRC)
	{
		pRcv->m_dwTimeStamp = GetExclusiveSSRC();
	}
	else
	{
		pRcv->m_dwTimeStamp = dwSSRC;
	}
	
	//创建RTP对象
	pRcv->m_pcRtp = CreateRtp(pRcv->m_dwTimeStamp, FALSE);
	if(pRcv->m_pcRtp == NULL)
	{
		goto end;
	}
	
	//csp add 2008-11-20
	//创建RTCP对象
	pRcv->m_pcRtcp = CreateRtcp(pRcv->m_dwTimeStamp);
	if(pRcv->m_pcRtcp == NULL)
	{
		goto end;
	}
	
	//csp add 2008-11-20
	//设置和RTP对应的RTCP。
	SetRtpRtcp(pRcv->m_pcRtp, pRcv->m_pcRtcp);
	
	//设置RTP数据回调
	SetRtpCallBack(pRcv->m_pcRtp, RcvCallBack, (u32)pRcv);
	
	pRcv->m_pFrameBuf = (u8 *)malloc(pRcv->m_dwMaxFrameSize);
	if( NULL == pRcv->m_pFrameBuf )
	{
		goto end;
	}
	
	if(!SemBCreate(&pRcv->m_hSem))
	{
		goto end;
	}
	
	//对于mp4、h.264我们采用重发方式,默认不重发
	memset(&tRSParam, 0, sizeof(TRSParam));
	
	ResetNetRcvRSFlag(pRcv,tRSParam,FALSE);
	
	pRcv->m_ptAudioInfo = (TAudioInfo *)malloc(sizeof(TAudioInfo));
	if(pRcv->m_ptAudioInfo == NULL)
	{
		goto end;
	}
	memset(pRcv->m_ptAudioInfo, 0, sizeof(TAudioInfo));
	
	return pRcv;
	
end:
	
	return NULL;
}

ifly_netrcv_t* CreateNetRcvRtp(u32 dwMaxFrameSize, PRTPCALLBACK pRtpCallBackProc, u32 dwContext, u32 dwSSRC)
{
	ifly_netrcv_t *pRcv = NULL;
	TRSParam tRSParam;
	
	if( dwMaxFrameSize == 0 || dwMaxFrameSize > MAX_FRAME_SIZE || pRtpCallBackProc == NULL )
	{
		return NULL;
	}
	
	pRcv = (ifly_netrcv_t *)malloc(sizeof(ifly_netrcv_t));
	InitNetRcv(pRcv);
	
	pRcv->m_pRtpCallBackHandler	= pRtpCallBackProc;
	pRcv->m_dwContext			= dwContext;
	pRcv->m_dwMaxFrameSize		= dwMaxFrameSize;
	
	if(0 == dwSSRC)
	{
		pRcv->m_dwTimeStamp = GetExclusiveSSRC();
	}
	else
	{
		pRcv->m_dwTimeStamp = dwSSRC;
	}
	
    //创建RTP对象
	pRcv->m_pcRtp = CreateRtp(pRcv->m_dwTimeStamp, FALSE);
	if(pRcv->m_pcRtp == NULL)
	{
		goto end;
	}
	
	//csp add 2008-11-20
	//创建RTCP对象
	pRcv->m_pcRtcp = CreateRtcp(pRcv->m_dwTimeStamp);
	if(pRcv->m_pcRtcp == NULL)
	{
		goto end;
	}
	
	//csp add 2008-11-20
	//设置和RTP对应的RTCP。
	SetRtpRtcp(pRcv->m_pcRtp, pRcv->m_pcRtcp);
	
	//设置RTP数据回调
	SetRtpCallBack(pRcv->m_pcRtp,RcvCallBack,(u32)pRcv);
	
	pRcv->m_pFrameBuf = (u8 *)malloc(pRcv->m_dwMaxFrameSize);
	if( NULL == pRcv->m_pFrameBuf )
	{
		goto end;
	}
	
	if(!SemBCreate(&pRcv->m_hSem))
	{
		goto end;
	}
	
	//对于mp4、h.264我们采用重发方式,默认不重发
	memset(&tRSParam, 0, sizeof(TRSParam));
	
	ResetNetRcvRSFlag(pRcv,tRSParam,FALSE);
	
	pRcv->m_ptAudioInfo = (TAudioInfo *)malloc(sizeof(TAudioInfo));
	if(pRcv->m_ptAudioInfo == NULL)
	{
		goto end;
	}
	memset(pRcv->m_ptAudioInfo, 0, sizeof(TAudioInfo));
	
	return pRcv;
	
end:
	
   	return NULL;
}

u16 SetNetRcvLocalParam(ifly_netrcv_t *pRcv,TLocalNetParam tLocalNetParam)
{
	u16 wRet = MEDIASTREAM_NO_ERROR;
	
	TRemoteAddr tRemoteAddr;
	
	if(pRcv == NULL || pRcv->m_pcRtp == NULL)
	{
		return ERROR_RCV_NO_CREATE;        
	}
	
	//相同的地址不会重设
	if(	pRcv->m_tLocalNetParam.m_tLocalNet.m_wRTCPPort	== tLocalNetParam.m_tLocalNet.m_wRTCPPort &&
		pRcv->m_tLocalNetParam.m_tLocalNet.m_wRTPPort	== tLocalNetParam.m_tLocalNet.m_wRTPPort &&
		pRcv->m_tLocalNetParam.m_tLocalNet.m_dwRTCPAddr == tLocalNetParam.m_tLocalNet.m_dwRTCPAddr &&
		pRcv->m_tLocalNetParam.m_tLocalNet.m_dwRTPAddr	== tLocalNetParam.m_tLocalNet.m_dwRTPAddr )
	{
		return MEDIASTREAM_NO_ERROR;
	}
	
	pRcv->m_tLocalNetParam        = tLocalNetParam;
	pRcv->m_tRcvStatus.m_tRcvAddr = tLocalNetParam;
	
	//设置RTP当地地址
	wRet = SetRtpLocalAddr(pRcv->m_pcRtp,tLocalNetParam.m_tLocalNet.m_dwRTPAddr,tLocalNetParam.m_tLocalNet.m_wRTPPort,TRUE);
	if(IFLYFAILED(wRet))
	{
		return wRet;
	}
	
	//csp add 2008-11-20
	//设置RTCP当地地址
	wRet = SetRtcpLocalAddr(pRcv->m_pcRtcp, tLocalNetParam.m_tLocalNet.m_dwRTCPAddr, tLocalNetParam.m_tLocalNet.m_wRTCPPort);
	if(IFLYFAILED(wRet))
	{
		return wRet;
	}
    
	//设置RTCP反馈地址
	tRemoteAddr.m_byNum			   = 1;
	tRemoteAddr.m_tAddr[0].m_dwIP  = tLocalNetParam.m_dwRtcpBackAddr;
	tRemoteAddr.m_tAddr[0].m_wPort = tLocalNetParam.m_wRtcpBackPort;
	wRet = SetRtcpRemoteAddr(pRcv->m_pcRtcp, &tRemoteAddr);
    if(IFLYFAILED(wRet))
	{
		return wRet;
	}
	
	pRcv->m_tRcvStatus.m_tRcvAddr = tLocalNetParam;
	
	return MEDIASTREAM_NO_ERROR;
}

u16 RemoveNetRcvLocalParam(ifly_netrcv_t *pRcv)
{
	BOOL bRcvStart;
	TRemoteAddr tRemoteAddr;
	
	if(pRcv == NULL || pRcv->m_pcRtp == NULL)
	{
		return ERROR_RCV_NO_CREATE;        
	}
	
	//先停止接收
	bRcvStart = pRcv->m_bRcvStart;
	pRcv->m_bRcvStart = FALSE;
	
	RemoveRtpLocalAddr(pRcv->m_pcRtp);
	
	//csp add 2008-11-20
	RemoveRtcpLocalAddr(pRcv->m_pcRtcp);
	
	memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
	SetRtpRemoteAddr(pRcv->m_pcRtp,&tRemoteAddr);
	
	memset(&pRcv->m_tLocalNetParam, 0, sizeof(pRcv->m_tLocalNetParam));
	pRcv->m_tRcvStatus.m_tRcvAddr = pRcv->m_tLocalNetParam;
	pRcv->m_tRcvStatus.m_tRcvAddr = pRcv->m_tLocalNetParam;
	
	//恢复原状
	pRcv->m_bRcvStart = bRcvStart;
	
	return MEDIASTREAM_NO_ERROR;
}

u16 ResetNetRcvRSFlag(ifly_netrcv_t *pRcv, TRSParam tRSParam, BOOL bRepeatSnd)
{
	s32  i;
	BOOL bReAlloc;
	
	if(pRcv->m_hSem != NULL) SemTake(pRcv->m_hSem);
	
	pRcv->m_dwQueneNum      = 0;
	pRcv->m_dwMinTimeStamp  = 0;
	pRcv->m_dwMaxTimeStamp  = 0;
	pRcv->m_dwLastTimeStamp	= 0;
	pRcv->m_bRepeatSend		= bRepeatSnd;
	for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
	{
		memset(pRcv->m_ptPackInfo[i], 0, sizeof(TPackInfo));
		memset(pRcv->m_ptCBPackInfo[i*2], 0, sizeof(TCBPackInfo));
		memset(pRcv->m_ptCBPackInfo[i*2+1], 0, sizeof(TCBPackInfo));	
	}
	
	bReAlloc = FALSE;
	if(FALSE == bRepeatSnd)
	{
		if(DEFAULT_PACK_QUENE_NUM != pRcv->m_dwMaxQueneNum)
		{
			for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
			{
				SAFE_DELETE(pRcv->m_ptPackInfo[i])
				SAFE_DELETE(pRcv->m_ptCBPackInfo[i*2])
				SAFE_DELETE(pRcv->m_ptCBPackInfo[i*2+1])
			}
			bReAlloc = TRUE;
		}
		pRcv->m_dwMaxQueneNum = DEFAULT_PACK_QUENE_NUM;
		memset(&pRcv->m_tRSParam, 0, sizeof(TRSParam));
	}
	else
	{
		pRcv->m_tRSParam.m_wFirstTimeSpan  = VIDEO_TIME_SPAN * 
					((tRSParam.m_wFirstTimeSpan + VIDEO_TIME_SPAN - 1)/(VIDEO_TIME_SPAN));
		pRcv->m_tRSParam.m_wSecondTimeSpan = VIDEO_TIME_SPAN *
					((tRSParam.m_wSecondTimeSpan + VIDEO_TIME_SPAN - 1)/VIDEO_TIME_SPAN);
		pRcv->m_tRSParam.m_wThirdTimeSpan  = VIDEO_TIME_SPAN *
					((tRSParam.m_wThirdTimeSpan + VIDEO_TIME_SPAN - 1)/VIDEO_TIME_SPAN);
		pRcv->m_tRSParam.m_wRejectTimeSpan = VIDEO_TIME_SPAN *
					((tRSParam.m_wRejectTimeSpan + VIDEO_TIME_SPAN - 1)/VIDEO_TIME_SPAN);
		
		if(pRcv->m_tRSParam.m_wRejectTimeSpan > MAX_PACK_QUENE_NUM*VIDEO_TIME_SPAN)
		{
			pRcv->m_tRSParam.m_wRejectTimeSpan = MAX_PACK_QUENE_NUM*VIDEO_TIME_SPAN;
		}
		if(pRcv->m_tRSParam.m_wRejectTimeSpan < DEFAULT_PACK_QUENE_NUM*VIDEO_TIME_SPAN)
		{
			pRcv->m_tRSParam.m_wRejectTimeSpan = DEFAULT_PACK_QUENE_NUM*VIDEO_TIME_SPAN;
		}
		
		if(pRcv->m_tRSParam.m_wRejectTimeSpan != pRcv->m_dwMaxQueneNum*VIDEO_TIME_SPAN)
		{
			for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
			{
				SAFE_DELETE(pRcv->m_ptPackInfo[i])
				SAFE_DELETE(pRcv->m_ptCBPackInfo[i*2])
				SAFE_DELETE(pRcv->m_ptCBPackInfo[i*2+1])
			}
			bReAlloc = TRUE;
		}
		pRcv->m_dwMaxQueneNum = pRcv->m_tRSParam.m_wRejectTimeSpan/VIDEO_TIME_SPAN;
		if(pRcv->m_dwMaxQueneNum > MAX_PACK_QUENE_NUM)
		{
			pRcv->m_dwMaxQueneNum = MAX_PACK_QUENE_NUM;
		}
		printf("m_dwMaxQueneNum=%d\n",pRcv->m_dwMaxQueneNum);
	}
	
	if(TRUE == bReAlloc)
	{
		//重新分配缓冲队列
		for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
		{
			pRcv->m_ptPackInfo[i] = (TPackInfo *)malloc(sizeof(TPackInfo));
			if(NULL == pRcv->m_ptPackInfo[i])
			{
				pRcv->m_dwMaxQueneNum = i;
				if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
				FreeNetRcvBuf(pRcv);
				return ERROR_NET_RCV_MEMORY;
			}
			memset(pRcv->m_ptPackInfo[i], 0, sizeof(TPackInfo));
			pRcv->m_ptCBPackInfo[i*2] = (TCBPackInfo *)malloc(sizeof(TCBPackInfo));
			if(NULL == pRcv->m_ptCBPackInfo[i*2])
			{
				pRcv->m_dwMaxQueneNum = i;
				if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
				FreeNetRcvBuf(pRcv);
				return ERROR_NET_RCV_MEMORY;
			}
			memset(pRcv->m_ptCBPackInfo[i*2], 0, sizeof(TCBPackInfo));
			pRcv->m_ptCBPackInfo[i*2+1] = (TCBPackInfo *)malloc(sizeof(TCBPackInfo));
			if(NULL == pRcv->m_ptCBPackInfo[i*2+1])
			{
				pRcv->m_dwMaxQueneNum = i;
				if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
				FreeNetRcvBuf(pRcv);
				return ERROR_NET_RCV_MEMORY;
			}
			memset(pRcv->m_ptCBPackInfo[i*2+1], 0, sizeof(TCBPackInfo));
		}
	}
	
	if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
	
	return MEDIASTREAM_NO_ERROR;
}

u16 StartNetRcv(ifly_netrcv_t *pRcv)
{
	pRcv->m_bRcvStart = TRUE;
	pRcv->m_tRcvStatus.m_bRcvStart = TRUE;
	return MEDIASTREAM_NO_ERROR;
}

u16 StopNetRcv(ifly_netrcv_t *pRcv)
{
	pRcv->m_bRcvStart = FALSE;
	pRcv->m_tRcvStatus.m_bRcvStart = FALSE;//TRUE;
	return MEDIASTREAM_NO_ERROR;
}

u16 GetNetRcvStatus(ifly_netrcv_t *pRcv,TRcvStatus *pRcvStatus)
{
	*pRcvStatus = pRcv->m_tRcvStatus;
	return MEDIASTREAM_NO_ERROR;
}

u16 GetNetRcvStatistics(ifly_netrcv_t *pRcv,TRcvStatistics *pRcvStatistics)
{
	*pRcvStatistics = pRcv->m_tRcvStatistics;
	return MEDIASTREAM_NO_ERROR;
}

u16	FreeNetRcvBuf(ifly_netrcv_t *pRcv)
{
	s32 i;
	
	if(pRcv->m_pcRtp)
	{
		DestroyRtp(pRcv->m_pcRtp);
		pRcv->m_pcRtp = NULL;
	}
	
	//csp add 2008-11-20
	if(pRcv->m_pcRtcp)
	{
		DestroyRtcp(pRcv->m_pcRtcp);
		pRcv->m_pcRtcp = NULL;
	}
	
	SAFE_DELETE(pRcv->m_pFrameBuf)
	SAFE_DELETE(pRcv->m_ptAudioInfo)
	for(i=0; i<(INT32)pRcv->m_dwMaxQueneNum; i++)
	{
		SAFE_DELETE(pRcv->m_ptPackInfo[i])
		SAFE_DELETE(pRcv->m_ptCBPackInfo[i*2])
		SAFE_DELETE(pRcv->m_ptCBPackInfo[i*2+1])
	}
	pRcv->m_bRcvStart				= FALSE;
	pRcv->m_dwFrameId				= 0;
	pRcv->m_dwTimeStamp				= 0;
	pRcv->m_dwMaxFrameSize			= 0;
	pRcv->m_pFrameCallBackHandler	= NULL;
	pRcv->m_pRtpCallBackHandler		= NULL;
	pRcv->m_dwContext				= 0;
	pRcv->m_dwLastSSRC				= 0;
	
	pRcv->m_dwQueneNum				= 0;
	pRcv->m_dwMaxQueneNum			= 0;
	pRcv->m_bRepeatSend				= FALSE;
	pRcv->m_dwMinTimeStamp			= 0;
	pRcv->m_dwMaxTimeStamp			= 0;
	pRcv->m_dwLastTimeStamp			= 0;
	memset(&pRcv->m_tRSParam, 0, sizeof(TRSParam));
	
	memset(&pRcv->m_tRcvStatus, 0, sizeof(pRcv->m_tRcvStatus));
	memset(&pRcv->m_tRcvStatistics, 0, sizeof(pRcv->m_tRcvStatistics));
	memset(&pRcv->m_tLocalNetParam, 0, sizeof(pRcv->m_tLocalNetParam));
	memset(&pRcv->m_tLastInfo, 0, sizeof(pRcv->m_tLastInfo));
	memset(&pRcv->m_FrmHdr, 0, sizeof(pRcv->m_FrmHdr));
	
	memset(&pRcv->m_tMp4Header, 0, sizeof(pRcv->m_tMp4Header));
	
	if(pRcv->m_hSem != NULL)
	{
		SemDelete(pRcv->m_hSem);
		pRcv->m_hSem = NULL;
	}
	
	return MEDIASTREAM_NO_ERROR;
}

u16	DestroyNetRcv(ifly_netrcv_t *pRcv)
{
	if(pRcv)
	{
		FreeNetRcvBuf(pRcv);
		free(pRcv);
	}
	
	return MEDIASTREAM_NO_ERROR;
}

void DealNetRcvData(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	if(pRtpPack == NULL || pRcv->m_pFrameBuf == NULL) return;
    
	pRcv->m_tRcvStatistics.m_dwPackNum++;
	
	//根据不同的载荷，作不同的处理
	switch(pRtpPack->m_byPayload)
	{
	case MEDIA_TYPE_H264:
		{
			DealH264( pRcv,pRtpPack );
			break;
		}
	case MEDIA_TYPE_MP4:
		{
			DealMpg4( pRcv,pRtpPack );
			break;
		}
	case MEDIA_TYPE_MJPEG:
		{
			DealMpg4( pRcv,pRtpPack );
			break;
		}
	case MEDIA_TYPE_MP3:
		{
			DealMp3( pRcv,pRtpPack );
			break;
		}
	case MEDIA_TYPE_PCMU:
	case MEDIA_TYPE_PCMA:
		{
			DealG711( pRcv,pRtpPack );
			break;			
		}
    case MEDIA_TYPE_G7231:
        {
            DealG723( pRcv,pRtpPack );
            break;
        }
	case MEDIA_TYPE_G728:
	case MEDIA_TYPE_G729:
	case MEDIA_TYPE_RAWAUDIO:
		{
            DealG728( pRcv,pRtpPack );
            break;
        }
	case MEDIA_TYPE_G722:
		{
            DealG722( pRcv,pRtpPack );
            break;
        }
	default:
		break;
	}
	return;
}

void RcvCallBack(TRtpPack *pRtpPack, u32 dwContext)
{
	ifly_netrcv_t *pMain = (ifly_netrcv_t *)dwContext;
	if(pMain != NULL)
	{
		DealNetRcvData(pMain,pRtpPack);
	}
}

u16 DealNetRcvRtcpTimer(ifly_netrcv_t *pRcv)
{
	//是否创建
	if( NULL == pRcv || NULL == pRcv->m_pcRtp || NULL == pRcv->m_pcRtcp )
	{
		return ERROR_RCV_NO_CREATE;
	}
	
	return DealRtcpTimer(pRcv->m_pcRtcp);
}

void DealG711(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	//u8 byMode = *(pRtpPack->m_pRealData);
	
    if(pRcv->m_bRcvStart && pRcv->m_pRtpCallBackHandler != NULL)
    {
        pRcv->m_pRtpCallBackHandler(pRtpPack, pRcv->m_dwContext);
        return;
    }
	
	//断帧统计
	if(pRtpPack->m_wSequence != (u16)(pRcv->m_tLastInfo.m_wSeq + 1))
	{
		//printf("audio lose,wSequence=%d,tLastInfo.wSeq=%d,wFrameID=%d\n",pRtpPack->m_wSequence,pRcv->m_tLastInfo.m_wSeq,pRcv->m_FrmHdr.m_dwFrameID);
		pRcv->m_FrmHdr.m_dwFrameID++;
		pRcv->m_tRcvStatus.m_dwFrameID = pRcv->m_FrmHdr.m_dwFrameID;
		pRcv->m_tRcvStatistics.m_dwPackLose++;
		
		if(pRtpPack->m_wSequence <= pRcv->m_tLastInfo.m_wSeq)
		{
            pRcv->m_tRcvStatistics.m_dwPackIndexError++;
		}
	}
	pRcv->m_FrmHdr.m_dwFrameID++;
    pRcv->m_tRcvStatus.m_dwFrameID  = pRcv->m_FrmHdr.m_dwFrameID;
	
	pRcv->m_FrmHdr.m_byMediaType	= pRtpPack->m_byPayload;
	pRcv->m_FrmHdr.m_dwDataSize		= pRtpPack->m_nRealSize ;
	pRcv->m_FrmHdr.m_dwTimeStamp	= pRtpPack->m_dwTimeStamp;
	pRcv->m_FrmHdr.m_pData			= pRtpPack->m_pRealData ;
	
	pRcv->m_tRcvStatistics.m_dwFrameNum++;
	
	if(pRcv->m_bRcvStart && pRcv->m_pFrameCallBackHandler != NULL)
	{
		pRcv->m_pFrameCallBackHandler(&pRcv->m_FrmHdr, pRcv->m_dwContext);
	}
	pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
}

void DealG723(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	if(pRcv->m_bRcvStart && pRcv->m_pRtpCallBackHandler != NULL)
    {
        pRcv->m_pRtpCallBackHandler(pRtpPack, pRcv->m_dwContext);
        return;
    }
	
	//断帧统计
	if(pRtpPack->m_wSequence != (u16)(pRcv->m_tLastInfo.m_wSeq + 1))
	{
		pRcv->m_FrmHdr.m_dwFrameID++;
		pRcv->m_tRcvStatus.m_dwFrameID = pRcv->m_FrmHdr.m_dwFrameID;
		pRcv->m_tRcvStatistics.m_dwPackLose++;
		
		if(pRtpPack->m_wSequence <= pRcv->m_tLastInfo.m_wSeq)
		{
            pRcv->m_tRcvStatistics.m_dwPackIndexError++;
		}
	}
	pRcv->m_FrmHdr.m_dwFrameID++;
    pRcv->m_tRcvStatus.m_dwFrameID  = pRcv->m_FrmHdr.m_dwFrameID;
	
	pRcv->m_FrmHdr.m_byMediaType	= pRtpPack->m_byPayload;
	pRcv->m_FrmHdr.m_dwDataSize		= pRtpPack->m_nRealSize ;
	pRcv->m_FrmHdr.m_dwTimeStamp	= pRtpPack->m_dwTimeStamp;
	pRcv->m_FrmHdr.m_pData			= pRtpPack->m_pRealData ;
	
	pRcv->m_tRcvStatistics.m_dwFrameNum++;
	
	if(pRcv->m_bRcvStart && pRcv->m_pFrameCallBackHandler != NULL)
	{
		pRcv->m_pFrameCallBackHandler(&pRcv->m_FrmHdr, pRcv->m_dwContext);
	}
	pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
}

void DealG722(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	if(pRcv->m_bRcvStart && pRcv->m_pRtpCallBackHandler != NULL)
    {
        pRcv->m_pRtpCallBackHandler(pRtpPack, pRcv->m_dwContext);
        return;
    }
	
	//断帧统计
	if(pRtpPack->m_wSequence != (u16)(pRcv->m_tLastInfo.m_wSeq + 1))
	{
		pRcv->m_FrmHdr.m_dwFrameID++;
		pRcv->m_tRcvStatus.m_dwFrameID = pRcv->m_FrmHdr.m_dwFrameID;
		pRcv->m_tRcvStatistics.m_dwPackLose++;
		
		if(pRtpPack->m_wSequence <= pRcv->m_tLastInfo.m_wSeq)
		{
            pRcv->m_tRcvStatistics.m_dwPackIndexError++;
		}
	}
	pRcv->m_FrmHdr.m_dwFrameID++;
    pRcv->m_tRcvStatus.m_dwFrameID  = pRcv->m_FrmHdr.m_dwFrameID;
	
	pRcv->m_FrmHdr.m_byMediaType	= pRtpPack->m_byPayload;
	pRcv->m_FrmHdr.m_dwDataSize		= pRtpPack->m_nRealSize ;
	pRcv->m_FrmHdr.m_dwTimeStamp	= pRtpPack->m_dwTimeStamp;
	pRcv->m_FrmHdr.m_pData			= pRtpPack->m_pRealData ;
	
	pRcv->m_tRcvStatistics.m_dwFrameNum++;
	
	if(pRcv->m_bRcvStart && pRcv->m_pFrameCallBackHandler != NULL)
	{
		pRcv->m_pFrameCallBackHandler(&pRcv->m_FrmHdr, pRcv->m_dwContext);
	}
	pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
}

void DealG728(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	if(pRcv->m_bRcvStart && pRcv->m_pRtpCallBackHandler != NULL)
    {
        pRcv->m_pRtpCallBackHandler(pRtpPack, pRcv->m_dwContext);
        return;
    }
	
	//断帧统计
	if(pRtpPack->m_wSequence != (u16)(pRcv->m_tLastInfo.m_wSeq + 1))
	{
		pRcv->m_FrmHdr.m_dwFrameID++;
		pRcv->m_tRcvStatus.m_dwFrameID = pRcv->m_FrmHdr.m_dwFrameID;
		pRcv->m_tRcvStatistics.m_dwPackLose++;
		
		if(pRtpPack->m_wSequence <= pRcv->m_tLastInfo.m_wSeq)
		{
            pRcv->m_tRcvStatistics.m_dwPackIndexError++;
		}
	}
	pRcv->m_FrmHdr.m_dwFrameID++;
    pRcv->m_tRcvStatus.m_dwFrameID  = pRcv->m_FrmHdr.m_dwFrameID;
	
	pRcv->m_FrmHdr.m_byMediaType	= pRtpPack->m_byPayload;
	pRcv->m_FrmHdr.m_dwDataSize		= pRtpPack->m_nRealSize ;
	pRcv->m_FrmHdr.m_dwTimeStamp	= pRtpPack->m_dwTimeStamp;
	pRcv->m_FrmHdr.m_pData			= pRtpPack->m_pRealData ;
	
	pRcv->m_tRcvStatistics.m_dwFrameNum++;
	
	if(pRcv->m_bRcvStart && pRcv->m_pFrameCallBackHandler != NULL)
	{
		pRcv->m_pFrameCallBackHandler(&pRcv->m_FrmHdr, pRcv->m_dwContext);
	}
	pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
}

void DealMpg4(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	s32  i,j,l;
	BOOL bNewFrameCome;
	s32  min;
	s32  null;
	u32  dwmin;
	s32  u;
	
	//printf("[DealMpg4] m_nRealSize1=%d\n", pRtpPack->m_nRealSize);
	
	if(TRUE == pRcv->m_bRepeatSend)
	{
		DealMpg4_RSCheck( pRcv,pRtpPack );
		return;
	}
	
	//printf("[DealMpg4] m_nRealSize2=%d\n", pRtpPack->m_nRealSize);
	
	//包的有效性判断
	if(pRtpPack->m_nRealSize < 0 || 
	   pRtpPack->m_nRealSize > MAX_EXTEND_PACK_SIZE)
	{
		pRcv->m_tRcvStatistics.m_dwPackLose++;
		pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
		pRcv->m_tLastInfo.m_dwTimeStamp = pRtpPack->m_dwTimeStamp;
		
		printf("[DealMpg4] m_nRealSize Exception, m_nRealSize=%d\n", 
			pRtpPack->m_nRealSize);
		
		return;
	}
	
	//printf("[DealMpg4] m_nRealSize3=%d\n", pRtpPack->m_nRealSize);
	
	if(pRcv->m_hSem != NULL) SemTake(pRcv->m_hSem);
	
	//查找已回调的帧队列,已回调则不作处理
	if(TRUE == FindCBQuene(pRcv,pRtpPack->m_dwTimeStamp))
	{
		if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
		return;
	}
	
	//printf("[DealMpg4] m_nRealSize4=%d\n", pRtpPack->m_nRealSize);
	
	//提取扩展数据
	pRcv->m_tMp4Header.m_byPackNum  = *(pRtpPack->m_pExData + EX_TOTALNUM_POS);
    pRcv->m_tMp4Header.m_byIndex    = *(pRtpPack->m_pExData + EX_INDEX_POS);
	
	if(pRcv->m_tMp4Header.m_byIndex < 1 || 
		pRcv->m_tMp4Header.m_byIndex > MAX_EXTEND_PACK_NUM )
	{
		pRcv->m_tRcvStatistics.m_dwPackLose++;
		pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
		pRcv->m_tLastInfo.m_dwTimeStamp = pRtpPack->m_dwTimeStamp;
		
		printf("[DealMpg4] m_byIndex Exception, m_byIndex=%d\n",pRcv->m_tMp4Header.m_byIndex);
		
		if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
		return;
	}
	
	if(pRtpPack->m_byMark)
	{
		//取的RTP的扩展数据
		pRcv->m_tMp4Header.m_nMode = (INT32)(pRtpPack->m_pExData[EX_FRAMEMODE_POS]);
		
		//视频扩展数据
		pRcv->m_tMp4Header.m_nRate = (INT32)(pRtpPack->m_pExData[EX_FRAMERATE_POS]);
		pRcv->m_tMp4Header.m_dwFrameId = 
			     ntohl(*((u32 *)(pRtpPack->m_pExData + EX_FRAMEID_POS)));
		pRcv->m_tMp4Header.m_wWidth    =
			     ntohs(*((u16 *)(pRtpPack->m_pExData + EX_WIDTH_POS)));
		pRcv->m_tMp4Header.m_wHeight   =
			     ntohs(*((u16 *)(pRtpPack->m_pExData + EX_HEIGHT_POS)));
	}
	
  	//不同源过来的数据，清空队列
	if(pRcv->m_dwLastSSRC != pRtpPack->m_dwSSRC)
	{
		pRcv->m_dwQueneNum = 0;
		
		for(i=0; i<(INT32)pRcv->m_dwMaxQueneNum; i++)
		{
			memset(pRcv->m_ptPackInfo[i], 0, sizeof(TPackInfo));
			memset(pRcv->m_ptCBPackInfo[i*2], 0, sizeof(TCBPackInfo));
			memset(pRcv->m_ptCBPackInfo[i*2+1], 0, sizeof(TCBPackInfo));	
		}
	}
	
	//帧不连续, 由于发送端有重发，故此参数不考虑	
	if(pRcv->m_dwLastSSRC == pRtpPack->m_dwSSRC &&
		pRtpPack->m_wSequence <= pRcv->m_tLastInfo.m_wSeq &&
		pRtpPack->m_wSequence != 0)
	{
		//printf("hehehe1,Net receive(0x%x),seq=%d,lastseq=%d\n",pRcv,pRtpPack->m_wSequence,pRcv->m_tLastInfo.m_wSeq);
		pRcv->m_tRcvStatistics.m_dwPackIndexError++;
	}

	/*if(pRtpPack->m_wSequence != (u16)(pRcv->m_tLastInfo.m_wSeq+1) &&
		pRtpPack->m_wSequence != 0 && pRcv->m_tLastInfo.m_wSeq != 0xFFFF)
	{
		printf("hehehe2,Net receive(0x%x),seq=%d,lastseq=%d\n",pRcv,pRtpPack->m_wSequence,pRcv->m_tLastInfo.m_wSeq);
	}*/
	
    pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
	//pRcv->m_tLastInfo.m_dwTimeStamp = pRtpPack->m_dwTimeStamp;//新加
	
	//记录是否有新的数据帧到来
	bNewFrameCome = FALSE;
	
	//查询是否队列已有相同帧的数据
	for (j=0; j<(INT32)pRcv->m_dwMaxQueneNum; j++)
	{
		if(pRcv->m_ptPackInfo[j]->m_dwTimeStamp == pRtpPack->m_dwTimeStamp)
		{
			break;
		}
	}
	//找到
	if (j < (INT32)pRcv->m_dwMaxQueneNum)
	{
		//该数据包是否已经来过
		if(pRcv->m_ptPackInfo[j]->m_byBit[pRcv->m_tMp4Header.m_byIndex] != 1)
		{
			if( MAX_EXTEND_PACK_SIZE*(pRcv->m_tMp4Header.m_byIndex-1) + pRtpPack->m_nRealSize > MAX_FRAME_SIZE)
			{
				printf("[DealMpg4] Exception 1, m_byIndex=%d, m_nRealSize=%d\n", 
						pRcv->m_tMp4Header.m_byIndex, pRtpPack->m_nRealSize);
				
				if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
				return;
			}
			
			//向队列复制缓冲
			memcpy( pRcv->m_ptPackInfo[j]->m_lpBuf + MAX_EXTEND_PACK_SIZE*(pRcv->m_tMp4Header.m_byIndex-1),
					pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
			
			//累加数据长度
			pRcv->m_ptPackInfo[j]->m_dwPackLen += pRtpPack->m_nRealSize;
			
			//将该包所在的位置置为1      
			pRcv->m_ptPackInfo[j]->m_byBit[pRcv->m_tMp4Header.m_byIndex] = 1;
			
			//是否是一帧的最后一包
			if(pRtpPack->m_byMark)
			{
				//得到帧的扩展数据
				pRcv->m_ptPackInfo[j]->m_byMediaType  = pRtpPack->m_byPayload;
				pRcv->m_ptPackInfo[j]->m_byMakerIndex = pRcv->m_tMp4Header.m_byIndex;
				
				//视频扩展数据
				pRcv->m_ptPackInfo[j]->m_bKeyFrame    = pRcv->m_tMp4Header.m_nMode;
				pRcv->m_ptPackInfo[j]->m_byFrameRate  = pRcv->m_tMp4Header.m_nRate;
				pRcv->m_ptPackInfo[j]->m_dwFrameId    = pRcv->m_tMp4Header.m_dwFrameId;
				pRcv->m_ptPackInfo[j]->m_wHeiht       = pRcv->m_tMp4Header.m_wHeight;
				pRcv->m_ptPackInfo[j]->m_wWidth       = pRcv->m_tMp4Header.m_wWidth;
			}
		}
	}
	else
	{
		bNewFrameCome = TRUE;
		
		//printf("******Net receive(0x%x) IndexError=%d,dwMaxQueneNum=%d,dwQueneNum=%d\n", 
			//pRcv,pRcv->m_tRcvStatistics.m_dwPackIndexError,pRcv->m_dwMaxQueneNum,pRcv->m_dwQueneNum);
		
		//查找最小的队列和空队列
		min  =  pRcv->m_dwMaxQueneNum + 1;
		null =  pRcv->m_dwMaxQueneNum + 1;
		dwmin = 0xffffffff;
		for(u=pRcv->m_dwMaxQueneNum-1; u>=0; u--)
		{
			if(dwmin > pRcv->m_ptPackInfo[u]->m_dwTimeStamp)
			{
				dwmin = pRcv->m_ptPackInfo[u]->m_dwTimeStamp;
				min   = u;
			}
			if(pRcv->m_ptPackInfo[u]->m_dwTimeStamp == 0) null = u;
		}
		//有空队列
		if(null != (INT32)pRcv->m_dwMaxQueneNum + 1)
		{
			pRcv->m_dwQueneNum++;
			
		    pRcv->m_ptPackInfo[null]->m_dwTimeStamp = pRtpPack->m_dwTimeStamp;
		  	pRcv->m_ptPackInfo[null]->m_dwPackLen   = 0;
			
			//记录总小包数
			pRcv->m_ptPackInfo[null]->m_dwPackNum   = pRcv->m_tMp4Header.m_byPackNum;
			//记录起始包序列
			pRcv->m_ptPackInfo[null]->m_wStartSeq   = 
						pRtpPack->m_wSequence - (pRcv->m_tMp4Header.m_byIndex - 1);
			
			if( MAX_EXTEND_PACK_SIZE*(pRcv->m_tMp4Header.m_byIndex-1) + pRtpPack->m_nRealSize > MAX_FRAME_SIZE)
			{
				printf("[DealMpg4] Exception 2, m_byIndex=%d, m_nRealSize=%d   \n", 
					pRcv->m_tMp4Header.m_byIndex, pRtpPack->m_nRealSize);
				
				if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
				return;
			}
			
			memcpy( pRcv->m_ptPackInfo[null]->m_lpBuf +
					MAX_EXTEND_PACK_SIZE*(pRcv->m_tMp4Header.m_byIndex-1),
					pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
			
			//累加数据长度
			pRcv->m_ptPackInfo[null]->m_dwPackLen += pRtpPack->m_nRealSize;
			
			//将该包所在的位置置为1      
			pRcv->m_ptPackInfo[null]->m_byBit[pRcv->m_tMp4Header.m_byIndex] = 1;
			
			//是否是一帧的最后一包
			if(pRtpPack->m_byMark)
			{
				pRcv->m_ptPackInfo[null]->m_byMediaType  = pRtpPack->m_byPayload;
				pRcv->m_ptPackInfo[null]->m_byMakerIndex = pRcv->m_tMp4Header.m_byIndex;
				
				//视频扩展数据
				pRcv->m_ptPackInfo[null]->m_bKeyFrame    = pRcv->m_tMp4Header.m_nMode;
				pRcv->m_ptPackInfo[null]->m_byFrameRate  = pRcv->m_tMp4Header.m_nRate;
				pRcv->m_ptPackInfo[null]->m_dwFrameId    = pRcv->m_tMp4Header.m_dwFrameId;
				pRcv->m_ptPackInfo[null]->m_wHeiht       = pRcv->m_tMp4Header.m_wHeight;
				pRcv->m_ptPackInfo[null]->m_wWidth       = pRcv->m_tMp4Header.m_wWidth;
			}
		}
		else
		{
			//最小队列
			pRcv->m_tRcvStatistics.m_dwPackLose++;//可能少统计
			
			//接收的组包缓冲队列已满，将踢出一个最早到达同时没有组包完全的一个数据帧
			//printf("[DealMpg4] Net receive lose pack:%d  \n", 
											//pRcv->m_tRcvStatistics.m_dwPackLose);
			
			//printf("[DealMpg4] Net receive(0x%x) lose pack:%d,dwMaxQueneNum=%d,dwQueneNum=%d\n", 
											//pRcv,pRcv->m_tRcvStatistics.m_dwPackLose,pRcv->m_dwMaxQueneNum,pRcv->m_dwQueneNum);
			
			//将丢弃过的数据帧记录到回调的帧队列，防止重复回调给上层
			FillCBQuene(pRcv,pRcv->m_ptPackInfo[min]->m_dwTimeStamp);
			
			memset(pRcv->m_ptPackInfo[min]->m_byBit, 0, sizeof(pRcv->m_ptPackInfo[min]->m_byBit));
			pRcv->m_ptPackInfo[min]->m_dwTimeStamp  = pRtpPack->m_dwTimeStamp;
			pRcv->m_ptPackInfo[min]->m_dwPackLen    = 0;
			pRcv->m_ptPackInfo[min]->m_byMakerIndex = 0;
			
			//记录总小包数
			pRcv->m_ptPackInfo[min]->m_dwPackNum   = pRcv->m_tMp4Header.m_byPackNum;
			//记录起始包序列
			pRcv->m_ptPackInfo[min]->m_wStartSeq   = 
				pRtpPack->m_wSequence - (pRcv->m_tMp4Header.m_byIndex - 1);
			
			if( MAX_EXTEND_PACK_SIZE*(pRcv->m_tMp4Header.m_byIndex-1) + pRtpPack->m_nRealSize > MAX_FRAME_SIZE)
			{
				printf("[DealMpg4] Exception 3, m_byIndex=%d, m_nRealSize=%d\n", 
									  pRcv->m_tMp4Header.m_byIndex, pRtpPack->m_nRealSize);
				
				if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
				return;
			}
			
			memcpy( pRcv->m_ptPackInfo[min]->m_lpBuf + MAX_EXTEND_PACK_SIZE*(pRcv->m_tMp4Header.m_byIndex-1),
					pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
			
			//累加数据长度
			pRcv->m_ptPackInfo[min]->m_dwPackLen += pRtpPack->m_nRealSize;
			
			//将该包所在的位置置为1      
			pRcv->m_ptPackInfo[min]->m_byBit[pRcv->m_tMp4Header.m_byIndex] = 1;
			
			//是否是一帧的最后一包
			if(pRtpPack->m_byMark)
			{
				pRcv->m_ptPackInfo[min]->m_byMediaType  = pRtpPack->m_byPayload;
				pRcv->m_ptPackInfo[min]->m_byMakerIndex = pRcv->m_tMp4Header.m_byIndex;
				
				//视频扩展数据
				pRcv->m_ptPackInfo[min]->m_bKeyFrame    = pRcv->m_tMp4Header.m_nMode;
				pRcv->m_ptPackInfo[min]->m_byFrameRate  = pRcv->m_tMp4Header.m_nRate;
				pRcv->m_ptPackInfo[min]->m_dwFrameId    = pRcv->m_tMp4Header.m_dwFrameId;
				pRcv->m_ptPackInfo[min]->m_wHeiht       = pRcv->m_tMp4Header.m_wHeight;
				pRcv->m_ptPackInfo[min]->m_wWidth       = pRcv->m_tMp4Header.m_wWidth;
			}
		}
	}
	
	pRcv->m_dwLastSSRC = pRtpPack->m_dwSSRC;
	//找出满队列，回调给上层
	l=FindFullQuene(pRcv);
	if(l < (INT32)pRcv->m_dwMaxQueneNum)
	{
		//csp add 2008-11-28
		u32 LastRcvFrameId = pRcv->m_tRcvStatus.m_dwFrameID;
		
		//设置帧信息
		pRcv->m_FrmHdr.m_dwPreBufSize					= 0;
		pRcv->m_FrmHdr.m_byMediaType  					= pRcv->m_ptPackInfo[l]->m_byMediaType;
		pRcv->m_FrmHdr.m_dwDataSize						= pRcv->m_ptPackInfo[l]->m_dwPackLen;
		pRcv->m_FrmHdr.m_pData							= pRcv->m_ptPackInfo[l]->m_lpBuf;
		pRcv->m_FrmHdr.m_dwTimeStamp					= pRcv->m_ptPackInfo[l]->m_dwTimeStamp;
		
		//mpeg4视频包		 	 
		pRcv->m_FrmHdr.m_byFrameRate					= pRcv->m_ptPackInfo[l]->m_byFrameRate;
		pRcv->m_FrmHdr.m_dwFrameID						= pRcv->m_ptPackInfo[l]->m_dwFrameId;
		pRcv->m_tRcvStatus.m_dwFrameID					= pRcv->m_FrmHdr.m_dwFrameID; 
		
		pRcv->m_FrmHdr.m_tVideoParam.m_bKeyFrame		= pRcv->m_ptPackInfo[l]->m_bKeyFrame;   
		pRcv->m_FrmHdr.m_tVideoParam.m_wVideoHeight		= pRcv->m_ptPackInfo[l]->m_wHeiht;
		pRcv->m_FrmHdr.m_tVideoParam.m_wVideoWidth		= pRcv->m_ptPackInfo[l]->m_wWidth;
		
		//将回调过的数据帧记录到回调的帧队列，防止重复回调给上层
		FillCBQuene(pRcv,pRcv->m_ptPackInfo[l]->m_dwTimeStamp);
		
		//csp add 2008-11-28
		if(LastRcvFrameId != 0 && LastRcvFrameId + 1 != pRcv->m_tRcvStatus.m_dwFrameID)
		{
			if(!pRcv->m_FrmHdr.m_tVideoParam.m_bKeyFrame)
			{
				//printf("lose frame:(last id:%d,recv id:%d)\n",LastRcvFrameId,pRcv->m_tRcvStatus.m_dwFrameID);
				DealNetRcvFrameLoseEvent(pRcv,pRcv->m_tRcvStatus.m_dwFrameID-1);
			}
		}
		
		//回调
		pRcv->m_tRcvStatistics.m_dwFrameNum++;
		//printf("call back\n");
		if(pRcv->m_bRcvStart && pRcv->m_pFrameCallBackHandler != NULL)
		{
			pRcv->m_pFrameCallBackHandler(&pRcv->m_FrmHdr, pRcv->m_dwContext);
		}
		
		//帧缓冲清空
		memset(pRcv->m_ptPackInfo[l]->m_byBit, 0, sizeof(pRcv->m_ptPackInfo[l]->m_byBit));
		pRcv->m_ptPackInfo[l]->m_dwTimeStamp    = 0;
		pRcv->m_ptPackInfo[l]->m_dwPackLen      = 0;
		pRcv->m_ptPackInfo[l]->m_byMakerIndex   = 0;
		pRcv->m_ptPackInfo[l]->m_dwPackNum      = 0;
		
		pRcv->m_dwQueneNum--;
	}
	
	if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
}

void DealH264(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	DealMpg4( pRcv,pRtpPack );
}

void DealMp3(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	
}

void DealMpg4_RSCheck(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack)
{
	s32 i = 0;
	s32 j = 0;
	
	//包的有效性判断
	if(pRtpPack->m_nRealSize < 0 || 
	   pRtpPack->m_nRealSize > MAX_EXTEND_PACK_SIZE)
	{
		pRcv->m_tRcvStatistics.m_dwPackLose++;
		pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
		pRcv->m_tLastInfo.m_dwTimeStamp = pRtpPack->m_dwTimeStamp;
		
		printf("[DealMpg4_RSCheck] m_nRealSize Exception, m_nRealSize=%d   \n", 
			pRtpPack->m_nRealSize);
		
		return;
	}
	
	//提取扩展数据
	pRcv->m_tMp4Header.m_byPackNum = *(pRtpPack->m_pExData + EX_TOTALNUM_POS);
    pRcv->m_tMp4Header.m_byIndex = *(pRtpPack->m_pExData + EX_INDEX_POS);
	
	if(pRcv->m_tMp4Header.m_byIndex < 1 || 
		pRcv->m_tMp4Header.m_byIndex > MAX_EXTEND_PACK_NUM )
	{
		pRcv->m_tRcvStatistics.m_dwPackLose++;
		pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
		pRcv->m_tLastInfo.m_dwTimeStamp = pRtpPack->m_dwTimeStamp;
		
		printf("[DealMpg4_RSCheck] m_byIndex Exception, m_byIndex=%d   \n", 
			pRcv->m_tMp4Header.m_byIndex);
		
		return;
	}
	
	if(pRtpPack->m_byMark)
	{
		//取得RTP的扩展数据
		pRcv->m_tMp4Header.m_nMode = (s32)(pRtpPack->m_pExData[EX_FRAMEMODE_POS]);
		
		//视频扩展数据
		pRcv->m_tMp4Header.m_nRate = (s32)(pRtpPack->m_pExData[EX_FRAMERATE_POS]);
		pRcv->m_tMp4Header.m_dwFrameId = 
				 ntohl(*((u32 *)(pRtpPack->m_pExData + EX_FRAMEID_POS)));
		pRcv->m_tMp4Header.m_wWidth    =
				 ntohs(*((u16 *)(pRtpPack->m_pExData + EX_WIDTH_POS)));
		pRcv->m_tMp4Header.m_wHeight   =
				 ntohs(*((u16 *)(pRtpPack->m_pExData + EX_HEIGHT_POS)));
	}
	
	if(pRcv->m_hSem != NULL) SemTake(pRcv->m_hSem);
	
  	//不同源过来的数据，清空队列
	if(pRcv->m_dwLastSSRC != pRtpPack->m_dwSSRC)
	{
		pRcv->m_dwQueneNum			= 0;
		pRcv->m_dwMinTimeStamp		= 0;
		pRcv->m_dwMaxTimeStamp		= 0;
		pRcv->m_dwLastTimeStamp		= 0;
		
		for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
		{
			memset(pRcv->m_ptPackInfo[i], 0, sizeof(TPackInfo));
			memset(pRcv->m_ptCBPackInfo[i*2], 0, sizeof(TCBPackInfo));
			memset(pRcv->m_ptCBPackInfo[i*2+1], 0, sizeof(TCBPackInfo));	
		}
	}
	
	//帧不连续, 由于发送端有重发，故此参数不考虑	
	if(pRcv->m_dwLastSSRC == pRtpPack->m_dwSSRC&&
		pRtpPack->m_wSequence <= pRcv->m_tLastInfo.m_wSeq&&
		pRtpPack->m_wSequence != 0)
	{
		//printf("ssrc:(0x%08x,0x%08x),seq:(%u,%u)\n",pRtpPack->m_dwSSRC,pRcv->m_dwLastSSRC,pRtpPack->m_wSequence,pRcv->m_tLastInfo.m_wSeq);
		
		pRcv->m_tRcvStatistics.m_dwPackIndexError++;
	}
	
    pRcv->m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
	
	//上次回调或丢弃过的 对应的时间戳,已过期则不作处理
	if(0 != pRcv->m_dwLastTimeStamp && 
		pRtpPack->m_dwTimeStamp <= pRcv->m_dwLastTimeStamp)
	{
		//printf("time stamp is too old:(0x%08x < 0x%08x)\n",pRtpPack->m_dwTimeStamp,pRcv->m_dwLastTimeStamp);
		
		pRcv->m_dwLastSSRC = pRtpPack->m_dwSSRC;
		
		if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
		
		return;
	}
	else
	{
		//printf("##########time stamp is not old:(0x%08x > 0x%08x)\n",pRtpPack->m_dwTimeStamp,pRcv->m_dwLastTimeStamp);
	}
	
	//查询是否队列已有相同帧的数据
	for (j=0; j<(s32)pRcv->m_dwMaxQueneNum; j++)
	{
		if (pRcv->m_ptPackInfo[j]->m_dwTimeStamp == pRtpPack->m_dwTimeStamp)
		{
			//printf("same time stamp,index=%d\n",j);
			break;
		}
	}
	//找到
	if (j < (s32)pRcv->m_dwMaxQueneNum)
	{
		//该数据包是否已经来过
		if (pRcv->m_ptPackInfo[j]->m_byBit[pRcv->m_tMp4Header.m_byIndex] != 1)
		{
			if( MAX_EXTEND_PACK_SIZE*
				(pRcv->m_tMp4Header.m_byIndex-1) + pRtpPack->m_nRealSize > MAX_FRAME_SIZE)
			{
				printf("[DealMpg4_RSCheck] Exception 1, m_byIndex=%d, m_nRealSize=%d   \n", 
					pRcv->m_tMp4Header.m_byIndex, pRtpPack->m_nRealSize);
				
				if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
				return;
			}
			
			//向队列复制缓冲
			memcpy( pRcv->m_ptPackInfo[j]->m_lpBuf +
				    MAX_EXTEND_PACK_SIZE*(pRcv->m_tMp4Header.m_byIndex-1),
					pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
			
			//累加数据长度
            pRcv->m_ptPackInfo[j]->m_dwPackLen += pRtpPack->m_nRealSize;
			
			//将该包所在的位置置为1      
			pRcv->m_ptPackInfo[j]->m_byBit[pRcv->m_tMp4Header.m_byIndex] = 1;
			
			//是否是一帧的最后一包
			if(pRtpPack->m_byMark)
			{
				//得到帧的扩展数据
				pRcv->m_ptPackInfo[j]->m_byMediaType  = pRtpPack->m_byPayload;
				pRcv->m_ptPackInfo[j]->m_byMakerIndex = pRcv->m_tMp4Header.m_byIndex;
				
				//视频扩展数据
				pRcv->m_ptPackInfo[j]->m_bKeyFrame    = pRcv->m_tMp4Header.m_nMode;
				pRcv->m_ptPackInfo[j]->m_byFrameRate  = pRcv->m_tMp4Header.m_nRate;
				pRcv->m_ptPackInfo[j]->m_dwFrameId    = pRcv->m_tMp4Header.m_dwFrameId;
				pRcv->m_ptPackInfo[j]->m_wHeiht       = pRcv->m_tMp4Header.m_wHeight;
				pRcv->m_ptPackInfo[j]->m_wWidth       = pRcv->m_tMp4Header.m_wWidth;
		   }
		}
		//回调、重排、更新 最低位及最高位 时间戳 记录
		CallBackAndSerial(pRcv,FALSE);
	}
	else
	{
		//队列已满
		if(pRcv->m_dwQueneNum == pRcv->m_dwMaxQueneNum)
		{
			//比 最低位 时间戳 还小
			if(pRtpPack->m_dwTimeStamp < pRcv->m_dwMinTimeStamp)
			{
				//丢弃
			}
			else if(pRtpPack->m_dwTimeStamp > pRcv->m_dwMaxTimeStamp)
			{
				s32 u = 0;
				
				//计算新的 最低位 时间戳
				//s32 nMin = (pRtpPack->m_dwTimeStamp) + VIDEO_TIME_SPAN - pRcv->m_tRSParam.m_wRejectTimeSpan;
				u32 nMin = (pRtpPack->m_dwTimeStamp) + VIDEO_TIME_SPAN - pRcv->m_tRSParam.m_wRejectTimeSpan;
				if(nMin < 0)
				{
					nMin = pRtpPack->m_dwTimeStamp % VIDEO_TIME_SPAN;
				}
				
				//小于 dwMin 都进行丢弃，
				//同时做回调、重排、更新 最低位及最高位 时间戳 记录
				for(u=0; u<(s32)pRcv->m_dwMaxQueneNum; u++)
				{
					if(pRcv->m_ptPackInfo[0]->m_dwTimeStamp < (u32)nMin)
					{
						CallBackAndSerial(pRcv,TRUE);
					}
				}
				//放入队列 
				InsertQuene( pRcv, pRtpPack, INSERT_NEW_MAX_POS );
				
				//然后做回调、重排、更新 最低位及最高位 时间戳 记录
				CallBackAndSerial(pRcv,FALSE);
				
				//重传检测  (回调 重排 处理)
				DealNetRcvRSQSndQuest(pRcv,pRtpPack->m_dwTimeStamp);
			}
			else
			{
				//放入队列
				InsertQuene( pRcv, pRtpPack, INSERT_NEW_MID_POS );
				
				//然后做回调、重排、更新 最低位及最高位 时间戳 记录
				CallBackAndSerial(pRcv,FALSE);
				
				//重传检测  (回调 重排 处理)
				DealNetRcvRSQSndQuest(pRcv,pRtpPack->m_dwTimeStamp);
			}
		}
		//队列未满
		else
		{
			u32 nMin = 0;//s32 nMin = 0;//csp modify 2008-11-26
			u32 dwMax = 0;
			
			if((0 == pRcv->m_dwMinTimeStamp &&
			    0 == pRcv->m_dwMaxTimeStamp ))
			{
				//更新 最低位及最高位 时间戳 记录
				pRcv->m_dwMinTimeStamp = pRtpPack->m_dwTimeStamp;
				pRcv->m_dwMaxTimeStamp = pRtpPack->m_dwTimeStamp;
				
				//放入队列
				InsertQuene( pRcv, pRtpPack, INSERT_NEW_MID_POS );
				
				//然后做回调、重排、更新 最低位及最高位 时间戳 记录
				CallBackAndSerial(pRcv,FALSE);
				
				pRcv->m_dwLastSSRC = pRtpPack->m_dwSSRC;
				
				if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
				
				return;
			}		
			
			//计算可能的 最低位 时间戳
			nMin = pRcv->m_dwMaxTimeStamp + VIDEO_TIME_SPAN - pRcv->m_tRSParam.m_wRejectTimeSpan;
			if(nMin < 0)
			{
				nMin = pRcv->m_dwMaxTimeStamp / VIDEO_TIME_SPAN;
			}
			
			//计算新的 最高位 时间戳
			dwMax = pRcv->m_dwMinTimeStamp - VIDEO_TIME_SPAN + pRcv->m_tRSParam.m_wRejectTimeSpan;
			
			//小于（最大帧－缓冲总时间跨度）
			if(pRtpPack->m_dwTimeStamp < (u32)nMin)
			{
				//丢弃
			}
			else if(pRtpPack->m_dwTimeStamp > dwMax)
			{
				s32 nUpper = 0;
				
				//计算新的 最低位 时间戳
				//csp modify 2008-11-26
				//s32 nMin = (pRtpPack->m_dwTimeStamp) + VIDEO_TIME_SPAN - pRcv->m_tRSParam.m_wRejectTimeSpan;
				u32 nMin = (pRtpPack->m_dwTimeStamp) + VIDEO_TIME_SPAN - pRcv->m_tRSParam.m_wRejectTimeSpan;
				if(nMin < 0)
				{
					nMin = pRtpPack->m_dwTimeStamp % VIDEO_TIME_SPAN;
				}
				
				nUpper = (s32)(pRcv->m_dwMaxTimeStamp - pRcv->m_dwMinTimeStamp) / VIDEO_TIME_SPAN;
				
				//小于 dwMin 都进行丢弃，
				//同时做回调、重排、更新 最低位及最高位 时间戳 记录
				
				if(nUpper > 0)
				{
					s32 u = 0;
					for(u=0; u<(s32)nUpper+1; u++)
					{
						if(pRcv->m_ptPackInfo[0]->m_dwTimeStamp < (u32)nMin)
						{
							CallBackAndSerial(pRcv,TRUE);
						}
					}
				}
				
				//放入队列 
				InsertQuene( pRcv, pRtpPack, INSERT_NEW_MAX_POS );
				
				//然后做回调、重排、更新 最低位及最高位 时间戳 记录
				CallBackAndSerial(pRcv,FALSE);
				
				//重传检测  (回调 重排 处理)
				DealNetRcvRSQSndQuest(pRcv,pRtpPack->m_dwTimeStamp);
			}
			else if(pRtpPack->m_dwTimeStamp < pRcv->m_dwMinTimeStamp)
			{
				//放入队列 
				InsertQuene( pRcv, pRtpPack, INSERT_NEW_MIN_POS );
				
				//然后做回调、重排、更新 最低位及最高位 时间戳 记录
				CallBackAndSerial(pRcv,FALSE);
			}
			else
			{
				//放入队列 
				InsertQuene( pRcv, pRtpPack, INSERT_NEW_MID_POS );
				
				//然后做回调、重排、更新 最低位及最高位 时间戳 记录
				CallBackAndSerial(pRcv,FALSE);
				
				//重传检测 (回调 重排 处理)
				DealNetRcvRSQSndQuest(pRcv,pRtpPack->m_dwTimeStamp);
			}
		}
	}
	
	pRcv->m_dwLastSSRC = pRtpPack->m_dwSSRC;
	
	if(pRcv->m_hSem != NULL) SemGive(pRcv->m_hSem);
}

BOOL FindCBQuene(ifly_netrcv_t *pRcv,u32 dwTimeStamp)
{
	BOOL bRet = FALSE;
	s32 nIndex;
	
	for(nIndex=0; nIndex<(INT32)pRcv->m_dwMaxQueneNum*2; nIndex++)
	{
		if(pRcv->m_ptCBPackInfo[nIndex]->m_dwTimeStamp == dwTimeStamp)
		{
			bRet = TRUE;
			break;
		}
	}
	
	return bRet;
}

void FillCBQuene(ifly_netrcv_t *pRcv,u32 dwTimeStamp)
{
	//搜索时间戳最小队列元素(空闲，如果队列元素已满，则覆盖时间戳最小的元素)
	
	s32 nIndex = 0;
	s32 nMinIndex = 0;
	u32	dwMinTimeStamp = 0xffffffff;
	
	for(nIndex=0; nIndex<(INT32)pRcv->m_dwMaxQueneNum*2; nIndex++)
	{
		if(0 == pRcv->m_ptCBPackInfo[nIndex]->m_dwTimeStamp)
		{
			nMinIndex = nIndex;
			break;
		}
		if(pRcv->m_ptCBPackInfo[nIndex]->m_dwTimeStamp < dwMinTimeStamp)
		{
			dwMinTimeStamp = pRcv->m_ptCBPackInfo[nIndex]->m_dwTimeStamp;
			nMinIndex = nIndex;
		}
	}
	pRcv->m_ptCBPackInfo[nMinIndex]->m_dwTimeStamp = dwTimeStamp;
}

s32 FindFullQuene(ifly_netrcv_t *pRcv)
{
	s32  u,j;
	BOOL bFull;
	s32  min = pRcv->m_dwMaxQueneNum + 1;
	u32  dwTimeStamp = 0xffffffff;
	
	//搜索时间戳最小队列
	for(u=0; u<(INT32)pRcv->m_dwMaxQueneNum; u++)
	{
		if(pRcv->m_ptPackInfo[u]->m_dwTimeStamp < dwTimeStamp &&
			pRcv->m_ptPackInfo[u]->m_dwTimeStamp != 0)
		{
			min = u;
			dwTimeStamp = pRcv->m_ptPackInfo[u]->m_dwTimeStamp;
		}
	}
	
	//查找最小的队列是否已满
	bFull = TRUE;
	if(pRcv->m_ptPackInfo[min]->m_byMakerIndex != 0)
	{
		for(j=1; j<=pRcv->m_ptPackInfo[min]->m_byMakerIndex; j++)
		{
			if(pRcv->m_ptPackInfo[min]->m_byBit[j] == 0) 
			{
				bFull = FALSE;
				break;
			}
		}
	}
	else
	{
		bFull = FALSE;
	}
	
	//返回最小队列
	return (bFull ? min : (pRcv->m_dwMaxQueneNum + 1));
}

void CallBackAndSerial(ifly_netrcv_t *pRcv,BOOL bDisCard)
{
	//检测队列第一帧是否已满
	
	TPackInfo* ptPackInfo = pRcv->m_ptPackInfo[0];
	
	BOOL bFull = TRUE;
	
	if(ptPackInfo->m_byMakerIndex != 0)
	{
		s32 i = 0;
		for(i=1; i<=ptPackInfo->m_byMakerIndex; i++)
		{
			if(ptPackInfo->m_byBit[i] == 0) 
			{
				bFull = FALSE;
				break;
			}
		}
	}
	else
	{
         bFull = FALSE;
	}
	
	ptPackInfo = pRcv->m_ptPackInfo[0];
	
	//已满则回调重排
	if(TRUE == bFull)
	{
		s32 nUpper = 0;
		
		//设置帧信息
		pRcv->m_FrmHdr.m_dwPreBufSize	= 0;
		pRcv->m_FrmHdr.m_byMediaType  	= ptPackInfo->m_byMediaType;
		pRcv->m_FrmHdr.m_dwDataSize		= ptPackInfo->m_dwPackLen;
		pRcv->m_FrmHdr.m_pData          = ptPackInfo->m_lpBuf;
		pRcv->m_FrmHdr.m_dwTimeStamp    = ptPackInfo->m_dwTimeStamp;
		
		//mpeg4视频包		 	 
		pRcv->m_FrmHdr.m_byFrameRate	= ptPackInfo->m_byFrameRate;
		pRcv->m_FrmHdr.m_dwFrameID		= ptPackInfo->m_dwFrameId;
		pRcv->m_tRcvStatus.m_dwFrameID	= pRcv->m_FrmHdr.m_dwFrameID; 
		
		pRcv->m_FrmHdr.m_tVideoParam.m_bKeyFrame	= ptPackInfo->m_bKeyFrame;   
		pRcv->m_FrmHdr.m_tVideoParam.m_wVideoHeight = ptPackInfo->m_wHeiht;
		pRcv->m_FrmHdr.m_tVideoParam.m_wVideoWidth  = ptPackInfo->m_wWidth;
		
		//回调
		pRcv->m_tRcvStatistics.m_dwFrameNum++;
		if(pRcv->m_bRcvStart && pRcv->m_pFrameCallBackHandler != NULL)
		{
			pRcv->m_pFrameCallBackHandler(&pRcv->m_FrmHdr, pRcv->m_dwContext);
		}
		
		//排序, 向前挪移一位
		nUpper = (s32)(pRcv->m_dwMaxTimeStamp - pRcv->m_dwMinTimeStamp) / VIDEO_TIME_SPAN;
		
		if(nUpper > 0)
		{
			s32 u = 0;
			for(u=1; u<(s32)(nUpper+1); u++)
			{
				pRcv->m_ptPackInfo[u-1] = pRcv->m_ptPackInfo[u];
			}
			pRcv->m_ptPackInfo[nUpper] = ptPackInfo;
		}
		
		pRcv->m_dwMinTimeStamp += VIDEO_TIME_SPAN;
		if(1 == pRcv->m_dwQueneNum)
		{
			pRcv->m_dwMaxTimeStamp = pRcv->m_dwMinTimeStamp;		
		}
		
		pRcv->m_dwQueneNum--;
		
		//更新 回调或丢弃过的 对应的时间戳
		if(pRcv->m_dwLastTimeStamp < ptPackInfo->m_dwTimeStamp)
		{
			pRcv->m_dwLastTimeStamp = ptPackInfo->m_dwTimeStamp;
		}		
		
		//帧缓冲清空
		memset(ptPackInfo,0,sizeof(TPackInfo));//csp add 2008-11-26
		memset(ptPackInfo->m_byBit, 0, sizeof(ptPackInfo->m_byBit));
		ptPackInfo->m_dwTimeStamp    = 0;
		ptPackInfo->m_dwPackLen      = 0;
		ptPackInfo->m_byMakerIndex   = 0;
		ptPackInfo->m_dwPackNum      = 0;
		
		return;
	}
	
	if(TRUE == bDisCard)
	{
		//排序, 向前挪移一位
		s32 nUpper = (s32)(pRcv->m_dwMaxTimeStamp - pRcv->m_dwMinTimeStamp) / VIDEO_TIME_SPAN;
		
		if(nUpper > 0)
		{
			s32 u = 0;
			for(u=1; u<(s32)(nUpper+1); u++)
			{
				pRcv->m_ptPackInfo[u-1] = pRcv->m_ptPackInfo[u];
			}
			pRcv->m_ptPackInfo[nUpper] = ptPackInfo;
		}
		
		//更新 最低位及最高位 时间戳 记录
		
		if(pRcv->m_dwMinTimeStamp == pRcv->m_dwMaxTimeStamp)
		{
			pRcv->m_dwMinTimeStamp += VIDEO_TIME_SPAN;
			pRcv->m_dwMaxTimeStamp += VIDEO_TIME_SPAN;
		}
		else
		{
			pRcv->m_dwMinTimeStamp += VIDEO_TIME_SPAN;
		}
		
		if(0 != ptPackInfo->m_dwTimeStamp)
		{
			if(1 == pRcv->m_dwQueneNum)
			{
				pRcv->m_dwMaxTimeStamp = pRcv->m_dwMinTimeStamp;		
			}
			pRcv->m_dwQueneNum--;
		}
		
		//更新 回调或丢弃过的 对应的时间戳
		if(0 != ptPackInfo->m_dwTimeStamp)
		{
			if(pRcv->m_dwLastTimeStamp < ptPackInfo->m_dwTimeStamp)
			{
				pRcv->m_dwLastTimeStamp = ptPackInfo->m_dwTimeStamp;
			}			
		}
		else
		{
			//更新 回调或丢弃过的 对应的时间戳
			if(pRcv->m_dwLastTimeStamp < (pRcv->m_dwMinTimeStamp - VIDEO_TIME_SPAN))
			{
				pRcv->m_dwLastTimeStamp = pRcv->m_dwMinTimeStamp - VIDEO_TIME_SPAN;
			}	
		}
		
		//帧缓冲清空
		memset(ptPackInfo,0,sizeof(TPackInfo));//csp add 2008-11-26
		memset(ptPackInfo->m_byBit, 0, sizeof(ptPackInfo->m_byBit));
		ptPackInfo->m_dwTimeStamp    = 0;
		ptPackInfo->m_dwPackLen      = 0;
		ptPackInfo->m_byMakerIndex   = 0;
		ptPackInfo->m_dwPackNum      = 0;
	}
}

void InsertQuene(ifly_netrcv_t *pRcv,TRtpPack *pRtpPack,u16 wPos)
{
	u32 dwOffset = pRcv->m_dwMaxQueneNum;
	
	//比队列最高位还大
	if(INSERT_NEW_MAX_POS == wPos)
	{
		dwOffset = (pRtpPack->m_dwTimeStamp - pRcv->m_dwMinTimeStamp) / VIDEO_TIME_SPAN;
		if(dwOffset >= pRcv->m_dwMaxQueneNum)
		{
			if(0 != pRcv->m_dwQueneNum)
			{
				s32 i = 0;
				for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
				{
					memset(pRcv->m_ptPackInfo[i], 0, sizeof(TPackInfo));
					memset(pRcv->m_ptCBPackInfo[i*2], 0, sizeof(TCBPackInfo));
					memset(pRcv->m_ptCBPackInfo[i*2+1], 0, sizeof(TCBPackInfo));	
				}
				pRcv->m_dwQueneNum = 0;
			}
		}
		
		if(0 == pRcv->m_dwQueneNum)
		{
			dwOffset = 0;
			pRcv->m_dwMinTimeStamp = pRtpPack->m_dwTimeStamp;
		}
		//更新 最高位 时间戳 记录
		pRcv->m_dwMaxTimeStamp = pRtpPack->m_dwTimeStamp;
	}
	
	//处在队列中间
	if(INSERT_NEW_MID_POS == wPos)
	{
		dwOffset = (pRtpPack->m_dwTimeStamp - pRcv->m_dwMinTimeStamp) / VIDEO_TIME_SPAN;	
		if(pRtpPack->m_dwTimeStamp > pRcv->m_dwMaxTimeStamp)
		{
			//更新 最高位 时间戳 记录
			pRcv->m_dwMaxTimeStamp = pRtpPack->m_dwTimeStamp;
		}
	}
	
	//比队列最低位还小
	if(INSERT_NEW_MIN_POS == wPos)
	{
		s32 nUpper = (s32)(pRcv->m_dwMaxTimeStamp - pRcv->m_dwMinTimeStamp) / VIDEO_TIME_SPAN;
		dwOffset = (pRcv->m_dwMinTimeStamp - pRtpPack->m_dwTimeStamp) / VIDEO_TIME_SPAN;
		
		//csp add 2008-11-25
		if(nUpper + dwOffset >= pRcv->m_dwMaxQueneNum)
		{
			if(dwOffset >= pRcv->m_dwMaxQueneNum)
			{
				if(0 != pRcv->m_dwQueneNum)
				{
					s32 i = 0;
					for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
					{
						memset(pRcv->m_ptPackInfo[i], 0, sizeof(TPackInfo));
						memset(pRcv->m_ptCBPackInfo[i*2], 0, sizeof(TCBPackInfo));
						memset(pRcv->m_ptCBPackInfo[i*2+1], 0, sizeof(TCBPackInfo));	
					}
					pRcv->m_dwQueneNum = 0;
				}
				
				pRcv->m_dwMinTimeStamp = pRtpPack->m_dwTimeStamp;
				pRcv->m_dwMaxTimeStamp = pRtpPack->m_dwTimeStamp;
				
				dwOffset = 0;
				
				//return;
			}
			else
			{
				if(0 != pRcv->m_dwQueneNum)
				{
					s32 i = 0;
					for(i=0; i<(s32)pRcv->m_dwMaxQueneNum; i++)
					{
						memset(pRcv->m_ptPackInfo[i], 0, sizeof(TPackInfo));
						memset(pRcv->m_ptCBPackInfo[i*2], 0, sizeof(TCBPackInfo));
						memset(pRcv->m_ptCBPackInfo[i*2+1], 0, sizeof(TCBPackInfo));	
					}
					pRcv->m_dwQueneNum = 0;
				}
				
				pRcv->m_dwMinTimeStamp = pRtpPack->m_dwTimeStamp;
				pRcv->m_dwMaxTimeStamp = pRtpPack->m_dwTimeStamp;
				
				dwOffset = 0;
				
				//return;
			}
		}
		else
		{
			//csp modify 2008-11-26
			//if(dwOffset > 0 && nUpper > 0)
			if(dwOffset > 0 && nUpper >= 0)
			{
				s32 u = 0;
				TPackInfo* ptPackInfo = pRcv->m_ptPackInfo[0];
				for(u=nUpper+1; u>0; u--)
				{
					ptPackInfo = pRcv->m_ptPackInfo[u+dwOffset-1];
					pRcv->m_ptPackInfo[u+dwOffset-1] = pRcv->m_ptPackInfo[u-1];
					pRcv->m_ptPackInfo[u-1] = ptPackInfo;
				}
			}
			
			//更新 最低位 时间戳 记录
			pRcv->m_dwMinTimeStamp = pRtpPack->m_dwTimeStamp;
			
			//csp modify 2008-11-26
			/*if(nUpper <= 0)
			{
				pRcv->m_dwMaxTimeStamp = pRcv->m_dwMinTimeStamp;
			}*/
			if(nUpper < 0)
			{
				printf("error,nUpper=%d\n",nUpper);
				pRcv->m_dwMaxTimeStamp = pRcv->m_dwMinTimeStamp;
			}
			
			dwOffset = 0;
		}
	}
	
	//边界保护，防止越界
	if(dwOffset < pRcv->m_dwMaxQueneNum)
	{
		if( MAX_EXTEND_PACK_SIZE*
			(pRcv->m_tMp4Header.m_byIndex-1) + pRtpPack->m_nRealSize > MAX_FRAME_SIZE)
		{
			printf("[DealMpg4] Exception 2, m_byIndex=%d, m_nRealSize=%d   \n", 
				pRcv->m_tMp4Header.m_byIndex, pRtpPack->m_nRealSize);
			
			return;
		}
		
		pRcv->m_ptPackInfo[dwOffset]->m_dwTimeStamp = pRtpPack->m_dwTimeStamp;
		pRcv->m_ptPackInfo[dwOffset]->m_dwPackLen   = 0;
		
		//记录总小包数
		pRcv->m_ptPackInfo[dwOffset]->m_dwPackNum   = pRcv->m_tMp4Header.m_byPackNum;
		//记录起始包序列
		pRcv->m_ptPackInfo[dwOffset]->m_wStartSeq   = 
			pRtpPack->m_wSequence - (pRcv->m_tMp4Header.m_byIndex - 1);
		
		//向队列复制缓冲
		memcpy( pRcv->m_ptPackInfo[dwOffset]->m_lpBuf +
				MAX_EXTEND_PACK_SIZE * (pRcv->m_tMp4Header.m_byIndex-1),
				pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
		
		//累加数据长度
		pRcv->m_ptPackInfo[dwOffset]->m_dwPackLen += pRtpPack->m_nRealSize;
		
		//将该包所在的位置置为1      
		pRcv->m_ptPackInfo[dwOffset]->m_byBit[pRcv->m_tMp4Header.m_byIndex] = 1;
		
		//是否是一帧的最后一包
		if(pRtpPack->m_byMark)
		{
			//得到帧的扩展数据
			pRcv->m_ptPackInfo[dwOffset]->m_byMediaType  = pRtpPack->m_byPayload;
			pRcv->m_ptPackInfo[dwOffset]->m_byMakerIndex = pRcv->m_tMp4Header.m_byIndex;
			
			//视频扩展数据
			pRcv->m_ptPackInfo[dwOffset]->m_bKeyFrame    = pRcv->m_tMp4Header.m_nMode;
			pRcv->m_ptPackInfo[dwOffset]->m_byFrameRate  = pRcv->m_tMp4Header.m_nRate;
			pRcv->m_ptPackInfo[dwOffset]->m_dwFrameId    = pRcv->m_tMp4Header.m_dwFrameId;
			pRcv->m_ptPackInfo[dwOffset]->m_wHeiht       = pRcv->m_tMp4Header.m_wHeight;
			pRcv->m_ptPackInfo[dwOffset]->m_wWidth       = pRcv->m_tMp4Header.m_wWidth;
		}
		
		pRcv->m_dwQueneNum++;
	}
}

void DealNetRcvRSQSndQuest(ifly_netrcv_t *pRcv,u32 dwTimeStamp)
{
	s32 nUpper = 0;
	
	u32 dwTempTS = 0;
	u32 dwIndex = 0;
	
	if(g_nRepeatSnd == 0 || NULL == pRcv || FALSE == pRcv->m_bRepeatSend)
	{
		return;
	}
	
	if(pRcv->m_pcRtp == NULL|| pRcv->m_pcRtcp == NULL)
	{
		return;        
	}
	
	nUpper = (s32)(pRcv->m_dwMaxTimeStamp - pRcv->m_dwMinTimeStamp) / VIDEO_TIME_SPAN;
	dwIndex = 0;
	
	if(nUpper > 0)
	{
		//搜索目标对应索引
		
		if(0 != pRcv->m_tRSParam.m_wFirstTimeSpan)
		{
			dwTempTS = dwTimeStamp - pRcv->m_tRSParam.m_wFirstTimeSpan;
			if( dwTempTS <= pRcv->m_dwMaxTimeStamp && dwTempTS >= pRcv->m_dwMinTimeStamp)
			{
				dwIndex = (dwTempTS - pRcv->m_dwMinTimeStamp)/VIDEO_TIME_SPAN;
				SendNetRcvRSQSndQuest(pRcv,dwIndex);
			}
		}
		
		if(0 != pRcv->m_tRSParam.m_wSecondTimeSpan)
		{
			dwTempTS = dwTimeStamp - pRcv->m_tRSParam.m_wSecondTimeSpan;
			if( dwTempTS <= pRcv->m_dwMaxTimeStamp && dwTempTS >= pRcv->m_dwMinTimeStamp)
			{
				dwIndex = (dwTempTS - pRcv->m_dwMinTimeStamp)/VIDEO_TIME_SPAN;
				SendNetRcvRSQSndQuest(pRcv,dwIndex);
			}
		}
		
		if(0 != pRcv->m_tRSParam.m_wThirdTimeSpan)
		{
			dwTempTS = dwTimeStamp - pRcv->m_tRSParam.m_wThirdTimeSpan;
			if( dwTempTS <= pRcv->m_dwMaxTimeStamp && dwTempTS >= pRcv->m_dwMinTimeStamp)
			{
				dwIndex = (dwTempTS - pRcv->m_dwMinTimeStamp)/VIDEO_TIME_SPAN;
				SendNetRcvRSQSndQuest(pRcv,dwIndex);
			}
		}
	}
}

void SendNetRcvRSQSndQuest(ifly_netrcv_t *pRcv,u32 dwIndex)
{
	TPackInfo* ptPackInfo = NULL;
	
	BOOL bSendRSQ = FALSE;
	
	if( dwIndex >= pRcv->m_dwMaxQueneNum )
	{
		return;
	}
	
	ptPackInfo = pRcv->m_ptPackInfo[dwIndex];
	
	//对于最大128kbyte的数据帧而言，小包数 <  96
	if(ptPackInfo->m_dwPackNum >= MAX_PACK_NUM)
	{
		return;
	}
	
	memset(&pRcv->m_tSndRSQ, 0, sizeof(pRcv->m_tSndRSQ));
	
	//重发请求类型：以SN+TIMESTAMP 或者只以TIMESTAMP请求
	if(0 == ptPackInfo->m_dwTimeStamp)
	{
		s32 nMax = 0;
		s32 j = 0;
		
		pRcv->m_tSndRSQ.m_byRSQType = TIMESTAMP_RSQ_TYPE;
		
		//填充重发帧的时间戳
		for(j=0; j<(s32)pRcv->m_dwMaxQueneNum; j++)
		{
			if(0 != pRcv->m_ptPackInfo[j]->m_dwTimeStamp)
			{
				nMax = j;
			}
		}
		if(nMax != (s32)pRcv->m_dwMaxQueneNum && nMax > (s32)dwIndex)
		{
			pRcv->m_tSndRSQ.m_dwTimeStamp = pRcv->m_ptPackInfo[nMax]->m_dwTimeStamp - 
								        (nMax-dwIndex) * VIDEO_TIME_SPAN;			
			
			//csp add 2008-11-20
			//order rs rtcp convert
			pRcv->m_tSndRSQ.m_dwTimeStamp = htonl(pRcv->m_tSndRSQ.m_dwTimeStamp);
			pRcv->m_tSndRSQ.m_dwRtpIP = pRcv->m_tLocalNetParam.m_tLocalNet.m_dwRTPAddr;
			pRcv->m_tSndRSQ.m_wRtpPort = htons(pRcv->m_tLocalNetParam.m_tLocalNet.m_wRTPPort);
			
			//bSendRSQ = TRUE;//csp modify 2008-11-26
			
			//if(bSendRSQ) printf("TIMESTAMP_RSQ_TYPE\n");
		}
	}
	else
	{
		s32 nOffset = 0;
		
		s32 nMuti = 0;
		s32 nResi = 0;
		
		u32 *pdwMaskBit = NULL;
		
		s32 nSize = 0;
		
		pRcv->m_tSndRSQ.m_byRSQType = SN_RSQ_TYPE;
		
		//填充重发帧的时间戳及起始包序列
		pRcv->m_tSndRSQ.m_byPackNum = (u8)ptPackInfo->m_dwPackNum;//csp add 2008-11-20
		pRcv->m_tSndRSQ.m_wStartSeqNum = ptPackInfo->m_wStartSeq;
		pRcv->m_tSndRSQ.m_dwTimeStamp = ptPackInfo->m_dwTimeStamp;
		
		//csp add 2008-11-20
		//order rs rtcp convert
		pRcv->m_tSndRSQ.m_dwTimeStamp = htonl(pRcv->m_tSndRSQ.m_dwTimeStamp);
		pRcv->m_tSndRSQ.m_wStartSeqNum = htons(pRcv->m_tSndRSQ.m_wStartSeqNum);
		pRcv->m_tSndRSQ.m_dwRtpIP = pRcv->m_tLocalNetParam.m_tLocalNet.m_dwRTPAddr;
		pRcv->m_tSndRSQ.m_wRtpPort = htons(pRcv->m_tLocalNetParam.m_tLocalNet.m_wRTPPort);
		
		///填充重发帧的各包的重发掩码位
		pdwMaskBit = (u32*)&pRcv->m_tSndRSQ.m_byMaskBit;			
		for(nOffset=0; nOffset<(s32)ptPackInfo->m_dwPackNum; nOffset++)
		{
			nMuti = nOffset/32;
			nResi = nOffset%32;
			//00010101 （1－需要重发, 0－不重发）
			if(ptPackInfo->m_byBit[nOffset+1] == 0)
			{
				bSendRSQ = TRUE;
				
				pdwMaskBit[nMuti] = SetBitField(pdwMaskBit[nMuti], 1, nResi, 1);
			}
		}
		
		//csp add 2008-11-20
		//order rs rtcp convert
		nSize = MAX_PACK_NUM / (8 * sizeof(u32));
		ConvertH2N( (u8*)pdwMaskBit, 0, nSize);
		
		//if(bSendRSQ) printf("SN_RSQ_TYPE\n");
		
		//csp add 2008-11-26
		if(TRUE == bSendRSQ)
		{
			if(ptPackInfo->m_byMakerIndex != 0 && ptPackInfo->m_bKeyFrame)
			{
				//printf("repeat request key frame,FrameId=%d\n",ptPackInfo->m_dwFrameId);
			}
			else
			{
				//printf("unknown frame type\n");
				bSendRSQ = FALSE;
			}
		}
	}
	
	if(TRUE == bSendRSQ)
	{
		if(2 == g_nShowDebugInfo)
		{
			//printf("[Snd:%d] :  Acquire wStartSeqNum=%d, Acquire dwTimeStamp=%d, m_dwQueneNum=%d\n", bSendRSQ, pRcv->m_tSndRSQ.m_wStartSeqNum, pRcv->m_tSndRSQ.m_dwTimeStamp, pRcv->m_dwQueneNum);
			printf("[Snd:%d] :  Acquire wStartSeqNum=%d, Acquire dwTimeStamp=0x%08x, m_dwQueneNum=%d\n", bSendRSQ, ntohs(pRcv->m_tSndRSQ.m_wStartSeqNum), ntohl(pRcv->m_tSndRSQ.m_dwTimeStamp), pRcv->m_dwQueneNum);
		}
		
		SendRtcpRSQ(pRcv->m_pcRtcp,&pRcv->m_tSndRSQ);
	}
}

//csp add 2008-11-28
void DealNetRcvFrameLoseEvent(ifly_netrcv_t *pRcv,u32 m_dwFrameId)
{
	SendNetRcvFrameLoseEvent(pRcv,m_dwFrameId);
}

//csp add 2008-11-28
void SendNetRcvFrameLoseEvent(ifly_netrcv_t *pRcv,u32 m_dwFrameId)
{
	BOOL bSendRSQ = TRUE;
	
	TRtcpSDESLose tSDESLose;
	
	memset(&tSDESLose,0,sizeof(tSDESLose));
	
	//order rs rtcp convert
	tSDESLose.m_dwLoseFrameId = htonl(m_dwFrameId);
	tSDESLose.m_dwRtpIP = pRcv->m_tLocalNetParam.m_tLocalNet.m_dwRTPAddr;
	tSDESLose.m_wRtpPort = htons(pRcv->m_tLocalNetParam.m_tLocalNet.m_wRTPPort);
	
	if(TRUE == bSendRSQ)
	{
		//if(2 == g_nShowDebugInfo)
		{
			printf("[Snd:%d] :  lose frame id=%d\n", bSendRSQ, m_dwFrameId);
		}
		
		SendRtcpFrameLoseEvent(pRcv->m_pcRtcp,&tSDESLose);
	}
}
