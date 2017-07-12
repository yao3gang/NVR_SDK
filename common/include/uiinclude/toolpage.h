#ifndef _TOOLPAGE_H_
#define _TOOLPAGE_H_

#include "ui.h"

#define PIC_TOOL_PARAM			"data/pics/tools/param.png"
#define PIC_TOOL_HDD			"data/pics/tools/hdd.png"
#define PIC_TOOL_ONLINE			"data/pics/tools/online.png"
#define PIC_TOOL_LOCK			"data/pics/tools/lock.png"
#define PIC_TOOL_CLEARALARM		"data/pics/tools/clearalarm.png"
#define PIC_TOOL_KEY			"data/pics/tools/key.png"
#define PIC_TOOL_RESUME			"data/pics/tools/resume.png"
#define PIC_TOOL_UPDATE			"data/pics/tools/update.png"
#define PIC_TOOL_LAYOUT			"data/pics/tools/layout.png"
#define PIC_TOOL_RESTART		"data/pics/tools/restart.png"
#define PIC_TOOL_INFO			"data/pics/tools/info.png"

#define IDC_STATIC_TOOL_PARAM			IDD_DIALOG_TOOL+1
#define IDC_STATIC_TOOL_HDD				IDD_DIALOG_TOOL+2
#define IDC_STATIC_TOOL_ONLINE			IDD_DIALOG_TOOL+3
#define IDC_STATIC_TOOL_LOCK			IDD_DIALOG_TOOL+4
#define IDC_STATIC_TOOL_CLEARALARM		IDD_DIALOG_TOOL+5
#define IDC_STATIC_TOOL_KEY				IDD_DIALOG_TOOL+6
#define IDC_STATIC_TOOL_RESUME			IDD_DIALOG_TOOL+7
#define IDC_STATIC_TOOL_UPDATE			IDD_DIALOG_TOOL+8
#define IDC_STATIC_TOOL_LAYOUT			IDD_DIALOG_TOOL+9
#define IDC_STATIC_TOOL_RESTART			IDD_DIALOG_TOOL+10
#define IDC_STATIC_TOOL_INFO			IDD_DIALOG_TOOL+11
#define IDC_PICTURE_TOOL_PARAM			IDD_DIALOG_TOOL+12
#define IDC_PICTURE_TOOL_HDD			IDD_DIALOG_TOOL+13
#define IDC_PICTURE_TOOL_ONLINE			IDD_DIALOG_TOOL+14
#define IDC_PICTURE_TOOL_LOCK			IDD_DIALOG_TOOL+15
#define IDC_PICTURE_TOOL_CLEARALARM		IDD_DIALOG_TOOL+16
#define IDC_PICTURE_TOOL_KEY			IDD_DIALOG_TOOL+17
#define IDC_PICTURE_TOOL_RESUME			IDD_DIALOG_TOOL+18
#define IDC_PICTURE_TOOL_UPDATE			IDD_DIALOG_TOOL+19
#define IDC_PICTURE_TOOL_LAYOUT			IDD_DIALOG_TOOL+20
#define IDC_PICTURE_TOOL_RESTART		IDD_DIALOG_TOOL+21
#define IDC_PICTURE_TOOL_INFO			IDD_DIALOG_TOOL+22

BOOL CreateToolPage();
BOOL ShowToolPage();

void OnToolParam(s32 param);
void OnToolOnline(s32 param);
void OnToolLock(s32 param);
void OnToolClearAlarm(s32 param);
void OnToolKey(s32 param);
void OnToolResume(s32 param);
void OnToolLayout(s32 param);
void OnToolRestart(s32 param);
void OnToolInfo(s32 param);

#endif

