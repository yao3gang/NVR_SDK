#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif /* __cplusplus */
#endif /* __cplusplus */

#include "msrtcp.h"

PSETVIDEORATEUP m_pSetVideoRateUpCB = NULL;
BOOL m_bNeedRTCP = FALSE;
BOOL m_bThreadRun = FALSE;
ifly_rtcp_t* CreateRTCPRcvSnd()
{
	INT32 nReuseAddr;
	ifly_rtcp_t *pRcvSnd;

	printf("CreateRTCPRcvSnd\n");

	pRcvSnd = (ifly_rtcp_t *)malloc(sizeof(ifly_rtcp_t));
	if(pRcvSnd == NULL)
	{
		printf("CreateRTCPRcvSnd error: pRcvSnd\n");
		return NULL;
	}

	//printf("CreateRTCPRcvSnd hehe1\n");
	
	pRcvSnd->m_hSocketRcv = OpenSocket();
	if(pRcvSnd->m_hSocketRcv == NULL)
	{
		printf("CreateRTCPRcvSnd error: pRcvSnd->m_hSocketRcv\n");
		return NULL;
	}	
	else 
	{
		pRcvSnd->m_hSocketRcv->m_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	}

	nReuseAddr = 1;
	if( SOCKET_ERROR  == setsockopt(pRcvSnd->m_hSocketRcv->m_hSocket, SOL_SOCKET, SO_REUSEADDR, 
		(char *)&nReuseAddr, sizeof(nReuseAddr)) )
	{
		PrintSocketErrMsg(pRcvSnd->m_hSocketRcv,"Socket setsockopt SO_REUSEADDR Error",TRUE);
		#ifdef WIN32
		closesocket(pRcvSnd->m_hSocketRcv->m_hSocket);
		#else
		close(pRcvSnd->m_hSocketRcv->m_hSocket);
		#endif
		return NULL;
	}
	else
	{
		printf("Socket setsockopt SO_REUSEADDR success\n");
	}
	
	pRcvSnd->m_hSocketSnd = OpenSocket();
	if(pRcvSnd->m_hSocketSnd == NULL)
	{
		printf("CreateRTCPRcvSnd error: pRcvSnd->m_hSocketSnd\n");
		return NULL;
	}
	else 
	{
		pRcvSnd->m_hSocketSnd->m_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}

	nReuseAddr = 1;
	if( SOCKET_ERROR  == setsockopt(pRcvSnd->m_hSocketSnd->m_hSocket, SOL_SOCKET, SO_REUSEADDR, 
		(char *)&nReuseAddr, sizeof(nReuseAddr)) )
	{
		PrintSocketErrMsg(pRcvSnd->m_hSocketSnd,"Socket setsockopt SO_REUSEADDR Error",TRUE);
		#ifdef WIN32
		closesocket(pRcvSnd->m_hSocketSnd->m_hSocket);
		#else
		close(pRcvSnd->m_hSocketSnd->m_hSocket);
		#endif
		return NULL;
	}
	else
	{
		printf("Socket setsockopt SO_REUSEADDR success\n");
	}
	m_bThreadRun = TRUE;
	printf("CreateRTCPRcvSnd ok!\n");
	return pRcvSnd;
}

u16 SetRTCPNetParam(ifly_rtcp_t* pRTCP,TNetSndParam* tNetSndParam, TNetRcvParam* tNetRcvParam)
{
	SOCKADDR_IN addr;
	
	printf("SetRTCPNetParam\n");
//	printf("sizeof(TNetSndParam) = %d\n", sizeof(TNetSndParam));
	
	if (!pRTCP)
	{
		printf("SetRTCPNetParam error: pRTCP\n");
		return -1;
	}
	
	{
		//接收
		printf("SetRTCPNetParam: tNetSndParam\n");
		
		//绑定socket
		memset(&addr, 0, sizeof(SOCKADDR_IN));
		addr.sin_family      = AF_INET; 
		addr.sin_addr.s_addr = tNetRcvParam->m_tLocalNet.m_dwRTCPAddr;//inet_addr("192.168.1.20");
		addr.sin_port        = htons(tNetRcvParam->m_tLocalNet.m_wRTCPPort);
		printf("SetRTCPNetParam: snd bind\n");
		if( SOCKET_ERROR == bind(pRTCP->m_hSocketRcv->m_hSocket,(struct sockaddr *)&addr,sizeof(SOCKADDR_IN)) )
		{
			//printf("SetRTCPNetParam error: snd bind error = %d\n",WSAGetLastError());
			printf("socket=0x%x, ip=0x%x, port=0x%x\n", pRTCP->m_hSocketRcv->m_hSocket, addr.sin_addr.s_addr, addr.sin_port);
			PrintSocketErrMsg(pRTCP->m_hSocketRcv,"Socket bind Error",TRUE);
			CloseSocket(pRTCP->m_hSocketRcv);
			return -1;
		}
		
		//发送
		memset(&addr, 0, sizeof(SOCKADDR_IN));
		addr.sin_family      = AF_INET; 
		addr.sin_addr.s_addr = (tNetSndParam->m_tLocalNet.m_dwRTCPAddr);
		addr.sin_port        = htons(tNetSndParam->m_tLocalNet.m_wRTCPPort);
		printf("SetRTCPNetParam: snd bind2\n");
		if( SOCKET_ERROR == bind(pRTCP->m_hSocketSnd->m_hSocket,(struct sockaddr *)&addr,sizeof(SOCKADDR_IN)) )
		{
			printf("SetRTCPNetParam error: snd bind2\n");
			//PrintSocketErrMsg(pSocket,"Socket bind Error",TRUE);
			CloseSocket(pRTCP->m_hSocketRcv);
			return -1;
		}
		
		pRTCP->m_tLocalAddr.m_dwIP = tNetRcvParam->m_tLocalNet.m_dwRTCPAddr;
		pRTCP->m_tLocalAddr.m_wPort = tNetRcvParam->m_tLocalNet.m_wRTCPPort;
		pRTCP->m_tRemoteAddr.m_tAddr[0].m_dwIP = tNetSndParam->m_tRemoteNet[0].m_dwRTCPAddr;
		pRTCP->m_tRemoteAddr.m_tAddr[0].m_wPort = tNetSndParam->m_tRemoteNet[0].m_wRTCPPort;
	}
	
	printf("SetRTCPNetParam ok\n");
	return MEDIASTREAM_NO_ERROR;
}

u16 SendRTCPSR(ifly_rtcp_t* pRTCP,ifly_netsnd_t* pSnd)
{
	RTCP_PACKET RTCPPack;
	int nRet=0;
	
	if (!pRTCP || !pSnd)
	{
		return 0;
	}
	
	if (!m_bNeedRTCP)
	{
		return 0;
	}
	
	//printf("SendRTCPSR\n");
	
	memset(&RTCPPack, 0 , sizeof(RTCP_PACKET));
	RTCPPack.CommonHead.ucPT = RTP_RTCP_SR;
	RTCPPack.CommonHead.uwLen = sizeof(RTCP_COMMON_HEAD);
	RTCPPack.CommonHead.bit5Count = 1;
	RTCPPack.CommonHead.bit2V = 2;
	RTCPPack.CommonHead.bit1P = 0;

	RTCPPack.RtcpType.SR.udwSentRtpNum = pSnd->m_tSndStatistics.m_dwPackSendNum;
	nRet = SocketSendTo(pRTCP->m_hSocketSnd, (u8*)&RTCPPack, sizeof(RTCP_COMMON_HEAD)+sizeof(SEND_REPORT), 
					(u32)pRTCP->m_tRemoteAddr.m_tAddr[0].m_dwIP,
					(u16)pRTCP->m_tRemoteAddr.m_tAddr[0].m_wPort);
	
	//printf("SendRTCPSR, nRet = %d\n", nRet);
	
	return nRet;
}

u16 SendRTCPRR(ifly_rtcp_t* pRTCP,ifly_netrcv_t *pRcv, BYTE bylostrate)
{
	RTCP_PACKET RTCPPack;
	int nRet=0;
	
	if (!pRTCP || !pRcv)
	{
		return -1;
	}
	
	if (!m_bNeedRTCP)
	{
		return 0;
	}
	
	printf("SendRTCPRR\n");
	
	memset(&RTCPPack, 0 , sizeof(RTCP_PACKET));
	RTCPPack.CommonHead.ucPT = RTP_RTCP_RR;
	RTCPPack.CommonHead.uwLen = sizeof(RTCP_COMMON_HEAD);
	RTCPPack.CommonHead.bit5Count = 1;
	RTCPPack.CommonHead.bit2V = 2;
	RTCPPack.CommonHead.bit1P = 0;
	
	RTCPPack.RtcpType.RR.RrList[0].bit24LostNum = pRcv->m_tRcvStatistics.m_dwPackLose;
	RTCPPack.RtcpType.RR.RrList[0].bit8Fraction = bylostrate;
	
	nRet = SocketSendTo(pRTCP->m_hSocketSnd, (u8*)&RTCPPack, sizeof(RTCP_COMMON_HEAD)+sizeof(RECEIVE_REPORT), 
						pRTCP->m_tRemoteAddr.m_tAddr[0].m_dwIP,
						pRTCP->m_tRemoteAddr.m_tAddr[0].m_wPort);
	
	printf("SendRTCPRR, nRet = %d\n", nRet);
	
	return nRet;
}

void *VideoRtcpTask(void* param)		//void* param
{
	RTCPParam *RTCPparam = (RTCPParam *)param;
	ifly_rtcp_t* pRTCP = NULL;
	ifly_netsnd_t* pSnd = NULL;
	ifly_netrcv_t *pRcv = NULL;
	
	int rcvlen = 0;
	char rcvbuf[100];
	struct sockaddr fromAddr;
	int fromAddrlen = sizeof(fromAddr);
	RTCP_PACKET tmpSRPack;
	RTCP_PACKET	tmpRRPack;
	BYTE bylostrate;
	int nLostCount = 0;
	int nNoLostCount = 0;
	
/*
	long s;
	SOCKADDR_IN addr;
*/
	
	pRTCP = RTCPparam->pRTCP;
	pSnd = RTCPparam->pSnd;
	pRcv = RTCPparam->pRcv;
	
	//printf("VideoRtcpTask\n");
	//printf("VideoRtcpTask:pRTCP = 0x%x,psnd = 0x%x, pRcv = 0x%x\n", pRTCP, pSnd, pRcv);
	if (!pRTCP)
	{
		//printf("VideoRtcpTask error: pRTCP\n");
		return 0;
	}
	
/*
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family      = AF_INET; 
	addr.sin_addr.s_addr = inet_addr("192.168.1.20");//(tNetSndParam->m_tLocalNet.m_dwRTCPAddr);
	addr.sin_port        = htons(65200);
	
	if( SOCKET_ERROR == bind(s,(struct sockaddr *)&addr,sizeof(SOCKADDR_IN)) )
	{
		printf("bind error ERROR = %d\n",WSAGetLastError());
		//PrintSocketErrMsg(pSocket,"Socket bind Error",TRUE);
		CloseSocket(pRTCP->m_hSocketRcv);
		Sleep(20);
		return;
	}
*/
	
	while (pRTCP)
	{
		if (!m_bThreadRun)
		{
			break;
		}
		if(!m_bNeedRTCP)	
		{
#ifdef WIN32
			Sleep(20);
#else
			usleep(20*1000);
#endif
			continue;
		}
		if (pRcv && !pSnd )
		{
			rcvlen = recvfrom(pRTCP->m_hSocketRcv->m_hSocket, rcvbuf, 100, 0, &fromAddr, &fromAddrlen);
			if (rcvlen == SOCKET_ERROR)
			{
				//printf("VideoRtcpTask error: rcvlen, ERROR = %d\n",WSAGetLastError());
#ifdef WIN32
				Sleep(20);
#else
				usleep(20*1000);
#endif
				continue;
			}
			memcpy(&tmpSRPack, rcvbuf, sizeof(RTCP_COMMON_HEAD)+sizeof(SEND_REPORT));
			if (tmpSRPack.CommonHead.bit1P!=0 || tmpSRPack.CommonHead.bit2V!= 2)
			{
				//printf("VideoRtcpTask error: bit1P bit2V\n");
#ifdef WIN32
				Sleep(20);
#else
				usleep(20*1000);
#endif
				continue;
			}
			
			if (tmpSRPack.CommonHead.ucPT != RTP_RTCP_SR)
			{
				//printf("VideoRtcpTask error: RTP_RTCP_SR\n");
#ifdef WIN32
				Sleep(20);
#else
				usleep(20*1000);
#endif
				continue;
			}
			bylostrate = 255 - ((pRcv->m_tRcvStatistics.m_dwPackNum - pRTCP->m_nLastRcvPackNum)*255)/(tmpSRPack.RtcpType.SR.udwSentRtpNum-pRTCP->m_nLastSndPackNum);
			
			pRTCP->m_nLastSndPackNum = tmpSRPack.RtcpType.SR.udwSentRtpNum;
			pRTCP->m_nLastRcvPackNum = pRcv->m_tRcvStatistics.m_dwPackNum;

			SendRTCPRR(pRTCP, pRcv, bylostrate);
		}
		else if (pSnd && !pRcv )
		{
			//printf("VideoRtcpTask hehe1\n");
			struct timeval tv;
			int  ret;
			fd_set rtcp_fds;
			
    		FD_ZERO(&rtcp_fds);
    		FD_SET(pRTCP->m_hSocketRcv->m_hSocket,&rtcp_fds);
    		tv.tv_sec  = 2;
   			tv.tv_usec = 0;
			ret = select(pRTCP->m_hSocketRcv->m_hSocket + 1, &rtcp_fds, NULL, NULL, &tv);
			if(ret < 0)
			{
				break;
			}
			else if(ret == 0)
			{
				continue;
			}
			else
			{
				if(FD_ISSET(pRTCP->m_hSocketRcv->m_hSocket,&rtcp_fds))
				{
					rcvlen = recvfrom(pRTCP->m_hSocketRcv->m_hSocket, rcvbuf, 100, 0, &fromAddr, &fromAddrlen);
					//printf("VideoRtcpTask hehe2\n");
					if (rcvlen  == SOCKET_ERROR)
					{
#ifdef WIN32
						Sleep(20);
#else
						usleep(20*1000);
#endif
						continue;
					}
					memcpy(&tmpRRPack, rcvbuf, sizeof(RTCP_COMMON_HEAD)+sizeof(SEND_REPORT));
					if (tmpRRPack.CommonHead.bit1P!=0 || tmpRRPack.CommonHead.bit2V!= 2)
					{
						//printf("VideoRtcpTask error: bit1P bit2V\n");
#ifdef WIN32
						Sleep(20);
#else
						usleep(20*1000);
#endif
						continue;
					}
					
					if (tmpRRPack.CommonHead.ucPT != RTP_RTCP_RR)
					{
						//printf("VideoRtcpTask error: RTP_RTCP_RR\n");
#ifdef WIN32
						Sleep(20);
#else
						usleep(20*1000);
#endif
						continue;
					}
					
					//do something to control media rate...
					printf("recived RR :bit24LostNum = %d, bit8Fraction = %d\n",tmpRRPack.RtcpType.RR.RrList[0].bit24LostNum, tmpRRPack.RtcpType.RR.RrList[0].bit8Fraction);
					
					if (tmpRRPack.RtcpType.RR.RrList[0].bit8Fraction >= 5)			//>25
					{
						nLostCount++;
						nNoLostCount = 0;
					}
					else if (tmpRRPack.RtcpType.RR.RrList[0].bit8Fraction == 0)		//<=25
					{
						nNoLostCount++;
						nLostCount = 0;
					}
					if (nLostCount>=1)
					{
						//to do :down video stream rate
						printf("nNoLostCount = %d, nLostCount = %d\n", nNoLostCount, nLostCount);
						if (m_pSetVideoRateUpCB)
						{
							m_pSetVideoRateUpCB(FALSE);//FALSE
						}
						nLostCount = 0;
					}
					if (nNoLostCount >= 2)
					{
						//to do: up video stream rate
						printf("nNoLostCount = %d, nLostCount = %d\n", nNoLostCount, nLostCount);
						if (m_pSetVideoRateUpCB)
						{
							m_pSetVideoRateUpCB(TRUE);
						}
						nNoLostCount = 0;
					}
				}
			}
		}
		else
		{
#ifdef WIN32
			Sleep(20);
#else
			usleep(20*1000);
#endif
		}
	}
	return 0;
}

void CloseRTCPRcvSnd(ifly_rtcp_t* pRTCP)
{
	if (!pRTCP)
	{
		return;
	}
	#ifdef WIN32
	closesocket(pRTCP->m_hSocketSnd->m_hSocket);
	closesocket(pRTCP->m_hSocketRcv->m_hSocket);
	#else
	close(pRTCP->m_hSocketSnd->m_hSocket);
	close(pRTCP->m_hSocketRcv->m_hSocket);
	#endif
	memset(&pRTCP->m_tLocalAddr, 0, sizeof(pRTCP->m_tLocalAddr));
	memset(&pRTCP->m_tRemoteAddr, 0, sizeof(pRTCP->m_tRemoteAddr));
	pRTCP->m_nLastRcvPackNum = 0;
	pRTCP->m_nLastSndPackNum = 0;
	pRTCP = NULL;
	m_bNeedRTCP = FALSE;
	m_bThreadRun = FALSE;
}

void SetNeedRTCP(BOOL bNeed)
{
	m_bNeedRTCP = bNeed;
}

void SetCBsSetVideoRate(PSETVIDEORATEUP pSetVideoRateCB)
{
	m_pSetVideoRateUpCB = pSetVideoRateCB;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
