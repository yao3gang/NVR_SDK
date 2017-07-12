#include "mssend.h"

extern s32 g_nRepeatSnd;
extern s32 g_nDiscardSpan;
extern s32 g_nShowDebugInfo;

#define CREATE_CHECK(pSnd)					\
    if(pSnd == NULL || pSnd->m_pcRtp == NULL)				\
	{										\
		return ERROR_SND_NOCREATE;			\
	}										\
	
void InitNetSnd(ifly_netsnd_t *pSnd)
{
	if(pSnd)
	{
		pSnd->m_pcRtp			= NULL;
		pSnd->m_pcRtcp			= NULL;//csp add 2008-11-20
		
		//pSnd->m_pSelfFrameBuf = NULL;
		//pSnd->m_hSndSynSem    = NULL;
		
		pSnd->m_dwFrameId		= 0;
		pSnd->m_dwTimeStamp		= 0;
		pSnd->m_byMediaType		= 0;
		pSnd->m_dwMaxFrameSize	= 0;
		pSnd->m_nMaxSendNum		= 0;
		pSnd->m_byFrameRate     = 0;
		
		pSnd->m_bRepeatSend		= FALSE;
		pSnd->m_wBufTimeSpan	= 0;//csp add 2008-11-20
		
		memset(pSnd->m_byExBuf,0,sizeof(pSnd->m_byExBuf));
		//memset(&pSnd->m_tSelfFrmHdr,0,sizeof(&pSnd->m_tSelfFrmHdr));
		
		memset(&pSnd->m_tSndStatus, 0, sizeof(pSnd->m_tSndStatus));
		memset(&pSnd->m_tSndStatistics, 0, sizeof(pSnd->m_tSndStatistics));
		
		//memset(&pSnd->m_tOldRtp, 0, sizeof(pSnd->m_tOldRtp));//csp add 2008-11-20
		
		//csp add 2008-11-28
		pSnd->m_dwNewKeyFrameId = 0;
		pSnd->m_dwLoseFrameId	= 0;
	}
}

void FreeNetSndBuf(ifly_netsnd_t* pSnd)
{
	if(pSnd)
	{
		//csp add 2008-11-20
		if(pSnd->m_pcRtcp)
		{
			DestroyRtcp(pSnd->m_pcRtcp);
			pSnd->m_pcRtcp = NULL;
		}
		
		if(pSnd->m_pcRtp)
		{
			DestroyRtp(pSnd->m_pcRtp);
			pSnd->m_pcRtp = NULL;
		}
		
		//if(pSnd->m_pSelfFrameBuf) free(pSnd->m_pSelfFrameBuf);
		//if(pSnd->m_hSndSynSem) SemDelete(pSnd->m_hSndSynSem);
		
		pSnd->m_dwFrameId		= 0;
		pSnd->m_dwTimeStamp		= 0;
		pSnd->m_byMediaType		= 0;
		pSnd->m_dwMaxFrameSize	= 0;
		pSnd->m_nMaxSendNum		= 0;
		pSnd->m_byFrameRate		= 0;
		
		pSnd->m_bRepeatSend    = FALSE;
		pSnd->m_wBufTimeSpan   = 0;
		
		memset(&pSnd->m_tSndStatus, 0, sizeof(pSnd->m_tSndStatus));
		memset(&pSnd->m_tSndStatistics, 0, sizeof(pSnd->m_tSndStatistics));
		
		//memset(&pSnd->m_tOldRtp, 0, sizeof(pSnd->m_tOldRtp));
		
		//csp add 2008-11-28
		pSnd->m_dwNewKeyFrameId = 0;
		pSnd->m_dwLoseFrameId	= 0;
		
		free(pSnd);
	}
}

u16 DestroyNetSnd(ifly_netsnd_t* pSnd)
{
	if(pSnd)
	{
		//csp add 2008-11-20
		if(pSnd->m_pcRtcp)
		{
			DestroyRtcp(pSnd->m_pcRtcp);
			pSnd->m_pcRtcp = NULL;
		}
		
		if(pSnd->m_pcRtp)
		{
			DestroyRtp(pSnd->m_pcRtp);
			pSnd->m_pcRtp = NULL;
		}
		
		//if(pSnd->m_pSelfFrameBuf) free(pSnd->m_pSelfFrameBuf);
		//if(pSnd->m_hSndSynSem) SemDelete(pSnd->m_hSndSynSem);
		
		free(pSnd);
	}
	
	return MEDIASTREAM_NO_ERROR;
}

ifly_netsnd_t* CreateNetSnd(u32 dwMaxFrameSize, u32 dwNetBand, u8 byFrameRate, u8 byMediaType, u32 dwSSRC)
{
	ifly_netsnd_t *pSnd = NULL;
	INT32 nRealPackLen = 0;
	INT32 nAveFrame = 0;
	
	BOOL32 bAllocLPBuf = FALSE;//csp add 2008-11-20
	
	if( (dwMaxFrameSize == 0) || (dwMaxFrameSize > MAX_FRAME_SIZE) || (dwNetBand == 0) || (byFrameRate > 30) ||
		(byMediaType != MEDIA_TYPE_MP3 && 
		byMediaType != MEDIA_TYPE_MP4 &&
		byMediaType != MEDIA_TYPE_H264 &&
		byMediaType != MEDIA_TYPE_MJPEG &&
		byMediaType != MEDIA_TYPE_PCMU &&
		byMediaType != MEDIA_TYPE_PCMA &&
		byMediaType != MEDIA_TYPE_G7231 &&
		byMediaType != MEDIA_TYPE_G722 &&
		byMediaType != MEDIA_TYPE_G728 &&
		byMediaType != MEDIA_TYPE_G729 &&
		byMediaType != MEDIA_TYPE_RAWAUDIO))
    {
        return NULL;
    }
	
	if(0 == byFrameRate)
	{
		byFrameRate = 25;
	}
	
	pSnd = (ifly_netsnd_t *)malloc(sizeof(ifly_netsnd_t));
	if(pSnd == NULL)
	{
		return NULL;
	}
	InitNetSnd(pSnd);
	
	//SemBCreate(&pSnd->m_hSndSynSem);
	
	pSnd->m_dwFrameId++;
	pSnd->m_byFrameRate = byFrameRate;
	
	pSnd->m_byMediaType = byMediaType;
	pSnd->m_dwMaxFrameSize = dwMaxFrameSize;
	
	if(0 == dwSSRC)
	{
		pSnd->m_dwTimeStamp = GetExclusiveSSRC();
	}
	else
	{
		pSnd->m_dwTimeStamp = dwSSRC;
	}
	
	//计算一次发送的最大发送包数
	nRealPackLen = MAX_PACK_SIZE - EX_HEADER_SIZE - MAX_PACK_EX_LEN;
	nAveFrame    = (INT32)(dwNetBand/(byFrameRate*8));//帧平均长度
	pSnd->m_nMaxSendNum = (nAveFrame+nRealPackLen-1)/nRealPackLen;
	
	//printf("before--pSnd->m_nMaxSendNum=%d\n",pSnd->m_nMaxSendNum);
	//最大不应超过环形缓冲存储的包总数
	if(pSnd->m_nMaxSendNum > LOOP_BUF_UINT_NUM)
	{
		pSnd->m_nMaxSendNum = LOOP_BUF_UINT_NUM;
	}
	//printf("after--pSnd->m_nMaxSendNum=%d\n",pSnd->m_nMaxSendNum);
	
	//对于mp4、h.264我们 需要放入环形缓冲后，再进行发送，以便进行码流控制
	if(byMediaType == MEDIA_TYPE_MP4 || 
		byMediaType == MEDIA_TYPE_H264 || 
		byMediaType == MEDIA_TYPE_MJPEG)
	{
		bAllocLPBuf = TRUE;
	}
	pSnd->m_pcRtp = CreateRtp(pSnd->m_dwTimeStamp, bAllocLPBuf);//csp modify 2008-11-20
	pSnd->m_pcRtcp = CreateRtcp(pSnd->m_dwTimeStamp);//csp add 2008-11-20
	
	//设置和RTP对应的RTCP。
	SetRtpRtcp(pSnd->m_pcRtp, pSnd->m_pcRtcp);
	
	//设置和RTCP对应的RTP。
	SetRtcpRtp(pSnd->m_pcRtcp, pSnd->m_pcRtp);
	
	//csp add 2008-11-28
	SetRtcpFrameLoseCallBack(pSnd->m_pcRtcp, DealNetSndFrameLoseEvent, (u32)pSnd);
	
	//pSnd->m_pSelfFrameBuf = (u8 *)malloc(dwMaxFrameSize);
	//memset(&pSnd->m_tSelfFrmHdr, 0, sizeof(pSnd->m_tSelfFrmHdr));
	//pSnd->m_tSelfFrmHdr.m_pData = pSnd->m_pSelfFrameBuf;
	
	//pSnd->m_tSelfFrmHdr.m_byMediaType		= pSnd->m_byMediaType;
	//pSnd->m_tSelfFrmHdr.m_byFrameRate		= pSnd->m_byFrameRate;
	
	pSnd->m_tSndStatus.m_byMediaType		= pSnd->m_byMediaType;
	pSnd->m_tSndStatus.m_dwFrameID			= pSnd->m_dwFrameId;
	pSnd->m_tSndStatus.m_dwMaxFrameSize		= pSnd->m_dwMaxFrameSize;
	pSnd->m_tSndStatus.m_dwNetBand			= dwNetBand;
	pSnd->m_tSndStatus.m_byFrameRate		= pSnd->m_byFrameRate;
	
	return pSnd;
}

//设置网络发送参数(进行底层套结字的创建，绑定端口,以及发送目标地址的设定等动作)
u16 SetNetSndNetParam(ifly_netsnd_t* pSnd,TNetSndParam tNetSndParam)
{
	u16 wRet;
	TRemoteAddr tRemoteAddr;
	s32 i,j;
	
	CREATE_CHECK(pSnd)	//是否创建
	
	if(tNetSndParam.m_byNum > MAX_NETSND_DEST_NUM)
	{
		return ERROR_SND_PARAM;
	}
	
	wRet = MEDIASTREAM_NO_ERROR;
	
	//设置RTP当地地址
	wRet= SetRtpLocalAddr(pSnd->m_pcRtp,tNetSndParam.m_tLocalNet.m_dwRTPAddr,tNetSndParam.m_tLocalNet.m_wRTPPort,FALSE);
	if(IFLYFAILED(wRet))
	{
		return wRet;
	}
	
	//设置RTP发送地址对
	memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
	tRemoteAddr.m_byNum = tNetSndParam.m_byNum;
	for(i=0; i<tRemoteAddr.m_byNum; i++)
	{
		tRemoteAddr.m_tAddr[i].m_dwIP  = 
			tNetSndParam.m_tRemoteNet[i].m_dwRTPAddr;
		tRemoteAddr.m_tAddr[i].m_wPort =
			tNetSndParam.m_tRemoteNet[i].m_wRTPPort;
	}
	
	wRet= SetRtpRemoteAddr(pSnd->m_pcRtp,&tRemoteAddr);
	if(IFLYFAILED(wRet))
	{
		return wRet;
	}
	
	//csp add 2008-11-20
	//设置RTCP当地地址
	wRet= SetRtcpLocalAddr(pSnd->m_pcRtcp, tNetSndParam.m_tLocalNet.m_dwRTCPAddr,
		tNetSndParam.m_tLocalNet.m_wRTCPPort);
	if(IFLYFAILED(wRet))
	{
		return wRet;
	}
	
	//设置RTCP发送地址对
	memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
    	tRemoteAddr.m_byNum = tNetSndParam.m_byNum;
	for(j=0; j<tRemoteAddr.m_byNum; j++)
	{
		tRemoteAddr.m_tAddr[j].m_dwIP = tNetSndParam.m_tRemoteNet[j].m_dwRTCPAddr;
		tRemoteAddr.m_tAddr[j].m_wPort = tNetSndParam.m_tRemoteNet[j].m_wRTCPPort;
	}
	wRet = SetRtcpRemoteAddr(pSnd->m_pcRtcp, &tRemoteAddr);
	if(IFLYFAILED(wRet))
	{
		return wRet;
	}
	
	pSnd->m_tSndStatus.m_tSendAddr = tNetSndParam;
	return MEDIASTREAM_NO_ERROR;
}

//移除网络发送本地地址参数(进行底层套结字的删除，释放端口等动作)
u16 RemoveNetSndLocalNetParam(ifly_netsnd_t* pSnd)
{
	CREATE_CHECK(pSnd)	//是否创建
		
	RemoveRtpLocalAddr(pSnd->m_pcRtp);
	
	RemoveRtcpLocalAddr(pSnd->m_pcRtcp);//csp add 2008-11-20
	
	return MEDIASTREAM_NO_ERROR;
}

//重置帧ID
u16 ResetNetSndFrameId(ifly_netsnd_t* pSnd)
{
	CREATE_CHECK(pSnd) //是否创建
	
	pSnd->m_dwFrameId++;
	pSnd->m_tSndStatus.m_dwFrameID = pSnd->m_dwFrameId;
	
	return MEDIASTREAM_NO_ERROR;
}

//重置同步源SSRC
u16 ResetNetSndSSRC(ifly_netsnd_t* pSnd,u32 dwSSRC)
{
	CREATE_CHECK(pSnd) //是否创建
		
	if(0 == dwSSRC)
	{
		dwSSRC = GetExclusiveSSRC();
	}
	
	ResetRtpSSRC(pSnd->m_pcRtp,dwSSRC);
	
	ResetRtcpSSRC(pSnd->m_pcRtcp,dwSSRC);//csp add 2008-11-20
	
	return MEDIASTREAM_NO_ERROR;
}

//重置发送端对于mpeg4或者H.264采用的重传处理的开关,关闭后，将不对已经发送的数据包进行缓存
u16 ResetNetSndRSFlag(ifly_netsnd_t* pSnd, u16 wBufTimeSpan, BOOL32 bRepeatSnd)
{
	u16 wRet;
	
	u16 wScale = 2; //放大比例
	u16 wRLBUnitNum = 0;
	
	CREATE_CHECK(pSnd) //是否创建
	
	//对于mp4、h.264我们采用重发方式
	
	wRet = MEDIASTREAM_NO_ERROR;
	
	if(pSnd->m_byMediaType == MEDIA_TYPE_MP4 || pSnd->m_byMediaType == MEDIA_TYPE_H264 || pSnd->m_byMediaType == MEDIA_TYPE_MJPEG)
	{
		pSnd->m_wBufTimeSpan = VIDEO_TIME_SPAN * ((wBufTimeSpan + VIDEO_TIME_SPAN - 1)/(VIDEO_TIME_SPAN));
		pSnd->m_bRepeatSend = bRepeatSnd;
	}
	else
	{
		pSnd->m_wBufTimeSpan = 0;
		pSnd->m_bRepeatSend = FALSE;
	}
	
	wScale = 2; //放大比例
	wRLBUnitNum = (pSnd->m_wBufTimeSpan * pSnd->m_nMaxSendNum * pSnd->m_byFrameRate * wScale) / 1000;
	if(wRLBUnitNum < MIN_RS_UNIT_NUM)
	{
		wRLBUnitNum = MIN_RS_UNIT_NUM;
	}
	if(wRLBUnitNum > MAX_RS_UNIT_NUM)
	{
		wRLBUnitNum = MAX_RS_UNIT_NUM;
	}
	
	if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
	{
		printf("[NetSnd::ResetRSFlag] bRepeatSnd:%d, wBufTimeSpan:%d, real bRepeatSnd:%d, real wBufTimeSpan:%d, wRLBUnitNum:%d\n", 
			bRepeatSnd, pSnd->m_wBufTimeSpan, pSnd->m_bRepeatSend, pSnd->m_wBufTimeSpan, wRLBUnitNum);
	}
	
	wRet = ResetRtpRSFlag(pSnd->m_pcRtp, wRLBUnitNum, pSnd->m_bRepeatSend);//csp modify 2008-11-20
	
	return wRet;
}

//设置发送选项
u16 SetNetSndInfo(ifly_netsnd_t* pSnd, u32 dwNetBand, u8 byFrameRate)
{
	s32  nRealPackLen = MAX_PACK_SIZE - EX_HEADER_SIZE - MAX_PACK_EX_LEN;
	s32  nAveFrame    = (s32)(dwNetBand / (byFrameRate * 8));
	
	pSnd->m_nMaxSendNum = (nAveFrame +  nRealPackLen - 1) / nRealPackLen;
    //最大不应超过环形缓冲存储的包总数
	if(pSnd->m_nMaxSendNum > LOOP_BUF_UINT_NUM)
	{
		pSnd->m_nMaxSendNum = LOOP_BUF_UINT_NUM;
	}
	
	printf("SetNetSndInfo:m_nMaxSendNum=%d\n",pSnd->m_nMaxSendNum);
	
	//csp add 2008-11-20
	//重置重传发送缓冲时间长度
	if(TRUE == pSnd->m_bRepeatSend)
	{
		ResetNetSndRSFlag(pSnd, pSnd->m_wBufTimeSpan, TRUE);
	}
	
	pSnd->m_byFrameRate = byFrameRate;
	pSnd->m_tSndStatus.m_dwNetBand = dwNetBand;
	
    return MEDIASTREAM_NO_ERROR;
}

u16 SendExPack(ifly_netsnd_t* pSnd, PFRAMEHDR pFrmHdr, BOOL bAudio, int avgInterTime)
{
	u16 wRet = MEDIASTREAM_NO_ERROR;
    s32 nRealPackLen = MAX_EXTEND_PACK_SIZE;
	s32 nPackNum = (pFrmHdr->m_dwDataSize  + nRealPackLen - 1) / nRealPackLen;
	TRtpPack tRtpPack;
	s32 nPackIndex;
	s32 nLastPackLen;
	s32 nBytesThold = (pFrmHdr->m_tVideoParam.m_wBitRate)*128/50; // bytes sent every 20ms
	
	//csp add 2008-11-28
	s32 nRealSendNum = 0;
	s32 nReaminSendNum = pSnd->m_nMaxSendNum;
	
	//printf("SendExPack:TimeStamp=%d\n",pFrmHdr->m_dwTimeStamp);
	
	//始终会有一包发送
	if(nPackNum == 0) nPackNum = 1;
	
	if(nPackNum > MAX_EXTEND_PACK_NUM)//包的个数不能超过单字节所能表达的值
	{
		pSnd->m_tSndStatistics.m_dwFrameLoseNum++;
        return ERROR_SND_FRAME;  
	}
    
	//传给CKdvRtp类的RTP数据包信息  
	memset(&tRtpPack, 0, sizeof(TRtpPack));
	memset(pSnd->m_byExBuf, 0, MAX_PACK_EX_LEN);
	tRtpPack.m_byMark		= 0;
	tRtpPack.m_byPayload	= pFrmHdr->m_byMediaType;
	tRtpPack.m_dwTimeStamp	= pSnd->m_dwTimeStamp;
	tRtpPack.m_byExtence	= 1;//扩展包
    tRtpPack.m_nRealSize	= nRealPackLen;
	tRtpPack.m_nExSize		= 1;//单位为sizeof(u32)长度。
	tRtpPack.m_nPreBufSize	= pFrmHdr->m_dwPreBufSize;
	
	//csp add 2008-11-28
	if(!bAudio)
	{
		SendRtpPack(pSnd->m_pcRtp,pSnd->m_nMaxSendNum,&nRealSendNum);
		nReaminSendNum = pSnd->m_nMaxSendNum - nRealSendNum;
		//printf("max:%d,real:%d,remain:%d\n",pSnd->m_nMaxSendNum,nRealSendNum,nReaminSendNum);
	}
	
	//csp add 2008-11-28
	if(!bAudio)
	{
		if(pFrmHdr->m_tVideoParam.m_bKeyFrame)
		{
			pSnd->m_dwNewKeyFrameId = pSnd->m_dwFrameId;
			if(pSnd->m_dwFrameId < pSnd->m_dwLoseFrameId)
			{
				pSnd->m_dwLoseFrameId = 0;
			}
		}
		else
		{
			if(pSnd->m_dwLoseFrameId != 0 && pSnd->m_dwLoseFrameId >= pSnd->m_dwNewKeyFrameId)
			{
				//printf("lose frame,not send,frame id:(lose:%d,cur:%d,newkey:%d)\n",pSnd->m_dwLoseFrameId,pSnd->m_dwFrameId,pSnd->m_dwNewKeyFrameId);
				return MEDIASTREAM_NO_ERROR;
			}
		}
	}
	
	//循环处理n-1包
	for(nPackIndex=1; nPackIndex<nPackNum; nPackIndex++)
	{
		//有效数据
		tRtpPack.m_pRealData		= pFrmHdr->m_pData + 
										(nPackIndex-1)*nRealPackLen;
        //扩展数据
		//总包数
		pSnd->m_byExBuf[EX_TOTALNUM_POS]  = (u8)nPackNum;
		//包序号
		pSnd->m_byExBuf[EX_INDEX_POS]     = (u8)nPackIndex;
		tRtpPack.m_pExData = pSnd->m_byExBuf; 
		//写入一包，不一定直接发送，取决于第二个参数
		//printf("before WriteRtpPack 1\n");
		wRet = WriteRtpPack(pSnd->m_pcRtp, &tRtpPack, bAudio);
		if(IFLYFAILED(wRet))
		{
			//缓冲满
			pSnd->m_tSndStatistics.m_dwFrameLoseNum++;
			SendRtpPack(pSnd->m_pcRtp,nReaminSendNum,&nRealSendNum);//csp modify 2008-11-28
			return wRet;
		}
		//printf("after WriteRtpPack 2\n");
		//统计
		pSnd->m_tSndStatistics.m_dwPackSendNum++;
		pSnd->m_tSndStatistics.m_tempBytes += nRealPackLen;
		
		if(pSnd->m_tSndStatistics.m_tempBytes >= (u32)nBytesThold)
		{
			if(avgInterTime > 0)
			{
				#ifndef WIN32
				usleep(min(pSnd->m_tSndStatistics.m_tempBytes*20/nBytesThold-20+1, 1000)*1000);//06-11
				//printf("sleep= %d\n",min(pSnd->m_tSndStatistics.m_tempBytes*20/nBytesThold-20+1, 40)*1000);
				#endif
			}
			pSnd->m_tSndStatistics.m_tempBytes = 0;
		}
	}
	
	/*最后一包的处理，不同于前n-1包，在最后一包会放入帧信息，如帧ID、
	  是否关键帧、视频帧宽高。 
	*/
	nLastPackLen = pFrmHdr->m_dwDataSize - nRealPackLen * (nPackNum - 1);
	
	tRtpPack.m_byMark			= 1;
	tRtpPack.m_nRealSize		= nLastPackLen;
	tRtpPack.m_pRealData		= pFrmHdr->m_pData + 
								  (nPackNum - 1) * nRealPackLen;
	//音频帧
	if(bAudio)
	{
		tRtpPack.m_nExSize = 1;
		//总包数
		pSnd->m_byExBuf[EX_TOTALNUM_POS]    = (u8)nPackNum;
		//包序号
		pSnd->m_byExBuf[EX_INDEX_POS]		= (u8)nPackIndex;
		//音频模式
		pSnd->m_byExBuf[EX_FRAMEMODE_POS]	= (u8)pFrmHdr->m_byAudioMode;
		tRtpPack.m_pExData = pSnd->m_byExBuf; 
        //音频帧直接发送
		wRet = WriteRtpPack(pSnd->m_pcRtp,&tRtpPack,TRUE);
        //统计
		pSnd->m_tSndStatistics.m_dwPackSendNum++;
		
		if(avgInterTime > 0)
		{
			#ifndef WIN32
			usleep(avgInterTime*1000);//06-11
			#endif
		}
		
		return wRet;
	}
	else
	{
		//视频帧处理
		tRtpPack.m_nExSize			= MAX_PACK_EX_LEN / sizeof(u32);//
		//总包数
		pSnd->m_byExBuf[EX_TOTALNUM_POS]    = (u8)nPackNum;
		//包序号
		pSnd->m_byExBuf[EX_INDEX_POS]		= (u8)nPackIndex;
		//帧率
		pSnd->m_byExBuf[EX_FRAMERATE_POS]	= (u8)pSnd->m_byFrameRate;
		//是否关键帧
		pSnd->m_byExBuf[EX_FRAMEMODE_POS]   = (u8)pFrmHdr->m_tVideoParam.m_bKeyFrame;
		//帧ID
		*((u32 *)(&(pSnd->m_byExBuf[EX_FRAMEID_POS])))    = htonl(pSnd->m_dwFrameId);
		//视频帧宽
		*((u16 *)(&(pSnd->m_byExBuf[EX_WIDTH_POS])))       = 
			                           htons(pFrmHdr->m_tVideoParam.m_wVideoWidth);
		//视频帧高
		*((u16 *)(&(pSnd->m_byExBuf[EX_HEIGHT_POS])))      = 
			                          htons(pFrmHdr->m_tVideoParam.m_wVideoHeight);
		
		tRtpPack.m_pExData = pSnd->m_byExBuf;
		
		wRet = WriteRtpPack(pSnd->m_pcRtp,&tRtpPack,FALSE);
		if(IFLYFAILED(wRet))
		{
			//缓冲满
			pSnd->m_tSndStatistics.m_dwFrameLoseNum++;
			SendRtpPack(pSnd->m_pcRtp,nReaminSendNum,&nRealSendNum);//csp modify 2008-11-28
			return wRet;
		}
		//统计
		pSnd->m_tSndStatistics.m_dwPackSendNum++;
		
		//真实发送数据包
		wRet = SendRtpPack(pSnd->m_pcRtp,nReaminSendNum,&nRealSendNum);//批量发送//csp modify 2008-11-28
		
		pSnd->m_tSndStatistics.m_tempBytes += nLastPackLen;
		
		if(pSnd->m_tSndStatistics.m_tempBytes >= (u32)nBytesThold)
		{
			if(avgInterTime > 0)
			{
				#ifndef WIN32
				usleep(min(pSnd->m_tSndStatistics.m_tempBytes*20/nBytesThold-20+1, 1000)*1000);//06-11
				//printf("sleep= %d\n",min(pSnd->m_tSndStatistics.m_tempBytes*20/nBytesThold-20+1, 40)*1000);
				#endif
			}
			pSnd->m_tSndStatistics.m_tempBytes = 0;
		}
		
		return wRet;
	}
	
	return MEDIASTREAM_NO_ERROR;
}

u16 SendSdPack(ifly_netsnd_t* pSnd,PFRAMEHDR pFrmHdr)
{
	u16 wRet = MEDIASTREAM_NO_ERROR;
    TRtpPack tRtpPack;
	
	//printf("SendSdPack:TimeStamp=%d\n",pFrmHdr->m_dwTimeStamp);
	
	//一帧直接发送，不做切分
	if(pFrmHdr->m_dwDataSize > MAX_PACK_SIZE)
	{		
		pSnd->m_tSndStatistics.m_dwFrameLoseNum++;
		return ERROR_PACK_TOO_LEN;
	}
	
	//传给底层的RTP结构
	memset(&tRtpPack, 0, sizeof(TRtpPack));
	tRtpPack.m_byMark		= 0;
	tRtpPack.m_byExtence	= 0;//非扩展包
	tRtpPack.m_byPayload	= pFrmHdr->m_byMediaType;
	tRtpPack.m_dwTimeStamp  = pSnd->m_dwTimeStamp;//时间戳
	tRtpPack.m_nExSize      = 0;
	tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;	
	tRtpPack.m_byMark		= 0;
	//老的G.711标准：数据块前不加标识位。
	tRtpPack.m_nRealSize    = pFrmHdr->m_dwDataSize;// + G711_REVERSE_BIT;
	tRtpPack.m_pRealData    = pFrmHdr->m_pData;// - G711_REVERSE_BIT;
	
	//直接发送
	wRet = WriteRtpPack(pSnd->m_pcRtp,&tRtpPack,TRUE);
	if(IFLYFAILED(wRet))
	{
		pSnd->m_tSndStatistics.m_dwFrameLoseNum++;
		return wRet;
	}
	pSnd->m_tSndStatistics.m_dwPackSendNum++;
	
	//标准RTP包,直接发送不经过缓冲。
	return MEDIASTREAM_NO_ERROR;
}

//发送数据包
u16 SendFrame(ifly_netsnd_t* pSnd,PFRAMEHDR pFrmHdr,int avgInterTime)
{
	u16 wRet;
	
	CREATE_CHECK(pSnd) //是否创建
	
	//参数有效性判断，pFrmHdr->m_dwDataSize为零，表示发送空包 
	if( (pFrmHdr == NULL)|| 
		(pFrmHdr->m_dwDataSize > pSnd->m_dwMaxFrameSize)|| 
		(pFrmHdr->m_byMediaType != pSnd->m_byMediaType)|| 
		pFrmHdr->m_pData == NULL)
	{
		if(pFrmHdr == NULL)
		{
			printf("hehe1\n");
		}
		if(pFrmHdr->m_dwDataSize > pSnd->m_dwMaxFrameSize)
		{
			printf("hehe2,(%d,%d)\n",pFrmHdr->m_dwDataSize,pSnd->m_dwMaxFrameSize);
		}
		if(pFrmHdr->m_byMediaType != pSnd->m_byMediaType)
		{
			printf("hehe3,(%d,%d)\n",pFrmHdr->m_byMediaType,pSnd->m_byMediaType);
		}
		if(pFrmHdr->m_pData == NULL)
		{
			printf("hehe4\n");
		}
		return ERROR_SND_PARAM;
	}
	
	wRet = MEDIASTREAM_NO_ERROR;
	
	//帧ID累加
	pSnd->m_dwFrameId++;
	pSnd->m_tSndStatus.m_dwFrameID = pSnd->m_dwFrameId;
	
	pSnd->m_tSndStatistics.m_dwFrameNum++;
	
	/*根据不同的媒体类型作不同的处理，目前只有两种发送方式:
	1.标准包的发送，如h261、G.711、h263、G.723
	2.扩展包的发送, 如h264、Mpeg4、mp3
	*/
	switch(pFrmHdr->m_byMediaType)
	{
	case MEDIA_TYPE_MP4:
		{
			#ifdef RS_ENABLE
			pSnd->m_dwTimeStamp += VIDEO_TIME_SPAN;
			#else
			if(pFrmHdr->m_dwTimeStamp)
			{
				pSnd->m_dwTimeStamp = pFrmHdr->m_dwTimeStamp;
			}
			else
			{
				pSnd->m_dwTimeStamp = TickGet();
			}
			#endif
			
			wRet = SendExPack(pSnd,pFrmHdr,FALSE,avgInterTime);
			
			break;
		}
	case MEDIA_TYPE_H264:
		{
			#ifdef RS_ENABLE
			pSnd->m_dwTimeStamp += VIDEO_TIME_SPAN;
			#else
			if(pFrmHdr->m_dwTimeStamp)
			{
				pSnd->m_dwTimeStamp = pFrmHdr->m_dwTimeStamp;
			}
			else
			{
				pSnd->m_dwTimeStamp = TickGet();
			}
			#endif
			
			wRet = SendExPack(pSnd,pFrmHdr,FALSE,avgInterTime);
			
			break;
		}
	case MEDIA_TYPE_MJPEG:
		{
			#ifdef RS_ENABLE
			pSnd->m_dwTimeStamp += VIDEO_TIME_SPAN;
			#else
			if(pFrmHdr->m_dwTimeStamp)
			{
				pSnd->m_dwTimeStamp = pFrmHdr->m_dwTimeStamp;
			}
			else
			{
				pSnd->m_dwTimeStamp = TickGet();
			}
			#endif
			
			wRet = SendExPack(pSnd,pFrmHdr,FALSE,avgInterTime);
			
			break;
		}
	case MEDIA_TYPE_MP3:
		{
			#ifdef RS_ENABLE
			pSnd->m_dwTimeStamp += VIDEO_TIME_SPAN;
			#else
			if(pFrmHdr->m_dwTimeStamp)
			{
				pSnd->m_dwTimeStamp = pFrmHdr->m_dwTimeStamp;
			}
			else
			{
				pSnd->m_dwTimeStamp = TickGet();
			}
			#endif
			
			wRet = SendExPack(pSnd,pFrmHdr,TRUE,avgInterTime);
			
			break;
		}
	case MEDIA_TYPE_PCMU:
	case MEDIA_TYPE_PCMA:
	case MEDIA_TYPE_G7231:
	case MEDIA_TYPE_G722:
	case MEDIA_TYPE_G728:
	case MEDIA_TYPE_G729:
	case MEDIA_TYPE_RAWAUDIO:
		{
			//pSnd->m_dwTimeStamp += pFrmHdr->m_dwDataSize;
			if(pFrmHdr->m_dwTimeStamp)
			{
				pSnd->m_dwTimeStamp = pFrmHdr->m_dwTimeStamp;
			}
			else
			{
				pSnd->m_dwTimeStamp = TickGet();
			}
			
			wRet = SendSdPack(pSnd,pFrmHdr);
			break;
		}
	default:
		wRet = ERROR_SND_PARAM;
		break;
	}
	
	return wRet;
}

//得到状态
u16 GetNetSndStatus(ifly_netsnd_t* pSnd,TSndStatus *pSndStatus)
{
	*pSndStatus = pSnd->m_tSndStatus;
	return MEDIASTREAM_NO_ERROR;
}

//得到统计
u16 GetNetSndStatistics(ifly_netsnd_t* pSnd,TSndStatistics *pSndStatistics)
{
	*pSndStatistics = pSnd->m_tSndStatistics;
    return MEDIASTREAM_NO_ERROR;
}

//得到发送端高级设置参数(重传等)
u16 GetNetSndAdvancedInfo(ifly_netsnd_t* pSnd,TAdvancedSndInfo *pAdvancedSndInfo)
{
	pAdvancedSndInfo->m_bRepeatSend  = pSnd->m_bRepeatSend;
	pAdvancedSndInfo->m_nMaxSendNum  = pSnd->m_nMaxSendNum;
	pAdvancedSndInfo->m_wBufTimeSpan = 0;
	
    return MEDIASTREAM_NO_ERROR;
}

u16 DealNetSndRtcpTimer(ifly_netsnd_t* pSnd)
{
	CREATE_CHECK(pSnd) //是否创建
	
	return DealRtcpTimer(pSnd->m_pcRtcp);
}

//csp add 2008-11-28
u16 DealNetSndFrameLoseEvent(TRtcpSDESLose *ptLose, u32 dwContext)
{
	ifly_netsnd_t *pSnd = (ifly_netsnd_t *)dwContext;
	
	CREATE_CHECK(pSnd) //是否创建
	
	pSnd->m_dwLoseFrameId = ntohl(ptLose->m_dwLoseFrameId);
	//printf("NetSnd:lose frame id:%d,cur frame id:%d,new key frame id:%d\n",ntohl(ptLose->m_dwLoseFrameId),pSnd->m_dwFrameId,pSnd->m_dwNewKeyFrameId);
	
	return MEDIASTREAM_NO_ERROR;
}
