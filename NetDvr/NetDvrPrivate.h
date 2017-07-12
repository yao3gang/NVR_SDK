#ifndef NETDVR_HEAD_PRIVATE
#define NETDVR_HEAD_PRIVATE

#if 0
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include<stdlib.h>
#include<crtdbg.h>
#include "debug_new.h" // +
#endif
#endif


#include "ctrlprotocol.h"
#include "common.h"
#include "custommp4.h"
#include "netdvr.h"
#include "avilib.h"

#include "IPHlpApi.h"
#pragma comment(lib,"IpHelper\\IPHlpApi.Lib")

#ifdef __cplusplus
extern "C"{
#endif
	
#include <jpeglib.h>
	
#ifdef __cplusplus
}
#endif

#define _NVR_

#ifndef _NVR_
//#define DEC_AVCDEC
#endif

#define DEC_HISI

#include "AudioPlay.h"
extern CAudioPlay* g_currAudioPlay;

#ifdef WIN32
#include <mmsystem.h>
#pragma comment(lib,"WINMM.LIB")
#endif

#ifdef DEC_AVCDEC
#include "avcdecoder.h" 
#pragma comment(lib,"avcdec.lib")
#endif


#ifdef DEC_HISI
#ifdef _NVR_
#include "hi_config.h"
#include "hi_h264api.h"
#pragma comment(lib,"hi_h264dec_w.lib")
#endif
#include "hi_voice_api.h"
#pragma comment(lib,"lib_VoiceEngine_static.lib")
#endif

#define MAX_DVR_NUM			256

#define MAX_ALARMIN_NUM		4
#define MAX_ALARMOUT_NUM	1

#define RCV_PORT_BASE		56000//64000
#define RCV_PORT_NUM		256
#define MAX_PLAYER_NUM		4

#define MAX_HDDNUM			8

#define MAX_VOIP_NUM		1
#define VOIP_BUF_SIZE		480
#define DAYS_OF_WEEK		7

#define REC_FILE_LEN_MAX	((128<<20) - (1<<19))//(3 << 20)
#define REC_FILE_LEN_MIN	(1 << 20)
#define PATH_LEN_MAX		MAX_PATH
#define FILENAME_LEN_MAX	FILENAME_MAX
#define NETDVR_PATH_DEFAULT "D:\\NETDVR"

#define MAX_VIDEO_WIDTH		704
#define MAX_VIDEO_HEIGHT	576


#define MAX_VIDEODECLEN			704*576*2

#define MAX_AUDIO_DECLEN		1024 //(960+(8<<10))

enum NETDVR_PORT_STATUS {
	NETDVR_PORT_SET_FREE = 0,
	NETDVR_PORT_GET_FREE,
};

enum NETDVR_RCV_TYPE {
	NETDVR_RCV_VIDEO = 0,
	NETDVR_RCV_AUDIO,
	NETDVR_RCV_PLAYCB_VIDEO,
	NETDVR_RCV_PLAYCB_AUDIO,
	NETDVR_RCV_VOIP,
	NETDVR_RCV_FILE,
	NETDVR_RCV_SUBVIDEO,
};

typedef void * Decoder_Handle;

#ifdef WIN32
typedef HANDLE MutexHandle;
#else
typedef pthread_mutex_t MutexHandle;
#endif

#ifdef _NVR_
	//csp modify 20121122
	//#define MAX_FRAME_SIZE			(INT32)(1024*1024)
	#define MAX_FRAME_SIZE				(INT32)(512*1024)
#else
	// #ifndef MAX_FRAME_SIZE
	#define MAX_FRAME_SIZE				(INT32)(256*1024)
	// #endif
#endif

typedef struct
{
    u8     m_byMediaType; //媒体类型
    u8    *m_pData;       //数据缓冲
	u32    m_dwPreBufSize;//m_pData缓冲前预留了多少空间，用于加
	// RTP option的时候偏移指针一般为12+4+12
	// (FIXED HEADER + Extence option + Extence bit)
    u32    m_dwDataSize;  //m_pData指向的实际缓冲大小
    u8     m_byFrameRate; //发送帧率,用于接收端
	u32    m_dwFrameID;   //帧标识，用于接收端
	u32    m_dwTimeStamp; //时间戳, 用于接收端
    union
    {
        struct{
			BOOL32    m_bKeyFrame;    //频帧类型(I or P)
			u16       m_wVideoWidth;  //视频帧宽
			u16       m_wVideoHeight; //视频帧宽
			u32       m_wBitRate;
		}m_tVideoParam;
        u8    m_byAudioMode;//音频模式
    };
}FRAMEHDR,*PFRAMEHDR;

struct NETDVR_alarmParam_t
{
	unsigned char flag_deal;				//1: deal with input alarm ; 0: for no
	unsigned char in_copy;					//if copy input alarm params
	unsigned char copyin;					//the alarm input channel copy to 
	alarmintype_t typein;					//input alarm type:see alarmintype_t

	unsigned char out_copy;					//if copy output alarm params
	unsigned char copyout;					//the alarm output channel copy to 	
	alarmouttype_t typeout;					//output alarm type:see alarmouttype_t
	delay_t delay;					//see alarmouttype_t
};


struct SvSpsPps  
{
	BYTE buf[50];
	BYTE buflen;
};


struct IFLY_RecordPara_t
{
	custommp4_t* file;
	char filename[FILENAME_LEN_MAX];
	char path[PATH_LEN_MAX];
	
	pFrameCallBack p_rec_cb;
	u32 dwContentRec;
	MutexHandle cb_rec_lock;
	
	pRecFilenameCallBack p_filename_cb;
	u32 dwContentFilename;
	MutexHandle cb_filename_lock;
	
//	u32 cur_length;
	u32 max_length;
	
	u8 chn;
	u8 b_record_on;
	u8 b_wait_key;
	u8 firstvideo;

	u32 first_stamp;
	u32 last_frame_id;

	MutexHandle rec_lock;

	NETDVR_AudioProperty_t* p_audio_property;
	SvSpsPps m_sps;
	SvSpsPps m_pps;
};


enum PIC_TYPE {
	PIC_TYPE_BMP = 0,					//BMP
	PIC_TYPE_JPG = 1,					//JPG
};

struct IFLY_Snapshot_t
{
	MutexHandle snap_lock;

	char filename[FILENAME_LEN_MAX];
	char path[PATH_LEN_MAX];

	u8 b_snap_on;

	enum PIC_TYPE pictype;
};


typedef struct
{
	SOCKHANDLE	sockfd;
	u8	bStart;
	u32 linkid;
	ifly_TCP_Stream_Req req;
	void* pContent;
#if 0
	DWORD tickTimeLast;
	DWORD time;
	DWORD m_dwCount;
	BOOL first;
#endif
}ifly_stearmrcv_t;


struct IFLY_MediaRcvPara_t
{
	ifly_stearmrcv_t* prcv_t;
	ifly_TCP_Stream_Req req;

	NETDVR_VideoProperty_t* p_video_property;
	NETDVR_AudioProperty_t* p_audio_property;

	CAudioPlay *pAudioPlay;

	Decoder_Handle decoder_hdr;
	hiVOICE_ADPCM_STATE_S decaudio_hdr;

#ifdef DEC_AVCDEC
	avcdYUVbuffer_s m_recoBuf;
	avcdYUVbuffer_s m_refBuf;
#endif

	BYTE bVideoDecFlag;
	BYTE bDecKeyFrameFlg;
	MutexHandle dec_lock;
	
	pFrameCallBack pFrameCB;
	u32 dwContentRaw;
	MutexHandle cb_raw_lock;
	
	pDecFrameCallBack pDecFrameCB;
	u32 dwContent;
	MutexHandle cb_dec_lock;
	
	fmt_type_t fmt;

	u8 rcv_type;

	struct IFLY_RecordPara_t *p_record_para;
	struct IFLY_Snapshot_t *p_snapshot;

	struct IFLY_RecordPara_t *p_record_para2;

	BOOL bPreviewAudioMute;

	u32 dwOldVideoFrameId;
	u32 bInterrupted;

	ifly_spspps_t  spspps;

	unsigned char* p_frmdecBuf;

	u16 wCurrFrmWidth;
	u16 wCurrFrmHeight;
	unsigned char byDeinterlacing;

	BYTE bOpened;

	//csp modify
	u16 decoder_width;
	u16 decoder_height;
};

struct IFLY_MediaSndPara_t
{

	ifly_stearmrcv_t* psnd_t;	
	NETDVR_VOIPProperty_t*	p_voip_property;
	HWAVEIN m_hwi;
	WAVEHDR m_wh1;
	BYTE m_RecBuf1[MAX_AUDIO_DECLEN];
	WAVEHDR m_wh2;
	BYTE m_RecBuf2[MAX_AUDIO_DECLEN];
	BOOL m_bInterPhoneEnding;
	FRAMEHDR m_InterPhoneFrmHdr;

	BYTE sndmode;
	pDecFrameCallBack pDecFrameCB;
	u32 dwContent;

};

struct IFLY_Player_t
{
 	u32 m_PlayId;

	struct IFLY_MediaRcvPara_t pb_rcv;

	PFUN_MSG_T p_cb_playover;
	u32 dwContentPlayover;
	PFUN_MSGHASAUDIO_T p_cb_hasaudio;
	u32 dwContentHasaudio;
	PFUN_PROGRESS_T p_cb_progress;
	u32 dwContentProgress;

	struct NETDVR_progressParam_t progress;
};

struct IFLY_FileReciever_t
{
	ifly_stearmrcv_t* prcv_t;
	ifly_TCP_Stream_Req req;
	void *pDeviceHandle;

	char save_path[PATH_LEN_MAX];
	char save_filename[FILENAME_LEN_MAX];

	u32 recieved_size;
	u32 rcv_size;;

	FILE *fp;

	PFUN_PROGRESS_T p_cb_progress;
	u32 dwContentProgress;

	PFUN_TIMEDLPROGRESS_T p_cb_timedlprogress;
	u32 dwContentTimeDlProgress;

	PFUN_SAVE_T p_cb_save;
	u32 dwContentSav;

	PFUN_ERROR_T p_cb_err;
	u32 dwContentErr;

	HANDLE reciever_handle;
	u32 serverip;

	MutexHandle reciever_lock;

	NETDVR_AudioProperty_t*	p_audio_property;
	u8 bOpened;
	struct IFLY_FileReciever_t* pNext;
};


#define NETDVR_UPDATE_RUN 0
#define NETDVR_UPDATE_SUCCESS 1
#define NETDVR_UPDATE_REQ_STOP 2 
#define NETDVR_UPDATE_STOPPED 3
#define NETDVR_UPDATE_EXIT 4
#define NETDVR_UPDATE_FLASH_WRITE 5

struct IFLY_Update_t
{
	ifly_stearmrcv_t* prcv_t;
	ifly_TCP_Stream_Req req;

	u16 error_code;
	u8 b_updating;
	u8 status;

	char update_path[PATH_LEN_MAX];
	char update_filename[FILENAME_LEN_MAX];

	PFUN_PROGRESS_T p_cb_progress;
	unsigned int dwContentUpdate;

	u32 updated_size;
	u32 file_size;
	HANDLE update_handle;
	FILE *fp;


	PFUN_ERROR_T p_cb_err;
	u32 dwContentErr;
	MutexHandle update_lock;
};

struct NETDVR_Command_t
{
	u16 command;					//the commad type
	u8 b_chk_login;					//check login before send command
	u8 b_send_user;					//send user name
	char *cmd_param;				//send command with the parameters
	u32 cmd_param_len;				//the length of the command parameters
	
	char *cmd_ret_param;			//the return parameters of the command
	u32 cmd_ret_maxlen;				//the max length of the return parameters

	u32 timeout;
};

#define MAX_SERIALPORT	2
enum SERIALPORT
{
	SERIAL_232=1, 
	SERIAL_485=2
};

struct IFLY_SERIAL_T
{
	enum SERIALPORT serialport;
	ifly_stearmrcv_t* prcv_t;
	ifly_TCP_Stream_Req req;
	HANDLE hRcvthread;
	pSerialDataCallBack cbSerialDataCallBack;
	unsigned int dwContent;
	u8 bOpened;
};

//远程格式化
struct IFLY_FORMATHDD_T
{
	u8 hddindex;
	ifly_stearmrcv_t* prcv_t;
	//ifly_TCP_Stream_Req req;
	HANDLE hRcvthread;
	pFormatProgressCallBack pCBFmtProgress;
	unsigned int dwContent;
	ifly_TCP_Pos progress;
	u8 bOpened;
};

struct IFLY_REMOTESNAP_T 
{
	ifly_stearmrcv_t* prcv_t;
	char filename[MAX_PATH*2];
};

#define MEDIA_LINK_CLIENT	128

struct ifly_monitor_t
{
	u8 channel;
	void* pDeviceHandle;
	struct IFLY_MediaRcvPara_t video_rcv;
	struct IFLY_MediaRcvPara_t audio_rcv;
	struct IFLY_RecordPara_t record_para;
	struct IFLY_RecordPara_t record_para2;
	struct IFLY_Snapshot_t snap_shot;
	struct ifly_monitor_t* pNext;
};

struct ifly_playback_t
{
	u8	playtype; //0 文件 1 时间
	void* pDeviceHandle;
	struct IFLY_Player_t player;
	MutexHandle player_lock;
	struct ifly_playback_t* pNext;
};

#define MAX_RECFIND 24
struct PRI_RecFind_t
{
	char flag[8];		//"recfind"			开头的标识符
	ifly_recsearch_param_t  findcond;		//保存的是网络序
	ifly_recfileinfo_t	finddatabuf[MAX_RECFIND];
	unsigned short sum;						//total records of found logs
	unsigned short startID;					//if no file is indexed, startID will be 0, or it'll be a value based on index condition's startID(struct NETDVR_logSearchCondition_t)
	unsigned short endID;					//结束的记录,基址为1.当endID和startID相等时,表示之返回一条记录
	unsigned short currentfileindex;		//当前文件序号 0开始
	struct NETDVR_INNER_t *pDeviceHandle;	//保存设备句柄
};

struct PRI_MotionFind_t
{
	char flag[8];		//"mdfind"			开头的标识符
	PRI_RecFind_t	tRecInfo;
	unsigned int	dwCurrFileMotionTime;	//当前文件的移动侦测当前时间 （UTC1970.1.1）
	PRI_MotionFind_t* pNext;
};

struct NETDVR_INNER_t
{
	CRITICAL_SECTION m_hcs;
	u16 dvr_id;
	u8 b_login;
	u8 b_cmdConnectLost;
	CPHandle cph;
	
	BOOL bRcvThreadState;
	HANDLE hRcvEvent;
	HANDLE hRcvThread;
	ifly_stearmrcv_t m_mediarcv[MEDIA_LINK_CLIENT];

	struct NETDVR_DeviceInfo_t si;
	struct NETDVR_loginInfo_t li;
//	struct NETDVR_extloginInfo_t extli;

	NETDVR_VideoProperty_t	video_property;
	NETDVR_AudioProperty_t	audio_property;
	NETDVR_VOIPProperty_t	voip_property;

	struct ifly_monitor_t* m_pMonitor;
	struct ifly_playback_t* m_pPlayBack;

	struct IFLY_FileReciever_t file_reciever;
	struct IFLY_FileReciever_t* filereciever;
	struct IFLY_Update_t update;
	
// 	struct IFLY_Player_t player[MAX_PLAYER_NUM];
// 	MutexHandle player_lock[MAX_PLAYER_NUM];

	struct IFLY_MediaRcvPara_t voip_rcv[MAX_VOIP_NUM];
	struct IFLY_MediaSndPara_t voip_snd[MAX_VOIP_NUM];

// 	struct IFLY_Player_t getfileframe[MAX_PLAYER_NUM];
// 	MutexHandle getfileframe_lock[MAX_PLAYER_NUM];

	PFUN_MSG_T p_cb_connlost;
	u32 dwContentConnlost;

	PFUN_SearchDevice_CB p_cb_searchdevice;
	u32 dwSearchDevice;
	
	PFUN_ALARMSTATE_T p_cb_alarmstate;
	struct NETDVR_AlarmUploadState_t alarmstate;
	u8 byAlarmUploadFlag;
	u32 dwAlarmStateContent;

	u32 b_voip;

	struct IFLY_SERIAL_T serial[MAX_SERIALPORT];

	struct IFLY_FORMATHDD_T fmt_hdd[MAX_HDDNUM];
	
	struct IFLY_REMOTESNAP_T remotesnap;

	u8 bEnableRecon;

	pCBReconnMsg p_cb_recconn;
	u32 dwContentReconn;

	char SvrDomain[128];

	PRI_MotionFind_t* m_pMDList;
};

struct NETDVR_DVR_POOL
{
	unsigned int count;
	MutexHandle port_lock;
	
	struct NETDVR_INNER_t *p_dvr[MAX_DVR_NUM];
	MutexHandle dvr_lock[MAX_DVR_NUM];
}; 


extern struct NETDVR_DVR_POOL g_dvr_pool;
extern MutexHandle g_pool_lock;
extern unsigned int g_connect_timeout;
extern BYTE g_dvrExist[MAX_DVR_NUM];
extern DWORD g_dwReconnectTime;

#define USE_ADPCM
#ifdef USE_ADPCM
typedef struct Tagadpcm_state {
    short	valprev;	/* Previous output value */
    short	index;		/* Index into stepsize table */
}adpcm_state;

/* Intel ADPCM step variation table */
static int indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
		-1, -1, -1, -1, 2, 4, 6, 8,
};

static int stepsizeTable[89] = {
		7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
		19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
		50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
		130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
		337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
		876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
		2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
		5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
		15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};
#endif

#define WAIT_WAVE_OUT_DONE
#define AUDIO_PLAY_BUFNUM (2)
#define AUDIO_PLAY_BUFLEN (10 << 10)//(10 << 10)
#define AUDIO_PLAY_INITBUF (480 << 4)

static HWAVEOUT g_m_hwo = NULL;
static char g_audio_in_buf[AUDIO_PLAY_BUFLEN];
static int g_audio_buf_used = 0;
static MutexHandle g_audio_buf_lock;
static WAVEHDR g_hWaveHdr[AUDIO_PLAY_BUFNUM];
static char g_audio_play_buf[AUDIO_PLAY_BUFNUM][AUDIO_PLAY_BUFLEN];

//static unsigned char s_frame_dec[704 * 576 * 4];


typedef struct
{
	u8   *buf;
	u32  unitNum;
	u32  unitLen;
	u32  rpos;
	u32  wpos;
	HANDLE lock;
	HANDLE rsem;
	HANDLE wsem;
}ifly_msgQ_t;

extern ifly_msgQ_t g_decmsgQ;

typedef struct
{
	u8		m_byMediaType; //媒体类型
	u8		m_pData[MAX_FRAME_SIZE];       //数据缓冲
	u32		m_dwPreBufSize;//m_pData缓冲前预留了多少空间，用于加
	// RTP option的时候偏移指针一般为12+4+12
	// (FIXED HEADER + Extence option + Extence bit)
	u32		m_dwDataSize;  //m_pData指向的实际缓冲大小
	u8		m_byFrameRate; //发送帧率,用于接收端
	u32		m_dwFrameID;   //帧标识，用于接收端
	u32		m_dwTimeStamp; //时间戳, 用于接收端
	union
	{
		struct{
			BOOL32    m_bKeyFrame;    //频帧类型(I or P)
			u16       m_wVideoWidth;  //视频帧宽
			u16       m_wVideoHeight; //视频帧宽
			u32       m_wBitRate;
		}m_tVideoParam;
		u8    m_byAudioMode;//音频模式
	};
}FRAMEHDR2;

typedef struct
{
	FRAMEHDR2 frame;
 	IFLY_MediaRcvPara_t *p_MediaRcvPara;
	u32 dvr_id;
}FrameMsg_t;

#define MAX_SAVE_FRM 128

extern HANDLE g_hDecVideoThread;
extern BOOL g_bDecVideoThreadRun;
extern HANDLE g_hDecThreadEvent;

extern HANDLE g_hReConnectThread;
extern BOOL g_bReConnectThreadRun;
extern HANDLE g_hReConnectThreadEvent;

//#define USE_CONNMSG_THREAD

#ifdef USE_CONNMSG_THREAD
#define STARTCONNECTMSG WM_USER+100
#define STOPCONNECTMSG WM_USER+101
#define THREAD_QUIT		WM_USER+105
extern HANDLE g_hConnMsgThread;
extern BOOL g_bConnMsgThreadRun;
extern HANDLE g_hConnMsgThreadEvent;
extern DWORD g_dwConnMsgThreadID;
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////			Functions             ////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// MutexHandle
//////////////////////////////////////////////////////////////////////////
BOOL CreateMutexHandle(MutexHandle *pMutex);
BOOL CloseMutexHandle(MutexHandle hMutex);
BOOL LockMutex(MutexHandle hMutex);
BOOL UnlockMutex(MutexHandle hMutex);
// MutexHandle
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// common function
//////////////////////////////////////////////////////////////////////////
int iflydvr_get_error_code(int event);
BOOL create_directory(LPTSTR lpPath);
int set_out_path(char *p_path, const char *p_dir_path);

int setSocketNoDelay(SOCKET hSock);
int GetIPGUID(u32 ip, CPGuid* guid);
#if 0
int config_port_free(unsigned short *port, int flag);
#endif
int get_local_ip(CPHandle cph, struct sockaddr_in *p_addr);
int set_ip(CPHandle cph, u32 *p_set, u32 ip);
int set_portEx(u16 *p_set, u16 port, u8 rcvtype, u8 chn);
#if 0
int set_port(u16 *p_set, u16 port);
#endif



enum PortType {UDPPORT, TCPPORT};
BOOL IsPortUsed(WORD wPort, enum PortType byPortFlag);
WORD GetOneUnUsingPort(WORD wInPort, enum PortType byPortFlag);

// common function
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// command and notify
//////////////////////////////////////////////////////////////////////////
#if 0
int command_server(int Handle, const struct NETDVR_Command_t *p_command);
#endif
int send_command(CPHandle cph, u16 command, const void *in_data, u32 in_length, void *out_data, u32 max_out_length, u32 timeout);
int send_command_noout(CPHandle cph, u16 command, const void *in_data, u32 in_length, u32 timeout);

int ctrl_player(int Handle, int linkid_n, u16 command, char *pbuf = NULL, u32 para_len=0);

void get_notify_dvr(CPHandle svrcph, struct NETDVR_INNER_t **pp);
void DealNotify(CPHandle svrcph, u16 event, u8 *pbyMsgBuf, int msgLen, u8 *pbyAckBuf, int *pAckLen, void* pContext);
// command and notify
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// deal media frame
//////////////////////////////////////////////////////////////////////////
void DealMediaFrame(PFRAMEHDR pFrmHdr, unsigned int dwContext);
BOOL is_video_frame(u8 media_type);
// deal media frame
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Audio 
//////////////////////////////////////////////////////////////////////////
BOOL open_audio_out(u8 audio_mode, u16 samplepersec);
void close_audio_out(void);
void deal_play_audio(struct IFLY_MediaRcvPara_t* p_MediaRcvPara, pFrameHeadrDec pFrmHdr);

void adpcm_decoder(char indata[], short outdata[], int len,adpcm_state *state);
int audio_dec(PFRAMEHDR p_framehdr, unsigned char *p_buf);

void deal_audio_rcv(PFRAMEHDR p_framehdr, struct IFLY_MediaRcvPara_t* p_MediaRcvPara, unsigned char *p_buf);
void deal_playcb_audio_rcv(PFRAMEHDR p_framehdr,  struct IFLY_MediaRcvPara_t* p_MediaRcvPara, unsigned char *p_buf);

int open_voip_sender(struct IFLY_MediaSndPara_t *p_sender);
int deal_voip_rcv(PFRAMEHDR p_framehdr, struct IFLY_MediaRcvPara_t* p_MediaRcvPara, unsigned char *p_buf);
#ifdef WAIT_WAVE_OUT_DONE
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
#endif
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
// Audio
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// video
//////////////////////////////////////////////////////////////////////////

int open_reciever(struct IFLY_MediaRcvPara_t *p_reciever, pFrameCallBack pCBFun, unsigned int dwContent, DWORD dwRtpIP);
int close_reciever(struct IFLY_MediaRcvPara_t *p_reciever);

void deal_video_rcv(PFRAMEHDR pFrmHdr, struct IFLY_MediaRcvPara_t *p_MediaRcvPara, u8 *p_buf);
void deal_playcb_video_rcv(PFRAMEHDR pFrmHdr, struct IFLY_MediaRcvPara_t *p_MediaRcvPara, u8 *p_buf);

void reg_rcvcb_dec(struct IFLY_MediaRcvPara_t *p_reciever, pDecFrameCallBack pCBFun, unsigned int dwContent);
void unreg_rcvcb_dec(struct IFLY_MediaRcvPara_t *p_reciever);

void create_rcvvideo_decoder(struct IFLY_MediaRcvPara_t *p_reciever, NETDVR_VideoProperty_t* p_video_property);
int set_rcvvideo_decoder_fmt(struct IFLY_MediaRcvPara_t *p_reciever, fmt_type_t fmt);
void destroy_rcvvideo_decoder(struct IFLY_MediaRcvPara_t *p_reciever);

void CALLBACK deal_record_filename(char *p_filename, u32 dwContent);
void CALLBACK deal_frame_record(pFrameHeadr pFrmHdr, u32 dwContent);
// video
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// snapshot
//////////////////////////////////////////////////////////////////////////
void set_default_snap(struct IFLY_Snapshot_t *p_snap, int dvrid, u8 chn);
void snapshot_rgb_2bmp(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot, u8 bit_count);
void snapshot_yuv_2bmp(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot, u8 yuv_type);
void snapshot_rgb_2jpg(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot, u8 factor);
void snapshot_yuv_2jpg(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot, u8 yuv_type);
void do_snapshot(FrameHeadrDec *p_framehdrdec, struct IFLY_Snapshot_t *p_snapshot);
// snapshot
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// logininfo
//////////////////////////////////////////////////////////////////////////
void set_logininfo(struct NETDVR_INNER_t *p, const struct NETDVR_loginInfo_t *pLoginInfo);
// logininfo
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// download
//////////////////////////////////////////////////////////////////////////

void open_reciever_file(struct IFLY_FileReciever_t *p_reciever);
// download
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// upload
//////////////////////////////////////////////////////////////////////////
FILE *open_update_file(struct IFLY_Update_t *p_update);

// upload
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// tcp rcv
//////////////////////////////////////////////////////////////////////////

DWORD WINAPI RcvTcpFrameThread(LPVOID lpParam);
BOOL SetRcvTcpFrame(struct NETDVR_INNER_t *p, ifly_stearmrcv_t** pprcv_t, ifly_TCP_Stream_Req req, BOOL bRcv, void* pContent =NULL, u32 *pErrcode = NULL);
// tcp rcv
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// fpga(R7016) video decode
//////////////////////////////////////////////////////////////////////////
#ifdef DEC_AVCDEC
void _stdcall MsgOut(void *user , char * msg);
void *my_aligned_malloc(int size, int alignment);
void *my_aligned_free(void *paligned);
#endif

#ifdef DEC_AVCDEC
int _stdcall My_avcdDecodeOneNal(avcdDecoder_t *dec, unsigned char *nalUnitBits, int nalUnitLen,
		avcdYUVbuffer_s *outBuf  , avcdYUVbuffer_s * refBuf, unsigned char keyflag);
#endif

ifly_stearmrcv_t* AddStreamConnect(struct NETDVR_INNER_t *p, ifly_TCP_Stream_Req req, void* pContent, u32 *pErrcode = NULL);
BOOL StartStreamConnect(ifly_stearmrcv_t* prcv_t, u32 ip, u16 port, ifly_TCP_Stream_Req req, u32 *pErrcode = NULL);
void DelStreamConnect(ifly_stearmrcv_t* prcv_t);


BOOL initMsgQ(ifly_msgQ_t *mq,u32 unitNum,u32 unitLen);
int  readMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 readLen);
int  writeMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 writeLen);
BOOL destroyMsgQ(ifly_msgQ_t *mq);

BOOL GetMsgQReadInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pReadLen);
BOOL skipReadMsgQ(ifly_msgQ_t *mq);

BOOL GetMsgQWriteInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pWriteLen);
BOOL skipWriteMsgQ(ifly_msgQ_t *mq);

DWORD WINAPI DecVideoFrameThread(LPVOID lpParam);

DWORD WINAPI DownLoadThread(LPVOID lpParam);
DWORD WINAPI UpdateThread(LPVOID lpParam);
DWORD WINAPI SerialRcvThread(LPVOID lpParam);
DWORD WINAPI FormatHddThread(LPVOID lpParam);


DWORD WINAPI ReConnectThread(LPVOID lpParam);

DWORD WINAPI TimeDownLoadThread( LPVOID lpParam );

#ifdef USE_CONNMSG_THREAD
DWORD WINAPI DealConnectMsgThread(LPVOID lpParam);
#endif

DWORD Domain2IP(char *pDomain);

//csp modify
//int looprecv(SOCKET s, char * buf, unsigned int rcvsize);
int loopsend(SOCKET s, char * buf, unsigned int sndsize);

int loopread(FILE* file, char * buf, unsigned int read_size);
int loopwrite(FILE* file, char * buf, unsigned int write_size);

BOOL ChkMediaRcvPointer(NETDVR_INNER_t* pDevice, IFLY_MediaRcvPara_t *p_MediaRcvPara);

void CleanMsgQ(ifly_msgQ_t *mq, IFLY_MediaRcvPara_t *p_MediaRcvPara);

BOOL ChkRealHandle(int nRealHandle);
BOOL ChkPlayBackHandle( int nPlayBackHandle );
//////////////////////////////////////////////////////////////////////////
#endif //NETDVR_HEAD_PRIVATE

