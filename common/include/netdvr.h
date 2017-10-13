#if !defined(AFX_NETDVRSDK_H__363D9034_1695_488A_B57F_949BD9F5CA79__INCLUDED_)
#define AFX_NETDVRSDK_H__363D9034_1695_488A_B57F_949BD9F5CA79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef __cplusplus
extern "C"{
#endif

enum NETDVR_RETURN_CODE	//
{
	NETDVR_SUCCESS     = 0,					
	NETDVR_REC_STATUS_STARTED,				
	NETDVR_REC_STATUS_STOPPED,				
	NETDVR_ERROR = 2000,					
	NETDVR_ERR_UNKNOWN,						
	NETDVR_ERR_PARAM,						
	NETDVR_ERR_RET_PARAM,					
	NETDVR_ERR_NOINIT,
	NETDVR_ERR_COMMAND, 
	NETDVR_ERR_NEW_CONNECTION,
	NETDVR_ERR_SEND,
	NETDVR_ERR_OUTOFMEMORY,
	NETDVR_ERR_RESOURCE,
	NETDVR_ERR_FILENOTEXIST, 
	NETDVR_ERR_BAUDLIMIT,						//max = 5
	NETDVR_ERR_CREATESOCKET,					//create socket error
	NETDVR_ERR_SOCKET,							//socket error
	NETDVR_ERR_CONNECT,							//connect error
	NETDVR_ERR_BIND,							//bind error -1985
	NETDVR_ERR_LISTEN,		
	NETDVR_ERR_NETSND,							//send error
	NETDVR_ERR_NETRCV,							//recv error
	NETDVR_ERR_TIMEOUT,							//timeout 
	NETDVR_ERR_CHNERROR,						// > channel limit -1980
	NETDVR_ERR_DEVICEBUSY,						//device bush
	NETDVR_ERR_WRITEFLASH,						//erase error 
	NETDVR_ERR_VERIFY,							//verify error 
	NETDVR_ERR_CONFLICT,						//system resource conflict
	NETDVR_ERR_BUSY,							//system busy-1975
	NETDVR_ERR_USER_SAME,						//user name conflict
	NETDVR_ERR_LINKLIMIT,
	NETDVR_ERR_DATABASE,
	/* === error code for login === */
	NETDVR_ERR_NOUSER,
	NETDVR_ERR_PASSWD,
	NETDVR_ERR_MACADDR, 
	NETDVR_ERR_RELOGIN,
	NETDVR_ERR_NOLOGIN,
	/* === net player === */
	NETDVR_ERR_NETDVR_PLAYER_FULL,
	/* === updateing ==== */
	NETDVR_ERR_UPDATING,
	/* === remote file download error === */
	NETDVR_ERR_DOWNLOAD,
	NETDVR_ERR_FILEOPEN,
	NETDVR_ERR_USERSTOPPED,
	NETDVR_ERR_SERIAL_REOPEN,
	NETDVR_ERR_GET_LOCALMACADDR,
	NETDVR_ERR_SDK_CHECKFAILED,
	NETDVR_ERR_RERECIVE,

};

//录像查找回放类型
enum NETDVR_REC_INDEX_MASK {
	NETDVR_REC_INDEX_TIMER = 0x1,
	NETDVR_REC_INDEX_MD = 0x2,
	NETDVR_REC_INDEX_ALARM = 0x4,
	NETDVR_REC_INDEX_HAND = 0x8,
	NETDVR_REC_INDEX_ALL = 0x10,
};

// #ifndef MEDIA_TYPE
// #define MEDIA_TYPE
//图像编码类型
#define  MEDIA_TYPE_H264		(BYTE)98//H.264//可能是109?
#define  MEDIA_TYPE_MP4			(BYTE)97//MPEG-4
#define  MEDIA_TYPE_H261		(BYTE)31//H.261
#define  MEDIA_TYPE_H263		(BYTE)34//H.263
#define  MEDIA_TYPE_MJPEG		(BYTE)26//Motion JPEG
#define  MEDIA_TYPE_MP2			(BYTE)33//MPEG2 video

//语音编码类型
#define	 MEDIA_TYPE_MP3			(BYTE)96//mp3
#define  MEDIA_TYPE_PCMU		(BYTE)0//G.711 ulaw
#define  MEDIA_TYPE_PCMA		(BYTE)8//G.711 Alaw
#define	 MEDIA_TYPE_G7231		(BYTE)4//G.7231
#define	 MEDIA_TYPE_G722		(BYTE)9//G.722
#define	 MEDIA_TYPE_G728		(BYTE)15//G.728
#define	 MEDIA_TYPE_G729		(BYTE)18//G.729
#define	 MEDIA_TYPE_RAWAUDIO	(BYTE)19//raw audio
#define  MEDIA_TYPE_ADPCM		(BYTE)20//adpcm
#define  MEDIA_TYPE_ADPCM_HISI	(BYTE)21//adpcm-hisi
// #endif//MEDIA_TYPE


/* ========================== enum of decoded format type (supported currently)============================= */
//解码视频格式
typedef enum NETDVR_FMT_TYPE {
		NETDVR_FMT_RGB24 = 2,					//rgb24 format
		NETDVR_FMT_RGB32 = 4,					//rgb32 format
		NETDVR_FMT_YV12 = 6,					//yv12 format
		NETDVR_FMT_I420 = 8,					//i420 format
		NETDVR_FMT_YUY2 = 10,					//yuy2 format(snapshot is not supported currently)
} fmt_type_t;

/* ========================== structure for each encoded frame ============================= */
//压缩码流帧结构
typedef struct FrameHeadr
{
	unsigned char mediaType;			//encoded (video/audio) media type:
	unsigned char *pData;				//encoded data buffer
	unsigned int preBufSize;			//pre buffer size, normally it is 12+4+12
	// (FIXED HEADER + Extence option + Extence bit)
	unsigned int dataSize;				//actual buffer size pointed by pData
	unsigned char frameRate;			//frame rate, used on receive part. 
	unsigned int frameID;				//fram id，used on receive part. 
	unsigned int timeStamp;				//time stamp, used on receive part. 
	union
	{
		struct{
			int keyFrame;				//I(1) or P(0)
			unsigned short videoWidth;	//video width
			unsigned short videoHeight;	//video height
		} videoParam;
		unsigned char audioMode;		//8, 16, or 32 bit
	};
} FrameHeadr;
typedef FrameHeadr* pFrameHeadr;

/* ========================== structure for each decoded  frame ============================= */
//解码码流帧结构
typedef struct FrameHeadrDec
{
	unsigned char mediaType;			//original encoded (video/audio) media type:
	char reserved1[3];					//reserved
	void *data;							//decoded data buf
	unsigned int data_size;				//decoded data length
	char reserved2[32];					//reserved for extensible development
	union
	{
		struct{
			fmt_type_t fmt;				//decoded format
			unsigned short width;		//video width
			unsigned short height;		//video height
		} video_param;
		unsigned char audio_mode;		//8, 16, or 32 bit
	};
} FrameHeadrDec;
typedef FrameHeadrDec* pFrameHeadrDec;

/* =============== callback function type for user register  ============= */

// function ponter type for callback raw data
typedef void (CALLBACK*   pFrameCallBack)(pFrameHeadr pFrmHdr, unsigned int dwContext);
// function ponter type for callback decoded data
typedef void (CALLBACK*   pDecFrameCallBack)(pFrameHeadrDec pFrmHdrDec, unsigned int dwContext);
// function ponter type for callback filename
typedef void (CALLBACK*   pRecFilenameCallBack)(char *p_filename, unsigned int dwContext);

#pragma pack( push, 1 )
//按1字节对齐


/* ========================== structure for the DVR information ============================= */
struct NETDVR_DeviceInfo_t
{
	unsigned int	deviceIP; 						//设备IP  
	unsigned short	devicePort;						//设备端口 
	char			device_name[32];				//设备名称
	char			device_mode[32];				//设备型号
	unsigned char	maxChnNum;						//最大通道数
	unsigned char	maxAudioNum;					//最大音频数
	unsigned char	maxSubstreamNum;				//最大子码流数
	unsigned char	maxPlaybackNum;					//最大回放数
	unsigned char	maxAlarmInNum;					//最大报警输入数
	unsigned char	maxAlarmOutNum;					//最大报警输出数
	unsigned char	maxHddNum;						//最大硬盘数
	unsigned char	nNVROrDecoder;	//区别NVR和解码器--- 跃天1NVR; 2解码器
	unsigned char	reserved[15];					//预留
};

struct NETDVR_VideoProperty_t 
{
	unsigned char	videoEncType;					//视频编码类型
	unsigned short	max_videowidth;					//最大视频宽
	unsigned short	max_videoheight;				//最大视频高
	unsigned char	reserved[3];					//预留
};

struct NETDVR_AudioProperty_t
{
	unsigned char	audioEnctype;					//预览音频编码类型
	unsigned char	audioBitPerSample;				//预览音频位数
	unsigned short	audioSamplePerSec;				//预览音频采样率
	unsigned short	audioFrameSize;					//预览音频每帧大小
	unsigned short	audioFrameDurTime;				//预览音频每帧间隔
	unsigned char	reserved[4];					//预留
};

struct NETDVR_VOIPProperty_t 
{
	unsigned char	VOIPBitPerSample;				//语音对讲位数
	unsigned short VOIPSamplePerSec;				//语音对讲采样率
	unsigned short VOIPFrameSize;					//语音对讲每帧大小
	unsigned char	reserved[3];					//预留
};

struct NETDVR_MDProperty_t 
{
	unsigned char	MDCol;							//移动侦测列数
	unsigned char	MDRow;							//移动侦测行数
	unsigned char	reserved[4];					//预留
};

/* ========================== structure for client's login information ============================= */
struct NETDVR_loginInfo_t
{
	char username[12];					//username for login
	char loginpass[12];					//password for login
	char macAddr[18];					//client mac address
	unsigned int ipAddr;				//client ip address
};

//csp modify 20130519
struct NETDVR_AdvPrivilege_t
{
	char	username[12];				//用户名
	char	reserved1[4];
	u64		nRemoteView[1];				//远程预览权限
	u64		reserved2[29];
};

/* ========================== 子码流参数 ============================= */
typedef enum NETDVR_SUBFLAG 
{
	NETDVR_SUBFLAG_CIF = 0,
	NETDVR_SUBFLAG_QCIF = 1,	
}subflag_t;

typedef enum NETDVR_SUBBITTYPE {
	NETDVR_SUBBITRATE_FIXED = 0,					//fixed bit rate
	NETDVR_SUBBITRATE_VARIABLE,					//variable bit rate
} subbittype_t;

typedef enum NETDVR_SUBVIDEOQUALITY {
	NETDVR_SUBVIDEOQUALITY_BEST = 0,				//best video quality
	NETDVR_SUBVIDEOQUALITY_BETTER,				//better video quality
	NETDVR_SUBVIDEOQUALITY_GOOD,					//good video quality
	NETDVR_SUBVIDEOQUALITY_NORMAL,				//normal video quality
	NETDVR_SUBVIDEOQUALITY_BAD,					//bad video quality
	NETDVR_SUBVIDEOQUALITY_WORSE,				//worse video quality
} subvideoquality_t;

typedef enum NETDVR_SUBFRAMERATE {
	NETDVR_SUBFRAMERATE_25 = 25,					//25f/s
	NETDVR_SUBFRAMERATE_20 = 20,					//20f/s
	NETDVR_SUBFRAMERATE_15 = 15,					//15f/s
	NETDVR_SUBFRAMERATE_10 = 10,					//10f/s
	NETDVR_SUBFRAMERATE_5 = 5,					//5f/s
	NETDVR_SUBFRAMERATE_2 = 2,					//2f/s
	NETDVR_SUBFRAMERATE_1 = 1,					//1f/s
} subframerate_t;

typedef enum NETDVR_SUBBITRATE {
	NETDVR_SUBBITRATE_64 = 64,					//64kbps
	NETDVR_SUBBITRATE_128 = 128,					//128kbps
	NETDVR_SUBBITRATE_256 = 256,					//256kbps
	NETDVR_SUBBITRATE_384 = 384,					//384kbps
	NETDVR_SUBBITRATE_512 = 512,					//512kbps
	NETDVR_SUBBITRATE_768 = 768,					//768kbps
	NETDVR_SUBBITRATE_1024 = 1024,				//1Mbps
	NETDVR_SUBBITRATE_1536 = 1536,				//1.5Mbps
	NETDVR_SUBBITRATE_2048 = 2048,				//2Mbps
} subbitrate_t;

struct NETDVR_SubStreamParam_t								
{
	unsigned char		chn;
	subflag_t			sub_flag;					//区分多种子码流列问 0 cif 1 qcif
	subbittype_t		sub_bit_type;				//子码流位率类型
	unsigned short		sub_intraRate;				//关键帧间隔
	unsigned char		sub_qi;						//关键帧量化因子
	unsigned char		sub_minQ;					//最小量化因子
	unsigned char		sub_maxQ;					//最大量化因子
	subvideoquality_t	sub_quality;				//子码流图像质量
	subframerate_t 		sub_framerate;				//子码流的帧率
	subbitrate_t 		sub_bitrate;				//子码流的位率
	unsigned int		copy2chnmask;				//复制到其他通道。每一位一个通道
	unsigned char		reserved[16];				//保留字段
};

enum NETDVR_OVERLAP {
	NETDVR_OVERLAP_NO = 0,					//not overlap when disk(s) is(are) full
	NETDVR_OVERLAP_YES = 1,					//overlap when disk(s) is(are) full
};

enum NETDVR_DVRSTATUS {
	NETDVR_DVRSTATUS_HIDDEN = 0,			//don't display the DVR status 
	NETDVR_DVRSTATUS_DISPLAY = 1,			//display the DVR status
};

/* ========================== keybord lock time============================= */
enum NETDVR_LOCKTIME {
	NETDVR_LOCKTIME_NEVER = 0,			//never lock
	NETDVR_LOCKTIME_MIN_1 = 60,			//lock time equals 1 minute
	NETDVR_LOCKTIME_MIN_2 = 120,			//lock time equals 2 minutes
	NETDVR_LOCKTIME_MIN_5 = 300,			//lock time equals 5 minutes
	NETDVR_LOCKTIME_MIN_10 = 600,			//lock time equals 10 minutes
	NETDVR_LOCKTIME_MIN_20 = 1200,			//lock time equals 20 minutes
	NETDVR_LOCKTIME_MIN_30 = 1800,			//lock time equals 30 minutes
};

enum NETDVR_SWITCHTIME {
	NETDVR_SWITCHTIME_NEVER = 0,		//never switch
	NETDVR_SWITCHTIME_SEC_5 = 5,		//switch time equals 5 seconds
	NETDVR_SWITCHTIME_SEC_10 = 10,		//switch time equals 10 seconds
	NETDVR_SWITCHTIME_SEC_20 = 20,		//switch time equals 20 seconds
	NETDVR_SWITCHTIME_SEC_30 = 30,		//switch time equals 30 seconds
	NETDVR_SWITCHTIME_MIN_1 = 60,		//switch time equals 1 minute
	NETDVR_SWITCHTIME_MIN_2 = 120,		//switch time equals 2 minute
	NETDVR_SWITCHTIME_MIN_5 = 300,		//switch time equals 5 minute
};

enum NETDVR_VIDEOFORMAT {
	NETDVR_VIDEOFORMAT_PAL = 0,			//PAL video format
	NETDVR_VIDEOFORMAT_NTSC = 1,		//NTSC video format
};
//typedef enum NETDVR_VIDEOFORMAT video_format_t;



/* ========================== VGASOLUTION param structure =========================== */

struct NETDVR_VGAPROPERTY
{
	unsigned short		width;						//分辨率宽
	unsigned short		height;						//分辨率高
	unsigned char		flashrate;					//分辨率刷新率
};

struct NETDVR_VGARESOLLIST
{
	struct NETDVR_VGAPROPERTY vgapro[16];
};

enum NETDVR_TRANSPARENCY {
	NETDVR_TRANSPARENCY_NO = 0,				//no transparency
	NETDVR_TRANSPARENCY_LOW = 1,			//low transparency
	NETDVR_TRANSPARENCY_MID = 2,			//middle transparency
	NETDVR_TRANSPARENCY_HIGH = 3,			//high transparency
};

/* ========================== sys param structure =========================== */
struct NETDVR_systemParam_t
{
	unsigned short	 device_id;					//the dvr device id for remote control
	char			device_name[32];			//device name of dvr
	unsigned char	disable_pw;
	enum NETDVR_OVERLAP flag_overlap;			//flag_overlap=1, then overlap the disk when the disk is full. or if flag_overlap=0, then not overlap
	enum NETDVR_DVRSTATUS flag_statusdisp;		//decide whether to dispay system status or not
	enum NETDVR_LOCKTIME lock_time;				//lock time for keybord
	enum NETDVR_SWITCHTIME switch_time;			//switch time for preview mode
	enum NETDVR_VIDEOFORMAT video_format;		//PAL or NTSC
	unsigned char vga_solution;
	enum NETDVR_TRANSPARENCY transparency;		//menu transparency
	int languageindex;				//language index of languagelist
	unsigned char flag_bestrow;				//24小时覆盖
	char reserved[31];
}; 

enum NETDVR_BAUDRATE {
	NETDVR_BAUDRATE_115200 = 0,	
	NETDVR_BAUDRATE_57600 = 1,
	NETDVR_BAUDRATE_38400 = 2,
	NETDVR_BAUDRATE_19200 = 3,
	NETDVR_BAUDRATE_9600 = 4,
	NETDVR_BAUDRATE_4800 = 5,
	NETDVR_BAUDRATE_2400 = 6,
	NETDVR_BAUDRATE_1200 = 7,
	NETDVR_BAUDRATE_300 = 8,
};

enum NETDVR_DATABITSEL {
	NETDVR_DATABITSEL_8 = 0,				//data bit: 8 bits
	NETDVR_DATABITSEL_7 = 1,				//data bit: 7 bits
	NETDVR_DATABITSEL_6 = 2,				//data bit: 6 bits
};

enum NETDVR_STOPBITSEL {
	NETDVR_STOPBITSEL_1 = 0,				//stop bit: 1 bit
	NETDVR_STOPBITSEL_2 = 1,				//stop bit: 2 bits
};

enum NETDVR_CHECK_TYPE {
	NETDVR_CHECK_NONE = 0,					//no check
	NETDVR_CHECK_ODD = 1,					//odd check
	NETDVR_CHECK_EVEN = 2,					//even check
};

enum NETDVR_FLOWCONTROL {
	NETDVR_FLOWCONTROL_NO = 0,				//no flow control
	NETDVR_FLOWCONTROL_HARDWARE = 1,		//flow control by hardware
	NETDVR_FLOWCONTROL_XON_XOFF = 2,		//flow control by Xon/Xoff
};

enum NETDVR_PROTOCOLTYPE {
	NETDVR_PROTOCOLTYPE_PELCO_D = 0,		//protocol type : Pelco-d
	NETDVR_PROTOCOLTYPE_PELCO_P = 1,		//protocol type : Pelco-p
	NETDVR_PROTOCOLTYPE_B01 = 2,			//protocol type : B01
	NETDVR_PROTOCOLTYPE_SAMSUNG = 3,		//protocol type : Samsung
	NETDVR_PROTOCOLTYPE_HY = 4,
	NETDVR_PROTOCOLTYPE_Panasonic = 5,
	NETDVR_PROTOCOLTYPE_Pelco_9750 = 6,
	NETDVR_PROTOCOLTYPE_PelcoD1 = 7,
	NETDVR_PROTOCOLTYPE_PelcoD_S1 = 8,
	NETDVR_PROTOCOLTYPE_PelcoP1 = 9,
	NETDVR_PROTOCOLTYPE_PelcoP5 = 10,
	NETDVR_PROTOCOLTYPE_Philips = 11,
	NETDVR_PROTOCOLTYPE_Sanli = 12,
	NETDVR_PROTOCOLTYPE_Santachi =13,
	NETDVR_PROTOCOLTYPE_Sharp = 14,
	NETDVR_PROTOCOLTYPE_Sony = 15,
	NETDVR_PROTOCOLTYPE_Yaan = 16,

};

#define MAX_PRESET_NUM 128					//preset limit 0-MAX_PRESET_NUM
#define MAX_CRUISE_PATH_NUM 16				//max cruise paths of each PTZ
#define MAX_CRUISE_POS_NUM 16				//max cruise positions of each cruise path
#define MAX_CRUISE_SPEED 9					//max cruise speed
#define MAX_DWELL_TIME 255					//max dwell time

enum NETDVR_YTTRACKCONTROL {
	NETDVR_YT_COM_STARTRECORDTRACK = 0,
	NETDVR_YT_COM_STOPRECORDTRACK = 1,
	NETDVR_YT_COM_STARTTRACK = 2,
	NETDVR_YT_COM_STOPTRACK = 3,	
};

enum NETDVR_YTPRESETPOINTCONTROL
{
	NETDVR_YT_COM_ADDPRESETPOINT = 0,
	NETDVR_YT_COM_DELPRESETPOINT = 1,
	NETDVR_YT_COM_TOPRESETPOINT = 2,
};

struct NETDVR_cruisePos_t
{
	unsigned char preset_no;				//1 ~ MAX_PRESET_NUM
	unsigned char dwell_time;				//1 ~ 255 secondes
	unsigned char cruise_speed;				//1 ~ MAX_CRUISE_SPEED(1 : the slowest speed)
	unsigned char cruise_flag;				//1-add 2-del
};

struct NETDVR_cruisePath_t
{
	unsigned char chn;
	unsigned char path_no;										//1 ~ MAX_CRUISE_PATH_NUM	
	struct NETDVR_cruisePos_t cruise_pos[MAX_CRUISE_POS_NUM];	//cruise poisitions of cruise path
};

struct NETDVR_ptzParam_t 
{
#if 1//csp modify
	unsigned char		chn;						//通道
	unsigned short address;							//解码器地址
	enum NETDVR_BAUDRATE baud_ratesel;				//数据通讯速率
	enum NETDVR_DATABITSEL data_bitsel;				//数据位
	enum NETDVR_STOPBITSEL stop_bitsel;				//停止位
	enum NETDVR_CHECK_TYPE check_type;				//校验类型
	enum NETDVR_FLOWCONTROL flow_control;			//流控
	enum NETDVR_PROTOCOLTYPE protocol;				//协议
	unsigned int	copy2Chnmask;						//复制到其他通道。每一位一个通道
	unsigned char	comstyle;					//串口类型(232 or 485)
	unsigned char	enableptz;
	char reserved[30];								//保留字段
#else
	unsigned char		chn;						//通道
	unsigned short address;							//解码器地址
	enum NETDVR_BAUDRATE baud_ratesel;				//数据通讯速率
	enum NETDVR_DATABITSEL data_bitsel;				//数据位
	enum NETDVR_STOPBITSEL stop_bitsel;				//停止位
	enum NETDVR_CHECK_TYPE check_type;				//校验类型
	enum NETDVR_FLOWCONTROL flow_control;			//流控
	enum NETDVR_PROTOCOLTYPE protocol;				//协议
	unsigned int copy2Chnmask;						//复制到其他通道。每一位一个通道
	unsigned char      comstyle;					//串口类型(232 or 485)
	char reserved[31];								//保留字段
#endif
};

enum NETDVR_PTZCONTROL {
	NETDVR_PTZ_COM_STOP = 0,
	NETDVR_PTZ_COM_MOVEUP = 1,
	NETDVR_PTZ_COM_MOVEDOWN = 2,
	NETDVR_PTZ_COM_MOVELEFT = 3,
	NETDVR_PTZ_COM_MOVERIGHT = 4,
	NETDVR_PTZ_COM_ROTATION = 5,
	NETDVR_PTZ_COM_ZOOMADD = 6,
	NETDVR_PTZ_COM_ZOOMSUBTRACT = 7,
	NETDVR_PTZ_COM_FOCUSADD = 8,
	NETDVR_PTZ_COM_FOCUSSUBTRACT = 9,
	NETDVR_PTZ_COM_APERTUREADD = 10,
	NETDVR_PTZ_COM_APERTURESUBTRACT = 11,
	NETDVR_PTZ_COM_LIGHTINGOPEN = 12,
	NETDVR_PTZ_COM_LIGHTINGCLOSE = 13,
	NETDVR_PTZ_COM_WIPERSOPEN = 14,
	NETDVR_PTZ_COM_WIPERSCLOSE = 15,
	NETDVR_PTZ_COM_FAST = 16,
	NETDVR_PTZ_COM_NORMAL = 17,
	NETDVR_PTZ_COM_SLOW = 18,
	NETDVR_PTZ_COM_AUXILIARYOPEN = 19,
	NETDVR_PTZ_COM_AUXILIARYCLOSE = 20,
};

struct NETDVR_PtzCtrl_t
{
	unsigned char  chn;							//通道
	enum NETDVR_PTZCONTROL  cmd;							//命令
	unsigned char	aux;						//辅助功能
};

typedef enum NETDVR_LOGSEARCH_MODE {
	NETDVR_LOGSEARCH_MODE_BOTH = 0,			//search by both type and time
	NETDVR_LOGSEARCH_MODE_TYPE = 1,			//search by type
	NETDVR_LOGSEARCH_MODE_TIME = 2,			//search by time
} logsearch_mode_t;

typedef enum NETDVR_LOGSEARCH_MAIN {
	NETDVR_LOGSEARCH_MAIN_ALARM = 0,		//search alarm logs
	NETDVR_LOGSEARCH_MAIN_LOCALOP = 1,		//search local operation logs
	NETDVR_LOGSEARCH_MAIN_REMOTEOP = 2,		//search remote operation logs
	NETDVR_LOGSEARCH_MAIN_EXCEPTION = 3,	//search exception logs
	NETDVR_LOGSEARCH_MAIN_ALL = 4,			//search all logs
} logsearch_main_t;

typedef enum NETDVR_LOGSEARCH_SLAVE {
	/*NETDVR_LOGSEARCH_MAIN_ALARM's slave type*/
	NETDVR_LOGSEARCH_ALARM_IN_BEGIN = 0,	//search alarm input begining logs
	NETDVR_LOGSEARCH_ALARM_IN_END,			//search alarm input ending logs
	NETDVR_LOGSEARCH_ALARM_MD_BEGIN,		//search md alarm begining logs
	NETDVR_LOGSEARCH_ALARM_MD_END,			//search md alarm ending logs
	NETDVR_LOGSEARCH_ALARM_ALL,				//search all alarm logs
	/*NETDVR_LOGSEARCH_MAIN_LOCALOP's slave type*/
	NETDVR_LOGSEARCH_LOCALOP_STARTUP = 16,	//search local operatoin's startup logs
	NETDVR_LOGSEARCH_LOCALOP_SHUTDOWN,		//search local operatoin's shutdown logs
	NETDVR_LOGSEARCH_LOCALOP_LOGIN,			//search local operatoin's login logs
	NETDVR_LOGSEARCH_LOCALOP_LOGOUT,		//search local operatoin's logout logs
	NETDVR_LOGSEARCH_LOCALOP_CONFIG,		//search local operatoin's config logs
	NETDVR_LOGSEARCH_LOCALOP_REC_START,		//search local operatoin's record start logs
	NETDVR_LOGSEARCH_LOCALOP_REC_STOP,		//search local operatoin's record stop logs
	NETDVR_LOGSEARCH_LOCALOP_UPDATE,		//search local operatoin's update logs
	NETDVR_LOGSEARCH_LOCALOP_FORMAT,		//search local operatoin's format logs
	NETDVR_LOGSEARCH_LOCALOP_ALL,			//search all local operatoin logs
	/*NETDVR_LOGSEARCH_MAIN_REMOTEOP's slave type*/
	NETDVR_LOGSEARCH_REMOTEOP_LOGIN = 32,	//search remote operatoin's login logs
	NETDVR_LOGSEARCH_REMOTEOP_LOGOUT,		//search remote operatoin's logout logs
	NETDVR_LOGSEARCH_REMOTEOP_REC_START,	//search remote operatoin's record start logs
	NETDVR_LOGSEARCH_REMOTEOP_REC_STOP,		//search remote operatoin's record stop logs
	NETDVR_LOGSEARCH_REMOTEOP_CONFIG,		//search remote operatoin's config logs
	NETDVR_LOGSEARCH_REMOTEOP_RESTART,		//search remote operatoin's restart logs
	NETDVR_LOGSEARCH_REMOTEOP_VOIP_START,	//search remote operatoin's voip start logs
	NETDVR_LOGSEARCH_REMOTEOP_VOIP_STOP,	//search remote operatoin's voip stop logs
	NETDVR_LOGSEARCH_REMOTEOP_UPDATE,		//search remote operatoin's update logs
	NETDVR_LOGSEARCH_REMOTEOP_ALL,			//search all remote operatoin logs
	/*NETDVR_LOGSEARCH_MAIN_EXCEPTION's slave type*/
	NETDVR_LOGSEARCH_EXCEPTION_VLOSS = 48,	//search exception's video loss logs
	NETDVR_LOGSEARCH_EXCEPTION_VBLIND,		//search exception's video blind logs
	NETDVR_LOGSEARCH_EXCEPTION_HDD_ERR,		//search exception's hdd error logs
	NETDVR_LOGSEARCH_EXCEPTION_HDD_FULL,	//search exception's hdd full logs
	NETDVR_LOGSEARCH_EXCEPTION_ILLEGALOP,	//search exception's illegal operation logs
	NETDVR_LOGSEARCH_EXCEPTION_ALL,			//search all exception logs
} logsearch_slave_t;



/* ========================== structure for log index condition============================= */
struct NETDVR_logSearchCondition_t 
{
	logsearch_mode_t query_mode;			//search by 3 modes: see logsearch_mode_t
	logsearch_main_t main_type;				//see logsearch_main_t
	logsearch_slave_t slave_type;			//see logsearch_slave_t
	unsigned short max_return;				//the max return logs number(how many logs you want to get) [<=12]
	unsigned short startID;					//ignore the front (startID - 1) logs [>=1]
	unsigned int start_time;				//search from start time
	unsigned int end_time;					//search to end time

};

/* ========================== structure for each indexed log information============================= */
struct NETDVR_logInfo_t
{
	unsigned int startTime;					//log created time
	unsigned char main;
	unsigned char slave;
	char loginfo[32];						//log info
	struct NETDVR_logInfo_t *pnext;
};

/* ========================== structure for index result's information of logs============================= */
struct NETDVR_logSearchResult_t
{
	unsigned short sum;						//total records of found logs
	unsigned short startID;					//if no file is indexed, startID will be 0, or it'll be a value based on index condition's startID(struct NETDVR_logSearchCondition_t)
	unsigned short endID;					//结束的记录,基址为1.当endID和startID相等时,表示之返回一条记录
	
	struct NETDVR_logInfo_t *pH;
};

//xdc
/* ========================== condition of seach IPC============================= */ 
struct NETDVR_ipcSeachCondition
{
	unsigned int protocol_type;					//0 all,1 onvif, 2 YueTian
	unsigned short max_return;					//最大返回数	
};

//xdc
/* ========================== structure of seach IPC============================= */ 
struct NETDVR_ipcInfo
{
	unsigned char channel_no;
	unsigned char enable;
	unsigned char stream_type;
	unsigned char trans_type;
	unsigned int protocol_type;
	unsigned int dwIp;
	unsigned short wPort;
	unsigned char force_fps;
	unsigned char frame_rate;
	char user[32];
	char pwd[32];
	char name[32];
	char uuid[64];
	char address[64];//onvif使用
	char ipc_type;
	char reserved2[2];
	//NVR的最大通道数
	char max_nvr_chn;
	//要申请NVR 的哪一路码流
	char req_nvr_chn;
	char reserved[43];
	unsigned int net_mask;
	unsigned int net_gateway;
	unsigned int dns1;
	unsigned int dns2;
	struct NETDVR_ipcInfo *pnext;
};
//xdc
/* ========================== structure for index result's information of ipc============================= */
struct NETDVR_ipcSearch
{
	unsigned short sum;						//total records of found logs
	unsigned short startID;					//if no file is indexed, startID will be 0, or it'll be a value based on index condition's startID(struct NETDVR_logSearchCondition_t)
	unsigned short endID;					//结束的记录,基址为1.当endID和startID相等时,表示之返回一条记录

	struct NETDVR_ipcInfo *pIPC;
};
//yaogang modify 20141030
//PATROL_PARA
typedef struct NETDVR_PatrolPara
{
	unsigned char nIsPatrol; //当前轮巡是否启用(数字:0=否;1=是)
	unsigned char nInterval; //当前轮巡切换时间(数字:单位秒s)
	unsigned char nPatrolMode; //当前轮巡时所用的预览模式(数字:参考Mode 1 4 9 16)
	unsigned char nPatrolChnNum;//轮巡通道数16 32 48 64
	unsigned char nInterval_num;//轮巡切换时间可供选择的数量
	unsigned char nPatrolMode_num;//轮巡预览模式可供选择的数量
	unsigned char value[1];//存放轮巡时间和模式的值。eg: 5 10 20 30 60 1 4 9 16
} NETDVR_PatrolPara;

typedef struct NETDVR_PreviewPara
{
	unsigned char nPreviewMode; //当前所用的预览模式(数字:参考Mode 1 4 9 16 32)	
	unsigned char ModePara;//预览起始通道0-15
} NETDVR_PreviewPara;

//通道属性
typedef struct
{
    int x;  //点坐标x
    int y; //点坐标y
} SPoint;

typedef struct { 
	char strChnName[32];  // 各通道通道名
	unsigned char nShowChnName;  // 各通道是否显示通道名(数字列表:0=否;1=是)
	SPoint sChnNamePos; // 各通道名XY坐标
	unsigned char nEncShowChnName; // 编码各通道是否显示通道名(数字列表:0=否;1=是)
	SPoint sEncChnNamePos; // 编码各通道名XY坐标(以D1为基准)
	unsigned char nEncShowTime; // 编码各通道否显示时间(数字列表:0=否;1=是)
	SPoint sEncTimePos; // 编码各通道时间XY坐标(以D1为基准)
} SBizCfgStrOsd;

typedef struct NETDVR_ChnPara
{
	unsigned char nchn;
	SBizCfgStrOsd para;
} NETDVR_ChnPara;

typedef struct NETDVR_IPCChnStatus
{
	unsigned char max_chn_num;//最大通道数
	unsigned char chn_status[20];//一个bit对应一个通道，共160通道
} NETDVR_IPCChnStatus;


/* ========================== structure for record file's remote index condition============================= */
struct NETDVR_fileSearchCondition_t
{
	unsigned short chn;								//searched channel number 通道1-16 按位组合
	unsigned short type;						//record type
	unsigned int start_time;				//search from start time
	unsigned int end_time;					//search to end time
	unsigned short startID;					//ignore the front (startID - 1) records
	unsigned short max_return;				//the max return records number(how many records you want to get)
	unsigned short chn17to32;				//通道17-32 按位组合
	unsigned char reserved[5];
	char bankcardno[21];				//卡号
};

/* ========================== structure for each indexed record file information============================= */
struct NETDVR_recFileInfo_t
{
	unsigned char channel_no;				//the channel number of the record file
	unsigned char type;						//the record type of file
	unsigned int start_time;				//start time of the record file
	unsigned int end_time;					//end time of the record file
	unsigned char image_format;				//frame type:3(Pal-cif) ; 4(Pal-D1); 8(NTSC-cif); 9(NTSC-D1)
	unsigned char stream_flag;				//stream flag:0 for video flow ; 1 for audio flow
	unsigned int size;						//size of record file
	unsigned int offset;					//the file offset in dvr file pools
	char filename[64];						//name of record file
	struct NETDVR_recFileInfo_t *pnext;	//poiter reference to the next record file information
};

/* ========================== structure for index result's information============================= */
struct NETDVR_recFileSearchResult_t
{
	unsigned short sum;						//totals of remote indexed files.
	unsigned short startID;					//if no file is indexed, startID will be 0, or it'll be a value based on index condition's startID(struct NETDVR_fileSearchCondition_t)
	unsigned short endID;					//if startID isn't 0, then (endID - startID + 1) files is indexed.
	struct NETDVR_recFileInfo_t *precInfo;//if the first file of all indexed files.
};

struct NETDVR_TimePlayCond_t
{
	unsigned char chn;								//searched channel number
	unsigned short type;						//record type
	unsigned int start_time;				//search from start time
	unsigned int end_time;	
};
	

#define NETDVR_VIDEO_LOST_DECT 0			//dect video lost
#define NETDVR_VIDEO_LOST_UNDECT 1			//not dect video lost

#define NETDVR_VIDEO_MD_CLOSE 0				//not dect video motion
#define NETDVR_VIDEO_MD_SEN_LOWEST 1		//dect video motion in a lowest sensitivity
#define NETDVR_VIDEO_MD_SEN_LOWER 2			//dect video motion in a lower sensitivity
#define NETDVR_VIDEO_MD_SEN_LOW 3			//dect video motion in a low sensitivity
#define NETDVR_VIDEO_MD_SEN_HIGH 4			//dect video motion in a high sensitivity
#define NETDVR_VIDEO_MD_SEN_HIGHER 5		//dect video motion in a higher sensitivity
#define NETDVR_VIDEO_MD_SEN_HIGHEST 6		//dect video motion in a highest sensitivity



struct NETDVR_videoParam_t
{
	unsigned char		channel_no;					//通道号
	char	channelname[32];			//通道名
	unsigned char		flag_name;					//名称位置显示
	unsigned short		chnpos_x;					//名称x坐标
	unsigned short		chnpos_y;					//名称y坐标
	unsigned char		flag_time;					//时间位置显示
	unsigned short		timepos_x;					//时间x坐标
	unsigned short		timepos_y;					//时间y坐标
	unsigned char		flag_mask;					//遮盖

	struct net_maskAREA_t
	{
		unsigned short	 	x;
		unsigned short		y;
		unsigned short		width;
		unsigned short		height;
	}maskinfo[4];
	//handler
	unsigned char		flag_safechn;				//安全通道标记
	unsigned int		copy2chnmask;				//复制到其他通道。每一位一个通道
	unsigned char		reserved[16];				//保留字段
};

typedef enum NETDVR_FLOW_TYPE {
	NETDVR_FLOW_VIDEO = 0,						//video flow
	NETDVR_FLOW_MUTI,							//multiple flow(both video and audio)
} flow_type_t;

typedef enum NETDVR_BITRATE_TYPE {
	NETDVR_BITRATE_FIXED = 0,					//fixed bit rate
	NETDVR_BITRATE_VARIABLE,					//variable bit rate
} bitrate_type_t;

typedef enum NETDVR_BITRATE {
	NETDVR_BITRATE_64 = 64,					//64kbps
	NETDVR_BITRATE_128 = 128,					//128kbps
	NETDVR_BITRATE_256 = 256,					//256kbps
	NETDVR_BITRATE_384 = 384,					//384kbps
	NETDVR_BITRATE_512 = 512,					//512kbps
	NETDVR_BITRATE_768 = 768,					//768kbps
	NETDVR_BITRATE_1024 = 1024,				//1Mbps
	NETDVR_BITRATE_1536 = 1536,				//1.5Mbps
	NETDVR_BITRATE_2048 = 2048,				//2Mbps
} bitrate_t;

typedef enum NETDVR_VIDEO_QUALITY {
	NETDVR_VIDEO_QUALITY_BEST = 0,				//best video quality
	NETDVR_VIDEO_QUALITY_BETTER,				//better video quality
	NETDVR_VIDEO_QUALITY_GOOD,					//good video quality
	NETDVR_VIDEO_QUALITY_NORMAL,				//normal video quality
	NETDVR_VIDEO_QUALITY_BAD,					//bad video quality
	NETDVR_VIDEO_QUALITY_WORSE,				//worse video quality
} video_quality_t;

typedef enum NETDVR_FRAMERATE {
	NETDVR_FRAMERATE_25 = 25,					//25f/s
	NETDVR_FRAMERATE_20 = 20,					//20f/s
	NETDVR_FRAMERATE_15 = 15,					//15f/s
	NETDVR_FRAMERATE_10 = 10,					//10f/s
	NETDVR_FRAMERATE_5 = 5,					//5f/s
	NETDVR_FRAMERATE_2 = 2,					//2f/s
	NETDVR_FRAMERATE_1 = 1,					//1f/s
} framerate_t;

typedef enum NETDVR_PRERECORD_TIME {
	NETDVR_PRERECORD_TIME_0 = 0,				//do not pre-record
	NETDVR_PRERECORD_TIME_5 = 5,					//pre-record time: 5s
	NETDVR_PRERECORD_TIME_10 = 10,					//pre-record time: 10s
	NETDVR_PRERECORD_TIME_15 = 15,					//pre-record time: 15s
	NETDVR_PRERECORD_TIME_20 = 20,					//pre-record time: 20s
	NETDVR_PRERECORD_TIME_25 = 25,					//pre-record time: 25s
	NETDVR_PRERECORD_TIME_30 = 30,					//pre-record time: 30s
} prerecord_time_t;

typedef enum NETDVR_POSTRECORD_TIME {
	NETDVR_POSTRECORD_TIME_5 = 5,					//post-record time: 5s
	NETDVR_POSTRECORD_TIME_10 = 10,					//post-record time: 10s
	NETDVR_POSTRECORD_TIME_30 = 30,					//post-record time: 30s
	NETDVR_POSTRECORD_TIME_60 = 60,					//post-record time: 60s
	NETDVR_POSTRECORD_TIME_120 = 120,				//post-record time: 120s
	NETDVR_POSTRECORD_TIME_300 = 300,				//post-record time: 300s
	NETDVR_POSTRECORD_TIME_600 = 600,				//post-record time: 600s
} postrecord_time_t;


// struct NETDVR_recordParam_t
// {	
// 	unsigned char		channelno;					//通道号
// 	flow_type_t			code_type;					//see flow_type_t
// 	bitrate_type_t		bit_type;				//see bitrate_type_t
// 	bitrate_t			bit_max;						//see bitrate_t
// 	unsigned short		intraRate;					//关键帧间隔
// 	unsigned char		qi;							//关键帧量化因子
// 	unsigned char		minQ;						//最小量化因子
// 	unsigned char		maxQ;						//最大量化因子
// 	video_quality_t		quality;				//see video_quality_t
// 	framerate_t			frame_rate;					//see framerate_t
// 	prerecord_time_t	pre_record;			//see prerecord_time_t
// 	postrecord_time_t	post_record;			//see postrecord_time_t
// 
// 	unsigned int		copy2chnmask;
//  	unsigned char		reserved[16];				//保留字段
// 
// };

struct NETDVR_recordParam_t
{	
	unsigned char		channelno;					//通道号
	flow_type_t			code_type;					//see flow_type_t
	bitrate_type_t		bit_type;				//see bitrate_type_t
	bitrate_t			bit_max;						//see bitrate_t
	unsigned short		intraRate;					//关键帧间隔
	unsigned char		qi;							//关键帧量化因子
	unsigned char		minQ;						//最小量化因子
	unsigned char		maxQ;						//最大量化因子
	video_quality_t		quality;				//see video_quality_t
	framerate_t			frame_rate;					//see framerate_t
	prerecord_time_t	pre_record;			//see prerecord_time_t
	postrecord_time_t	post_record;			//see postrecord_time_t
	unsigned int		copy2chnmask;
	unsigned char		supportdeinter;				//是否支持deinter设置 1是 0否 (待用)
	unsigned char		deinterval;					//deinter强度 0-4 禁用，弱，中，强，超强
	unsigned char		supportResolu;				//是否支持设置录像分辨率
	unsigned char		resolutionpos;				//分辨率选项值
	unsigned char		reserved1[12];				//保留字段	
};

typedef enum NETDVR_WEEKDAY {
	NETDVR_WEEKDAY_1 = 0,						//Monday
	NETDVR_WEEKDAY_2,							//Tuesday
	NETDVR_WEEKDAY_3,							//Wednesday
	NETDVR_WEEKDAY_4,							//Thursday
	NETDVR_WEEKDAY_5,							//Friday
	NETDVR_WEEKDAY_6,							//Saturday
	NETDVR_WEEKDAY_7,							//Sunday
} weekday_t;


/* ======================================================= */
struct RecTimeField_t
{
	unsigned int starttime;					//起始时间
	unsigned int endtime;					//终止时间
	unsigned char	flag_sch;					//定时录像
	unsigned char 	flag_md;					//移动侦测录像
	unsigned char  flag_alarm;					//报警录像
	unsigned char	reserved[4];				//保留字段
};

//录像布防
struct NETDVR_RecordSCH_t
{
	unsigned char		chn;						//通道
	enum NETDVR_WEEKDAY		weekday;					//星期
	struct RecTimeField_t recTimeFieldt[4];
	unsigned char		copy2Weekday;				//复制到其他天  按位 
	unsigned int		copy2Chnmask;				//复制到其他通道。按位
	unsigned char		reserved[16];				//保留字段
};

//手动录像
struct NETDVR_ManualRecord_t
{
	unsigned int  chnRecState;					//通道手动录像状态 按位	
};



typedef enum NETDVR_ALARMINTYPE {
	NETDVR_ALARMIN_HIGH = 1,				//high level alarm input
	NETDVR_ALARMIN_LOW,						//low level alarm input
} alarmintype_t;

typedef enum NETDVR_ALARMOUTYPE {
	NETDVR_ALARMOUT_NO = 1,					//alarm output type:normal open
	NETDVR_ALARMOUT_NC,						//alarm output type:normal close
} alarmouttype_t;

typedef enum NETDVR_DELAY_TIME {
	NETDVR_DELAY_5 = 5,					//5s
	NETDVR_DELAY_10=10,					//10s
	NETDVR_DELAY_30=30,					//30s
	NETDVR_DELAY_60=60,					//60s
	NETDVR_DELAY_120=120,				//120s
	NETDVR_DELAY_300=300,				//300s
	NETDVR_DELAY_MANUAL=0xffff, 		//manual
} delay_t;

struct AlarmInPtz							//关联ptz
{
	unsigned char		flag_preset;			//预置点
	unsigned char		preset;
	unsigned char		flag_cruise;			//巡航点
	unsigned char		cruise;
	unsigned char		flag_track;				//轨迹
};

struct NETDVR_alarmInParam_t
{
	unsigned char		inid;						//报警输入量
	unsigned char		flag_deal;					//1: deal with input alarm ; 0: for no
	alarmintype_t		typein;						//input alarm type:see alarmintype_t
	unsigned int		triRecChn;					//触发录像通道，每一位一通道
	unsigned int		triAlarmoutid;				//触发报警输出，按位
	unsigned char		flag_buzz;					//触发蜂鸣器
	unsigned char		flag_email;					//触发emaill
	unsigned char		flag_mobile;				//触发手机报警
	delay_t		delay;								//报警输出延时
	unsigned int		copy2AlarmInmask;
	struct AlarmInPtz	alarmptz[32];
	unsigned char		flag_enablealarm;			//报警启用标志
	unsigned char		reserved[15];				//保留字段
};

struct NETDVR_alarmOutParam_t
{
	unsigned char		outid;						//报警输出量 
	alarmouttype_t		typeout;					//报警输出类型
	unsigned int		copy2AlarmOutmask;			//复制到其他报警输出。按位
	//csp modify
	//unsigned char		reserved[16];				//保留字段
	delay_t				alarmoutdelay;				//报警输出延时
	unsigned char		flag_buzz;					//触发蜂鸣器
	delay_t				buzzdelay;					//蜂鸣器延时
	unsigned char		reserved[7];				//保留字段
};

struct NETDVR_AlarmNoticeParam_t									
{
	char			alarm_email[32];			//报警email地址
	char			alarm_mobile[32];			//报警手机地址
	unsigned char	reserved[32];				//保留字段
};

struct NETDVR_networkParam_t
{
	char				mac_address[18];			//mac地址
	unsigned int		ip_address;					//ip地址
	unsigned short		server_port;				//设备端口
	unsigned int		net_mask;					//掩码
	unsigned int		net_gateway;				//网关
	unsigned int		dns;						//dns
	unsigned char		flag_multicast;				//组播启用标记
	unsigned int		multicast_address;			//组播地址
	unsigned short		http_port;					//http端口
	unsigned char		flag_pppoe;					//pppoe启用标记
	char				pppoe_user_name[64];		//pppoe用户名
	char				pppoe_passwd[64];			//pppoe密码
	unsigned char		flag_dhcp;					//dhcp启用标志
	unsigned char		ddnsserver;					//ddns服务商
	unsigned char		flag_ddns;					//ddns启用标志
	char				ddns_domain[64];			//ddns域名
	char				ddns_user[64];				//ddns用户名
	char				ddns_passwd[64];			//ddns密码
	unsigned int		centerhost_address;			//中心服务器地址
	unsigned short		centerhost_port;			//中心服务器端口
	unsigned short		mobile_port;				//手机监控端口
	char				hwid[12];					//俊明视定制
	unsigned char		reserved[2];				//保留字段
};


struct NETDVR_progressParam_t
{
	unsigned int curr_pos;
	unsigned int total_size;
};

struct NETDVR_TimeDLprogressParam_t
{
	unsigned int currfile_recv; //当前文件已接收大小
	unsigned int currfile_size; //当前文件大小
	int currindex; //当前第几个文件 1开始计数
	int totalnum; //总共有多少文件
	unsigned int totalfilesize;	//总文件大小 MB为单位，按1024除
	unsigned int totalrecvsize;
};

struct NETDVR_TimeDLfilename
{
	unsigned char channel_no;				//the channel number of the record file
	unsigned char type;						//the record type of file
	unsigned int start_time;				//start time of the record file
	unsigned int end_time;					//end time of the record file
	unsigned char image_format;				//frame type:3(Pal-cif) ; 4(Pal-D1); 8(NTSC-cif); 9(NTSC-D1)
	unsigned char stream_flag;				//stream flag:0 for video flow ; 1 for audio flow
	unsigned int size;						//size of record file
	unsigned int offset;					//the file offset in dvr file pools
	char recvefilename[64]; //对方发过来的文件名
	unsigned int currindex; //当前第几个文件 1开始计数
	unsigned int totalnum; //总共有多少文件
};

struct NETDVR_AlarmUploadState_t
{
	//0-信号量报警,1-硬盘满,2-信号丢失,3－移动侦测,4－硬盘未格式化,
	//5-读写硬盘出错,6-遮挡报警,7-制式不匹配, 8-非法访问
	unsigned char	type;		
	unsigned char	state;			//1报警 2恢复
	unsigned char	id;				//通道,硬盘号,报警输入号,取决于type 
	unsigned short	reserved1;		//预留
	unsigned int	reserved2;		//预留	
};

typedef void (CALLBACK*  PFUN_MSG_T)(unsigned int dwContent);
typedef void (CALLBACK*  PFUN_MSGHASAUDIO_T)(unsigned char b_has_audio, unsigned int dwContent);
typedef void (CALLBACK*  PFUN_ERROR_T)(unsigned short err_code, unsigned int dwContent);
typedef void (CALLBACK*  PFUN_PROGRESS_T)(struct NETDVR_progressParam_t progress, unsigned int dwContent);
typedef void (CALLBACK*  PFUN_ALARMSTATE_T)(struct NETDVR_AlarmUploadState_t alarmstate, unsigned int dwContent);
typedef void (CALLBACK*  PFUN_SAVE_T)( NETDVR_TimeDLfilename fileinfo, char* s_save_filename, unsigned int dwContent);
typedef void (CALLBACK*  PFUN_TIMEDLPROGRESS_T)( struct NETDVR_TimeDLprogressParam_t progress, unsigned int dwContent);

#define NETDVR_UPDATE_MOTHERBBOARD 0
#define NETDVR_UPDATE_PANEL 1

struct NETDVR_updateParam_t 
{
	char filename[64];						//not neccessary
	unsigned int size;						//file size
	unsigned int verify;					//verify 
	unsigned short version;					//file versions
	unsigned short reserved;				//update option: NETDVR_UPDATE_MOTHERBBOARD/NETDVR_UPDATE_PANEL
};

/* ========================== Motion Detect structures =========================== */

#define NETDVR_MD_MIN_SENSE	0						
#define NETDVR_MD_MAX_SENSE	5
						
struct NETDVR_motionDetection_t 
{
	unsigned char	chn;
	unsigned int 	trigRecChn;						//触发录像通道 按位
	unsigned int 	trigAlarmOut;					//触发报警输出 按位
	unsigned char	flag_buzz;						//
	unsigned char	flag_email;						//触发emaill
	unsigned char	flag_mobile;					//触发手机报警
	unsigned char	sense;
	delay_t			delay;							//延时
	unsigned char	block[44*36];					//
	unsigned int 	copy2Chnmask;					//复制到其他通道。按位
	unsigned char	reserved[12];					//预留
};

//视频丢失
struct NETDVR_VideoLostParam_t
{
	unsigned char		chn;
	unsigned int 		trigRecChn;					//触发录像通道 按位
	unsigned int 		trigAlarmOut;				//触发报警输出 按位
	unsigned char 		flag_buzz;					//蜂鸣器
	unsigned char		flag_email;					//触发emaill
	unsigned char		flag_mobile;				//触发手机报警
	delay_t 			delay;						//延时
	unsigned int 		copy2Chnmask;				//复制到其他通道。按位
	unsigned char		reserved[12];				//保留字段
};

//for 2.0
struct NETDVR_motionDetectionfor2_0_t 
{
	unsigned char	chn;
	unsigned int 	trigRecChn;						//触发录像通道 按位
	unsigned int 	trigAlarmOut;					//触发报警输出 按位
	unsigned char	flag_buzz;						//
	unsigned char	flag_email;						//触发emaill
	unsigned char	flag_mobile;					//触发手机报警
	unsigned char	sense;
	delay_t			delay;							//延时
	unsigned char	block[44*36];					//
	struct AlarmInPtz	alarmptz[32];				//ptz
	unsigned int 	copy2Chnmask;					//复制到其他通道。按位
	unsigned char	flag_enablealarm;				//报警启用标志
	unsigned char	reserved[11];					//预留
};

//视频丢失
struct NETDVR_VideoLostParamfor2_0_t
{
	unsigned char		chn;
	unsigned int 		trigRecChn;					//触发录像通道 按位
	unsigned int 		trigAlarmOut;				//触发报警输出 按位
	unsigned char 		flag_buzz;					//蜂鸣器
	unsigned char		flag_email;					//触发emaill
	unsigned char		flag_mobile;				//触发手机报警
	delay_t 			delay;						//延时
	struct AlarmInPtz	alarmptz[32];				//ptz
	unsigned int 		copy2Chnmask;				//复制到其他通道。按位
	unsigned char		reserved[12];				//保留字段
};

//视频遮挡
struct NETDVR_VideoBlockParam_t
{
	unsigned char		chn;
	unsigned int 		trigRecChn;					//触发录像通道 按位
	unsigned int 		trigAlarmOut;				//触发报警输出 按位
	unsigned char 		flag_buzz;					//蜂鸣器
	unsigned char		flag_email;					//触发emaill
	unsigned char		flag_mobile;				//触发手机报警
	delay_t				delay;						//延时
	unsigned int 		copy2Chnmask;				//复制到其他通道。按位
	unsigned char		reserved[12];				//保留字段
};



/* ========================== Alarm IN set pts structures=========================== */
struct NETDVR_alarmInPtz_t 
{
	unsigned char channo;					//the channel of the PTZ
	unsigned char flag_preset;				//enable preset point or not
	unsigned char preset;					//the enabled preset point number
	unsigned char flag_cruise;				//enable cruise or not
	unsigned char cruise;					//the enabled cruise number
	unsigned char flag_track;				//enable track or not
};

struct NETDVR_alarmHandler_t 
{
	unsigned char	flag_buzz;				//audio alarm flag
	unsigned char	flag_send;				//reserved
	unsigned char	flag_alarmout;					//trigger alarm out flag
	unsigned char	alarm_out[4];				//alarm output set
	unsigned char	reserved[28];
	unsigned char	recchan[16];				//triggered record channel
	unsigned char	flag_email;					//触发emaill
	unsigned char	flag_mobile;				//触发手机报警
	unsigned short 	delay;						//延时
	struct NETDVR_alarmInPtz_t alarminptz;
};

/* ==================== user control structure ======================== */
struct NETDVR_userInfo_t 
{
	char	name[12];
	char	password[12];

	char	mac_address[18];
	
	/* 1:open， 0:close */
	unsigned char		rcamer;						//remote yuntai
	unsigned char		rrec;						//remote record
	unsigned char		rplay;						//remote playback
	unsigned char		rsetpara;					//remote set param
	unsigned char		rlog;						//remote get log
	unsigned char		rtool;						//remote use tool
	unsigned char		rpreview;					//remote preview
	unsigned char		ralarm;						//remote alarm
	unsigned char		rvoip;						//voip
	unsigned char		lcamer;						//local yuntai
	unsigned char		lrec;						//local record
	unsigned char		lplay;						//local palyback
	unsigned char		lsetpara;					//local set param
	unsigned char		llog;						//local log
	unsigned char		ltool;						//local tool
};

struct NETDVR_UserNumber_t 
{
	struct NETDVR_userInfo_t userinfo[8];
};

/* ========================== remote HDD INFO structures=========================== */
struct NETDVR_hddInfo_t 
{
	unsigned char		hdd_index;					//硬盘序号
	unsigned char		hdd_exist;  				//1 exist; 0 not exist
	unsigned int		capability;					//MB
	unsigned int		freesize;					//MB
	unsigned char		reserved[2];				//预留

};

/* ========================== remote System Version info structures=========================== */
struct NETDVR_SysVerInfo_t
{
	char devicename[32];
	char devicemodel[32];
	char deviceser[32];
	char version[64];
};



/*=========================remote pic adjust============================*/
enum NETDVR_PICADJUST {
	NETDVR_PIC_BRIGHTNESS = 0,					
	NETDVR_PIC_CONTRAST,
	NETDVR_PIC_HUE,
	NETDVR_PIC_SATURATION,
};

struct NETDVR_PICADJUST_T
{
	unsigned char		channel_no;					//通道号
	enum NETDVR_PICADJUST		flag;						//调节标志:0-3
	unsigned char		val;						//调节值
	unsigned int		copy2chnmask;				//复制到其他通道。每一位一个通道
};

typedef enum NETDVR_TimeFormat_T
{
	NETDVR_TF_YYYYMMDD = 0,
	NETDVR_TF_MMDDYYYY = 1,
}timeFormat_t;

//系统时间
struct NETDVR_SysTime_t
{
	unsigned int		systemtime;					//系统时间
	timeFormat_t		format;						//时间格式 选项值
	unsigned char		flag_time;					//预览时间位置显示
	unsigned short		timepos_x;					//预览时间x坐标
	unsigned short		timepos_y;					//预览时间y坐标
};

enum NETDVR_YTCOM_Protocol_t
{
	YT_PROTOCOL_N5 = 0,
	YT_PROTOCOL_D4,
	YT_PROTOCOL_S8,
	YT_PROTOCOL_S7,
	YT_PROTOCOL_F2,
	YT_PROTOCOL_G2E,
	YT_PROTOCOL_G2N,
	YT_PROTOCOL_CR,
	YT_PROTOCOL_TY,
	YT_PROTOCOL_YH,
};

enum NETDVR_YTCOM_CheckBit_t
{
	YT_CHECKBIT_NONE = 0,
	YT_CHECKBIT_ODD,
	YT_CHECKBIT_EVEN,
};

enum NETDVR_YTCOM_DataBit_t
{
	YT_DATABIT_8 = 8,
	YT_DATABIT_7 = 7,
	YT_DATABIT_6 = 6,
};

enum NETDVR_YTCOM_StopBit_t
{
	YT_STOPBIT_1 = 1,
	YT_STOPBIT_2 = 2,
};

struct NETDVR_ComParam_t
{
	enum NETDVR_YTCOM_Protocol_t com_protocol;
	unsigned int  com_baudrate;
	enum NETDVR_YTCOM_CheckBit_t com_checkbit;
	enum NETDVR_YTCOM_DataBit_t com_databit;
	enum NETDVR_YTCOM_StopBit_t com_stopbit;
	unsigned char serialport;	//1-232, 2-485
	unsigned char reserved[23];	//保留
};

//系统语言列表
enum NETDVR_SYS_LANGUAGE{
	NETDVR_SYS_LANGUAGE_CHS = 0,		//简体中文
	NETDVR_SYS_LANGUAGE_ENU = 1,		//美式英文
	NETDVR_SYS_LANGUAGE_CHT = 2,       //繁体中文
	NETDVR_SYS_LANGUAGE_GER = 3,		//德语
	NETDVR_SYS_LANGUAGE_FRE = 4,		//法语
	NETDVR_SYS_LANGUAGE_SPA = 5,		//西班牙语
	NETDVR_SYS_LANGUAGE_ITA = 6,		//意大利
	NETDVR_SYS_LANGUAGE_POR = 7,		//葡萄牙语
	NETDVR_SYS_LANGUAGE_RUS = 8,		//俄语
	NETDVR_SYS_LANGUAGE_TUR = 9,		//土耳其语
	NETDVR_SYS_LANGUAGE_THA = 10,		//泰国语
	NETDVR_SYS_LANGUAGE_JAP = 11,		//日语
	NETDVR_SYS_LANGUAGE_HAN = 12,		//韩语
	NETDVR_SYS_LANGUAGE_POL = 13,		//波兰语
	NETDVR_SYS_LANGUAGE_HEB = 14,		//希伯来语Hebrew
	NETDVR_SYS_LANGUAGE_HUN = 15,		//匈牙利语Hungarian
	NETDVR_SYS_LANGUAGE_ROM = 16,		//罗马语Roma
	NETDVR_SYS_LANGUAGE_IRA = 17,		//伊朗语
	NETDVR_SYS_LANGUAGE_CZE = 18,		//捷克语
	NETDVR_SYS_LANGUAGE_VIE = 19,		//越南语
	NETDVR_SYS_LANGUAGE_LIT = 20,		//立陶宛
	NETDVR_SYS_LANGUAGE_SLO = 21,		//斯洛伐克
	NETDVR_SYS_LANGUAGE_ARA = 22,		//阿拉伯语
	NETDVR_SYS_LANGUAGE_GRE = 23,		//希腊语
	NETDVR_SYS_LANGUAGE_RMN = 24,		//罗马尼亚语
	NETDVR_SYS_LANGUAGE_FAR = 25,		//波斯语
	NETDVR_SYS_LANGUAGE_BUL = 26,		//保加利亚
	NETDVR_SYS_LANGUAGE_INVALID = -1	//仅作填充无效值用
};

struct NETDVR_SysLangList_t
{
	unsigned char max_langnum;		//最多支持语言数 <=32
	NETDVR_SYS_LANGUAGE langlist[32];	// language list, 最多32项,
	unsigned char reserved[16];	//预留
};

//位率列表
struct  NETDVR_bitRateList_t
{
	unsigned char	chn;				//通道
	unsigned char	videotype;			//0 主码流, 1 子码流
	unsigned int	bitratelist[16];	//位率列表, 单位kb, 未使用的填0
	unsigned char	reserved[16];		//预留
};

//OEM_XINGWANG
struct NETDVR_xwServer_t
{
	unsigned char   flag_server;
	unsigned int	ip_server;
	unsigned short  port_server;
	unsigned short  port_download;
	char			device_serial[32];
	char			device_passwd[32];
	unsigned char	flag_verifytime;
};

struct NETDVR_SMTPServer_t
{
	char alarm_email[32]; //Alarm out email
	char SMTP_svr[32]; //SMTP server
	char username[32]; //User name
	char userpw[32]; //User password 
	unsigned short		SMTPport;			//SMTP端口
	unsigned char		flag_ssl;			//是否使用SSL
	unsigned char reserved[13]; 
};

struct NETDVR_SMTPServerfor2_0_t
{
	char alarm_email1[32]; //Alarm out email
	char alarm_email2[32]; //Alarm out email
	char alarm_email3[32]; //Alarm out email
	char SMTP_svr[32]; //SMTP server
	char username[32]; //User name
	char userpw[32]; //User password 
	unsigned short		SMTPport;			//SMTP端口
	unsigned char		flag_ssl;			//是否使用SSL
	unsigned char reserved[13]; 
};

//龙安视安防qq
struct NETDVR_LASServer_t
{
	unsigned char sn[64];
	unsigned char productcode[20];
	unsigned char macaddr[18];
	unsigned char reserved[16];
};

//预置点列表
struct NETDVR_PresetList_t
{
	unsigned char chn;	//通道
	unsigned char totalAddedpreset; //已添加的预置点总数 <=128
	struct presetstate_t
	{
		unsigned char preset;	//预置点号  1-128
		unsigned char bAdded;	//是否已添加
		char name[12];	//预置点名称
	}presetstate[128]; //预置点状态。下标0-127分别代表1-128号预置点
};

//预置点名称
struct NETDVR_PresetName_t
{
	unsigned char chn;		//通道
	unsigned char preset;	//预置点号  1-128
	char name[12];	//预置点名称
};

//云台速度
#define PTZ_RATE_FAST	16
#define PTZ_RATE_NORMAL	17
#define PTZ_RATE_SLOW	18
struct NETDVR_PTZRate_t
{
	unsigned char chn;  //通道
	unsigned char rate; //速度 16 快速 17常速 18慢速
};


//恢复默认画面参数
struct NETDVR_Reset_Picadjust_t
{
	unsigned char chn;  //通道 0开始
	unsigned char adjtype; //按位表示类型，第1-4位分别表示亮度，对比度，色调，饱和度，置为1的表示恢复默认值， 0xf表示全恢复
};

//帧率列表 
struct NETDVR_Framerate_list_t
{
	unsigned char chn;	//通道
	unsigned char type;	// 0 主码流 1 子码流
	unsigned char framerate[10];	//帧率列表 未使用的填0
};

// struct NETDVR_FramerateListByResolution_t
// {
// 	unsigned char chn;	//通道
// 	unsigned char frametype;	// 0 主码流 1 子码流
// 	unsigned char resolutiontype;	// 见CTRL_GET_RESOLUTION_LIST
// 	unsigned char framerate[10];	//帧率列表 未使用的填0
// };

//主/子码流分辨率列表 CTRL_GET_RESOLUTION_LIST
#define NETDVR_VIDEORESOLU_BEGIN	1 //从1开始
#define NETDVR_VIDEORESOLU_QCIF		1 //QCIF
#define NETDVR_VIDEORESOLU_CIF		2 //CIF
#define NETDVR_VIDEORESOLU_HD1		3 //HD1
#define NETDVR_VIDEORESOLU_D1		4 //D1
#define NETDVR_VIDEORESOLU_720P		5
#define NETDVR_VIDEORESOLU_1080P	6
#define NETDVR_VIDEORESOLU_960H		7
//#define NETDVR_VIDEORESOLU_END	NETDVR_VIDEORESOLU_D1	//在NETDVR_VIDEORESOLU_D1这里结束，可以通过判断是否在BEGIN和END之间来确认是否正确
#define NETDVR_VIDEORESOLU_END		NETDVR_VIDEORESOLU_960H

struct NETDVR_VideoResolu_list_t
{
	unsigned char chn;	//通道
	unsigned char type;	// 0 主码流 1 子码流
	unsigned char videoresolu[8];	//分辨率列表 未使用的填0 VIDEORESOLU_BEGIN ~ VIDEORESOLU_END
};
//南瑞服务器
struct NETDVR_NRServer_t
{
	unsigned int nrserverip; //服务器地址
	unsigned short serverport; //服务器端口
	unsigned short databindport; //数据绑定端口
	unsigned char reserved[16]; //预留
};

//获取报警输入值&设置报警输出值
struct NETDVR_AlarmVal_t
{
	unsigned char alarmid;		//alarmid 0开始
	unsigned char  val;			//取值 0未触发 1触发
	unsigned char reserved[2];	//预留
};


struct	NETDVR_specialinfo_t
{
	unsigned char mainbordmode; //主板型号 预留
	unsigned char characterset; //字符集: 0-ascii，1-GB2312, 2-UTF8, 3-UTF16 
	unsigned char reserved[62]; //预留
};


struct NETDVR_MOTIONCOND
{
	unsigned int	channelmask;	//通道号  按位
	unsigned int	dwstartTime;         //录像开始时间 距离1970.1.1的UTC时间
	unsigned int    dwstopTime;         //录像结束时间 距离1970.1.1的UTC时间
	unsigned char   byRes[60];        //保留
}; 

#define MAX_SECONDS 60

struct NETDVR_MOTION_SECOND
{
	unsigned char	 byChn;				//产生此次移动侦测的通道 0开始
	unsigned int     dwMotionTime;      //录像时间 距离1970.1.1的UTC时间
	unsigned int     dwMotionNum;       // dwMotionTime这一秒内移动侦测数目
	unsigned char    byRes[12];          //保留
};

struct NETDVR_MOTION_DATA
{
	unsigned int  dwSecondNum;    //该结构中包含多少秒的信息
	struct NETDVR_MOTION_SECOND  tMotionData[MAX_SECONDS];
};

struct NETDVR_Makekeyframe_t
{
	unsigned char chn;			//通道 0开始
	unsigned char type;		// 0 主码流 1子码流
	unsigned char reserved[6];	//预留
};  // 8 Byte

struct  NETDVR_CrearSvr_Info_t
{
	unsigned char flagPlatform; //是否启用平台 0不启用 1启用
	unsigned int ipPlatform; //平台地址
	unsigned short portPlatform; //平台端口
	char PUID[19];//PUID
	char passwd[32]; //接入密码
	unsigned char  reserved[32]; //预留
};

struct NETDVR_DDNSinfo_t
{
	char ddns_user[64];				//ddns用户名
	char ddns_passwd[64];			//ddns密码
	char ddns_domain[64];			//ddns域名
	unsigned char  reserved[16];	//预留
};

//报警输入布防
struct AlarmTimeField_t
{
	unsigned int starttime; //起始时间
    unsigned int endtime; //终止时间
    unsigned char  flag_alarm; //报警输入
    unsigned char reserved[6]; //保留字段
};

struct NETDVR_AlarmSCH_t
{
	unsigned char chn; //通道
	enum NETDVR_WEEKDAY weekday; //星期
	struct AlarmTimeField_t alarmTimeFieldt[4];
	unsigned char copy2Weekday; //复制到其他天  按位 
	unsigned int copy2Chnmask; //复制到其他通道。按位
	unsigned char reserved[16]; //保留字段
};

struct  NETDVR_MDSenselist_t
{
	unsigned char	mdsenselist[12];	//移动侦测灵敏度列表
};

struct  NETDVR_MDAlarmDelaylist_t
{
	unsigned short	mdalarmdelaylist[16];	//移动侦测报警输出延时列表
};

struct  NETDVR_BaudRateList_t
{
	//csp modify
	//unsigned short baudratelist[16];	//波特率列表
	unsigned int baudratelist[16];		//波特率列表
};

struct  NETDVR_PTZProtocolList_t
{
	unsigned char   maxNum;
	unsigned char	baudratelist[19];	//云台协议列表
};

struct RecSchTimeField_t
{
	unsigned int starttime; //起始时间
	unsigned int endtime; //终止时间
	unsigned char reserved[6]; //保留字段
};

struct  NETDVR_RecSchTime_t
{
	unsigned char chn; //通道
	unsigned char weekday; //星期
	unsigned char type;
	RecSchTimeField_t TimeFiled[24];
    unsigned char copy2Weekday; //复制到其他天  按位 
    unsigned int copy2Chnmask; //复制到其他通道。按位
	unsigned char reserved[16]; //保留字段
};

struct  NETDVR_HuiNaInfo_t
{
	unsigned int admin_host;             //现在做会纳3g数据服务器ip地址
	unsigned short ctrl_port;            //会纳登陆端口
	unsigned short data_port;            //会纳数据端口
	unsigned char   server_enable_flag;  //心跳开启使能
	unsigned short   heartbeat_time;	 //心跳周期
	char   huina_http_ulr[64];			 //会纳http地址
	char   device_flag[16];				 //设备编号
	char   shop_num[16];				 //会纳店铺编号
	unsigned char reserved[5];
};

//csp modify
struct NETDVR_reboottime_t
{
	unsigned int  reboot_time; // 设备重启时间，已分钟为单位
	unsigned char reserve[4];
};

//csp modify
struct NETDVR_alarmtimeinterval_t
{
	unsigned short alarm_timeinterval; //s为单位，报警上传时间间隔
	unsigned char reserve[2];
};

struct NETDVR_DDNSList_t
{
	char DDNSlist[20][20];
};
#pragma pack( pop )

/* =============== NETDVR user function definetion  ============= */

//NETDVR must call NETDVR_startup() to startup at the beginning, and call NETDVR_cleanup at the ending
//////////////////////////////////////////////////////////////////////////
// 初始化
int __stdcall NETDVR_startup(void);
int __stdcall NETDVR_cleanup(void);

//NETDVR_createDVR to create a new DVR, NETDVR_destroyDVR to destroy an exist DVR
//////////////////////////////////////////////////////////////////////////
//创建 &销毁
int __stdcall NETDVR_createDVR(int *p_handle, unsigned int serverip, unsigned short serverport);
int __stdcall NETDVR_createDVRbyDomain(int *p_handle, char *pDomain, unsigned short serverport);
int __stdcall NETDVR_destroyDVR(int Handle);


//NETDVR_regCBMsgConnLost:register a callback function to deal with connectin lost notification message for dvr
//注册断链通知
int __stdcall NETDVR_regCBMsgConnLost(int Handle, PFUN_MSG_T p_cb_func, unsigned int dwContent);


//////////////////////////////////////////////////////////////////////////
// 登录 &注销
/*
NETDVR_loginServer let DVR login to a server
NETDVR_logoutServer to logout from a server
*/
int __stdcall NETDVR_loginServer(int Handle, const struct NETDVR_loginInfo_t *p_para);
int __stdcall NETDVR_logoutServer(int Handle);

//csp modify 20130519
//获取用户高级权限
int __stdcall NETDVR_GetAdvPrivilege(int Handle, const struct NETDVR_loginInfo_t *pLoginInfo, struct NETDVR_AdvPrivilege_t *pAdvPrivilege);

//////////////////////////////////////////////////////////////////////////
// 实时预览
#pragma pack( push, 4 )
struct RealPlayClientInfo_t
{
	unsigned char	rcv_chn; //channel
	unsigned char	streamtype; //0 main stream, 1 substream
	pFrameCallBack	pEncFrameCBFunc; //Call back function to encode
	unsigned int	dwEncFrameContent; //Context for encoder callback function to identify play reciever
	pDecFrameCallBack	pDecFrameCBFunc; //Call back function to decode
	unsigned int	dwDecFrameContent; //Context for decoder callback function to identify play reciever
	//csp modify 20130423
	unsigned char	wnd_num;
	unsigned char	wnd_index;
	unsigned char	reserved[6];
};
#pragma pack( pop )

// 开始&停止预览
int __stdcall NETDVR_StartRealPlay(int Handle, const struct RealPlayClientInfo_t* pClientinfo, int* pRealPlayHandle);
int __stdcall NETDVR_StopRealPlay(int nRealPlayHandle);

//csp modify 20130423
int __stdcall NETDVR_SetMonitorInfo(int nRealPlayHandle, unsigned char chn, unsigned char wnd_num, unsigned char wnd_index);

//控制是否解码
int __stdcall NETDVR_SetVideoDecFlag(int nRealPlayHandle, unsigned char bDec);

//控制音频
int __stdcall NETDVR_OpenRealAudio(int nRealPlayHandle);
int __stdcall NETDVR_CloseRealAudio(int nRealPlayHandle);
int __stdcall NETDVR_MuteRealAudio(int nRealPlayHandle, bool bMute);

//抓图
int __stdcall NETDVR_snapshot(int nRealPlayHandle, char *path, char *filename);

//录像
int __stdcall NETDVR_startRecord(int nRealPlayHandle, char *p_dir_path, unsigned int file_max_len);
int __stdcall NETDVR_stopRecord(int nRealPlayHandle);
int __stdcall NETDVR_setRecordCB(int nRealPlayHandle, pFrameCallBack pRecordCBFun, unsigned int dwContent);
int __stdcall NETDVR_setRecordFileNameCB(int nRealPlayHandle, pRecFilenameCallBack pRecFilenameCBFunc, unsigned int dwContent);

//////////////////////////////////////////////////////////////////////////
// 远程录像搜索
/*
NETDVR_recFilesSearch: index record file(s)
NETDVR_recFilesSearchClean: clean the index result
*/
int __stdcall NETDVR_recFilesSearch(int Handle, const struct NETDVR_fileSearchCondition_t *prfs, struct NETDVR_recFileSearchResult_t *pdesc);
int __stdcall NETDVR_recFilesSearchClean(struct NETDVR_recFileSearchResult_t *pdesc);

//////////////////////////////////////////////////////////////////////////
// 下载
/*
NETDVR_regCBFileRecieverProgress: set the reciever progress callback function
NETDVR_regCBFileRecieverError: set the reciever error callback function
*/
int __stdcall NETDVR_regCBFileRecieverProgress(int Handle, PFUN_PROGRESS_T p_cb_progress, unsigned int dwContent);
int __stdcall NETDVR_regCBFileRecieverError(int Handle, PFUN_ERROR_T p_cb_err, unsigned int dwContent);

/*
NETDVR_startFileDownload: start to download file
NETDVR_stopFileDownload: stop downloading file
*/
int __stdcall NETDVR_startFileDownload(int Handle,  char *s_save_dir, char *s_save_filename, const struct NETDVR_recFileInfo_t *pFileInfo);
int __stdcall NETDVR_stopFileDownload(int Handle);


//////////////////////////////////////////////////////////////////////////
// 回放
#pragma pack( push, 4 )
struct PlayBackClientInfo_t
{
	unsigned char sendmode;	//0:按照帧实际时间间隔发送  1:无时间间隔直接发送
	pFrameCallBack	pEncFrameCBFunc; //Call back function to encode
	unsigned int	dwEncFrameContent; //Context for encoder callback function to identify play reciever
	pDecFrameCallBack	pDecFrameCBFunc; //Call back function to decode
	unsigned int	dwDecFrameContent; //Context for decoder call function to identify play reciever
	PFUN_PROGRESS_T	pProgressCBFunc; //register a callback function to deal with progress notification message for one dvr's play reciever
	unsigned int	dwProgressContent; //Context for progress callback function to identify play reciever
	PFUN_MSG_T		pPlayOverCBFunc; //register a callback function to deal with play over notification message for one dvr's play reciever
	unsigned int	dwPlayOverContent; //Context for play over callback function to identify play reciever
};
#pragma pack( pop )

//开始 &停止回放
int __stdcall NETDVR_startPlayByFile(int Handle,  const struct NETDVR_recFileInfo_t *pFileInfo, const struct PlayBackClientInfo_t* pClientInfo, int* pPlayHandle);
int __stdcall NETDVR_startPlayByTime(int Handle,  const struct NETDVR_TimePlayCond_t *pTimePlayInfo, const struct PlayBackClientInfo_t* pClientInfo, int* pPlayHandle);
int __stdcall NETDVR_stopPlayBack(int nPlayBackHandle);

//回放控制
//静音(不停码流)
int __stdcall NETDVR_setPlayCBAudioMute(int nPlayBackHandle, bool bMute);
/*
NETDVR_pausePlay: pause a play
NETDVR_resumePlay: resume a paused play
NETDVR_singleFramePlay: single frame play
NETDVR_fastPlay: play faster(when playrate = 8,  reset playrate to 1)
NETDVR_slowPlay: play slower(when playrate = -8,  reset playrate to 1)
NETDVR_setPlayRate: set play rate to a new value(play_rate [-8, 8])(-8 means 1/8)
*/
int __stdcall NETDVR_pausePlay(int nPlayBackHandle);
int __stdcall NETDVR_resumePlay(int nPlayBackHandle);
int __stdcall NETDVR_singleFramePlay(int nPlayBackHandle);
int __stdcall NETDVR_fastPlay(int nPlayBackHandle);
int __stdcall NETDVR_slowPlay(int nPlayBackHandle);
int __stdcall NETDVR_setPlayRate(int nPlayBackHandle, int play_rate);
/*
NETDVR_playPrevious: play previous segments when play by file; play previous file when play by time
NETDVR_playNext: play next segments when play by file; play next file when play by time
NETDVR_playSeek: play to a new position
NETDVR_playMute: play in mute or not depending on b_mute
NETDVR_playProgress: let DVR send play progress depending on b_send_progress
*/
int __stdcall NETDVR_playPrevious(int nPlayBackHandle);
int __stdcall NETDVR_playNext(int nPlayBackHandle);
int __stdcall NETDVR_playSeek(int nPlayBackHandle, unsigned int new_time);
int __stdcall NETDVR_playMute(int nPlayBackHandle, int b_mute);
int __stdcall NETDVR_playProgress(int nPlayBackHandle, int b_send_progress);


//////////////////////////////////////////////////////////////////////////
//参数配置

/*
NETDVR_setSystemParams : set system parameters
NETDVR_getSystemParams : get system parameters
*/
int __stdcall NETDVR_setSystemParams(int Handle, const struct NETDVR_systemParam_t *pSysPara);
int __stdcall NETDVR_getSystemParams(int Handle, struct NETDVR_systemParam_t *pSysPara);

/*
NETDVR_setPtzParam : set PTZ parameters for one channel
NETDVR_getPtzParam : get PTZ parameters for one channel
NETDVR_setPtzParams: send a command(ptz_cmd) to control the ptz

*/
int __stdcall NETDVR_setPtzParams(int Handle, const struct NETDVR_ptzParam_t *ptzParam);
int __stdcall NETDVR_getPtzParams(int Handle, unsigned char chn, struct NETDVR_ptzParam_t *p_ptz_param);
int __stdcall NETDVR_PtzControl(int Handle, const struct NETDVR_PtzCtrl_t *p_para);

/*

NETDVR_startYuntaiCruise: start to run a designated cruise
NETDVR_stopYuntaiCruise: stop running a designated cruise
*/
int __stdcall NETDVR_startYuntaiCruise(int Handle, unsigned char chn, unsigned char path_no);
int __stdcall NETDVR_stopYuntaiCruise(int Handle, unsigned char chn, unsigned char path_no);

/*
NETDVR_startVoip: let the DVR's voip channel start sending and recieving real voip
NETDVR_stopVoip: let the DVR's voip channel stop sending and recieving real voip
*/
int __stdcall NETDVR_startVOIP(int Handle, int voipindex);
int __stdcall NETDVR_stopVOIP(int Handle, int voipindex);

int __stdcall NETDVR_VOIPRegRcvCB(int Handle, int voipindex, pDecFrameCallBack rcvfunc, unsigned int dwContext);
int __stdcall NETDVR_VOIPRegCaptureCB(int Handle, int voipindex, pDecFrameCallBack getcapturefunc, unsigned int dwContext);
int __stdcall NETDVR_VOIPSetSendMode(int Handle, int voipindex, unsigned char flagmode);
int __stdcall NETDVR_VOIPSendData(int Handle, int voipindex, FrameHeadrDec voipdata);


/*
NETDVR_logSearch: index log records
NETDVR_logSearchClean: clean the index result
*/
int __stdcall NETDVR_logSearch(int Handle, struct NETDVR_logSearchCondition_t *p_condition, struct NETDVR_logSearchResult_t *p_result);
int __stdcall NETDVR_logSearchClean(struct NETDVR_logSearchResult_t *p_result);

/*
NETDVR_setVideoParams : set video parameters
NETDVR_getVideoParams : get video parameters
*/
int __stdcall NETDVR_setVideoParams(int Handle, const struct NETDVR_videoParam_t *p_para);
int __stdcall NETDVR_getVideoParams(int Handle, unsigned char chn, struct NETDVR_videoParam_t *p_para);

//yaogang modify 20170715 简易设置通道名的接口
int __stdcall NETDVR_SetChnName(int Handle, unsigned char chn, const char *pname, int len);
int __stdcall NETDVR_GetChnName(int Handle, unsigned char chn, char *pname, int size);


/*
NETDVR_setRecordParams : set record parameters
NETDVR_getRecordParams : get record parameters
*/
int __stdcall NETDVR_setRecordParams(int Handle, const struct NETDVR_recordParam_t *p_para);
int __stdcall NETDVR_getRecordParams(int Handle, unsigned char chn, struct NETDVR_recordParam_t *p_para);


/*
NETDVR_setAlarmInParams : set input alarm parameters
NETDVR_getAlarmInParams : get input alarm parameters
*/
int __stdcall NETDVR_setAlarmInParams(int Handle, const struct NETDVR_alarmInParam_t *p_para);
int __stdcall NETDVR_getAlarmInParams(int Handle, unsigned char in_id, struct NETDVR_alarmInParam_t *p_para);



/*
NETDVR_setAlarmOutParams : set output alarm parameters
NETDVR_getAlarmOutParams : get output alarm parameters
NETDVR_clearAlarms: clear all the alarms
*/
int __stdcall NETDVR_setAlarmOutParams(int Handle, const struct NETDVR_alarmOutParam_t *p_para);
int __stdcall NETDVR_getAlarmOutParams(int Handle, unsigned char out_id, struct NETDVR_alarmOutParam_t *p_para);
int __stdcall NETDVR_clearAlarms(int Handle);


/*
NETDVR_setNetworkParams : set network parameters
NETDVR_getNetworkParams : get network parameters
*/
int __stdcall NETDVR_setNetworkParams(int Handle, const struct NETDVR_networkParam_t *p_para);
int __stdcall NETDVR_getNetworkParams(int Handle, struct NETDVR_networkParam_t *p_para);

/*

NETDVR_regCBUpdateProgress: register a callback function for update progress
NETDVR_startUpdate: each time before calling NETDVR_startUpdate, NETDVR_initUpdate must be called at first
NETDVR_stopUpdate:stop update
*/

int __stdcall NETDVR_regCBUpdateProgress(int Handle, PFUN_PROGRESS_T p_cb_func, unsigned int dwContent);
int __stdcall NETDVR_regCBUpdateError(int Handle, PFUN_ERROR_T p_cb_err, unsigned int dwContent);
int __stdcall NETDVR_startUpdate(int Handle, const struct NETDVR_updateParam_t *p_update_para, char *s_update_dir, char *s_update_filename);
int __stdcall NETDVR_stopUpdate(int Handle);

/*
NETDVR_restoreFactorySettings: To restore the factory settings
NETDVR_reboot: To reboot the dvr
NETDVR_shutdown: To shutdown the dvr
*/
int __stdcall NETDVR_restoreFactorySettings(int Handle);
int __stdcall NETDVR_reboot(int Handle);
int __stdcall NETDVR_shutdown(int Handle);

/*
NETDVR_getSystemTime: To get the system time
NETDVR_setSystemTime: To set the system time
*/
int __stdcall NETDVR_getSystemTime(int Handle, struct NETDVR_SysTime_t *p_para);
int __stdcall NETDVR_setSystemTime(int Handle, const struct NETDVR_SysTime_t *p_para);

/*
NETDVR_getMotionDection: To get motion dection area, flag and sense
NETDVR_setMotionDection: To set motion dection area, flag and sense
*/
int __stdcall NETDVR_getMotionDection(int Handle, unsigned char chn, struct NETDVR_motionDetection_t *p_para);
int __stdcall NETDVR_setMotionDection(int Handle, const struct NETDVR_motionDetection_t *p_para);

/*
*/
int __stdcall NETDVR_getVideoLost(int Handle, unsigned char chn, struct NETDVR_VideoLostParam_t *p_para);
int __stdcall NETDVR_setVideoLost(int Handle, const struct NETDVR_VideoLostParam_t *p_para);

/*
NETDVR_remoteGetHDDInfo:get system hdd info
*/
int __stdcall NETDVR_remoteGetHddInfo(int Handle, unsigned char hddindex, struct NETDVR_hddInfo_t *p_hddinfo);

/*
NETDVR_remoteGetSYSVerInfo: get system version info
*/
int __stdcall NETDVR_remoteGetSysVerInfo(int Handle, struct NETDVR_SysVerInfo_t *p_para);

/*
NETDVR_sdkChkout: check sdk version
*/
int __stdcall NETDVR_sdkCheckout(int Handle, char* chkstring);

/*
NETDVR_setConnectTimeout: set connect timeout
NETDVR_getConnectTimeout: get current connect timeout, default: 3000 ms
*/
int __stdcall NETDVR_setConnectTimeout(unsigned int millisecond);
int __stdcall NETDVR_getConnectTimeout(unsigned int *pMillisecond);

int __stdcall NETDVR_setPicAdjust(int Handle, const struct NETDVR_PICADJUST_T *p_para);
int __stdcall NETDVR_getPicAdjust(int Handle, unsigned char chn, enum NETDVR_PICADJUST type, struct NETDVR_PICADJUST_T *p_para);


int __stdcall NETDVR_sendExtendCmd(int Handle, unsigned short wCommand, const void *pInData, int nInDataLen, void* pOutData, int nMaxOutDatalen);

int __stdcall NETDVR_GetDeviceInfo(int Handle, struct NETDVR_DeviceInfo_t *pDeviceInfo);
int __stdcall NETDVR_GetVideoProperty(int Handle, struct NETDVR_VideoProperty_t *pVideoPro);
int __stdcall NETDVR_GetAudioProperty(int Handle, struct NETDVR_AudioProperty_t *pVideoPro);
int __stdcall NETDVR_GetVoipProperty(int Handle, struct NETDVR_VOIPProperty_t *pVoipPro);
int __stdcall NETDVR_GetMDProperty(int Handle, struct NETDVR_MDProperty_t *pMDPro);

int __stdcall NETDVR_GetVGAsoluton(int Handle, struct NETDVR_VGARESOLLIST *pvgasol);

int __stdcall NETDVR_SetCruiseParam(int Handle, const struct NETDVR_cruisePath_t *p_cruise_path);
int __stdcall NETDVR_GetCruiseParam(int Handle, unsigned char chn, unsigned char pathnum, struct NETDVR_cruisePath_t *p_cruise_path);

int __stdcall NETDVR_GetSubStreamParam(int Handle, unsigned char chn, struct NETDVR_SubStreamParam_t *p_para);
int __stdcall NETDVR_SetSubStreamParam(int Handle, const struct NETDVR_SubStreamParam_t *p_para);

int __stdcall NETDVR_GetAlarmNoticeParam(int Handle, struct NETDVR_AlarmNoticeParam_t *p_para);
int __stdcall NETDVR_SetAlarmNoticeParam(int Handle, const struct NETDVR_AlarmNoticeParam_t *p_para);

int __stdcall NETDVR_GetRecordSCH(int Handle, unsigned char chn, enum NETDVR_WEEKDAY day, struct NETDVR_RecordSCH_t *p_para);
int __stdcall NETDVR_SetRecordSCH(int Handle, const struct NETDVR_RecordSCH_t *p_para);

int __stdcall NETDVR_GetVideoBlockParam(int Handle, unsigned char chn, struct NETDVR_VideoBlockParam_t *p_para);
int __stdcall NETDVR_SetVideoBlockParam(int Handle, const struct NETDVR_VideoBlockParam_t *p_para);

int __stdcall NETDVR_GetUserInfo(int Handle, struct NETDVR_UserNumber_t *p_para);
int __stdcall NETDVR_AddUserInfo(int Handle, const struct NETDVR_userInfo_t *p_para);
int __stdcall NETDVR_EditUserInfo(int Handle, const struct NETDVR_userInfo_t *p_para);
int __stdcall NETDVR_DelUserInfo(int Handle, const char *username);

int __stdcall NETDVR_GetRecordState(int Handle, struct NETDVR_ManualRecord_t *p_para);
int __stdcall NETDVR_SetRecordState(int Handle, const struct NETDVR_ManualRecord_t *p_para);

//搜索设备相关
typedef void (CALLBACK*  PFUN_SearchDevice_CB)(struct NETDVR_DeviceInfo_t dev, void* pContext);
int __stdcall NETDVR_RegCallBackSearchDevice(PFUN_SearchDevice_CB p_cb_func, void* pContext);
int __stdcall NETDVR_SearchDevice();

int __stdcall NETDVR_PatrolSync(); //广播设备，同步轮巡


int __stdcall NETDVR_SetYTTrack(int Handle, unsigned char chn, enum NETDVR_YTTRACKCONTROL yt_cmd);
int __stdcall NETDVR_SetYTPresetPoint(int Handle, unsigned char chn, unsigned char preset_pos, enum NETDVR_YTPRESETPOINTCONTROL yt_com);


int __stdcall NETDVR_regCBAlarmState(int Handle, PFUN_ALARMSTATE_T p_cb_func, unsigned int dwContent);
int __stdcall NETDVR_SetAlarmUpload(int Handle, const unsigned char uploadflag);

int __stdcall NETDVR_DeinterlaceVideoFrame(pFrameHeadrDec pFrmHdrDec);
int __stdcall NETDVR_CtrlCDROM(int Handle,unsigned char ctrlflag);

//serialport 1－232串口；2－485串口 
int __stdcall NETDVR_GetComParam(int Handle, unsigned char serialport, struct NETDVR_ComParam_t *p_para);
int __stdcall NETDVR_SetComParam(int Handle, const struct NETDVR_ComParam_t *p_para);


//透明串口
typedef void(CALLBACK*  pSerialDataCallBack)(int lSerialPort, unsigned char bySerialChannel, char *pRecvDataBuffer, unsigned int dwBufSize, unsigned int dwContent);
//lSerialPort 1－232串口；2－485串口 
int __stdcall NETDVR_SerialStart(int Handle, int lSerialPort,	pSerialDataCallBack cbSerialDataCallBack, unsigned int dwContent);
//byChannel, 使用485串口时有效，从1开始；232串口作为透明通道时该值设置为0 
//dwBufSize 最大值4096
int __stdcall NETDVR_SerialSend(int Handle, int lSerialPort, unsigned char byChannel, char* pSendBuf, unsigned int dwBufSize);
int __stdcall NETDVR_SerialStop(int Handle, int lSerialPort);

//重连
enum RECONNMSG{RECONN_BEGIN, RECONN_SUCESS, RECONN_FAILED};
typedef void (CALLBACK*  pCBReconnMsg)(enum RECONNMSG msg, unsigned int dwContent);
//设置全局重连时间，单位毫秒。默认10s
int __stdcall NETDVR_setReconnectTime(unsigned int millisecond);
//注册重连消息回调
int __stdcall NETDVR_regCBMsgReconnect(int Handle, pCBReconnMsg p_cb_func, unsigned int dwContent);
//设置重连标记 1 开启重连 0 关闭重连 。默认不重连
int __stdcall NETDVR_setReconnectFlag(int Handle, unsigned char reconflag);


//获得系统支持语言列表
int __stdcall NETDVR_getSysLangList(int Handle, NETDVR_SysLangList_t* p_para);

//获得通道视频位率列表
int __stdcall NETDVR_getBitRateList(int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_bitRateList_t* p_para);

//xwserver
int __stdcall NETDVR_getxwServerParams(int Handle, struct NETDVR_xwServer_t *pxwServ);
int __stdcall NETDVR_setxwServerParams(int Handle, const struct NETDVR_xwServer_t *pxwServ);

//SMTP
int __stdcall NETDVR_getSMTPServerParams(int Handle, struct NETDVR_SMTPServer_t *pSMTPServ);
int __stdcall NETDVR_setSMTPServerParams(int Handle, const struct NETDVR_SMTPServer_t *pSMTPServ);

//龙安视安防qq
int __stdcall NETDVR_getLASAFQQInfo(int Handle, struct NETDVR_LASServer_t *pLASServ);
int __stdcall NETDVR_setLASAFQQInfo(int Handle, const struct NETDVR_LASServer_t *pLASServ);

//远程格式化
typedef void (CALLBACK*  pFormatProgressCallBack)(unsigned char hddindex, unsigned char pos, unsigned short errcode, unsigned int dwContent);
int __stdcall NETDVR_formatHdd(int Handle, unsigned char hddindex, pFormatProgressCallBack pCBFunc, unsigned int dwContent);

int __stdcall NETDVR_remoteSnap(int Handle, unsigned char chn, char* filename);

int __stdcall NETDVR_IsUseUTCTime(int Handle, unsigned char *pFlag);

//获得预置点列表
int __stdcall NETDVR_getPresetList(int Handle, unsigned char chn, struct NETDVR_PresetList_t* pList);

//添加预置点(带名称)
int __stdcall NETDVR_AddPresetByName(int Handle, const struct NETDVR_PresetName_t *p_para);

//获得PTZ速度
int __stdcall NETDVR_getPTZRate(int Handle, unsigned char chn, struct NETDVR_PTZRate_t* p_para);

//重置画面调节参数
int __stdcall NETDVR_resetPicAdjust(int Handle, const struct NETDVR_Reset_Picadjust_t *pPara);

//获得帧率列表
int __stdcall NETDVR_getFrameRateList(int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_Framerate_list_t* p_para);

//获得分辨率列表
int __stdcall NETDVR_getVideoResoluList(int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_VideoResolu_list_t* p_para);

//获得系统支持最大遮盖数
int __stdcall NETDVR_getMaxIMGMaskNum(int Handle,  unsigned char* pNum);

//南瑞服务器
int __stdcall NETDVR_setNRServer(int Handle, const struct NETDVR_NRServer_t *p_para);
int __stdcall NETDVR_getNRServer( int Handle, NETDVR_NRServer_t * p_para);

//报警
int __stdcall NETDVR_setAlarmOutVal(int Handle, const struct NETDVR_AlarmVal_t *p_para);//设置报警输出值
int __stdcall NETDVR_getAlarmInVal( int Handle, unsigned char alarm_in_id,struct NETDVR_AlarmVal_t * p_para);//获取报警输入值

//按时间下载
//设备句柄， 通道，类型，起始时间， 注册回调集合，下载句柄
#pragma pack( push, 4 )
struct TimeDownloadInfo_t
{
	unsigned short rcv_chn; //channel  通道1-16
	unsigned short rcv_chn17to32; //channel 通道17-32
	unsigned short streamtype; //0 main stream, 1 substream
	unsigned int startTime;	
	unsigned int endTime;	
	PFUN_TIMEDLPROGRESS_T p_cb_progress;
	unsigned int dwProgressContent;
	PFUN_ERROR_T p_cb_err;
	unsigned int dwErrContent;
	PFUN_SAVE_T p_cb_save;
	unsigned int dwSaveContent;
};
#pragma pack( pop )
int __stdcall NETDVR_startTimeDownload(int Handle, const TimeDownloadInfo_t *pDownloadInfo, int* pDownloadHandle);
int __stdcall NETDVR_stopTimeDownload(int nDownloadHandle);

//获取报警上传状态
int __stdcall NETDVR_getAlarmUploadState( int Handle, unsigned char id, unsigned char type, NETDVR_AlarmUploadState_t* pPara);

//特殊设备信息
int __stdcall NETDVR_getSpecialinfo_t(int Handle, NETDVR_specialinfo_t* pPara);

//查找移动侦测信息

//开始查找
int __stdcall NETDVR_FindMotion_Info(int Handle, const NETDVR_MOTIONCOND* pMDCond, int* lpFindHandle);
//查找下一条
int __stdcall NETDVR_GetNextMotionInfo(int lFindHandle, NETDVR_MOTION_DATA* lpMotiondData);
//结束查找
int __stdcall NETDVR_FindMotionClose(int lFindHandle);

//请求关键帧
int __stdcall NETDVR_MakeKeyFrame(int Handle, NETDVR_Makekeyframe_t* pPara);

//获得创世平台信息
int __stdcall NETDVR_getCrearSvrInfo(int Handle, struct NETDVR_CrearSvr_Info_t *p_para);
//设置创世平台信息
int __stdcall NETDVR_setCrearSvrInfo(int Handle, const struct NETDVR_CrearSvr_Info_t *p_para);

int __stdcall NETDVR_startRecord2(int nRealPlayHandle, char *p_dir_path, unsigned int file_max_len);
int __stdcall NETDVR_stopRecord2(int nRealPlayHandle);
int __stdcall NETDVR_setRecord2CB(int nRealPlayHandle, pFrameCallBack pRecordCBFun, unsigned int dwContent);
int __stdcall NETDVR_setRecord2FileNameCB(int nRealPlayHandle, pRecFilenameCallBack pRecFilenameCBFunc, unsigned int dwContent);
int __stdcall NETDVR_GetRecord2State(int nRealPlayHandle, unsigned char* pState);

//注册和注销DDNS
int __stdcall NETDVR_RegistDdns( int Handle, const struct NETDVR_DDNSinfo_t *pPara );
int __stdcall NETDVR_CancelDdns( int Handle, const struct NETDVR_DDNSinfo_t *pPara );

int __stdcall NETDVR_GetAlarmSCH(int Handle, unsigned char chn, enum NETDVR_WEEKDAY day, struct NETDVR_AlarmSCH_t *p_para); 
int __stdcall NETDVR_SetAlarmSCH(int Handle, const struct NETDVR_AlarmSCH_t *p_para); 

int __stdcall NETDVR_RegDecCB(int nRealPlayHandle, pDecFrameCallBack pCBFun, unsigned int dwContent);

//for code2.0
int __stdcall NETDVR_getMDSenseList( int Handle, NETDVR_MDSenselist_t* p_para );
int __stdcall NETDVR_getMDAlarmDelayList( int Handle, NETDVR_MDAlarmDelaylist_t* p_para );
int __stdcall NETDVR_getBaudRateList( int Handle, NETDVR_BaudRateList_t* p_para );
int __stdcall NETDVR_getPTZProtocolList( int Handle, NETDVR_PTZProtocolList_t* p_para );

//int __stdcall NETDVR_getFrameRateListByresolution( int Handle, NETDVR_FramerateListByResolution_t* pList );
int __stdcall NETDVR_GetRecordSCHByType(int Handle, unsigned char chn, unsigned char SchType, enum NETDVR_WEEKDAY day, struct NETDVR_RecSchTime_t *p_para);
int __stdcall NETDVR_SetRecordSCHByType(int Handle, const struct NETDVR_RecSchTime_t *p_para);
//

int __stdcall NETDVR_getHuiNaInfo(int Handle, struct NETDVR_HuiNaInfo_t *p_para);
int __stdcall NETDVR_setHuiNaInfo(int Handle, const struct NETDVR_HuiNaInfo_t *p_para);

//for2.0
int __stdcall NETDVR_getMotionDectionfor2_0(int Handle, unsigned char chn, struct NETDVR_motionDetectionfor2_0_t *p_para);
int __stdcall NETDVR_setMotionDectionfor2_0(int Handle, const struct NETDVR_motionDetectionfor2_0_t *p_para);

/*
*/
int __stdcall NETDVR_getVideoLostfor2_0(int Handle, unsigned char chn, struct NETDVR_VideoLostParamfor2_0_t *p_para);
int __stdcall NETDVR_setVideoLostfor2_0(int Handle, const struct NETDVR_VideoLostParamfor2_0_t *p_para);

int __stdcall NETDVR_getSMTPServerParamsfor2_0(int Handle, struct NETDVR_SMTPServerfor2_0_t *pSMTPServ);
int __stdcall NETDVR_setSMTPServerParamsfor2_0(int Handle, const struct NETDVR_SMTPServerfor2_0_t *pSMTPServ);

//csp modify
int __stdcall NETDVR_getRebootTime(int Handle, struct NETDVR_reboottime_t *p_para);
int __stdcall NETDVR_getSDKVersion(char* szVersion);
//int __stdcall NETDVR_setSDKLogFlag(unsigned char nFlag);
int __stdcall NETDVR_getAlarmTimeInterval(int Handle, struct NETDVR_alarmtimeinterval_t *p_para);
int __stdcall NETDVR_setAlarmTimeInterval(int Handle, const struct NETDVR_alarmtimeinterval_t *p_para);

int __stdcall NETDVR_getDDNSList(int Handle, struct NETDVR_DDNSList_t *p_para);
/************************* below functions under development********************************/

//xdc
int __stdcall NETDVR_seachIPCList(int Handle,struct NETDVR_ipcSeachCondition *ipcS,struct NETDVR_ipcSearch *p_para);
int __stdcall NETDVR_getAddIPCList(int Handle,struct NETDVR_ipcSeachCondition *ipcS,struct NETDVR_ipcSearch *p_para);
int __stdcall NETDVR_setIPC(int Handle,struct NETDVR_ipcInfo *pIPC);
//pIPC->channel_no 取值在[1-16]，则添加到指定通道
//= 0，则添加到最小未使用的通道
int __stdcall NETDVR_addIPC(int Handle,struct NETDVR_ipcInfo *pIPC);
int __stdcall NETDVR_deleteIPC(int Handle,struct NETDVR_ipcInfo *pIPC);
//yaogang modify 20141025 
//NETDVR_seachIPCList ,  NETDVR_getAddIPCList 会创建各自的链表。
//在此释放链表
int __stdcall NETDVR_destoryIPCList(struct NETDVR_ipcSearch *p_para);

//patrol para
int __stdcall NETDVR_GetPatrolPara(int Handle, struct NETDVR_PatrolPara *p_para, int *psize);
int __stdcall NETDVR_SetPatrolPara(int Handle, struct NETDVR_PatrolPara *p_para);

//preview para
int __stdcall NETDVR_GetPreviewPara(int Handle, struct NETDVR_PreviewPara *p_para);
int __stdcall NETDVR_SetPreviewPara(int Handle, struct NETDVR_PreviewPara *p_para);

//通道属性
int __stdcall NETDVR_GetChnPara(int Handle, struct NETDVR_ChnPara * p_para);
int __stdcall NETDVR_SetChnPara(int Handle, struct NETDVR_ChnPara *p_para);

//清除桌面报警三角标（注意只有在当前没有报警源存在时，才会清除）
int __stdcall NETDVR_CleanDesktopAlarmIcon(int Handle); 

//跃天解码板CMS remote CTRL
int __stdcall NETDVR_CLOSE_GUIDE(int Handle);

//yaogang modify 20160122
//获取IPC通道连接状态(一个IPC有两个通道，主、子码流)
int __stdcall NETDVR_GetIPCChnLinkStatus(int Handle, struct NETDVR_IPCChnStatus * p_para);




#ifdef __cplusplus
}
#endif
         
#endif
