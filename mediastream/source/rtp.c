#include "msrtp.h"

extern s32 g_nRepeatSnd;
extern s32 g_nDiscardSpan;
extern s32 g_nShowDebugInfo;

void InitRtp(ifly_rtp_t *pRtp)
{
	if(pRtp)
	{
		pRtp->m_pRtcp			= NULL;//csp add 2008-11-20
		
		pRtp->m_dwSSRC			= 0; 
		//pRtp->m_wSeqNum		= 0;
		pRtp->m_wSeqNum			= 0xFFFF;
		pRtp->m_pSocket			= NULL;
		pRtp->m_pLoopBuf		= NULL;
		pRtp->m_pPackBuf		= NULL;
		pRtp->m_pCallBack		= NULL;
		pRtp->m_dwContext		= (u32)NULL;
		pRtp->m_dwTotalPackNum	= 0;
		
		pRtp->m_pRLoopBuf		= NULL;
		pRtp->m_pRSPackBuf		= NULL;
		pRtp->m_bRepeatSend		= FALSE;
		pRtp->m_bIsRLBFull		= FALSE;
		pRtp->m_bIsRLBWrited	= FALSE;
		pRtp->m_wRLBMinSN		= 0;
		pRtp->m_wRLBMinPos		= 0;
		pRtp->m_wRLBUnitNum		= 0;
		pRtp->m_wRLBWritePos	= 0;
		pRtp->m_nRLBUnitSize	= MAX_PACK_SIZE+RTP_FIXEDHEADER_SIZE+sizeof(s32);
		
		pRtp->m_hSynSem = NULL;
		//m_hSynSem 开始有信号
		SemBCreate(&pRtp->m_hSynSem);
		
		memset(&pRtp->m_tRemoteAddr, 0, sizeof(pRtp->m_tRemoteAddr));
		memset(&pRtp->m_tLocalAddr, 0, sizeof(pRtp->m_tLocalAddr)); 
		memset(pRtp->m_dwRtpHeader, 0, sizeof(pRtp->m_dwRtpHeader));
	}
}

u16 DestroyRtp(ifly_rtp_t *pRtp)
{
	/*if(pRtp)
	{
		if(pRtp->m_pLoopBuf) DestroyLoopBuf(pRtp->m_pLoopBuf);
		if(pRtp->m_pPackBuf) free(pRtp->m_pPackBuf);
		if(pRtp->m_pSocket)  CloseSocket(pRtp->m_pSocket);
		free(pRtp);
	}*/
	if(pRtp)
	{
		FreeRtpBuf(pRtp);
		if(pRtp->m_hSynSem != NULL)
		{
			SemDelete(pRtp->m_hSynSem);
			pRtp->m_hSynSem = NULL;
		}
		free(pRtp);
	}
	return MEDIASTREAM_NO_ERROR;
}

void FreeRtpBuf(ifly_rtp_t *pRtp)
{
	if(pRtp)
	{
		if(pRtp->m_pSocket)
		{
			CloseSocket(pRtp->m_pSocket);
			pRtp->m_pSocket = NULL;
		}
		if(pRtp->m_pLoopBuf)
		{
			DestroyLoopBuf(pRtp->m_pLoopBuf);
			pRtp->m_pLoopBuf = NULL;
		}
		if(pRtp->m_pPackBuf)
		{
			free(pRtp->m_pPackBuf);
			pRtp->m_pPackBuf = NULL;
		}
		if(pRtp->m_pRSPackBuf)
		{
			free(pRtp->m_pRSPackBuf);
			pRtp->m_pRSPackBuf = NULL;
		}
		
		memset(&pRtp->m_tRemoteAddr, 0, sizeof(pRtp->m_tRemoteAddr));
		memset(&pRtp->m_tLocalAddr, 0, sizeof(pRtp->m_tLocalAddr)); 
		pRtp->m_pRtcp			= NULL;
		pRtp->m_dwSSRC			= 0; 
		pRtp->m_dwTotalPackNum	= 0;
		pRtp->m_wSeqNum			= 0xFFFF;
		pRtp->m_pCallBack		= NULL;
		pRtp->m_dwContext		= (u32)NULL;
		
		SemTake(pRtp->m_hSynSem);
		if(pRtp->m_pRLoopBuf)
		{
			free(pRtp->m_pRLoopBuf);
			pRtp->m_pRLoopBuf = NULL;
		}
		pRtp->m_bRepeatSend		= FALSE;
		pRtp->m_bIsRLBFull		= FALSE;
		pRtp->m_bIsRLBWrited	= FALSE;
		pRtp->m_wRLBMinSN		= 0;
		pRtp->m_wRLBMinPos		= 0;
		pRtp->m_wRLBWritePos	= 0;
		pRtp->m_wRLBUnitNum		= 0;
		SemGive(pRtp->m_hSynSem);
	}
}

// 数据回调
void RtpCallBackProc(u8 *pBuf, s32 nSize, u32 dwContext)
{
	ifly_rtp_t *pMain = (ifly_rtp_t *)dwContext;
	if(pMain != NULL)
	{
		DealRtpData(pMain, pBuf, nSize);
	}
	else
	{
		printf("[RtpCallBackProc] dwContext == 0\n");
	}
}

void DealRtpData(ifly_rtp_t *pRtp,u8 *pBuf, s32 nSize)
{
	TRtpPack tRtpPack;
	s32 nOffSet;
	s32 xStart;
	u32 *header=(u32*)pBuf;
	
	if(pRtp == NULL || pBuf == NULL || nSize < RTP_FIXEDHEADER_SIZE) return;
	
	memset(&tRtpPack, 0, sizeof(tRtpPack));
	
	//RTP fixed Header Convert
    ConvertN2H(pBuf, 0, 3);
	//CSRC convert
    ConvertN2H(pBuf, 3, GetBitField(header[0], 24, 4));
    tRtpPack.m_dwTimeStamp = header[1];
    tRtpPack.m_wSequence   = (u16)GetBitField(header[0], 0, 16);
    tRtpPack.m_dwSSRC      = header[2];
    tRtpPack.m_byMark      = (u8)GetBitField(header[0], 23, 1);
    tRtpPack.m_byPayload   = (u8)GetBitField(header[0], 16, 7);
    
    nOffSet = RTP_FIXEDHEADER_SIZE + GetBitField(header[0], 24, 4) * sizeof(u32);   
	
	tRtpPack.m_nRealSize    = nSize - nOffSet;
	tRtpPack.m_pRealData    = pBuf + nOffSet; 
	tRtpPack.m_byExtence    = (u8)GetBitField(header[0], 28, 1);
	if (tRtpPack.m_byExtence)/*Extension Bit Set*/
    {
        xStart = nOffSet/sizeof(u32);
        ConvertN2H(pBuf, xStart, 1);
		tRtpPack.m_nExSize = (u16)GetBitField(header[xStart], 0, 16);
        tRtpPack.m_pExData = pBuf + (xStart+1) * sizeof(u32); 
		tRtpPack.m_nRealSize -= ((tRtpPack.m_nExSize + 1)*sizeof(u32));
		tRtpPack.m_pRealData += ((tRtpPack.m_nExSize + 1)*sizeof(u32));
    }
	
	if(GetBitField(header[0], 29, 1))/*Padding Bit Set*/
    {
		tRtpPack.m_nRealSize -= (s32)(*(pBuf+nSize-1));
    }
    
	if(tRtpPack.m_nRealSize < 0 && tRtpPack.m_nRealSize > MAX_RCV_PACK_SIZE)
	{
		printf("[Rtp::DealRtpData] RTP REALSIZE EXCEPTION:m_nRealSize=%d\n", tRtpPack.m_nRealSize);
		return;
	}
	
	//printf("[Rtp::DealRtpData] RTP Info:m_nRealSize=%d, Padding Bit=%d, m_byExtence=%d, m_dwTimeStamp=%d, m_wSequence=%d, m_byPayload=%d, m_dwSSRC=%d, m_byMark=%d\n", 
		//tRtpPack.m_nRealSize, GetBitField(header[0], 29, 1), tRtpPack.m_byExtence, tRtpPack.m_dwTimeStamp, tRtpPack.m_wSequence, tRtpPack.m_byPayload, tRtpPack.m_dwSSRC, tRtpPack.m_byMark);
	
	if(pRtp->m_pCallBack != NULL)
	{
		pRtp->m_pCallBack(&tRtpPack, pRtp->m_dwContext);
	}
	else
	{
		printf("[Rtp::DealRtpData] m_pCallBack == NULL\n");
	}
	
	//cap add 2008-11-20
	if(pRtp->m_pRtcp != NULL)
	{
		UpdateRtcpRcv(pRtp->m_pRtcp, tRtpPack.m_dwSSRC, GetSystemTick(), tRtpPack.m_dwTimeStamp, tRtpPack.m_wSequence);
	}
}

ifly_rtp_t* CreateRtp(u32 dwSSRC, BOOL32 bAllocLPBuf)
{
	ifly_rtp_t *pRtp = NULL;
	
	if(dwSSRC == 0)
	{
		goto end;
	}
	
	pRtp = (ifly_rtp_t *)malloc(sizeof(ifly_rtp_t));
	if(pRtp == NULL)
	{
		return NULL;
	}
	InitRtp(pRtp);
	
	pRtp->m_dwSSRC = dwSSRC;
	//pRtp->m_wSeqNum = 0;
	pRtp->m_wSeqNum = 0xFFFF;  
	
	pRtp->m_pSocket = OpenSocket();
	if(pRtp->m_pSocket == NULL)
	{
		goto end;
	}
    
	SetSocketCallBack(pRtp->m_pSocket,RtpCallBackProc,(u32)pRtp);
	
	if(TRUE == bAllocLPBuf)
	{
		//csp add 2008-11-20
		//创建环状缓冲
		pRtp->m_pLoopBuf = CreateLoopBuf(MAX_PACK_SIZE+RTP_FIXEDHEADER_SIZE, LOOP_BUF_UINT_NUM);
		if(pRtp->m_pLoopBuf == NULL)
		{
			goto end;
		}
	}
	else
	{
		pRtp->m_pLoopBuf = NULL;
	}
	
	pRtp->m_pPackBuf = (u8 *)malloc(MAX_PACK_SIZE+RTP_FIXEDHEADER_SIZE);
	if(pRtp->m_pPackBuf == NULL)
	{
		goto end;
	}
	
	return pRtp;
	
end:
	
	printf("create rtp failed\n");
	
	return NULL;
}

u16 SetRtpCallBack(ifly_rtp_t *pRtp, PRCVPROC pCallBackHandle, u32 dwContext)
{
	pRtp->m_pCallBack = pCallBackHandle;
	pRtp->m_dwContext = dwContext;
	return MEDIASTREAM_NO_ERROR;
}

#ifndef WIN32
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <signal.h>
#include <netinet/ip.h>
#endif

//获得本机ip  //08-03-08 chenjie for 组播
void GetIp(char *ifname, u32 *ip);

void GetIp(char *ifname, u32 *ip)
{
#ifndef WIN32
    int    ret,s;
    struct ifreq  ifr;
    struct sockaddr_in *addr;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ret = ioctl(s, SIOCGIFADDR, &ifr);
    //printf("ret %d \n",ret);
	if(ret < 0)
	{
		*ip = 0;
	}
	else
	{
		addr = (struct sockaddr_in*)&ifr.ifr_addr;
		*ip = addr->sin_addr.s_addr;
	}
    close(s);
#else
	*ip = ADDR_ANY;
#endif
}

u16 SetRtpLocalAddr(ifly_rtp_t *pRtp,u32 dwIp, u16 wPort, BOOL bRcv)
{
	u32 localip = 0; //08-03-08 chenjie for 组播
	
	if(pRtp == NULL || pRtp->m_pSocket == NULL)
	{
		return ERROR_RTP_NO_INIT;
	}
	
	if(0 == wPort)
	{
		return ERROR_SND_CREATE_SOCK;
	}
	
	//the same to last set  
	if(dwIp == pRtp->m_tLocalAddr.m_dwIP &&
		wPort == pRtp->m_tLocalAddr.m_wPort)
	{
		return MEDIASTREAM_NO_ERROR;
	}
	
	GetIp("eth0", &localip);  //08-03-08 chenjie for 组播
	
	//create local socket ,no bind local ip
    if(!SocketCreate(pRtp->m_pSocket, SOCK_DGRAM, wPort, localip/*ADDR_ANY*/, dwIp, bRcv)) //08-03-08 chenjie for 组播
	{
		return ERROR_SND_CREATE_SOCK;
	}
	pRtp->m_tLocalAddr.m_dwIP  = dwIp;
	pRtp->m_tLocalAddr.m_wPort = wPort;
	return MEDIASTREAM_NO_ERROR;
}

u16 RemoveRtpLocalAddr(ifly_rtp_t *pRtp)
{
	if(pRtp == NULL || pRtp->m_pSocket == NULL)
	{
		return ERROR_RTP_NO_INIT;
	}
	
    	SocketDestroy(pRtp->m_pSocket,TRUE);
	
	memset(&pRtp->m_tLocalAddr, 0, sizeof(pRtp->m_tLocalAddr)); 
	
	return MEDIASTREAM_NO_ERROR;
}

u16 SetRtpRemoteAddr(ifly_rtp_t *pRtp,TRemoteAddr *pRemoteAddr)
{
	pRtp->m_tRemoteAddr = *pRemoteAddr;
	return MEDIASTREAM_NO_ERROR;
}

//此函数没问题
u16 CheckPackAvailabe(const TRtpPack *ptRtpPack, s32 *pnHeadLen)
{
	*pnHeadLen = RTP_FIXEDHEADER_SIZE;
	
	// SET RTP EXTENCE BITS;
	if(ptRtpPack->m_byExtence == 1)
	{
		//长度大小内部控制，不会出现异常			
		if( (ptRtpPack->m_nExSize < 0) || 
			(ptRtpPack->m_nExSize * sizeof(u32) > MAX_PACK_EX_LEN) )
		{
			return ERROR_LOOP_BUF_SIZE;
		}
		
		*pnHeadLen += sizeof(u32);
		*pnHeadLen += ptRtpPack->m_nExSize * sizeof(u32);
	}
	
	//长度大小内部控制，不会出现异常
	if( (*pnHeadLen < 0) || 
		(ptRtpPack->m_nRealSize < 0) || 
		((ptRtpPack->m_nRealSize+*pnHeadLen) > (MAX_PACK_SIZE+RTP_FIXEDHEADER_SIZE)) )
	{
		return ERROR_LOOP_BUF_SIZE;
	}
	
	return MEDIASTREAM_NO_ERROR;
}

u16 DirectSendRtpPack(ifly_rtp_t *pRtp, u8 *pPackBuf, s32 nPackLen, u32 dwTimeStamp)
{
	u16 wRet = MEDIASTREAM_NO_ERROR;
	
	s32 nSndNum = 0;
	
	s32 i = 0;
	
	pRtp->m_dwTotalPackNum++;
	
	//DIRECT SEND
	for(i=0; i<pRtp->m_tRemoteAddr.m_byNum; i++)
	{
		//发送端模拟丢包
		if( (g_nDiscardSpan > 0) && ((pRtp->m_dwTotalPackNum%g_nDiscardSpan) == 0) )
		{
			
		}
		else
		{
			nSndNum = SocketSendTo(pRtp->m_pSocket, pPackBuf, nPackLen,
				pRtp->m_tRemoteAddr.m_tAddr[i].m_dwIP, 
				pRtp->m_tRemoteAddr.m_tAddr[i].m_wPort);
			if(nSndNum <= 0)
			{
				if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
				{
					printf("[Rtp::DirectSend] Direct Send Error\n");
				}
				return ERROR_SND_CREATE_SOCK;
			}
		}
		
		//update RTCP Staus
		wRet = UpdateRtcpSend(pRtp->m_pRtcp, nPackLen, dwTimeStamp);
		if(IFLYFAILED(wRet))
		{
			return wRet;
		}
	}
	
	return wRet;
}

u16 SaveRtpPackIntoLPBuf(ifly_rtp_t *pRtp, u8 *pPackBuf, s32 nPackLen, BOOL32 bTrans)
{
	if(FALSE == bTrans)
	{
		//printf("WriteRtpPackIntoRLB 1\n");
		//写入重发环形缓冲
		WriteRtpPackIntoRLB(pRtp, pPackBuf, nPackLen);
		//printf("WriteRtpPackIntoRLB 2\n");
	}
	if(NULL != pRtp->m_pLoopBuf)
	{
		//写入待发环形缓冲
		u16 wRet = WriteLoopBuf(pRtp->m_pLoopBuf, pPackBuf, nPackLen);
		if(LOOPBUF_NO_ERROR != wRet)
		{
			printf("WriteLoopBuf error,wRet=%d\n",wRet);
			return ERROR_LOOPBUF_FULL;
		}
	}
	else
	{
		return ERROR_LOOPBUF_FULL;
	}
	
	return MEDIASTREAM_NO_ERROR;
}

u16 WriteRtpPackIntoRLB(ifly_rtp_t *pRtp, u8 *pPacketBuf, s32 nPacketSize)
{
	u16 wRet = LOOPBUF_NO_ERROR;
	
	if(pRtp == NULL || pRtp->m_hSynSem == NULL)
	{
		return ERROR_CREATE_SEMAPORE;
	}
	
	if(nPacketSize < RTP_FIXEDHEADER_SIZE || 
		nPacketSize > (pRtp->m_nRLBUnitSize-(s32)sizeof(s32)) || 
		NULL == pPacketBuf)
	{
		return ERROR_LOOP_BUF_PARAM;
	}
	
	SemTake(pRtp->m_hSynSem);
	
	if(FALSE == pRtp->m_bRepeatSend || NULL == pRtp->m_pRLoopBuf || 0 == g_nRepeatSnd)
	{
		pRtp->m_bIsRLBFull		= FALSE;
		pRtp->m_bIsRLBWrited	= FALSE;
		pRtp->m_wRLBMinSN		= 0;
		pRtp->m_wRLBMinPos		= 0;
		pRtp->m_wRLBWritePos	= 0;
		pRtp->m_wRLBUnitNum		= 0;
		SemGive(pRtp->m_hSynSem);
		return ERROR_LOOP_BUF_NOCREATE;
	}
	
	//写入RTP数据包长度
	//*((s32 *)(pRtp->m_pRLoopBuf+pRtp->m_wRLBWritePos*pRtp->m_nRLBUnitSize)) = nPacketSize;
	memcpy(pRtp->m_pRLoopBuf+pRtp->m_wRLBWritePos*pRtp->m_nRLBUnitSize, &nPacketSize, sizeof(s32));
	
	//拷贝RTP数据包
	memcpy(pRtp->m_pRLoopBuf+pRtp->m_wRLBWritePos*pRtp->m_nRLBUnitSize+sizeof(s32), pPacketBuf, nPacketSize);
	
	if(FALSE == pRtp->m_bIsRLBWrited)
	{
		pRtp->m_wRLBMinPos = 0;
		pRtp->m_wRLBMinSN = pRtp->m_wSeqNum;
		pRtp->m_bIsRLBWrited = TRUE;
	}
	//如果缓冲未写满过，则最小包序列不变，否则最低位置的rtp包被覆盖，最小包序列＋1	
	else if(TRUE == pRtp->m_bIsRLBFull)
	{
		pRtp->m_wRLBMinSN++;
		
		pRtp->m_wRLBMinPos = pRtp->m_wRLBWritePos+1;
		if(pRtp->m_wRLBMinPos == pRtp->m_wRLBUnitNum)
		{
			pRtp->m_wRLBMinPos = 0;
		}
	}
	
	//重置写入点位置,如果缓冲写满，则循环写入，覆盖最低位置的rtp包
	pRtp->m_wRLBWritePos++;
	if(pRtp->m_wRLBWritePos == pRtp->m_wRLBUnitNum)
	{
		pRtp->m_wRLBWritePos = 0;
		pRtp->m_bIsRLBFull  = TRUE;
	}
	
	SemGive(pRtp->m_hSynSem);
	
	return wRet;
}

/*=============================================================================
函数名		：ReadRtppackRLBBySN
功能		：根据 SN 从重发环形缓冲中取出需要重发的序列包
算法实现	：（可选项）
引用全局变量：无
输入参数说明：
pBuf [out]         缓冲
nBufSize [out]     缓冲长度
dwTimeStamp[in]    包所在帧的时间戳
wSeqNum[in]        包的包序列
返回值说明：参见错误码定义
=============================================================================*/
u16 ReadRtppackRLBBySN(ifly_rtp_t *pRtp, u8 *pBuf, s32 *pnBufSize, u32 dwTimeStamp, u16 wSeqNum)
{
	u32 dwRLBMaxSN = 0;
	s32 nOffset = -1;
	
	u8 *pPacketBuf = NULL;
	
	u32 *dwHeader = NULL;
	u32 dwOldTimeStamp = 0;
	
	if(pRtp == NULL || pRtp->m_hSynSem == NULL)
	{
		return ERROR_CREATE_SEMAPORE;
	}
	
	if(NULL == pBuf || NULL == pnBufSize)
	{
		return ERROR_LOOP_BUF_PARAM;
	}
	
	SemTake(pRtp->m_hSynSem);
	
	if(FALSE == pRtp->m_bRepeatSend || NULL == pRtp->m_pRLoopBuf || 0 == pRtp->m_wRLBUnitNum || 0 == g_nRepeatSnd)
	{
		SemGive(pRtp->m_hSynSem);
		return ERROR_LOOP_BUF_NOCREATE;
	}
	
	//printf("wSeqNum=%d,dwTimeStamp=%d,MinSN=%d,WritePos=%d,Full=%d,UnitSize=%d,UnitNum=%d\n",wSeqNum,dwTimeStamp,pRtp->m_wRLBMinSN,pRtp->m_wRLBWritePos,pRtp->m_bIsRLBFull,pRtp->m_nRLBUnitSize,pRtp->m_wRLBUnitNum);
	
	//包序列校验
	//获取所要查找的包相距缓冲起始点所在的偏移 (－1 标识没有找到)
	dwRLBMaxSN  = 0;
	nOffset = -1;
	if(FALSE == pRtp->m_bIsRLBFull)
	{
		if(pRtp->m_wRLBWritePos >= 1)
		{
			dwRLBMaxSN = pRtp->m_wRLBMinSN+pRtp->m_wRLBWritePos-1;
		}
		else
		{
			SemGive(pRtp->m_hSynSem);
			return ERROR_LOOP_BUF_NULL;
		}
	}
	else
	{
		dwRLBMaxSN = pRtp->m_wRLBMinSN+pRtp->m_wRLBUnitNum-1;
	}
	if(dwRLBMaxSN > 0xFFFF)
	{
		if(wSeqNum >= pRtp->m_wRLBMinSN)
		{
			nOffset = pRtp->m_wRLBMinPos+wSeqNum-pRtp->m_wRLBMinSN;
		}
		if(wSeqNum <= (dwRLBMaxSN-0xFFFF-1))
		{
			nOffset = pRtp->m_wRLBMinPos+wSeqNum+0xFFFF+1-pRtp->m_wRLBMinSN;
		}
	}
	else
	{
		if(wSeqNum >= pRtp->m_wRLBMinSN && 
			wSeqNum <= dwRLBMaxSN)
		{
			nOffset = pRtp->m_wRLBMinPos+wSeqNum-pRtp->m_wRLBMinSN;
		}		
	}
	if(nOffset >= pRtp->m_wRLBUnitNum)
	{
		nOffset %= pRtp->m_wRLBUnitNum;
	}
	
	//(－1 标识没有找到)
	if(nOffset < 0)
	{
		if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
		{
			printf("[Rtp::ReadRLBBySN] Acquire wSeqNum=%d, Real nSequence=-1\n", wSeqNum);
		}
		SemGive(pRtp->m_hSynSem);
		return ERROR_LOOP_BUF_NULL;
	}
	
	pPacketBuf = pRtp->m_pRLoopBuf+nOffset*pRtp->m_nRLBUnitSize;
	if(NULL == pPacketBuf)
	{
		SemGive(pRtp->m_hSynSem);
		return ERROR_LOOP_BUF_NULL;
	}
	
	//时间戳校验
	dwHeader = (u32*)(pPacketBuf+sizeof(u32));
	
	//dwOldTimeStamp = dwHeader[1];
	memcpy(&dwOldTimeStamp, (u8*)dwHeader+4, sizeof(u32));
	
	//order rs rtcp convert
	ConvertN2H((u8 *)&dwOldTimeStamp, 0, 1);
	
	if(dwOldTimeStamp == dwTimeStamp)
	{
		if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
		{
			//s32 nSequence = dwHeader[0];
			
			s32 nSequence = 0;
			memcpy(&nSequence, (u8*)dwHeader, sizeof(u32));
			
			//order rs rtcp convert
			ConvertN2H((u8 *)&nSequence, 0, 1);
			nSequence = (s32)GetBitField(nSequence, 0, 16);
			
			printf("[Rtp::ReadRLBBySN] nOffset=%d, m_wRLBWritePos=%d, Acquire wSeqNum=%d, Real nSequence=%d\n", nOffset, pRtp->m_wRLBWritePos, wSeqNum, nSequence);
		}
	}
	else
	{
		if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
		{
			printf("[Rtp::ReadRLBBySN] Acquire wSeqNum=%d, Acquire dwTimeStamp=%d, Real dwTimeStamp=-1\n", wSeqNum, dwTimeStamp);
		}
		SemGive(pRtp->m_hSynSem);
		return ERROR_LOOP_BUF_NULL;
	}
	
	//取出RTP数据包长度
	//*pnBufSize = *((s32 *)(pPacketBuf));
	memcpy(pnBufSize, pPacketBuf, sizeof(s32));
    if(*pnBufSize < RTP_FIXEDHEADER_SIZE || 
	   *pnBufSize > (pRtp->m_nRLBUnitSize - (s32)sizeof(s32)))
    {
		SemGive(pRtp->m_hSynSem);
        return ERROR_LOOP_BUF_SIZE;
    }
	
    //取出RTP数据包
	memcpy(pBuf, pPacketBuf+sizeof(s32), *pnBufSize);
	
	SemGive(pRtp->m_hSynSem);
	
	return LOOPBUF_NO_ERROR;
}

/*=============================================================================
	函数名		：FindRtppackRLBByTS
	功能		：根据 TIMESTAMP 从重发环形缓冲中取出需要重发的起始序列包
	算法实现	：（可选项）
	引用全局变量：无
	输入参数说明：
					pBuf [out]         缓冲
					byPackNum [out]    总报数
					dwTimeStamp[in]    包所在帧的时间戳
	返回值说明：-1 没找到，成功返回起始序列包
=============================================================================*/
s32 FindRtppackRLBByTS(ifly_rtp_t *pRtp, u8 *pBuf, u8 *pbyPackNum, u32 dwTimeStamp)
{
	u32 dwRLBMax = 0;
	s32 nOffset = -1;
	u8  *pPacketBuf = NULL;
	
	u32 dwIndex = 0;
	
	u32 *dwHeader = NULL;
	
	u32 dwOldTimeStamp = 0;
	
	if(pRtp == NULL || pRtp->m_hSynSem == NULL)
	{
		return -1;
	}
	
	if(NULL == pBuf || NULL == pbyPackNum)
	{
		return -1;
	}
	
	SemTake(pRtp->m_hSynSem);

	if(FALSE == pRtp->m_bRepeatSend || NULL == pRtp->m_pRLoopBuf || 0 == pRtp->m_wRLBUnitNum || 0 == g_nRepeatSnd)
	{
		SemGive(pRtp->m_hSynSem);
		return -1;
	}

	//包序列查找
	//获取所要查找的包相距缓冲起始点所在的偏移 (－1 标识没有找到)
	dwRLBMax = 0;
	nOffset = -1;
	pPacketBuf = NULL;
	if(FALSE == pRtp->m_bIsRLBFull)
	{
		dwRLBMax = pRtp->m_wRLBWritePos;		
	}
	else
	{
		dwRLBMax = pRtp->m_wRLBUnitNum;
	}
	for(dwIndex=0; dwIndex<dwRLBMax; dwIndex++)
	{
		pPacketBuf = pRtp->m_pRLoopBuf+dwIndex*pRtp->m_nRLBUnitSize;
		if(NULL == pPacketBuf)
		{
			SemGive(pRtp->m_hSynSem);
			return -1;
		}
		
		//时间戳校验
		dwHeader = (u32*)(pPacketBuf+sizeof(u32));
		
		//dwOldTimeStamp = dwHeader[1];
		memcpy(&dwOldTimeStamp, (u8*)dwHeader+4, sizeof(u32));
		
		//order rs rtcp convert
		ConvertN2H((u8 *)&dwOldTimeStamp, 0, 1);
		
		if(dwOldTimeStamp == dwTimeStamp)
		{
			u8  byExtence = 0;
			u8* pExDataBuf = NULL;
			u8* pRealDataBuf = NULL;
			s32 nOffSet = 0;
			
			s32 xStart = 0;
			
			u8 byBitIndex = 0;
			u16 wStartSeq = 0;
			
			u16  wSequence = 0;
			
			//计算该帧的起始包，以及总包数
			///////////////////////////////////////////////////////////
			//1 . 分析RTP包的 时间戳(TS)，SN,  Index:
			
			u32 dwStdHeadArr[3];  // 12 字节
			u32 dwExdHeadArr[4];  // 16 字节
			memset(&dwStdHeadArr, 0, sizeof(dwStdHeadArr));	
			memset(&dwExdHeadArr, 0, sizeof(dwExdHeadArr));	
			
			//标准rtp头信息------
			
			memcpy((u8*)(dwStdHeadArr), (pPacketBuf+sizeof(s32)), sizeof(dwStdHeadArr));
			//RTP fixed Header Convert

			//order rs rtcp convert
			ConvertN2H((u8*)(dwStdHeadArr), 0, 3);
			wSequence = (u16)GetBitField(dwStdHeadArr[0], 0, 16);
			
			byExtence = 0;
			pExDataBuf = NULL;
			pRealDataBuf = NULL;
			nOffSet = RTP_FIXEDHEADER_SIZE + GetBitField(dwStdHeadArr[0], 24, 4) * sizeof(u32);   
			
			pRealDataBuf = (pPacketBuf+sizeof(s32)) + nOffSet; 
			byExtence    = (u8)GetBitField(dwStdHeadArr[0], 28, 1);
			if(0 == byExtence)//Extension Bit Set
			{
				SemGive(pRtp->m_hSynSem);
				return -1;
			}
			
			//扩展rtp头信息------
			
			xStart = nOffSet/sizeof(u32);
			memcpy((u8*)(dwExdHeadArr), pRealDataBuf, sizeof(dwExdHeadArr));
			ConvertN2H((u8*)(dwExdHeadArr), 0, 1);			
			pExDataBuf = (u8*)(&dwExdHeadArr[1]); 
			
			//提取扩展数据------
			
			//记录总小包数
			*pbyPackNum = (u8)(*(pExDataBuf + EX_TOTALNUM_POS));
			//获取本包索引
			byBitIndex = (u8)(*(pExDataBuf + EX_INDEX_POS));
			//记录起始包序列
			wStartSeq = wSequence - (byBitIndex - 1);
			///////////////////////////////////////////////////////////
			
			if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
			{
				printf("[Rtp::FindRLBByTS] Acquire dwTimeStamp=%d, Real Get wStartSeq==%d\n", dwTimeStamp, wStartSeq);
			}
			
			SemGive(pRtp->m_hSynSem);
			return wStartSeq;
		}
	}
	
	if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
	{
		printf("[Rtp::FindRLBByTS] Acquire dwTimeStamp=%d, Real Get wStartSeq=-1\n", dwTimeStamp);
	}
	
	SemGive(pRtp->m_hSynSem);
	
	return -1;
}

/*=============================================================================
	函数名		：DealRSQBackQuest
	功能		：处理重发请求，从重发环形缓冲中取出请求重发的小包重发出去
	算法实现	：（可选项）
	引用全局变量：无
	输入参数说明：ptRSQ 重发请求结构指针
	返回值说明：参见错误码定义
=============================================================================*/
u16 DealRtpRSQBackQuest(ifly_rtp_t *pRtp, TRtcpSDESRSQ *ptRSQ)
{
	u16 wRet = MEDIASTREAM_NO_ERROR;
	
	u8  byPackNum = 0;
	s32 nRSPackSize = 0;
	
	if(g_nRepeatSnd == 0)
	{
		return MEDIASTREAM_NO_ERROR;
	}
	
	//printf("ptRSQ0=0x%08x\n",(int)ptRSQ);
	//fflush(stdout);
	
	if(pRtp == NULL || FALSE == pRtp->m_bRepeatSend || NULL == pRtp->m_pSocket || 0 == g_nRepeatSnd)
	{
		return ERROR_LOOP_BUF_NOCREATE;
	}
	
	if(SN_RSQ_TYPE == ptRSQ->m_byRSQType)
	{
		BOOL32 bRS = 0;//是否重发
		u32 *pdwMaskBit = NULL;
		
		s32 nSize = 0;
		
		s32 nMuti = 0;
		s32 nResi = 0;
		
		s32 nOffset = 0;
		
		//printf("ptRSQ1=0x%08x\n",(int)ptRSQ);
		//fflush(stdout);
		
		byPackNum = ptRSQ->m_byPackNum;
		
		if(byPackNum >= MAX_PACK_NUM)
		{
			return wRet;
		}
		if(byPackNum == 0)
		{
			byPackNum = MAX_PACK_NUM;
		}
		
		bRS = 0;//是否重发
		pdwMaskBit = (u32*)ptRSQ->m_byMaskBit;
		
		//order rs rtcp convert
		ptRSQ->m_dwTimeStamp = ntohl(ptRSQ->m_dwTimeStamp);
		ptRSQ->m_wStartSeqNum = ntohs(ptRSQ->m_wStartSeqNum);
		nSize = MAX_PACK_NUM / (8*sizeof(u32));
		
		//printf("ConvertN2H 1,pdwMaskBit=0x%08x\n",(int)pdwMaskBit);
		//fflush(stdout);
		
		ConvertN2H( (u8*)pdwMaskBit, 0, nSize);
		
		//printf("ConvertN2H 2\n");
		//fflush(stdout);
		
		nMuti = 0;
		nResi = 0;
		
		//u32 dwTimeStamp = GetSystemTick();
		
		for(nOffset=0; nOffset<byPackNum; nOffset++)
		{
			s32 nSndNum = 0;
			s32 j = 0;
			
			//分析重发请求帧中各包的重发掩码位，00010101 （1－需要重发, 0－不重发）
			nMuti = nOffset/32;
			nResi = nOffset%32;
			bRS = (BOOL32)GetBitField(pdwMaskBit[nMuti], nResi, 1);
			if(FALSE == bRS)
			{
				continue;
			}
			
			//printf("ReadRtppackRLBBySN 1-1\n");
			//fflush(stdout);
			
			//read a pack
			if(LOOPBUF_NO_ERROR != ReadRtppackRLBBySN(pRtp, pRtp->m_pRSPackBuf, &nRSPackSize, 
					ptRSQ->m_dwTimeStamp, (u16)(ptRSQ->m_wStartSeqNum+nOffset)))
			{
				//有必要继续发送吗???
				//continue;
				printf("ReadRtppackRLBBySN failed 1\n");
				fflush(stdout);
				break;
			}
			
			//printf("ReadRtppackRLBBySN 1-2\n");
			//fflush(stdout);
			
			nSndNum = 0;
			//send all remote peer
			for(j=0; j<pRtp->m_tRemoteAddr.m_byNum; j++)
			{
				nSndNum = SocketSendTo(pRtp->m_pSocket, pRtp->m_pRSPackBuf, nRSPackSize, 
					                        pRtp->m_tRemoteAddr.m_tAddr[j].m_dwIP, 
					                        pRtp->m_tRemoteAddr.m_tAddr[j].m_wPort);
				
				if(nSndNum <= 0)
				{
					if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
					{
						printf("[Rtp::DealRSQBackQuest] Direct Send Rtp By SN_RSQ_TYPE Error\n");
					}
					return ERROR_SND_CREATE_SOCK;
				}
			}
			
			//printf("[Rtp::ReadRLBBySN]: Spend Time:%d\n", GetSystemTick()-dwTimeStamp);
		}
	}
	if(TIMESTAMP_RSQ_TYPE == ptRSQ->m_byRSQType)
	{
		s32 nRet = 0;
		
		u16 wSequence = 0;
		
		s32 nOffset = 0;
		
		//u32 dwTimeStamp = GetSystemTick();
		
		//printf("FindRtppackRLBByTS 1,ptRSQ2=0x%08x\n",(int)ptRSQ);
		//fflush(stdout);
		
		//order rs rtcp convert
		ptRSQ->m_dwTimeStamp = ntohl(ptRSQ->m_dwTimeStamp);
		
		nRet = FindRtppackRLBByTS(pRtp, pRtp->m_pRSPackBuf, &byPackNum, ptRSQ->m_dwTimeStamp);
		
		//printf("FindRtppackRLBByTS 2\n");
		//fflush(stdout);
		
		if(nRet < 0 || nRet > 0xFFFF)
		{
			return wRet;
		}
		if(byPackNum >= MAX_PACK_NUM)
		{
			return wRet;
		}
		if(byPackNum == 0)
		{
			byPackNum = MAX_PACK_NUM;
		}
		
		wSequence = nRet;
		
		for(nOffset=0; nOffset<byPackNum; nOffset++)
		{
			s32 nSndNum = 0;
			s32 j = 0;
			
			//分析重发请求帧中时间戳，检索并发出相应数据包
			
			//printf("ReadRtppackRLBBySN 2-1\n");
			//fflush(stdout);
			
			//read a pack
			if(LOOPBUF_NO_ERROR != ReadRtppackRLBBySN(pRtp, pRtp->m_pRSPackBuf, &nRSPackSize, 
										ptRSQ->m_dwTimeStamp, wSequence))
			{
				//有必要继续发送吗???
				//continue;
				printf("ReadRtppackRLBBySN failed 2\n");
				fflush(stdout);
				break;
			}
			
			//printf("ReadRtppackRLBBySN 2-2\n");
			//fflush(stdout);
			
			nSndNum = 0;
			//send all remote peer 
			for(j=0; j<pRtp->m_tRemoteAddr.m_byNum; j++)
			{
				nSndNum = SocketSendTo(pRtp->m_pSocket, pRtp->m_pRSPackBuf, nRSPackSize, 
					                        pRtp->m_tRemoteAddr.m_tAddr[j].m_dwIP, 
					                        pRtp->m_tRemoteAddr.m_tAddr[j].m_wPort);
				
				if(nSndNum <= 0)
				{
					if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
					{
						printf("[Rtp::DealRSQBackQuest] Direct Send By Rtp TIMESTAMP_RSQ_TYPE Error\n");
					}
					return ERROR_SND_CREATE_SOCK;
				}
			}
			
			wSequence++;
			
			//printf("[FindRLBByTS and ReadRLBBySN]: Spend Time:%d\n", GetSystemTick()-dwTimeStamp);
		}
	}
	
	return wRet;
}

u16 WriteRtpPack(ifly_rtp_t *pRtp, TRtpPack *pRtpPack, BOOL bSend)
{
	u16 wRet = MEDIASTREAM_NO_ERROR;
	s32 nHeadLen = 0;
	
	s32 nMove = 0;
	u32 dwExHeader = 0;
//	s32 i = 0;
	u32 *pdwFixedHeader = NULL;
	u32 *pdwExHeader = NULL;
	
	if(pRtp == NULL || pRtp->m_dwSSRC == 0)
	{
		return ERROR_RTP_NO_INIT;
	}
	
	wRet = CheckPackAvailabe(pRtpPack, &nHeadLen);
	if(IFLYFAILED(wRet))
	{
		printf("rtp packet is not valid\n");
		return wRet;
	}
	
	//判断是否有预留空间.
	//if(pRtpPack->m_nPreBufSize < nHeadLen)
    if(pRtpPack->m_nPreBufSize < (INT32)(RTP_FIXEDHEADER_SIZE + EX_HEADER_SIZE + 
		                        pRtpPack->m_nExSize * sizeof(u32)))
	{
		// SET RTP FIXED HEADER
		pRtp->m_dwRtpHeader[0] = 0; 
		pRtp->m_dwRtpHeader[0] = SetBitField(pRtp->m_dwRtpHeader[0], 2, 30, 2);//version bit
		pRtp->m_dwRtpHeader[0] = SetBitField(pRtp->m_dwRtpHeader[0], pRtpPack->m_byExtence, 28, 1);//X bit
		pRtp->m_dwRtpHeader[0] = SetBitField(pRtp->m_dwRtpHeader[0], pRtpPack->m_byMark, 23, 1);//mark bit
		pRtp->m_dwRtpHeader[0] = SetBitField(pRtp->m_dwRtpHeader[0], pRtpPack->m_byPayload, 16, 7);//payload
		
		pRtp->m_wSeqNum++;
		/*if(pRtp->m_wSeqNum % 200 == 0)
		{
			printf("hehe1,m_wSeqNum=%d,m_dwTimeStamp=0x%08x\n",pRtp->m_wSeqNum,pRtpPack->m_dwTimeStamp);
		}*/
		
		pRtp->m_dwRtpHeader[0] = SetBitField(pRtp->m_dwRtpHeader[0], pRtp->m_wSeqNum, 0, 16);
		
		pRtp->m_dwRtpHeader[1] = pRtpPack->m_dwTimeStamp;//timestamp bit
		pRtp->m_dwRtpHeader[2] = pRtp->m_dwSSRC;//SSRC bit
		
		// order convert
		pRtp->m_dwRtpHeader[0] = htonl( pRtp->m_dwRtpHeader[0]);
		pRtp->m_dwRtpHeader[1] = htonl( pRtp->m_dwRtpHeader[1]);
		pRtp->m_dwRtpHeader[2] = htonl( pRtp->m_dwRtpHeader[2]);
		
		memcpy(pRtp->m_pPackBuf,pRtp->m_dwRtpHeader,sizeof(pRtp->m_dwRtpHeader));
		nMove  += sizeof(pRtp->m_dwRtpHeader);
		
		// SET RTP EXTENCE BITS;
		if(pRtpPack->m_byExtence == 1)
		{
			dwExHeader = 0;
			
			//set extence Header bit
			dwExHeader = SetBitField(dwExHeader,pRtpPack->m_nExSize, 0, 16);
			dwExHeader = htonl(dwExHeader);
			memcpy(pRtp->m_pPackBuf + nMove, &dwExHeader, sizeof(u32));
			nMove     += sizeof(u32);
			
            memcpy(pRtp->m_pPackBuf + nMove, pRtpPack->m_pExData, pRtpPack->m_nExSize * sizeof(u32));
			nMove     += pRtpPack->m_nExSize * sizeof(u32);
		}
		
        //copy real data memory 
		memcpy(pRtp->m_pPackBuf + nMove, pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
       	
#if 0
		if(bSend)
		{
			s32 nSndNum = 0;
			//DIRECT SEND
			for(i=0; i<pRtp->m_tRemoteAddr.m_byNum; i++)
			{
				nSndNum = SocketSendTo(pRtp->m_pSocket, pRtp->m_pPackBuf, nMove + pRtpPack->m_nRealSize,
									pRtp->m_tRemoteAddr.m_tAddr[i].m_dwIP, 
									pRtp->m_tRemoteAddr.m_tAddr[i].m_wPort);
				
				//update RTCP Staus
				wRet = UpdateRtcpSend(pRtp->m_pRtcp, nMove + pRtpPack->m_nRealSize, pRtpPack->m_dwTimeStamp);
				if(KDVFAILED(wRet))
				{
					return wRet;
				}
			}
		}
		else
		{
			//写入重发环形缓冲
			WriteRtpPackIntoRLB(pRtp, pRtp->m_pPackBuf, nMove + pRtpPack->m_nRealSize);
			
			//WRITE LOOPBUF
			if(LOOPBUF_NO_ERROR != WriteLoopBuf(pRtp->m_pLoopBuf, pRtp->m_pPackBuf, nMove + pRtpPack->m_nRealSize))
			{
				return ERROR_LOOPBUF_FULL;
			}
		}
		
		return MEDIASTREAM_NO_ERROR;
#else
		if(bSend)//csp modify 2008-11-20
		{
			//printf("DirectSendRtpPack 1\n");
			wRet = DirectSendRtpPack(pRtp, pRtp->m_pPackBuf, nMove + pRtpPack->m_nRealSize, pRtpPack->m_dwTimeStamp);
			//printf("DirectSendRtpPack 1\n");
		}
		else
		{
			//printf("SaveRtpPackIntoLPBuf 1\n");
			wRet = SaveRtpPackIntoLPBuf(pRtp, pRtp->m_pPackBuf, nMove + pRtpPack->m_nRealSize, FALSE);
			//printf("SaveRtpPackIntoLPBuf 1\n");
		}
		
		return wRet;
#endif
	}
	
    //have space to save，reduce memory copy;
	pdwFixedHeader = NULL;
	if(pRtpPack->m_byExtence == 1)
	{
		pdwFixedHeader = (u32 *)(pRtpPack->m_pRealData -
			                 RTP_FIXEDHEADER_SIZE - EX_HEADER_SIZE - 
							 pRtpPack->m_nExSize * sizeof(u32)); 
	}
	else
	{
		pdwFixedHeader = (u32 *)(pRtpPack->m_pRealData - RTP_FIXEDHEADER_SIZE); 
	}
	
	pdwFixedHeader[0] = 0; 
	pdwFixedHeader[0] = SetBitField(pdwFixedHeader[0], 2, 30, 2);//version bit	
	pdwFixedHeader[0] = SetBitField(pdwFixedHeader[0], pRtpPack->m_byExtence, 28, 1);//X bit
	pdwFixedHeader[0] = SetBitField(pdwFixedHeader[0], pRtpPack->m_byMark, 23, 1);//marker bit
	pdwFixedHeader[0] = SetBitField(pdwFixedHeader[0], pRtpPack->m_byPayload, 16, 7);//payload bit
	
	pRtp->m_wSeqNum++;
	/*if(pRtp->m_wSeqNum % 200 == 0)
	{
		printf("hehe2,m_wSeqNum=%d,m_dwTimeStamp=0x%08x\n",pRtp->m_wSeqNum,pRtpPack->m_dwTimeStamp);
	}*/
	
	pdwFixedHeader[0] = SetBitField(pdwFixedHeader[0], pRtp->m_wSeqNum, 0, 16);
	
	pdwFixedHeader[1] = pRtpPack->m_dwTimeStamp; //timestamp bit
	pdwFixedHeader[2] = pRtp->m_dwSSRC;          //SSRC bit
	
	//order convert
	pdwFixedHeader[0] = htonl( pdwFixedHeader[0] );
	pdwFixedHeader[1] = htonl( pdwFixedHeader[1] );
	pdwFixedHeader[2] = htonl( pdwFixedHeader[2] );
	nMove  += RTP_FIXEDHEADER_SIZE;
	
	// SET RTP EXTENCE BITS;
	if(pRtpPack->m_byExtence == 1)
	{
		pdwExHeader = pdwFixedHeader + nMove/sizeof(u32);
		
		// set extence header bit
		*pdwExHeader = SetBitField(*pdwExHeader,pRtpPack->m_nExSize, 0, 16);
		*pdwExHeader = htonl(*pdwExHeader);
		nMove     += sizeof(u32);
		
		memcpy(((u8 *)pdwFixedHeader) + nMove, pRtpPack->m_pExData, pRtpPack->m_nExSize * sizeof(u32));//copy extence bit
		nMove     += pRtpPack->m_nExSize * sizeof(u32);
	}
	
#if 0
	if(bSend)//csp modify 2008-11-20
	{
		//direct send 
		for(i=0; i<pRtp->m_tRemoteAddr.m_byNum; i++)
		{
			SocketSendTo(pRtp->m_pSocket, (u8 *)pdwFixedHeader, 
								nMove + pRtpPack->m_nRealSize,
								pRtp->m_tRemoteAddr.m_tAddr[i].m_dwIP, 
								pRtp->m_tRemoteAddr.m_tAddr[i].m_wPort);
			
			//update RTCP Staus
			wRet = UpdateRtcpSend(pRtp->m_pRtcp, nMove + pRtpPack->m_nRealSize, 
				pRtpPack->m_dwTimeStamp);
			if(KDVFAILED(wRet))
			{
				return wRet;
			}
		}
	}
	else
	{
		//写入重发环形缓冲
		WriteRtpPackIntoRLB(pRtp, (u8 *)pdwFixedHeader, nMove + pRtpPack->m_nRealSize);
		
		//write loopbuf
		if(LOOPBUF_NO_ERROR != WriteLoopBuf(pRtp->m_pLoopBuf,(u8 *)pdwFixedHeader,
							   nMove + pRtpPack->m_nRealSize))
		{
			return ERROR_LOOPBUF_FULL;
		}
	}
	
	return MEDIASTREAM_NO_ERROR;
#else
	if(bSend)
	{
		wRet = DirectSendRtpPack(pRtp, (u8 *)pdwFixedHeader, nMove + pRtpPack->m_nRealSize, pRtpPack->m_dwTimeStamp);
	}
	else
	{   
		wRet = SaveRtpPackIntoLPBuf(pRtp, (u8 *)pdwFixedHeader, nMove + pRtpPack->m_nRealSize, FALSE);
	}
	
	return wRet;
#endif
}

u16 SendRtpPack(ifly_rtp_t *pRtp,INT32 nPackNum,INT32 *pnRealPackNum)
{
#if 1//csp modify 2008-11-20
	u16 wRet = MEDIASTREAM_NO_ERROR;
	
	INT32 nPackSize = 0;
	INT32 i=0;
	INT32 j=0;
	u32 dwTimeStamp;
	
	s32 nSndNum = 0;
	
	//csp add 2008-11-28
	if(pnRealPackNum)
	{
		*pnRealPackNum = 0;
	}
	
	//csp add 2008-11-28
	if(nPackNum <= 0)
	{
		return MEDIASTREAM_NO_ERROR;
	}
	
	if(pRtp == NULL || pRtp->m_pLoopBuf == NULL || pRtp->m_pSocket == NULL)
	{
		return ERROR_RTP_NO_INIT;
	}
	
	for(i=0; i<nPackNum; i++)
	{
		//read a pack
        if(LOOPBUF_NO_ERROR != ReadLoopBuf(pRtp->m_pLoopBuf,pRtp->m_pPackBuf,&nPackSize))
		{
			return MEDIASTREAM_NO_ERROR;
		}
		
		//csp add 2008-11-28
		if(pnRealPackNum)
		{
			*pnRealPackNum++;
		}
		
		pRtp->m_dwTotalPackNum++;
		
		//发送端模拟丢包
		if(g_nDiscardSpan > 0 && 
			(pRtp->m_dwTotalPackNum%g_nDiscardSpan) == 0)
		{
			continue;
		}
		
		nSndNum = 0;
		//send all remote peer 
		for(j=0; j<pRtp->m_tRemoteAddr.m_byNum; j++)
		{
			nSndNum = SocketSendTo(pRtp->m_pSocket, pRtp->m_pPackBuf, nPackSize, 
				pRtp->m_tRemoteAddr.m_tAddr[j].m_dwIP, 
				pRtp->m_tRemoteAddr.m_tAddr[j].m_wPort);
			
			if(nSndNum <= 0)
			{
				if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
				{
					printf("[Rtp::Send] Write And Then Send Rtp Error\n");
				}
				return ERROR_SND_CREATE_SOCK;
			}
			
			//update RTCP Staus
			//dwTimeStamp	= *((u32 *)(pRtp->m_pPackBuf + sizeof(u32)));
			memcpy(&dwTimeStamp,pRtp->m_pPackBuf + sizeof(u32),sizeof(u32));
			dwTimeStamp = ntohl(dwTimeStamp);
			wRet = UpdateRtcpSend(pRtp->m_pRtcp, nPackSize, dwTimeStamp);
			if(IFLYFAILED(wRet))
			{
				return wRet;
			}
		}
	}
#endif
	
	return MEDIASTREAM_NO_ERROR;
}

void ResetRtpSeq(ifly_rtp_t *pRtp)
{
	/*//65535 -> 1
	if(0xFFFF == pRtp->m_wSeqNum)
	{
		pRtp->m_wSeqNum = 1;
	}
	else
	{
		pRtp->m_wSeqNum++;
	}*/
	//65535 -> 0
	pRtp->m_wSeqNum++;
}

void ResetRtpSSRC(ifly_rtp_t *pRtp,u32 dwSSRC)
{
	pRtp->m_dwSSRC = dwSSRC;
}

u16 ResetRtpRSFlag(ifly_rtp_t *pRtp, u16 wRLBUnitNum, BOOL32 bRepeatSnd)
{
	//重传缓冲是否需要重新分配
	BOOL32 bAlloc = FALSE;
	
	s32 m_nBufLen = 0;
	
	if(pRtp == NULL || pRtp->m_hSynSem == NULL)
	{
		return ERROR_CREATE_SEMAPORE;
	}
	
	SemTake(pRtp->m_hSynSem);
	
	if(FALSE == bRepeatSnd)
	{
		//SAFE_DELETE(pRtp->m_pRLoopBuf);
		//pRtp->m_wRLBUnitNum = 0;
		
		/*if(pRtp->m_pRLoopBuf)
		{
			free(pRtp->m_pRLoopBuf);
			pRtp->m_pRLoopBuf = NULL;
		}
		pRtp->m_wRLBUnitNum = 0;*/
	}
	else
	{
		if(NULL == pRtp->m_pRLoopBuf)
		{
			bAlloc = TRUE;
		}
		else if(wRLBUnitNum != pRtp->m_wRLBUnitNum)
		{
			//SAFE_DELETE(m_pRLoopBuf);
			
			if(pRtp->m_pRLoopBuf)
			{
				free(pRtp->m_pRLoopBuf);
				pRtp->m_pRLoopBuf = NULL;
			}
			
			pRtp->m_wRLBUnitNum  = 0; 
			bAlloc = TRUE;
		}
	}
	
	if(TRUE == bAlloc)
	{
		pRtp->m_wRLBUnitNum = wRLBUnitNum;
		
		//创建重发环状缓冲
		m_nBufLen = pRtp->m_nRLBUnitSize * pRtp->m_wRLBUnitNum;
		pRtp->m_pRLoopBuf = (u8 *)malloc(m_nBufLen);
		if(NULL == pRtp->m_pRLoopBuf)
		{
			pRtp->m_bIsRLBFull   = FALSE;
			pRtp->m_bIsRLBWrited = FALSE;
			pRtp->m_wRLBUnitNum  = 0; 
			pRtp->m_wRLBMinSN    = 0; 
			pRtp->m_wRLBMinPos   = 0; 
			pRtp->m_wRLBWritePos = 0;
			SemGive(pRtp->m_hSynSem);
			return ERROR_SND_MEMORY;
		}
		pRtp->m_pRSPackBuf = malloc(MAX_PACK_SIZE+RTP_FIXEDHEADER_SIZE);
		if(NULL == pRtp->m_pRSPackBuf)
		{
			//SAFE_DELETE(m_pRLoopBuf);
			
			if(pRtp->m_pRLoopBuf)
			{
				free(pRtp->m_pRLoopBuf);
				pRtp->m_pRLoopBuf = NULL;
			}
			
			pRtp->m_wRLBUnitNum  = 0; 
			pRtp->m_bIsRLBFull   = FALSE;
			pRtp->m_bIsRLBWrited = FALSE;
			pRtp->m_wRLBMinSN    = 0; 
			pRtp->m_wRLBMinPos   = 0; 
			pRtp->m_wRLBWritePos = 0;
			SemGive(pRtp->m_hSynSem);
			return ERROR_SND_MEMORY;
		}
	}
	
	pRtp->m_bRepeatSend  = bRepeatSnd;
	pRtp->m_bIsRLBFull   = FALSE;
	pRtp->m_bIsRLBWrited = FALSE;
	pRtp->m_wRLBMinSN    = 0; 
	pRtp->m_wRLBMinPos   = 0; 
	pRtp->m_wRLBWritePos = 0;
	
	SemGive(pRtp->m_hSynSem);
	
	return MEDIASTREAM_NO_ERROR;
}

u16 SetRtpRtcp(ifly_rtp_t *pRtp, ms_rtcp_t *pRtcp)
{
	pRtp->m_pRtcp = pRtcp;
	return MEDIASTREAM_NO_ERROR;
}
