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

//��������
#define CTRL_CMD_BASE					10000
#define CTRL_CMD_LOGIN					CTRL_CMD_BASE+1			//Զ�̵�¼����
#define CTRL_CMD_LOGOFF					CTRL_CMD_BASE+2			//ע���û���¼
#define CTRL_CMD_GETDEVICEINFO			CTRL_CMD_BASE+3			//����豸��Ϣ
#define CTRL_CMD_GETVIDEOPROPERTY		CTRL_CMD_BASE+4			//��Ƶ������Ϣ
#define CTRL_CMD_GETAUDIOPROPERTY		CTRL_CMD_BASE+5			//��Ƶ������Ϣ
#define CTRL_CMD_GETVOIPPROPERTY		CTRL_CMD_BASE+6			//voip������Ϣ
#define CTRL_CMD_GETMDPROPERTY			CTRL_CMD_BASE+7			//�ƶ����������Ϣ
#define CTRL_CMD_STOPVIDEOMONITOR		CTRL_CMD_BASE+8			//ֹͣ��ƵԤ��
#define CTRL_CMD_STOPAUDIOMONITOR		CTRL_CMD_BASE+9			//ֹͣ��ƵԤ��
#define CTRL_CMD_STOPVOIP				CTRL_CMD_BASE+10		//ֹͣvoip
#define CTRL_CMD_STOPDOWNLOAD			CTRL_CMD_BASE+11		//ֹͣ����
#define CTRL_CMD_STOPFILEPLAY			CTRL_CMD_BASE+12		//ֹͣ�ļ��ط�
#define CTRL_CMD_STOPTIMEPLAY			CTRL_CMD_BASE+13		//ֹͣʱ��ط�
#define CTRL_CMD_FASTPLAY				CTRL_CMD_BASE+14		//���ٲ���
#define CTRL_CMD_SLOWPLAY				CTRL_CMD_BASE+15		//���ٲ���
#define CTRL_CMD_SETPLAYRATE			CTRL_CMD_BASE+16		//��������
#define CTRL_CMD_PAUSEPLAY				CTRL_CMD_BASE+17		//��ͣ����
#define CTRL_CMD_RESUMEPLAY				CTRL_CMD_BASE+18		//�ָ�����
#define CTRL_CMD_SINGLEPLAY				CTRL_CMD_BASE+19		//֡��
#define CTRL_CMD_FASTBACKPLAY			CTRL_CMD_BASE+20		//����
#define CTRL_CMD_PLAYPREV				CTRL_CMD_BASE+21		//��һ��
#define CTRL_CMD_PLAYNEXT				CTRL_CMD_BASE+22		//��һ��
#define CTRL_CMD_PLAYSEEK				CTRL_CMD_BASE+23		//��λ
#define CTRL_CMD_PLAYMUTE				CTRL_CMD_BASE+24		//�طž���
#define CTRL_CMD_PLAYPROGRESS 			CTRL_CMD_BASE+25		//���Ž���
#define CTRL_CMD_GETNETWORK				CTRL_CMD_BASE+26		//����������
#define CTRL_CMD_SETNETWORK				CTRL_CMD_BASE+27		//�����������
#define CTRL_CMD_GETVGASOLLIST			CTRL_CMD_BASE+28		//���VGA�ֱ���
#define CTRL_CMD_GETSYSPARAM			CTRL_CMD_BASE+29		//���ϵͳ����
#define CTRL_CMD_SETSYSPARAM 			CTRL_CMD_BASE+30		//����ϵͳ����
#define CTRL_CMD_GETRECPARAM			CTRL_CMD_BASE+31		//���¼�����
#define CTRL_CMD_SETRECPARAM 			CTRL_CMD_BASE+32		//����¼�����
#define	CTRL_CMD_GETSUBSTREAMPARAM		CTRL_CMD_BASE+33		//�������������
#define	CTRL_CMD_SETSUBSTREAMPARAM		CTRL_CMD_BASE+34		//��������������
#define CTRL_CMD_GETIMGPARAM			CTRL_CMD_BASE+35		//���ͼ�����
#define CTRL_CMD_SETIMGPARAM 			CTRL_CMD_BASE+36		//����ͼ�����
#define CTRL_CMD_GETPICADJ 				CTRL_CMD_BASE+37		//��û������
#define CTRL_CMD_SETPICADJ 				CTRL_CMD_BASE+38		//���û������
#define CTRL_CMD_GETALARMINPARAM 		CTRL_CMD_BASE+39		//��ñ����������
#define CTRL_CMD_SETALARMINPARAM 		CTRL_CMD_BASE+40		//���ñ����������
#define CTRL_CMD_GETALARMOUTPARAM 		CTRL_CMD_BASE+41		//��ñ����������
#define CTRL_CMD_SETALARMOUTPARAM 		CTRL_CMD_BASE+42		//���ñ����������
#define	CTRL_CMD_GETALARMNOTICYPARAM	CTRL_CMD_BASE+43		//��ñ���֪ͨ����
#define	CTRL_CMD_SETALARMNOTICYPARAM	CTRL_CMD_BASE+44		//���ñ���֪ͨ����
#define CTRL_CMD_GETRECSCHPARAM 		CTRL_CMD_BASE+45		//���¼�񲼷�����
#define CTRL_CMD_SETRECSCHPARAM 		CTRL_CMD_BASE+46		//����¼�񲼷�����
#define CTRL_CMD_GETMDPARAM 			CTRL_CMD_BASE+47		//����ƶ�������
#define CTRL_CMD_SETMDPARAM 			CTRL_CMD_BASE+48		//�����ƶ�������
#define CTRL_CMD_GETVIDEOLOSTPARAM		CTRL_CMD_BASE+49		//�����Ƶ��ʧ����
#define CTRL_CMD_SETVIDEOLOSTPARAM 		CTRL_CMD_BASE+50		//������Ƶ��ʧ����
#define CTRL_CMD_GETVIDEOBLOCKPARAM 	CTRL_CMD_BASE+51		//�����Ƶ�ڵ�����
#define CTRL_CMD_SETVIDEOBLOCKPARAM 	CTRL_CMD_BASE+52		//������Ƶ�ڵ�����
#define CTRL_CMD_GETPTZPARAM			CTRL_CMD_BASE+53		//�����̨����
#define CTRL_CMD_SETPTZPARAM 			CTRL_CMD_BASE+54		//������̨����
#define CTRL_CMD_SETPRESET 				CTRL_CMD_BASE+55		//����Ԥ�õ����
#define CTRL_CMD_GETCRUISEPARAM 		CTRL_CMD_BASE+56		//���Ѳ��·������
#define CTRL_CMD_SETCRUISEPARAM 		CTRL_CMD_BASE+57		//����Ѳ��·������
#define CTRL_CMD_CTRLCRUISEPATH 		CTRL_CMD_BASE+58		//Ѳ��·������
#define CTRL_CMD_CTRLPTZTRACK 			CTRL_CMD_BASE+59		//�켣����
#define CTRL_CMD_GETHDDINFO 			CTRL_CMD_BASE+60		//��ô�����Ϣ
#define CTRL_CMD_GETUSERINFO			CTRL_CMD_BASE+61		//����û���Ϣ
#define CTRL_CMD_SETUSERINFO 			CTRL_CMD_BASE+62		//�����û���Ϣ
#define CTRL_CMD_SETRESTORE				CTRL_CMD_BASE+63		//�ָ�Ĭ��
#define CTRL_CMD_CLEARALARM				CTRL_CMD_BASE+64		//�������
#define CTRL_CMD_GETSYSTIME				CTRL_CMD_BASE+65		//���ϵͳʱ��
#define CTRL_CMD_SETSYSTIME				CTRL_CMD_BASE+66		//����ϵͳʱ��
#define CTRL_CMD_GETSYSINFO				CTRL_CMD_BASE+67		//���ϵͳ��Ϣ
#define CTRL_CMD_SHUTDOWN				CTRL_CMD_BASE+68		//�ر�ϵͳ
#define CTRL_CMD_REBOOT					CTRL_CMD_BASE+69		//����ϵͳ
#define CTRL_CMD_PTZCTRL				CTRL_CMD_BASE+70		//��̨����
#define CTRL_CMD_GETMANUALREC			CTRL_CMD_BASE+71		//����ֶ�¼��״̬
#define CTRL_CMD_SETMANUALREC			CTRL_CMD_BASE+72		//�����ֶ�¼��״̬
#define CTRL_CMD_LOGSEARCH				CTRL_CMD_BASE+73		//��־�ļ�����
#define CTRL_CMD_RECFILESEARCH			CTRL_CMD_BASE+74		//�ط��ļ�����
#define CTRL_CMD_GETSPSPPS				CTRL_CMD_BASE+75		//���spspps����
#define CTRL_CMD_ALARMUPLOADCENTER		CTRL_CMD_BASE+76		//�����ϴ�����
#define CTRL_CMD_CDROM					CTRL_CMD_BASE+77		//��������
#define CTRL_CMD_COM_PROTOCOL_GET		CTRL_CMD_BASE+78		//��ô��ڲ���
#define CTRL_CMD_COM_PROTOCOL_SET		CTRL_CMD_BASE+79		//���ô��ڲ���
#define CTRL_CMD_SERIALSTOP				CTRL_CMD_BASE+80		//ֹͣ͸������
#define CTRL_CMD_GETAUDIOVOLUME			CTRL_CMD_BASE+81		//���ĳͨ��������Ϣ
#define CTRL_CMD_SETAUDIOVOLUME			CTRL_CMD_BASE+82		//����ĳͨ��������Ϣ
#define CTRL_CMD_GETSYSLANGLIST			CTRL_CMD_BASE+83		//���ϵͳ�����б�
#define CTRL_CMD_GETBITRATELIST			CTRL_CMD_BASE+84		//�������λ���б�
#define CTRL_CMD_XWSERVER_GET			CTRL_CMD_BASE+85		//csp modify 20100419
#define CTRL_CMD_XWSERVER_SET			CTRL_CMD_BASE+86		//csp modify 20100419
//#define CTRL_CMD_GETRECFILEINDEX		CTRL_CMD_BASE+87		//csp modify 20100419
#define CTRL_CMD_GETEMAILSMTP			CTRL_CMD_BASE+88		//��ñ���EmailSMTP��Ϣ
#define CTRL_CMD_SETEMAILSMTP			CTRL_CMD_BASE+89		//���ñ���EmailSMTP��Ϣ
#define CTRL_CMD_LASAFQQ_GETINFO		CTRL_CMD_BASE+90		//��������Ӱ���qq��Ϣ
#define CTRL_CMD_LASAFQQ_SETINFO		CTRL_CMD_BASE+91		//���������Ӱ���qq��Ϣ
#define CTRL_CMD_SETMAC					CTRL_CMD_BASE+92		//����MAC��ַ
#define CTRL_CMD_USEUTCTIME				CTRL_CMD_BASE+93		//�Ƿ�ʹ��UTC��ʽʱ��
#define CTRL_CMD_GETPRESETLIST			CTRL_CMD_BASE+94		//���Ԥ�õ��б�
#define CTRL_CMD_ADDPRESET_BYNAME		CTRL_CMD_BASE+95		//���Ԥ�õ㣨���ƣ�
#define CTRL_CMD_GETPTZRATE				CTRL_CMD_BASE+96		//�����̨��ǰ�ٶ�
#define CTRL_CMD_RESETPICADJUST			CTRL_CMD_BASE+97		//�ָ�Ĭ�ϻ������
#define CTRL_CMD_GETFRAMERATELIST		CTRL_CMD_BASE+98		//���֡���б�
#define CTRL_CMD_GETMAX_IMGMASKNUM		CTRL_CMD_BASE+99		//���֧�ֵ�����ڸ���
#define CTRL_GET_RESOLUTION_LIST		CTRL_CMD_BASE+100		//�����/�������ֱ����б�
#define CTRL_CMD_NRSERVER_GET			CTRL_CMD_BASE+101		//��ȡ�������������
#define CTRL_CMD_NRSERVER_SET			CTRL_CMD_BASE+102		//�����������������
#define CTRL_CMD_GET_ALARMIN_VAL		CTRL_CMD_BASE+103       //��ȡ��������ֵ
#define CTRL_CMD_SET_ALARMOUT_VAL		CTRL_CMD_BASE+104       //���ñ������ֵ
#define CTRL_CMD_STOPGETFILEBYTIME		CTRL_CMD_BASE+105		//ֹͣ��ʱ������
#define CTRL_CMD_GETALARMULSTATE		CTRL_CMD_BASE+106		//��ȡ�����ϴ�״̬
#define CTRL_CMD_GET_SPECIALDEVICEINFO	CTRL_CMD_BASE+107		//��������ϵͳ��Ϣ������ĳЩ�ͺŵļ���
#define	CTRL_CMD_GET_CREAROSVR_INFO		CTRL_CMD_BASE+108		//��ô���ƽ̨��Ϣ
#define	CTRL_CMD_SET_CREAROSVR_INFO		CTRL_CMD_BASE+109		//���ô���ƽ̨��Ϣ
#define CTRL_CMD_GET_HARDWAREVERSION	CTRL_CMD_BASE+110		//���Ӳ���汾��Ϣ	
#define	CTRL_CMD_GET_RECFILE_LIMIT_PARAM CTRL_CMD_BASE+111		//���¼���ļ����Ʋ���
#define	CTRL_CMD_SET_RECFILE_LIMIT_PARAM CTRL_CMD_BASE+112		//����¼���ļ����Ʋ���
#define CTRL_CMD_MAKE_KEYFRAME			CTRL_CMD_BASE+113		//ǿ����/����������һ���ؼ�֡
// #define CTRL_CMD_EXTLOGIN				CTRL_CMD_BASE+114		//Զ�̵�¼����(�û������15���ַ�)
// #define CTRL_CMD_EXTLOGOFF				CTRL_CMD_BASE+115		//ע���û���¼(�û������15���ַ�)
// #define CTRL_CMD_EXTGETUSERINFO			CTRL_CMD_BASE+116		//����û���Ϣ(�û������15���ַ�)
// #define CTRL_CMD_EXTSETUSERINFO 			CTRL_CMD_BASE+117		//�����û���Ϣ(�û������15���ַ�)
#define	CTRL_CMD_REGISTDDNS				CTRL_CMD_BASE+118		//ע��DDNS
#define CTRL_CMD_CANCELDDNS				CTRL_CMD_BASE+119		//ע��DDNS
#define CTRL_CMD_GETALARMSCHPARAM		CTRL_CMD_BASE+120		//��ñ������벼������
#define CTRL_CMD_SETALARMSCHPARAM		CTRL_CMD_BASE+121		//���ñ������벼������
// #define CTRL_CMD_SEARCHWIFIINFO			CTRL_CMD_BASE+122		//����WIFI
// #define CTRL_CMD_ENABLEWIFI				CTRL_CMD_BASE+123		//����WIFI
// #define CTRL_CMD_DISCONNECTWIFI			CTRL_CMD_BASE+124		//�Ͽ�WIFI
// #define CTRL_CMD_GETPORTINFO			CTRL_CMD_BASE+125		//
// #define CTRL_CMD_SETPORTINFO			CTRL_CMD_BASE+126		//
// #define CTRL_CMD_GETHDMIINFO			CTRL_CMD_BASE+127		//
// #define CTRL_CMD_SETHDMIINFO			CTRL_CMD_BASE+128		//
#define CTRL_CMD_GETMDSENSELIST			CTRL_CMD_BASE+129		//��ȡ�ƶ�����������б�
#define CTRL_CMD_GETMDALARMDELAYLIST 	CTRL_CMD_BASE+130		//��ȡ�ƶ���ⱨ�������ʱ�б�
#define CTRL_CMD_GETBAUDRATELIST		CTRL_CMD_BASE+131		//��ȡ�������б�
#define CTRL_CMD_GETPTZPROTOCOLLIST		CTRL_CMD_BASE+132		//��ȡ��̨Э���б�
//#define CTRL_GETFRAMERATELIST_BYRESOLUTION	CTRL_CMD_BASE+133		//�����/�Ӷ�Ӧ�ֱ��ʵ�֡���б�
#define CTRL_CMD_GETRECSCHPARAMBYTYPE 	CTRL_CMD_BASE+133		//���������ͻ��¼�񲼷�����
#define CTRL_CMD_SETRECSCHPARAMBYTYPE	CTRL_CMD_BASE+134		//���������ͷֿ�����¼�񲼷�����
#define CTRL_CMD_GETHUINAINFO			CTRL_CMD_BASE+135		//��ȡ������Ϣ
#define CTRL_CMD_SETHUINAINFO			CTRL_CMD_BASE+136		//���û�����Ϣ
// #define CTRL_CMD_GETNONETWORKPARAM 		CTRL_CMD_BASE+137		//������粻ͨ����
// #define CTRL_CMD_SETNONETWORKPARAM 		CTRL_CMD_BASE+138		//�������粻ͨ����

//csp modify
#define CTRL_CMD_GETREBOOTTIME			CTRL_CMD_BASE+145		//��ȡ����ʱ��
#define CTRL_CMD_GETUPLOADALARMTIME		CTRL_CMD_BASE+150		//��ȡ�����ϴ�ʱ����
#define CTRL_CMD_SETUPLOADALARMTIME		CTRL_CMD_BASE+151		//���ñ����ϴ�ʱ����

#define CTRL_CMD_GETDDNSLIST			CTRL_CMD_BASE+152		//��ȡDDNS�������б�

//csp modify 20130423
#define CTRL_CMD_SETMONITORINFO			CTRL_CMD_BASE+160		//����Զ�̼����Ϣ

//csp modify 20130519
#define CTRL_CMD_GETADVPRIVILEGE		CTRL_CMD_BASE+161		//��ȡ�û��߼�Ȩ��
//xdc
#define CTRL_CMD_GETSEACHIPCLIST		CTRL_CMD_BASE+162		//��ȡ����������ͷ�б�
#define CTRL_CMD_GETADDIPCLIST			CTRL_CMD_BASE+163		//��ȡ����ӵ�����ͷ�б�
#define CTRL_CMD_SETIPC					CTRL_CMD_BASE+164			//���ö�Ӧͨ������ͷ
#define CTRL_CMD_ADDIPC					CTRL_CMD_BASE+165			//�������ͷ
#define CTRL_CMD_DELETEIPC				CTRL_CMD_BASE+166			//ɾ������ͷ
#define CTRL_CMD_GETTHEOTHER			CTRL_CMD_BASE+167		//��ȡ��������ʣ�µ�����ͷ�б�
//yaogang modify 20141030
#define CTRL_CMD_GET_PATROL_PARA		CTRL_CMD_BASE+168		//��ȡ��Ѳ����
#define CTRL_CMD_SET_PATROL_PARA		CTRL_CMD_BASE+169		//������Ѳ����
#define CTRL_CMD_GET_PREVIEW_PARA		CTRL_CMD_BASE+170		//��ȡԤ������
#define CTRL_CMD_SET_PREVIEW_PARA		CTRL_CMD_BASE+171		//����Ԥ������
#define CTRL_CMD_GET_CHN_PARA			CTRL_CMD_BASE+172		//��ȡͨ������
#define CTRL_CMD_SET_CHN_PARA			CTRL_CMD_BASE+173		//����ͨ������
#define CTRL_CMD_CLEAN_ALARM_ICON		CTRL_CMD_BASE+174		//������汨�����Ǳ�
#define CTRL_CMD_CLOSE_GUIDE			CTRL_CMD_BASE+175		//Ծ������CMS remote CTRL

//yaogang modify 20160122
#define CTRL_CMD_GET_IPCCHN_LINKSTATUS	CTRL_CMD_BASE+176		//��ȡIPCͨ������״̬(һ��IPC������ͨ��������������)

//yaogang modify 20170715 ��������ͨ�����Ľӿ�
#define CTRL_CMD_GET_CHN_NAME	CTRL_CMD_BASE+177
#define CTRL_CMD_SET_CHN_NAME	CTRL_CMD_BASE+178



//Э��
#define CTRL_PROTOCOL_SERVERPORT		6630					//�������˿� 6610
#define CTRL_PROTOCOL_MAXLINKNUM		256
#define CTRL_PROTOCOL_CONNECT_BLOCK		INFINITE				//���ӷ�ʽ:����
#define CTRL_PROTOCOL_CONNECT_DEFAULT	3000					//ȱʡ����ʱ��:3��
#define CTRL_VERSION					0x0100
#define CTRL_COMMAND					0
#define CTRL_NOTIFY						1
#define CTRL_ACK						2

//�¼�֪ͨ����
#define CTRL_NOTIFY_BASE				20000
#define	CTRL_NOTIFY_CONNLOST			CTRL_NOTIFY_BASE+0		//������Ϣ
#define	CTRL_NOTIFY_HEARTBEAT_REQ		CTRL_NOTIFY_BASE+1		//��������
#define	CTRL_NOTIFY_HEARTBEAT_RESP		CTRL_NOTIFY_BASE+2		//�����ظ�
#define CTRL_NOTIFY_PLAYEND				CTRL_NOTIFY_BASE+3		//�������
#define CTRL_NOTIFY_PLAYPROGRESS		CTRL_NOTIFY_BASE+4		//�������֪ͨ
#define CTRL_NOTIFY_HASAUDIO			CTRL_NOTIFY_BASE+5		//�ط��ļ��Ƿ�����Ƶ
#define CTRL_NOTIFY_UPDATEPROGRESS		CTRL_NOTIFY_BASE+6		//��������
#define CTRL_NOTIFY_ALARMINFO			CTRL_NOTIFY_BASE+7		//�첽������Ϣ

//Ӧ������
#define CTRL_SUCCESS					0						//�ɹ�
#define CTRL_FAILED_BASE				30000					//������ƫ����
#define CTRL_FAILED_USER				CTRL_FAILED_BASE+1		//�����ڵ��û���
#define CTRL_FAILED_PASSWORD			CTRL_FAILED_BASE+2		//�������
#define CTRL_FAILED_COMMAND				CTRL_FAILED_BASE+3		//δ�Ͽɵ�����
#define CTRL_FAILED_PARAM				CTRL_FAILED_BASE+4		//��Ч����
#define CTRL_FAILED_OUTOFMEMORY			CTRL_FAILED_BASE+5		//�ڴ治��
#define CTRL_FAILED_RESOURCE			CTRL_FAILED_BASE+6		//��Դ����
#define CTRL_FAILED_FILENOTEXIST		CTRL_FAILED_BASE+7		//�ļ�������
#define CTRL_FAILED_DATABASE			CTRL_FAILED_BASE+8		//���ݿ����
#define CTRL_FAILED_RELOGIN				CTRL_FAILED_BASE+9		//�ظ���¼
#define CTRL_FAILED_BAUDLIMIT			CTRL_FAILED_BASE+10		//����ÿһ·ͨ�����֧��ʵʱ���			
#define CTRL_FAILED_CREATESOCKET		CTRL_FAILED_BASE+11		//�����׽���ʧ��
#define CTRL_FAILED_CONNECT				CTRL_FAILED_BASE+12		//��������ʧ��
#define CTRL_FAILED_BIND				CTRL_FAILED_BASE+13		//��ʧ��
#define CTRL_FAILED_LISTEN				CTRL_FAILED_BASE+14		//����ʧ��
#define CTRL_FAILED_NETSND				CTRL_FAILED_BASE+15		//���緢�ͳ���
#define CTRL_FAILED_NETRCV				CTRL_FAILED_BASE+16		//������ճ���
#define CTRL_FAILED_TIMEOUT				CTRL_FAILED_BASE+17		//�������ӳ�ʱ
#define CTRL_FAILED_CHNERROR			CTRL_FAILED_BASE+18		//����ͨ������
#define CTRL_FAILED_DEVICEBUSY			CTRL_FAILED_BASE+19		//�豸����æ
#define CTRL_FAILED_WRITEFLASH			CTRL_FAILED_BASE+20		//��дflash����
#define CTRL_FAILED_VERIFY				CTRL_FAILED_BASE+21		//У���
#define CTRL_FAILED_CONFLICT			CTRL_FAILED_BASE+22		//ϵͳ��Դ��ͻ
#define CTRL_FAILED_BUSY				CTRL_FAILED_BASE+23		//ϵͳ����æ
#define CTRL_FAILED_LINKLIMIT			CTRL_FAILED_BASE+24		//�Ѵﵽ��������
#define CTRL_FAILED_USER_SAME			CTRL_FAILED_BASE+25		//�û�����ͬ07-08-02
#define CTRL_FAILED_MACADDR				CTRL_FAILED_BASE+26		//Զ�̷��ʵ�pc mac��ַ����
#define CTRL_FAILED_NOINIT				CTRL_FAILED_BASE+27		//ģ��δ��ʼ��
#define CTRL_FAILED_USER_MAX			CTRL_FAILED_BASE+28		//�û������//wrchen 080529
#define CTRL_FAILED_UNKNOWN				CTRL_FAILED_BASE+9999	//δ֪����
//����
#define CTRL_CONNECTION_NULL			0x0
#define CTRL_CONNECTION_TCPCLIENT		0x1
#define CTRL_CONNECTION_TCPSERVER		0x2

//�鲥�����豸
#define CTRL_DEVICESEARCH_ACKCLIENT		0X1
#define CTRL_DEVICESEARCH_ACKSERVER		0x2
#define SEARCHPORT						6666
#define ACKSEARCHPORT					SEARCHPORT+1
#define MULTICASTGROUP					"224.0.1.2"

//��������
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
	time_t last_msg_time;//�ϴ�ͨ��ʱ�䣬�����������Ӧ������
}ifly_cp_t,*CPHandle;

typedef struct
{
	u32		length;						//��Ϣ����		
	u16		type;						//��Ϣ����		
	u16		event;						//��Ϣ��		
	u16		number;						//��ˮ��		
	u16		version;					//�汾��		
}ifly_cp_header_t;

//Э��ͷ�ṹ
typedef struct 
{
	u8	safeguid[16];			// PROTOGUID
	u8  protocolver;			//Э��汾
	u8	byConnType;			//�������ͣ�0x1���������0x2���������䣻0x3���㲥������0x4  ��Ѳͬ��(ֻ���Ѿ�ʹ����Ѳ���豸��Ч)
	u8	reserved[2];			//�����ֶ�
}ifly_ProtocolHead_t;


//�豸��Ϣ
typedef struct 
{
	u32	deviceIP; 						//�豸IP  
	u16	devicePort;						//�豸�˿� 
	char device_name[32];				//�豸����
	char device_mode[32];				//�豸�ͺ�
	u8	maxChnNum;						//���ͨ����
	u8	maxAduioNum;					//�����Ƶ��
	u8	maxSubstreamNum;				//�����������
	u8	maxPlaybackNum;					//���ط���
	u8	maxAlarmInNum;					//��󱨾�������
	u8	maxAlarmOutNum;					//��󱨾������
	u8	maxHddNum;						//���Ӳ����
	u8	nNVROrDecoder;	//����NVR�ͽ�����--- Ծ��
	u8	reserved[15];					//Ԥ��
}ifly_DeviceInfo_t;


//��Ƶ����
typedef struct  
{
	u8	videoEncType;					//��Ƶ��������
	u16	max_videowidth;					//
	u16	max_videoheight;					
	u8	reserved[3];					//Ԥ��
}ifly_Video_Property_t;

//��Ƶ����
typedef struct  
{
	u8	audioEnctype;					//Ԥ����Ƶ��������
	u8	audioBitPerSample;				//Ԥ����Ƶλ��
	u16 audioSamplePerSec;				//Ԥ����Ƶ������
	u16 audioFrameSize;					//Ԥ����Ƶÿ֡��С
	u16	audioFrameDurTime;				//Ԥ����Ƶÿ֡���
	u8	reserved[4];					//Ԥ��
}ifly_Audio_Property_t;

//VOIP����
typedef struct  
{
	u8	VOIPBitPerSample;				//�����Խ�λ��
	u16 VOIPSamplePerSec;				//�����Խ�������
	u16 VOIPFrameSize;					//�����Խ�ÿ֡��С
	u8	reserved[3];					//Ԥ��
}ifly_VOIP_Property_t;

//�ƶ��������
typedef struct  
{
	u8	MDCol;							//�ƶ��������
	u8	MDRow;							//�ƶ��������
	u8	reserved[4];					//Ԥ��
}ifly_MD_Property_t;


//������������ṹ
typedef struct 
{
	u8 command;							//0��Ԥ�� 1���ļ��ط� 2��ʱ��ط� 3���ļ����� 4������ 
										//5 VOIP 6 �ļ���֡���� 7 ʱ�䰴֡���� 8 ͸��ͨ��
										//9 Զ�̸�ʽ��Ӳ�� 10 ������ץͼ 11 ��·��ʱ��ط� 12 ��ʱ�������ļ�
	union		//72byte
	{
		struct
		{
			u8		chn;				//Ԥ��ͨ��
			u8		type;				//0:��������Ƶ 1����������Ƶ 2����������Ƶ
		}Monitor_t;						//Ԥ�� command = 0
		
		struct
		{
			char	filename[64];		//�طŵ��ļ�
			u32		offset;				//�ļ�ƫ��
		}FilePlayBack_t; 				//�ļ��ط� command = 1,�� command = 6
		
		struct
		{
			u8		channel;			//ͨ����
			u16		type;				//����
			u32		start_time;			//��ʼʱ��
			u32		end_time;			//��ֹʱ��
		}TimePlayBack_t ;				//ʱ��ط� command = 2,�� command = 7
		
		struct
		{
			char	filename[64];		//���ص��ļ�
			u32		offset;				//�ļ�ƫ��
			u32		size;				//�ļ���С
		}FileDownLoad_t;				//�ļ����� command = 3
		
		struct
		{
			u32		size;				//�ļ�����
			u32		verify;				//У���
			u16		version;			//�ļ��汾��
			u16		updateType;			//0:�������� 1���������
		}Update_t;						//Զ������  command = 4��
		
		//VOIP command
		u8 voipreserved;				//VOIPԤ�� command = 5��

		//͸��ͨ��
		u8 SerialPort;					//͸��ͨ�� 1-232����  2-485����	command = 8��

		//Զ�̸�ʽ��Ӳ��
		u8 formatHddIndex;				//Ҫ��ʽ����Ӳ�̺� 0��ʼ

		struct
		{
			u16		chnMask;			//ͨ���� ��λ ��ָͨ��1-16
			u16		type;				//����
			u32		start_time;			//��ʼʱ��
			u32		end_time;			//��ֹʱ��
			u16		chn17to32Mask;		//ͨ��17-32 ��λ
		}MultiTimePlayBack_t ;			//��·ʱ��ط� command = 11��12
	};
	u8	reserved[7];					//Ԥ��	
}ifly_TCP_Stream_Req;					 


//�������������˻�Ӧ�ṹ
typedef struct
{
	u32	errcode;						//���ӳɹ�����0�� ���򷵻�����������
	u32	ackid;							//�����ļ��طţ�ʱ��طš� ���ں����ͻ��˿��ƻطŲ��������ֽ���, Ԥ������
}ifly_TCP_Stream_Ack;

typedef struct  
{
	u16 errcode;
	u8 pos;
}ifly_TCP_Pos;

//Ԥ�����طź� VOIP֡�ṹ
typedef struct
{
	u8	m_byMediaType;					//����
	u8	m_byFrameRate;					//֡��
	u8	m_bKeyFrame;					//�Ƿ�ؼ�֡
	int m_nVideoWidth;					//��
	int m_nVideoHeight;					//��
	u32 m_dwDataSize;					//���ݴ�С
	u32 m_dwFrameID;					//֡ID
	u32 m_dwTimeStamp;					//ʱ���
}ifly_MediaFRAMEHDR_t;

//��¼
typedef struct
{
	char	username[12];				//�û���
	char	loginpass[12];				//����
	char	macAddr[18];				//MAC��ַ		
	u32		ipAddr;						//IP��ַ
}ifly_loginpara_t;

//�������
typedef struct
{
	char	mac_address[18];			//mac��ַ
	u32		ip_address;					//ip��ַ
	u16		server_port;				//�豸�˿�
	u32		net_mask;					//����
	u32		net_gateway;				//����
	u32		dns;						//dns
	u8		flag_multicast;				//�鲥���ñ��
	u32		multicast_address;			//�鲥��ַ
	u16		http_port;					//http�˿�
	u8		flag_pppoe;					//pppoe���ñ��
	char	pppoe_user_name[64];		//pppoe�û���
	char	pppoe_passwd[64];			//pppoe����
	u8      flag_dhcp;					//dhcp���ñ�־
	u8		ddnsserver;					//ddns������
	u8		flag_ddns;					//ddns���ñ�־
	char	ddns_domain[64];			//ddns����
	char	ddns_user[64];				//ddns�û���
	char	ddns_passwd[64];			//ddns����
	u32		centerhost_address;			//���ķ�������ַ
	u16		centerhost_port;			//���ķ������˿�
	u16		mobile_port;				//�ֻ���ض˿�
	char	hwid[12];					//�����Ӷ���
	u8		reserved[2];				//�����ֶ�
}ifly_NetWork_t;

//�ֱ������֧��16�ֲ�֧�ֵ�����0
typedef struct
{
	u16		width;						//�ֱ��ʿ�
	u16		height;						//�ֱ��ʸ�
	u8		flashrate;					//�ֱ���ˢ����
}ifly_VGA_Pro;

typedef struct 
{
	ifly_VGA_Pro vgapro[16];
}ifly_VGA_Solution;

//ϵͳ����
typedef struct
{
	u16		device_id;					//�豸id
	char	device_name[32];			//�豸����
	u8		flag_overwrite;				//Ӳ����ʱ���Ǳ��
	u16		lock_time;					//��������ʱ��			
	u16		switch_time;				//�л�ʱ��
	u8		flag_statusdisp;			//״̬��ʾ���
	u8		video_format;				//��Ƶ��ʽ					
	u8		vga_solution;				//VGA�ֱ��� ifly_VGA_Solution[16];
	u8		transparency;				//�˵�͸����
	u8		language;					//ϵͳ����
	u8		disable_pw;					//�������
	u8		flag_bestrow;				//24Сʱ����
	u8		reserved[15];				//�����ֶ�
}ifly_SysParam_t;

//¼�����
typedef struct									
{
	u8		channelno;					//ͨ����
	u8		code_type;					//��������
	u8		bit_type;					//λ������
	u32		bit_max;					//λ������
	u16		intraRate;					//�ؼ�֡���
	u8		qi;							//�ؼ�֡��������
	u8		minQ;						//��С��������
	u8		maxQ;						//�����������
	u8		quality;					//ͼ������
	u8		frame_rate;					//��Ƶ֡��
	u16		pre_record;					//Ԥ¼ʱ��
	u16		post_record;				//¼����ʱ
	u32		copy2chnmask;				//���Ƶ�����ͨ����ÿһλһ��ͨ��
	u8		supportdeinter;				//�Ƿ�֧��deinter���� 1�� 0�� (����)
	u8		deinterval;					//deinterǿ�� 0-4 ���ã������У�ǿ����ǿ
	u8		supportResolu;				//�Ƿ�֧������¼��ֱ���
	u8		resolutionpos;				//�ֱ���ѡ��ֵ
	u8		reserved[12];				//�����ֶ�
}ifly_RecordParam_t;

//����������
typedef struct									
{
	u8		channelno;					//ͨ����
	u8		sub_flag;					//���ֶ������������� 0 cif 1 qcif
	u8		sub_bit_type;				//������λ������
	u16		sub_intraRate;				//�ؼ�֡���
	u8		sub_qi;						//�ؼ�֡��������
	u8		sub_minQ;					//��С��������
	u8		sub_maxQ;					//�����������
	u8		sub_quality;				//������ͼ������
	u8 		sub_framerate;				//��������֡��
	int 	sub_bitrate;				//��������λ��
	u32		copy2chnmask;				//���Ƶ�����ͨ����ÿһλһ��ͨ��
	u8		reserved[16];				//�����ֶ�
}ifly_SubStreamParam_t;

//ͼ�����
typedef struct
{
	u8		channel_no;					//ͨ����
	char	channelname[128];//32		//ͨ����yaogang modify 20171121 ���泤��ͨ����
	u8		flag_name;					//����λ����ʾ
	u16		chnpos_x;					//����x����
	u16		chnpos_y;					//����y����
	u8		flag_time;					//ʱ��λ����ʾ
	u16		timepos_x;					//ʱ��x����
	u16		timepos_y;					//ʱ��y����
	u8		flag_mask;					//�ڸ�
	struct  Mask_t
	{
		u16	 	x;
		u16		y;
		u16		width;
		u16		height;
	}MaskInfo[4];
	u8		flag_safechn;				//��ȫͨ�����
	u32		copy2chnmask;				//���Ƶ�����ͨ����ÿһλһ��ͨ��
	u8		reserved[16];				//�����ֶ�
}ifly_ImgParam_t;


//�������
typedef	struct  
{
	u8		channel_no;					//ͨ����
	u8		flag;						//���ڱ�־:0-3
	u8		val;						//����ֵ
	u32		copy2chnmask;				//���Ƶ�����ͨ����ÿһλһ��ͨ��
}ifly_PicAdjust_t;

//�����������
typedef struct									
{
	u8		inid;						//���������� 
	u8		typein;						//������������
	u8		flag_deal;					//�Ƿ񴥷��������봦��
	u32		triRecChn;					//����¼��ͨ����ÿһλһͨ��
	u32		triAlarmoutid;				//���������������λ
	u8		flag_buzz;					//����������
	u8		flag_email;					//����emaill
	u8		flag_mobile;				//�����ֻ�����
	u16		delay;						//���������ʱ
	struct  AlarmInPtz_t				//����ptz
	{
		u8		flag_preset;			//Ԥ�õ�
		u8		preset;
		u8		flag_cruise;			//Ѳ����
		u8		cruise;
		u8		flag_track;				//�켣
	} AlarmInPtz [32];
	u32		copy2AlarmInmask;			//���Ƶ������������롣��λ
	u8		flag_enablealarm;			//�������ñ�־
	u8		reserved[15];				//�����ֶ�
}ifly_AlarmInParam_t;

//�����������
typedef struct									
{
	u8		outid;						//��������� 
	u8		typeout;					//�����������
	u32		copy2AlarmOutmask;			//���Ƶ����������������λ
#if 1//csp modify
	u16		alarmoutdelay;				//���������ʱ
	u8		flag_buzz;					//����������
	u16		buzzdelay;					//��������ʱ
	u8		reserved[11];
#else
	u8		reserved[16];				//�����ֶ�
#endif
}ifly_AlarmOutParam_t;

//����֪ͨ����: email�����ֻ�����
typedef struct									
{

	char	alarm_email[32];			//����email��ַ
	char	alarm_mobile[32];			//�����ֻ���ַ
	u8		reserved[32];				//�����ֶ�
}ifly_AlarmNoticeParam_t;

//¼�񲼷�
typedef struct
{
	u8		chn;						//ͨ��
	u8		weekday;					//����
	struct RecTimeField_t
	{
		u32 starttime;					//��ʼʱ��
		u32 endtime;					//��ֹʱ��
		u8	flag_sch;					//��ʱ¼��
		u8 	flag_md;					//�ƶ����¼��
		u8  flag_alarm;					//����¼��
		u8	reserved[4];				//�����ֶ�
	}TimeFiled[4];
	u8		copy2Weekday;				//���Ƶ�������  ��λ 
	u32		copy2Chnmask;				//���Ƶ�����ͨ������λ
	u8		reserved[16];				//�����ֶ�
}ifly_RecordSCH_t;

//�ƶ����
typedef struct
{
	u8 		chn;						//ͨ��
	u32 	trigRecChn;					//����¼��ͨ�� ��λ
	u32 	trigAlarmOut;				//����������� ��λ
	u8 		flag_buzz;					//������
	u8		flag_email;					//����emaill
	u8		flag_mobile;				//�����ֻ�����
	u16 	delay;						//��ʱ
	u8		sense;						//������
	u8		block[44*36];				//����
	u32		copy2Chnmask;				//���Ƶ�����ͨ������λ
	u8		reserved[12];				//�����ֶ�
}ifly_MDParam_t;

//��Ƶ��ʧ
typedef struct
{
	u8 		chn;						//ͨ��
	u32 	trigRecChn;					//����¼��ͨ�� ��λ
	u32 	trigAlarmOut;				//����������� ��λ
	u8 		flag_buzz;					//������
	u8		flag_email;					//����emaill
	u8		flag_mobile;				//�����ֻ�����
	u16 	delay;						//��ʱ
	u32		copy2Chnmask;				//���Ƶ�����ͨ������λ
	u8		reserved[12];				//�����ֶ�
}ifly_VideoLostParam_t;

//for2.0
//�ƶ����
typedef struct
{
	u8 		chn;						//ͨ��
	u32 	trigRecChn;					//����¼��ͨ�� ��λ
	u32 	trigAlarmOut;				//����������� ��λ
	u8 		flag_buzz;					//������
	u8		flag_email;					//����emaill
	u8		flag_mobile;				//�����ֻ�����
	u16 	delay;						//��ʱ
	u8		sense;						//������
	u8		block[44*36];				//����
	struct  MDPtz_t				//����ptz
	{
		u8		flag_preset;			//Ԥ�õ�
		u8		preset;
		u8		flag_cruise;			//Ѳ����
		u8		cruise;
		u8		flag_track;				//�켣
	} MDPtz [32];
	u32		copy2Chnmask;				//���Ƶ�����ͨ������λ
	u8		flag_enablealarm;			//�������ñ�־
	u8		reserved[11];				//�����ֶ�
}ifly_MDParamfor2_0_t;

//��Ƶ��ʧ
typedef struct
{
	u8 		chn;						//ͨ��
	u32 	trigRecChn;					//����¼��ͨ�� ��λ
	u32 	trigAlarmOut;				//����������� ��λ
	u8 		flag_buzz;					//������
	u8		flag_email;					//����emaill
	u8		flag_mobile;				//�����ֻ�����
	u16 	delay;						//��ʱ
	struct  VideoLostPtz_t				//����ptz
	{
		u8		flag_preset;			//Ԥ�õ�
		u8		preset;
		u8		flag_cruise;			//Ѳ����
		u8		cruise;
		u8		flag_track;				//�켣
	} VideoLostPtz [32];
	u32		copy2Chnmask;				//���Ƶ�����ͨ������λ
	u8		reserved[12];				//�����ֶ�
}ifly_VideoLostParamfor2_0_t;

//��Ƶ�ڵ�
typedef struct
{
	u8 		chn;						//ͨ��
	u32 	trigRecChn;					//����¼��ͨ�� ��λ
	u32 	trigAlarmOut;				//����������� ��λ
	u8 		flag_buzz;					//������
	u8		flag_email;					//����emaill
	u8		flag_mobile;				//�����ֻ�����
	u16 	delay;						//��ʱ
	u32		copy2Chnmask;				//���Ƶ�����ͨ������λ
	u8		reserved[12];				//�����ֶ�
}ifly_VideoBlockParam_t;

//��̨����
typedef struct
{
	u8		chn;						//ͨ��
	u16		address;					//��������ַ
	u8		baud_ratesel;				//������ ѡ��ֵ
	u8		data_bitsel;				//����λ ѡ��ֵ
	u8		stop_bitsel;				//ֹͣλ ѡ��ֵ
	u8		crccheck;					//У��
	u8		flow_control;				//����
	u8		protocol;					//Э������
	u32		copy2Chnmask;				//���Ƶ�����ͨ�� ��λ
	u8		comstyle;					//��������(232 or 485)
#if 1//csp modify
	u8		enableptz;					//ͨ����̨(����)ʹ��
#else
	u8		reserved;					//�����ֶ�
#endif
}ifly_PTZParam_t;

//Ԥ�õ����
typedef struct
{
	u8		chn;						//ͨ��
	u8		presetpoint;				//Ԥ�õ��
	u8		option;						//���� 0��� 1ɾ�� 2 ��Ԥ�õ�
}ifly_PtzPresetCtr_t;

//Ѳ��·������
typedef struct
{
	u8		chn;						//ͨ��
	u8		cruise_path;				//Ѳ��·��
	struct Cruise_point_t
	{
		u8  preset;						//Ԥ�õ�
		u8  weeltime;					//ͣ��ʱ��
		u8  rate;						//�ٶ�
		u8  flag_add;					//0������ 1��� 2ɾ��
	}Cruise_point[16];
}ifly_PtzCruisePathParam_t;

//Ѳ��·������
typedef struct
{
	u8		chn;						//ͨ��
	u8		cruisepath;					//Ѳ��·����
	u8		flagoption;					//���� 0ֹͣ 1��ʼ 
}ifly_CruisePathCtr_t;

//�켣����
typedef struct
{
	u8		chn;						//ͨ��
	u8		flagoption;					//���� 0��¼ 1ֹͣ��¼  2��ʼ�켣 3ֹͣ�켣 
}ifly_TrackCtr_t;

//Ӳ�̹���
typedef struct 
{
	u8		hdd_index;					//Ӳ�����
	u8		hdd_exist;  				//1 exist; 0 not exist
	u32		capability;					//MB
	u32		freesize;					//MB
	u8		reserved[2];				//Ԥ��
} ifly_hddInfo_t;

//�û���Ϣ
typedef struct 
{
	char	name[12];
	char	password[12];
	char	mac_address[18];//mac��ַ	
	u8		rcamer;		//remote yuntai 2.0��̨
	u8		rrec;		//remote record 2.0¼��
	u8		rplay;		//remote playback 2.0�ط�
	u8		rsetpara;	//remote set param 2.0ϵͳ����
	u8		rlog;		//remote get log 2.0ϵͳ��־
	u8		rtool;		//remote use tool 2.0Ӳ�̹���
	u8		rpreview;	//remote preview 2.0Զ�̵�¼
	u8		ralarm;		//remote alarm 2.0���ݱ���
	u8		rvoip;		//voip 2.0�����Խ�
	u8		lcamer;		//local yuntai 2.0�ָ���������
	u8		lrec;		//local record 2.0ϵͳ����
	u8		lplay;		//local palyback 2.0�ػ�
	u8		lsetpara;	//local set param 2.0admin
	u8		llog;		//local log 2.0mac�󶨱�־
	u8		ltool;		//local tool	
}ifly_user_t;

typedef struct
{
	ifly_user_t	userNum[8];		//���8 ���û�
}ifly_userNumber_t;

typedef struct
{
	ifly_user_t	userInfo;		//�û���Ϣ
	u8		flagOption;					//0����û� 1�༭�û� 2ɾ���û�
}ifly_userMgr_t;

//ϵͳʱ��
typedef struct
{
	u32		systemtime;					//ϵͳʱ��
	u8		format;						//ʱ���ʽ ѡ��ֵ
	u8		flag_time;					//Ԥ��ʱ��λ����ʾ
	u16		timepos_x;					//Ԥ��ʱ��x����
	u16		timepos_y;					//Ԥ��ʱ��y����
}ifly_sysTime_t;

//ϵͳ��Ϣ
typedef struct
{
	char	devicename[32];				//�豸��
	char	devicemodel[32];			//�豸�ͺ�
	char	deviceser[32];				//�豸���к�
	char	version[64];				//����汾
}ifly_sysinfo_t;

//��̨����
//��������:
//0-ֹͣ 1�� 2�� 3 �� 4�� 5��ת 
//6�䱶�� 7�䱶�� 8����� 9����� 10��Ȧ�� 
//11��Ȧ�� 12�ƹ⿪ 13�ƹ�� 14��ˢ�� 15��ˢ�� 
//16 ���� 17���� 18���� 19������ 20������
typedef struct
{
	u8  chn;							//ͨ��
	u8  cmd;							
	u8	reserved;
}ifly_PtzCtrl_t;

//�ֶ�¼��
typedef struct
{
	u32  chnRecState;					//ͨ���ֶ�¼��״̬ ��λ	
}ifly_ManualRecord_t;

//����ͷ����
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
}ifly_search_ipc;


//��־����
typedef struct
{
	u8		query_mode;					//��ѯ��ʽ		
	u8		main_type;					//������		
	u8		slave_type;					//������		
	u16		max_return;					//��󷵻���		
	u16		startID;					//���شӵ�һ����¼��ʼ		
	u32		start_time;					//��ʼʱ��		
	u32		end_time;					//����ʱ��			
	u8      reserved[16];				//Ԥ��		
}ifly_search_log_t;

typedef struct
{
	u32		startTime;
	u8		main_type;					//������		
	u8		slave_type;					//������		
	char	loginfo[32];				//������Ϣ		
}ifly_logInfo_t;

typedef struct
{
	u16		sum;						//�ܼ�¼��		
	u16		startID;					//��ʼ�ļ�¼,��ַΪ1.���޷�������ʱ,startID��ֵΪ0		
	u16		endID;						//�����ļ�¼,��ַΪ1.��endID��startID���ʱ,��ʾ֮����һ����¼		
}ifly_search_desc_t;

//�ط��ļ�����
typedef struct
{
	u16		channel_mask;				//ͨ�� ��λ��� ��ָͨ��1-16
	u16		type_mask;					//���� ��λ���
	u32		start_time;					//��ʼʱ��
	u32		end_time;					//��ֹʱ��
	u16		startID;					//���صĵ�һ����¼,��1��ʼ
	u16		max_return;					//ÿ�η��ص�����¼��
	u16		chn17to32mask;				//ͨ��17-32 ��λ���
	u8		reserved[5];				//Ԥ��
	char	bankcardno[21];				//����
}ifly_recsearch_param_t;

////Զ�̻طŷ��ص���Ϣ�ṹ
typedef struct
{
	u8		channel_no;
	u8		type;
	u32		start_time;
	u32		end_time;
	u8		image_format;				//3:cif;4:4cif
	u8		stream_flag;				//0:��Ƶ��;1:������
	u32		size;
	u32		offset;
	char	filename[64];
}ifly_recfileinfo_t;

//�������֪ͨ
typedef struct
{
	u32	 id;							//�ط�id
	u32  currPos;						//��ǰ����
	u32  totallen;						//��ʱ��
}ifly_progress_t;

//�ط��ļ��Ƿ�����Ƶ
typedef struct  
{
	u8 bHasAudio;						//1����Ƶ��0����Ƶ
}ifly_play_hasaudio_t;

//�첽������Ϣ
typedef struct
{
	u32	 time;							//��������ʱ��
	u32  alarmInfo;						//����ֵ
	u32  reserved;						//Ԥ��
}ifly_alarminfo_t;


typedef struct  
{
	u8 chn;							//ͨ��
	u8 type;						//0-�������� 1-������
}ifly_spspps_req_t;


typedef struct
{	
	u8 chn;							//ͨ��
	u8 type;						//0-�������� 1-������
	u8 spsbuf[20];					//sps buf
	u8 spslen;						//sps����
	u8 ppsbuf[20];					//pps buf
	u8 ppslen;						//pps����
}ifly_spspps_t;

typedef struct 
{
	//0-�ź�������,1-Ӳ����,2-�źŶ�ʧ,3���ƶ����,4��Ӳ��δ��ʽ��,
	//5-��дӲ�̳���,6-�ڵ�����,7-��ʽ��ƥ��, 8-�Ƿ�����
	u8 type;		
	u8 state;       //1���� 2�ָ�
	u8 id;			//ͨ��,Ӳ�̺�,���������,ȡ����type 
	u16 reserved1;  //Ԥ��
	u32  reserved2; //Ԥ��
}ifly_alarmstate_t;


typedef struct
{
	u8 com_protocol;	//Э���� 0:N5,1:D4,2:S8,3:S7,4:F2,5:G2E,6:G2N,7:CR,8:TY,9:YH
	u32 com_baudrate;	//������
	u8 com_checkbit;	//У��λ 0:NONE,1:��У�飬2:żУ��
	u8 com_databit;		//����λ 8��7��6
	u8 com_stopbit;		//ֹͣλ 1��2
	u8 serialport;		//1-232, 2-485
	u8 reserved[23];	//����
}ifly_com_param_t;

//͸�����ڷ�������ͷ
typedef struct  
{
	u8 byChannel;	//����ͨ����ʹ��485����ʱ��Ч����1��ʼ��232������Ϊ͸��ͨ��ʱ��ֵ����Ϊ0
	u32 dwDataSize;		//���͵����ݳ���
	u8 reserved[16];	//Ԥ��
}ifly_serial_hdr;

//CTRL_CMD_GETAUDIOVOLUME	���ĳͨ��������Ϣ
//CTRL_CMD_SETAUDIOVOLUME	����ĳͨ��������Ϣ
typedef struct
{
	u8 audioDeviceTye; //0 ͨ���������� 1 �Խ��������� 2�Խ��������
	u8 chn;			//ͨ��
	u8 muteflag;	//�������
	u8 volume;		//����ֵ 0-100
	u8 audioEnctype;	//�����ʽ
	u8 reserved[4];	//Ԥ��
}ifly_audio_volume_t;

//ϵͳ�����б�
enum SYS_LANGUAGE{
	SYS_LANGUAGE_CHS = 0,		//��������
	SYS_LANGUAGE_ENU = 1,		//��ʽӢ��
	SYS_LANGUAGE_CHT = 2,       //�����G��
	SYS_LANGUAGE_GER = 3,		//����
	SYS_LANGUAGE_FRE = 4,		//����
	SYS_LANGUAGE_SPA = 5,		//��������
	SYS_LANGUAGE_ITA = 6,		//�����
	SYS_LANGUAGE_POR = 7,		//��������
	SYS_LANGUAGE_RUS = 8,		//����
	SYS_LANGUAGE_TUR = 9,		//��������
	SYS_LANGUAGE_THA = 10,		//̩����
	SYS_LANGUAGE_JAP = 11,		//����
	SYS_LANGUAGE_HAN = 12,		//����
	SYS_LANGUAGE_POL = 13,		//������
	SYS_LANGUAGE_HEB = 14,      //ϣ������Hebrew
	SYS_LANGUAGE_HUN = 15,		//��������Hungarian
	SYS_LANGUAGE_ROM = 16,      //������Roma
	SYS_LANGUAGE_IRA = 17,		//������
	SYS_LANGUAGE_CZE = 18,		//�ݿ���
	SYS_LANGUAGE_VIE = 19,		//Խ����
	SYS_LANGUAGE_LIT = 20,		//������
	SYS_LANGUAGE_SLO = 21,		//˹�工��
	SYS_LANGUAGE_ARA = 22,		//��������
	SYS_LANGUAGE_GRE = 23,		//ϣ����
	SYS_LANGUAGE_RMN = 24,		//����������
	SYS_LANGUAGE_FAR = 25,		//��˹��
	SYS_LANGUAGE_BUL = 26,		//��������
	SYS_LANGUAGE_INVALID = -1	//���������Чֵ��
};

typedef struct  
{
	u8 max_langnum;		//���֧�������� <=32
	u8 langlist[32];	// language list, ���32��,ÿ��ֵΪSYS_LANGUAGE(תu8)
	u8 reserved[16];	//Ԥ��
}ifly_SysLangList_t;

//λ���б�
typedef struct  
{
	u8	chn;				//ͨ��
	u8	videotype;			//0 ������, 1 ������
	u32 bitratelist[16];	//λ���б�, ��λkb, δʹ�õ���0
	u8	reserved[16];		//Ԥ��
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


//����EMail SMTP��Ϣ
typedef struct  
{
	char	alarm_email[32];	//���ͱ���email��ַ
	char	SMTP_svr[32];		//emali��SMTP������
	char	username[32];		//�����û���
	char	userpw[32];			//��������
	u16		SMTPport;			//SMTP�˿�
	u8		flag_ssl;			//�Ƿ�ʹ��SSL
	u8		reserved[13];		//Ԥ��
}ifly_AlarmEmail_SMTP_t;

typedef struct  
{
	char	alarm_email1[32];	//���ͱ���email��ַ
	char	alarm_email2[32];	//���ͱ���email��ַ
	char	alarm_email3[32];	//���ͱ���email��ַ
	char	SMTP_svr[32];		//emali��SMTP������
	char	username[32];		//�����û���
	char	userpw[32];			//��������
	u16		SMTPport;			//SMTP�˿�
	u8		flag_ssl;			//�Ƿ�ʹ��SSL
	u8		reserved[13];		//Ԥ��
}ifly_AlarmSMTPfo2_0_t;

//������ץͼ
typedef struct  
{
	u8 chn;						//Ҫץͼ��ͨ��
	u8 reserved[4];
}ifly_snapreq_t;

typedef struct  
{
	u32 imglenth;				//ͼƬ����
	u8 reserved[4];
}ifly_snapack_t;

//�����Ӱ���qq
typedef struct  
{
	u8 sn[64];					//sn
	u8 productcode[20];			//��Ʒ������
	u8 macaddr[18];				//mac��ַ
	u8 reserved[16];			//Ԥ��
}ifly_las_afqq_info;

typedef struct
{
	u8	bUseUTC;
	u8	reserved[16];		//Ԥ��
}ifly_utctime_t;

//Ԥ�õ�����
typedef struct  
{
	u8 chn;		//ͨ��
	u8 preset;	//Ԥ�õ��  1-128
	char name[12];	//Ԥ�õ�����
}ifly_presetname_t;

//Ԥ�õ��б�
typedef struct
{
	u8 chn;	//ͨ��
	u8 totalAddedpreset; //����ӵ�Ԥ�õ����� <=128
	struct presetstate_t
	{
		u8 preset;	//Ԥ�õ��  1-128
		u8 bAdded;	//�Ƿ������
		char name[12];	//Ԥ�õ�����
	}presetstate[128]; //Ԥ�õ�״̬��
}ifly_preset_list_t;

//��̨�ٶ�
typedef struct
{
	u8 chn;  //ͨ��
	u8 rate; //�ٶ� 16 ���� 17���� 18����
}ifly_ptz_rate_t;


//�ָ�Ĭ�ϻ��������CTRL_CMD_RESETPICADJUST
typedef struct  
{
	u8 chn;  //ͨ�� 0��ʼ
	u8 adjtype; //��λ��ʾ���ͣ���1-4λ�ֱ��ʾ���ȣ��Աȶȣ�ɫ�������Ͷȣ���Ϊ1�ı�ʾ�ָ�Ĭ��ֵ�� 0xf��ʾȫ�ָ�
}ifly_reset_picadjust_t;

//֡���б� CTRL_CMD_GETFRAMERATELIST
typedef struct
{
	u8 chn;	//ͨ��
	u8 type;	// 0 ������ 1 ������
	u8 framerate[10];	//֡���б� δʹ�õ���0
}ifly_framerate_list_t;

// typedef struct
// {
// 	u8 chn;	//ͨ��
// 	u8 frametype;	// 0 ������ 1 ������
// 	u8 resolutiontype;	// ��CTRL_GET_RESOLUTION_LIST
// 	u8 framerate[10];	//֡���б� δʹ�õ���0
// }ifly_frameratelistbyresolution_t;

//��/�������ֱ����б� CTRL_GET_RESOLUTION_LIST
#define VIDEORESOLU_BEGIN	1
#define VIDEORESOLU_QCIF	1
#define VIDEORESOLU_CIF		2
#define VIDEORESOLU_HD1		3
#define VIDEORESOLU_D1		4
#define VIDEORESOLU_END		VIDEORESOLU_D1
typedef struct  
{
	u8 chn;	//ͨ��
	u8 type;	// 0 ������ 1 ������
	u8 videoresolu[8];	//�ֱ����б� δʹ�õ���0 VIDEORESOLU_BEGIN ~ VIDEORESOLU_END
}ifly_videoresolu_list_t;

typedef struct  
{
	u32 nrserverip; //��������ַ
	u16 serverport; //�������˿�
	u16 databindport; //���ݰ󶨶˿�
	u8 reserved[16]; //Ԥ��
}ifly_nrserver_t; //���������

//����
typedef struct  
{
	u8 alarmid; //alarmid 0��ʼ
	u8  val; //ȡֵ 0δ���� 1����
	u8 reserved[2]; //Ԥ��
}ifly_alarm_val_t;

typedef struct
{
	ifly_recfileinfo_t fileinfo; //�ļ���Ϣ
	int currindex; //��ǰ�ڼ����ļ� 1��ʼ����
	int totalnum; //�ܹ��ж����ļ�
	u32 totalfilesize;	//���ļ���С MBΪ��λ����1024��
	u8 reserved[16]; //Ԥ��
}ifly_dowloadbytime_file_t;

typedef struct
{
	//0-�ź�������,1-Ӳ����,2-�źŶ�ʧ,3���ƶ����,4��Ӳ��δ��ʽ��,
	//5-��дӲ�̳���,6-�ڵ�����,7-��ʽ��ƥ��, 8-�Ƿ�����
	u8	type;		
	u8	state;			//1���� 2�ָ�
	u8	id;				//ͨ��,Ӳ�̺�,���������,ȡ����type 
	u16	reserved1;		//Ԥ��
	u32	reserved2;		//Ԥ��	
}ifly_alarmuploadstate_t;

//�����豸��ϢCTRL_CMD_GET_SPECIALDEVICEINFO
typedef struct
{
	u8 mainbordmode; //�����ͺ� Ԥ��
	u8 characterset; //�ַ���: 0-ascii��1-GB2312, 2-UTF8, 3-UTF16 
	u8 reserved[62]; //Ԥ��
}ifly_specialinfo_t;


//CTRL_CMD_GET_CREAROSVR_INFO	//��ô���ƽ̨��Ϣ
//CTRL_CMD_SET_CREAROSVR_INFO	//���ô���ƽ̨��Ϣ
typedef struct  
{
	u8   flagPlatform;	//�Ƿ�����ƽ̨ 0������ 1����
	u32  ipPlatform;	//ƽ̨��ַ
	u16  portPlatform;	//ƽ̨�˿�
	char PUID[19];
	char passwd[32];	//��������
	u8	 reserved[32];	//Ԥ��
}ifly_crearo_svr_info;


//CTRL_CMD_GET_HARDWAREVERSION
typedef struct  
{
	char	hardwareModel[64];		//Ӳ���ͺ�
	char	hardwareVersion[64];	//Ӳ���汾
	u8		reserved[64];			//Ԥ��
}ifly_hardware_info;


//CTRL_CMD_GET_RECFILE_LIMIT_PARAM ���¼���ļ����Ʋ���
//CTRL_CMD_SET_RECFILE_LIMIT_PARAM ����¼���ļ����Ʋ���
typedef struct  
{
	u16	savedays;		//�ļ���������
	u32	maxsize;		//�ļ�����С
	u32 maxtimelen;		//�ļ��ʱ��
	u8	reserved[16];	//Ԥ��
}ifly_recfile_limit_param;

//#define CTRL_CMD_MAKE_KEYFRAME			CTRL_CMD_BASE+113		//ǿ����/����������һ���ؼ�֡
typedef struct
{
	u8 chn;			//ͨ�� 0��ʼ
	u8 type;		// 0 ������ 1������
	u8 reserved[6];	//Ԥ��
}ifly_makekeyframe_t;  // 8 Byte

typedef struct
{
	char	ddns_user[64];				//ddns�û���
	char	ddns_passwd[64];			//ddns����
	char	ddns_domain[64];			//ddns����
	u8		reserved[16];		//Ԥ��
}ifly_ddnsinfo_t;

//�������벼��
typedef struct
{
	u8 chn; //ͨ��
	u8 weekday; //����
	struct AlarmTimeField_t
    {
		u32 starttime; //��ʼʱ��
		u32 endtime; //��ֹʱ��
        u8  flag_alarm; //��������
		u8 reserved[6]; //�����ֶ�
    }TimeFiled[4];
    u8 copy2Weekday; //���Ƶ�������  ��λ 
    u32 copy2Chnmask; //���Ƶ�����ͨ������λ
	u8 reserved[16]; //�����ֶ�
}ifly_AlarmSCH_t;

typedef struct  
{
	u8	mdsenselist[12];	//�ƶ�����������б�
}ifly_MDSenselist_t;

typedef struct  
{
	u16	mdalarmdelaylist[16];	//�ƶ���ⱨ�������ʱ�б�
}ifly_MDAlarmDelaylist_t;

typedef struct  
{
	//csp modify
	//u16 baudratelist[16];	//�������б�
	u32 baudratelist[16];	//�������б�
}ifly_BaudRateList_t;

typedef struct  
{
	u8  maxNum;
	u8	baudratelist[19];	//��̨Э���б�
}ifly_PTZProtocolList_t;

typedef struct  
{
	u8 chn; //ͨ��
	u8 weekday; //����
	u8 type;
	struct RecSchTimeField_t
    {
		u32 starttime; //��ʼʱ��
		u32 endtime; //��ֹʱ��
		u8 reserved[6]; //�����ֶ�
    }TimeFiled[24];
    u8 copy2Weekday; //���Ƶ�������  ��λ 
    u32 copy2Chnmask; //���Ƶ�����ͨ������λ
	u8 reserved[16]; //�����ֶ�
}ifly_RecSchTime_t;

typedef struct  
{
	u32 admin_host;             //����������3g���ݷ�����ip��ַ
	u16 ctrl_port;            //���ɵ�½�˿�
	u16 data_port;            //�������ݶ˿�
	u8   server_enable_flag;  //��������ʹ��
	u16   heartbeat_time;	 //��������
	char   huina_http_ulr[64];			 //����http��ַ
	char   device_flag[16];				 //�豸���
	char   shop_num[16];				 //���ɵ��̱��
	u8 reserved[5];
}ifly_HuiNaInfo_t;

//csp modify
typedef struct
{
	u32 reboot_time; // �豸����ʱ��
	u8 reserve[4];
}ifly_reboottime_t;

//csp modify
typedef struct
{
	u16 alarm_timeinterval; //sΪ��λ�������ϴ�ʱ����
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
	char	username[12];				//�û���
	char	reserved1[4];
	u64		nRemoteView[1];				//Զ��Ԥ��Ȩ��
	u64		reserved2[29];
}ifly_AdvPrivilege_t;
#pragma pack( pop )

typedef u16 (*pMESSAGE_CB)(CPHandle cph,u16 event,u8 *pbyMsgBuf,int msgLen,u8 *pbyAckBuf,int *pAckLen,void* pContext);
void SetMsgCallBack(pMESSAGE_CB pCB,void *pContext);

//�Զ������豸
//sever�ˣ������豸��Ϣ
void SetDeviceInfo(ifly_DeviceInfo_t deviceinfo);

//client�ˣ�ע�������ص�����
typedef void (*pFindDeivce_CB)(ifly_DeviceInfo_t deviceinfo, void* pContext);
void SetFindDeviceCB(pFindDeivce_CB pCB, void *pContext);
//client�ˣ���ʼ����
int SearchDevice();

//�㲥�豸��ͬ����Ѳ
int PatrolSync();


//sever�ˣ�ע���������ӻص�����
//�����������������ʱ���ڻص�����д���
//pAddStreamTCPLink�� �ɹ�ʱ����0������-1
typedef int (*pAddStreamTCPLink)(SOCKHANDLE hSock, ifly_TCP_Stream_Req req, void* pContext);
void SetAddStreamLinkCB(pAddStreamTCPLink pCB, void *pContext);

//����tcp����ͷ�������������������������
int SendTcpConnHead(SOCKHANDLE hSock, u8 type);

void SetCliAckPort(u16 wPort);

u16  CPLibInit(u16 wPort);
u16  CPLibCleanup();

/************************
**07-08-21 verifying connecting req from custorm
*************************/
CPHandle CurConnect_out(u32 i);
//���ӷ�����
CPHandle CPConnect(u32 dwServerIp, u16 wServerPort, u32 dwTimeOut, u16 *pwErrorCode);
//�崦����
u16  CleanCPHandle(CPHandle cph);
//�����¼�֪ͨ
u16  CPPost(CPHandle cph, u16 event, const void *content, int length);
//��������
u16  CPSend(CPHandle cph, u16 event, const void *content, int length, void* ackbuf, int ackbuflen, int *realacklen, u32 dwTimeOut);

//�õ�TCP���ӵı���ip��ַ��˿�
BOOL GetCPHandleLocalAddr(CPHandle cph, struct sockaddr_in *pAddr);

//����NAT̽���
BOOL SendSTUNPacket(u32 dwLocalIpAddr, u16 wLocalPort, u32 dwRemoteIpAddr, u16 wRemotePort, u8 *pbyBuf, u32 dwBufLen);

u16  GetTransactionNum();



/***************************************************************************************************************/
//���ͽ�������������������ؿ��ƴ�������ս��
//��������Ϊ��
//	1.client�˽�Ҫ���͵Ĳ������ض��ĸ�ʽ�����Ȼ����ñ�����
//	2.server�˽��յ�����󣬽�����Ӧ�Ĵ���
//	3.server���ݴ��������ض�Ӧ����Ϣ��client�����ر�socket ���ӣ�

//���������
//	dwHostIp��������IP��ַ
//	dwTimeOut�������ĳ�ʱʱ��
//	pBuf�����͵�����ͷ��������������Ϣ�������������У�pBuf = ifly_cp_header_t + ifly_monitor_param_t
//	dwBufLen��pBuf�ĳ���
//����ֵ��
//	0�����óɹ�
//	����������ʧ��
/***************************************************************************************************************/
u16  SendToHost(u32 dwHostIp, u32 dwTimeOut, void *pBuf, u32 dwBufLen);

/***************************************************************************************************************/
//���ͽ������������������Ҫ��������ö������Ϣ
//��������Ϊ��
//	1.client�˽�Ҫ���͵Ĳ������ض��ĸ�ʽ�����Ȼ����ñ�����
//	2.server�˽��յ�����󣬽�����Ӧ�Ĵ���
//	3.server���ݴ��������ض�Ӧ����Ϣ��client�����ر�socket ���ӣ�

//���������
//	dwHostIp��������IP��ַ
//	dwTimeOut�������ĳ�ʱʱ��
//	pinBuf�����͵�����ͷ��������������Ϣ�������������У�pinBuf = ifly_cp_header_t + ifly_monitor_param_t
//	dwinBufLen��pinBuf�ĳ���
//	dwoutBufLen��poutBuf��󳤶ȣ���ֹ�ڴ����
//���������
//	poutBuf�����������յ�����ͷ��������������Ϣ������Զ�̻طŲ�ѯ�У�poutBuf = ifly_cp_header_t + ��ѯ�Ľ��
//	dwoutBufLen��ʵ�ʽ��յ���buf�ĳ���
//����ֵ��
//	0�����óɹ�
//	����������ʧ��
//ע�⣺
//	����Ӧ���������dwoutBufLen�����Ʒ��ظ�client��������ݣ�������ܵ����������
/***************************************************************************************************************/
u16  SendToHost2(u32 dwHostIp, u32 dwTimeOut, void *pinBuf, u32 dwinBufLen, void *poutBuf, u32 *dwoutBufLen);



#ifdef __cplusplus
}
#endif

#endif
