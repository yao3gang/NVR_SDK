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

//¼����һط�����
enum NETDVR_REC_INDEX_MASK {
	NETDVR_REC_INDEX_TIMER = 0x1,
	NETDVR_REC_INDEX_MD = 0x2,
	NETDVR_REC_INDEX_ALARM = 0x4,
	NETDVR_REC_INDEX_HAND = 0x8,
	NETDVR_REC_INDEX_ALL = 0x10,
};

// #ifndef MEDIA_TYPE
// #define MEDIA_TYPE
//ͼ���������
#define  MEDIA_TYPE_H264		(BYTE)98//H.264//������109?
#define  MEDIA_TYPE_MP4			(BYTE)97//MPEG-4
#define  MEDIA_TYPE_H261		(BYTE)31//H.261
#define  MEDIA_TYPE_H263		(BYTE)34//H.263
#define  MEDIA_TYPE_MJPEG		(BYTE)26//Motion JPEG
#define  MEDIA_TYPE_MP2			(BYTE)33//MPEG2 video

//������������
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
//������Ƶ��ʽ
typedef enum NETDVR_FMT_TYPE {
		NETDVR_FMT_RGB24 = 2,					//rgb24 format
		NETDVR_FMT_RGB32 = 4,					//rgb32 format
		NETDVR_FMT_YV12 = 6,					//yv12 format
		NETDVR_FMT_I420 = 8,					//i420 format
		NETDVR_FMT_YUY2 = 10,					//yuy2 format(snapshot is not supported currently)
} fmt_type_t;

/* ========================== structure for each encoded frame ============================= */
//ѹ������֡�ṹ
typedef struct FrameHeadr
{
	unsigned char mediaType;			//encoded (video/audio) media type:
	unsigned char *pData;				//encoded data buffer
	unsigned int preBufSize;			//pre buffer size, normally it is 12+4+12
	// (FIXED HEADER + Extence option + Extence bit)
	unsigned int dataSize;				//actual buffer size pointed by pData
	unsigned char frameRate;			//frame rate, used on receive part. 
	unsigned int frameID;				//fram id��used on receive part. 
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
//��������֡�ṹ
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
//��1�ֽڶ���


/* ========================== structure for the DVR information ============================= */
struct NETDVR_DeviceInfo_t
{
	unsigned int	deviceIP; 						//�豸IP  
	unsigned short	devicePort;						//�豸�˿� 
	char			device_name[32];				//�豸����
	char			device_mode[32];				//�豸�ͺ�
	unsigned char	maxChnNum;						//���ͨ����
	unsigned char	maxAudioNum;					//�����Ƶ��
	unsigned char	maxSubstreamNum;				//�����������
	unsigned char	maxPlaybackNum;					//���ط���
	unsigned char	maxAlarmInNum;					//��󱨾�������
	unsigned char	maxAlarmOutNum;					//��󱨾������
	unsigned char	maxHddNum;						//���Ӳ����
	unsigned char	nNVROrDecoder;	//����NVR�ͽ�����--- Ծ��1NVR; 2������
	unsigned char	reserved[15];					//Ԥ��
};

struct NETDVR_VideoProperty_t 
{
	unsigned char	videoEncType;					//��Ƶ��������
	unsigned short	max_videowidth;					//�����Ƶ��
	unsigned short	max_videoheight;				//�����Ƶ��
	unsigned char	reserved[3];					//Ԥ��
};

struct NETDVR_AudioProperty_t
{
	unsigned char	audioEnctype;					//Ԥ����Ƶ��������
	unsigned char	audioBitPerSample;				//Ԥ����Ƶλ��
	unsigned short	audioSamplePerSec;				//Ԥ����Ƶ������
	unsigned short	audioFrameSize;					//Ԥ����Ƶÿ֡��С
	unsigned short	audioFrameDurTime;				//Ԥ����Ƶÿ֡���
	unsigned char	reserved[4];					//Ԥ��
};

struct NETDVR_VOIPProperty_t 
{
	unsigned char	VOIPBitPerSample;				//�����Խ�λ��
	unsigned short VOIPSamplePerSec;				//�����Խ�������
	unsigned short VOIPFrameSize;					//�����Խ�ÿ֡��С
	unsigned char	reserved[3];					//Ԥ��
};

struct NETDVR_MDProperty_t 
{
	unsigned char	MDCol;							//�ƶ��������
	unsigned char	MDRow;							//�ƶ��������
	unsigned char	reserved[4];					//Ԥ��
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
	char	username[12];				//�û���
	char	reserved1[4];
	u64		nRemoteView[1];				//Զ��Ԥ��Ȩ��
	u64		reserved2[29];
};

/* ========================== ���������� ============================= */
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
	subflag_t			sub_flag;					//���ֶ������������� 0 cif 1 qcif
	subbittype_t		sub_bit_type;				//������λ������
	unsigned short		sub_intraRate;				//�ؼ�֡���
	unsigned char		sub_qi;						//�ؼ�֡��������
	unsigned char		sub_minQ;					//��С��������
	unsigned char		sub_maxQ;					//�����������
	subvideoquality_t	sub_quality;				//������ͼ������
	subframerate_t 		sub_framerate;				//��������֡��
	subbitrate_t 		sub_bitrate;				//��������λ��
	unsigned int		copy2chnmask;				//���Ƶ�����ͨ����ÿһλһ��ͨ��
	unsigned char		reserved[16];				//�����ֶ�
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
	unsigned short		width;						//�ֱ��ʿ�
	unsigned short		height;						//�ֱ��ʸ�
	unsigned char		flashrate;					//�ֱ���ˢ����
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
	unsigned char flag_bestrow;				//24Сʱ����
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
	unsigned char		chn;						//ͨ��
	unsigned short address;							//��������ַ
	enum NETDVR_BAUDRATE baud_ratesel;				//����ͨѶ����
	enum NETDVR_DATABITSEL data_bitsel;				//����λ
	enum NETDVR_STOPBITSEL stop_bitsel;				//ֹͣλ
	enum NETDVR_CHECK_TYPE check_type;				//У������
	enum NETDVR_FLOWCONTROL flow_control;			//����
	enum NETDVR_PROTOCOLTYPE protocol;				//Э��
	unsigned int	copy2Chnmask;						//���Ƶ�����ͨ����ÿһλһ��ͨ��
	unsigned char	comstyle;					//��������(232 or 485)
	unsigned char	enableptz;
	char reserved[30];								//�����ֶ�
#else
	unsigned char		chn;						//ͨ��
	unsigned short address;							//��������ַ
	enum NETDVR_BAUDRATE baud_ratesel;				//����ͨѶ����
	enum NETDVR_DATABITSEL data_bitsel;				//����λ
	enum NETDVR_STOPBITSEL stop_bitsel;				//ֹͣλ
	enum NETDVR_CHECK_TYPE check_type;				//У������
	enum NETDVR_FLOWCONTROL flow_control;			//����
	enum NETDVR_PROTOCOLTYPE protocol;				//Э��
	unsigned int copy2Chnmask;						//���Ƶ�����ͨ����ÿһλһ��ͨ��
	unsigned char      comstyle;					//��������(232 or 485)
	char reserved[31];								//�����ֶ�
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
	unsigned char  chn;							//ͨ��
	enum NETDVR_PTZCONTROL  cmd;							//����
	unsigned char	aux;						//��������
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
	unsigned short endID;					//�����ļ�¼,��ַΪ1.��endID��startID���ʱ,��ʾ֮����һ����¼
	
	struct NETDVR_logInfo_t *pH;
};

//xdc
/* ========================== condition of seach IPC============================= */ 
struct NETDVR_ipcSeachCondition
{
	unsigned int protocol_type;					//0 all,1 onvif, 2 YueTian
	unsigned short max_return;					//��󷵻���	
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
	char address[64];//onvifʹ��
	char ipc_type;
	char reserved2[2];
	//NVR�����ͨ����
	char max_nvr_chn;
	//Ҫ����NVR ����һ·����
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
	unsigned short endID;					//�����ļ�¼,��ַΪ1.��endID��startID���ʱ,��ʾ֮����һ����¼

	struct NETDVR_ipcInfo *pIPC;
};
//yaogang modify 20141030
//PATROL_PARA
typedef struct NETDVR_PatrolPara
{
	unsigned char nIsPatrol; //��ǰ��Ѳ�Ƿ�����(����:0=��;1=��)
	unsigned char nInterval; //��ǰ��Ѳ�л�ʱ��(����:��λ��s)
	unsigned char nPatrolMode; //��ǰ��Ѳʱ���õ�Ԥ��ģʽ(����:�ο�Mode 1 4 9 16)
	unsigned char nPatrolChnNum;//��Ѳͨ����16 32 48 64
	unsigned char nInterval_num;//��Ѳ�л�ʱ��ɹ�ѡ�������
	unsigned char nPatrolMode_num;//��ѲԤ��ģʽ�ɹ�ѡ�������
	unsigned char value[1];//�����Ѳʱ���ģʽ��ֵ��eg: 5 10 20 30 60 1 4 9 16
} NETDVR_PatrolPara;

typedef struct NETDVR_PreviewPara
{
	unsigned char nPreviewMode; //��ǰ���õ�Ԥ��ģʽ(����:�ο�Mode 1 4 9 16 32)	
	unsigned char ModePara;//Ԥ����ʼͨ��0-15
} NETDVR_PreviewPara;

//ͨ������
typedef struct
{
    int x;  //������x
    int y; //������y
} SPoint;

typedef struct { 
	char strChnName[32];  // ��ͨ��ͨ����
	unsigned char nShowChnName;  // ��ͨ���Ƿ���ʾͨ����(�����б�:0=��;1=��)
	SPoint sChnNamePos; // ��ͨ����XY����
	unsigned char nEncShowChnName; // �����ͨ���Ƿ���ʾͨ����(�����б�:0=��;1=��)
	SPoint sEncChnNamePos; // �����ͨ����XY����(��D1Ϊ��׼)
	unsigned char nEncShowTime; // �����ͨ������ʾʱ��(�����б�:0=��;1=��)
	SPoint sEncTimePos; // �����ͨ��ʱ��XY����(��D1Ϊ��׼)
} SBizCfgStrOsd;

typedef struct NETDVR_ChnPara
{
	unsigned char nchn;
	SBizCfgStrOsd para;
} NETDVR_ChnPara;

typedef struct NETDVR_IPCChnStatus
{
	unsigned char max_chn_num;//���ͨ����
	unsigned char chn_status[20];//һ��bit��Ӧһ��ͨ������160ͨ��
} NETDVR_IPCChnStatus;


/* ========================== structure for record file's remote index condition============================= */
struct NETDVR_fileSearchCondition_t
{
	unsigned short chn;								//searched channel number ͨ��1-16 ��λ���
	unsigned short type;						//record type
	unsigned int start_time;				//search from start time
	unsigned int end_time;					//search to end time
	unsigned short startID;					//ignore the front (startID - 1) records
	unsigned short max_return;				//the max return records number(how many records you want to get)
	unsigned short chn17to32;				//ͨ��17-32 ��λ���
	unsigned char reserved[5];
	char bankcardno[21];				//����
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
	unsigned char		channel_no;					//ͨ����
	char	channelname[32];			//ͨ����
	unsigned char		flag_name;					//����λ����ʾ
	unsigned short		chnpos_x;					//����x����
	unsigned short		chnpos_y;					//����y����
	unsigned char		flag_time;					//ʱ��λ����ʾ
	unsigned short		timepos_x;					//ʱ��x����
	unsigned short		timepos_y;					//ʱ��y����
	unsigned char		flag_mask;					//�ڸ�

	struct net_maskAREA_t
	{
		unsigned short	 	x;
		unsigned short		y;
		unsigned short		width;
		unsigned short		height;
	}maskinfo[4];
	//handler
	unsigned char		flag_safechn;				//��ȫͨ�����
	unsigned int		copy2chnmask;				//���Ƶ�����ͨ����ÿһλһ��ͨ��
	unsigned char		reserved[16];				//�����ֶ�
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
// 	unsigned char		channelno;					//ͨ����
// 	flow_type_t			code_type;					//see flow_type_t
// 	bitrate_type_t		bit_type;				//see bitrate_type_t
// 	bitrate_t			bit_max;						//see bitrate_t
// 	unsigned short		intraRate;					//�ؼ�֡���
// 	unsigned char		qi;							//�ؼ�֡��������
// 	unsigned char		minQ;						//��С��������
// 	unsigned char		maxQ;						//�����������
// 	video_quality_t		quality;				//see video_quality_t
// 	framerate_t			frame_rate;					//see framerate_t
// 	prerecord_time_t	pre_record;			//see prerecord_time_t
// 	postrecord_time_t	post_record;			//see postrecord_time_t
// 
// 	unsigned int		copy2chnmask;
//  	unsigned char		reserved[16];				//�����ֶ�
// 
// };

struct NETDVR_recordParam_t
{	
	unsigned char		channelno;					//ͨ����
	flow_type_t			code_type;					//see flow_type_t
	bitrate_type_t		bit_type;				//see bitrate_type_t
	bitrate_t			bit_max;						//see bitrate_t
	unsigned short		intraRate;					//�ؼ�֡���
	unsigned char		qi;							//�ؼ�֡��������
	unsigned char		minQ;						//��С��������
	unsigned char		maxQ;						//�����������
	video_quality_t		quality;				//see video_quality_t
	framerate_t			frame_rate;					//see framerate_t
	prerecord_time_t	pre_record;			//see prerecord_time_t
	postrecord_time_t	post_record;			//see postrecord_time_t
	unsigned int		copy2chnmask;
	unsigned char		supportdeinter;				//�Ƿ�֧��deinter���� 1�� 0�� (����)
	unsigned char		deinterval;					//deinterǿ�� 0-4 ���ã������У�ǿ����ǿ
	unsigned char		supportResolu;				//�Ƿ�֧������¼��ֱ���
	unsigned char		resolutionpos;				//�ֱ���ѡ��ֵ
	unsigned char		reserved1[12];				//�����ֶ�	
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
	unsigned int starttime;					//��ʼʱ��
	unsigned int endtime;					//��ֹʱ��
	unsigned char	flag_sch;					//��ʱ¼��
	unsigned char 	flag_md;					//�ƶ����¼��
	unsigned char  flag_alarm;					//����¼��
	unsigned char	reserved[4];				//�����ֶ�
};

//¼�񲼷�
struct NETDVR_RecordSCH_t
{
	unsigned char		chn;						//ͨ��
	enum NETDVR_WEEKDAY		weekday;					//����
	struct RecTimeField_t recTimeFieldt[4];
	unsigned char		copy2Weekday;				//���Ƶ�������  ��λ 
	unsigned int		copy2Chnmask;				//���Ƶ�����ͨ������λ
	unsigned char		reserved[16];				//�����ֶ�
};

//�ֶ�¼��
struct NETDVR_ManualRecord_t
{
	unsigned int  chnRecState;					//ͨ���ֶ�¼��״̬ ��λ	
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

struct AlarmInPtz							//����ptz
{
	unsigned char		flag_preset;			//Ԥ�õ�
	unsigned char		preset;
	unsigned char		flag_cruise;			//Ѳ����
	unsigned char		cruise;
	unsigned char		flag_track;				//�켣
};

struct NETDVR_alarmInParam_t
{
	unsigned char		inid;						//����������
	unsigned char		flag_deal;					//1: deal with input alarm ; 0: for no
	alarmintype_t		typein;						//input alarm type:see alarmintype_t
	unsigned int		triRecChn;					//����¼��ͨ����ÿһλһͨ��
	unsigned int		triAlarmoutid;				//���������������λ
	unsigned char		flag_buzz;					//����������
	unsigned char		flag_email;					//����emaill
	unsigned char		flag_mobile;				//�����ֻ�����
	delay_t		delay;								//���������ʱ
	unsigned int		copy2AlarmInmask;
	struct AlarmInPtz	alarmptz[32];
	unsigned char		flag_enablealarm;			//�������ñ�־
	unsigned char		reserved[15];				//�����ֶ�
};

struct NETDVR_alarmOutParam_t
{
	unsigned char		outid;						//��������� 
	alarmouttype_t		typeout;					//�����������
	unsigned int		copy2AlarmOutmask;			//���Ƶ����������������λ
	//csp modify
	//unsigned char		reserved[16];				//�����ֶ�
	delay_t				alarmoutdelay;				//���������ʱ
	unsigned char		flag_buzz;					//����������
	delay_t				buzzdelay;					//��������ʱ
	unsigned char		reserved[7];				//�����ֶ�
};

struct NETDVR_AlarmNoticeParam_t									
{
	char			alarm_email[32];			//����email��ַ
	char			alarm_mobile[32];			//�����ֻ���ַ
	unsigned char	reserved[32];				//�����ֶ�
};

struct NETDVR_networkParam_t
{
	char				mac_address[18];			//mac��ַ
	unsigned int		ip_address;					//ip��ַ
	unsigned short		server_port;				//�豸�˿�
	unsigned int		net_mask;					//����
	unsigned int		net_gateway;				//����
	unsigned int		dns;						//dns
	unsigned char		flag_multicast;				//�鲥���ñ��
	unsigned int		multicast_address;			//�鲥��ַ
	unsigned short		http_port;					//http�˿�
	unsigned char		flag_pppoe;					//pppoe���ñ��
	char				pppoe_user_name[64];		//pppoe�û���
	char				pppoe_passwd[64];			//pppoe����
	unsigned char		flag_dhcp;					//dhcp���ñ�־
	unsigned char		ddnsserver;					//ddns������
	unsigned char		flag_ddns;					//ddns���ñ�־
	char				ddns_domain[64];			//ddns����
	char				ddns_user[64];				//ddns�û���
	char				ddns_passwd[64];			//ddns����
	unsigned int		centerhost_address;			//���ķ�������ַ
	unsigned short		centerhost_port;			//���ķ������˿�
	unsigned short		mobile_port;				//�ֻ���ض˿�
	char				hwid[12];					//�����Ӷ���
	unsigned char		reserved[2];				//�����ֶ�
};


struct NETDVR_progressParam_t
{
	unsigned int curr_pos;
	unsigned int total_size;
};

struct NETDVR_TimeDLprogressParam_t
{
	unsigned int currfile_recv; //��ǰ�ļ��ѽ��մ�С
	unsigned int currfile_size; //��ǰ�ļ���С
	int currindex; //��ǰ�ڼ����ļ� 1��ʼ����
	int totalnum; //�ܹ��ж����ļ�
	unsigned int totalfilesize;	//���ļ���С MBΪ��λ����1024��
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
	char recvefilename[64]; //�Է����������ļ���
	unsigned int currindex; //��ǰ�ڼ����ļ� 1��ʼ����
	unsigned int totalnum; //�ܹ��ж����ļ�
};

struct NETDVR_AlarmUploadState_t
{
	//0-�ź�������,1-Ӳ����,2-�źŶ�ʧ,3���ƶ����,4��Ӳ��δ��ʽ��,
	//5-��дӲ�̳���,6-�ڵ�����,7-��ʽ��ƥ��, 8-�Ƿ�����
	unsigned char	type;		
	unsigned char	state;			//1���� 2�ָ�
	unsigned char	id;				//ͨ��,Ӳ�̺�,���������,ȡ����type 
	unsigned short	reserved1;		//Ԥ��
	unsigned int	reserved2;		//Ԥ��	
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
	unsigned int 	trigRecChn;						//����¼��ͨ�� ��λ
	unsigned int 	trigAlarmOut;					//����������� ��λ
	unsigned char	flag_buzz;						//
	unsigned char	flag_email;						//����emaill
	unsigned char	flag_mobile;					//�����ֻ�����
	unsigned char	sense;
	delay_t			delay;							//��ʱ
	unsigned char	block[44*36];					//
	unsigned int 	copy2Chnmask;					//���Ƶ�����ͨ������λ
	unsigned char	reserved[12];					//Ԥ��
};

//��Ƶ��ʧ
struct NETDVR_VideoLostParam_t
{
	unsigned char		chn;
	unsigned int 		trigRecChn;					//����¼��ͨ�� ��λ
	unsigned int 		trigAlarmOut;				//����������� ��λ
	unsigned char 		flag_buzz;					//������
	unsigned char		flag_email;					//����emaill
	unsigned char		flag_mobile;				//�����ֻ�����
	delay_t 			delay;						//��ʱ
	unsigned int 		copy2Chnmask;				//���Ƶ�����ͨ������λ
	unsigned char		reserved[12];				//�����ֶ�
};

//for 2.0
struct NETDVR_motionDetectionfor2_0_t 
{
	unsigned char	chn;
	unsigned int 	trigRecChn;						//����¼��ͨ�� ��λ
	unsigned int 	trigAlarmOut;					//����������� ��λ
	unsigned char	flag_buzz;						//
	unsigned char	flag_email;						//����emaill
	unsigned char	flag_mobile;					//�����ֻ�����
	unsigned char	sense;
	delay_t			delay;							//��ʱ
	unsigned char	block[44*36];					//
	struct AlarmInPtz	alarmptz[32];				//ptz
	unsigned int 	copy2Chnmask;					//���Ƶ�����ͨ������λ
	unsigned char	flag_enablealarm;				//�������ñ�־
	unsigned char	reserved[11];					//Ԥ��
};

//��Ƶ��ʧ
struct NETDVR_VideoLostParamfor2_0_t
{
	unsigned char		chn;
	unsigned int 		trigRecChn;					//����¼��ͨ�� ��λ
	unsigned int 		trigAlarmOut;				//����������� ��λ
	unsigned char 		flag_buzz;					//������
	unsigned char		flag_email;					//����emaill
	unsigned char		flag_mobile;				//�����ֻ�����
	delay_t 			delay;						//��ʱ
	struct AlarmInPtz	alarmptz[32];				//ptz
	unsigned int 		copy2Chnmask;				//���Ƶ�����ͨ������λ
	unsigned char		reserved[12];				//�����ֶ�
};

//��Ƶ�ڵ�
struct NETDVR_VideoBlockParam_t
{
	unsigned char		chn;
	unsigned int 		trigRecChn;					//����¼��ͨ�� ��λ
	unsigned int 		trigAlarmOut;				//����������� ��λ
	unsigned char 		flag_buzz;					//������
	unsigned char		flag_email;					//����emaill
	unsigned char		flag_mobile;				//�����ֻ�����
	delay_t				delay;						//��ʱ
	unsigned int 		copy2Chnmask;				//���Ƶ�����ͨ������λ
	unsigned char		reserved[12];				//�����ֶ�
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
	unsigned char	flag_email;					//����emaill
	unsigned char	flag_mobile;				//�����ֻ�����
	unsigned short 	delay;						//��ʱ
	struct NETDVR_alarmInPtz_t alarminptz;
};

/* ==================== user control structure ======================== */
struct NETDVR_userInfo_t 
{
	char	name[12];
	char	password[12];

	char	mac_address[18];
	
	/* 1:open�� 0:close */
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
	unsigned char		hdd_index;					//Ӳ�����
	unsigned char		hdd_exist;  				//1 exist; 0 not exist
	unsigned int		capability;					//MB
	unsigned int		freesize;					//MB
	unsigned char		reserved[2];				//Ԥ��

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
	unsigned char		channel_no;					//ͨ����
	enum NETDVR_PICADJUST		flag;						//���ڱ�־:0-3
	unsigned char		val;						//����ֵ
	unsigned int		copy2chnmask;				//���Ƶ�����ͨ����ÿһλһ��ͨ��
};

typedef enum NETDVR_TimeFormat_T
{
	NETDVR_TF_YYYYMMDD = 0,
	NETDVR_TF_MMDDYYYY = 1,
}timeFormat_t;

//ϵͳʱ��
struct NETDVR_SysTime_t
{
	unsigned int		systemtime;					//ϵͳʱ��
	timeFormat_t		format;						//ʱ���ʽ ѡ��ֵ
	unsigned char		flag_time;					//Ԥ��ʱ��λ����ʾ
	unsigned short		timepos_x;					//Ԥ��ʱ��x����
	unsigned short		timepos_y;					//Ԥ��ʱ��y����
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
	unsigned char reserved[23];	//����
};

//ϵͳ�����б�
enum NETDVR_SYS_LANGUAGE{
	NETDVR_SYS_LANGUAGE_CHS = 0,		//��������
	NETDVR_SYS_LANGUAGE_ENU = 1,		//��ʽӢ��
	NETDVR_SYS_LANGUAGE_CHT = 2,       //��������
	NETDVR_SYS_LANGUAGE_GER = 3,		//����
	NETDVR_SYS_LANGUAGE_FRE = 4,		//����
	NETDVR_SYS_LANGUAGE_SPA = 5,		//��������
	NETDVR_SYS_LANGUAGE_ITA = 6,		//�����
	NETDVR_SYS_LANGUAGE_POR = 7,		//��������
	NETDVR_SYS_LANGUAGE_RUS = 8,		//����
	NETDVR_SYS_LANGUAGE_TUR = 9,		//��������
	NETDVR_SYS_LANGUAGE_THA = 10,		//̩����
	NETDVR_SYS_LANGUAGE_JAP = 11,		//����
	NETDVR_SYS_LANGUAGE_HAN = 12,		//����
	NETDVR_SYS_LANGUAGE_POL = 13,		//������
	NETDVR_SYS_LANGUAGE_HEB = 14,		//ϣ������Hebrew
	NETDVR_SYS_LANGUAGE_HUN = 15,		//��������Hungarian
	NETDVR_SYS_LANGUAGE_ROM = 16,		//������Roma
	NETDVR_SYS_LANGUAGE_IRA = 17,		//������
	NETDVR_SYS_LANGUAGE_CZE = 18,		//�ݿ���
	NETDVR_SYS_LANGUAGE_VIE = 19,		//Խ����
	NETDVR_SYS_LANGUAGE_LIT = 20,		//������
	NETDVR_SYS_LANGUAGE_SLO = 21,		//˹�工��
	NETDVR_SYS_LANGUAGE_ARA = 22,		//��������
	NETDVR_SYS_LANGUAGE_GRE = 23,		//ϣ����
	NETDVR_SYS_LANGUAGE_RMN = 24,		//����������
	NETDVR_SYS_LANGUAGE_FAR = 25,		//��˹��
	NETDVR_SYS_LANGUAGE_BUL = 26,		//��������
	NETDVR_SYS_LANGUAGE_INVALID = -1	//���������Чֵ��
};

struct NETDVR_SysLangList_t
{
	unsigned char max_langnum;		//���֧�������� <=32
	NETDVR_SYS_LANGUAGE langlist[32];	// language list, ���32��,
	unsigned char reserved[16];	//Ԥ��
};

//λ���б�
struct  NETDVR_bitRateList_t
{
	unsigned char	chn;				//ͨ��
	unsigned char	videotype;			//0 ������, 1 ������
	unsigned int	bitratelist[16];	//λ���б�, ��λkb, δʹ�õ���0
	unsigned char	reserved[16];		//Ԥ��
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
	unsigned short		SMTPport;			//SMTP�˿�
	unsigned char		flag_ssl;			//�Ƿ�ʹ��SSL
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
	unsigned short		SMTPport;			//SMTP�˿�
	unsigned char		flag_ssl;			//�Ƿ�ʹ��SSL
	unsigned char reserved[13]; 
};

//�����Ӱ���qq
struct NETDVR_LASServer_t
{
	unsigned char sn[64];
	unsigned char productcode[20];
	unsigned char macaddr[18];
	unsigned char reserved[16];
};

//Ԥ�õ��б�
struct NETDVR_PresetList_t
{
	unsigned char chn;	//ͨ��
	unsigned char totalAddedpreset; //����ӵ�Ԥ�õ����� <=128
	struct presetstate_t
	{
		unsigned char preset;	//Ԥ�õ��  1-128
		unsigned char bAdded;	//�Ƿ������
		char name[12];	//Ԥ�õ�����
	}presetstate[128]; //Ԥ�õ�״̬���±�0-127�ֱ����1-128��Ԥ�õ�
};

//Ԥ�õ�����
struct NETDVR_PresetName_t
{
	unsigned char chn;		//ͨ��
	unsigned char preset;	//Ԥ�õ��  1-128
	char name[12];	//Ԥ�õ�����
};

//��̨�ٶ�
#define PTZ_RATE_FAST	16
#define PTZ_RATE_NORMAL	17
#define PTZ_RATE_SLOW	18
struct NETDVR_PTZRate_t
{
	unsigned char chn;  //ͨ��
	unsigned char rate; //�ٶ� 16 ���� 17���� 18����
};


//�ָ�Ĭ�ϻ������
struct NETDVR_Reset_Picadjust_t
{
	unsigned char chn;  //ͨ�� 0��ʼ
	unsigned char adjtype; //��λ��ʾ���ͣ���1-4λ�ֱ��ʾ���ȣ��Աȶȣ�ɫ�������Ͷȣ���Ϊ1�ı�ʾ�ָ�Ĭ��ֵ�� 0xf��ʾȫ�ָ�
};

//֡���б� 
struct NETDVR_Framerate_list_t
{
	unsigned char chn;	//ͨ��
	unsigned char type;	// 0 ������ 1 ������
	unsigned char framerate[10];	//֡���б� δʹ�õ���0
};

// struct NETDVR_FramerateListByResolution_t
// {
// 	unsigned char chn;	//ͨ��
// 	unsigned char frametype;	// 0 ������ 1 ������
// 	unsigned char resolutiontype;	// ��CTRL_GET_RESOLUTION_LIST
// 	unsigned char framerate[10];	//֡���б� δʹ�õ���0
// };

//��/�������ֱ����б� CTRL_GET_RESOLUTION_LIST
#define NETDVR_VIDEORESOLU_BEGIN	1 //��1��ʼ
#define NETDVR_VIDEORESOLU_QCIF		1 //QCIF
#define NETDVR_VIDEORESOLU_CIF		2 //CIF
#define NETDVR_VIDEORESOLU_HD1		3 //HD1
#define NETDVR_VIDEORESOLU_D1		4 //D1
#define NETDVR_VIDEORESOLU_720P		5
#define NETDVR_VIDEORESOLU_1080P	6
#define NETDVR_VIDEORESOLU_960H		7
//#define NETDVR_VIDEORESOLU_END	NETDVR_VIDEORESOLU_D1	//��NETDVR_VIDEORESOLU_D1�������������ͨ���ж��Ƿ���BEGIN��END֮����ȷ���Ƿ���ȷ
#define NETDVR_VIDEORESOLU_END		NETDVR_VIDEORESOLU_960H

struct NETDVR_VideoResolu_list_t
{
	unsigned char chn;	//ͨ��
	unsigned char type;	// 0 ������ 1 ������
	unsigned char videoresolu[8];	//�ֱ����б� δʹ�õ���0 VIDEORESOLU_BEGIN ~ VIDEORESOLU_END
};
//���������
struct NETDVR_NRServer_t
{
	unsigned int nrserverip; //��������ַ
	unsigned short serverport; //�������˿�
	unsigned short databindport; //���ݰ󶨶˿�
	unsigned char reserved[16]; //Ԥ��
};

//��ȡ��������ֵ&���ñ������ֵ
struct NETDVR_AlarmVal_t
{
	unsigned char alarmid;		//alarmid 0��ʼ
	unsigned char  val;			//ȡֵ 0δ���� 1����
	unsigned char reserved[2];	//Ԥ��
};


struct	NETDVR_specialinfo_t
{
	unsigned char mainbordmode; //�����ͺ� Ԥ��
	unsigned char characterset; //�ַ���: 0-ascii��1-GB2312, 2-UTF8, 3-UTF16 
	unsigned char reserved[62]; //Ԥ��
};


struct NETDVR_MOTIONCOND
{
	unsigned int	channelmask;	//ͨ����  ��λ
	unsigned int	dwstartTime;         //¼��ʼʱ�� ����1970.1.1��UTCʱ��
	unsigned int    dwstopTime;         //¼�����ʱ�� ����1970.1.1��UTCʱ��
	unsigned char   byRes[60];        //����
}; 

#define MAX_SECONDS 60

struct NETDVR_MOTION_SECOND
{
	unsigned char	 byChn;				//�����˴��ƶ�����ͨ�� 0��ʼ
	unsigned int     dwMotionTime;      //¼��ʱ�� ����1970.1.1��UTCʱ��
	unsigned int     dwMotionNum;       // dwMotionTime��һ�����ƶ������Ŀ
	unsigned char    byRes[12];          //����
};

struct NETDVR_MOTION_DATA
{
	unsigned int  dwSecondNum;    //�ýṹ�а������������Ϣ
	struct NETDVR_MOTION_SECOND  tMotionData[MAX_SECONDS];
};

struct NETDVR_Makekeyframe_t
{
	unsigned char chn;			//ͨ�� 0��ʼ
	unsigned char type;		// 0 ������ 1������
	unsigned char reserved[6];	//Ԥ��
};  // 8 Byte

struct  NETDVR_CrearSvr_Info_t
{
	unsigned char flagPlatform; //�Ƿ�����ƽ̨ 0������ 1����
	unsigned int ipPlatform; //ƽ̨��ַ
	unsigned short portPlatform; //ƽ̨�˿�
	char PUID[19];//PUID
	char passwd[32]; //��������
	unsigned char  reserved[32]; //Ԥ��
};

struct NETDVR_DDNSinfo_t
{
	char ddns_user[64];				//ddns�û���
	char ddns_passwd[64];			//ddns����
	char ddns_domain[64];			//ddns����
	unsigned char  reserved[16];	//Ԥ��
};

//�������벼��
struct AlarmTimeField_t
{
	unsigned int starttime; //��ʼʱ��
    unsigned int endtime; //��ֹʱ��
    unsigned char  flag_alarm; //��������
    unsigned char reserved[6]; //�����ֶ�
};

struct NETDVR_AlarmSCH_t
{
	unsigned char chn; //ͨ��
	enum NETDVR_WEEKDAY weekday; //����
	struct AlarmTimeField_t alarmTimeFieldt[4];
	unsigned char copy2Weekday; //���Ƶ�������  ��λ 
	unsigned int copy2Chnmask; //���Ƶ�����ͨ������λ
	unsigned char reserved[16]; //�����ֶ�
};

struct  NETDVR_MDSenselist_t
{
	unsigned char	mdsenselist[12];	//�ƶ�����������б�
};

struct  NETDVR_MDAlarmDelaylist_t
{
	unsigned short	mdalarmdelaylist[16];	//�ƶ���ⱨ�������ʱ�б�
};

struct  NETDVR_BaudRateList_t
{
	//csp modify
	//unsigned short baudratelist[16];	//�������б�
	unsigned int baudratelist[16];		//�������б�
};

struct  NETDVR_PTZProtocolList_t
{
	unsigned char   maxNum;
	unsigned char	baudratelist[19];	//��̨Э���б�
};

struct RecSchTimeField_t
{
	unsigned int starttime; //��ʼʱ��
	unsigned int endtime; //��ֹʱ��
	unsigned char reserved[6]; //�����ֶ�
};

struct  NETDVR_RecSchTime_t
{
	unsigned char chn; //ͨ��
	unsigned char weekday; //����
	unsigned char type;
	RecSchTimeField_t TimeFiled[24];
    unsigned char copy2Weekday; //���Ƶ�������  ��λ 
    unsigned int copy2Chnmask; //���Ƶ�����ͨ������λ
	unsigned char reserved[16]; //�����ֶ�
};

struct  NETDVR_HuiNaInfo_t
{
	unsigned int admin_host;             //����������3g���ݷ�����ip��ַ
	unsigned short ctrl_port;            //���ɵ�½�˿�
	unsigned short data_port;            //�������ݶ˿�
	unsigned char   server_enable_flag;  //��������ʹ��
	unsigned short   heartbeat_time;	 //��������
	char   huina_http_ulr[64];			 //����http��ַ
	char   device_flag[16];				 //�豸���
	char   shop_num[16];				 //���ɵ��̱��
	unsigned char reserved[5];
};

//csp modify
struct NETDVR_reboottime_t
{
	unsigned int  reboot_time; // �豸����ʱ�䣬�ѷ���Ϊ��λ
	unsigned char reserve[4];
};

//csp modify
struct NETDVR_alarmtimeinterval_t
{
	unsigned short alarm_timeinterval; //sΪ��λ�������ϴ�ʱ����
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
// ��ʼ��
int __stdcall NETDVR_startup(void);
int __stdcall NETDVR_cleanup(void);

//NETDVR_createDVR to create a new DVR, NETDVR_destroyDVR to destroy an exist DVR
//////////////////////////////////////////////////////////////////////////
//���� &����
int __stdcall NETDVR_createDVR(int *p_handle, unsigned int serverip, unsigned short serverport);
int __stdcall NETDVR_createDVRbyDomain(int *p_handle, char *pDomain, unsigned short serverport);
int __stdcall NETDVR_destroyDVR(int Handle);


//NETDVR_regCBMsgConnLost:register a callback function to deal with connectin lost notification message for dvr
//ע�����֪ͨ
int __stdcall NETDVR_regCBMsgConnLost(int Handle, PFUN_MSG_T p_cb_func, unsigned int dwContent);


//////////////////////////////////////////////////////////////////////////
// ��¼ &ע��
/*
NETDVR_loginServer let DVR login to a server
NETDVR_logoutServer to logout from a server
*/
int __stdcall NETDVR_loginServer(int Handle, const struct NETDVR_loginInfo_t *p_para);
int __stdcall NETDVR_logoutServer(int Handle);

//csp modify 20130519
//��ȡ�û��߼�Ȩ��
int __stdcall NETDVR_GetAdvPrivilege(int Handle, const struct NETDVR_loginInfo_t *pLoginInfo, struct NETDVR_AdvPrivilege_t *pAdvPrivilege);

//////////////////////////////////////////////////////////////////////////
// ʵʱԤ��
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

// ��ʼ&ֹͣԤ��
int __stdcall NETDVR_StartRealPlay(int Handle, const struct RealPlayClientInfo_t* pClientinfo, int* pRealPlayHandle);
int __stdcall NETDVR_StopRealPlay(int nRealPlayHandle);

//csp modify 20130423
int __stdcall NETDVR_SetMonitorInfo(int nRealPlayHandle, unsigned char chn, unsigned char wnd_num, unsigned char wnd_index);

//�����Ƿ����
int __stdcall NETDVR_SetVideoDecFlag(int nRealPlayHandle, unsigned char bDec);

//������Ƶ
int __stdcall NETDVR_OpenRealAudio(int nRealPlayHandle);
int __stdcall NETDVR_CloseRealAudio(int nRealPlayHandle);
int __stdcall NETDVR_MuteRealAudio(int nRealPlayHandle, bool bMute);

//ץͼ
int __stdcall NETDVR_snapshot(int nRealPlayHandle, char *path, char *filename);

//¼��
int __stdcall NETDVR_startRecord(int nRealPlayHandle, char *p_dir_path, unsigned int file_max_len);
int __stdcall NETDVR_stopRecord(int nRealPlayHandle);
int __stdcall NETDVR_setRecordCB(int nRealPlayHandle, pFrameCallBack pRecordCBFun, unsigned int dwContent);
int __stdcall NETDVR_setRecordFileNameCB(int nRealPlayHandle, pRecFilenameCallBack pRecFilenameCBFunc, unsigned int dwContent);

//////////////////////////////////////////////////////////////////////////
// Զ��¼������
/*
NETDVR_recFilesSearch: index record file(s)
NETDVR_recFilesSearchClean: clean the index result
*/
int __stdcall NETDVR_recFilesSearch(int Handle, const struct NETDVR_fileSearchCondition_t *prfs, struct NETDVR_recFileSearchResult_t *pdesc);
int __stdcall NETDVR_recFilesSearchClean(struct NETDVR_recFileSearchResult_t *pdesc);

//////////////////////////////////////////////////////////////////////////
// ����
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
// �ط�
#pragma pack( push, 4 )
struct PlayBackClientInfo_t
{
	unsigned char sendmode;	//0:����֡ʵ��ʱ��������  1:��ʱ����ֱ�ӷ���
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

//��ʼ &ֹͣ�ط�
int __stdcall NETDVR_startPlayByFile(int Handle,  const struct NETDVR_recFileInfo_t *pFileInfo, const struct PlayBackClientInfo_t* pClientInfo, int* pPlayHandle);
int __stdcall NETDVR_startPlayByTime(int Handle,  const struct NETDVR_TimePlayCond_t *pTimePlayInfo, const struct PlayBackClientInfo_t* pClientInfo, int* pPlayHandle);
int __stdcall NETDVR_stopPlayBack(int nPlayBackHandle);

//�طſ���
//����(��ͣ����)
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
//��������

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

//yaogang modify 20170715 ��������ͨ�����Ľӿ�
int __stdcall NETDVR_SetChnName(int Handle, unsigned char chn, const char *pname, int len);
int __stdcall NETDVR_GetChnName(int Handle, unsigned char chn, char *pname, int size);//size >= 128


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

//�����豸���
typedef void (CALLBACK*  PFUN_SearchDevice_CB)(struct NETDVR_DeviceInfo_t dev, void* pContext);
int __stdcall NETDVR_RegCallBackSearchDevice(PFUN_SearchDevice_CB p_cb_func, void* pContext);
int __stdcall NETDVR_SearchDevice();

int __stdcall NETDVR_PatrolSync(); //�㲥�豸��ͬ����Ѳ


int __stdcall NETDVR_SetYTTrack(int Handle, unsigned char chn, enum NETDVR_YTTRACKCONTROL yt_cmd);
int __stdcall NETDVR_SetYTPresetPoint(int Handle, unsigned char chn, unsigned char preset_pos, enum NETDVR_YTPRESETPOINTCONTROL yt_com);


int __stdcall NETDVR_regCBAlarmState(int Handle, PFUN_ALARMSTATE_T p_cb_func, unsigned int dwContent);
int __stdcall NETDVR_SetAlarmUpload(int Handle, const unsigned char uploadflag);

int __stdcall NETDVR_DeinterlaceVideoFrame(pFrameHeadrDec pFrmHdrDec);
int __stdcall NETDVR_CtrlCDROM(int Handle,unsigned char ctrlflag);

//serialport 1��232���ڣ�2��485���� 
int __stdcall NETDVR_GetComParam(int Handle, unsigned char serialport, struct NETDVR_ComParam_t *p_para);
int __stdcall NETDVR_SetComParam(int Handle, const struct NETDVR_ComParam_t *p_para);


//͸������
typedef void(CALLBACK*  pSerialDataCallBack)(int lSerialPort, unsigned char bySerialChannel, char *pRecvDataBuffer, unsigned int dwBufSize, unsigned int dwContent);
//lSerialPort 1��232���ڣ�2��485���� 
int __stdcall NETDVR_SerialStart(int Handle, int lSerialPort,	pSerialDataCallBack cbSerialDataCallBack, unsigned int dwContent);
//byChannel, ʹ��485����ʱ��Ч����1��ʼ��232������Ϊ͸��ͨ��ʱ��ֵ����Ϊ0 
//dwBufSize ���ֵ4096
int __stdcall NETDVR_SerialSend(int Handle, int lSerialPort, unsigned char byChannel, char* pSendBuf, unsigned int dwBufSize);
int __stdcall NETDVR_SerialStop(int Handle, int lSerialPort);

//����
enum RECONNMSG{RECONN_BEGIN, RECONN_SUCESS, RECONN_FAILED};
typedef void (CALLBACK*  pCBReconnMsg)(enum RECONNMSG msg, unsigned int dwContent);
//����ȫ������ʱ�䣬��λ���롣Ĭ��10s
int __stdcall NETDVR_setReconnectTime(unsigned int millisecond);
//ע��������Ϣ�ص�
int __stdcall NETDVR_regCBMsgReconnect(int Handle, pCBReconnMsg p_cb_func, unsigned int dwContent);
//����������� 1 �������� 0 �ر����� ��Ĭ�ϲ�����
int __stdcall NETDVR_setReconnectFlag(int Handle, unsigned char reconflag);


//���ϵͳ֧�������б�
int __stdcall NETDVR_getSysLangList(int Handle, NETDVR_SysLangList_t* p_para);

//���ͨ����Ƶλ���б�
int __stdcall NETDVR_getBitRateList(int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_bitRateList_t* p_para);

//xwserver
int __stdcall NETDVR_getxwServerParams(int Handle, struct NETDVR_xwServer_t *pxwServ);
int __stdcall NETDVR_setxwServerParams(int Handle, const struct NETDVR_xwServer_t *pxwServ);

//SMTP
int __stdcall NETDVR_getSMTPServerParams(int Handle, struct NETDVR_SMTPServer_t *pSMTPServ);
int __stdcall NETDVR_setSMTPServerParams(int Handle, const struct NETDVR_SMTPServer_t *pSMTPServ);

//�����Ӱ���qq
int __stdcall NETDVR_getLASAFQQInfo(int Handle, struct NETDVR_LASServer_t *pLASServ);
int __stdcall NETDVR_setLASAFQQInfo(int Handle, const struct NETDVR_LASServer_t *pLASServ);

//Զ�̸�ʽ��
typedef void (CALLBACK*  pFormatProgressCallBack)(unsigned char hddindex, unsigned char pos, unsigned short errcode, unsigned int dwContent);
int __stdcall NETDVR_formatHdd(int Handle, unsigned char hddindex, pFormatProgressCallBack pCBFunc, unsigned int dwContent);

int __stdcall NETDVR_remoteSnap(int Handle, unsigned char chn, char* filename);

int __stdcall NETDVR_IsUseUTCTime(int Handle, unsigned char *pFlag);

//���Ԥ�õ��б�
int __stdcall NETDVR_getPresetList(int Handle, unsigned char chn, struct NETDVR_PresetList_t* pList);

//���Ԥ�õ�(������)
int __stdcall NETDVR_AddPresetByName(int Handle, const struct NETDVR_PresetName_t *p_para);

//���PTZ�ٶ�
int __stdcall NETDVR_getPTZRate(int Handle, unsigned char chn, struct NETDVR_PTZRate_t* p_para);

//���û�����ڲ���
int __stdcall NETDVR_resetPicAdjust(int Handle, const struct NETDVR_Reset_Picadjust_t *pPara);

//���֡���б�
int __stdcall NETDVR_getFrameRateList(int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_Framerate_list_t* p_para);

//��÷ֱ����б�
int __stdcall NETDVR_getVideoResoluList(int Handle, unsigned char chn, unsigned char vidoetype, NETDVR_VideoResolu_list_t* p_para);

//���ϵͳ֧������ڸ���
int __stdcall NETDVR_getMaxIMGMaskNum(int Handle,  unsigned char* pNum);

//���������
int __stdcall NETDVR_setNRServer(int Handle, const struct NETDVR_NRServer_t *p_para);
int __stdcall NETDVR_getNRServer( int Handle, NETDVR_NRServer_t * p_para);

//����
int __stdcall NETDVR_setAlarmOutVal(int Handle, const struct NETDVR_AlarmVal_t *p_para);//���ñ������ֵ
int __stdcall NETDVR_getAlarmInVal( int Handle, unsigned char alarm_in_id,struct NETDVR_AlarmVal_t * p_para);//��ȡ��������ֵ

//��ʱ������
//�豸����� ͨ�������ͣ���ʼʱ�䣬 ע��ص����ϣ����ؾ��
#pragma pack( push, 4 )
struct TimeDownloadInfo_t
{
	unsigned short rcv_chn; //channel  ͨ��1-16
	unsigned short rcv_chn17to32; //channel ͨ��17-32
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

//��ȡ�����ϴ�״̬
int __stdcall NETDVR_getAlarmUploadState( int Handle, unsigned char id, unsigned char type, NETDVR_AlarmUploadState_t* pPara);

//�����豸��Ϣ
int __stdcall NETDVR_getSpecialinfo_t(int Handle, NETDVR_specialinfo_t* pPara);

//�����ƶ������Ϣ

//��ʼ����
int __stdcall NETDVR_FindMotion_Info(int Handle, const NETDVR_MOTIONCOND* pMDCond, int* lpFindHandle);
//������һ��
int __stdcall NETDVR_GetNextMotionInfo(int lFindHandle, NETDVR_MOTION_DATA* lpMotiondData);
//��������
int __stdcall NETDVR_FindMotionClose(int lFindHandle);

//����ؼ�֡
int __stdcall NETDVR_MakeKeyFrame(int Handle, NETDVR_Makekeyframe_t* pPara);

//��ô���ƽ̨��Ϣ
int __stdcall NETDVR_getCrearSvrInfo(int Handle, struct NETDVR_CrearSvr_Info_t *p_para);
//���ô���ƽ̨��Ϣ
int __stdcall NETDVR_setCrearSvrInfo(int Handle, const struct NETDVR_CrearSvr_Info_t *p_para);

int __stdcall NETDVR_startRecord2(int nRealPlayHandle, char *p_dir_path, unsigned int file_max_len);
int __stdcall NETDVR_stopRecord2(int nRealPlayHandle);
int __stdcall NETDVR_setRecord2CB(int nRealPlayHandle, pFrameCallBack pRecordCBFun, unsigned int dwContent);
int __stdcall NETDVR_setRecord2FileNameCB(int nRealPlayHandle, pRecFilenameCallBack pRecFilenameCBFunc, unsigned int dwContent);
int __stdcall NETDVR_GetRecord2State(int nRealPlayHandle, unsigned char* pState);

//ע���ע��DDNS
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
//pIPC->channel_no ȡֵ��[1-16]������ӵ�ָ��ͨ��
//= 0������ӵ���Сδʹ�õ�ͨ��
int __stdcall NETDVR_addIPC(int Handle,struct NETDVR_ipcInfo *pIPC);
int __stdcall NETDVR_deleteIPC(int Handle,struct NETDVR_ipcInfo *pIPC);
//yaogang modify 20141025 
//NETDVR_seachIPCList ,  NETDVR_getAddIPCList �ᴴ�����Ե�����
//�ڴ��ͷ�����
int __stdcall NETDVR_destoryIPCList(struct NETDVR_ipcSearch *p_para);

//patrol para
int __stdcall NETDVR_GetPatrolPara(int Handle, struct NETDVR_PatrolPara *p_para, int *psize);
int __stdcall NETDVR_SetPatrolPara(int Handle, struct NETDVR_PatrolPara *p_para);

//preview para
int __stdcall NETDVR_GetPreviewPara(int Handle, struct NETDVR_PreviewPara *p_para);
int __stdcall NETDVR_SetPreviewPara(int Handle, struct NETDVR_PreviewPara *p_para);

//ͨ������
int __stdcall NETDVR_GetChnPara(int Handle, struct NETDVR_ChnPara * p_para);
int __stdcall NETDVR_SetChnPara(int Handle, struct NETDVR_ChnPara *p_para);

//������汨�����Ǳ꣨ע��ֻ���ڵ�ǰû�б���Դ����ʱ���Ż������
int __stdcall NETDVR_CleanDesktopAlarmIcon(int Handle); 

//Ծ������CMS remote CTRL
int __stdcall NETDVR_CLOSE_GUIDE(int Handle);

//yaogang modify 20160122
//��ȡIPCͨ������״̬(һ��IPC������ͨ��������������)
int __stdcall NETDVR_GetIPCChnLinkStatus(int Handle, struct NETDVR_IPCChnStatus * p_para);




#ifdef __cplusplus
}
#endif
         
#endif
