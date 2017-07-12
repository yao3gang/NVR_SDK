#include "mssocket.h"

#ifdef WIN32

#define MAX_NET_RCV_BUF_LEN		128000*8*4
#define MAX_NET_SND_BUF_LEN		128000*4*4

#define MIN_NET_BUF_LEN			64000

#else

#define MAX_NET_RCV_BUF_LEN		128000
#define MAX_NET_SND_BUF_LEN		128000

#define MIN_NET_BUF_LEN			64000

#endif

#define  MAXWATCHSOCKPORT		(u16)20999//max watchsock port
#define  MINWATCHSOCKPORT		(u16)20800//min watchsock port

TMediaSndList   g_tMediaSndList;		//发送对象列表全局变量
TMediaSndList   g_tMediaSndListTmp;
SEMHANDLE		g_hMediaSndSem = NULL;	//发送对象列表的访问维护的信号量

TMediaRcvList   g_tMediaRcvList;		//接收对象列表全局变量
TMediaRcvList	g_tMediaRcvListTmp;
SEMHANDLE		g_hMediaRcvSem = NULL;	//接收对象列表的访问维护的信号量

//全局的时间戳tick数，用于统一提供收发对象的SSRC，保证唯一性
static   u32		   g_dwTimeStamp		= 0;
//用于统一提供收发对象的SSRC的同步信号量
static	 SEMHANDLE     g_hGetTimeStampSem	= NULL;

//线程引用计数
static	 INT32         g_nRefCount			= 0;
//socket接收线程handle
static	 THREADHANDLE  g_hRcvTask			= (THREADHANDLE)NULL;
//rtcp定时rtcp包上报线程handle//csp add 2008-11-20
static	 THREADHANDLE  g_hSndTask			= (THREADHANDLE)NULL;
//win32是否初始化了sock库的判断			
static   BOOL32        g_bSockInit			= FALSE;	
//控制线程的socket
static   SOCKHANDLE    g_hWatchSocket		= INVALID_SOCKET;
//watch socket port,deafult is MINWATCHSOCKPORT;
static   u16		   g_wWatchSockPort		= MINWATCHSOCKPORT;
//判断线程退出的信号量
static	 SEMHANDLE     g_hSem				= NULL;
//判断任务退出
static   BOOL          g_bExitTask			= FALSE;
//用于序列化 接收套接字 的创建删除请求操作的同步信号量
static	 SEMHANDLE     g_hCreateDelSem		= NULL;

//全局发送socket列表结构
typedef struct
{
	s32 m_nSockCount;//发送套接字总数 <= MAX_SND_NUM
	ifly_socket_t *m_tSockUnit[MAX_SND_NUM];
}TSndSockList;

//发送套接字列表的访问维护的信号量
static   SEMHANDLE	   g_hSndSockSem		= NULL;
//发送socket列表全局变量 
static   TSndSockList  g_tSndSockList;
//发送socket列表增减所用的中转结构
static   TSndSockList  g_tSndSockListTmp;

//全局接收socket列表结构
typedef struct
{
	s32 m_nSockCount;//接收套接字总数 <= FD_SETSIZE
	ifly_socket_t *m_tSockUnit[FD_SETSIZE];
}TSockList;

//接收socket列表全局变量
static   TSockList     g_tSockList;
//接收socket列表增减所用的中转结构
static   TSockList     g_tSockListTmp;

//Socket 状态消息
typedef enum 
{
	SOCK_CREATE = 0,
	SOCK_RCV    = 1, 
	SOCK_STOP   = 2, 
	SOCK_DEL    = 3, 
	SOCK_EXIT   = 4
}TSockState;

s32 g_nRepeatSnd = 1;
s32 g_nDiscardSpan = 0;
s32 g_nShowDebugInfo = 1;

//统一提供收发对象的SSRC的操作
u32 GetExclusiveSSRC()
{
	u32 dwRet = 0;
	
	if(NULL == g_hGetTimeStampSem)
	{
		return dwRet;
	}
	
	SemTake(g_hGetTimeStampSem);
	
	if(0 == g_dwTimeStamp)
	{
		g_dwTimeStamp = TickGet();
	}
	else
	{
		g_dwTimeStamp++;
	}
	
	dwRet = g_dwTimeStamp;
	
	SemGive(g_hGetTimeStampSem);
	
	return dwRet;
}

BOOL32 SendCtlMsg(s32 nMode, u32 dwContext)
{
	BOOL	bRet	= FALSE;
	INT32	nSndNum	= 0;
	SOCKADDR_IN  AddrIn;
	u32		dwBuf[2];
	
	if(g_hWatchSocket  == INVALID_SOCKET)
	{
		return bRet;
	}
	
	SemTake(g_hCreateDelSem);
	
	memset( &AddrIn, 0, sizeof(AddrIn));
	AddrIn.sin_family	   = AF_INET; 
	AddrIn.sin_addr.s_addr = inet_addr("127.0.0.1");
	AddrIn.sin_port		   = htons(g_wWatchSockPort);
	
	dwBuf[0] = (u32)nMode;
	dwBuf[1] = dwContext;
	
	nSndNum = sendto( g_hWatchSocket, (char*)dwBuf, sizeof(dwBuf), 0, 
		(struct sockaddr *)&AddrIn, sizeof(SOCKADDR_IN) );
	
	if(nSndNum == sizeof(dwBuf)) bRet = TRUE;
	
	SemGive(g_hCreateDelSem);
	
	return bRet;
}

u16 mediastreamStartup()
{
	SOCKADDR_IN addr;
	
	if(g_nRefCount==0)
	{
		memset(&g_tSockList,0,sizeof(g_tSockList));
		memset(&g_tSndSockList,0,sizeof(g_tSndSockList));
		memset(&g_tMediaRcvList,0,sizeof(g_tMediaRcvList));
		memset(&g_tMediaSndList,0,sizeof(g_tMediaSndList));
		
		/*终止任务*/
		if(g_hRcvTask != (THREADHANDLE)NULL)
		{
			IFly_ThreadTerminate(g_hRcvTask);
			g_hRcvTask = (THREADHANDLE)NULL;			 
		}
		//csp add 2008-11-20
		if(g_hSndTask != (THREADHANDLE)NULL)
		{
			IFly_ThreadTerminate(g_hSndTask);
			g_hSndTask = (THREADHANDLE)NULL;			 
		}
		
		/*清除ws2_32.dll的使用*/
		if(g_bSockInit)
		{
			SockCleanup();
			g_bSockInit = FALSE;
		}
		/*初始化winsock库*/
		g_bSockInit = SockInit();
		if(!g_bSockInit) 
		{
			printf("\n mediastreamStartup SockInit Error \n");
			return ERROR_WSA_STARTUP;
		}
		
		/*创建一个同步二元信号量*/
		if(g_hSem != NULL)
		{
			SemDelete(g_hSem);
			g_hSem = NULL;
		}
		if(!SemBCreate(&g_hSem))
		{
			g_hSem = NULL;
			printf("\n mediastreamStartup SemBCreate g_hSem Error \n");
			return ERROR_CREATE_SEMAPORE;
		}
		
		if(g_hCreateDelSem != NULL)
		{
			SemDelete(g_hCreateDelSem);
			g_hCreateDelSem = NULL;
		}
		//g_hCreateDelSem 开始有信号
		if(!SemBCreate(&g_hCreateDelSem))
		{
			g_hCreateDelSem = NULL;
			printf("\n mediastreamStartup SemBCreate g_hCreateDelSem Error \n");
			return ERROR_CREATE_SEMAPORE;
		}
		
		if(g_hMediaRcvSem != NULL)
		{
			SemDelete(g_hMediaRcvSem);
			g_hMediaRcvSem = NULL;
		}
		//g_hMediaRcvSem 开始有信号
		if(!SemBCreate(&g_hMediaRcvSem))
		{
			g_hMediaRcvSem = NULL;
			printf("\n mediastreamStartup SemBCreate g_hMediaRcvSem Error \n");
			return ERROR_CREATE_SEMAPORE;
		}
		
		if(g_hMediaSndSem != NULL)
		{
			SemDelete(g_hMediaSndSem);
			g_hMediaSndSem = NULL;
		}
		//g_hMediaSndSem 开始有信号
		if(!SemBCreate(&g_hMediaSndSem))
		{
			g_hMediaSndSem = NULL;
			printf("\n mediastreamStartup SemBCreate g_hMediaSndSem Error \n");
			return ERROR_CREATE_SEMAPORE;
		}
		
		if(g_hSndSockSem != NULL)
		{
			SemDelete(g_hSndSockSem);
			g_hSndSockSem = NULL;
		}
		//g_hSndSockSem 开始有信号
		if(!SemBCreate(&g_hSndSockSem))
		{
			g_hSndSockSem = NULL;
			printf("\n mediastreamStartup SemBCreate g_hSndSockSem Error \n");
			return ERROR_CREATE_SEMAPORE;
		}
		
		if(g_hGetTimeStampSem != NULL)
		{
			SemDelete(g_hGetTimeStampSem);
			g_hGetTimeStampSem = NULL;
		}
		//g_hGetTimeStampSem 开始有信号
		if(!SemBCreate(&g_hGetTimeStampSem))
		{
			g_hGetTimeStampSem = NULL;
			printf("\n mediastreamStartup SemBCreate g_hGetTimeStampSem Error \n");
			return ERROR_CREATE_SEMAPORE;
		}
		//获取 SSRC 的基值
		g_dwTimeStamp = TickGet();
		
		/*创建控制套接字*/
		if(g_hWatchSocket != INVALID_SOCKET)
		{
			SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;  
		}
		g_hWatchSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if( INVALID_SOCKET == g_hWatchSocket )
		{
			printf("\n mediastreamStartup WatchSock socket() Error \n");
			return ERROR_SOCKET_CALL;
		}
		
		memset(&addr, 0, sizeof(SOCKADDR_IN));
		addr.sin_family      = AF_INET; 
		addr.sin_addr.s_addr = 0;
		addr.sin_port        = htons(g_wWatchSockPort);
		
	    while( (SOCKET_ERROR == bind(g_hWatchSocket, (struct sockaddr *)&addr, sizeof(SOCKADDR_IN))) &&
			   (g_wWatchSockPort <= MAXWATCHSOCKPORT) )
		{
			g_wWatchSockPort++;
			addr.sin_port    = htons(g_wWatchSockPort);
		}
      
		if(g_wWatchSockPort>MAXWATCHSOCKPORT)
		{
			SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;  
			printf("\n mediastreamStartup WatchSock bind Error \n");
			return ERROR_BIND_SOCKET;
		}
		
		g_bExitTask = FALSE;
		
		/* 创建分发套接字线程（任务） */
		g_hRcvTask = IFly_CreateThread( SocketTaskProc, "tRcvDataTask", PRI_MEDIARCV, STKSIZE_MEDIARCV, 0, 0, NULL);		
		if(g_hRcvTask == (THREADHANDLE)NULL)
		{
			SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;  
			printf("\n mediastreamStartup TaskCreate tRcvDataTask Error \n");
			return ERROR_CREATE_THREAD;
		}
		
		/*//csp add 2008-11-20
		//创建rtcp定时rtcp包上报线程（任务） 这个任务级别较低
		g_hSndTask = IFly_CreateThread( RtcpSndTaskProc, "tSndRtcpTask", 120, 512*1024, 0, 0, NULL );
		if(g_hSndTask == (THREADHANDLE)NULL)
		{
			SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;
			if(g_hRcvTask != (THREADHANDLE)NULL)
			{
				IFly_ThreadTerminate(g_hRcvTask);
				g_hRcvTask = (THREADHANDLE)NULL;			 
			}
			printf("\n mediastreamStartup TaskCreate tSndRtcpTask Error  \n");
			return ERROR_RTCP_SET_TIMER;//ERROR_CREATE_THREAD;
		}*/
	}
	
	g_nRefCount++;
	
	return TRUE;
}

BOOL mediastreamCleanup()
{
	s32 nExit = g_nRefCount>0 ? g_nRefCount-- : 0;//判断是否清除一些全局成员。
	
	if(nExit==1)
	{
		//csp add 2008-11-20
		if(g_hSndTask != (THREADHANDLE)NULL)
		{
			IFly_ThreadTerminate(g_hSndTask);
			g_hSndTask = (THREADHANDLE)NULL;			 
		}
		
		g_bExitTask = TRUE;		 //控制线程退出的变量置为TRUE

		SendCtlMsg(SOCK_EXIT,(u32)0); //向线程发消息退出
		
		if(g_hSem != NULL)
		{
			if( FALSE == SemTakeByTime( g_hSem,5000 ) )//等待线程退出
			{
				return FALSE;
			}
		}
		
        if(g_hSem != NULL)
		{
			SemDelete(g_hSem);
			g_hSem = NULL;
		}
		
		if(g_hCreateDelSem != NULL)
		{
			SemDelete(g_hCreateDelSem);
			g_hCreateDelSem = NULL;
		}
		if(g_hMediaRcvSem != NULL)
		{
			SemDelete(g_hMediaRcvSem);
			g_hMediaRcvSem = NULL;
		}		
		if(g_hMediaSndSem != NULL)
		{
			SemDelete(g_hMediaSndSem);
			g_hMediaSndSem = NULL;
		}
		if(g_hSndSockSem!=NULL)
		{
			SemDelete(g_hSndSockSem);
			g_hSndSockSem = NULL;
		}
		if(g_hGetTimeStampSem != NULL)
		{
			SemDelete(g_hGetTimeStampSem);
			g_hGetTimeStampSem = NULL;
		}
		
		//关闭watchsocket
		if(g_hWatchSocket != INVALID_SOCKET)
		{
			SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;  
		}
		
		if(g_bSockInit)
		{
			SockCleanup();
			g_bSockInit = FALSE;
		}
		
		memset(&g_tSockList, 0, sizeof(g_tSockList));
		memset(&g_tSndSockList, 0, sizeof(g_tSndSockList));
		memset(&g_tMediaRcvList, 0, sizeof(g_tMediaRcvList));
		memset(&g_tMediaSndList, 0, sizeof(g_tMediaSndList));
	}
	
	return TRUE;
}

#define CHECK_THREAD_INIT { if(g_nRefCount == 0) return FALSE;}//

void InitSocket(ifly_socket_t *pSocket)
{
	pSocket->m_hSocket			= INVALID_SOCKET;
	pSocket->m_bMultiCast		= FALSE;
	
	pSocket->m_hSynSem			= NULL;
	pSocket->m_hCreateSynSem	= NULL;
	pSocket->m_bSuccess			= FALSE;
	
	memset(&pSocket->m_tAddrIn, 0, sizeof(pSocket->m_tAddrIn));
	memset(&pSocket->m_tCreateSock, 0, sizeof(pSocket->m_tCreateSock));
	memset(&pSocket->m_tCallBack, 0, sizeof(pSocket->m_tCallBack));
	
	pSocket->m_nSndBufSize = 0;
	pSocket->m_nRcvBufSize = 0;
	
	if(!SemCCreate(&pSocket->m_hSynSem,0,1))
	{
		pSocket->m_hSynSem = NULL;
		return;
	}
	if(!SemCCreate(&pSocket->m_hCreateSynSem,0,1))
	{
		pSocket->m_hCreateSynSem = NULL;
		return;
	}
}

ifly_socket_t* OpenSocket()
{
	ifly_socket_t *pSocket;
	pSocket = (ifly_socket_t *)malloc(sizeof(ifly_socket_t));
	if(pSocket==NULL)
	{
		return NULL;
	}
	InitSocket(pSocket);
	return pSocket;
}

void CloseSocket(ifly_socket_t *pSocket)
{
	if(pSocket)
	{
		SocketDestroy(pSocket,TRUE);
		if(pSocket->m_hSynSem != NULL)
		{
			SemDelete(pSocket->m_hSynSem);
		}
		if(pSocket->m_hCreateSynSem != NULL)
		{
			SemDelete(pSocket->m_hCreateSynSem);
		}
		free(pSocket);//csp add 2007-07-04
	}
	//printf("sizeof(ifly_socket_t)=%d\n",sizeof(ifly_socket_t));
}

void SocketDestroy(ifly_socket_t *pSocket,BOOL bNotifyThread)
{
	BOOL  bRet;
	INT32 i,j;
	
	if(pSocket->m_hCreateSynSem == NULL || pSocket->m_hSynSem == NULL)
	{
		return;
	}
	
	if(INVALID_SOCKET == pSocket->m_hSocket)
	{
		return;
	}
	
	if(bNotifyThread && pSocket->m_tCreateSock.m_bRcv)
	{
		//对于接收套接字，由辅助线程统一注册记录，并进行删除,并回馈消息，同步删除结果
		
		bRet = SendCtlMsg(SOCK_DEL,(u32)pSocket);//退出全局socket列表
		
		if(FALSE == bRet)
		{
			PrintSocketErrMsg(pSocket,"Socket Close SendCtlMsg Error, Print Argument",FALSE);
			printf("g_hWatchSocket:%d  \n", g_hWatchSocket);
		}
		else
		{
			if(pSocket->m_hSynSem != NULL)
			{
				SemTakeByTime( pSocket->m_hSynSem,2000 );//等待socket退出
			}
		}
	}
	
	if(bNotifyThread && !pSocket->m_tCreateSock.m_bRcv)
	{
		SemTake(g_hSndSockSem);
		
		//从发送套接字链表中删除对象指针
		memset(&g_tSndSockListTmp, 0, sizeof(g_tSndSockListTmp));
		for(i=0; i<g_tSndSockList.m_nSockCount; i++)
		{
			if(g_tSndSockList.m_tSockUnit[i] == pSocket) continue;
			g_tSndSockListTmp.m_tSockUnit[g_tSndSockListTmp.m_nSockCount] \
				= g_tSndSockList.m_tSockUnit[i];
			g_tSndSockListTmp.m_nSockCount++;
		}
		g_tSndSockList.m_nSockCount = g_tSndSockListTmp.m_nSockCount;
		for(j=0; j<g_tSndSockListTmp.m_nSockCount; j++)
		{
			g_tSndSockList.m_tSockUnit[j] = 
				g_tSndSockListTmp.m_tSockUnit[j];
		}
		
		SemGive(g_hSndSockSem);		
	}
	
    if( TRUE == pSocket->m_bMultiCast )
	{
		//close multicast option
		struct ip_mreq tMreq;
		memset(&tMreq, 0, sizeof(struct ip_mreq));	
		tMreq.imr_multiaddr.s_addr = pSocket->m_tCreateSock.m_dwMultiAddr;
		tMreq.imr_interface.s_addr = pSocket->m_tCreateSock.m_dwLocalAddr;
		setsockopt( pSocket->m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&tMreq, sizeof(struct ip_mreq) );
		
        pSocket->m_bMultiCast = FALSE;
	}
	
	//close socket
	//if( INVALID_SOCKET != pSocket->m_hSocket )
	{
		SockClose(pSocket->m_hSocket);
		pSocket->m_hSocket = INVALID_SOCKET;
	}
	memset(&pSocket->m_tAddrIn, 0, sizeof(pSocket->m_tAddrIn));
}

BOOL SocketCreate(ifly_socket_t *pSocket,INT32 nSocketType, u16 wSocketPort, u32 dwLocalAddr, u32 dwMultiAddr, BOOL bRcv)
{
	BOOL32 bRet  = FALSE;
	BOOL32 bFind = FALSE;
	s32    nPos	 = 0;
	
	CHECK_THREAD_INIT
	
	if(pSocket == NULL || pSocket->m_hCreateSynSem == NULL || pSocket->m_hSynSem == NULL)
	{
		return bRet;
	}
	
	SocketDestroy(pSocket,TRUE);
	
	pSocket->m_tCreateSock.m_nSocketType = nSocketType;
	pSocket->m_tCreateSock.m_wSocketPort = wSocketPort;
	pSocket->m_tCreateSock.m_bRcv        = bRcv;
	pSocket->m_tCreateSock.m_dwLocalAddr = dwLocalAddr;
	pSocket->m_tCreateSock.m_dwMultiAddr = dwMultiAddr;
	
	//对于接收套接字，由辅助线程统一注册记录，并进行创建,并回馈消息，同步创建结果
	if(TRUE == bRcv)
	{
		pSocket->m_bSuccess = FALSE;
		
		bRet = SendCtlMsg(SOCK_CREATE, (u32)pSocket);
		if(bRet == FALSE)
		{
			PrintSocketErrMsg(pSocket,"Socket Create SendCtlMsg Error, Print Argument",FALSE);
			printf("g_hWatchSocket:%d\n", g_hWatchSocket);
		}
		
		if(bRet)
		{
			if(SemTakeByTime( pSocket->m_hCreateSynSem, 2000 ) == FALSE)
			{
				PrintSocketErrMsg(pSocket,"Socket Create OspSemTakeByTime Error\n",TRUE);
			}
			bRet = pSocket->m_bSuccess;
		}	
	}
	else//对于发送用套接字，直接由调用线程创建
	{
		bRet = SocketBuild(pSocket,TRUE);
		
		if(bRet)
		{
			SemTake(g_hSndSockSem);
			
			//加入发送套接字链表中记录对象指针
			bFind = FALSE;
			if(g_tSndSockList.m_nSockCount < MAX_SND_NUM)
			{
				for(nPos=0; nPos<g_tSndSockList.m_nSockCount; nPos++)
				{
					if(g_tSndSockList.m_tSockUnit[nPos] == pSocket)
					{
						bFind = TRUE;
						break;
					}
				}
				if(FALSE == bFind)
				{
					g_tSndSockList.m_nSockCount++;
					g_tSndSockList.m_tSockUnit[g_tSndSockList.m_nSockCount-1] = pSocket;
				}
			}
			
			SemGive(g_hSndSockSem);
		}
	}
	
	if(bRet == FALSE)
	{
		PrintSocketErrMsg(pSocket,"Socket Create Error",TRUE);
	}
	
    return bRet;
}

BOOL SocketBuild(ifly_socket_t *pSocket,BOOL32 bSend)
{
	SOCKADDR_IN addr;
	INT32 nReuseAddr;
	int optval;
	
	INT32 nBufSize;
	INT32 nTotalBufSize;
	INT32 nRealBufSize;
	INT32 nSize;	
	
	//生成socket对象
	pSocket->m_hSocket = socket( AF_INET, pSocket->m_tCreateSock.m_nSocketType, 0 );
	if( INVALID_SOCKET == pSocket->m_hSocket )
	{
		PrintSocketErrMsg(pSocket,"Socket socket() Error",TRUE);
		return FALSE;
	}
	
	// set reuse option
	nReuseAddr = 1;
	if( SOCKET_ERROR  == setsockopt(pSocket->m_hSocket, SOL_SOCKET, SO_REUSEADDR, 
		(char *)&nReuseAddr, sizeof(nReuseAddr)) )
	{
		PrintSocketErrMsg(pSocket,"Socket setsockopt SO_REUSEADDR Error",TRUE);
		SocketDestroy(pSocket,FALSE);
		return FALSE;
	}
	else
	{
		//printf("Socket setsockopt SO_REUSEADDR success\n");
	}
	
	//绑定socket
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family      = AF_INET; 
	addr.sin_addr.s_addr = 0;//pSocket->m_tCreateSock.m_dwLocalAddr;
	addr.sin_port        = htons(pSocket->m_tCreateSock.m_wSocketPort);
	if( SOCKET_ERROR == bind(pSocket->m_hSocket,(struct sockaddr *)&addr,sizeof(SOCKADDR_IN)) )
	{
		PrintSocketErrMsg(pSocket,"Socket bind Error",TRUE);
		SocketDestroy(pSocket,FALSE);
		return FALSE;
	}
	/*else
	{
		printf("Socket bind at port:%d success,m_hSocket=%d\n",pSocket->m_tCreateSock.m_wSocketPort,pSocket->m_hSocket);
	}*/
	
	if(FALSE == bSend)
	{
		//Set multicast option 
		if( IsMultiCastAddr(pSocket->m_tCreateSock.m_dwMultiAddr) )
		{
			struct ip_mreq tMreq;
			memset(&tMreq, 0, sizeof(struct ip_mreq));	
			tMreq.imr_multiaddr.s_addr = pSocket->m_tCreateSock.m_dwMultiAddr;
			tMreq.imr_interface.s_addr = pSocket->m_tCreateSock.m_dwLocalAddr;
			
			if( SOCKET_ERROR == setsockopt(pSocket->m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
				(char*)&tMreq, sizeof(struct ip_mreq)) )
			{
				PrintSocketErrMsg(pSocket,"Socket setsockopt IP_ADD_MEMBERSHIP Error",TRUE);
				SocketDestroy(pSocket,FALSE);
				return FALSE;
			}
			printf("socket join in the multigroup success\n");
			
			pSocket->m_bMultiCast = TRUE;
			return TRUE;
		}
		
		//Set Broadcast option
		if( IsBroadCastAddr(pSocket->m_tCreateSock.m_dwMultiAddr) )
		{
			optval = 1;
			if( SOCKET_ERROR == setsockopt(pSocket->m_hSocket, SOL_SOCKET, SO_BROADCAST,
				(char*)&optval, sizeof(optval)) )
			{
				PrintSocketErrMsg(pSocket,"Socket setsockopt SO_BROADCAST Error",TRUE);
				SocketDestroy(pSocket,FALSE);
				return FALSE;
			}
		}
	}
	/*else
	{
		//int nNetTimeout = 20;//20ms
		//int ret = setsockopt(pSocket->m_hSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&nNetTimeout,sizeof(int));

		struct timeval val;
		val.tv_sec = 0;
		val.tv_usec = 20000;//20ms
		int ret = setsockopt(pSocket->m_hSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&val,sizeof(val));
		
		printf("set send socket time out result:%d\n",ret);
		if(ret < 0)
		{
			printf("setsockopt errorcode=%d,errstr=%s\n",errno,strerror(errno));
		}
	}*/
	
	nBufSize			= MAX_NET_SND_BUF_LEN;
	nTotalBufSize		= MAX_NET_SND_BUF_LEN;
	nRealBufSize		= 0;
	nSize				= sizeof(nRealBufSize);
	
	/*
	调整套接字的接收、发送缓冲大小 ;
	1.最大接收缓冲大小为 128000*8*4 bytes, 最大发送缓冲大小为 128000*4 bytes
	  缺省及最小收发缓冲大小为 128000*2 bytes ;
	2.用于发送的套接字，其发送缓冲大小将由发送最大值开始尝试,接收缓冲大小为缺省值
	  用于接收的套接字，其接收缓冲大小将由接收最大值开始尝试,发送缓冲大小为缺省值 .
	*/
	
	if(FALSE == bSend)
	{
		nBufSize		= MIN_NET_BUF_LEN;
		nTotalBufSize	= MIN_NET_BUF_LEN;
	}
	
//#ifdef WIN32
	//Set Send buffer length
	while(nBufSize >= MIN_NET_BUF_LEN)
	{
		if( SOCKET_ERROR == setsockopt( pSocket->m_hSocket, SOL_SOCKET, SO_SNDBUF,
			(char *)&nTotalBufSize, sizeof(int)) )
		{
			printf("Socket setsockopt SO_SNDBUF : %d error\n",nBufSize);
			nBufSize = (nBufSize / 2);
			nTotalBufSize = nBufSize;
			continue;
		}
		
		if( SOCKET_ERROR == getsockopt( pSocket->m_hSocket, SOL_SOCKET, SO_SNDBUF,
			(char*)&nRealBufSize, &nSize) )
		{
			printf("Socket getsockopt SO_SNDBUF Error\n");
			nBufSize = (nBufSize / 2);
			nTotalBufSize = nBufSize;
			continue;
		}
		#ifdef WIN32
		/*NOTE : linux : nRealBufSize(get) = nBufSize(set) * 2 */
		else if(nRealBufSize != nBufSize)
		{
			printf("setsockopt SO_SNDBUF : %d invalid, real : %d, errno : %d, errstr : %s\n",nBufSize,nRealBufSize,errno,strerror(errno));
			nBufSize = (nBufSize / 2);
			nTotalBufSize = nBufSize;
			continue;
		}
		#endif
		else
		{
			printf("getsockopt SO_SNDBUF : %d OK\n",nBufSize);
			break;
		}
	}
	
	if(nBufSize < MIN_NET_BUF_LEN)
	{
		PrintSocketErrMsg(pSocket,"Socket setsockopt SO_SNDBUF Error",TRUE);
		SocketDestroy(pSocket,FALSE);
		return FALSE;
	}
//#endif
	
	pSocket->m_nSndBufSize = nBufSize;
	
	nRealBufSize = 0;
	if(TRUE == bSend)
	{
		nBufSize		= MIN_NET_BUF_LEN;
		nTotalBufSize	= MIN_NET_BUF_LEN;
	}
	else
	{	
		nBufSize		= MAX_NET_RCV_BUF_LEN;
		nTotalBufSize	= MAX_NET_RCV_BUF_LEN;
	}
	
#ifdef WIN32
	//Set recieve buffer length
	while(nBufSize >= MIN_NET_BUF_LEN)
	{
		if( SOCKET_ERROR == setsockopt( pSocket->m_hSocket, SOL_SOCKET, SO_RCVBUF,
			(char *)&nTotalBufSize, sizeof(int)) )
		{
			printf("Socket setsockopt SO_RCVBUF : %d error\n",nBufSize);
			nBufSize = (nBufSize / 2);
			nTotalBufSize = nBufSize;
			continue;
		}
		
		if( SOCKET_ERROR == getsockopt( pSocket->m_hSocket, SOL_SOCKET, SO_RCVBUF,
			(char*)&nRealBufSize, &nSize) )
		{
			printf("Socket getsockopt SO_RCVBUF Error\n");
			nBufSize = (nBufSize / 2);
			nTotalBufSize = nBufSize;
			continue;
		}
		else if(nRealBufSize != nBufSize)
		{
			printf("setsockopt SO_RCVBUF : %d invalid\n",nBufSize);
			nBufSize = (nBufSize / 2);
			nTotalBufSize = nBufSize;
			continue;
		}
		else
		{
			printf("getsockopt SO_RCVBUF : %d OK\n",nBufSize);
			break;
		}
	}
	
	if(nBufSize < MIN_NET_BUF_LEN)
	{
		PrintSocketErrMsg(pSocket,"Socket setsockopt SO_RCVBUF Error",TRUE);
		SocketDestroy(pSocket,FALSE);
		return FALSE;
	}
#endif
	
	pSocket->m_nRcvBufSize = nBufSize;
		
	//当套接字发送的数据包未被接收时，禁止 回馈的ICMP包 的通知
	// disable  new behavior using IOCTL: SIO_UDP_CONNRESET
	
	return TRUE;
}

//#include <sys/time.h>

INT32 SocketSendTo(ifly_socket_t *pSocket,u8 *pBuf,INT32 nSize,u32 dwRemoteIp,u16 wRemotePort)
{
	INT32 nRet = 0;
	
	if(pSocket->m_hSocket == INVALID_SOCKET)
	{
		return 0;
	}
	
	memset( &pSocket->m_tAddrIn, 0, sizeof(pSocket->m_tAddrIn));
	pSocket->m_tAddrIn.sin_family		= AF_INET; 
	pSocket->m_tAddrIn.sin_addr.s_addr	= dwRemoteIp;
	pSocket->m_tAddrIn.sin_port			= htons(wRemotePort);
    	
	//return sendto( pSocket->m_hSocket, (char*)pBuf, nSize, 0,
		//(struct sockaddr *)&pSocket->m_tAddrIn, sizeof(SOCKADDR_IN) );
	
	/*struct timeval start,end;
	long span;
	
	gettimeofday(&start,NULL);*/
	
#ifdef WIN32
	nRet = sendto( pSocket->m_hSocket, (char*)pBuf, nSize, 0,
		(struct sockaddr *)&pSocket->m_tAddrIn, sizeof(SOCKADDR_IN) );
#else
	//nRet = sendto( pSocket->m_hSocket, (char*)pBuf, nSize, MSG_DONTWAIT,
	//	(struct sockaddr *)&pSocket->m_tAddrIn, sizeof(SOCKADDR_IN) );
	nRet = sendto( pSocket->m_hSocket, (char*)pBuf, nSize, MSG_DONTWAIT,
		(struct sockaddr *)&pSocket->m_tAddrIn, sizeof(SOCKADDR_IN) );
#endif
	
	/*gettimeofday(&end,NULL);
	span = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);	
	
	if(nRet<0)
	{
		printf("*ret=%d,error=%d,span=%ld,bytes=%d*\n",nRet,errno,span,nSize);
	}
	else
	{
		if(span>1000)
		{
			printf("net send success span:%ld,bytes:%d\n",span,nSize);
		}
	}*/
	
	return nRet;
}

void SocketCallBack(ifly_socket_t *pSocket, u8 *pBuf, s32 nBufSize)
{
	if(pSocket->m_tCallBack.m_pCallBack != NULL)
	{
		pSocket->m_tCallBack.m_pCallBack(pBuf, nBufSize, pSocket->m_tCallBack.m_dwContext);
	}
	else
	{
		printf("[SocketCallBack] m_tCallBack.m_pCallBack == NULL   \n");
	}
}

void SetSocketCallBack(ifly_socket_t *pSocket,PCALLBACK pCallBack,u32 dwContext)
{
	pSocket->m_tCallBack.m_pCallBack = pCallBack;
	pSocket->m_tCallBack.m_dwContext = dwContext;
}

void PrintSocketErrMsg(ifly_socket_t *pSocket,char *szErrMsg,BOOL bShowSockArg)
{
	printf("\n PrintSocketErrMsg::szErrMsg: %s \n", szErrMsg);
	if(TRUE == bShowSockArg)
	{
		printf("Print Argument List   \n");
		printf("LocalAddr     MultiAddr    Port    bRcv\n");
		printf("%x     %x     %d     %d \n", 
			pSocket->m_tCreateSock.m_dwLocalAddr, pSocket->m_tCreateSock.m_dwMultiAddr, 
			pSocket->m_tCreateSock.m_wSocketPort, pSocket->m_tCreateSock.m_bRcv);
	}
}

BOOL IsMultiCastAddr(u32 dwIP)
{
	u8 byLOByte = LOBYTE( LOWORD(dwIP) );
	
	if( (byLOByte >= 224) && (byLOByte <= 239))
		return TRUE;
	else
		return FALSE;
}

BOOL IsBroadCastAddr(u32 dwIP)
{
	u8 byHIByte = HIBYTE( HIWORD(dwIP) );
	
	if( 255 == byHIByte )
		return TRUE;
	else
		return FALSE;
}

//u32 SocketTaskProc(void * pParam)
void* SocketTaskProc(void * pParam)
{
	u8 *pBuf;
	u32	dwBufLen;
	INT32 nOutSize;
	INT32 nRcvNum;
	fd_set rcvSocketfd;
	u32 *pMode;
	ifly_socket_t *pMain;
	BOOL bFind;
	INT32 nPos;
	INT32 i,j,k;
	//FRAMEHDR tFrmHdr;
	
#ifndef WIN32
	printf("$$$$$$$$$$$$$$$$$$SocketTaskProc id:%d\n",gettid());
	Dump_Thread_Info("socketTask",gettid());
#endif
	
	if(g_hWatchSocket == INVALID_SOCKET)
	{
		return 0;
	}
	if(g_hSem == NULL) 
	{	
		return 0;
	}
	
	dwBufLen = RTP_FIXEDHEADER_SIZE + MAX_RCV_PACK_SIZE;
	pBuf     = (u8 *)malloc(dwBufLen+4);
	nOutSize = 0;
	nRcvNum  = 0;
	
	//memset( &tFrmHdr,0, sizeof(tFrmHdr) );
	
	//g_hSem开始有信号，进入线程后置为无信号
	SemTake(g_hSem);// be used to wait for  Exiting thread;
	
	FD_ZERO(&rcvSocketfd);
	FD_SET(g_hWatchSocket,&rcvSocketfd);
	
  	while(select(FD_SETSIZE, &rcvSocketfd, NULL, NULL, NULL ) != SOCKET_ERROR)
	{
		if(g_bExitTask==TRUE) break;
		
		//printf("[SocketTaskProc] select is running\n");
		
		if( FD_ISSET( g_hWatchSocket, &rcvSocketfd ) )
		{
			memset(pBuf, 0, 8);
			//解析控制包
			recvfrom(g_hWatchSocket, (char *)pBuf, 8, 0, NULL, NULL);
			pMode = (u32 *)pBuf;				  //控制命令
            		pMain = (ifly_socket_t *)(*(pMode+1));//对象
			if(pMain == NULL) continue;
			
			//解析控制命令
			switch((INT32)(*pMode))
			{
			case SOCK_CREATE:   //创建socket
				{
					if(g_tSockList.m_nSockCount >= FD_SETSIZE)
					{
						PrintSocketErrMsg(pMain,"g_tSockList.m_nSockCount >= FD_SETSIZE",TRUE);
						
						//完成线程同步
						if(pMain->m_hCreateSynSem != NULL) 
						{
							pMain->m_bSuccess = FALSE;
							SemGive(pMain->m_hCreateSynSem);
						}
						break;
					}
					if(FALSE == SocketBuild(pMain,FALSE))
					{
						//完成线程同步
						if(pMain->m_hCreateSynSem != NULL) 
						{
							pMain->m_bSuccess = FALSE;
							SemGive(pMain->m_hCreateSynSem);
						}
						 break;
					}
					//加入socket list
					bFind = FALSE;
					for(nPos=0; nPos<g_tSockList.m_nSockCount; nPos++)
					{
						if(g_tSockList.m_tSockUnit[nPos] == pMain)
						{
							bFind=TRUE;
							break;
						}
					}
					if(!bFind)
					{
    					g_tSockList.m_nSockCount++;
						g_tSockList.m_tSockUnit[g_tSockList.m_nSockCount-1] = pMain;
					}
					
					//完成线程同步
					if(pMain->m_hCreateSynSem != NULL) 
					{
						pMain->m_bSuccess = TRUE;
						SemGive(pMain->m_hCreateSynSem);
					}
					
				    break;
				}
			case SOCK_DEL:
				{
					//从数组中删除对象指针
					memset(&g_tSockListTmp, 0, sizeof(g_tSockListTmp));
					for(i=0; i<g_tSockList.m_nSockCount; i++)
					{
						if(g_tSockList.m_tSockUnit[i] == pMain) continue;
						g_tSockListTmp.m_tSockUnit[g_tSockListTmp.m_nSockCount] \
							                       = g_tSockList.m_tSockUnit[i];
						g_tSockListTmp.m_nSockCount++;
					}
					g_tSockList.m_nSockCount = g_tSockListTmp.m_nSockCount;
					for(j=0; j<g_tSockListTmp.m_nSockCount; j++)
					{
						g_tSockList.m_tSockUnit[j] = 
							           g_tSockListTmp.m_tSockUnit[j];
					}

                    //完成线程同步
					if(pMain->m_hSynSem != NULL) SemGive(pMain->m_hSynSem);
				    break;
				}
			default : break;

			}
		}
		//循环查找激活socket
		for(j=0; j<g_tSockList.m_nSockCount; j++)
		{
			if(g_tSockList.m_tSockUnit[j]->m_hSocket == INVALID_SOCKET)
			{
				continue;
			}
			if(FD_ISSET(g_tSockList.m_tSockUnit[j]->m_hSocket, &rcvSocketfd))
			{
				nRcvNum = recvfrom(g_tSockList.m_tSockUnit[j]->m_hSocket, 
								   (char *)pBuf,
								   dwBufLen,
							       //MAX_PACK_SIZE + RTP_FIXEDHEADER_SIZE,
								   0, NULL, NULL);
				if( nRcvNum <= 0)
				{
					printf("[SocketTaskProc] recvfrom Exception: nRcvNum = %d, LastError = %d\n",nRcvNum,errno);
					continue;
				}
				//printf("recvfrom success\n");
                SocketCallBack(g_tSockList.m_tSockUnit[j], pBuf, nRcvNum);
			}
		}
		
		//set socket to select
		FD_ZERO(&rcvSocketfd);
		//FD_SET( g_hWatchSocket, &rcvSocketfd);
		if(g_hWatchSocket != INVALID_SOCKET)
		{
			FD_SET( g_hWatchSocket, &rcvSocketfd);
		}
		else
		{
			//报警，g_hWatchSocket == INVALID_SOCKET
			printf("\n Warning: g_hWatchSocket == INVALID_SOCKET, Ctl Thread Exception..... \n");
		}
		
		for(k=0; k<g_tSockList.m_nSockCount; k++)
		{
			if(g_tSockList.m_tSockUnit[k]->m_hSocket != INVALID_SOCKET)
			{
				FD_SET(g_tSockList.m_tSockUnit[k]->m_hSocket, &rcvSocketfd);
			}
			else
			{
				//暂时不作数组回收，而在析构时统一处理
			}
		}
	}
	
	//printf("recv thread quit!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	//exit(1);
	
	SAFE_DELETE(pBuf)
	
	if(g_hSem!=NULL) SemGive(g_hSem);// be used to wait for  Exiting thread;
	
	return 0;
}

extern u32 SystemClkRateGet();

/*=============================================================================
函数名		: RtcpSndTaskProc
功能		：rtcp定时rtcp包上报线程（任务）入口函数 这个任务级别较低
算法实现	：（可选项）
引用全局变量：无
输入参数说明：
pParam     user data 
返回值说明：无
=============================================================================*/
void* RtcpSndTaskProc(void * pParam)
{
#ifndef WIN32
	printf("$$$$$$$$$$$$$$$$$$RtcpSndTaskProc id:%d\n",gettid());
	Dump_Thread_Info("RtcpSndTaskProc",gettid());
#endif
	
	while(TRUE)
	{
		u32 dwStartTS = 0;
		u32 dwEndTS = 0;
		u32 dwSpanTime = 0;
		
		//线程安全退出
		if(g_bExitTask == TRUE) break;
		
		if(6 == g_nShowDebugInfo)
		{
			printf("[RtcpSndTaskProc] is running 1\n");
		}
		
		dwStartTS = GetSystemTick();
		
		if(NULL != g_hMediaSndSem)
		{		
			ifly_mediasnd_t *pcMediaSnd = NULL;
			
			s32 nPos = 0;
			
			if(NULL != g_hMediaSndSem)
			{
				SemTake(g_hMediaSndSem);
			}
			
			for(nPos=0; nPos<g_tMediaSndList.m_nMediaSndCount; nPos++)
			{
				pcMediaSnd = g_tMediaSndList.m_tMediaSndUnit[nPos];
				if(NULL != pcMediaSnd)
				{
					//printf("DealMediaSndRtcpTimer 1\n");
					DealMediaSndRtcpTimer(pcMediaSnd);
					//printf("DealMediaSndRtcpTimer 2\n");
				}
			}
			
			if(NULL != g_hMediaSndSem)
			{
				SemGive(g_hMediaSndSem);
			}
		}
		
		if( NULL != g_hMediaRcvSem )
		{
			ifly_mediarcv_t *pcMediaRcv = NULL;
			
			s32 nPos = 0;
			
			if(NULL != g_hMediaRcvSem)
			{
				SemTake(g_hMediaRcvSem);
			}
			
			for(nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
			{
				pcMediaRcv = g_tMediaRcvList.m_tMediaRcvUnit[nPos];
				if(NULL != pcMediaRcv)
				{
					//printf("DealMediaRcvRtcpTimer 1\n");
					DealMediaRcvRtcpTimer(pcMediaRcv);
					//printf("DealMediaRcvRtcpTimer 2\n");
				}
			}
			
			if(NULL != g_hMediaRcvSem)
			{
				SemGive(g_hMediaRcvSem);
			}
		}
		
		dwEndTS = GetSystemTick();
		if(dwEndTS > dwStartTS)
		{
			dwSpanTime = (dwEndTS - dwStartTS)*1000/SystemClkRateGet();
		}
		else
		{
			dwSpanTime = 0;
		}

		if(6 == g_nShowDebugInfo)
		{
			printf("[RtcpSndTaskProc] is running 2\n");
		}
		
		// 5秒做一次rtcp包上报
		if(dwSpanTime < 5000)
		{
#ifdef WIN32
			Sleep((5000 - dwSpanTime));
#else
			usleep((5000 - dwSpanTime) * 1000);
#endif
		}
	}
	
	printf("RtcpSndTaskProc is over ...... \n");
	
	return 0;
}
