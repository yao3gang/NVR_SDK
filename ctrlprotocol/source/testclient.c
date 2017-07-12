#include "ctrlprotocol.h"
#include <stdio.h>

#pragma comment( lib, "Ws2_32.lib" )

void FindDeivce(ifly_DeviceInfo_t deviceinfo, void* pContext)
{

	printf("ip: 0x%x\n port: %d\n %s\n%s\n", 
		deviceinfo.deviceIP,
		ntohs(deviceinfo.devicePort),
		deviceinfo.device_name,
		deviceinfo.device_mode
		);
	 

}

typedef struct
{
	SOCKHANDLE	sockfd;
	u8	bStart;
	u32 linkid;
	ifly_TCP_Stream_Req req;
}ifly_stearmrcv_t;

ifly_stearmrcv_t g_rcv[STREAM_LINK_MAXNUM];

#ifndef MAX_FRAME_SIZE
#define MAX_FRAME_SIZE				(INT32)256*1024
#endif

void* netrcvthread(void *pParam)
{
	
	int	 i;
	while(1)
	{

		for (i=0; i<STREAM_LINK_MAXNUM; i++ )
		{
			if((g_rcv[i].sockfd != INVALID_SOCKET) && (g_rcv[i].bStart))
			{
				if (g_rcv[i].req.command == 0) //video monitor
				{
					
					ifly_MediaFRAMEHDR_t hdr;
					
					u32 remian = sizeof(ifly_MediaFRAMEHDR_t);
					u32 recvlen = 0;
					int ret = 0;
					char buf[512] = {0};
					char frmaebuf[MAX_FRAME_SIZE] = {0};
					while(remian > 0)
					{
						
						ret=recv(g_rcv[i].sockfd,buf+recvlen,remian,0);
						if(ret < 0)
						{
							goto over;
						}
						if(ret == 0)
						{
							goto over;
						}
						recvlen += ret;
						remian -= ret;
					}
					
					memcpy(&hdr, buf, sizeof(ifly_MediaFRAMEHDR_t));
					hdr.m_dwDataSize = ntohl(hdr.m_dwDataSize);
					hdr.m_dwFrameID = ntohl(hdr.m_dwFrameID);
					hdr.m_dwTimeStamp = ntohl(hdr.m_dwTimeStamp);
					hdr.m_nVideoHeight = ntohl(hdr.m_nVideoHeight);
					hdr.m_nVideoWidth = ntohl(hdr.m_nVideoWidth);
					
					printf("size=%8d, id = %d, h = %d, w = %d \n", hdr.m_dwDataSize, hdr.m_dwFrameID, hdr.m_nVideoHeight, hdr.m_nVideoWidth);

					remian = hdr.m_dwDataSize;
					recvlen = 0;
					while(remian > 0)
					{
						
						ret=recv(g_rcv[i].sockfd,frmaebuf+recvlen,remian,0);
						if(ret < 0)
						{
							goto over;
						}
						if(ret == 0)
						{
							goto over;
						}
						recvlen += ret;
						remian -= ret;
					}

					if (recvlen == hdr.m_dwDataSize)
					{
						continue;
					}
					
over:
					
					closesocket(g_rcv[i].sockfd);
					g_rcv[i].sockfd = INVALID_SOCKET;
					return 0;		
					
				}
				
			}
			else
			{
// 				Sleep(10);
// 				continue;
			}
		}
	}
}


struct TCP_KEEPALIVE {
    u_long  onoff;
    u_long  keepalivetime;
    u_long  keepaliveinterval;
} ;

#define SIO_KEEPALIVE_VALS   _WSAIOW(IOC_VENDOR,4)

void SetNewStreamConnect(u32 ip, u16 port, ifly_TCP_Stream_Req req)
{
	int i=0;
	for (i=0; i<STREAM_LINK_MAXNUM; i++ )
	{
		if(g_rcv[i].sockfd == INVALID_SOCKET)
		{
			struct sockaddr_in server;
			struct TCP_KEEPALIVE klive;
			struct TCP_KEEPALIVE outKeepAlive = {0,0,0};
			unsigned long ulBytesReturn = 0;

			int retConnect = 0;
			int j = 0;
			int err;
			int ret;
			
			g_rcv[i].sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			
			if (g_rcv[i].sockfd == INVALID_SOCKET)
			{
				return;
			}
			

			server.sin_addr.s_addr=ip;
			server.sin_family=AF_INET;
			server.sin_port=htons(port);
						
			for (j=0; j<3; j++)
			{
				retConnect = connect(g_rcv[i].sockfd,(struct sockaddr*)&server,sizeof(server));
				if (retConnect == 0)
				{
					break;
				}
			}
			
			err = WSAGetLastError();
			
			if(retConnect)
			{
				closesocket(g_rcv[i].sockfd);
				g_rcv[i].sockfd = INVALID_SOCKET;
				return ;
			}
			
			SendTcpConnHead(g_rcv[i].sockfd, 0x2);

			klive.onoff = 1;//启用保活
			klive.keepalivetime = 5000;
			klive.keepaliveinterval = 1000 * 5;//重试间隔为5秒 Resend if No-Reply
			
			
			ret = WSAIoctl(
				g_rcv[i].sockfd,
				SIO_KEEPALIVE_VALS,
				&klive,
				sizeof(klive),
				&outKeepAlive,
				sizeof(outKeepAlive),
				(unsigned long *)&ulBytesReturn,
				0,
				NULL
				);
			
			ret = send(g_rcv[i].sockfd,(char *)&req,sizeof(ifly_TCP_Stream_Req),0);
			if (ret > 0)
			{
				
				u32 remian = sizeof(ifly_TCP_Stream_Ack);
				u32 recvlen = 0;
				int ret = 0;
				char buf[512] = {0};
				char frmaebuf[MAX_FRAME_SIZE] = {0};
				while(remian > 0)
				{
					ret=recv(g_rcv[i].sockfd,buf+recvlen,remian,0);
					if(ret < 0)
					{
						break;
					}
					if(ret == 0)
					{
						break;
					}
					recvlen += ret;
					remian -= ret;
				}

				if (recvlen == sizeof(ifly_TCP_Stream_Ack))
				{
					ifly_TCP_Stream_Ack ack;
					memcpy(&ack, buf,sizeof(ifly_TCP_Stream_Ack));
					ack.ackid = ntohl(ack.ackid);
					ack.errcode = ntohl(ack.errcode);
					if (ack.errcode == 0)
					{
						g_rcv[i].linkid = ack.ackid;
						memcpy(&g_rcv[i].req, &req,sizeof(ifly_TCP_Stream_Req));
						g_rcv[i].bStart =1;
					}
					else
					{
						closesocket(g_rcv[i].sockfd);
						g_rcv[i].sockfd = INVALID_SOCKET;
					}
				}
				else
				{
					closesocket(g_rcv[i].sockfd);
					g_rcv[i].sockfd = INVALID_SOCKET;
				}

			}
			else
			{
				closesocket(g_rcv[i].sockfd);
				g_rcv[i].sockfd = INVALID_SOCKET;
			}

			return;
		}
		
	}
	
}

int main()
{
	CPHandle cph = NULL;
	int i;
	HANDLE hThreadRcv = NULL;
	ifly_TCP_Stream_Req req;
	CPLibInit(0);
	
	SetFindDeviceCB(FindDeivce, 0);
	SearchDevice();

	cph = CPConnect(inet_addr("192.168.1.101"), 6630, 3000, NULL);
	if (!cph)
	{
		printf("connect err\n");
	}

	memset(g_rcv,0,sizeof(g_rcv));
	for (i=0; i<STREAM_LINK_MAXNUM; i++ )
	{
		g_rcv[i].sockfd = INVALID_SOCKET;
	}

	hThreadRcv = CreateThread(NULL,1<<20,(LPTHREAD_START_ROUTINE)netrcvthread,NULL,0,NULL);

	memset(&req, 0, sizeof(req));
	req.command = 0;

	SetNewStreamConnect(inet_addr("192.168.1.101"), 6630, req);
	while(1)
	{
		Sleep(1000);
	}

	if (hThreadRcv)
	{
		CloseHandle(hThreadRcv);
		hThreadRcv =NULL;
	}
	CPLibCleanup(TRUE);
	
	return 0;
}


#if 0
int main()
{
	ifly_monitor_param_t mp;
	//ifly_remote_playback_t rp;
	u32 dwServerIp;
	u16 wServerPort;

	char szHostName[64];
	struct hostent* pHostLocal;
	int  count=0;
	
	u16 wRet;
	DWORD start,end;

	int i;
	
	CPLibInit(0);
	
	gethostname(szHostName, sizeof(szHostName));
	pHostLocal = gethostbyname(szHostName);
	
	for(i=0;;i++)
	{
		count++;
		if(pHostLocal->h_addr_list[i] + pHostLocal->h_length >= pHostLocal->h_name)
		{
			break;
		}
	}

	mp.dwIp = inet_addr(inet_ntoa(*(struct in_addr *)pHostLocal->h_addr_list[0]));//inet_addr("224.1.2.3");
	printf("szHostName=%s,mp.dwIp=0x%08x,ip count=%d,all_addr len=%d,in_addr len=%d\n",szHostName,mp.dwIp,count,pHostLocal->h_length,sizeof(struct in_addr));
	
	dwServerIp = inet_addr("192.168.1.33");
	wServerPort = CTRL_PROTOCOL_SERVERPORT;

#if 0
	for(i=0;i<4;i++)
	{
		mp.byChn = i;
		mp.wPort = 64000+mp.byChn*4;

#if 0
		start = GetTickCount();
		wRet = StartNetMonitor(mp,dwServerIp,wServerPort,20*CTRL_PROTOCOL_CONNECT_DEFAULT);
		end = GetTickCount();
		printf("StartNetMonitor:wRet=%d,start=%d,end=%d,span=%d\n",wRet,start,end,end-start);
#else	
		start = GetTickCount();
		wRet = StopNetMonitor(mp,dwServerIp,wServerPort,CTRL_PROTOCOL_CONNECT_BLOCK);
		end = GetTickCount();
		printf("StopNetMonitor:wRet=%d,start=%d,end=%d,span=%d\n",wRet,start,end,end-start);
#endif

		mp.wPort += 2;

#if 0
		start = GetTickCount();
		wRet = StartAudioNetMonitor(mp,dwServerIp,wServerPort,20*CTRL_PROTOCOL_CONNECT_DEFAULT);
		end = GetTickCount();
		printf("StartNetMonitor:wRet=%d,start=%d,end=%d,span=%d\n",wRet,start,end,end-start);
#else	
		start = GetTickCount();
		wRet = StopAudioNetMonitor(mp,dwServerIp,wServerPort,CTRL_PROTOCOL_CONNECT_BLOCK);
		end = GetTickCount();
		printf("StopNetMonitor:wRet=%d,start=%d,end=%d,span=%d\n",wRet,start,end,end-start);
#endif
	}
	
	//rp.dwIp = mp.dwIp;
	//rp.wVideoPort = mp.wPort = 64000;
	//strcpy(rp.filename,"rec/chn0/20061024034349.mp4");
	//start = GetTickCount();
	//wRet = StartRemotePlay(rp,dwServerIp,wServerPort,CTRL_PROTOCOL_CONNECT_BLOCK);
	//end = GetTickCount();
	//printf("StartRemotePlay:wRet=%d,start=%d,end=%d,span=%d\n",wRet,start,end,end-start);
#else
	{
		ifly_cp_header_t cphead;
		CPHandle cph = CPConnect(dwServerIp,wServerPort,CTRL_PROTOCOL_CONNECT_BLOCK,NULL);
		if(cph != NULL)
		{
			struct sockaddr_in tAddr;
			GetCPHandleLocalAddr(cph,&tAddr);

			for(i=0;i<2;i++)
			{
				mp.byFlagSubcode = 0;
				mp.byChn = i;
				mp.wPort = 64000+i*4;
				wRet = CPSend(cph,CTRL_CMD_STARTVIDEOMONITOR,&mp,sizeof(mp),&cphead,sizeof(cphead),NULL,CTRL_PROTOCOL_CONNECT_DEFAULT);
				printf("result:%d,ack:%d\n",wRet,cphead.event);
			}

			
		}
		while(1) Sleep(1000);
	}
#endif

	CPLibCleanup(TRUE);
	
	return 0;
}
#endif
