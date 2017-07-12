#ifndef _MS_SEND_H_
#define _MS_SEND_H_

#include "msrtp.h"

typedef struct ifly_netsnd_t
{
	ifly_rtp_t			*m_pcRtp;
	
	ms_rtcp_t			*m_pcRtcp;//csp add 2008-11-20
	
//	TH261HeaderList		*m_ptH261HeaderList;
//	TH263HeaderList		*m_ptH263HeaderList;
	
//	TRtpPack			m_tOldRtp;
//	TRtpPack			m_tOldRtp;//csp add 2008-11-20
	
	u32					m_dwFrameId;
	u32					m_dwTimeStamp;
	u8					m_byFrameRate;
	
	u8					m_byMediaType; 
	u32					m_dwMaxFrameSize;
	
	u8					m_byExBuf[MAX_PACK_EX_LEN]; 
	
	s32					m_nMaxSendNum;//根据带块计算的最大发送次数;
	
	TSndStatus			m_tSndStatus;
	TSndStatistics		m_tSndStatistics;
	
//	FRAMEHDR			m_tSelfFrmHdr;
//	u8					*m_pSelfFrameBuf;	
	
	BOOL32				m_bRepeatSend;//是否重发
	u16					m_wBufTimeSpan;
	
	
	
	//csp add 2008-11-28
	u32					m_dwNewKeyFrameId;
	u32					m_dwLoseFrameId;
}ifly_netsnd_t;

#ifdef __cplusplus
extern "C" {
#endif

//创建发送模块
ifly_netsnd_t* CreateNetSnd(u32 dwMaxFrameSize, u32 dwNetBand, u8 byFrameRate, u8 byMediaType, u32 dwSSRC);

//删除发送模块
u16 DestroyNetSnd(ifly_netsnd_t* pSnd);

//设置网络发送参数(进行底层套结字的创建，绑定端口,以及发送目标地址的设定等动作)
u16 SetNetSndNetParam(ifly_netsnd_t* pSnd,TNetSndParam tNetSndParam);

//移除网络发送本地地址参数(进行底层套结字的删除，释放端口等动作)
u16 RemoveNetSndLocalNetParam(ifly_netsnd_t* pSnd);

//重置帧ID
u16 ResetNetSndFrameId(ifly_netsnd_t* pSnd);
//重置同步源SSRC
u16 ResetNetSndSSRC(ifly_netsnd_t* pSnd,u32 dwSSRC);

//重置发送端对于mpeg4或者H.264采用的重传处理的开关,关闭后，将不对已经发送的数据包进行缓存
u16 ResetNetSndRSFlag(ifly_netsnd_t* pSnd, u16 wBufTimeSpan, BOOL32 bRepeatSnd);

//设置发送选项
u16 SetNetSndInfo(ifly_netsnd_t* pSnd, u32 dwNetBand, u8 byFrameRate);
//发送数据包
u16 SendFrame(ifly_netsnd_t* pSnd,PFRAMEHDR pFrmHdr,int avgInterTime);//06-11

//得到状态
u16 GetNetSndStatus(ifly_netsnd_t* pSnd,TSndStatus *pSndStatus);
//得到统计
u16 GetNetSndStatistics(ifly_netsnd_t* pSnd,TSndStatistics *pSndStatistics);
//得到发送端高级设置参数(重传等)
u16 GetNetSndAdvancedInfo(ifly_netsnd_t* pSnd,TAdvancedSndInfo *pAdvancedSndInfo);

u16 DealNetSndRtcpTimer(ifly_netsnd_t* pSnd);

//csp add 2008-11-28
u16 DealNetSndFrameLoseEvent(TRtcpSDESLose *ptLose, u32 dwContext);

#ifdef __cplusplus
}
#endif

#endif
