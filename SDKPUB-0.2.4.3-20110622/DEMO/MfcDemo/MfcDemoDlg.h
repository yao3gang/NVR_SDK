// MfcDemoDlg.h : header file
//

#if !defined(AFX_MFCDEMODLG_H__AA24552B_7663_4F22_A521_483A8366B5CA__INCLUDED_)
#define AFX_MFCDEMODLG_H__AA24552B_7663_4F22_A521_483A8366B5CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "netdvr.h"
#include "TLFileLib.h"
#include "DDrawShow.h"
#define  start WM_USER+100
#define  stop  WM_USER+101
#define  esc   WM_USER+102

/////////////////////////////////////////////////////////////////////////////
// CMfcDemoDlg dialog
#define MAX_PLAYWND	4

struct playinfo
{
	int chn;
	LPVOID pDlg;
};
class CMfcDemoDlg : public CDialog
{
// Construction
public:
	CMfcDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMfcDemoDlg)
	enum { IDD = IDD_MFCDEMO_DIALOG };
	CSliderCtrl	m_ctrSlider;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMfcDemoDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	NETDVR_DeviceInfo_t m_devinfo;
	static DWORD WINAPI _PlayThread_S(LPVOID param);
	static DWORD WINAPI _CtrlThread_S(LPVOID param);
	int curr;
	DWORD palyid[16];
	playinfo play[16];
	bool exit;
	DWORD id;
	int dlhandle;
	HANDLE playluck;
	TLFILE_t file;
	NETDVR_AudioProperty_t m_AudioPro;

private:
	int m_RealHandle[16/*MAX_PLAYWND*/];
	
	int m_PlayBackHandle;

	int m_GetFileFrmHandle;
protected:
	HICON m_hIcon;

	int m_hDvr;
	// Generated message map functions
	//{{AFX_MSG(CMfcDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBTlogin();
	afx_msg void OnBTlogout();
	afx_msg void OnPlayAll();
	afx_msg void OnStopAll();
	afx_msg void OnStartRec();
	afx_msg void OnStopRec();
	afx_msg void OnSnap();
	afx_msg void OnPlayAudio();
	afx_msg void OnStopAudio();
	afx_msg void OnSelchangeCOMBOVideoType();
	afx_msg void OnMute();
	afx_msg void OnStartVoip();
	afx_msg void OnStopVoip();
	afx_msg void OnRecSearch();
	afx_msg void OnSearchClean();
	afx_msg void OnFilePlay();
	afx_msg void OnTimePlay();
	afx_msg void OnPbPause();
	afx_msg void OnPbResume();
	afx_msg void OnPbStep();
	afx_msg void OnPbFast();
	afx_msg void OnPbSlow();
	afx_msg void OnPbRate();
	afx_msg void OnPbPrev();
	afx_msg void OnPbNext();
	afx_msg void OnPbSeek();
	afx_msg void OnPbMute();
	afx_msg void OnDownLoad();
	afx_msg void OnPtzParam();
	afx_msg void OnPtzCtr();
	afx_msg void OnPtzStop();
	afx_msg void OnSetPreset();
	afx_msg void OnGoPreset();
	afx_msg void OnClearPreset();
	afx_msg void OnStartRecTrack();
	afx_msg void OnStopRecTrack();
	afx_msg void OnStartTrack();
	afx_msg void OnStopTrack();
	afx_msg void OnInsertCruise();
	afx_msg void OnDeleteCruise();
	afx_msg void OnStartCruise();
	afx_msg void OnStopCruise();
	afx_msg void OnParamSys();
	afx_msg void OnParamVideo();
	afx_msg void OnParamRec();
	afx_msg void OnParamRecTime();
	afx_msg void OnParamException();
	afx_msg void OnParamNetwork();
	afx_msg void OnParamVideoLost();
	afx_msg void OnParamSHTime();
	afx_msg void OnManualRec();
	afx_msg void OnSysTime();
	afx_msg void OnHddInfo();
	afx_msg void OnSysInfo();
	afx_msg void OnUpdate();
	afx_msg void OnRestore();
	afx_msg void OnReboot();
	afx_msg void OnShutDown();
	afx_msg void OnMDParam();
	afx_msg void OnMDTime();
	afx_msg void OnMDAlarm();
	afx_msg void OnAlarmInParam();
	afx_msg void OnAlarmInTime();
	afx_msg void OnAlarmInAlarm();
	afx_msg void OnAlarmInPtz();
	afx_msg void OnAlarmOutParam();
	afx_msg void OnAlarmOutTime();
	afx_msg void OnClearAlarms();
	afx_msg void OnAlarmState();
	afx_msg void OnLogSearch();
	afx_msg void OnLogClean();
	afx_msg void OnGetUserList();
	afx_msg void OnClearUserList();
	afx_msg void OnAddUser();
	afx_msg void OnEditUser();
	afx_msg void OnDelUser();
	afx_msg void OnRegConnLost();
	afx_msg void OnSDKcheck();
	afx_msg void OnGetConnTime();
	afx_msg void OnSetConnTime();
	afx_msg void OnRegAlarmUpload();
	afx_msg void OnGetFileFrame();
	afx_msg void OnSerialport();
	afx_msg void OnSerialParam();
	afx_msg void OnSerialOpen();
	afx_msg void OnSerialClose();
	afx_msg void OnSetReconnect();
	afx_msg void OnButtonPolling();
	afx_msg void OnButtonTimedownload();
	afx_msg void OntestLAS();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCDEMODLG_H__AA24552B_7663_4F22_A521_483A8366B5CA__INCLUDED_)
