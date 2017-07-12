#ifndef _MS_RTCP2_H_
#define _MS_RTCP2_H_

#include "mssocket.h"

#ifdef WIN32 
typedef  void * HANDLE; 
#endif

#define MAXSDES                   255//
#define FROM1900TILL1970          (u32)0x83AA7E80//
#define MAX_RTCP_PACK             (s32)1460//  

#define MAX_DROPOUT               3000//
#define MAX_MISORDER              100//
#define MIN_SEQUENTIAL            2//
#define RTP_SEQ_MOD               0x10000//seq num cycle unit

#define reduceNNTP(a) (((a).msdw<<16)+((a).lsdw>>16))

#define W32Len(l)  ((l + 3) / 4)  /* length in 32-bit u16s */

/* initial bit field value for RTCP headers: V=2,P=0,RC=0,PT=0,len=0 */
#define RTCP_HEADER_INIT          0x80000000

/* RTCP header bit locations - see the standard */
#define HEADER_V                  30      /* version                       */
#define HEADER_P                  29      /* padding                       */
#define HEADER_RC                 24      /* reception report count        */
#define HEADER_PT                 16      /* packet type                   */
#define HEADER_len                0       /* packet length in 32-bit u16s */

/* RTCP header bit field lengths - see the standard */
#define HDR_LEN_V                 2       /* version                       */
#define HDR_LEN_P                 1       /* padding                       */
#define HDR_LEN_RC                5       /* reception report count        */
#define HDR_LEN_PT                8       /* packet type                   */
#define HDR_LEN_len               16      /* packet length in 32-bit u16s */


/* used to overcome byte-allignment issues */
#define SIZEOF_RTCPHEADER         (sizeof(u32) * 2)//
#define SIZEOF_SR                 (sizeof(u32) * 5)//
#define SIZEOF_RR                 (sizeof(u32) * 6)//

#define SIZEOF_SDES(sdes)         (((sdes).m_byLength + 6) & 0xfc)
                                   //32-bit boundary,6=1+1+1+3
                                   

typedef enum {
	    RTCP_SR   = 200,               /* sender report            */
		RTCP_RR   = 201,               /* receiver report          */
		RTCP_SDES = 202,               /* source description items */
		RTCP_BYE  = 203,               /* end of participation     */
		RTCP_APP  = 204                /* application specific     */
}TRtcpType;

typedef enum {
		RTCP_SDES_END   = 0,
		RTCP_SDES_CNAME = 1,
		RTCP_SDES_NAME  = 2,
		RTCP_SDES_EMAIL = 3,
		RTCP_SDES_PHONE = 4,
		RTCP_SDES_LOC   = 5,
		RTCP_SDES_TOOL  = 6,
		RTCP_SDES_NOTE  = 7,
		RTCP_SDES_PRIV  = 8,
		RTCP_SDES_LOSE	= 9//csp add 2008-11-28
}TRtcpSDesType;

typedef struct tagUint64 
{
	u32  msdw; //most significant u16;
	u32  lsdw; //least significant u16;
}TUint64; 

typedef struct tagRtcpSR
{
	TUint64 m_tNNTP;//time
	u32     m_dwRTP;//timestamp
	
	u32     m_nPackets;
	u32     m_nBytes;
}TRtcpSR;

typedef struct tagRtcpRR
{
	u32  m_dwSSRC;
	u32  m_nFLost;      /* 8Bit fraction lost and 24 bit cumulative lost */
	u32  m_nExtMaxSeq;
	u32  m_nJitter;
	u32  m_dwLSR; //time
	u32  m_dwDLSR;//time
}TRtcpRR;

//CNAME SDES
typedef struct tagRtcpSDES
{
	u8  m_byType;
	u8  m_byLength; //only text length
	s8  m_szValue[MAXSDES + 1];     /* leave a place for an asciiz */
}TRtcpSDES;


//重发请求类型：以SN+TIMESTAMP 或者只以TIMESTAMP请求
#define SN_RSQ_TYPE          (u8)0
#define TIMESTAMP_RSQ_TYPE   (u8)1

//NOTE RSQ SDES
typedef struct tagRtpRSQ
{
	u8    m_byType;
	u8    m_byLength;					//only text length
//	u16   m_wMagic;						//掩码位填充位（0xFEFE）
	u32   m_dwRtpIP;					//与rtcp相对应的rtp接收ip   (网络序)
	u16   m_wRtpPort;					//与rtcp相对应的rtp接收port (网络序)
	u8    m_byRSQType;					//重发请求类型：以SN+TIMESTAMP 或者只以TIMESTAMP请求
	u8    m_byPackNum;					//重发请求帧中的总小包数（以SN+TIMESTAMP请求时有效）
	u16   m_wStartSeqNum;				//重发请求帧中的起始包序列
	u32   m_dwTimeStamp;				//重发请求帧的时间戳
	u8    m_byMaskBit[MAX_PACK_NUM/8];	//重发请求帧中各包的重发掩码位，00010101 （1－需要重发, 0－不重发）
}TRtcpSDESRSQ;

//csp add 2008-11-28
//NOTIFY FRAME LOSE SDES
typedef struct tagRtpLose
{
	u8    m_byType;
	u8    m_byLength;					//only text length
//	u16   m_wMagic;
	u32   m_dwRtpIP;					//与rtcp相对应的rtp接收ip   (网络序)
	u16   m_wRtpPort;					//与rtcp相对应的rtp接收port (网络序)
	
	u16   m_byReserved1;
	
	u32   m_dwLoseFrameId;
	
	u32   m_byReserved2;
}TRtcpSDESLose;

typedef struct tagMyInfo
{
	BOOL32    m_bActive;
	s32       m_nCollision;//和标准类型一致
	u32       m_dwSSRC; 
	u32       m_dwTimestamp;
	TRtcpSR   m_tSR;
	TRtcpSDES m_tCName;
}TMyInfo;

typedef struct tagRtpSouce
{
	u16  m_nMaxSeq;             /* highest seq. number seen */
	u32  m_nCycles;             /* shifted count of seq. number cycles */
	u32  m_nBaseSeq;            /* base seq number */
	u32  m_nBadSeq;             /* last 'bad' seq number + 1 */
	u32  m_nProbation;          /* sequ. packets till source is valid */
	u32  m_nReceived;           /* packets received */
	u32  m_nExpectedPrior;      /* packet expected at last interval */
	u32  m_nReceivedPrior;      /* packet received at last interval */
	u32  m_nTransit;            /* relative trans time for prev pkt */
	u32  m_nJitter;             /* estimated jitter */
    /* ... */
}TRtpSource;

typedef struct tagRtcpInfo
{
	s32        m_nInvalid;
	BOOL32     m_bActive;
	TRtpSource m_tSrc;
	u32        m_dwSSRC; 
	u32        m_dwLSRMyTime; 
	TRtcpSR    m_tSR;
	TRtcpRR    m_tToRR;
	TRtcpRR    m_tFromRR;        
	TRtcpSDES  m_tCName;
}TRtcpInfo;

typedef struct tagRtcpHeader 
{
	u32   m_dwBits; 
	u32   m_dwSSRC; 
}TRtcpHeader;

typedef struct tagRtcpInfoList
{
    s32       m_nSessionNum;
	TRtcpInfo	m_tRtcpInfo[MAX_SESSION_NUM];
}TRtcpInfoList;

typedef struct tagBuffer
{
	u32  m_dwLen;
	u8  *m_pBuf;
}TBuffer;

typedef u16 (*PLOSEPROC)(TRtcpSDESLose *ptLose, u32 dwContext); 

struct ifly_rtp_t;

typedef struct ms_rtcp_t
{
	ifly_socket_t *m_pSocket;
    u32			  *m_pdwBuf;
	
	TAddr         m_tLocalAddr;
	TRemoteAddr   m_tRemoteAddr;
	
//	TIMEID        m_hTimeId;
	
    TMyInfo       m_tMyInfo;
    TRtcpInfoList m_tRtcpInfoList;
    
	u32			  m_nStartTime;
	u32			  m_nStartMilliTime;
	
	struct ifly_rtp_t *m_pRtp;
	u32           *m_pCustomBuf;
	
//	TRtcpSDESRSQ  m_tBackRSQ;//重发请求包
	
	SEMHANDLE    m_hSem;
	
#ifdef WIN32
//	u32    m_dwThreadID;
//	HANDLE m_hThread;
//	HANDLE m_hEnd;
#endif
	
	//csp add 2008-11-28
	PLOSEPROC    m_pFrameLoseCallBack;
	u32			 m_dwFrameLoseContext;
}ms_rtcp_t;

#ifdef __cplusplus
extern "C" {
#endif
	
	//public:
	ms_rtcp_t* CreateRtcp(u32 dwSSRC);
	
	void InitRtcp(ms_rtcp_t *pRtcp);
	u16  DestroyRtcp(ms_rtcp_t *pRtcp);
	void FreeRtcpBuf(ms_rtcp_t *pRtcp);
	
	u16  SetRtcpRtp(ms_rtcp_t *pRtcp, struct ifly_rtp_t *pRtp);
	u16  SetRtcpLocalAddr(ms_rtcp_t *pRtcp, u32 dwIp, u16 wPort);
	u16  RemoveRtcpLocalAddr(ms_rtcp_t *pRtcp);
	u16  SetRtcpRemoteAddr(ms_rtcp_t *pRtcp, TRemoteAddr *pRemoteAddr);
	u16  UpdateRtcpSend(ms_rtcp_t *pRtcp, s32 nSendDataSize, u32 dwTimeStamp);
	u16  UpdateRtcpRcv(ms_rtcp_t *pRtcp, u32 dwSSRC, u32 dwLocalTimestamp, u32 dwTimestamp, u16 wSequence);
	void ResetRtcpSSRC(ms_rtcp_t *pRtcp, u32 dwSSRC);
	
	u16  SendRtcpRSQ(ms_rtcp_t *pRtcp, TRtcpSDESRSQ *pRSQ);
	
	//csp add 2008-11-28
	u16  SendRtcpFrameLoseEvent(ms_rtcp_t *pRtcp, TRtcpSDESLose *pLose);
	
	//private:
	void RtcpDataCallBack(u8 *pBuf, s32 nBufSize, u32 dwContext);
	void DealRtcpData(ms_rtcp_t *pRtcp, u8 *pBuf, s32 nBufSize);
	
	u16  DealRtcpTimer(ms_rtcp_t *pRtcp);
	
	void ProcessRTCPPacket(ms_rtcp_t *pRtcp, u8 *pData, s32 nDataLen, TRtcpType type, s32 nRCount, TUint64 myTime);
	
	TRtcpInfo *GetRtcpInfo(ms_rtcp_t *pRtcp, u32 dwSSRC);
	TRtcpInfo *AddRtcpInfo(ms_rtcp_t *pRtcp, TRtcpInfo *pInfo);
	
	void ParseRtcpRSQ(ms_rtcp_t *pRtcp, TRtcpSDES* pSdes, u32 nLen);
	
	//csp add 2008-11-28
	void ParseRtcpFrameLoseEvent(ms_rtcp_t *pRtcp, TRtcpSDES* pSdes, u32 nLen);
	
	BOOL32 UpdateRtcpSeq(ms_rtcp_t *pRtcp, TRtpSource *pRtpSource, u16 seq, u32 dwArrivalTS, u32 dwTimeStamp);
	void InitRtcpSeq(ms_rtcp_t *pRtcp, TRtpSource *pRtpSource, u16 seq);
	
	TRtcpHeader MakeHeader(u32 dwSSRC, u8 count, TRtcpType type, u16 dataLen);
	
	BOOL32  BufAddToBuffer(TBuffer *pTo, TBuffer *pFrom, u32 Offset);
	BOOL32  BufValid(TBuffer *pBuf, u32 dwSize);
	TBuffer BufCreate(void* pData, u32 dwSize);
	
	u32 GetLost(TRtpSource *pRtpSource);
	u32 GetJitter(TRtpSource *pRtpSource);
	u32 GetSequence(TRtpSource *pRtpSource);
	
	void CreateRTCPPacket(ms_rtcp_t *pRtcp, TBuffer *ptBuf);
	
	void CreateCustomRTCPPacket(ms_rtcp_t *pRtcp, TBuffer *ptBuf, TRtcpSDESRSQ *pRSQ);
	
	//csp add 2008-11-28
	void CreateCustomRTCPPacket2(ms_rtcp_t *pRtcp, TBuffer *ptBuf, TRtcpSDESLose *pLose);
	
	//csp add 2008-11-28
	u16 SetRtcpFrameLoseCallBack(ms_rtcp_t *pRtcp, PLOSEPROC pCallBackHandle, u32 dwContext);
#ifdef __cplusplus
}
#endif

#endif // !defined(_KDVRTCP_0603_H_)
