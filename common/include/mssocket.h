#ifndef _MS_SOCKET_H_
#define _MS_SOCKET_H_

#include "mscommon.h"

typedef void (*PCALLBACK)(u8 *pBuf, s32 nBufSize, u32 dwContext);

typedef struct
{
	PCALLBACK m_pCallBack;
	u32 m_dwContext;
}TCallBack;

typedef struct
{
	s32 m_nSocketType;
	u16 m_wSocketPort;
	u32 m_dwLocalAddr;
	u32 m_dwMultiAddr;
	BOOL32 m_bRcv;
}TCreateSock;

typedef struct  
{
	SOCKHANDLE	 m_hSocket;
	BOOL32		 m_bMultiCast;
	SOCKADDR_IN  m_tAddrIn;
	
	SEMHANDLE    m_hSynSem;		   //用于套结字删除时的同步
	SEMHANDLE    m_hCreateSynSem;  //用于套结字创建时的同步
	BOOL32		 m_bSuccess;
	
	TCreateSock  m_tCreateSock;
	TCallBack    m_tCallBack;
	
	s32		     m_nSndBufSize;
	s32		     m_nRcvBufSize;
	
	//u32        m_tdwIPList[MAX_LOCAL_IP_NUM];  //记录加入组播组时 列举到的当前设置的所有有效ip
	//u16        m_dwIPNum;                      //记录加入组播组时 列举到ip 数目
	
	//ip_mreq	 *m_psMreq;
}ifly_socket_t;

#ifdef __cplusplus
extern "C" {
#endif

ifly_socket_t* OpenSocket();
void  CloseSocket(ifly_socket_t *pSocket);

void  SocketDestroy(ifly_socket_t *pSocket,BOOL bNotifyThread);
BOOL  SocketCreate(ifly_socket_t *pSocket,INT32 nSocketType, u16 wSocketPort, u32 dwLocalAddr, u32 dwMultiAddr, BOOL bRcv);
BOOL  SocketBuild(ifly_socket_t *pSocket,BOOL32 bSend);

INT32 SocketSendTo(ifly_socket_t *pSocket,u8 *pBuf,INT32 nSize,u32 dwRemoteIp,u16 wRemotePort);
void  SocketCallBack(ifly_socket_t *pSocket, u8 *pBuf, s32 nBufSize);
void  SetSocketCallBack(ifly_socket_t *pSocket,PCALLBACK pCallBack,u32 dwContext);
void  PrintSocketErrMsg(ifly_socket_t *pSocket,char *szErrMsg, BOOL bShowSockArg);

BOOL  IsMultiCastAddr(u32 dwIP);
BOOL  IsBroadCastAddr(u32 dwIP);

//u32 SocketTaskProc(void * pParam);
void* SocketTaskProc(void * pParam);

//u32 RtcpSndTaskProc(void * pParam);
void* RtcpSndTaskProc(void * pParam);

#ifdef __cplusplus
}
#endif

#endif
