#ifndef _CTRLPROTOCOL_H_
#define _CTRLPROTOCOL_H_

#include "iflytype.h"
#include "camera.h"
#include <time.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#endif

#ifdef WIN32
#define SOCKHANDLE		SOCKET
#undef  FD_SETSIZE
#define FD_SETSIZE	257
#else
#define SOCKHANDLE		int
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
#define INFINITE	0xffffffff
#endif

//命令类型
#define CTRL_CMD_BASE					10000
#define CTRL_CMD_LOGIN					CTRL_CMD_BASE+1			//远程登录命令
#define CTRL_CMD_LOGOFF					CTRL_CMD_BASE+2			//注销用户登录
#define CTRL_CMD_GETDEVICEINFO			CTRL_CMD_BASE+3			//获得设备信息
#define CTRL_CMD_GETVIDEOPROPERTY		CTRL_CMD_BASE+4			//视频属性信息
#define CTRL_CMD_GETAUDIOPROPERTY		CTRL_CMD_BASE+5			//音频属性信息
#define CTRL_CMD_GETVOIPPROPERTY		CTRL_CMD_BASE+6			//voip属性信息
#define CTRL_CMD_GETMDPROPERTY			CTRL_CMD_BASE+7			//移动侦测属性信息
#define CTRL_CMD_STOPVIDEOMONITOR		CTRL_CMD_BASE+8			//停止视频预览
#define CTRL_CMD_STOPAUDIOMONITOR		CTRL_CMD_BASE+9			//停止音频预览
#define CTRL_CMD_STOPVOIP				CTRL_CMD_BASE+10		//停止voip
#define CTRL_CMD_STOPDOWNLOAD			CTRL_CMD_BASE+11		//停止下载
#define CTRL_CMD_STOPFILEPLAY			CTRL_CMD_BASE+12		//停止文件回放
#define CTRL_CMD_STOPTIMEPLAY			CTRL_CMD_BASE+13		//停止时间回放
#define CTRL_CMD_FASTPLAY				CTRL_CMD_BASE+14		//快速播放
#define CTRL_CMD_SLOWPLAY				CTRL_CMD_BASE+15		//慢速播放
#define CTRL_CMD_SETPLAYRATE			CTRL_CMD_BASE+16		//播放速率
#define CTRL_CMD_PAUSEPLAY				CTRL_CMD_BASE+17		//暂停播放
#define CTRL_CMD_RESUMEPLAY				CTRL_CMD_BASE+18		//恢复播放
#define CTRL_CMD_SINGLEPLAY				CTRL_CMD_BASE+19		//帧进
#define CTRL_CMD_FASTBACKPLAY			CTRL_CMD_BASE+20		//快退
#define CTRL_CMD_PLAYPREV				CTRL_CMD_BASE+21		//上一段
#define CTRL_CMD_PLAYNEXT				CTRL_CMD_BASE+22		//下一段
#define CTRL_CMD_PLAYSEEK				CTRL_CMD_BASE+23		//定位
#define CTRL_CMD_PLAYMUTE				CTRL_CMD_BASE+24		//回放静音
#define CTRL_CMD_PLAYPROGRESS 			CTRL_CMD_BASE+25		//播放进度
#define CTRL_CMD_GETNETWORK				CTRL_CMD_BASE+26		//获得网络参数
#define CTRL_CMD_SETNETWORK				CTRL_CMD_BASE+27		//设置网络参数
#define CTRL_CMD_GETVGASOLLIST			CTRL_CMD_BASE+28		//获得VGA分辨率
#define CTRL_CMD_GETSYSPARAM			CTRL_CMD_BASE+29		//获得系统参数
#define CTRL_CMD_SETSYSPARAM 			CTRL_CMD_BASE+30		//设置系统参数
#define CTRL_CMD_GETRECPARAM			CTRL_CMD_BASE+31		//获得录像参数
#define CTRL_CMD_SETRECPARAM 			CTRL_CMD_BASE+32		//设置录像参数
#define	CTRL_CMD_GETSUBSTREAMPARAM		CTRL_CMD_BASE+33		//获得子码流参数
#define	CTRL_CMD_SETSUBSTREAMPARAM		CTRL_CMD_BASE+34		//设置子码流参数
#define CTRL_CMD_GETIMGPARAM			CTRL_CMD_BASE+35		//获得图像参数
#define CTRL_CMD_SETIMGPARAM 			CTRL_CMD_BASE+36		//设置图像参数
#define CTRL_CMD_GETPICADJ 				CTRL_CMD_BASE+37		//获得画面参数
#define CTRL_CMD_SETPICADJ 				CTRL_CMD_BASE+38		//设置画面参数
#define CTRL_CMD_GETALARMINPARAM 		CTRL_CMD_BASE+39		//获得报警输入参数
#define CTRL_CMD_SETALARMINPARAM 		CTRL_CMD_BASE+40		//设置报警输入参数
#define CTRL_CMD_GETALARMOUTPARAM 		CTRL_CMD_BASE+41		//获得报警输出参数
#define CTRL_CMD_SETALARMOUTPARAM 		CTRL_CMD_BASE+42		//设置报警输出参数
#define	CTRL_CMD_GETALARMNOTICYPARAM	CTRL_CMD_BASE+43		//获得报警通知参数
#define	CTRL_CMD_SETALARMNOTICYPARAM	CTRL_CMD_BASE+44		//设置报警通知参数
#define CTRL_CMD_GETRECSCHPARAM 		CTRL_CMD_BASE+45		//获得录像布防参数
#define CTRL_CMD_SETRECSCHPARAM 		CTRL_CMD_BASE+46		//设置录像布防参数
#define CTRL_CMD_GETMDPARAM 			CTRL_CMD_BASE+47		//获得移动侦测参数
#define CTRL_CMD_SETMDPARAM 			CTRL_CMD_BASE+48		//设置移动侦测参数
#define CTRL_CMD_GETVIDEOLOSTPARAM		CTRL_CMD_BASE+49		//获得视频丢失参数
#define CTRL_CMD_SETVIDEOLOSTPARAM 		CTRL_CMD_BASE+50		//设置视频丢失参数
#define CTRL_CMD_GETVIDEOBLOCKPARAM 	CTRL_CMD_BASE+51		//获得视频遮挡参数
#define CTRL_CMD_SETVIDEOBLOCKPARAM 	CTRL_CMD_BASE+52		//设置视频遮挡参数
#define CTRL_CMD_GETPTZPARAM			CTRL_CMD_BASE+53		//获得云台参数
#define CTRL_CMD_SETPTZPARAM 			CTRL_CMD_BASE+54		//设置云台参数
#define CTRL_CMD_SETPRESET 				CTRL_CMD_BASE+55		//设置预置点参数
#define CTRL_CMD_GETCRUISEPARAM 		CTRL_CMD_BASE+56		//获得巡航路径参数
#define CTRL_CMD_SETCRUISEPARAM 		CTRL_CMD_BASE+57		//设置巡航路径参数
#define CTRL_CMD_CTRLCRUISEPATH 		CTRL_CMD_BASE+58		//巡航路径控制
#define CTRL_CMD_CTRLPTZTRACK 			CTRL_CMD_BASE+59		//轨迹控制
#define CTRL_CMD_GETHDDINFO 			CTRL_CMD_BASE+60		//获得磁盘信息
#define CTRL_CMD_GETUSERINFO			CTRL_CMD_BASE+61		//获得用户信息
#define CTRL_CMD_SETUSERINFO 			CTRL_CMD_BASE+62		//设置用户信息
#define CTRL_CMD_SETRESTORE				CTRL_CMD_BASE+63		//恢复默认
#define CTRL_CMD_CLEARALARM				CTRL_CMD_BASE+64		//清除报警
#define CTRL_CMD_GETSYSTIME				CTRL_CMD_BASE+65		//获得系统时间
#define CTRL_CMD_SETSYSTIME				CTRL_CMD_BASE+66		//设置系统时间
#define CTRL_CMD_GETSYSINFO				CTRL_CMD_BASE+67		//获得系统信息
#define CTRL_CMD_SHUTDOWN				CTRL_CMD_BASE+68		//关闭系统
#define CTRL_CMD_REBOOT					CTRL_CMD_BASE+69		//重启系统
#define CTRL_CMD_PTZCTRL				CTRL_CMD_BASE+70		//云台控制
#define CTRL_CMD_GETMANUALREC			CTRL_CMD_BASE+71		//获得手动录像状态
#define CTRL_CMD_SETMANUALREC			CTRL_CMD_BASE+72		//设置手动录像状态
#define CTRL_CMD_LOGSEARCH				CTRL_CMD_BASE+73		//日志文件搜索
#define CTRL_CMD_RECFILESEARCH			CTRL_CMD_BASE+74		//回放文件搜索
#define CTRL_CMD_GETSPSPPS				CTRL_CMD_BASE+75		//获得spspps数据
#define CTRL_CMD_ALARMUPLOADCENTER		CTRL_CMD_BASE+76		//报警上传中心
#define CTRL_CMD_CDROM					CTRL_CMD_BASE+77		//光驱控制
#define CTRL_CMD_COM_PROTOCOL_GET		CTRL_CMD_BASE+78		//获得串口参数
#define CTRL_CMD_COM_PROTOCOL_SET		CTRL_CMD_BASE+79		//设置串口参数
#define CTRL_CMD_SERIALSTOP				CTRL_CMD_BASE+80		//停止透明串口
#define CTRL_CMD_GETAUDIOVOLUME			CTRL_CMD_BASE+81		//获得某通道音量信息
#define CTRL_CMD_SETAUDIOVOLUME			CTRL_CMD_BASE+82		//设置某通道音量信息
#define CTRL_CMD_GETSYSLANGLIST			CTRL_CMD_BASE+83		//获得系统语言列表
#define CTRL_CMD_GETBITRATELIST			CTRL_CMD_BASE+84		//获得码流位率列表
#define CTRL_CMD_XWSERVER_GET			CTRL_CMD_BASE+85		//csp modify 20100419
#define CTRL_CMD_XWSERVER_SET			CTRL_CMD_BASE+86		//csp modify 20100419
//#define CTRL_CMD_GETRECFILEINDEX		CTRL_CMD_BASE+87		//csp modify 20100419
#define CTRL_CMD_GETEMAILSMTP			CTRL_CMD_BASE+88		//获得报警EmailSMTP信息
#define CTRL_CMD_SETEMAILSMTP			CTRL_CMD_BASE+89		//设置报警EmailSMTP信息
#define CTRL_CMD_LASAFQQ_GETINFO		CTRL_CMD_BASE+90		//获得龙安视安防qq信息
#define CTRL_CMD_LASAFQQ_SETINFO		CTRL_CMD_BASE+91		//设置龙安视安防qq信息
#define CTRL_CMD_SETMAC					CTRL_CMD_BASE+92		//设置MAC地址
#define CTRL_CMD_USEUTCTIME				CTRL_CMD_BASE+93		//是否使用UTC格式时间
#define CTRL_CMD_GETPRESETLIST			CTRL_CMD_BASE+94		//获得预置点列表
#define CTRL_CMD_ADDPRESET_BYNAME		CTRL_CMD_BASE+95		//添加预置点（名称）
#define CTRL_CMD_GETPTZRATE				CTRL_CMD_BASE+96		//获得云台当前速度
#define CTRL_CMD_RESETPICADJUST			CTRL_CMD_BASE+97		//恢复默认画面参数
#define CTRL_CMD_GETFRAMERATELIST		CTRL_CMD_BASE+98		//获得帧率列表
#define CTRL_CMD_GETMAX_IMGMASKNUM		CTRL_CMD_BASE+99		//获得支持的最大遮盖数
#define CTRL_GET_RESOLUTION_LIST		CTRL_CMD_BASE+100		//获得主/子码流分辨率列表
#define CTRL_CMD_NRSERVER_GET			CTRL_CMD_BASE+101		//获取南瑞服务器配置
#define CTRL_CMD_NRSERVER_SET			CTRL_CMD_BASE+102		//设置南瑞服务器配置
#define CTRL_CMD_GET_ALARMIN_VAL		CTRL_CMD_BASE+103       //获取报警输入值
#define CTRL_CMD_SET_ALARMOUT_VAL		CTRL_CMD_BASE+104       //设置报警输出值
#define CTRL_CMD_STOPGETFILEBYTIME		CTRL_CMD_BASE+105		//停止按时间下载
#define CTRL_CMD_GETALARMULSTATE		CTRL_CMD_BASE+106		//获取报警上传状态
#define CTRL_CMD_GET_SPECIALDEVICEINFO	CTRL_CMD_BASE+107		//获得特殊的系统信息，用于某些型号的兼容
#define	CTRL_CMD_GET_CREAROSVR_INFO		CTRL_CMD_BASE+108		//获得创世平台信息
#define	CTRL_CMD_SET_CREAROSVR_INFO		CTRL_CMD_BASE+109		//设置创世平台信息
#define CTRL_CMD_GET_HARDWAREVERSION	CTRL_CMD_BASE+110		//获得硬件版本信息	
#define	CTRL_CMD_GET_RECFILE_LIMIT_PARAM CTRL_CMD_BASE+111		//获得录像文件限制参数
#define	CTRL_CMD_SET_RECFILE_LIMIT_PARAM CTRL_CMD_BASE+112		//设置录像文件限制参数
#define CTRL_CMD_MAKE_KEYFRAME			CTRL_CMD_BASE+113		//强制主/子码流产生一个关键帧
// #define CTRL_CMD_EXTLOGIN				CTRL_CMD_BASE+114		//远程登录命令(用户名最大15个字符)
// #define CTRL_CMD_EXTLOGOFF				CTRL_CMD_BASE+115		//注销用户登录(用户名最大15个字符)
// #define CTRL_CMD_EXTGETUSERINFO			CTRL_CMD_BASE+116		//获得用户信息(用户名最大15个字符)
// #define CTRL_CMD_EXTSETUSERINFO 			CTRL_CMD_BASE+117		//设置用户信息(用户名最大15个字符)
#define	CTRL_CMD_REGISTDDNS				CTRL_CMD_BASE+118		//注册DDNS
#define CTRL_CMD_CANCELDDNS				CTRL_CMD_BASE+119		//注销DDNS
#define CTRL_CMD_GETALARMSCHPARAM		CTRL_CMD_BASE+120		//获得报警输入布防参数
#define CTRL_CMD_SETALARMSCHPARAM		CTRL_CMD_BASE+121		//设置报警输入布防参数
// #define CTRL_CMD_SEARCHWIFIINFO			CTRL_CMD_BASE+122		//搜索WIFI
// #define CTRL_CMD_ENABLEWIFI				CTRL_CMD_BASE+123		//启用WIFI
// #define CTRL_CMD_DISCONNECTWIFI			CTRL_CMD_BASE+124		//断开WIFI
// #define CTRL_CMD_GETPORTINFO			CTRL_CMD_BASE+125		//
// #define CTRL_CMD_SETPORTINFO			CTRL_CMD_BASE+126		//
// #define CTRL_CMD_GETHDMIINFO			CTRL_CMD_BASE+127		//
// #define CTRL_CMD_SETHDMIINFO			CTRL_CMD_BASE+128		//
#define CTRL_CMD_GETMDSENSELIST			CTRL_CMD_BASE+129		//获取移动侦测灵敏度列表
#define CTRL_CMD_GETMDALARMDELAYLIST 	CTRL_CMD_BASE+130		//获取移动侦测报警输出延时列表
#define CTRL_CMD_GETBAUDRATELIST		CTRL_CMD_BASE+131		//获取波特率列表
#define CTRL_CMD_GETPTZPROTOCOLLIST		CTRL_CMD_BASE+132		//获取云台协议列表
//#define CTRL_GETFRAMERATELIST_BYRESOLUTION	CTRL_CMD_BASE+133		//获得主/子对应分辨率的帧率列表
#define CTRL_CMD_GETRECSCHPARAMBYTYPE 	CTRL_CMD_BASE+133		//按布防类型获得录像布防参数
#define CTRL_CMD_SETRECSCHPARAMBYTYPE	CTRL_CMD_BASE+134		//按布防类型分开设置录像布防参数
#define CTRL_CMD_GETHUINAINFO			CTRL_CMD_BASE+135		//获取会纳信息
#define CTRL_CMD_SETHUINAINFO			CTRL_CMD_BASE+136		//设置会纳信息
// #define CTRL_CMD_GETNONETWORKPARAM 		CTRL_CMD_BASE+137		//获得网络不通参数
// #define CTRL_CMD_SETNONETWORKPARAM 		CTRL_CMD_BASE+138		//设置网络不通参数

//csp modify
#define CTRL_CMD_GETREBOOTTIME			CTRL_CMD_BASE+145		//获取重启时间
#define CTRL_CMD_GETUPLOADALARMTIME		CTRL_CMD_BASE+150		//获取报警上传时间间隔
#define CTRL_CMD_SETUPLOADALARMTIME		CTRL_CMD_BASE+151		//设置报警上传时间间隔

#define CTRL_CMD_GETDDNSLIST			CTRL_CMD_BASE+152		//获取DDNS服务商列表

//csp modify 20130423
#define CTRL_CMD_SETMONITORINFO			CTRL_CMD_BASE+160		//设置远程监控信息

//csp modify 20130519
#define CTRL_CMD_GETADVPRIVILEGE		CTRL_CMD_BASE+161		//获取用户高级权限
//xdc
#define CTRL_CMD_GETSEACHIPCLIST		CTRL_CMD_BASE+162		//获取搜索的摄像头列表
#define CTRL_CMD_GETADDIPCLIST			CTRL_CMD_BASE+163		//获取已添加的摄像头列表
#define CTRL_CMD_SETIPC					CTRL_CMD_BASE+164			//设置对应通道摄像头
#define CTRL_CMD_ADDIPC					CTRL_CMD_BASE+165			//添加摄像头
#define CTRL_CMD_DELETEIPC				CTRL_CMD_BASE+166			//删除摄像头
#define CTRL_CMD_GETTHEOTHER			CTRL_CMD_BASE+167		//获取搜索到的剩下的摄像头列表
//yaogang modify 20141030
#define CTRL_CMD_GET_PATROL_PARA		CTRL_CMD_BASE+168		//获取轮巡参数
#define CTRL_CMD_SET_PATROL_PARA		CTRL_CMD_BASE+169		//设置轮巡参数
#define CTRL_CMD_GET_PREVIEW_PARA		CTRL_CMD_BASE+170		//获取预览参数
#define CTRL_CMD_SET_PREVIEW_PARA		CTRL_CMD_BASE+171		//设置预览参数
#define CTRL_CMD_GET_CHN_PARA			CTRL_CMD_BASE+172		//获取通道参数
#define CTRL_CMD_SET_CHN_PARA			CTRL_CMD_BASE+173		//设置通道参数
#define CTRL_CMD_CLEAN_ALARM_ICON		CTRL_CMD_BASE+174		//清除桌面报警三角标
#define CTRL_CMD_CLOSE_GUIDE			CTRL_CMD_BASE+175		//跃天解码板CMS remote CTRL

//yaogang modify 20160122
#define CTRL_CMD_GET_IPCCHN_LINKSTATUS	CTRL_CMD_BASE+176		//获取IPC通道连接状态(一个IPC有两个通道，主、子码流)


//协议
#define CTRL_PROTOCOL_SERVERPORT		6630					//服务器端口 6610
#define CTRL_PROTOCOL_MAXLINKNUM		256
#define CTRL_PROTOCOL_CONNECT_BLOCK		INFINITE				//连接方式:阻塞
#define CTRL_PROTOCOL_CONNECT_DEFAULT	3000					//缺省连接时间:3秒
#define CTRL_VERSION					0x0100
#define CTRL_COMMAND					0
#define CTRL_NOTIFY						1
#define CTRL_ACK						2

//事件通知类型
#define CTRL_NOTIFY_BASE				20000
#define	CTRL_NOTIFY_CONNLOST			CTRL_NOTIFY_BASE+0		//断链消息
#define	CTRL_NOTIFY_HEARTBEAT_REQ		CTRL_NOTIFY_BASE+1		//心跳请求
#define	CTRL_NOTIFY_HEARTBEAT_RESP		CTRL_NOTIFY_BASE+2		//心跳回复
#define CTRL_NOTIFY_PLAYEND				CTRL_NOTIFY_BASE+3		//放像结束
#define CTRL_NOTIFY_PLAYPROGRESS		CTRL_NOTIFY_BASE+4		//放像进度通知
#define CTRL_NOTIFY_HASAUDIO			CTRL_NOTIFY_BASE+5		//回放文件是否有音频
#define CTRL_NOTIFY_UPDATEPROGRESS		CTRL_NOTIFY_BASE+6		//升级进度
#define CTRL_NOTIFY_ALARMINFO			CTRL_NOTIFY_BASE+7		//异步报警信息

//应答类型
#define CTRL_SUCCESS					0						//成功
#define CTRL_FAILED_BASE				30000					//错误码偏移量
#define CTRL_FAILED_USER				CTRL_FAILED_BASE+1		//不存在的用户名
#define CTRL_FAILED_PASSWORD			CTRL_FAILED_BASE+2		//密码错误
#define CTRL_FAILED_COMMAND				CTRL_FAILED_BASE+3		//未认可的命令
#define CTRL_FAILED_PARAM				CTRL_FAILED_BASE+4		//无效参数
#define CTRL_FAILED_OUTOFMEMORY			CTRL_FAILED_BASE+5		//内存不足
#define CTRL_FAILED_RESOURCE			CTRL_FAILED_BASE+6		//资源不足
#define CTRL_FAILED_FILENOTEXIST		CTRL_FAILED_BASE+7		//文件不存在
#define CTRL_FAILED_DATABASE			CTRL_FAILED_BASE+8		//数据库错误
#define CTRL_FAILED_RELOGIN				CTRL_FAILED_BASE+9		//重复登录
#define CTRL_FAILED_BAUDLIMIT			CTRL_FAILED_BASE+10		//超过每一路通道最多支持实时监控			
#define CTRL_FAILED_CREATESOCKET		CTRL_FAILED_BASE+11		//创建套结字失败
#define CTRL_FAILED_CONNECT				CTRL_FAILED_BASE+12		//网络连接失败
#define CTRL_FAILED_BIND				CTRL_FAILED_BASE+13		//绑定失败
#define CTRL_FAILED_LISTEN				CTRL_FAILED_BASE+14		//侦听失败
#define CTRL_FAILED_NETSND				CTRL_FAILED_BASE+15		//网络发送出错
#define CTRL_FAILED_NETRCV				CTRL_FAILED_BASE+16		//网络接收出错
#define CTRL_FAILED_TIMEOUT				CTRL_FAILED_BASE+17		//网络连接超时
#define CTRL_FAILED_CHNERROR			CTRL_FAILED_BASE+18		//超出通道限制
#define CTRL_FAILED_DEVICEBUSY			CTRL_FAILED_BASE+19		//设备正在忙
#define CTRL_FAILED_WRITEFLASH			CTRL_FAILED_BASE+20		//烧写flash出错
#define CTRL_FAILED_VERIFY				CTRL_FAILED_BASE+21		//校验错
#define CTRL_FAILED_CONFLICT			CTRL_FAILED_BASE+22		//系统资源冲突
#define CTRL_FAILED_BUSY				CTRL_FAILED_BASE+23		//系统正在忙
#define CTRL_FAILED_LINKLIMIT			CTRL_FAILED_BASE+24		//已达到连接上限
#define CTRL_FAILED_USER_SAME			CTRL_FAILED_BASE+25		//用户名相同07-08-02
#define CTRL_FAILED_MACADDR				CTRL_FAILED_BASE+26		//远程访问的pc mac地址错误
#define CTRL_FAILED_NOINIT				CTRL_FAILED_BASE+27		//模块未初始化
#define CTRL_FAILED_USER_MAX			CTRL_FAILED_BASE+28		//用户数最多//wrchen 080529
#define CTRL_FAILED_UNKNOWN				CTRL_FAILED_BASE+9999	//未知错误
//连接
#define CTRL_CONNECTION_NULL			0x0
#define CTRL_CONNECTION_TCPCLIENT		0x1
#define CTRL_CONNECTION_TCPSERVER		0x2

//组播搜索设备
#define CTRL_DEVICESEARCH_ACKCLIENT		0X1
#define CTRL_DEVICESEARCH_ACKSERVER		0x2
#define SEARCHPORT						6666
#define ACKSEARCHPORT					SEARCHPORT+1
#define MULTICASTGROUP					"224.0.1.2"

//码流传输
#define VIDEO_CHANNELS		16
#define AUDIO_CHANNELS		16
#define EACH_STREAM_TCP_LINKS	5
#define MAX_REMOTE_PLAYER_NUM	8
#define VOIP_NUM				1
#define DOWNLOAD_NUM			1
#define	UPDATE_NUM				1
#define MEDIA_LINKMAX_SVR			(VIDEO_CHANNELS+AUDIO_CHANNELS)*(EACH_STREAM_TCP_LINKS)+MAX_REMOTE_PLAYER_NUM+VOIP_NUM
#define	STREAM_LINK_MAXNUM		MEDIA_LINKMAX_SVR+DOWNLOAD_NUM+UPDATE_NUM


#ifdef WIN32
typedef HANDLE MutexHandle;
typedef HANDLE SemHandle;
#else
typedef pthread_mutex_t MutexHandle;
typedef sem_t SemHandle;
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#include <netinet/tcp.h>
#endif


#pragma pack( push, 1 )

typedef struct
{
	u8	data[6];
}CPGuid;

typedef struct
{
	u32         ip;
	u16			port;
	CPGuid		guid;
	SOCKHANDLE	sockfd;
	u8			conntype;
	u8          newmsgcome;
	u8          nolnkcount;
	//yaogang modify for server heart beat check
	MutexHandle hMutex;
	time_t last_msg_time;//上次通信时间，命令或心跳回应都可以
}ifly_cp_t,*CPHandle;

typedef struct
{
	u32		length;						//消息长度		
	u16		type;						//消息类型		
	u16		event;						//消息名		
	u16		number;						//流水号		
	u16		version;					//版本号		
}ifly_cp_header_t;

//协议头结构
typedef struct 
{
	u8	safeguid[16];			// PROTOGUID
	u8  protocolver;			//协议版本
	u8	byConnType;			//连接类型，0x1：控制信令；0x2：码流传输；0x3：广播搜索；0x4  轮巡同步(只对已经使能轮巡的设备有效)
	u8	reserved[2];			//保留字段
}ifly_ProtocolHead_t;


//设备信息
typedef struct 
{
	u32	deviceIP; 						//设备IP  
	u16	devicePort;						//设备端口 
	char device_name[32];				//设备名称
	char device_mode[32];				//设备型号
	u8	maxChnNum;						//最大通道数
	u8	maxAduioNum;					//最大音频数
	u8	maxSubstreamNum;				//最大子码流数
	u8	maxPlaybackNum;					//最大回放数
	u8	maxAlarmInNum;					//最大报警输入数
	u8	maxAlarmOutNum;					//最大报警输出数
	u8	maxHddNum;						//最大硬盘数
	u8	nNVROrDecoder;	//区别NVR和解码器--- 跃天
	u8	reserved[15];					//预留
}ifly_DeviceInfo_t;


//视频属性
typedef struct  
{
	u8	videoEncType;					//视频编码类型
	u16	max_videowidth;					//
	u16	max_videoheight;					
	u8	reserved[3];					//预留
}ifly_Video_Property_t;

//音频属性
typedef struct  
{
	u8	audioEnctype;					//预览音频编码类型
	u8	audioBitPerSample;				//预览音频位数
	u16 audioSamplePerSec;				//预览音频采样率
	u16 audioFrameSize;					//预览音频每帧大小
	u16	audioFrameDurTime;				//预览音频每帧间隔
	u8	reserved[4];					//预留
}ifly_Audio_Property_t;

//VOIP属性
typedef struct  
{
	u8	VOIPBitPerSample;				//语音对讲位数
	u16 VOIPSamplePerSec;				//语音对讲采样率
	u16 VOIPFrameSize;					//语音对讲每帧大小
	u8	reserved[3];					//预留
}ifly_VOIP_Property_t;

//移动侦测属性
typedef struct  
{
	u8	MDCol;							//移动侦测列数
	u8	MDRow;							//移动侦测行数
	u8	reserved[4];					//预留
}ifly_MD_Property_t;


//码流传输请求结构
typedef struct 
{
	u8 command;							//0：预览 1：文件回放 2：时间回放 3：文件下载 4：升级 
										//5 VOIP 6 文件按帧下载 7 时间按帧下载 8 透明通道
										//9 远程格式化硬盘 10 主机端抓图 11 多路按时间回放 12 按时间下载文件
	union		//72byte
	{
		struct
		{
			u8		chn;				//预览通道
			u8		type;				//0:主码流视频 1：主码流音频 2：子码流视频
		}Monitor_t;						//预览 command = 0
		
		struct
		{
			char	filename[64];		//回放的文件
			u32		offset;				//文件偏移
		}FilePlayBack_t; 				//文件回放 command = 1,或 command = 6
		
		struct
		{
			u8		channel;			//通道数
			u16		type;				//类型
			u32		start_time;			//开始时间
			u32		end_time;			//终止时间
		}TimePlayBack_t ;				//时间回放 command = 2,或 command = 7
		
		struct
		{
			char	filename[64];		//下载的文件
			u32		offset;				//文件偏移
			u32		size;				//文件大小
		}FileDownLoad_t;				//文件下载 command = 3
		
		struct
		{
			u32		size;				//文件长度
			u32		verify;				//校验和
			u16		version;			//文件版本号
			u16		updateType;			//0:主板升级 1：面板升级
		}Update_t;						//远程升级  command = 4；
		
		//VOIP command
		u8 voipreserved;				//VOIP预留 command = 5；

		//透明通道
		u8 SerialPort;					//透明通道 1-232串口  2-485串口	command = 8；

		//远程格式化硬盘
		u8 formatHddIndex;				//要格式化的硬盘号 0开始

		struct
		{
			u16		chnMask;			//通道数 按位 特指通道1-16
			u16		type;				//类型
			u32		start_time;			//开始时间
			u32		end_time;			//终止时间
			u16		chn17to32Mask;		//通道17-32 按位
		}MultiTimePlayBack_t ;			//多路时间回放 command = 11，12
	};
	u8	reserved[7];					//预留	
}ifly_TCP_Stream_Req;					 


//码流传输主机端回应结构
typedef struct
{
	u32	errcode;						//连接成功返回0， 否则返回其他错误码
	u32	ackid;							//对于文件回放，时间回放。 用于后续客户端控制回放操作和区分进度, 预览控制
}ifly_TCP_Stream_Ack;

typedef struct  
{
	u16 errcode;
	u8 pos;
}ifly_TCP_Pos;

//预览，回放和 VOIP帧结构
typedef struct
{
	u8	m_byMediaType;					//类型
	u8	m_byFrameRate;					//帧率
	u8	m_bKeyFrame;					//是否关键帧
	int m_nVideoWidth;					//宽
	int m_nVideoHeight;					//高
	u32 m_dwDataSize;					//数据大小
	u32 m_dwFrameID;					//帧ID
	u32 m_dwTimeStamp;					//时间戳
}ifly_MediaFRAMEHDR_t;

//登录
typedef struct
{
	char	username[12];				//用户名
	char	loginpass[12];				//密码
	char	macAddr[18];				//MAC地址		
	u32		ipAddr;						//IP地址
}ifly_loginpara_t;

//网络参数
typedef struct
{
	char	mac_address[18];			//mac地址
	u32		ip_address;					//ip地址
	u16		server_port;				//设备端口
	u32		net_mask;					//掩码
	u32		net_gateway;				//网关
	u32		dns;						//dns
	u8		flag_multicast;				//组播启用标记
	u32		multicast_address;			//组播地址
	u16		http_port;					//http端口
	u8		flag_pppoe;					//pppoe启用标记
	char	pppoe_user_name[64];		//pppoe用户名
	char	pppoe_passwd[64];			//pppoe密码
	u8      flag_dhcp;					//dhcp启用标志
	u8		ddnsserver;					//ddns服务商
	u8		flag_ddns;					//ddns启用标志
	char	ddns_domain[64];			//ddns域名
	char	ddns_user[64];				//ddns用户名
	char	ddns_passwd[64];			//ddns密码
	u32		centerhost_address;			//中心服务器地址
	u16		centerhost_port;			//中心服务器端口
	u16		mobile_port;				//手机监控端口
	char	hwid[12];					//俊明视定制
	u8		reserved[2];				//保留字段
}ifly_NetWork_t;

//分辨率最多支持16种不支持的请置0
typedef struct
{
	u16		width;						//分辨率宽
	u16		height;						//分辨率高
	u8		flashrate;					//分辨率刷新率
}ifly_VGA_Pro;

typedef struct 
{
	ifly_VGA_Pro vgapro[16];
}ifly_VGA_Solution;

//系统参数
typedef struct
{
	u16		device_id;					//设备id
	char	device_name[32];			//设备名称
	u8		flag_overwrite;				//硬盘满时覆盖标记
	u16		lock_time;					//键盘锁定时间			
	u16		switch_time;				//切换时间
	u8		flag_statusdisp;			//状态显示标记
	u8		video_format;				//视频制式					
	u8		vga_solution;				//VGA分辨率 ifly_VGA_Solution[16];
	u8		transparency;				//菜单透明度
	u8		language;					//系统语言
	u8		disable_pw;					//密码禁用
	u8		flag_bestrow;				//24小时覆盖
	u8		reserved[15];				//保留字段
}ifly_SysParam_t;

//录像参数
typedef struct									
{
	u8		channelno;					//通道号
	u8		code_type;					//码流类型
	u8		bit_type;					//位率类型
	u32		bit_max;					//位率上限
	u16		intraRate;					//关键帧间隔
	u8		qi;							//关键帧量化因子
	u8		minQ;						//最小量化因子
	u8		maxQ;						//最大量化因子
	u8		quality;					//图像质量
	u8		frame_rate;					//视频帧率
	u16		pre_record;					//预录时间
	u16		post_record;				//录像延时
	u32		copy2chnmask;				//复制到其他通道。每一位一个通道
	u8		supportdeinter;				//是否支持deinter设置 1是 0否 (待用)
	u8		deinterval;					//deinter强度 0-4 禁用，弱，中，强，超强
	u8		supportResolu;				//是否支持设置录像分辨率
	u8		resolutionpos;				//分辨率选项值
	u8		reserved[12];				//保留字段
}ifly_RecordParam_t;

//子码流参数
typedef struct									
{
	u8		channelno;					//通道号
	u8		sub_flag;					//区分多种子码流列问 0 cif 1 qcif
	u8		sub_bit_type;				//子码流位率类型
	u16		sub_intraRate;				//关键帧间隔
	u8		sub_qi;						//关键帧量化因子
	u8		sub_minQ;					//最小量化因子
	u8		sub_maxQ;					//最大量化因子
	u8		sub_quality;				//子码流图像质量
	u8 		sub_framerate;				//子码流的帧率
	int 	sub_bitrate;				//子码流的位率
	u32		copy2chnmask;				//复制到其他通道。每一位一个通道
	u8		reserved[16];				//保留字段
}ifly_SubStreamParam_t;

//图像参数
typedef struct
{
	u8		channel_no;					//通道号
	char	channelname[32];			//通道名
	u8		flag_name;					//名称位置显示
	u16		chnpos_x;					//名称x坐标
	u16		chnpos_y;					//名称y坐标
	u8		flag_time;					//时间位置显示
	u16		timepos_x;					//时间x坐标
	u16		timepos_y;					//时间y坐标
	u8		flag_mask;					//遮盖
	struct  Mask_t
	{
		u16	 	x;
		u16		y;
		u16		width;
		u16		height;
	}MaskInfo[4];
	u8		flag_safechn;				//安全通道标记
	u32		copy2chnmask;				//复制到其他通道。每一位一个通道
	u8		reserved[16];				//保留字段
}ifly_ImgParam_t;

//画面参数
typedef	struct  
{
	u8		channel_no;					//通道号
	u8		flag;						//调节标志:0-3
	u8		val;						//调节值
	u32		copy2chnmask;				//复制到其他通道。每一位一个通道
}ifly_PicAdjust_t;

//报警输入参数
typedef struct									
{
	u8		inid;						//报警输入量 
	u8		typein;						//报警输入类型
	u8		flag_deal;					//是否触发报警输入处理
	u32		triRecChn;					//触发录像通道，每一位一通道
	u32		triAlarmoutid;				//触发报警输出，按位
	u8		flag_buzz;					//触发蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16		delay;						//报警输出延时
	struct  AlarmInPtz_t				//关联ptz
	{
		u8		flag_preset;			//预置点
		u8		preset;
		u8		flag_cruise;			//巡航点
		u8		cruise;
		u8		flag_track;				//轨迹
	} AlarmInPtz [32];
	u32		copy2AlarmInmask;			//复制到其他报警输入。按位
	u8		flag_enablealarm;			//报警启用标志
	u8		reserved[15];				//保留字段
}ifly_AlarmInParam_t;

//报警输出参数
typedef struct									
{
	u8		outid;						//报警输出量 
	u8		typeout;					//报警输出类型
	u32		copy2AlarmOutmask;			//复制到其他报警输出。按位
#if 1//csp modify
	u16		alarmoutdelay;				//报警输出延时
	u8		flag_buzz;					//触发蜂鸣器
	u16		buzzdelay;					//蜂鸣器延时
	u8		reserved[11];
#else
	u8		reserved[16];				//保留字段
#endif
}ifly_AlarmOutParam_t;

//报警通知参数: email报警手机报警
typedef struct									
{

	char	alarm_email[32];			//报警email地址
	char	alarm_mobile[32];			//报警手机地址
	u8		reserved[32];				//保留字段
}ifly_AlarmNoticeParam_t;

//录像布防
typedef struct
{
	u8		chn;						//通道
	u8		weekday;					//星期
	struct RecTimeField_t
	{
		u32 starttime;					//起始时间
		u32 endtime;					//终止时间
		u8	flag_sch;					//定时录像
		u8 	flag_md;					//移动侦测录像
		u8  flag_alarm;					//报警录像
		u8	reserved[4];				//保留字段
	}TimeFiled[4];
	u8		copy2Weekday;				//复制到其他天  按位 
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		reserved[16];				//保留字段
}ifly_RecordSCH_t;

//移动侦测
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	u8		sense;						//灵敏度
	u8		block[44*36];				//区域
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		reserved[12];				//保留字段
}ifly_MDParam_t;

//视频丢失
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		reserved[12];				//保留字段
}ifly_VideoLostParam_t;

//for2.0
//移动侦测
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	u8		sense;						//灵敏度
	u8		block[44*36];				//区域
	struct  MDPtz_t				//关联ptz
	{
		u8		flag_preset;			//预置点
		u8		preset;
		u8		flag_cruise;			//巡航点
		u8		cruise;
		u8		flag_track;				//轨迹
	} MDPtz [32];
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		flag_enablealarm;			//报警启用标志
	u8		reserved[11];				//保留字段
}ifly_MDParamfor2_0_t;

//视频丢失
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	struct  VideoLostPtz_t				//关联ptz
	{
		u8		flag_preset;			//预置点
		u8		preset;
		u8		flag_cruise;			//巡航点
		u8		cruise;
		u8		flag_track;				//轨迹
	} VideoLostPtz [32];
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		reserved[12];				//保留字段
}ifly_VideoLostParamfor2_0_t;

//视频遮挡
typedef struct
{
	u8 		chn;						//通道
	u32 	trigRecChn;					//触发录像通道 按位
	u32 	trigAlarmOut;				//触发报警输出 按位
	u8 		flag_buzz;					//蜂鸣器
	u8		flag_email;					//触发emaill
	u8		flag_mobile;				//触发手机报警
	u16 	delay;						//延时
	u32		copy2Chnmask;				//复制到其他通道。按位
	u8		reserved[12];				//保留字段
}ifly_VideoBlockParam_t;

//云台参数
typedef struct
{
	u8		chn;						//通道
	u16		address;					//解码器地址
	u8		baud_ratesel;				//波特率 选项值
	u8		data_bitsel;				//数据位 选项值
	u8		stop_bitsel;				//停止位 选项值
	u8		crccheck;					//校验
	u8		flow_control;				//流控
	u8		protocol;					//协议类型
	u32		copy2Chnmask;				//复制到其他通道 按位
	u8		comstyle;					//串口类型(232 or 485)
#if 1//csp modify
	u8		enableptz;					//通道云台(串口)使能
#else
	u8		reserved;					//保留字段
#endif
}ifly_PTZParam_t;

//预置点控制
typedef struct
{
	u8		chn;						//通道
	u8		presetpoint;				//预置点号
	u8		option;						//操作 0添加 1删除 2 到预置点
}ifly_PtzPresetCtr_t;

//巡航路径参数
typedef struct
{
	u8		chn;						//通道
	u8		cruise_path;				//巡航路径
	struct Cruise_point_t
	{
		u8  preset;						//预置点
		u8  weeltime;					//停留时间
		u8  rate;						//速度
		u8  flag_add;					//0不处理 1添加 2删除
	}Cruise_point[16];
}ifly_PtzCruisePathParam_t;

//巡航路径控制
typedef struct
{
	u8		chn;						//通道
	u8		cruisepath;					//巡航路径号
	u8		flagoption;					//操作 0停止 1开始 
}ifly_CruisePathCtr_t;

//轨迹控制
typedef struct
{
	u8		chn;						//通道
	u8		flagoption;					//操作 0记录 1停止记录  2开始轨迹 3停止轨迹 
}ifly_TrackCtr_t;

//硬盘管理
typedef struct 
{
	u8		hdd_index;					//硬盘序号
	u8		hdd_exist;  				//1 exist; 0 not exist
	u32		capability;					//MB
	u32		freesize;					//MB
	u8		reserved[2];				//预留
} ifly_hddInfo_t;

//用户信息
typedef struct 
{
	char	name[12];
	char	password[12];
	char	mac_address[18];//mac地址	
	u8		rcamer;		//remote yuntai 2.0云台
	u8		rrec;		//remote record 2.0录像
	u8		rplay;		//remote playback 2.0回放
	u8		rsetpara;	//remote set param 2.0系统配制
	u8		rlog;		//remote get log 2.0系统日志
	u8		rtool;		//remote use tool 2.0硬盘管理
	u8		rpreview;	//remote preview 2.0远程登录
	u8		ralarm;		//remote alarm 2.0数据备份
	u8		rvoip;		//voip 2.0语音对讲
	u8		lcamer;		//local yuntai 2.0恢复出厂设置
	u8		lrec;		//local record 2.0系统升级
	u8		lplay;		//local palyback 2.0关机
	u8		lsetpara;	//local set param 2.0admin
	u8		llog;		//local log 2.0mac绑定标志
	u8		ltool;		//local tool	
}ifly_user_t;

typedef struct
{
	ifly_user_t	userNum[8];		//最多8 个用户
}ifly_userNumber_t;

typedef struct
{
	ifly_user_t	userInfo;		//用户信息
	u8		flagOption;					//0添加用户 1编辑用户 2删除用户
}ifly_userMgr_t;

//系统时间
typedef struct
{
	u32		systemtime;					//系统时间
	u8		format;						//时间格式 选项值
	u8		flag_time;					//预览时间位置显示
	u16		timepos_x;					//预览时间x坐标
	u16		timepos_y;					//预览时间y坐标
}ifly_sysTime_t;

//系统信息
typedef struct
{
	char	devicename[32];				//设备名
	char	devicemodel[32];			//设备型号
	char	deviceser[32];				//设备序列号
	char	version[64];				//软件版本
}ifly_sysinfo_t;

//云台控制
//控制命令:
//0-停止 1上 2下 3 左 4右 5自转 
//6变倍加 7变倍减 8焦距加 9焦距减 10光圈加 
//11光圈减 12灯光开 13灯光关 14雨刷开 15雨刷关 
//16 快速 17常速 18慢速 19辅助开 20辅助关
typedef struct
{
	u8  chn;							//通道
	u8  cmd;							
	u8	reserved;
}ifly_PtzCtrl_t;

//手动录像
typedef struct
{
	u32  chnRecState;					//通道手动录像状态 按位	
}ifly_ManualRecord_t;

//摄像头搜索
typedef struct
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
}ifly_search_ipc;


//日志搜索
typedef struct
{
	u8		query_mode;					//查询方式		
	u8		main_type;					//主类型		
	u8		slave_type;					//次类型		
	u16		max_return;					//最大返回数		
	u16		startID;					//返回从第一条记录开始		
	u32		start_time;					//开始时间		
	u32		end_time;					//结束时间			
	u8      reserved[16];				//预留		
}ifly_search_log_t;

typedef struct
{
	u32		startTime;
	u8		main_type;					//主类型		
	u8		slave_type;					//次类型		
	char	loginfo[32];				//具体信息		
}ifly_logInfo_t;

typedef struct
{
	u16		sum;						//总记录数		
	u16		startID;					//开始的记录,基址为1.当无返回数据时,startID的值为0		
	u16		endID;						//结束的记录,基址为1.当endID和startID相等时,表示之返回一条记录		
}ifly_search_desc_t;

//回放文件搜索
typedef struct
{
	u16		channel_mask;				//通道 按位组合 特指通道1-16
	u16		type_mask;					//类型 按位组合
	u32		start_time;					//开始时间
	u32		end_time;					//终止时间
	u16		startID;					//返回的第一条记录,从1开始
	u16		max_return;					//每次返回的最大记录数
	u16		chn17to32mask;				//通道17-32 按位组合
	u8		reserved[5];				//预留
	char	bankcardno[21];				//卡号
}ifly_recsearch_param_t;

////远程回放返回的信息结构
typedef struct
{
	u8		channel_no;
	u8		type;
	u32		start_time;
	u32		end_time;
	u8		image_format;				//3:cif;4:4cif
	u8		stream_flag;				//0:视频流;1:复合流
	u32		size;
	u32		offset;
	char	filename[64];
}ifly_recfileinfo_t;

//放像进度通知
typedef struct
{
	u32	 id;							//回放id
	u32  currPos;						//当前进度
	u32  totallen;						//总时间
}ifly_progress_t;

//回放文件是否有音频
typedef struct  
{
	u8 bHasAudio;						//1有音频，0无音频
}ifly_play_hasaudio_t;

//异步报警信息
typedef struct
{
	u32	 time;							//报警发生时间
	u32  alarmInfo;						//报警值
	u32  reserved;						//预留
}ifly_alarminfo_t;


typedef struct  
{
	u8 chn;							//通道
	u8 type;						//0-主码流， 1-子码流
}ifly_spspps_req_t;


typedef struct
{	
	u8 chn;							//通道
	u8 type;						//0-主码流， 1-子码流
	u8 spsbuf[20];					//sps buf
	u8 spslen;						//sps长度
	u8 ppsbuf[20];					//pps buf
	u8 ppslen;						//pps长度
}ifly_spspps_t;

typedef struct 
{
	//0-信号量报警,1-硬盘满,2-信号丢失,3－移动侦测,4－硬盘未格式化,
	//5-读写硬盘出错,6-遮挡报警,7-制式不匹配, 8-非法访问
	u8 type;		
	u8 state;       //1报警 2恢复
	u8 id;			//通道,硬盘号,报警输入号,取决于type 
	u16 reserved1;  //预留
	u32  reserved2; //预留
}ifly_alarmstate_t;


typedef struct
{
	u8 com_protocol;	//协议编号 0:N5,1:D4,2:S8,3:S7,4:F2,5:G2E,6:G2N,7:CR,8:TY,9:YH
	u32 com_baudrate;	//波特率
	u8 com_checkbit;	//校验位 0:NONE,1:奇校验，2:偶校验
	u8 com_databit;		//数据位 8，7，6
	u8 com_stopbit;		//停止位 1，2
	u8 serialport;		//1-232, 2-485
	u8 reserved[23];	//保留
}ifly_com_param_t;

//透明串口发送数据头
typedef struct  
{
	u8 byChannel;	//串口通道，使用485串口时有效，从1开始；232串口作为透明通道时该值设置为0
	u32 dwDataSize;		//发送的数据长度
	u8 reserved[16];	//预留
}ifly_serial_hdr;

//CTRL_CMD_GETAUDIOVOLUME	获得某通道音量信息
//CTRL_CMD_SETAUDIOVOLUME	设置某通道音量信息
typedef struct
{
	u8 audioDeviceTye; //0 通道播放音量 1 对讲输入音量 2对讲输出音量
	u8 chn;			//通道
	u8 muteflag;	//静音标记
	u8 volume;		//音量值 0-100
	u8 audioEnctype;	//编码格式
	u8 reserved[4];	//预留
}ifly_audio_volume_t;

//系统语言列表
enum SYS_LANGUAGE{
	SYS_LANGUAGE_CHS = 0,		//简体中文
	SYS_LANGUAGE_ENU = 1,		//美式英文
	SYS_LANGUAGE_CHT = 2,       //繁体諫文
	SYS_LANGUAGE_GER = 3,		//德语
	SYS_LANGUAGE_FRE = 4,		//法语
	SYS_LANGUAGE_SPA = 5,		//西班牙语
	SYS_LANGUAGE_ITA = 6,		//意大利
	SYS_LANGUAGE_POR = 7,		//葡萄牙语
	SYS_LANGUAGE_RUS = 8,		//俄语
	SYS_LANGUAGE_TUR = 9,		//土耳其语
	SYS_LANGUAGE_THA = 10,		//泰国语
	SYS_LANGUAGE_JAP = 11,		//日语
	SYS_LANGUAGE_HAN = 12,		//韩语
	SYS_LANGUAGE_POL = 13,		//波兰语
	SYS_LANGUAGE_HEB = 14,      //希伯来语Hebrew
	SYS_LANGUAGE_HUN = 15,		//匈牙利语Hungarian
	SYS_LANGUAGE_ROM = 16,      //罗马语Roma
	SYS_LANGUAGE_IRA = 17,		//伊朗语
	SYS_LANGUAGE_CZE = 18,		//捷克语
	SYS_LANGUAGE_VIE = 19,		//越南语
	SYS_LANGUAGE_LIT = 20,		//立陶宛
	SYS_LANGUAGE_SLO = 21,		//斯洛伐克
	SYS_LANGUAGE_ARA = 22,		//阿拉伯语
	SYS_LANGUAGE_GRE = 23,		//希腊语
	SYS_LANGUAGE_RMN = 24,		//罗马尼亚语
	SYS_LANGUAGE_FAR = 25,		//波斯语
	SYS_LANGUAGE_BUL = 26,		//保加利亚
	SYS_LANGUAGE_INVALID = -1	//仅作填充无效值用
};

typedef struct  
{
	u8 max_langnum;		//最多支持语言数 <=32
	u8 langlist[32];	// language list, 最多32项,每项值为SYS_LANGUAGE(转u8)
	u8 reserved[16];	//预留
}ifly_SysLangList_t;

//位率列表
typedef struct  
{
	u8	chn;				//通道
	u8	videotype;			//0 主码流, 1 子码流
	u32 bitratelist[16];	//位率列表, 单位kb, 未使用的填0
	u8	reserved[16];		//预留
}ifly_bitRateList_t;

//
typedef struct
{
	u8   flag_pingtai;
	u32  ip_pingtai;
	u16  port_pingtai;
	u16  port_download;
	char device_name[32];
	char device_passwd[32];
	u8  flag_verifytime;
}ifly_pingtai_xingwang_t;


//报警EMail SMTP信息
typedef struct  
{
	char	alarm_email[32];	//发送报警email地址
	char	SMTP_svr[32];		//emali的SMTP服务器
	char	username[32];		//邮箱用户名
	char	userpw[32];			//邮箱密码
	u16		SMTPport;			//SMTP端口
	u8		flag_ssl;			//是否使用SSL
	u8		reserved[13];		//预留
}ifly_AlarmEmail_SMTP_t;

typedef struct  
{
	char	alarm_email1[32];	//发送报警email地址
	char	alarm_email2[32];	//发送报警email地址
	char	alarm_email3[32];	//发送报警email地址
	char	SMTP_svr[32];		//emali的SMTP服务器
	char	username[32];		//邮箱用户名
	char	userpw[32];			//邮箱密码
	u16		SMTPport;			//SMTP端口
	u8		flag_ssl;			//是否使用SSL
	u8		reserved[13];		//预留
}ifly_AlarmSMTPfo2_0_t;

//主机端抓图
typedef struct  
{
	u8 chn;						//要抓图的通道
	u8 reserved[4];
}ifly_snapreq_t;

typedef struct  
{
	u32 imglenth;				//图片长度
	u8 reserved[4];
}ifly_snapack_t;

//龙安视安防qq
typedef struct  
{
	u8 sn[64];					//sn
	u8 productcode[20];			//产品条形码
	u8 macaddr[18];				//mac地址
	u8 reserved[16];			//预留
}ifly_las_afqq_info;

typedef struct
{
	u8	bUseUTC;
	u8	reserved[16];		//预留
}ifly_utctime_t;

//预置点名称
typedef struct  
{
	u8 chn;		//通道
	u8 preset;	//预置点号  1-128
	char name[12];	//预置点名称
}ifly_presetname_t;

//预置点列表
typedef struct
{
	u8 chn;	//通道
	u8 totalAddedpreset; //已添加的预置点总数 <=128
	struct presetstate_t
	{
		u8 preset;	//预置点号  1-128
		u8 bAdded;	//是否已添加
		char name[12];	//预置点名称
	}presetstate[128]; //预置点状态。
}ifly_preset_list_t;

//云台速度
typedef struct
{
	u8 chn;  //通道
	u8 rate; //速度 16 快速 17常速 18慢速
}ifly_ptz_rate_t;


//恢复默认画面参数　CTRL_CMD_RESETPICADJUST
typedef struct  
{
	u8 chn;  //通道 0开始
	u8 adjtype; //按位表示类型，第1-4位分别表示亮度，对比度，色调，饱和度，置为1的表示恢复默认值， 0xf表示全恢复
}ifly_reset_picadjust_t;

//帧率列表 CTRL_CMD_GETFRAMERATELIST
typedef struct
{
	u8 chn;	//通道
	u8 type;	// 0 主码流 1 子码流
	u8 framerate[10];	//帧率列表 未使用的填0
}ifly_framerate_list_t;

// typedef struct
// {
// 	u8 chn;	//通道
// 	u8 frametype;	// 0 主码流 1 子码流
// 	u8 resolutiontype;	// 见CTRL_GET_RESOLUTION_LIST
// 	u8 framerate[10];	//帧率列表 未使用的填0
// }ifly_frameratelistbyresolution_t;

//主/子码流分辨率列表 CTRL_GET_RESOLUTION_LIST
#define VIDEORESOLU_BEGIN	1
#define VIDEORESOLU_QCIF	1
#define VIDEORESOLU_CIF		2
#define VIDEORESOLU_HD1		3
#define VIDEORESOLU_D1		4
#define VIDEORESOLU_END		VIDEORESOLU_D1
typedef struct  
{
	u8 chn;	//通道
	u8 type;	// 0 主码流 1 子码流
	u8 videoresolu[8];	//分辨率列表 未使用的填0 VIDEORESOLU_BEGIN ~ VIDEORESOLU_END
}ifly_videoresolu_list_t;

typedef struct  
{
	u32 nrserverip; //服务器地址
	u16 serverport; //服务器端口
	u16 databindport; //数据绑定端口
	u8 reserved[16]; //预留
}ifly_nrserver_t; //南瑞服务器

//报警
typedef struct  
{
	u8 alarmid; //alarmid 0开始
	u8  val; //取值 0未触发 1触发
	u8 reserved[2]; //预留
}ifly_alarm_val_t;

typedef struct
{
	ifly_recfileinfo_t fileinfo; //文件信息
	int currindex; //当前第几个文件 1开始计数
	int totalnum; //总共有多少文件
	u32 totalfilesize;	//总文件大小 MB为单位，按1024除
	u8 reserved[16]; //预留
}ifly_dowloadbytime_file_t;

typedef struct
{
	//0-信号量报警,1-硬盘满,2-信号丢失,3－移动侦测,4－硬盘未格式化,
	//5-读写硬盘出错,6-遮挡报警,7-制式不匹配, 8-非法访问
	u8	type;		
	u8	state;			//1报警 2恢复
	u8	id;				//通道,硬盘号,报警输入号,取决于type 
	u16	reserved1;		//预留
	u32	reserved2;		//预留	
}ifly_alarmuploadstate_t;

//特殊设备信息CTRL_CMD_GET_SPECIALDEVICEINFO
typedef struct
{
	u8 mainbordmode; //主板型号 预留
	u8 characterset; //字符集: 0-ascii，1-GB2312, 2-UTF8, 3-UTF16 
	u8 reserved[62]; //预留
}ifly_specialinfo_t;


//CTRL_CMD_GET_CREAROSVR_INFO	//获得创世平台信息
//CTRL_CMD_SET_CREAROSVR_INFO	//设置创世平台信息
typedef struct  
{
	u8   flagPlatform;	//是否启用平台 0不启用 1启用
	u32  ipPlatform;	//平台地址
	u16  portPlatform;	//平台端口
	char PUID[19];
	char passwd[32];	//接入密码
	u8	 reserved[32];	//预留
}ifly_crearo_svr_info;


//CTRL_CMD_GET_HARDWAREVERSION
typedef struct  
{
	char	hardwareModel[64];		//硬件型号
	char	hardwareVersion[64];	//硬件版本
	u8		reserved[64];			//预留
}ifly_hardware_info;


//CTRL_CMD_GET_RECFILE_LIMIT_PARAM 获得录像文件限制参数
//CTRL_CMD_SET_RECFILE_LIMIT_PARAM 设置录像文件限制参数
typedef struct  
{
	u16	savedays;		//文件保留天数
	u32	maxsize;		//文件最大大小
	u32 maxtimelen;		//文件最长时间
	u8	reserved[16];	//预留
}ifly_recfile_limit_param;

//#define CTRL_CMD_MAKE_KEYFRAME			CTRL_CMD_BASE+113		//强制主/子码流产生一个关键帧
typedef struct
{
	u8 chn;			//通道 0开始
	u8 type;		// 0 主码流 1子码流
	u8 reserved[6];	//预留
}ifly_makekeyframe_t;  // 8 Byte

typedef struct
{
	char	ddns_user[64];				//ddns用户名
	char	ddns_passwd[64];			//ddns密码
	char	ddns_domain[64];			//ddns域名
	u8		reserved[16];		//预留
}ifly_ddnsinfo_t;

//报警输入布防
typedef struct
{
	u8 chn; //通道
	u8 weekday; //星期
	struct AlarmTimeField_t
    {
		u32 starttime; //起始时间
		u32 endtime; //终止时间
        u8  flag_alarm; //报警输入
		u8 reserved[6]; //保留字段
    }TimeFiled[4];
    u8 copy2Weekday; //复制到其他天  按位 
    u32 copy2Chnmask; //复制到其他通道。按位
	u8 reserved[16]; //保留字段
}ifly_AlarmSCH_t;

typedef struct  
{
	u8	mdsenselist[12];	//移动侦测灵敏度列表
}ifly_MDSenselist_t;

typedef struct  
{
	u16	mdalarmdelaylist[16];	//移动侦测报警输出延时列表
}ifly_MDAlarmDelaylist_t;

typedef struct  
{
	//csp modify
	//u16 baudratelist[16];	//波特率列表
	u32 baudratelist[16];	//波特率列表
}ifly_BaudRateList_t;

typedef struct  
{
	u8  maxNum;
	u8	baudratelist[19];	//云台协议列表
}ifly_PTZProtocolList_t;

typedef struct  
{
	u8 chn; //通道
	u8 weekday; //星期
	u8 type;
	struct RecSchTimeField_t
    {
		u32 starttime; //起始时间
		u32 endtime; //终止时间
		u8 reserved[6]; //保留字段
    }TimeFiled[24];
    u8 copy2Weekday; //复制到其他天  按位 
    u32 copy2Chnmask; //复制到其他通道。按位
	u8 reserved[16]; //保留字段
}ifly_RecSchTime_t;

typedef struct  
{
	u32 admin_host;             //现在做会纳3g数据服务器ip地址
	u16 ctrl_port;            //会纳登陆端口
	u16 data_port;            //会纳数据端口
	u8   server_enable_flag;  //心跳开启使能
	u16   heartbeat_time;	 //心跳周期
	char   huina_http_ulr[64];			 //会纳http地址
	char   device_flag[16];				 //设备编号
	char   shop_num[16];				 //会纳店铺编号
	u8 reserved[5];
}ifly_HuiNaInfo_t;

//csp modify
typedef struct
{
	u32 reboot_time; // 设备重启时间
	u8 reserve[4];
}ifly_reboottime_t;

//csp modify
typedef struct
{
	u16 alarm_timeinterval; //s为单位，报警上传时间间隔
	u8 reserve[2];
}ifly_uploadalarmtimeinterval_t;

typedef struct
{
	char DDNSlist[20][20];
}ifly_DDNSList_t;

//csp modify 20130423
typedef struct
{
	u32	id;
	u8	chn;
	u8	wnd_num;
	u8	wnd_index;
	u8	reserved[9];
}ifly_wndinfo_t;

//csp modify 20130519
typedef struct
{
	char	username[12];				//用户名
	char	reserved1[4];
	u64		nRemoteView[1];				//远程预览权限
	u64		reserved2[29];
}ifly_AdvPrivilege_t;
#pragma pack( pop )

typedef u16 (*pMESSAGE_CB)(CPHandle cph,u16 event,u8 *pbyMsgBuf,int msgLen,u8 *pbyAckBuf,int *pAckLen,void* pContext);
void SetMsgCallBack(pMESSAGE_CB pCB,void *pContext);

//自动搜索设备
//sever端：设置设备信息
void SetDeviceInfo(ifly_DeviceInfo_t deviceinfo);

//client端：注册搜索回调函数
typedef void (*pFindDeivce_CB)(ifly_DeviceInfo_t deviceinfo, void* pContext);
void SetFindDeviceCB(pFindDeivce_CB pCB, void *pContext);
//client端：开始搜索
int SearchDevice();

//广播设备，同步轮巡
int PatrolSync();


//sever端：注册码流连接回调函数
//当有码流传输的连接时，在回调里进行处理
//pAddStreamTCPLink： 成功时返回0，否则-1
typedef int (*pAddStreamTCPLink)(SOCKHANDLE hSock, ifly_TCP_Stream_Req req, void* pContext);
void SetAddStreamLinkCB(pAddStreamTCPLink pCB, void *pContext);

//发送tcp连接头，用于区别是信令还是码流传输
int SendTcpConnHead(SOCKHANDLE hSock, u8 type);

void SetCliAckPort(u16 wPort);

u16  CPLibInit(u16 wPort);
u16  CPLibCleanup();

/************************
**07-08-21 verifying connecting req from custorm
*************************/
CPHandle CurConnect_out(u32 i);
//连接服务器
CPHandle CPConnect(u32 dwServerIp, u16 wServerPort, u32 dwTimeOut, u16 *pwErrorCode);
//清处连接
u16  CleanCPHandle(CPHandle cph);
//发送事件通知
u16  CPPost(CPHandle cph, u16 event, const void *content, int length);
//发送命令
u16  CPSend(CPHandle cph, u16 event, const void *content, int length, void* ackbuf, int ackbuflen, int *realacklen, u32 dwTimeOut);

//得到TCP连接的本地ip地址与端口
BOOL GetCPHandleLocalAddr(CPHandle cph, struct sockaddr_in *pAddr);

//发送NAT探测包
BOOL SendSTUNPacket(u32 dwLocalIpAddr, u16 wLocalPort, u32 dwRemoteIpAddr, u16 wRemotePort, u8 *pbyBuf, u32 dwBufLen);

u16  GetTransactionNum();



/***************************************************************************************************************/
//发送交互命令给主机，并返回控制处理的最终结果
//交互过程为：
//	1.client端将要发送的参数按特定的格式打包，然后调用本函数
//	2.server端接收到请求后，进行相应的处理
//	3.server根据处理结果返回对应的信息给client，并关闭socket 连接！

//输入参数：
//	dwHostIp：主机的IP地址
//	dwTimeOut：交互的超时时间
//	pBuf：发送的数据头及真正的数据信息，例如网络监控中，pBuf = ifly_cp_header_t + ifly_monitor_param_t
//	dwBufLen：pBuf的长度
//返回值：
//	0：调用成功
//	其它：调用失败
/***************************************************************************************************************/
u16  SendToHost(u32 dwHostIp, u32 dwTimeOut, void *pBuf, u32 dwBufLen);

/***************************************************************************************************************/
//发送交互命令给主机，并需要从主机获得额外的信息
//交互过程为：
//	1.client端将要发送的参数按特定的格式打包，然后调用本函数
//	2.server端接收到请求后，进行相应的处理
//	3.server根据处理结果返回对应的信息给client，并关闭socket 连接！

//输入参数：
//	dwHostIp：主机的IP地址
//	dwTimeOut：交互的超时时间
//	pinBuf：发送的数据头及真正的数据信息，例如网络监控中，pinBuf = ifly_cp_header_t + ifly_monitor_param_t
//	dwinBufLen：pinBuf的长度
//	dwoutBufLen：poutBuf最大长度，防止内存溢出
//输出参数：
//	poutBuf：从主机接收的数据头及真正的数据信息，例如远程回放查询中，poutBuf = ifly_cp_header_t + 查询的结果
//	dwoutBufLen：实际接收到的buf的长度
//返回值：
//	0：调用成功
//	其它：调用失败
//注意：
//	主机应根据输入的dwoutBufLen来控制返回给client的最大数据，否则可能导致数据溢出
/***************************************************************************************************************/
u16  SendToHost2(u32 dwHostIp, u32 dwTimeOut, void *pinBuf, u32 dwinBufLen, void *poutBuf, u32 *dwoutBufLen);



#ifdef __cplusplus
}
#endif

#endif
