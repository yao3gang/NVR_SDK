#ifndef _UI_H_
#define _UI_H_

#include "common.h"

typedef enum 
{
	IFLY_KEYCODE_0               = 0x373008ff,
	IFLY_KEYCODE_1               = 0x383108ff,
	IFLY_KEYCODE_2               = 0x393208ff,
	IFLY_KEYCODE_3               = 0x3a3308ff,
	IFLY_KEYCODE_4               = 0x3b3408ff,
	IFLY_KEYCODE_5               = 0x3c3508ff,
	IFLY_KEYCODE_6               = 0x3d3608ff,
	IFLY_KEYCODE_7               = 0x3e3708ff,
	IFLY_KEYCODE_8               = 0x3f3808ff,
	IFLY_KEYCODE_9               = 0x403908ff,

	IFLY_KEYCODE_MENU            = 0x413a08ff,
	IFLY_KEYCODE_FN				 = 0x423b08ff,/*编辑*/
	IFLY_KEYCODE_SHIFT           = 0x433c08ff,/*输入法*/
	IFLY_KEYCODE_PLAY            = 0x443d08ff,
	IFLY_KEYCODE_RECORD          = 0x453e08ff,
	IFLY_KEYCODE_PTZ             = 0x463f08ff,/*云镜*/
	IFLY_KEYCODE_MULTI           = 0x474008ff,/*多画面*/
	IFLY_KEYCODE_VOIP            = 0x484108ff,

//	IFLY_KEYCODE_UP              = 0x494208ff,
//	IFLY_KEYCODE_DOWN            = 0x4a4308ff,
	IFLY_KEYCODE_DOWN            = 0x494208ff,
	IFLY_KEYCODE_UP				 = 0x4a4308ff,
	IFLY_KEYCODE_LEFT            = 0x4b4408ff,
	IFLY_KEYCODE_RIGHT           = 0x4c4508ff,

	IFLY_KEYCODE_ENTER           = 0x4d4608ff,
	IFLY_KEYCODE_ESC             = 0x4e4708ff,

	IFLY_KEYCODE_MUTE            = 0x4f4808ff,
	IFLY_KEYCODE_CLEAR           = 0x504908ff,
	IFLY_KEYCODE_VGA             = 0x514a08ff,
	
	IFLY_KEYCODE_PAUSE           = 0x524b08ff,

	IFLY_KEYCODE_FASTFORWARD     = 0x534c08ff,
	IFLY_KEYCODE_SLOWFORWARD     = 0x544d08ff,
	IFLY_KEYCODE_FASTBACK        = 0x554e08ff,

	IFLY_KEYCODE_PREV			 = 0x564f08ff,
	IFLY_KEYCODE_NEXT            = 0x575008ff,

	IFLY_KEYCODE_POWER           = 0x585108ff
	//IFLY_KEYCODE_POWER         = 0x06ff08ff
}ifly_keycode;

#define MAX_WND_NAME_LEN	64
#define MAX_CTRL_NUM        256

/* Global data structure */
typedef struct _GlobalData
{
    //u16 *display; /* Pointer to the screen frame buffer */
    s32 yFactor;    /* Vertical scaling factor (PAL vs. NTSC) */
}GlobalData;

extern GlobalData gbl;

/* Screen dimensions */
#define yScale(x)						(((x) * gbl.yFactor) / 10)
#define SCREEN_WIDTH					720
#define SCREEN_HEIGHT					yScale(480)
#define SCREEN_SIZE						(SCREEN_WIDTH * SCREEN_HEIGHT * SCREEN_BPP / 8)

#define INPUTMODE_NUM					0x0001
#define INPUTMODE_LOWER					0x0002
#define INPUTMODE_UPPER					0x0004
#define INPUTMODE_SIGN					0x0008
#define INPUTMODE_SECTION				0x0010

#define IDD_DIALOG_LOGIN				1000
#define IDD_DIALOG_SYSTEM				1300
#define IDD_DIALOG_TOOL					1600
#define IDD_DIALOG_RESUME				1900
#define IDD_DIALOG_HDD					2200
#define IDD_DIALOG_ONLINE				2500
#define IDD_DIALOG_SYSTEMPARAM			2800
#define IDD_DIALOG_UPDATE				3100
#define IDD_DIALOG_UPDATESUCC			3200
#define IDD_DIALOG_PASSWD				3400
#define IDD_DIALOG_RESTART				3700
#define IDD_DIALOG_SYSINFO				4000
#define IDD_DIALOG_LOG					4300
#define IDD_DIALOG_USERMANAGE			4600
#define IDD_DIALOG_DELUSER				4900
#define IDD_DIALOG_EDITUSER				5200
#define IDD_DIALOG_LAYOUT				5500
#define IDD_DIALOG_NETWORK				5800
#define IDD_DIALOG_EXCEPTION			6100
#define IDD_DIALOG_YUNTAI				6400
#define IDD_DIALOG_ALARMTIME			6700
#define IDD_DIALOG_PTZ					7000
#define IDD_DIALOG_ALARMDISPOSE			7300
#define IDD_DIALOG_ALARM				7600
#define IDD_DIALOG_IMAGEALARM			7900
#define IDD_DIALOG_IMAGEDETECTTIME		8200
#define IDD_DIALOG_PLAY					8500
#define IDD_DIALOG_IMAGESET				8800
#define IDD_DIALOG_CHNSTATE				9100
#define IDD_DIALOG_RECORDSET			9400
#define IDD_DIALOG_RECORDCONF			9700
#define IDD_DIALOG_LOCATION				10000
#define IDC_LOCATION_STR				IDD_DIALOG_LOCATION+1
#define IDD_DIALOG_ENVELOP				10300
#define IDC_REGION_ENVELOP				IDD_DIALOG_ENVELOP+1
#define IDD_DIALOG_MDAREA				10600
#define IDC_PARTITION_MDAREA			IDD_DIALOG_MDAREA+1
/*comment by lshu 20070320*/
#define IDD_DIALOG_YTPRESET             10900
#define IDD_DIALOG_YTTRACK				11200
#define IDD_DIALOG_YTCRUISE             11500
#define IDD_DIALOG_YUNTAICTL			11800
/*comment end*/
typedef enum _WndType
{
	TYPE_DIALOG,/*对话框*/
	TYPE_PICTURE,/*可加载jpg和png文件*/
	TYPE_STATICTEXT,/*静态控件*/
	TYPE_EDITBOX,/*编辑框*/
    TYPE_BUTTON,/*按钮*/
    TYPE_LISTBOX,/*列表框*/
	TYPE_COMBOBOX,/*组合框*/
	TYPE_CHECKBOX,/*复选框*/
	TYPE_PROGRESS,/*进度条*/
	TYPE_SLIDER,/*滑动条*/
    TYPE_BAR,/*任务条*/
	TYPE_MESSAGEBOX,/*消息框*/
	TYPE_PARTITION,/*移动侦测区域*/
	TYPE_LOCATION,/*资源定位*/
	TYPE_REGION,/*遮盖区域*/
	TYPE_INPUTBOX,
	TYPE_DATETIMECTRL,
	TYPE_PWEDITBOX/*PASSWORD编辑框20070410*/
}WndType;

#define MAX_REGION_NUM					4

typedef struct _WndPos
{
	u32 x;/*左上角的x坐标*/
	u32 y;/*左上角的y坐标*/
}WndPos;

typedef struct _WndRect
{
	u32 x;/*左上角的x坐标*/
	u32 y;/*左上角的y坐标*/
	u32 w;/*矩形宽*/
	u32 h;/*矩形高*/
}WndRect;

typedef struct _Wnd Wnd;

typedef BOOL (*FuncShowWindow)(Wnd* pWnd,BOOL bShow);
typedef s32  (*FuncGetFocus)(Wnd* pWnd);
typedef s32  (*FuncLoseFocus)(Wnd* pWnd);
typedef BOOL (*FuncDestroyWindow)(Wnd* pWnd);

struct _Wnd
{	
	WndType type;/*窗体类型，如button或editbox等*/
	u32 id;/*控件id,唯一标识本控件*/
	char name[MAX_WND_NAME_LEN];/*窗体名称*/
	WndRect rect;/*坐标*/
	Wnd* parent;/*父窗体*/
	
	/*通用属性*/
	BOOL visible;/*可见性*/
	BOOL tabstop;/*该控件是否接受焦点*/
	
	/*公共消息处理函数*/
	FuncShowWindow ShowWindow;/*显示窗体*/
	FuncGetFocus GetFocus;/*得到焦点*/
	FuncLoseFocus LoseFocus;/*失去焦点*/
	FuncDestroyWindow DestroyWindow;/*删除窗体*/
};

typedef void (*MsgFxn)(s32 param);

#define BEGIN_MSG_MAP(nID,msg,fxn,param) SetMsgMap(nID,msg,fxn,param)
#define BEGIN_MSG_UNMAP(nID,msg,fxn,param) SetMsgUnMap(nID,msg,fxn,param)

#ifdef __cplusplus
extern "C"
{
#endif

	u32  GetCurPage();
	
	BOOL CreateWnd(WndType type,u32 nID,char* pchWndName,WndRect* pRect,u32 nParentID,BOOL visible);
	BOOL ShowWnd(u32 nID,BOOL bShow);
	BOOL DestroyWnd(u32 nID);
	
	BOOL GetWndText(u32 nID,char *pch,int len);
	BOOL SetWndText(u32 nID,char *pch);

	Wnd* GetWndItem(u32 nID);

	BOOL GetCheckBoxSelect(u32 nID,BOOL *pbSel);
	BOOL SetCheckBoxSelect(u32 nID,BOOL bSel);

	BOOL AddComboBoxItem(s32 nID,char *pchStr);
	BOOL ClearComboBoxItem(s32 nID);
	BOOL GetComboBoxSelect(s32 nID,s32 *pnSel);
	BOOL SetComboBoxSelect(s32 nID,s32 nSel);
	s32  GetComboBoxItemCount(s32 nID);

	BOOL AddListBoxItem(s32 nID,char *pchStr);
	BOOL ClearListBoxItem(s32 nID);
	BOOL DelListBoxItem(s32 nID,s32 nIndex);
	BOOL GetListBoxSelect(s32 nID,s32 *pnSel);
	BOOL GetListBoxItemContext(s32 nID,s32 nIndex,char *pStr);
	BOOL SetListBoxItemContext(s32 nID,s32 nIndex,char *pStr);
	BOOL SetListBoxSelect(s32 nID,s32 nSel);
	s32  GetListBoxItemCount(s32 nID);
	
	BOOL SetProgressRange(s32 nID,s32 nLower, s32 nUpper);
	BOOL GetProgressRange(s32 nID,s32 *pnLower, s32 *pnUpper);
	s32  GetProgressPos(s32 nID);
	s32  SetProgressPos(s32 nID,s32 nPos);
	s32  OffsetProgressPos(s32 nID,s32 nPos);
	s32  SetProgressStep(s32 nID,s32 nStep);
	s32  ProgressStepIt(s32 nID);

	BOOL SetSliderRange(s32 nID,s32 nLower, s32 nUpper);
	BOOL GetSliderRange(s32 nID,s32 *pnLower, s32 *pnUpper);
	s32  GetSliderPos(s32 nID);
	s32  SetSliderPos(s32 nID,s32 nPos);
	s32  OffsetSliderPos(s32 nID,s32 nPos);
	s32  SetSliderStep(s32 nID,s32 nStep);
	s32  SliderStepIt(s32 nID);

	BOOL SetPicToBar(s32 nID,char *file,s32 x,s32 y);
	BOOL SetTextToBar(s32 nID,char *txt,s32 x,s32 y);

	BOOL GetLocationPos(s32 nID,WndPos *pPos);
	BOOL SetLocationPos(s32 nID,WndPos *pPos);
	void SetLocationFontHeight(s32 nID,int font_h);

	BOOL GetPartitionArea(s32 nID,char *pflag,int num);
	void SetPartitionArea(s32 nID,char *pflag,int num);

	BOOL GetRegionRect(s32 nID,WndRect *pRect,u8 *pNum);
	BOOL SetRegionRect(s32 nID,WndRect *pRect,u8 num);
	
	BOOL ShowMsgBox(char *pchText,char *pCaption,WndRect* pRect,BOOL bSure);
	BOOL SetMsgBoxText(char *pchText);

	BOOL GetKeyValue(ifly_keycode *key);
	BOOL SetMsgMap(u32 nID,ifly_keycode msg,MsgFxn fxn,s32 param);
	BOOL SetMsgUnMap(u32 nID,ifly_keycode msg,MsgFxn fxn,s32 param);
	
	BOOL uiInit();
	BOOL uiRelease();

	BOOL setOsdTransparency(u8 trans);
	BOOL setRegionOsdTransparency(u8 trans,int x,int y,int w,int h);
	
	BOOL CreateLocationPage();
	BOOL ShowLocationPage();
	void SetLocationFlag(u8 flag);
	u8   GetLocationFlag();
	
	BOOL CreateEnvelopPage();
	BOOL ShowEnvelopPage();

	BOOL CreateMDAreaPage();
	BOOL ShowMDAreaPage();

	int  SendToPanel(int code);

	BOOL AddInputBoxItem(s32 nID,char *pchStr);
	BOOL GetInputBoxSelect(s32 nID, s32 nIndex, char *pDeno);
	BOOL SetInputBoxSelect(s32 nID, s32 nIndex);

	void GetDateTimeCtrlTime(s32 param, u32 *pYear, u32 *pMonth, u32 * pDate, u32 *pHour, u32 *pMinute, u32 *pSecond);
    void SetDateTimeCtrlTime(s32 param, u32 nYear, u32 nMonth, u32 nDate, u32 nHour, u32 nMinute, u32 nSecond);
	void SetDateTimeCtrlEnable(s32 param, BOOL bYearEnable, BOOL bMonthEnable, BOOL bDateEnable, BOOL bHourEnable, BOOL bMinuteEnble, BOOL bSecondEnable);

	/*comment  by lshu 20070416*/
	BOOL SetEditBoxInputLen(s32 nID,s32 nSel);
	/*comment end */
	void SetInputMode(u32 nID, u32 nInputMode);//comment by lshu 20070508
	u32 GetInpurMode(u32 nID);//comment by lshu 20070508
#ifdef __cplusplus
}
#endif

#endif
