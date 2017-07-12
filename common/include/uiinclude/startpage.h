#ifndef _STARTPAGE_H_
#define _STARTPAGE_H_

#include "ui.h"

#define TVX 52
#define TVY -15

//#define PIC_START_PLAY        "data/pics/playing.png"
//#define PIC_START_REC         "data/pics/manulrencod.png"
//#define PIC_START_YUNTAI       "data/pics/yuntaictrl.png"
#define PIC_START_PLAY        "data/pics/sh_playing.png"
#define PIC_START_REC         "data/pics/sh_manulrencod.png"
#define PIC_START_YUNTAI       "data/pics/sh_yuntaictrl.png"
#define PIC_SYSTEM_SHORTCUT_TOOL			"data/pics/shortcut/sh_tool.png"
#define PIC_SYSTEM_SHORTCUT_LOG			"data/pics/shortcut/sh_log.png"
#define PIC_SYSTEM_SHORTCUT_USER			"data/pics/shortcut/sh_user.png"
#define PIC_SYSTEM_SHORTCUT_NETWORK		"data/pics/shortcut/sh_network.png"
#define PIC_SYSTEM_SHORTCUT_IMAGE		"data/pics/shortcut/sh_image.png"
#define PIC_SYSTEM_SHORTCUT_RECORD		"data/pics/shortcut/sh_record.png"
#define PIC_SYSTEM_SHORTCUT_YUNTAI		"data/pics/shortcut/sh_yuntai.png"
#define PIC_SYSTEM_SHORTCUT_ALARM		"data/pics/shortcut/sh_alarm.png"
#define PIC_SYSTEM_SHORTCUT_EXCEPTION	"data/pics/shortcut/sh_exception.png"
#define IDC_BUTTON_START_MENU         IDD_DIALOG_START+1
#define IDC_BUTTON_START_PLAY         IDD_DIALOG_START+2
#define IDC_BUTTON_START_PTZ         IDD_DIALOG_START+3
#define IDC_BUTTON_START_RECORD         IDD_DIALOG_START+4
#define IDC_BUTTON_START_TOOL         IDD_DIALOG_START+5
#define IDC_BUTTON_START_LOG         IDD_DIALOG_START+6
#define IDC_BUTTON_START_USER         IDD_DIALOG_START+7
#define IDC_BUTTON_START_NETWORK         IDD_DIALOG_START+8
#define IDC_BUTTON_START_IMAGE         IDD_DIALOG_START+9
#define IDC_BUTTON_START_RECORDSET         IDD_DIALOG_START+10
#define IDC_BUTTON_START_YUNTAI         IDD_DIALOG_START+11
#define IDC_BUTTON_START_ALARM         IDD_DIALOG_START+12
#define IDC_BUTTON_START_EXCEPTION         IDD_DIALOG_START+13
#define IDC_PIC_START_PLAY      IDD_DIALOG_START+14
#define IDC_PIC_START_REC        IDD_DIALOG_START+15
#define IDC_PIC_START_YUNTAI        IDD_DIALOG_START+16


#define IDC_BUTTON_TASKBAR_START     IDD_DIALOG_TASKBAR+1
#define IDC_BUTTON_TASKBAR_TOOL		IDD_DIALOG_TASKBAR+2
#define IDC_BUTTON_TASKBAR_LOG		IDD_DIALOG_TASKBAR+3
#define IDC_BUTTON_TASKBAR_USER 	IDD_DIALOG_TASKBAR+4
#define IDC_BUTTON_TASKBAR_NETWORK 	IDD_DIALOG_TASKBAR+5
#define IDC_BUTTON_TASKBAR_IMAGE 	IDD_DIALOG_TASKBAR+6
#define IDC_BUTTON_TASKBAR_RECORDSET 	IDD_DIALOG_TASKBAR+7
#define IDC_BUTTON_TASKBAR_YUNTAI 	IDD_DIALOG_TASKBAR+8
#define IDC_BUTTON_TASKBAR_ALARM 	IDD_DIALOG_TASKBAR+9
#define IDC_BUTTON_TASKBAR_EXCEPTION 	IDD_DIALOG_TASKBAR+10

#define IDC_BUTTON_TASKBAR_SHORTCUT	IDD_DIALOG_TASKBAR+11
//#define IDC_BUTTON_TASKBAR_         IDD_DIALOG_START+






BOOL CreateStartPage();
BOOL ShowStartPage();

BOOL CreateStartTvPage();
BOOL CreateTaskbarTvPage();

BOOL CreateTaskbarPage();
BOOL ShowTaskbarPage();


#endif

