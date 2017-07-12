#include "msrtp.h"

void InitRtcp(ms_rtcp_t *pRtcp)
{
	pRtcp->m_pRtp				= NULL;
	
	pRtcp->m_pSocket			= NULL;
	pRtcp->m_pdwBuf				= NULL;
	pRtcp->m_pCustomBuf			= NULL;
//	pRtcp->m_hTimeId			= 0;
	pRtcp->m_nStartTime			= 0;
    pRtcp->m_nStartMilliTime	= 0;
	pRtcp->m_hSem				= NULL;
	
#ifdef WIN32
//	pRtcp->m_hEnd = NULL;
//	pRtcp->m_hThread = NULL;
//	pRtcp->m_dwThreadID = 0;
#endif
	
	memset(&pRtcp->m_tLocalAddr, 0, sizeof(pRtcp->m_tLocalAddr));
	memset(&pRtcp->m_tRemoteAddr, 0, sizeof(pRtcp->m_tRemoteAddr));
    memset(&pRtcp->m_tMyInfo, 0, sizeof(pRtcp->m_tMyInfo));
	memset(&pRtcp->m_tRtcpInfoList, 0, sizeof(pRtcp->m_tRtcpInfoList));
	
	//csp add 2008-11-28
	pRtcp->m_pFrameLoseCallBack = NULL;
	pRtcp->m_dwFrameLoseContext = 0;
}

u16 DestroyRtcp(ms_rtcp_t *pRtcp)
{
	if(pRtcp)
	{
		FreeRtcpBuf(pRtcp);
		free(pRtcp);
	}
	return MEDIASTREAM_NO_ERROR;
}

void FreeRtcpBuf(ms_rtcp_t *pRtcp)
{
	if(pRtcp == NULL)
	{
		return;
	}
	
	if(pRtcp->m_pSocket)
	{
		CloseSocket(pRtcp->m_pSocket);
		pRtcp->m_pSocket = NULL;
	}
	if(pRtcp->m_pdwBuf)
	{
		free(pRtcp->m_pdwBuf);
		pRtcp->m_pdwBuf = NULL;
	}
	if(pRtcp->m_pCustomBuf)
	{
		free(pRtcp->m_pCustomBuf);
		pRtcp->m_pCustomBuf = NULL;
	}
	
/*
#ifdef WIN32
	if(pRtcp->m_hThread != NULL)
	{	
		u32 dwExitCode = 0;
		u32 dwTimeout = 50;
		if(pRtcp->m_hEnd != NULL)
		{
			SetEvent(pRtcp->m_hEnd);
		}
		while(dwTimeout)
		{
			GetExitCodeThread(pRtcp->m_hThread, &dwExitCode);
			
			if (dwExitCode != STILL_ACTIVE)
			{
				break;
			}
			else
			{
				Sleep(100);
			}
			
			--dwTimeout;
		}
		//5000ms后仍未关闭则强制杀掉该线程
		if(dwTimeout == 0)
		{
			TerminateThread(pRtcp->m_hThread,-1);
		}
		CloseHandle(pRtcp->m_hThread);
		pRtcp->m_dwThreadID = 0;
		pRtcp->m_hThread = NULL;
		CloseHandle(pRtcp->m_hEnd);
		pRtcp->m_hEnd = NULL;
	}
	
	//timeKillEvent(m_hTimeId);
	
#else
	if(pRtcp->m_hTimeId != 0)
	{
		timer_cancel(pRtcp->m_hTimeId);
		timer_delete(pRtcp->m_hTimeId);
		pRtcp->m_hTimeId = 0; 
	}
#endif
*/
	
	pRtcp->m_nStartTime = 0;
	pRtcp->m_nStartMilliTime = 0;
	
	pRtcp->m_pRtp = NULL;
	
	memset(&pRtcp->m_tLocalAddr, 0, sizeof(pRtcp->m_tLocalAddr));
	memset(&pRtcp->m_tRemoteAddr, 0, sizeof(pRtcp->m_tRemoteAddr));
	memset(&pRtcp->m_tMyInfo, 0, sizeof(pRtcp->m_tMyInfo));
	
	//csp add 2008-11-28
	pRtcp->m_pFrameLoseCallBack = NULL;
	pRtcp->m_dwFrameLoseContext = 0;
	
	if(pRtcp->m_hSem != NULL)
	{
		SemDelete(pRtcp->m_hSem);
		pRtcp->m_hSem = NULL;
	}
}

/*=============================================================================
函数名		:  GetNNTPTime
功能		： Get  time from 1900

  算法实现	：（可选项）
  引用全局变量：无
  输入参数说明: 无
  
	返回值说明： 64 bits time
=============================================================================*/
TUint64 GetNNTPTime(ms_rtcp_t *pRtcp)
{
    TUint64 nntpTime;
    
    if (!pRtcp->m_nStartTime) 
    {
        pRtcp->m_nStartTime      = time(NULL) + FROM1900TILL1970;
        pRtcp->m_nStartMilliTime = GetSystemTick();        
    }
	
    nntpTime.msdw = pRtcp->m_nStartTime;
    nntpTime.lsdw = GetSystemTick() - pRtcp->m_nStartMilliTime;
    nntpTime.msdw += nntpTime.lsdw/1000;
    nntpTime.lsdw %= 1000;
    nntpTime.lsdw *= 4294967;//65536*65536/1000;
	
    return nntpTime;
}

/*=============================================================================
函数名		:  GetRtcpInfo
功能		： get rtcp info

  算法实现	：（可选项）
  引用全局变量：无
  输入参数说明: 
  dwSSRC 同步源
  
	返回值说明： 成功为指向rtcpinfo 结构, 失败返回NULL;
=============================================================================*/
TRtcpInfo *GetRtcpInfo(ms_rtcp_t *pRtcp, u32 dwSSRC)
{
	s32 i=0;
    for(i=0; i<pRtcp->m_tRtcpInfoList.m_nSessionNum; i++)
	{
		if(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i].m_dwSSRC == dwSSRC)
		{
			return &(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i]);
		}
	}
	return NULL;
}

/*=============================================================================
函数名		:  AddRtcpInfo
功能		： Add rtcp info

  算法实现	：（可选项）
  引用全局变量：无
  输入参数说明: 
  Info   要增加rtcpinfo
  
	返回值说明： 成功为指向rtcpinfo 结构, 失败返回NULL;
=============================================================================*/
TRtcpInfo *AddRtcpInfo(ms_rtcp_t *pRtcp, TRtcpInfo *pInfo)
{
	s32 i=0;
    for(i=0; i<pRtcp->m_tRtcpInfoList.m_nSessionNum; i++)
	{
		if(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i].m_dwSSRC == pInfo->m_dwSSRC)
		{
			return &(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i]);
		}
		
		//填充无效SSRC数组成员
		if(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i].m_nInvalid&&
			pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i].m_dwSSRC == 0)
		{
			pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i] = *pInfo;
			return &(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i]);
		}
	}
	
	if(pRtcp->m_tRtcpInfoList.m_nSessionNum < MAX_SESSION_NUM)
	{
		pRtcp->m_tRtcpInfoList.m_tRtcpInfo[pRtcp->m_tRtcpInfoList.m_nSessionNum] = *pInfo;
		pRtcp->m_tRtcpInfoList.m_nSessionNum++;
		return &(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i]);
	}
	else
	{
		//find the Earliset time pos to update; 
		s32 nMaxEarlyTimePos = 0;
		u32 dwTime = 0;
		s32 j;
		for(j=0; j<pRtcp->m_tRtcpInfoList.m_nSessionNum; j++)
		{
			if(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[j].m_dwLSRMyTime < dwTime)
			{
                dwTime = pRtcp->m_tRtcpInfoList.m_tRtcpInfo[j].m_dwLSRMyTime;
				nMaxEarlyTimePos = j;
			}
		}
		pRtcp->m_tRtcpInfoList.m_tRtcpInfo[nMaxEarlyTimePos] = *pInfo;
		return &(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[i]);
	}
	return NULL;
}

/*=============================================================================
函数名		:  ParseRSQ
功能		： 解析自定义的重传请求
算法实现	：（可选项）
引用全局变量：无
输入参数说明: 

  返回值说明： 无
=============================================================================*/
void ParseRtcpRSQ(ms_rtcp_t *pRtcp, TRtcpSDES* pSdes, u32 nLen)
{
	TRtcpSDESRSQ *ptRSQ = NULL;
	
	//printf("repeat send request 1\n");
	
	if(NULL == pRtcp->m_pRtp || 
		NULL == pSdes || 
		sizeof(TRtcpSDESRSQ)+sizeof(u32) != nLen)
	{
		return;
	}
	
	if((sizeof(TRtcpSDESRSQ) - 2*sizeof(u8)) != pSdes->m_byLength)
	{
		return;
	}
	
	ptRSQ = (TRtcpSDESRSQ *)pSdes;
	
	//printf("$$$$$$$repeat send request 2\n");
	
	DealRtpRSQBackQuest(pRtcp->m_pRtp, ptRSQ);
	
	//printf("repeat send request 3\n");
}

//csp add 2008-11-28
void ParseRtcpFrameLoseEvent(ms_rtcp_t *pRtcp, TRtcpSDES* pSdes, u32 nLen)
{
	TRtcpSDESLose *ptLose = NULL;
	
	if(NULL == pRtcp->m_pRtp || 
		NULL == pSdes || 
		sizeof(TRtcpSDESLose)+sizeof(u32) != nLen)
	{
		return;
	}
	
	if((sizeof(TRtcpSDESLose) - 2*sizeof(u8)) != pSdes->m_byLength)
	{
		return;
	}
	
	ptLose = (TRtcpSDESLose *)pSdes;
	
	if(pRtcp->m_pFrameLoseCallBack != NULL)
	{
		pRtcp->m_pFrameLoseCallBack(ptLose, pRtcp->m_dwFrameLoseContext);
	}
}

/*=============================================================================
	函数名		:  ProcessRTCPPacket
	功能		： 处理单一数据包类型的RTCP包
	算法实现	：（可选项）
	引用全局变量：无
	输入参数说明: 
	              pData       数据缓冲
				  nDataLen    缓冲大小
                  type        RTCP type
	              nRCount     report Count   
				  myTime      local current wallclock time   
	返回值说明： 无
=============================================================================*/
void ProcessRTCPPacket(ms_rtcp_t *pRtcp, u8 *pData, s32 nDataLen, TRtcpType type, s32 nRCount, TUint64 myTime)
{
	s32 nScanned = 0;
    TRtcpInfo Info, *pInfo=NULL;
	
    if (nDataLen == 0)
	{
		return;
	}
	
    switch(type) 
    {
        case RTCP_SR:	
        case RTCP_RR:   
        {
            ConvertN2H(pData, 0, 1);
			
            //Info.m_dwSSRC = *(u32 *)(pData);
			memcpy(&Info.m_dwSSRC, pData, sizeof(u32));
			
			nScanned = sizeof(u32);
			
            if (Info.m_dwSSRC == pRtcp->m_tMyInfo.m_dwSSRC) 
            {
                pRtcp->m_tMyInfo.m_nCollision = 1;
                
                return;
            }
			
            pInfo = (TRtcpInfo *)GetRtcpInfo(pRtcp, Info.m_dwSSRC);
            
            if (pInfo == NULL) /* New source */
            {
                /* Initialize info */
                memset(&Info, 0, sizeof(Info));
				
                //Info.m_dwSSRC				= *(u32 *)(pData);
				memcpy(&Info.m_dwSSRC, pData, sizeof(u32));
				
                Info.m_tToRR.m_dwSSRC		= Info.m_dwSSRC;
                Info.m_bActive				= FALSE;
                Info.m_tSrc.m_nProbation	= MIN_SEQUENTIAL - 1;
                
                /* Add to list */
				pInfo = (TRtcpInfo *)AddRtcpInfo(pRtcp, &Info);
                
                if (pInfo == NULL) /* can't add to list? */
                {
                    /*array full*/
                }
            }
            break;
        }
		
        default: 
            break;
    }
	
    /* process the information */
    switch(type)
    {
        case RTCP_SR:
        {
            ConvertN2H(pData + nScanned, 0, W32Len(sizeof(TRtcpSR)));
			
            if (pInfo)
            {
                pInfo->m_tSR			= *(TRtcpSR *)(pData + nScanned);  
                pInfo->m_tToRR.m_dwLSR  = reduceNNTP(pInfo->m_tSR.m_tNNTP);
                pInfo->m_dwLSRMyTime    = reduceNNTP(myTime);
            }
			
            nScanned += SIZEOF_SR;
			/*break;*///maybe SDES
        }
		
        /* fall into RR */
		case RTCP_RR:   
        {
            if (pInfo)
            {
                TRtcpRR* rr = (TRtcpRR *)(pData + nScanned);
				
				s32 i = 0;
				
                ConvertN2H(pData + nScanned, 0, nRCount * W32Len(sizeof(TRtcpRR)));
                
                for (i=0; i < nRCount; i++)
                {
					//只处理自己发出的SR包的RR
                    if (rr[i].m_dwSSRC == pRtcp->m_tMyInfo.m_dwSSRC)
                    {
                        pInfo->m_tFromRR = rr[i];
                        break;
                    }
                }
            }
            break;//RR is packet'end;
        }
		
		case RTCP_SDES: 
        {
            TRtcpSDES *pSdes = NULL;
			
			s32 i = 0;
			
            for (i = 0; i < nRCount; i++)
            {
                ConvertN2H(pData + nScanned, 0, 1);
                
				//Info.m_dwSSRC = *(u32 *)(pData + nScanned);
				memcpy(&Info.m_dwSSRC, pData + nScanned, sizeof(u32));
				
                pSdes = (TRtcpSDES *)(pData + nScanned + sizeof(Info.m_dwSSRC));
				
				if(RTCP_SDES_NOTE == pSdes->m_byType)
				{
					ParseRtcpRSQ(pRtcp, pSdes, nDataLen);
				}
				//csp add 2008-11-28
				else if(RTCP_SDES_LOSE == pSdes->m_byType)
				{
					ParseRtcpFrameLoseEvent(pRtcp, pSdes, nDataLen);
				}
				
                pInfo = (TRtcpInfo*)GetRtcpInfo(pRtcp, Info.m_dwSSRC);
                
                if (pInfo != NULL)
                {
                    switch(pSdes->m_byType)
                    {
                        case RTCP_SDES_CNAME:
                            memcpy(&(pInfo->m_tCName), pSdes, SIZEOF_SDES(*pSdes));
                            pInfo->m_tCName.m_szValue[pSdes->m_byLength] = 0;
                            break;
						/* known SDES types that are not handled:
                        case RTCP_SDES_END:
                        case RTCP_SDES_NAME:
                        case RTCP_SDES_EMAIL:
                        case RTCP_SDES_PHONE:
                        case RTCP_SDES_LOC:
                        case RTCP_SDES_TOOL:
                        case RTCP_SDES_NOTE:
                        case RTCP_SDES_PRIV:
                            break;
						*/
                        }
                    }
					
                    nScanned += SIZEOF_SDES(*pSdes) + sizeof(u32);
                }
            break;
        }
		
        case RTCP_BYE:  
        {
            int i;
			
            for (i = 0; i < nRCount; i++)
            {
                ConvertN2H(pData + nScanned, 0, 1);
				
                //Info.m_dwSSRC = *(u32 *)(pData + nScanned);
				memcpy(&Info.m_dwSSRC, pData + nScanned, sizeof(u32));
				
                nScanned += sizeof(Info.m_dwSSRC);
                
                pInfo = (TRtcpInfo *)GetRtcpInfo(pRtcp, Info.m_dwSSRC);

                if (pInfo) 
                {
                    pInfo->m_nInvalid  = TRUE;
                    pInfo->m_dwSSRC    = 0;
                }					
            }
            break;
        }
		
        case RTCP_APP:  
            break;
    }
}

void RtcpDataCallBack(u8 *pBuf, s32 nBufSize, u32 dwContext)
{
	ms_rtcp_t *pMain = (ms_rtcp_t *)dwContext;
	if(pMain != NULL)
	{
		DealRtcpData(pMain, pBuf, nBufSize);
	}
}

void DealRtcpData(ms_rtcp_t *pRtcp, u8 *pBuf, s32 nBufSize)
{
	TRtcpHeader *ptHead  = NULL;
    u8			*currPtr = pBuf, *dataPtr, *compoundEnd;
    s32			hdr_count, hdr_len;
    TRtcpType   hdr_type;
	
	//printf("DealRtcpData 1\n");
	
	if(pRtcp->m_hSem == NULL) SemTake(pRtcp->m_hSem);
    
    compoundEnd = pBuf + nBufSize;
	
	//loop to deal compound pack 
    while (currPtr < compoundEnd)
    {
    	//printf("DealRtcpData 2\n");
		
        ptHead = (TRtcpHeader *)(currPtr);
        ConvertN2H(currPtr, 0, 1);
		
        hdr_count = GetBitField(ptHead->m_dwBits, HEADER_RC, HDR_LEN_RC);       
        hdr_type  = (TRtcpType)GetBitField(ptHead->m_dwBits, 
			HEADER_PT, HDR_LEN_PT);
        hdr_len   = sizeof(u32) * 
			(GetBitField(ptHead->m_dwBits, HEADER_len, HDR_LEN_len));
		
        if ((compoundEnd - currPtr) < hdr_len)
        {
            break;
        }
        
        dataPtr = (BYTE *)ptHead + sizeof(u32);
		
		//printf("DealRtcpData 3\n");
        
		//deal RTCP packet
        ProcessRTCPPacket(pRtcp, dataPtr, hdr_len, hdr_type, hdr_count, GetNNTPTime(pRtcp));
		
		//printf("DealRtcpData 4\n");
		
        currPtr += hdr_len + sizeof(u32);
    }
	
	if(pRtcp->m_hSem != NULL) SemGive(pRtcp->m_hSem);
}

ms_rtcp_t* CreateRtcp(u32 dwSSRC)
{
	ms_rtcp_t *pRtcp = NULL;
	
	pRtcp = (ms_rtcp_t *)malloc(sizeof(ms_rtcp_t));
	if(pRtcp == NULL)
	{
		return NULL;
	}
	InitRtcp(pRtcp);
	
	if(dwSSRC == 0)
	{
		goto end;
	}
	
	pRtcp->m_tMyInfo.m_dwSSRC = dwSSRC;
	if(SOCKET_ERROR == gethostname(pRtcp->m_tMyInfo.m_tCName.m_szValue,
		sizeof(pRtcp->m_tMyInfo.m_tCName.m_szValue )))
	{
		#ifndef WIN32
		printf("create rtcp:gethostname failed:(%d,%s)\n",errno,strerror(errno));
		#endif
		goto end;
	}
	
	pRtcp->m_tMyInfo.m_tCName.m_byLength = strlen(pRtcp->m_tMyInfo.m_tCName.m_szValue);
	
	pRtcp->m_pSocket = OpenSocket();
	if(pRtcp->m_pSocket == NULL)
	{
		goto end;
	}
	
	SetSocketCallBack(pRtcp->m_pSocket, RtcpDataCallBack, (u32)pRtcp);
	
	pRtcp->m_pdwBuf = malloc((MAX_RTCP_PACK + sizeof(u32) -1)/sizeof(u32) * sizeof(u32));
	if(pRtcp->m_pdwBuf == NULL)
	{
        goto end;
	}
	
	pRtcp->m_pCustomBuf = malloc((MAX_RTCP_PACK + sizeof(u32) -1)/sizeof(u32) * sizeof(u32));
	if(pRtcp->m_pCustomBuf == NULL)
	{
		goto end;
	}
	
	if(!SemBCreate(&pRtcp->m_hSem))
	{
		pRtcp->m_hSem=NULL;
		goto end;
	}
	
	return pRtcp;
	
end:
	
	printf("create rtcp failed\n");
	
	FreeRtcpBuf(pRtcp);
	pRtcp = NULL;
	
	return NULL;
}

u16 SetRtcpRtp(ms_rtcp_t *pRtcp, struct ifly_rtp_t *pRtp)
{
	pRtcp->m_pRtp = pRtp;
	return MEDIASTREAM_NO_ERROR;
}

//csp add 2008-11-28
u16 SetRtcpFrameLoseCallBack(ms_rtcp_t *pRtcp, PLOSEPROC pCallBackHandle, u32 dwContext)
{
	if(pRtcp != NULL)
	{
		pRtcp->m_pFrameLoseCallBack = pCallBackHandle;
		pRtcp->m_dwFrameLoseContext = dwContext;
	}
	return MEDIASTREAM_NO_ERROR;
}

void ResetRtcpSSRC(ms_rtcp_t *pRtcp, u32 dwSSRC)
{
	pRtcp->m_tMyInfo.m_dwSSRC = dwSSRC;
}

u16 SetRtcpLocalAddr(ms_rtcp_t *pRtcp, u32 dwIp, u16 wPort)
{
	if(pRtcp == NULL || pRtcp->m_pSocket == NULL)
	{
		return ERROR_RTCP_NO_INIT;
	}
	
	//the same to last set  
	if(dwIp == pRtcp->m_tLocalAddr.m_dwIP&&
		wPort == pRtcp->m_tLocalAddr.m_wPort)
	{
		return MEDIASTREAM_NO_ERROR;
	}
	
	if(!SocketCreate(pRtcp->m_pSocket, SOCK_DGRAM, wPort, ADDR_ANY, dwIp, TRUE))
	{
		return ERROR_SND_CREATE_SOCK;
	}
	
	pRtcp->m_tLocalAddr.m_dwIP  = dwIp;
	pRtcp->m_tLocalAddr.m_wPort = wPort;
	
	return MEDIASTREAM_NO_ERROR;
}

u16 RemoveRtcpLocalAddr(ms_rtcp_t *pRtcp)
{
	if(pRtcp || pRtcp->m_pSocket == NULL)
	{
		return ERROR_RTP_NO_INIT;
	}
	
    SocketDestroy(pRtcp->m_pSocket, TRUE);
	
	memset(&pRtcp->m_tLocalAddr, 0, sizeof(pRtcp->m_tLocalAddr)); 
	
	return MEDIASTREAM_NO_ERROR;
}

u16 SetRtcpRemoteAddr(ms_rtcp_t *pRtcp, TRemoteAddr *pRemoteAddr)
{
	pRtcp->m_tRemoteAddr = *pRemoteAddr;
	
	return MEDIASTREAM_NO_ERROR;
}

u16 UpdateRtcpSend(ms_rtcp_t *pRtcp, s32 nSendDataSize, u32 dwTimeStamp)
{
#ifdef RS_ENABLE
	if(pRtcp->m_hSem != NULL) SemTake(pRtcp->m_hSem);
	
    pRtcp->m_tMyInfo.m_bActive = TRUE;
    pRtcp->m_tMyInfo.m_tSR.m_nPackets++;
    pRtcp->m_tMyInfo.m_tSR.m_nBytes += nSendDataSize;
    pRtcp->m_tMyInfo.m_tSR.m_tNNTP   = GetNNTPTime(pRtcp);
    pRtcp->m_tMyInfo.m_tSR.m_dwRTP   = dwTimeStamp;
	
	if(pRtcp->m_hSem != NULL) SemGive(pRtcp->m_hSem);
	
    /*if (pRtcp->m_tMyInfo.m_nCollision)
	{
		return ERROR_RTP_SSRC_COLLISION;
	}*/
#endif
	
	return MEDIASTREAM_NO_ERROR;
}

u16 UpdateRtcpRcv(ms_rtcp_t *pRtcp, u32 dwSSRC, u32 dwLocalTimestamp, u32 dwTimestamp, u16 wSequence)
{
#ifdef RS_ENABLE
	TRtcpInfo *pRtcpInfo = NULL;
	
	if(pRtcp->m_hSem != NULL) SemTake(pRtcp->m_hSem);
	
    if (dwSSRC == pRtcp->m_tMyInfo.m_dwSSRC) 
    {
		pRtcp->m_tMyInfo.m_nCollision = 1;
		if(pRtcp->m_hSem != NULL) SemGive(pRtcp->m_hSem);
		return ERROR_RTP_SSRC_COLLISION;
    }
	
    pRtcpInfo = GetRtcpInfo(pRtcp, dwSSRC);
	
    if (pRtcpInfo == NULL&& pRtcp->m_pSocket != NULL) /* New source */
    {
		// Initialize info
		TRtcpInfo Info;
		memset(&Info, 0, sizeof(Info)); 		
		Info.m_dwSSRC				= dwSSRC;
		Info.m_tToRR.m_dwSSRC		= Info.m_dwSSRC;
		Info.m_bActive				= FALSE;
		Info.m_tSrc.m_nProbation	= MIN_SEQUENTIAL - 1;
		
		// Add to list
		pRtcpInfo = (TRtcpInfo *)AddRtcpInfo(pRtcp, &Info);
		
		if (pRtcpInfo == NULL) // can't add to list?
		{
			//array full
		}
	}
    else
    {
		if (!pRtcpInfo->m_nInvalid)
		{
			pRtcpInfo->m_bActive = TRUE;
			UpdateRtcpSeq(pRtcp, &(pRtcpInfo->m_tSrc), wSequence, dwLocalTimestamp, dwTimestamp);        
		}
    }
	
    if(pRtcp->m_hSem != NULL) SemGive(pRtcp->m_hSem);
#endif
	
	return MEDIASTREAM_NO_ERROR;
}

BOOL32 UpdateRtcpSeq(ms_rtcp_t *pRtcp, TRtpSource *pRtpSource, u16 seq, u32 dwArrivalTS, u32 dwTimeStamp)
{
#ifdef RS_ENABLE
	u16 uDelta = (u16)(seq - pRtpSource->m_nMaxSeq);
	
	   /*
	   * Source is not valid until MIN_SEQUENTIAL packets with
	   * sequential sequence numbers have been received.
	   */
	   if (pRtpSource->m_nProbation) 
       { 
		   /* packet is in sequence */
           if (seq == pRtpSource->m_nMaxSeq + 1) 
           {
               pRtpSource->m_nProbation--;
               pRtpSource->m_nMaxSeq = seq;
               if (pRtpSource->m_nProbation == 0) 
               {
                   InitRtcpSeq(pRtcp, pRtpSource, seq);
                   pRtpSource->m_nReceived++;
                   return TRUE;
               }
           }
           else
           {
               pRtpSource->m_nProbation = MIN_SEQUENTIAL - 1;
               pRtpSource->m_nProbation = seq;
           }
           return FALSE;
       }
       else if (uDelta < MAX_DROPOUT) 
       {
		   /* in order, with permissible gap */
           if (seq < pRtpSource->m_nMaxSeq) 
		   {
			   /* Sequence number wrapped - count another 64K cycle.*/
			   pRtpSource->m_nCycles += RTP_SEQ_MOD;
		   }
           pRtpSource->m_nMaxSeq = seq;
       }
       else if (uDelta <= RTP_SEQ_MOD - MAX_MISORDER)
       {
           if (seq == pRtpSource->m_nBadSeq)
           {
		   /*
		   * Two sequential packets -- assume that the other side
		   * restarted without telling us so just re-sync
		   * (i.e., pretend this was the first packet).
			   */
			   InitRtcpSeq(pRtcp, pRtpSource, seq);
           }
           else
           {
               pRtpSource->m_nBadSeq = (seq + 1) & (RTP_SEQ_MOD-1);
               return FALSE;
           }
       }
       else
       {
		   /* duplicate or reordered packet */ /*the same to the standard*/
       }
       {// for C
           s32  nTransit = (s32)(dwArrivalTS - dwTimeStamp);
           s32  nDelta = (s32)(nTransit - pRtpSource->m_nTransit);
           pRtpSource->m_nTransit = nTransit;
           if (nDelta < 0) nDelta = -nDelta;
		   
		   //reduce round-off error
           pRtpSource->m_nJitter += (nDelta - 
			   ((pRtpSource->m_nJitter + 8) >> 4));
       }
       pRtpSource->m_nReceived++;
#endif

	   return TRUE;
}

void InitRtcpSeq(ms_rtcp_t *pRtcp, TRtpSource *pRtpSource, u16 seq)
{
	pRtpSource->m_nBaseSeq       = seq;
	pRtpSource->m_nMaxSeq        = seq;
	pRtpSource->m_nBadSeq        = RTP_SEQ_MOD + 1;
	pRtpSource->m_nCycles        = 0;
	pRtpSource->m_nReceived      = 0;
    pRtpSource->m_nReceivedPrior = 0;
	pRtpSource->m_nExpectedPrior = 0;
}

u16 DealRtcpTimer(ms_rtcp_t *pRtcp)
{
	TBuffer tBuf = {0, NULL};
	
	s32 i = 0;
	
	if(pRtcp == NULL || pRtcp->m_pdwBuf == NULL || pRtcp->m_pSocket == NULL) 
	{
		return ERROR_RTP_NO_INIT;
	}
    
	tBuf = BufCreate(pRtcp->m_pdwBuf,MAX_RTCP_PACK);
	
	CreateRTCPPacket(pRtcp, &tBuf);
	
	if(pRtcp->m_hSem != NULL) SemTake(pRtcp->m_hSem);
	
	for(i=0; i<pRtcp->m_tRemoteAddr.m_byNum; i++)
	{
		if( (pRtcp->m_tRemoteAddr.m_tAddr[i].m_dwIP != 0) &&
			(pRtcp->m_tRemoteAddr.m_tAddr[i].m_wPort != 0) )
		{
			SocketSendTo(pRtcp->m_pSocket, tBuf.m_pBuf, tBuf.m_dwLen, 
				pRtcp->m_tRemoteAddr.m_tAddr[i].m_dwIP,
				pRtcp->m_tRemoteAddr.m_tAddr[i].m_wPort);
		}
	}
	
	if(pRtcp->m_hSem != NULL) SemGive(pRtcp->m_hSem);
	
	return MEDIASTREAM_NO_ERROR;
}

/*=============================================================================
函数名		:  MakeHeader
功能		： make RTCP fixed header

  算法实现	：（可选项）
  引用全局变量：无
  输入参数说明: 无
  
	返回值说明： 64 bits time
=============================================================================*/
TRtcpHeader MakeHeader(u32 dwSSRC, u8 count, TRtcpType type, u16 dataLen)
{
	TRtcpHeader header;
	
    header.m_dwSSRC = dwSSRC;
    
    header.m_dwBits = RTCP_HEADER_INIT;
	
	//set reception report count
    header.m_dwBits = SetBitField(header.m_dwBits, count, HEADER_RC, HDR_LEN_RC);
	//set report type
    header.m_dwBits = SetBitField(header.m_dwBits, type,  HEADER_PT, HDR_LEN_PT);
	//set one report len  ,see the standard 
    header.m_dwBits = SetBitField(header.m_dwBits, W32Len(dataLen) - 1, 
		HEADER_len, HDR_LEN_len);
    
    header.m_dwSSRC = htonl(header.m_dwSSRC);
	header.m_dwBits = htonl(header.m_dwBits);
	
    return header;
}

BOOL32 BufAddToBuffer(TBuffer *pTo, TBuffer *pFrom, u32 Offset)
{
	if (pFrom->m_dwLen + Offset <= pTo->m_dwLen)
    {
        memcpy((u8*)pTo->m_pBuf + Offset, pFrom->m_pBuf, pFrom->m_dwLen);
		return TRUE;
    }
    return FALSE;
}

BOOL32 BufValid(TBuffer *pBuf, u32 dwSize)
{
	return (dwSize <= pBuf->m_dwLen  &&  pBuf->m_pBuf);
}

TBuffer BufCreate(void* pData, u32 dwSize)
{
    TBuffer tBuf;
    tBuf.m_pBuf  = (u8 *)pData;
    tBuf.m_dwLen = dwSize;
    return tBuf;
}

void CreateRTCPPacket(ms_rtcp_t *pRtcp, TBuffer *ptBuf)
{
	TRtcpHeader  tHead;
    u32          dwAllocated = 0;
    TBuffer      tBufC;
    TRtcpType    type = RTCP_SR;
	
	u8 cc = 0;
	TRtcpInfo * pInfo = NULL;
	s32 Index = 0;
	
    if (BufValid(ptBuf, SIZEOF_RTCPHEADER + SIZEOF_SR))
    {
        TUint64 myTime = pRtcp->m_tMyInfo.m_tSR.m_tNNTP;
		
        dwAllocated = SIZEOF_RTCPHEADER;
        if (pRtcp->m_tMyInfo.m_bActive)
        {
			//SR packet
            pRtcp->m_tMyInfo.m_bActive = FALSE;
            tBufC = BufCreate(&(pRtcp->m_tMyInfo.m_tSR), SIZEOF_SR);
            BufAddToBuffer(ptBuf, &tBufC, dwAllocated);            
            ConvertH2N(ptBuf->m_pBuf + dwAllocated, 0, W32Len(tBufC.m_dwLen));
            dwAllocated += SIZEOF_SR;
        }
        else 
        {
            type = RTCP_RR;
        }
		
		//RR packets
		cc = 0;
        pInfo = NULL;
        for(Index=0; Index<pRtcp->m_tRtcpInfoList.m_nSessionNum; Index++)
        {
            pInfo = (TRtcpInfo *)&(pRtcp->m_tRtcpInfoList.m_tRtcpInfo[Index]);
            if (pInfo->m_bActive)
            {
                pInfo->m_tToRR.m_nFLost     = GetLost    (&(pInfo->m_tSrc));
                pInfo->m_tToRR.m_nJitter    = GetJitter  (&(pInfo->m_tSrc));
                pInfo->m_tToRR.m_nExtMaxSeq = GetSequence(&(pInfo->m_tSrc));
                pInfo->m_tToRR.m_dwDLSR     = 
                    (pInfo->m_dwLSRMyTime) ? 
					(reduceNNTP(myTime)-pInfo->m_dwLSRMyTime) : 0;
                
                tBufC = BufCreate(&(pInfo->m_tToRR), SIZEOF_RR);
				
                if (BufAddToBuffer(ptBuf, &tBufC, dwAllocated))
                {
                    cc++;
                    if (cc == 32) break;
                    ConvertH2N(ptBuf->m_pBuf + dwAllocated, 0, 
						W32Len(tBufC.m_dwLen));
                    dwAllocated += SIZEOF_RR;
                }                         
                pInfo->m_bActive = FALSE;
            }
        }
		
        tHead = MakeHeader(pRtcp->m_tMyInfo.m_dwSSRC, cc, type, (u16)dwAllocated);
       	tBufC = BufCreate(&tHead, SIZEOF_RTCPHEADER);
        BufAddToBuffer(ptBuf, &tBufC, 0);
		
        /* add an CNAME SDES packet to the compound packet */
        if (BufValid(ptBuf, 
            dwAllocated + SIZEOF_RTCPHEADER + SIZEOF_SDES(pRtcp->m_tMyInfo.m_tCName)))
        {
            TBuffer tSdesBuf;
			
            /* 'tSdesBuf' is inside the compound buffer 'buf' */
            tSdesBuf = BufCreate(ptBuf->m_pBuf + dwAllocated, 
				(SIZEOF_RTCPHEADER + SIZEOF_SDES(pRtcp->m_tMyInfo.m_tCName)));
			
            tHead = MakeHeader(pRtcp->m_tMyInfo.m_dwSSRC, 1, RTCP_SDES, 
				(u16)tSdesBuf.m_dwLen);
            
            memcpy(tSdesBuf.m_pBuf, (s8 *)&tHead, SIZEOF_RTCPHEADER);
            memcpy(tSdesBuf.m_pBuf + SIZEOF_RTCPHEADER, &(pRtcp->m_tMyInfo.m_tCName),
				SIZEOF_SDES(pRtcp->m_tMyInfo.m_tCName));
			
            dwAllocated += tSdesBuf.m_dwLen;
        }
        
        if (pRtcp->m_tMyInfo.m_nCollision == 1  &&
            BufValid(ptBuf, dwAllocated + SIZEOF_RTCPHEADER))
        {
            tHead = MakeHeader(pRtcp->m_tMyInfo.m_dwSSRC, 1, RTCP_BYE, 
				SIZEOF_RTCPHEADER);
            
            tBufC = BufCreate(&tHead, SIZEOF_RTCPHEADER);
            BufAddToBuffer(ptBuf, &tBufC, dwAllocated);
            pRtcp->m_tMyInfo.m_nCollision = 2;
            dwAllocated += SIZEOF_RTCPHEADER;
        }
    }
	
    ptBuf->m_dwLen = dwAllocated;
    return;
}

u32 GetLost(TRtpSource *pRtpSource)
{
	u32 extended_max;
    u32 expected;
    s32 received_interval;
    s32 expected_interval;
    s32 lost;
    s32 lost_interval;
    u8  fraction;
	
    extended_max = pRtpSource->m_nCycles + pRtpSource->m_nMaxSeq;
    expected = extended_max - pRtpSource->m_nBaseSeq + 1;
    lost = expected - pRtpSource->m_nReceived;
    expected_interval = expected - pRtpSource->m_nExpectedPrior;
    pRtpSource->m_nExpectedPrior = expected;
    received_interval = pRtpSource->m_nReceived - pRtpSource->m_nReceivedPrior;
	pRtpSource->m_nReceivedPrior = pRtpSource->m_nReceived;
    lost_interval = expected_interval - received_interval;
    
    if (expected_interval == 0  ||  lost_interval <= 0) 
        fraction = 0;
    else 
        fraction = (u8)((lost_interval << 8) / expected_interval);
	
    return (fraction << 24) + lost;
}

u32 GetJitter(TRtpSource *pRtpSource)
{
	return pRtpSource->m_nJitter >> 4;
}

u32 GetSequence(TRtpSource *pRtpSource)
{
	return pRtpSource->m_nMaxSeq + pRtpSource->m_nCycles;
}

void CreateCustomRTCPPacket(ms_rtcp_t *pRtcp, TBuffer *ptBuf, TRtcpSDESRSQ *pRSQ)
{
	TRtcpHeader		tHead;
    u32				dwAllocated = 0;
//	TRtcpType		emrtcptype = RTCP_SDES;
	TRtcpSDesType	emsdestype = RTCP_SDES_NOTE;
	
	/* add an NOTE SDES (TRtcpSDESRSQ) packet to the compound packet */
    if (BufValid(ptBuf, SIZEOF_RTCPHEADER + sizeof(TRtcpSDESRSQ)))
    {
        TBuffer tSdesBuf;
		
        /* 'tSdesBuf' is inside the compound buffer 'buf' */
        tSdesBuf = BufCreate(ptBuf->m_pBuf + dwAllocated, 
			SIZEOF_RTCPHEADER + sizeof(TRtcpSDESRSQ));
		
        tHead = MakeHeader(pRtcp->m_tMyInfo.m_dwSSRC, 1, RTCP_SDES, 
			(u16)tSdesBuf.m_dwLen);
        
		pRSQ->m_byType = emsdestype;
		pRSQ->m_byLength = sizeof(TRtcpSDESRSQ) - 2*sizeof(u8);
        memcpy(tSdesBuf.m_pBuf, (s8 *)&tHead, SIZEOF_RTCPHEADER);
        memcpy(tSdesBuf.m_pBuf + SIZEOF_RTCPHEADER, (s8 *)pRSQ, sizeof(TRtcpSDESRSQ));
		
        dwAllocated += tSdesBuf.m_dwLen;
    }
	
    ptBuf->m_dwLen = dwAllocated;
	
    return;
}

//csp add 2008-11-28
void CreateCustomRTCPPacket2(ms_rtcp_t *pRtcp, TBuffer *ptBuf, TRtcpSDESLose *pLose)
{
	TRtcpHeader		tHead;
    u32				dwAllocated = 0;
//	TRtcpType		emrtcptype = RTCP_SDES;
	TRtcpSDesType	emsdestype = RTCP_SDES_LOSE;
	
	/* add an LOSE SDES (TRtcpSDESLose) packet to the compound packet */
    if (BufValid(ptBuf, SIZEOF_RTCPHEADER + sizeof(TRtcpSDESLose)))
    {
        TBuffer tSdesBuf;
		
        /* 'tSdesBuf' is inside the compound buffer 'buf' */
        tSdesBuf = BufCreate(ptBuf->m_pBuf + dwAllocated, 
			SIZEOF_RTCPHEADER + sizeof(TRtcpSDESLose));
		
        tHead = MakeHeader(pRtcp->m_tMyInfo.m_dwSSRC, 1, RTCP_SDES, 
			(u16)tSdesBuf.m_dwLen);
        
		pLose->m_byType = emsdestype;
		pLose->m_byLength = sizeof(TRtcpSDESLose) - 2*sizeof(u8);
        memcpy(tSdesBuf.m_pBuf, (s8 *)&tHead, SIZEOF_RTCPHEADER);
        memcpy(tSdesBuf.m_pBuf + SIZEOF_RTCPHEADER, (s8 *)pLose, sizeof(TRtcpSDESLose));
		
        dwAllocated += tSdesBuf.m_dwLen;
    }
	
    ptBuf->m_dwLen = dwAllocated;
	
    return;
}

u16 SendRtcpRSQ(ms_rtcp_t *pRtcp, TRtcpSDESRSQ *pRSQ)
{
	TBuffer tBuf;
	TRtcpSDESRSQ tMyRtpRSQ = *pRSQ;
	
	if(pRtcp->m_pCustomBuf == NULL|| pRtcp->m_pSocket== NULL)
	{
		return ERROR_RTCP_NO_INIT;
	}
	
	tBuf = BufCreate(pRtcp->m_pCustomBuf,MAX_RTCP_PACK);
	
	CreateCustomRTCPPacket(pRtcp, &tBuf, &tMyRtpRSQ);
	
	//过滤掉这种可能
	if( (pRtcp->m_tLocalAddr.m_dwIP == pRtcp->m_tRemoteAddr.m_tAddr[0].m_dwIP ||
		0 == pRtcp->m_tRemoteAddr.m_tAddr[0].m_dwIP) &&
		(pRtcp->m_tLocalAddr.m_wPort == pRtcp->m_tRemoteAddr.m_tAddr[0].m_wPort) )
	{
		return MEDIASTREAM_NO_ERROR;
	}
	
	if(pRtcp->m_hSem != NULL) SemTake(pRtcp->m_hSem);
	
	//根据RTCP反馈地址设定进行重传请求
	if( (pRtcp->m_tRemoteAddr.m_tAddr[0].m_dwIP != 0)&&
		(pRtcp->m_tRemoteAddr.m_tAddr[0].m_wPort != 0) )
	{
		SocketSendTo(pRtcp->m_pSocket, tBuf.m_pBuf, tBuf.m_dwLen, 
			pRtcp->m_tRemoteAddr.m_tAddr[0].m_dwIP,
			pRtcp->m_tRemoteAddr.m_tAddr[0].m_wPort);
	}
	
	if(pRtcp->m_hSem != NULL) SemGive(pRtcp->m_hSem);
	
	return MEDIASTREAM_NO_ERROR;
}

//csp add 2008-11-28
u16 SendRtcpFrameLoseEvent(ms_rtcp_t *pRtcp, TRtcpSDESLose *pLose)
{
	TBuffer tBuf;
	TRtcpSDESLose tMyRtpLose = *pLose;
	
	if(pRtcp->m_pCustomBuf == NULL|| pRtcp->m_pSocket== NULL)
	{
		return ERROR_RTCP_NO_INIT;
	}
	
	tBuf = BufCreate(pRtcp->m_pCustomBuf,MAX_RTCP_PACK);
	
	CreateCustomRTCPPacket2(pRtcp, &tBuf, &tMyRtpLose);
	
	//过滤掉这种可能
	if( (pRtcp->m_tLocalAddr.m_dwIP == pRtcp->m_tRemoteAddr.m_tAddr[0].m_dwIP ||
		0 == pRtcp->m_tRemoteAddr.m_tAddr[0].m_dwIP) &&
		(pRtcp->m_tLocalAddr.m_wPort == pRtcp->m_tRemoteAddr.m_tAddr[0].m_wPort) )
	{
		return MEDIASTREAM_NO_ERROR;
	}
	
	if(pRtcp->m_hSem != NULL) SemTake(pRtcp->m_hSem);
	
	//根据RTCP反馈地址设定进行重传请求
	if( (pRtcp->m_tRemoteAddr.m_tAddr[0].m_dwIP != 0)&&
		(pRtcp->m_tRemoteAddr.m_tAddr[0].m_wPort != 0) )
	{
		SocketSendTo(pRtcp->m_pSocket, tBuf.m_pBuf, tBuf.m_dwLen, 
			pRtcp->m_tRemoteAddr.m_tAddr[0].m_dwIP,
			pRtcp->m_tRemoteAddr.m_tAddr[0].m_wPort);
	}
	
	if(pRtcp->m_hSem != NULL) SemGive(pRtcp->m_hSem);
	
	return MEDIASTREAM_NO_ERROR;
}
