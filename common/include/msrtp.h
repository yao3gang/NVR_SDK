#ifndef _MS_RTP_H_
#define _MS_RTP_H_

#include "mssocket.h"

#include "msrtcp2.h"

typedef void (*PRCVPROC)(TRtpPack *ptRtpPack, u32 dwContext);

typedef struct ifly_rtp_t
{
	ms_rtcp_t	*m_pRtcp;
	
	ifly_socket_t *m_pSocket;
	TLoopBuf    *m_pLoopBuf;
	
	TAddr       m_tLocalAddr;
	TRemoteAddr m_tRemoteAddr;
	
	u32			m_dwSSRC;
	u16         m_wSeqNum;
	
    //RTP FIXED HEADER;
	u32			m_dwRtpHeader[RTP_FIXEDHEADER_SIZE/sizeof(u32)];
	u8			*m_pPackBuf;  
	
	PRCVPROC    m_pCallBack;
	u32			m_dwContext;
	
	
	BOOL		m_bRepeatSend;  //是否重发
	SEMHANDLE   m_hSynSem;		//用于重发环形缓冲的访问互斥
	u8			*m_pRSPackBuf;
	u8			*m_pRLoopBuf;   //重发环形缓冲
	u16		    m_wRLBUnitNum;  //重发环形缓冲中小包空间总数//3.0
	u16			m_wRLBWritePos;
	BOOL		m_bIsRLBFull;   //重发环形缓冲是否已满
	BOOL		m_bIsRLBWrited; //重发环形缓冲是否已写入，进行了最小SN的记录
	INT32		m_nRLBUnitSize; //重发环形缓冲中单位长度
	u16			m_wRLBMinSN;    //重发环形缓冲中小包最小包序号
	u16			m_wRLBMinPos;   //重发环形缓冲的最小序号小包对应的所在位置
	
	
	u32			m_dwTotalPackNum;  //发送的总的小包数
}ifly_rtp_t;

#ifdef __cplusplus
extern "C" {
#endif

ifly_rtp_t* CreateRtp(u32 dwSSRC, BOOL32 bAllocLPBuf);//csp modify 2008-11-20

void InitRtp(ifly_rtp_t *pRtp);
u16  DestroyRtp(ifly_rtp_t *pRtp);
void FreeRtpBuf(ifly_rtp_t *pRtp);

u16  SetRtpRtcp(ifly_rtp_t *pRtp, ms_rtcp_t *pRtcp);

u16  SetRtpCallBack(ifly_rtp_t *pRtp, PRCVPROC pCallBackHandle, u32 dwContext);

u16  SetRtpLocalAddr(ifly_rtp_t *pRtp,u32 dwIp, u16 wPort, BOOL bRcv);
u16  RemoveRtpLocalAddr(ifly_rtp_t *pRtp);
u16  SetRtpRemoteAddr(ifly_rtp_t *pRtp,TRemoteAddr *pRemoteAddr);
u16  WriteRtpPack(ifly_rtp_t *pRtp,TRtpPack *pRtpPack, BOOL bSend);
u16  SendRtpPack(ifly_rtp_t *pRtp,INT32 nPackNum,INT32 *pnRealPackNum);//csp modify 2008-11-28
void ResetRtpSeq(ifly_rtp_t *pRtp);
void ResetRtpSSRC(ifly_rtp_t *pRtp,u32 dwSSRC);

u16  ResetRtpRSFlag(ifly_rtp_t *pRtp, u16 wRLBUnitNum, BOOL32 bRepeatSnd);//csp modify 2008-11-20

void RtpCallBackProc(u8 *pBuf, s32 nSize, u32 dwContext);
void DealRtpData(ifly_rtp_t *pRtp, u8 *pBuf, s32 nSize);

u16  DealRtpRSQBackQuest(ifly_rtp_t *pRtp, TRtcpSDESRSQ *ptRSQ);//csp modify 2008-11-28

u16  CheckPackAvailabe(const TRtpPack *ptRtpPack, s32 *pnHeadLen);
u16  DirectSendRtpPack(ifly_rtp_t *pRtp, u8 *pPackBuf, s32 nPackLen, u32 dwTimeStamp);
u16  SaveRtpPackIntoLPBuf(ifly_rtp_t *pRtp, u8 *pPackBuf, s32 nPackLen, BOOL32 bTrans);
u16  WriteRtpPackIntoRLB(ifly_rtp_t *pRtp, u8 *pPacketBuf, s32 nPacketSize);
u16  ReadRtppackRLBBySN(ifly_rtp_t *pRtp, u8 *pBuf, s32 *pnBufSize, u32 dwTimeStamp, u16 wSeqNum);
s32  FindRtppackRLBByTS(ifly_rtp_t *pRtp, u8 *pBuf, u8 *pbyPackNum, u32 dwTimeStamp);

#ifdef __cplusplus
}
#endif

#endif
