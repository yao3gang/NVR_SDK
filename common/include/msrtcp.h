#ifndef _RTCP_H_
#define _RTCP_H_

#include "iflytype.h"

#include "mssend.h"
#include "msrecv.h"
#include "mssocket.h"

#ifdef __cplusplus 
extern "C"{
#endif /* __cplusplus */

#pragma pack(push,1)//对应???

typedef struct ifly_rtcp_t
{
	ifly_socket_t* m_hSocketSnd;
	ifly_socket_t* m_hSocketRcv;
	TAddr       m_tLocalAddr;
	TRemoteAddr m_tRemoteAddr;
	int			m_nLastSndPackNum;
	int			m_nLastRcvPackNum;
}ifly_rtcp_t;

/*RTCP 报文负载类型常量*/
#define RTP_RTCP_SR   200
#define RTP_RTCP_RR   201

/* RR 接收者报告包的项目块数据结构, 也即发送者报告包数据结构, 两者完全相同. */
typedef struct RECEIVE_REPORT_PACKET_STRU
{
    /* 发起此报告包的源 */
    unsigned int udwSsrc;          
    
    /* 从收到上个SR/RR包后, 丢包比率 */
    unsigned int bit8Fraction:8;   
    
    /* 自从接收以来,来自该源的丢包总数 */
    unsigned int bit24LostNum:24;  
    
    /* 从该源收到的RTP包的最大序列号. 高16-bit是序列号周期的相应值(即回卷次数),
    低16-bit是收到的RTP包的最高序列号. */
    unsigned int udwLastSeq;       
    
    /* 数据包到达间隔时间统计变量估计值 */
    unsigned int udwJitter;        
    
    /* 从该源收到的上一个RTCP SR包的NTP时戳的中间32-bit */
    unsigned int udwLsr;           
    
    /* 从该源收到上个RR包到发送此包之间的间隔, 单位时1/65536秒 */
    unsigned int udwDlsr;          
} RR_LIST, SR_LIST;

/* 发送者报告包(SR) */
typedef struct SEND_REPORT_STRU 
{
    unsigned int udwSsrc;               
    unsigned int udwNtpMsw;             
    unsigned int udwNtpLsw;
    unsigned int udwRtpTimeStamp;              
    unsigned int udwSentRtpNum;              
    unsigned int udwSentRtpOctetCount;              
    //SR_LIST    SrList[1];
} SEND_REPORT;

/* 接收者报告包(RR) */
typedef struct RECEIVE_REPORT_STRU 
{
	unsigned int udwSsrc;      
    RR_LIST    RrList[1];   
} RECEIVE_REPORT;

/* 5种类型包联合:SR,RR,SDSE,BYE,APP. */
typedef union RTCP_PACKET_TYPE_STRU 
{
    /* 发送者报告包(SR) */
    SEND_REPORT SR;
	
    /* 接收者报告包(RR) */        
    RECEIVE_REPORT RR;
	
    /* 源描述符报告包(SDES) */
    //SDES_REPORT SDES;
	
    /* 指示与会者退出报告包(BYE) */
    //BYE_REPORT BYE;
} RTCP_PACKET_TYPE;

/* RTCP控制包包头结构, 发送任何RTCP包, 都必须带有如下定义的RTCP包头 */
typedef struct RTCP_COMMON_HEAD_STRU
{
	/*#ifdef _WIN95 */  /* 操作系统是WIN95 */ 
    BYTE bit5Count:5;   /* 接收/发送报告块的数量 */
    BYTE bit1P    :1;   /* RTCP包结尾填充数据标志位 */
    BYTE bit2V    :2;   /* RTCP协议版本号 */
	/*#else*/
//	BYTE bit2V    :2;   /* RTCP协议版本号 */
//	BYTE bit1P    :1;   /* RTCP包结尾填充数据标志位 */
//	BYTE bit5Count:5;   /* 接收/发送报告块的数量 */
	/*#endif*/
	
    BYTE  ucPT;         /* RTCP包类型 */
    unsigned short uwLen;/* 包的长度, 字是32-bit的字 */
} RTCP_COMMON_HEAD;

/* RTCP包的数据结构*/
typedef struct RTCP_STRU
{
    /* RTCP包头 */
    RTCP_COMMON_HEAD CommonHead;                
	
	/* 5种类型包联合:SR,RR,SDSE,BYE,APP. */
    RTCP_PACKET_TYPE RtcpType;
} RTCP_PACKET;

//VideoRtcpTask线程函数参数
typedef struct RTCPPARAM 
{
	ifly_rtcp_t* pRTCP;
	ifly_netsnd_t* pSnd;
	ifly_netrcv_t* pRcv;
} RTCPParam;

#pragma pack(pop)//对应???

//设置是否需要RTCP
//False的话可以创建销毁RCTP的socket，但不发包接收包
//放在CloseRTCPRcvSnd之前即可
void SetNeedRTCP(BOOL bNeed);


//创建RTCP的socket
ifly_rtcp_t* CreateRTCPRcvSnd();

//设置RTCP参数应该在设置好RTP参数之后
//设置RCTP发送接收的IP,端口，绑定
u16 SetRTCPNetParam(ifly_rtcp_t* pRTCP,TNetSndParam* tNetSndParam, TNetRcvParam* tNetRcvParam);

//发送SR包，隔一段时间发送
u16 SendRTCPSR(ifly_rtcp_t* pRTCP,ifly_netsnd_t* pSnd);

//发送RR包
u16 SendRTCPRR(ifly_rtcp_t* pRTCP,ifly_netrcv_t *pRcv, BYTE bylostrate);

//线程函数，用来SR的接收以及RR的发送接收
void *VideoRtcpTask(void* param);

//关闭RCTPsocket，pRTCP置为NULL
void CloseRTCPRcvSnd(ifly_rtcp_t* pRTCP);

//回调函数定义，参数为TRUE上调码流，False下调
typedef void (*PSETVIDEORATEUP)(BOOL bUp);

//注册回调函数
void SetCBsSetVideoRate(PSETVIDEORATEUP pSetVideoRateCB);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _RTCP_H_ */
