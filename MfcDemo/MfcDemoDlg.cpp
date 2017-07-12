// MfcDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MfcDemo.h"
#include "MfcDemoDlg.h"
#include <WINSOCK2.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMfcDemoDlg dialog

CMfcDemoDlg::CMfcDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMfcDemoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMfcDemoDlg)
	m_nAlarmID = 0;
	m_nAlarmVal = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMfcDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMfcDemoDlg)
	DDX_Control(pDX, IDC_SLIDER1, m_ctrSlider);
	DDX_Text(pDX, IDC_EDIT_ID, m_nAlarmID);
	DDV_MinMaxInt(pDX, m_nAlarmID, 0, 15);
	DDX_Text(pDX, IDC_EDIT_VAL, m_nAlarmVal);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMfcDemoDlg, CDialog)
	//{{AFX_MSG_MAP(CMfcDemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTlogin, OnBTlogin)
	ON_BN_CLICKED(IDC_BTlogout, OnBTlogout)
	ON_BN_CLICKED(IDC_PlayAll, OnPlayAll)
	ON_BN_CLICKED(IDC_StopAll, OnStopAll)
	ON_BN_CLICKED(IDC_StartRec, OnStartRec)
	ON_BN_CLICKED(IDC_StopRec, OnStopRec)
	ON_BN_CLICKED(IDC_Snap, OnSnap)
	ON_BN_CLICKED(IDC_PlayAudio, OnPlayAudio)
	ON_BN_CLICKED(IDC_StopAudio, OnStopAudio)
	ON_CBN_SELCHANGE(IDC_COMBOVideoType, OnSelchangeCOMBOVideoType)
	ON_BN_CLICKED(IDC_Mute, OnMute)
	ON_BN_CLICKED(IDC_StartVoip, OnStartVoip)
	ON_BN_CLICKED(IDC_StopVoip, OnStopVoip)
	ON_BN_CLICKED(IDC_RecSearch, OnRecSearch)
	ON_BN_CLICKED(IDC_SearchClean, OnSearchClean)
	ON_BN_CLICKED(IDC_FilePlay, OnFilePlay)
	ON_BN_CLICKED(IDC_TimePlay, OnTimePlay)
	ON_BN_CLICKED(IDC_PbPause, OnPbPause)
	ON_BN_CLICKED(IDC_PbResume, OnPbResume)
	ON_BN_CLICKED(IDC_PbStep, OnPbStep)
	ON_BN_CLICKED(IDC_PbFast, OnPbFast)
	ON_BN_CLICKED(IDC_PbSlow, OnPbSlow)
	ON_BN_CLICKED(IDC_PbRate, OnPbRate)
	ON_BN_CLICKED(IDC_PbPrev, OnPbPrev)
	ON_BN_CLICKED(IDC_PbNext, OnPbNext)
	ON_BN_CLICKED(IDC_PbSeek, OnPbSeek)
	ON_BN_CLICKED(IDC_PbMute, OnPbMute)
	ON_BN_CLICKED(IDC_DownLoad, OnDownLoad)
	ON_BN_CLICKED(IDC_PtzParam, OnPtzParam)
	ON_BN_CLICKED(IDC_PtzCtr, OnPtzCtr)
	ON_BN_CLICKED(IDC_PtzStop, OnPtzStop)
	ON_BN_CLICKED(IDC_SetPreset, OnSetPreset)
	ON_BN_CLICKED(IDC_GoPreset, OnGoPreset)
	ON_BN_CLICKED(IDC_ClearPreset, OnClearPreset)
	ON_BN_CLICKED(IDC_StartRecTrack, OnStartRecTrack)
	ON_BN_CLICKED(IDC_StopRecTrack, OnStopRecTrack)
	ON_BN_CLICKED(IDC_StartTrack, OnStartTrack)
	ON_BN_CLICKED(IDC_StopTrack, OnStopTrack)
	ON_BN_CLICKED(IDC_InsertCruise, OnInsertCruise)
	ON_BN_CLICKED(IDC_DeleteCruise, OnDeleteCruise)
	ON_BN_CLICKED(IDC_StartCruise, OnStartCruise)
	ON_BN_CLICKED(IDC_StopCruise, OnStopCruise)
	ON_BN_CLICKED(IDC_ParamSys, OnParamSys)
	ON_BN_CLICKED(IDC_ParamVideo, OnParamVideo)
	ON_BN_CLICKED(IDC_ParamRec, OnParamRec)
	ON_BN_CLICKED(IDC_ParamRecTime, OnParamRecTime)
	ON_BN_CLICKED(IDC_ParamException, OnParamException)
	ON_BN_CLICKED(IDC_ParamNetwork, OnParamNetwork)
	ON_BN_CLICKED(IDC_ParamVideoLost, OnParamVideoLost)
	ON_BN_CLICKED(IDC_ParamSHTime, OnParamSHTime)
	ON_BN_CLICKED(IDC_ManualRec, OnManualRec)
	ON_BN_CLICKED(IDC_SysTime, OnSysTime)
	ON_BN_CLICKED(IDC_HddInfo, OnHddInfo)
	ON_BN_CLICKED(IDC_SysInfo, OnSysInfo)
	ON_BN_CLICKED(IDC_Update, OnUpdate)
	ON_BN_CLICKED(IDC_Restore, OnRestore)
	ON_BN_CLICKED(IDC_Reboot, OnReboot)
	ON_BN_CLICKED(IDC_ShutDown, OnShutDown)
	ON_BN_CLICKED(IDC_MDParam, OnMDParam)
	ON_BN_CLICKED(IDC_MDTime, OnMDTime)
	ON_BN_CLICKED(IDC_MDAlarm, OnMDAlarm)
	ON_BN_CLICKED(IDC_AlarmInParam, OnAlarmInParam)
	ON_BN_CLICKED(IDC_AlarmInTime, OnAlarmInTime)
	ON_BN_CLICKED(IDC_AlarmInAlarm, OnAlarmInAlarm)
	ON_BN_CLICKED(IDC_AlarmInPtz, OnAlarmInPtz)
	ON_BN_CLICKED(IDC_AlarmOutParam, OnAlarmOutParam)
	ON_BN_CLICKED(IDC_AlarmOutTime, OnAlarmOutTime)
	ON_BN_CLICKED(IDC_ClearAlarms, OnClearAlarms)
	ON_BN_CLICKED(IDC_AlarmState, OnAlarmState)
	ON_BN_CLICKED(IDC_LOGSEARCH, OnLogSearch)
	ON_BN_CLICKED(IDC_LOGCLEAN, OnLogClean)
	ON_BN_CLICKED(IDC_GetUserList, OnGetUserList)
	ON_BN_CLICKED(IDC_ClearUserList, OnClearUserList)
	ON_BN_CLICKED(IDC_AddUser, OnAddUser)
	ON_BN_CLICKED(IDC_EditUser, OnEditUser)
	ON_BN_CLICKED(IDC_DelUser, OnDelUser)
	ON_BN_CLICKED(IDC_RegConnLost, OnRegConnLost)
	ON_BN_CLICKED(IDC_SDKcheck, OnSDKcheck)
	ON_BN_CLICKED(IDC_GetConnTime, OnGetConnTime)
	ON_BN_CLICKED(IDC_SetConnTime, OnSetConnTime)
	ON_BN_CLICKED(IDC_RegAlarmUpload, OnRegAlarmUpload)
	ON_BN_CLICKED(IDC_GetFileFrame, OnGetFileFrame)
	ON_BN_CLICKED(IDC_SERIALPORT, OnSerialport)
	ON_BN_CLICKED(IDC_SerialParam, OnSerialParam)
	ON_BN_CLICKED(IDC_SerialOpen, OnSerialOpen)
	ON_BN_CLICKED(IDC_SerialClose, OnSerialClose)
	ON_BN_CLICKED(IDC_SETRECONN, OnSetReconnect)
	ON_BN_CLICKED(IDC_BUTTON_POLLING, OnButtonPolling)
	ON_BN_CLICKED(IDC_BUTTON_TIMEDOWNLOAD, OnButtonTimedownload)
	ON_BN_CLICKED(IDC_testLAS, OntestLAS)
	ON_BN_CLICKED(IDC_BUTTON_GetAlarm, OnBUTTONGetAlarm)
	ON_BN_CLICKED(IDC_BUTTON_SetAlarm, OnBUTTONSetAlarm)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMfcDemoDlg message handlers


BOOL CMfcDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	GetDlgItem(IDC_ServerAddr)->SetWindowText("192.168.20.142");
	GetDlgItem(IDC_ServerPort)->SetWindowText("8670");
	GetDlgItem(IDC_UserID)->SetWindowText("admin");
	GetDlgItem(IDC_UserPW)->SetWindowText("");

	((CComboBox*)GetDlgItem(IDC_COMBOVideoType))->SetCurSel(0);

	m_hDvr = 0;
	dlhandle = 0;
	file = NULL;
	memset(m_RealHandle, 0, sizeof(m_RealHandle));
	memset(&m_AudioPro, 0, sizeof(NETDVR_AudioProperty_t));
	m_PlayBackHandle = 0;

	m_GetFileFrmHandle = 0;
	if (NETDVR_SUCCESS == NETDVR_startup())
	{
		//AfxMessageBox("NETDVR startup successfully!");
	}
	else
		AfxMessageBox("NETDVR startup failed!");
	curr=0;
	id=0;
	exit=false;

//	OnBTlogin();
//	OnRegConnLost();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMfcDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMfcDemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMfcDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CMfcDemoDlg::DestroyWindow() 
{
	if (m_hDvr)
	{
		exit=true;
		for (int i=0;i<16;i++)
		{
			PostThreadMessage(palyid[i], esc, NULL, NULL);		
		}
		
		int ret = NETDVR_destroyDVR(m_hDvr);
		m_hDvr = 0;
	}
	int time1 = GetTickCount();
	
	if (NETDVR_SUCCESS == NETDVR_cleanup())
	{
		TRACE(_T("time %d\n"), GetTickCount() - time1);
//		AfxMessageBox("NETDVR cleanup successfully!");
	}
	else
		AfxMessageBox("NETDVR cleanup failed!");

	//_CrtDumpMemoryLeaks();
	return CDialog::DestroyWindow();
}

void __stdcall CBReconnMsg(enum RECONNMSG msg, unsigned int dwContent)
{
	FILE* file=fopen("f:\\msg.txt","at");
	fprintf(file,"msg %d \n", msg);
	fflush(file);
	fclose(file);
}

CDDrawShow	m_ddraw[MAX_PLAYWND];
void CMfcDemoDlg::OnBTlogin() 
{
	exit=false;
	UpdateData(TRUE);

	char addbuf[80] = {0};
	GetDlgItem(IDC_ServerAddr)->GetWindowText(addbuf, 80);

	DWORD serverip = inet_addr(addbuf);

	CString strPort;
	GetDlgItem(IDC_ServerPort)->GetWindowText(strPort);
	WORD port = atoi(strPort);


	int ret = NETDVR_createDVR(&m_hDvr, serverip, port);
 	if (NETDVR_SUCCESS != ret)
	{
		CString szErr;
		szErr.Format("open failed! ret = %d",ret);
		AfxMessageBox(szErr);
		return;
	}


	struct NETDVR_loginInfo_t logininfo;
	memset(&logininfo, 0, sizeof(struct NETDVR_loginInfo_t));
	
	GetDlgItem(IDC_UserID)->GetWindowText(logininfo.username, sizeof(logininfo.username));
	GetDlgItem(IDC_UserPW)->GetWindowText(logininfo.loginpass, sizeof(logininfo.loginpass));


	ret = NETDVR_loginServer(m_hDvr, &logininfo);
	if (NETDVR_SUCCESS != ret)
	{
		CString szErr;
		szErr.Format("login failed! ret = %d",ret);
		AfxMessageBox(szErr);
		return;
	}
	

//	MessageBox("login ok ");

	memset(&m_devinfo, 0, sizeof(m_devinfo));
	ret = NETDVR_GetDeviceInfo(m_hDvr, &m_devinfo);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("NETDVR_GetDeviceInfo err!");
		return;
	}

	NETDVR_regCBMsgReconnect(m_hDvr, CBReconnMsg, 0);
	NETDVR_setReconnectFlag(m_hDvr, 1);

	CString info;
	in_addr in;
	in.s_addr = m_devinfo.deviceIP;
	info.Format("IP = %s\nPort = %d\ndevicename:%s\nmode:%s\nchnnum:%d\naudio:%d\nsubstream:%d\nplayback:%d\nalarmin:%d\nalarmout:%d\nhdd:%d\n", 
				inet_ntoa(in), 
				m_devinfo.devicePort, 
				m_devinfo.device_name,
				m_devinfo.device_mode,
				m_devinfo.maxChnNum,
				m_devinfo.maxAudioNum,
				m_devinfo.maxSubstreamNum,
				m_devinfo.maxPlaybackNum,
				m_devinfo.maxAlarmInNum,
				m_devinfo.maxAlarmOutNum,
				m_devinfo.maxHddNum);

//	AfxMessageBox(info);

	for (int i=0; i<MAX_PLAYWND; i++)
	{
		BOOL bret = m_ddraw[i].CreateDDrawObj(YV12, GetDlgItem(IDC_PLAYWND1+i)->GetSafeHwnd(), 704, 576);
	}

	GetDlgItem(IDC_ServerAddr)->EnableWindow(FALSE);
	GetDlgItem(IDC_ServerPort)->EnableWindow(FALSE);
	GetDlgItem(IDC_UserID)->EnableWindow(FALSE);
	GetDlgItem(IDC_UserPW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTlogin)->EnableWindow(FALSE);

	struct NETDVR_ComParam_t p_para;
	NETDVR_GetComParam(m_hDvr,1,&p_para);
	printf("@@@@@@@@@@@com_baudrate = %d\n",p_para.com_baudrate);
	
}

void CMfcDemoDlg::OnBTlogout() 
{
	exit=true;
	for (int i=0;i<16;i++)
	{
		PostThreadMessage(palyid[i], esc, NULL, NULL);		
	}
	UpdateData(TRUE);
	int ret = NETDVR_destroyDVR(m_hDvr);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("close failed!");
		return;
	}
	m_hDvr = 0;

	for (i=0; i<MAX_PLAYWND; i++)
	{
		m_ddraw[i].DestroyDDrawObj();
	}

	InvalidateRect(NULL);
	GetDlgItem(IDC_ServerAddr)->EnableWindow(TRUE);
	GetDlgItem(IDC_ServerPort)->EnableWindow(TRUE);
	GetDlgItem(IDC_UserID)->EnableWindow(TRUE);
	GetDlgItem(IDC_UserPW)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTlogin)->EnableWindow(TRUE);
}

void CALLBACK DealDecVideoFrame(pFrameHeadrDec pFrmHdrDec, unsigned int dwContext)
{
	//return;
	TRACE(_T("WndNum %d\n"), dwContext);
	dwContext %= 4;
	if (dwContext<MAX_PLAYWND && pFrmHdrDec->mediaType==MEDIA_TYPE_H264)
	{
		LPBYTE lpY = (LPBYTE)pFrmHdrDec->data;
		LPBYTE lpV = (LPBYTE)pFrmHdrDec->data+pFrmHdrDec->video_param.height*pFrmHdrDec->video_param.width;
		LPBYTE lpU = (LPBYTE)pFrmHdrDec->data+pFrmHdrDec->video_param.height*pFrmHdrDec->video_param.width*5/4;
		m_ddraw[dwContext].DrawYV12byDraw7(lpY, lpV, lpU, pFrmHdrDec->video_param.width, pFrmHdrDec->video_param.height);
	}
}

void CALLBACK get_encframe(pFrameHeadr pFrmHdr, unsigned int dwContextEnc)
{
	//do something	
	if (pFrmHdr->mediaType != MEDIA_TYPE_H264)
	{
//		return;
	}

	TRACE("w:%d, h:%d, t:%d, s=%d\n", pFrmHdr->videoParam.videoWidth, pFrmHdr->videoParam.videoHeight, pFrmHdr->timeStamp, pFrmHdr->dataSize);
	FILE * fp = fopen("c:\\123.bin", "ab");
	if (fp)
	{
		fwrite(pFrmHdr->pData, 1, pFrmHdr->dataSize, fp);
		fflush(fp);
		fclose(fp);
	}
}

void CMfcDemoDlg::OnPlayAll() 
{
	UpdateData(TRUE);
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBOVideoType))->GetCurSel();
	
	if (nSel == 0) //main stream
	{	
	
		for(int i = 0; i < min(MAX_PLAYWND, m_devinfo.maxChnNum); i++)
		{
		
			RealPlayClientInfo_t rpci;
			rpci.rcv_chn = i;
			rpci.streamtype = 0;
			rpci.pEncFrameCBFunc = NULL/*get_encframe*/;
			rpci.dwEncFrameContent = i;
			rpci.pDecFrameCBFunc = DealDecVideoFrame;
			rpci.dwDecFrameContent = i;

			int ret = NETDVR_StartRealPlay(m_hDvr, &rpci, &m_RealHandle[i]);
			if (NETDVR_SUCCESS != ret)
			{
				CString szErr;
				szErr.Format("NETDVR_StartRealPlay failed! ret = %d",ret);
				AfxMessageBox(szErr);
				return;
			}
			
		}
	} 
	else //sub stream
	{
		for(int i = 0; i < min(MAX_PLAYWND, m_devinfo.maxSubstreamNum); i++)
		{

			RealPlayClientInfo_t rpci;
			rpci.rcv_chn = i;
			rpci.streamtype = 1;
			rpci.pEncFrameCBFunc = NULL/*get_encframe*/;
			rpci.dwEncFrameContent = i;
			rpci.pDecFrameCBFunc = DealDecVideoFrame;
			rpci.dwDecFrameContent = i;
			
			int ret = NETDVR_StartRealPlay(m_hDvr, &rpci, &m_RealHandle[i]);
			if (NETDVR_SUCCESS != ret)
			{
				CString szErr;
				szErr.Format("NETDVR_StartRealPlay failed! ret = %d",ret);
				AfxMessageBox(szErr);
				return;
			}
		}
	}
	GetDlgItem(IDC_COMBOVideoType)->EnableWindow(FALSE);
}

void CMfcDemoDlg::OnStopAll() 
{
	UpdateData(TRUE);
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBOVideoType))->GetCurSel();
	
	if (nSel == 0) //main stream
	{	
	
		for(int i = 0; i < min(MAX_PLAYWND, m_devinfo.maxChnNum); i++)
		{

			int ret = NETDVR_StopRealPlay(m_RealHandle[i]);
			m_RealHandle[i] = 0;
			if (NETDVR_SUCCESS != ret)
			{
				CString szErr;
				szErr.Format("NETDVR_StopRealPlay failed! ret = %d",ret);
				AfxMessageBox(szErr);
				return;
			}
	
		}
	} 
	else //sub stream
	{
		for(int i = 0; i < min(MAX_PLAYWND, m_devinfo.maxSubstreamNum); i++)
		{
			int ret = NETDVR_StopRealPlay(m_RealHandle[i]);
			if (NETDVR_SUCCESS != ret)
			{
				CString szErr;
				szErr.Format("NETDVR_StopRealPlay failed! ret = %d",ret);
				AfxMessageBox(szErr);
				return;
			}
			
		}
	}
	InvalidateRect(NULL);
	GetDlgItem(IDC_COMBOVideoType)->EnableWindow(TRUE);
}

int nFileIndex = 0;
void CALLBACK RecFilenameCallBack(char *p_filename, unsigned int dwContext)
{
	sprintf(p_filename, "TestRec%d.ifv", nFileIndex);
	nFileIndex++;
}

BOOL is_video_frame(unsigned char media_type)
{
	switch (media_type)
	{
	case MEDIA_TYPE_H264:
		return TRUE;
	case MEDIA_TYPE_MP4:
		return TRUE;
	case MEDIA_TYPE_H261:
		return TRUE;
	case MEDIA_TYPE_H263:
		return TRUE;
	case MEDIA_TYPE_MJPEG:
		return TRUE;
	case MEDIA_TYPE_MP2:
		return TRUE;
	}
	
	return FALSE;
}

#define O_R				0
#define O_W_CREAT		1
#define O_W_APPEND		2
#define O_W_AUTO		3
#define PATH_LEN_MAX		260
#define FILENAME_LEN_MAX	260
int cur_length = 0;
unsigned int first_stamp = 0;
char* pWriteBuf = NULL;
void CALLBACK deal_frame_record(pFrameHeadr pFrmHdr, unsigned int dwContent)
{
	CMfcDemoDlg *pThis = (CMfcDemoDlg *)dwContent;
	unsigned char b_video;	

	if (!pFrmHdr)
	{
		return;
	}

	b_video = is_video_frame(pFrmHdr->mediaType);
	
	if (pThis->file)
	{
		if ((TL_FileEndPosition(pThis->file) + pFrmHdr->dataSize) > ((128<<20) - (1<<19)))
		{
			if (pThis->file)
			{
				TL_CloseFile(pThis->file);
				pThis->file = NULL;
				if (NULL != pWriteBuf)
				{
					free(pWriteBuf);
					pWriteBuf = NULL;
				}
			}
		}
	}
	
	//to open record file
	if (NULL == pThis->file)
	{
		if (!b_video)
		{
			return;
		}
		
		char record_file[PATH_LEN_MAX + FILENAME_LEN_MAX];
		
		strcpy(record_file, "C:\\test");
		char filename[FILENAME_LEN_MAX] = {0};
		SYSTEMTIME SystemTime;
		GetLocalTime(&SystemTime);
		wsprintf(filename, TEXT("%04d%02d%02d%02d%02d%02d.ifv"), SystemTime.wYear, SystemTime.wMonth
			, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
		
		strcat(record_file, "\\");
		strcat(record_file, filename);
		
		unsigned int nMaxWriteBufLen = 1<<20;
		pWriteBuf = (char*)malloc(nMaxWriteBufLen);
		if (NULL == pWriteBuf)
		{
			return;
		}
		memset(pWriteBuf, 0, nMaxWriteBufLen);
		pThis->file = TL_OpenFile(record_file, TL_FILE_CREATE, TL_FILE_WRITEWITHBUF, pWriteBuf, nMaxWriteBufLen);
		if (NULL == pThis->file)
		{
			return;
		}	
		
		{
			//if (p_record_para->p_audio_property)
			{
				unsigned short bits = pThis->m_AudioPro.audioBitPerSample;
				unsigned int samplepersec = pThis->m_AudioPro.audioSamplePerSec;
				unsigned int size = pThis->m_AudioPro.audioFrameSize;
				unsigned int dur = pThis->m_AudioPro.audioFrameDurTime;
				

				TL_FileSetAudio(pThis->file, 1, bits, samplepersec, TLFILE_A_GRAW, size, dur);
			}
			
			TL_FileSetVideo(pThis->file, 
				pFrmHdr->videoParam.videoWidth, 
				pFrmHdr->videoParam.videoHeight, 
				pFrmHdr->frameRate,  
				TLFILE_V_H264
				);		
		}	
		
		first_stamp = pFrmHdr->timeStamp;
	}
	
	if (b_video)
	{		
		int ret = TL_FileWriteVideoFrame(pThis->file, 
			pFrmHdr->pData, 
			pFrmHdr->dataSize, 
			pFrmHdr->timeStamp - first_stamp, 
			(UINT8)pFrmHdr->videoParam.keyFrame,
			pFrmHdr->frameID
			);		
	}
	else
	{
		TL_FileWriteAudioFrame(pThis->file, 
			pFrmHdr->pData, 
			pFrmHdr->dataSize, 
			pFrmHdr->timeStamp - first_stamp);
	}
	
	cur_length += pFrmHdr->dataSize; 
}

void CMfcDemoDlg::OnStartRec() 
{
	UpdateData(TRUE);

	int ret = 0;
	for(int i = 0; i < 1; i++)
	{
		
#if 1
		//set filename ,filepath,and max file size
		ret = NETDVR_GetAudioProperty(m_hDvr/*m_RealHandle[i]*/, &m_AudioPro);
		ret = NETDVR_setRecordFileNameCB(m_RealHandle[i], RecFilenameCallBack, 0);
		ret = NETDVR_setRecordCB(m_RealHandle[i], deal_frame_record, int(this));
		ret = NETDVR_startRecord(m_RealHandle[i], "c:\\testrecord", 2*1024*1024);
#else
		//default path is D:\\NETDVR
		//default max file size is 134217728 Byte
		ret = NETDVR_startRecord(m_RealHandle[i], NULL, 0);		
#endif
		if (NETDVR_SUCCESS == ret)
		{
			AfxMessageBox("startRecord successfully! \test path is C:\\test\\ ");
		}
		else
		{
			AfxMessageBox("startRecord failed!");
		}
	}

	
}

void CMfcDemoDlg::OnStopRec() 
{
	UpdateData(TRUE);
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBOVideoType))->GetCurSel();

	for(int i = 0; i < 1; i++)
	{
#if 1
		int ret = NETDVR_stopRecord(m_RealHandle[i]);
		TL_CloseFile(file);
		file = NULL;
		if (NULL != pWriteBuf)
		{
			free(pWriteBuf);
			pWriteBuf = NULL;
		}
#else
		int ret = NETDVR_stopRecord(m_RealHandle[i]);
#endif
	
	}

}

void CMfcDemoDlg::OnSnap() 
{
	UpdateData(TRUE);

#if 1
		//snap to jpg
		//default path is D:\\NETDVR
		int ret = NETDVR_snapshot(m_RealHandle[0], NULL/*c:\\*/, "test.jpg");
#else	

		//snap to bmp
		//default file name is like :dvr1chn120090313110434.bmp

		ret = NETDVR_snapshot(m_RealHandle[0], NULL, NULL);
		//ret = NETDVR_snapshot(m_hDvr, 0, NULL, "test.bmp");
#endif

	
}

void CMfcDemoDlg::OnPlayAudio() 
{	
	for(int i = 0; i < 1; i++)
	{	
		
		int ret = NETDVR_OpenRealAudio(m_RealHandle[i]);
		if (NETDVR_SUCCESS != ret)
		{
			AfxMessageBox("NETDVR_OpenRealAudio failed!");
			return;
		}
	}
		
}

void CMfcDemoDlg::OnStopAudio() 
{
	for(int i = 0; i < 1; i++)
	{	
	
		int ret = NETDVR_CloseRealAudio(m_RealHandle[i]);
		if (NETDVR_SUCCESS != ret)
		{
			AfxMessageBox("NETDVR_CloseRealAudio failed!");
			return;
		}

	}
	
}

void CMfcDemoDlg::OnSelchangeCOMBOVideoType() 
{
	UpdateData(TRUE);
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBOVideoType))->GetCurSel();
	
	if (nSel == 0)
	{
		GetDlgItem(IDC_PlayAudio)->EnableWindow(TRUE);
		GetDlgItem(IDC_StopAudio)->EnableWindow(TRUE);
	} 
	else if (nSel == 1)
	{
		GetDlgItem(IDC_PlayAudio)->EnableWindow(FALSE);
		GetDlgItem(IDC_StopAudio)->EnableWindow(FALSE);
	}
	
}

bool bMute = FALSE;
void CMfcDemoDlg::OnMute() 
{
	bMute = !bMute;
	for (int i=0; i<1; i++)
	{
		int ret = NETDVR_MuteRealAudio(m_RealHandle[i], bMute);
	}

	
}

void CMfcDemoDlg::OnStartVoip() 
{
	int ret;

	
	ret = NETDVR_startVOIP(m_hDvr, 0);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("startVOIP failed!");
		return;
	}
}

void CMfcDemoDlg::OnStopVoip() 
{
	int ret;
	
	ret = NETDVR_stopVOIP(m_hDvr, 0);
	
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("NETDVR_stopVOIP failed!");
		return;
	}

}

struct NETDVR_recFileSearchResult_t rec_desc = {NULL};
#include <sys/timeb.h>
void CMfcDemoDlg::OnRecSearch() 
{
	struct NETDVR_fileSearchCondition_t rfs;
	
	rfs.chn = 1;

	struct tm tmtemp;
	memset(&tmtemp, 0, sizeof(tmtemp));
	
	tmtemp.tm_year = 2011-1900;
	tmtemp.tm_mon = 0;
	tmtemp.tm_mday = 1;
	tmtemp.tm_hour =0;
	tmtemp.tm_min = 0;
	tmtemp.tm_sec = 0;

	rfs.start_time = mktime(&tmtemp); //2009-1-1 00:00:00

	struct timeb tb;
	ftime(&tb);
	rfs.start_time -= tb.timezone*60;

	tmtemp.tm_year = 2011-1900;
	tmtemp.tm_mon = 9;
	tmtemp.tm_mday = 18;
	tmtemp.tm_hour = 0;
	tmtemp.tm_min = 0;
	tmtemp.tm_sec = 0;

	rfs.end_time = mktime(&tmtemp); //2010-1-1 00:00:00
	rfs.end_time -= tb.timezone*60;
	rfs.startID = 1;  //must >= 1
	rfs.max_return = 24; //must <= 24
	rfs.type = NETDVR_REC_INDEX_ALL;
	
	if (rec_desc.sum != 0)
	{
		NETDVR_recFilesSearchClean(&rec_desc);
	}

	int ret = NETDVR_recFilesSearch(m_hDvr, &rfs, &rec_desc);
	if (NETDVR_SUCCESS == ret)
	{
		NETDVR_recFileInfo_t* p_FileInfo = NULL;
		//get data
		CString szInfo;
		szInfo.Format("FileSearch successfully!\n startID = %d, endID = %d, sum = %d\n",
			rec_desc.startID, rec_desc.endID, rec_desc.sum);

		while(rec_desc.endID != rec_desc.sum)
		{
			rfs.startID = rec_desc.endID + 1;
			NETDVR_recFilesSearchClean(&rec_desc);
			ret = NETDVR_recFilesSearch(m_hDvr, &rfs, &rec_desc);
			if (ret == NETDVR_SUCCESS)
			{
				//get data
			}
			else
			{
				CString szInfo;
				szInfo.Format("FileSearch failed!\n startID = %d, endID = %d, sum = %d\n",
				rec_desc.startID, rec_desc.endID, rec_desc.sum);
				AfxMessageBox(szInfo);
			}
		}

		AfxMessageBox(szInfo, MB_ICONINFORMATION); 		
	}
	else
	{
		AfxMessageBox("recFilesSearch failed!");
		return;
	}	
}

void CMfcDemoDlg::OnSearchClean() 
{
	int ret = NETDVR_recFilesSearchClean(&rec_desc);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("recFilesSearchClean failed!");
		return;
	}
}


CDDrawShow	m_ddrawPlayer;
void CALLBACK get_playbackframe(pFrameHeadrDec pFrmHdrDec, unsigned int dwContext)
{
	if (pFrmHdrDec->mediaType == BYTE(MEDIA_TYPE_H264))
	{
		LPBYTE lpY = (LPBYTE)pFrmHdrDec->data;
		LPBYTE lpV = (LPBYTE)pFrmHdrDec->data+pFrmHdrDec->video_param.height*pFrmHdrDec->video_param.width;
		LPBYTE lpU = (LPBYTE)pFrmHdrDec->data+pFrmHdrDec->video_param.height*pFrmHdrDec->video_param.width*5/4;
		m_ddrawPlayer.DrawYV12byDraw7(lpY, lpV, lpU, pFrmHdrDec->video_param.width, pFrmHdrDec->video_param.height);
	}

}

void CALLBACK playovermsg(unsigned int dwContent)
{
	AfxMessageBox("Play Over!",MB_ICONINFORMATION);
}

unsigned int oldpos = 0;
void CALLBACK playprogress(struct NETDVR_progressParam_t progress, unsigned int dwContent)
{

//	AfxMessageBox("1");
//	TRACE("total 0x%08x \n", progress.total_size);
	unsigned int pos = progress.curr_pos*100/progress.total_size;

	if (pos != oldpos)
	{
		CMfcDemoDlg* pThis = (CMfcDemoDlg*)dwContent;
		if (pThis)
		{
			pThis->m_ctrSlider.SetPos(pos);
		}
	}
	oldpos = pos;

}

BOOL bFilePlay = FALSE;
BOOL bTimePlay = FALSE;
BOOL bGetFileFrame = FALSE;

void CMfcDemoDlg::OnFilePlay() 
{
	if (rec_desc.sum == 0)
	{
		AfxMessageBox("no rec file!");
		return;
	}

	if (bTimePlay)
	{
		AfxMessageBox(" it's being time play !");
		return;
	}
	bFilePlay = !bFilePlay;
	if (bFilePlay)
	{
		int ret;

		
		//test the first file!
		DWORD dwWidth, dwHeight;
		if (rec_desc.precInfo)
		{
			switch(rec_desc.precInfo->image_format)
			{
			case 3: 
				dwWidth = 352;
				dwHeight = 288;
			break;
			case 4: 
				dwWidth = 704;
				dwHeight = 576;
			break;
			case 8: 
				dwWidth = 352;
				dwHeight = 240;
			break;
			case 9: 
				dwWidth = 704;
				dwHeight = 480;
			break;
			}
		}

// 		CString szTmp;
// 		szTmp.Format("%d %d", dwWidth, dwHeight);
// 		AfxMessageBox(szTmp);
		m_ddrawPlayer.CreateDDrawObj(YV12, GetDlgItem(IDC_PLAYWND5)->GetSafeHwnd(), dwWidth, dwHeight);
		m_ctrSlider.SetRange(0,100);
		m_ctrSlider.SetPos(0);

		PlayBackClientInfo_t pbci_t={0};
		pbci_t.sendmode = 1;
		pbci_t.pEncFrameCBFunc = get_encframe;
		pbci_t.dwEncFrameContent = 0;
		pbci_t.pDecFrameCBFunc = get_playbackframe;
		pbci_t.dwDecFrameContent = 0;
		pbci_t.pPlayOverCBFunc = playovermsg;
		pbci_t.dwPlayOverContent = 0;
		pbci_t.pProgressCBFunc = playprogress;
		pbci_t.dwProgressContent = 0;

		ret = NETDVR_startPlayByFile(m_hDvr, rec_desc.precInfo, &pbci_t, &m_PlayBackHandle);
		if (NETDVR_SUCCESS == ret)
		{
			NETDVR_playProgress(m_PlayBackHandle, TRUE);
			
		}
		else
		{
			m_ddrawPlayer.DestroyDDrawObj();
			m_ctrSlider.SetPos(0);

			AfxMessageBox("startRemoteFilePlay failed.");
			ret = NETDVR_stopPlayBack(m_PlayBackHandle);
			if (NETDVR_SUCCESS != ret)
			{
				AfxMessageBox("closePlayer failed!");
			}
			
		}

		GetDlgItem(IDC_FilePlay)->SetWindowText("StopFilePlay");
	} 
	else
	{
		m_ddrawPlayer.DestroyDDrawObj();
		GetDlgItem(IDC_FilePlay)->SetWindowText("FilePlay");


		int ret = NETDVR_stopPlayBack(m_PlayBackHandle);
		if (NETDVR_SUCCESS != ret)
		{
			CString szErr;
			szErr.Format("stopFilePlay failed. %d", ret);
			AfxMessageBox(szErr);
			//AfxMessageBox("stopFilePlay failed.");
			return;
		}
		m_ctrSlider.SetPos(0);
		InvalidateRect(NULL);

		
	}
	
}

void CMfcDemoDlg::OnTimePlay() 
{
	if (bFilePlay)
	{
		AfxMessageBox(" it's being file play !");
		return;
	}
	bTimePlay = !bTimePlay;
	if (bTimePlay)
	{
		int ret;
		
		m_ddrawPlayer.CreateDDrawObj(YV12, GetDlgItem(IDC_PLAYWND5)->GetSafeHwnd(), 704, 576);
		m_ctrSlider.SetRange(0,100);
		m_ctrSlider.SetPos(0);
		
		NETDVR_TimePlayCond_t tps;
		tps.chn = 0;
		
		struct tm tmtemp;
		memset(&tmtemp, 0, sizeof(tmtemp));
		
		tmtemp.tm_year = 2011-1900;
		tmtemp.tm_mon = 0;
		tmtemp.tm_mday = 1;
		
		tps.start_time = mktime(&tmtemp); //2009-1-1 00:00:00
		
		tmtemp.tm_year = 2013-1900;
		tps.end_time = mktime(&tmtemp); //2010-1-1 00:00:00

		tps.type = NETDVR_REC_INDEX_ALL;

		PlayBackClientInfo_t pbci_t={0};
		pbci_t.sendmode = 0;
		pbci_t.pEncFrameCBFunc = get_encframe;
		pbci_t.dwEncFrameContent = 0;
		pbci_t.pDecFrameCBFunc = get_playbackframe;
		pbci_t.dwDecFrameContent = 0;
		pbci_t.pPlayOverCBFunc = playovermsg;
		pbci_t.dwPlayOverContent = 0;
		pbci_t.pProgressCBFunc = playprogress;
		pbci_t.dwProgressContent = 0;

		ret = NETDVR_startPlayByTime(m_hDvr, &tps, &pbci_t, &m_PlayBackHandle);
		if (NETDVR_SUCCESS == ret)
		{
			NETDVR_playProgress(m_PlayBackHandle, TRUE);
			
		}
		else
		{
			m_ddrawPlayer.DestroyDDrawObj();
			m_ctrSlider.SetPos(0);
			
			AfxMessageBox("startRemoteFilePlay failed.");
			ret = NETDVR_stopPlayBack(m_PlayBackHandle);
			if (NETDVR_SUCCESS != ret)
			{
				AfxMessageBox("closePlayer failed!");
			}
			
		}
		
		GetDlgItem(IDC_TimePlay)->SetWindowText("StopTimeePlay");
	} 
	else
	{
		m_ddrawPlayer.DestroyDDrawObj();
		GetDlgItem(IDC_TimePlay)->SetWindowText("TimePlay");

		
		int ret = NETDVR_stopPlayBack(m_PlayBackHandle);
		if (NETDVR_SUCCESS != ret)
		{
			AfxMessageBox("stopTimePlay failed.");
			return;
		}
		
		m_ctrSlider.SetPos(0);	
		InvalidateRect(NULL);
	}
}

void CMfcDemoDlg::OnPbPause() 
{
	int ret = NETDVR_pausePlay(m_PlayBackHandle);
	
}



void CMfcDemoDlg::OnPbResume() 
{
	int ret = NETDVR_resumePlay(m_PlayBackHandle);
	
}

void CMfcDemoDlg::OnPbStep() 
{
	int ret = NETDVR_singleFramePlay(m_PlayBackHandle);
	
}

void CMfcDemoDlg::OnPbFast() 
{
	int ret = NETDVR_fastPlay(m_PlayBackHandle);
	
}

void CMfcDemoDlg::OnPbSlow() 
{
	int ret = NETDVR_slowPlay(m_PlayBackHandle);	
}

void CMfcDemoDlg::OnPbRate() 
{
	AfxMessageBox("this sample set rate to 2.0X");
	int ret = NETDVR_setPlayRate(m_PlayBackHandle, 2);	
	
}

void CMfcDemoDlg::OnPbPrev() 
{
	int ret = NETDVR_playPrevious(m_PlayBackHandle);
	
}

void CMfcDemoDlg::OnPbNext() 
{
	int ret = NETDVR_playNext(m_PlayBackHandle);
	
}

void CMfcDemoDlg::OnPbSeek() 
{
	AfxMessageBox("this sample seek to 50%");

	unsigned int totaltime = rec_desc.precInfo->end_time - rec_desc.precInfo->start_time; //sec
	unsigned int newpos =(totaltime*50/100)*1000; //ms

	int ret = NETDVR_playSeek(m_PlayBackHandle, newpos);	
	
}

void CMfcDemoDlg::OnPbMute() 
{
	int ret = NETDVR_playMute(m_PlayBackHandle, TRUE);
	
}



void CALLBACK callbackDLProgress(struct NETDVR_progressParam_t progress, unsigned int dwContent)
{
	if (progress.curr_pos >= progress.total_size)
	{
		AfxMessageBox("Download finished !\n");
	}
}

void CALLBACK callbackDLError(unsigned short err_code, unsigned int dwContent)
{
	if (err_code != 0)
	{
		CString	szErr;
		szErr.Format("Download err_code  = %d", err_code);
		AfxMessageBox(szErr);
	}
}
BOOL bDownload = FALSE;
void CMfcDemoDlg::OnDownLoad() 
{
	if (rec_desc.sum == 0)
	{
		AfxMessageBox("no rec file!");
		return;
	}

	bDownload = !bDownload;

	if (bDownload)
	{
		AfxMessageBox("this sample will download file and save to c:\\testdownload.ifv");
		int ret = NETDVR_regCBFileRecieverProgress(m_hDvr, callbackDLProgress, 0);
		ret = NETDVR_regCBFileRecieverError(m_hDvr, callbackDLError, 0);
		
		ret = NETDVR_startFileDownload(m_hDvr,  "c:\\", "testdownload.ifv", rec_desc.precInfo);
		if (NETDVR_SUCCESS != ret)
		{
			AfxMessageBox("NETDVR_startFileDownload failed!");
		}
		GetDlgItem(IDC_DownLoad)->SetWindowText("StopDL");
	} 
	else
	{
		GetDlgItem(IDC_DownLoad)->SetWindowText("DownLoad");
		int ret;
		
		ret = NETDVR_stopFileDownload(m_hDvr);
		if (NETDVR_SUCCESS != ret)
		{
			AfxMessageBox("NETDVR_stopFileDownload failed!");
			return;
		}

	}
}

void CMfcDemoDlg::OnPtzParam() 
{
	NETDVR_ptzParam_t ptzParam;
	memset(&ptzParam, 0, sizeof(ptzParam));

	//get ptzparam
	unsigned char chn = 0;
	int ret = NETDVR_getPtzParams(m_hDvr, chn, &ptzParam);
	if (NETDVR_SUCCESS == ret)
	{
		CString szInfo;
		szInfo.Format("Get PtzParam Success!\n BuadRate = %d,\n DataBitSel = %d,\n StopBitSel = %d,\n Checktype = %d,\n FlowCtr = %d,\n ProtocolType = %d,\n ", 
			ptzParam.baud_ratesel, ptzParam.data_bitsel, ptzParam.stop_bitsel, ptzParam.check_type, ptzParam.flow_control, ptzParam.protocol);
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{

		AfxMessageBox("getPtzParams  failed!");
		return;
	}

	//Set ptzparam
	ret = NETDVR_setPtzParams(m_hDvr, &ptzParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setPtzParams success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setPtzParams failed");
	}
	
}

void CMfcDemoDlg::OnPtzCtr() 
{
	AfxMessageBox("this sample set Ptz turn left!",MB_ICONINFORMATION);

	NETDVR_PtzCtrl_t ptzctr;
	memset(&ptzctr,0,sizeof(ptzctr));
	ptzctr.chn = 0;
	ptzctr.cmd = NETDVR_PTZ_COM_MOVELEFT;
	int ret = NETDVR_PtzControl(m_hDvr, &ptzctr);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("startPtzControl failed");
	}
}

void CMfcDemoDlg::OnPtzStop() 
{
	NETDVR_PtzCtrl_t ptzctr;
	memset(&ptzctr,0,sizeof(ptzctr));
	ptzctr.chn = 0;
	ptzctr.cmd = NETDVR_PTZ_COM_STOP;
	int ret = NETDVR_PtzControl(m_hDvr, &ptzctr);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("stopPtzControl failed");
	}
}

void CMfcDemoDlg::OnSetPreset() 
{
	unsigned char chn = 0;
	unsigned char presetpoint_pos = 1;
	int ret = NETDVR_SetYTPresetPoint(m_hDvr, chn, presetpoint_pos, NETDVR_YT_COM_ADDPRESETPOINT);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("setYuntaiPresetPoint failed");
	}	
}

void CMfcDemoDlg::OnGoPreset() 
{
	unsigned char chn = 0;
	unsigned char presetpoint_pos = 1;
	int ret = NETDVR_SetYTPresetPoint(m_hDvr, chn, presetpoint_pos, NETDVR_YT_COM_TOPRESETPOINT);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("goYuntaiPresetPoint failed");
	}
}

void CMfcDemoDlg::OnClearPreset() 
{
	unsigned char chn = 0;
	unsigned char presetpoint_pos = 1;
	int ret = NETDVR_SetYTPresetPoint(m_hDvr, chn, presetpoint_pos, NETDVR_YT_COM_DELPRESETPOINT);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("clearYuntaiPresetPoint failed");
	}
}

void CMfcDemoDlg::OnStartRecTrack() 
{
	unsigned char chn = 0;

	int ret = NETDVR_SetYTTrack(m_hDvr, chn, NETDVR_YT_COM_STARTRECORDTRACK);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("startRecordYuntaiTrack failed");
	}
}

void CMfcDemoDlg::OnStopRecTrack() 
{
	unsigned char chn = 0;
	
	int ret = NETDVR_SetYTTrack(m_hDvr, chn, NETDVR_YT_COM_STOPRECORDTRACK);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("stopRecordYuntaiTrack failed");
	}
	
}

void CMfcDemoDlg::OnStartTrack() 
{
	unsigned char chn = 0;
	
	int ret = NETDVR_SetYTTrack(m_hDvr, chn, NETDVR_YT_COM_STARTTRACK);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("startYuntaiTrack failed");
	}
}

void CMfcDemoDlg::OnStopTrack() 
{
	unsigned char chn = 0;
	
	int ret = NETDVR_SetYTTrack(m_hDvr, chn, NETDVR_YT_COM_STOPTRACK);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("stopYuntaiTrack failed");
	}
}

struct NETDVR_cruisePath_t cruisePath = {0};
void CMfcDemoDlg::OnInsertCruise() 
{
	unsigned char chn = 0;

	cruisePath.chn = chn;
	cruisePath.path_no = 1;
	cruisePath.cruise_pos[0].preset_no = 1;
	cruisePath.cruise_pos[0].dwell_time = 20;
	cruisePath.cruise_pos[0].cruise_speed = 5;
	cruisePath.cruise_pos[0].cruise_flag = 1;

	int ret = NETDVR_SetCruiseParam(m_hDvr, &cruisePath);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("insertYuntaiCruisePos failed");
	}
}

void CMfcDemoDlg::OnDeleteCruise() 
{
	unsigned char chn = 0;
	
	cruisePath.chn = chn;
	cruisePath.path_no = 1;
	cruisePath.cruise_pos[0].cruise_flag = 2;
	int ret = NETDVR_SetCruiseParam(m_hDvr, &cruisePath);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("deleteYuntaiCruisePos failed");
	}
}

void CMfcDemoDlg::OnStartCruise() 
{
	unsigned char chn = 0;
	int ret = NETDVR_startYuntaiCruise(m_hDvr, chn, 1);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("startYuntaiCruise failed");
	}	
}

void CMfcDemoDlg::OnStopCruise() 
{
	unsigned char chn = 0;
	int ret = NETDVR_stopYuntaiCruise(m_hDvr, chn, 1);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("stopYuntaiCruise failed");
	}
}

void CMfcDemoDlg::OnParamSys() 
{
	NETDVR_systemParam_t sysParam;
	memset(&sysParam, 0, sizeof(sysParam));
	
	//get sysParam

	int ret = NETDVR_getSystemParams(m_hDvr, &sysParam);
	if (NETDVR_SUCCESS == ret)
	{
		CString szInfo;
		szInfo.Format("GetSystemParams Success!\n Device_id: %d,\n Device_name: %s,\n overlap: %d,\n statusdisp: %d,\n lock_time: %d,\n switch_time: %d,\n video_format: %d,\n transparency: %d,\n language: %d,\n",
			sysParam.device_id, sysParam.device_name, sysParam.flag_overlap, 
			sysParam.flag_statusdisp, sysParam.lock_time, sysParam.switch_time, sysParam.video_format,
			sysParam.transparency, sysParam.languageindex
			);

		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getSystemParams  failed!");
		return;
	}

	//Set sysParam
	ret = NETDVR_setSystemParams(m_hDvr, &sysParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setSystemParams success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setSystemParams failed");
	}
	
}

void CMfcDemoDlg::OnParamVideo() 
{
	NETDVR_videoParam_t videoParam;
	memset(&videoParam, 0, sizeof(videoParam));
	
	//get videoParam
	unsigned char chn = 0;
	int ret = NETDVR_getVideoParams(m_hDvr, chn, &videoParam);
	if (NETDVR_SUCCESS == ret)
	{
		CString szInfo;
		//print some params
		szInfo.Format("getVideoParams Success!\n channel name: %s,\n name_x: %d,\n name_y: %d,\n flag_name: %d\n",
			videoParam.channelname, videoParam.chnpos_x, videoParam.chnpos_y, videoParam.flag_name
			);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("getVideoParams  failed!");
		return;
	}
	
	//Set videoParam
	ret = NETDVR_setVideoParams(m_hDvr, &videoParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setVideoParams success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setVideoParams failed");
	}
}

void CMfcDemoDlg::OnParamRec() 
{
#if 1
	NETDVR_recordParam_t recParam;
	memset(&recParam, 0, sizeof(recParam));
	
	//get recParam
	unsigned char chn = 0;
	int ret = NETDVR_getRecordParams(m_hDvr, chn, &recParam);
	if (NETDVR_SUCCESS == ret)
	{
		CString szInfo;
		//print some params
		szInfo.Format("getrecParam Success!\n code_type: %d,\n bit_type: %d,\n bit_max: %d,\n quality: %d,\n frame_rate: %d,\n resolu pos %d",
				recParam.code_type, recParam.bit_type, recParam.bit_max, recParam.quality, recParam.frame_rate,recParam.resolutionpos);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getrecParam  failed!");
		return;
	}
	//get Resolu_list
	if (recParam.supportResolu==0)
	{
		AfxMessageBox("not supportResolu ");
//		return;
	}
	NETDVR_VideoResolu_list_t m_GetVideoResolu_list;   //2011.11.12杨斌
	ret = NETDVR_getVideoResoluList(m_hDvr, recParam.channelno,0, &m_GetVideoResolu_list);
	if (NETDVR_SUCCESS == ret)
	{
		int i=0;
		CString szInfo;
		// for each(unsigned char ch in m_GetVideoResolu_list.videoresolu)
		for(int j=0;j<8;j++)
		{
			if (m_GetVideoResolu_list.videoresolu[j]!=0)
			{
				switch (m_GetVideoResolu_list.videoresolu[j])
				{
				case NETDVR_VIDEORESOLU_QCIF:
					
					szInfo+=_T("QCIF ");
					break;
				case NETDVR_VIDEORESOLU_CIF:
					szInfo+=_T("CIF ");
					break;
				case NETDVR_VIDEORESOLU_HD1:
					szInfo+=_T("HD1 ");
					break;
				case NETDVR_VIDEORESOLU_D1:
					szInfo+=_T("D1 ");
					break;
				default:
					break;
				}
				i++;
			}
		}
		AfxMessageBox(szInfo,MB_ICONINFORMATION);		
	}
	else
	{
		
		AfxMessageBox("Get Resolu_list  failed!");
		//		return;
	}
	recParam.resolutionpos=1;
	//Set recParam
	ret = NETDVR_setRecordParams(m_hDvr, &recParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setRecParam success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setRecParam failed");
	}
#endif
//子码流
#if 0
	NETDVR_SubStreamParam_t SubStrPra;
	memset(&SubStrPra, 0, sizeof(SubStrPra));
	unsigned char chn = 0;
	int	ret=NETDVR_GetSubStreamParam(m_hDvr, chn, &SubStrPra);
	if (NETDVR_SUCCESS == ret)
	{
		CString szInfo;
		//print some params
		szInfo.Format("getrecParam Success!\n resolu pos %d",
			SubStrPra.sub_flag);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getSubParam  failed!");
		return;
	}
	NETDVR_VideoResolu_list_t m_GetVideoResolu_list;   
	ret = NETDVR_getVideoResoluList(m_hDvr, SubStrPra.chn,1, &m_GetVideoResolu_list);
	if (NETDVR_SUCCESS == ret)
	{
		int i=0;
		CString szInfo;
		// for each(unsigned char ch in m_GetVideoResolu_list.videoresolu)
		for(int j=0;j<8;j++)
		{
			if (m_GetVideoResolu_list.videoresolu[j]!=0)
			{
				switch (m_GetVideoResolu_list.videoresolu[j])
				{
				case NETDVR_VIDEORESOLU_QCIF:
					
					szInfo+=_T("QCIF ");
					break;
				case NETDVR_VIDEORESOLU_CIF:
					szInfo+=_T("CIF ");
					break;
				case NETDVR_VIDEORESOLU_HD1:
					szInfo+=_T("HD1 ");
					break;
				case NETDVR_VIDEORESOLU_D1:
					szInfo+=_T("D1 ");
					break;
				default:
					break;
				}
				i++;
			}
		}
		AfxMessageBox(szInfo,MB_ICONINFORMATION);		
	}
	else
	{
		
		AfxMessageBox("Get Resolu_list  failed!");
//		return;
	}
	SubStrPra.sub_flag=(enum NETDVR_SUBFLAG)1;
	//Set recParam
// 	ret = NETDVR_SetSubStreamParam(m_hDvr, &SubStrPra);
// 	if (NETDVR_SUCCESS == ret)
// 	{
// 		AfxMessageBox("setSubParam success",MB_ICONINFORMATION);
// 	} 
// 	else
// 	{
// 		AfxMessageBox("setSubParam failed");
// 	}	
#endif
}

void CMfcDemoDlg::OnParamRecTime() 
{
	NETDVR_RecordSCH_t rectimeParam;
	memset(&rectimeParam, 0, sizeof(rectimeParam));
	
	//get rectimeParam
	unsigned char chn = 0;
	weekday_t weekday = NETDVR_WEEKDAY_1;
	int ret = NETDVR_GetRecordSCH(m_hDvr, chn, weekday, &rectimeParam);
	if (NETDVR_SUCCESS == ret)
	{

		int h1=0,h2=0,m1=0,m2=0,s1=0,s2=0;
		h1 = rectimeParam.recTimeFieldt[0].starttime/3600;
		m1= rectimeParam.recTimeFieldt[0].starttime/60 - h1*60;
		s1 = rectimeParam.recTimeFieldt[0].starttime%60;

		h2 = rectimeParam.recTimeFieldt[0].endtime/3600;
		m2= rectimeParam.recTimeFieldt[0].endtime/60 - h2*60;
		s2 = rectimeParam.recTimeFieldt[0].endtime%60;
		CString szInfo;
		//print some params
		szInfo.Format("getrectimeParam Success!\n start_time[0]: %02d:%02d:%02d\n end_time[0]: %02d:%02d:%02d\n",
			h1, m1, s1,
			h2, m2, s2
			);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getrectimeParam  failed!");
		return;
	}
	
	rectimeParam.recTimeFieldt[0].starttime=0;
	rectimeParam.recTimeFieldt[0].endtime = 7200;

	rectimeParam.recTimeFieldt[1].starttime=3600;
	rectimeParam.recTimeFieldt[1].endtime = 8400;

	rectimeParam.recTimeFieldt[2].starttime=7200;
	rectimeParam.recTimeFieldt[2].endtime = 9600;

	rectimeParam.recTimeFieldt[3].starttime=9000;
	rectimeParam.recTimeFieldt[3].endtime = 14400;

	rectimeParam.copy2Weekday = 0x7f;
	rectimeParam.copy2Chnmask = 0xf;

	for (int i=0; i<4; i++)
	{
		rectimeParam.recTimeFieldt[i].flag_sch = !rectimeParam.recTimeFieldt[i].flag_sch;
		rectimeParam.recTimeFieldt[i].flag_md = !rectimeParam.recTimeFieldt[i].flag_md;
		rectimeParam.recTimeFieldt[i].flag_alarm = !rectimeParam.recTimeFieldt[i].flag_alarm;
	}
	//Set rectimeParam

	int nstart  = GetTickCount();
	for (i = 0; i<7; i++)
	{
		rectimeParam.weekday = (enum NETDVR_WEEKDAY)i;
		ret = NETDVR_SetRecordSCH(m_hDvr, &rectimeParam);
	}
	
	if (NETDVR_SUCCESS == ret)
	{
		CString szDur;
		szDur.Format("setrectimeParam success\n%d",GetTickCount() - nstart);
		AfxMessageBox(szDur,MB_ICONINFORMATION);
		//AfxMessageBox("setrectimeParam success",MB_ICONINFORMATION);
	} 
	else
	{
		CString szDur;
		szDur.Format("setrectimeParam failed\nerr = %d",ret);

		AfxMessageBox(szDur);
	}
}

void CMfcDemoDlg::OnParamException() 
{

}

void CMfcDemoDlg::OnParamNetwork() 
{
	NETDVR_networkParam_t netParam;
	memset(&netParam, 0, sizeof(netParam));
	
	//get netParam

	int ret = NETDVR_getNetworkParams(m_hDvr, &netParam);
	if (NETDVR_SUCCESS == ret)
	{
		struct in_addr in;
		in.s_addr = netParam.ip_address;
		CString szInfo;
		//print some params
		szInfo.Format("getnetParam Success!\n mac: %s\n ip: %s\n port: %d\n http_port: %d",
			netParam.mac_address, inet_ntoa(in), netParam.server_port, netParam.http_port);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getnetParam  failed!");
		return;
	}
	
	//Set netParam
	ret = NETDVR_setNetworkParams(m_hDvr, &netParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setnetParam success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setnetParam failed");
	}	
}

void CMfcDemoDlg::OnParamVideoLost() 
{
	NETDVR_VideoLostParam_t VideoLostParam;
	memset(&VideoLostParam, 0, sizeof(VideoLostParam));
	
	//get VideoLostParam
	unsigned char chn = 0;

	int ret = NETDVR_getVideoLost(m_hDvr, chn, &VideoLostParam);
	if (NETDVR_SUCCESS == ret)
	{

		CString szInfo;
		szInfo.Format("getVideoLostTime Success!\n flag_buzz: %d\n",
			VideoLostParam.flag_buzz);
		
			
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getVideoLostTime  failed!");
		return;
	}
	
	//Set VideoLostParam
	ret = NETDVR_setVideoLost(m_hDvr, &VideoLostParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setVideoLostTime success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setVideoLostTime failed");
	}
}

void CMfcDemoDlg::OnParamSHTime() 
{

}

void CMfcDemoDlg::OnManualRec() 
{
	NETDVR_ManualRecord_t manualRecParam;
	memset(&manualRecParam, 0, sizeof(manualRecParam));
	
	//get manualRecParam
	
	int ret = NETDVR_GetRecordState(m_hDvr, &manualRecParam);
	if (NETDVR_SUCCESS == ret)
	{

	
		AfxMessageBox("GetRecordState sucess",MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("GetRecordState  failed!");
		return;
	}
	
	//Set manualRecParam
	manualRecParam.chnRecState = 0; //stop all
	ret = NETDVR_SetRecordState(m_hDvr, &manualRecParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("SetRecordState success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("SetRecordState failed");
	}		
}

void CMfcDemoDlg::OnSysTime() 
{
	NETDVR_SysTime_t systime;

	
	//get systime
	
	int ret = NETDVR_getSystemTime(m_hDvr, &systime);
	if (NETDVR_SUCCESS == ret)
	{
		struct tm *tm1;
		
		tm1 = gmtime((time_t*)&systime.systemtime);

		CString szInfo;
		//print some params
		szInfo.Format("getsystime Success!\n %04d-%02d-%02d %02d:%02d:%02d",
				tm1->tm_year+1900, tm1->tm_mon+1, tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getsystime  failed!");
		return;
	}
	
	//Set systime
	ret = NETDVR_setSystemTime(m_hDvr, &systime);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setsystime success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setsystime failed");
	}
}

void CMfcDemoDlg::OnHddInfo() 
{
	NETDVR_hddInfo_t hddinfo;
	memset(&hddinfo, 0, sizeof(hddinfo));
	
	//get hddinfo
	
	int ret = NETDVR_remoteGetHddInfo(m_hDvr, 0, &hddinfo);
	if (NETDVR_SUCCESS == ret)
	{
		
		CString szInfo;
		//print some params
		
	
		szInfo.Format("getHddInfo Success!\n hdd[0].exsit :%d\n hdd[0].capability :%d\n hdd[0].freesize :%d",
				hddinfo.hdd_exist, hddinfo.capability, hddinfo.freesize);
	
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getHddInfo  failed!");
		return;
	}
}

void CMfcDemoDlg::OnSysInfo() 
{
	NETDVR_SysVerInfo_t verinfo;
	memset(&verinfo, 0, sizeof(verinfo));
	
	int ret = NETDVR_remoteGetSysVerInfo(m_hDvr, &verinfo);
	if (NETDVR_SUCCESS == ret)
	{
		CString szInfo;
		szInfo.Format("getSysVerInfo successfully!\n devicename: %s, \n devicemodel: %s, \n deviceser: %s, \n version: %s \n ",
			verinfo.devicename,
			verinfo.devicemodel,
			verinfo.deviceser,
			verinfo.version
			);

		AfxMessageBox(szInfo,MB_ICONINFORMATION);
		
	}
	else
	{
		
		AfxMessageBox("getSysVerInfo  failed!");
		return;
	}
}

void CALLBACK UpdateProgressCallBackFunc(struct NETDVR_progressParam_t progress, unsigned int dwContent)
{
	//do something
}

void CMfcDemoDlg::OnUpdate() 
{
	NETDVR_updateParam_t update_para;
	memset(&update_para, 0, sizeof(NETDVR_updateParam_t));
	//strcpy(update_para.filename, "panel"/*"uImage.bin"*/);
	update_para.reserved = NETDVR_UPDATE_MOTHERBBOARD;//NETDVR_UPDATE_MOTHERBBOARD;
	
	CFile file;
	BOOL bRet = file.Open("c:\\mainboard_9504d1_V4.0.3_ZS002.bin", CFile::modeRead);
	if (!bRet)
	{
		AfxMessageBox("Open File c:\\mainboard-new.bin failed !");
		return;
	}
	
	update_para.size = file.GetLength();  

	file.Close();

	update_para.version = 0x0100;
	update_para.verify = 0;

	
	int ret = NETDVR_regCBUpdateProgress(m_hDvr, UpdateProgressCallBackFunc, 0);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("NETDVR_regCBUpdateProgress failed!");
		return;
	}
	
	ret = NETDVR_startUpdate(m_hDvr, &update_para, "c:", "mainboard_9504d1_V4.0.3_ZS002.bin"); //
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("NETDVR_startUpdate failed!");
		
	}

}

void CMfcDemoDlg::OnRestore() 
{
	int ret = NETDVR_restoreFactorySettings(m_hDvr);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("restoreFactorySettings failed!");
		return;
	}
}

void CMfcDemoDlg::OnReboot() 
{
	int ret = NETDVR_reboot(m_hDvr);

}

void CMfcDemoDlg::OnShutDown() 
{
	int ret = NETDVR_shutdown(m_hDvr);

}

void CMfcDemoDlg::OnMDParam() 
{
	NETDVR_motionDetection_t MDParam;
	memset(&MDParam, 0, sizeof(MDParam));
	
	//get MDParam
	
	unsigned char chn = 0;

	int ret = NETDVR_getMotionDection(m_hDvr, chn, &MDParam);
	if (NETDVR_SUCCESS == ret)
	{
		
		CString szInfo;
		//print some params

		AfxMessageBox("getMotionDection Success!\n ",MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getMotionDection  failed!");
		return;
	}
	
	//Set MDParam
	ret = NETDVR_setMotionDection(m_hDvr, &MDParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("SetMotionDection success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("SetMotionDection failed");
	}		
}

void CMfcDemoDlg::OnMDTime() 
{

}

void CMfcDemoDlg::OnMDAlarm() 
{

}

void CMfcDemoDlg::OnAlarmInParam() 
{
	NETDVR_alarmInParam_t alarmParam;
	memset(&alarmParam, 0, sizeof(alarmParam));
	
	//get alarmParam
	
	unsigned char id = 0;
	
	int ret = NETDVR_getAlarmInParams(m_hDvr, id, &alarmParam);
	if (NETDVR_SUCCESS == ret)
	{
		
		CString szInfo;
		//print some params
		szInfo.Format("getAlarmInParams Success!\n flag_deal: %d\n flag_buzz: %d",
			alarmParam.flag_deal, alarmParam.flag_buzz);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getAlarmInParams  failed!");
		return;
	}
	
	//Set alarmParam
	ret = NETDVR_setAlarmInParams(m_hDvr,&alarmParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setAlarmInParams success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setAlarmInParams failed");
	}		
}

void CMfcDemoDlg::OnAlarmInTime() 
{
	
}

void CMfcDemoDlg::OnAlarmInAlarm() 
{

}

void CMfcDemoDlg::OnAlarmInPtz() 
{

}

void CMfcDemoDlg::OnAlarmOutParam() 
{
	NETDVR_alarmOutParam_t alarmParam;
	memset(&alarmParam, 0, sizeof(alarmParam));
	
	//get alarmParam
	
	unsigned char id = 0;
	
	int ret = NETDVR_getAlarmOutParams(m_hDvr, id, &alarmParam);
	if (NETDVR_SUCCESS == ret)
	{
		
		CString szInfo;
		//print some params
		szInfo.Format("getAlarmOutParams Success!\n outid: %d\n typeout: %d",
			alarmParam.outid, alarmParam.typeout);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("getAlarmOutParams  failed!");
		return;
	}
	
	//Set alarmParam
	ret = NETDVR_setAlarmOutParams(m_hDvr, &alarmParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("setAlarmOutParams success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("setAlarmOutParams failed");
	}		
	
}

void CMfcDemoDlg::OnAlarmOutTime() 
{

}

void CMfcDemoDlg::OnClearAlarms() 
{
	int ret = NETDVR_clearAlarms(m_hDvr);
	
}

void CMfcDemoDlg::OnAlarmState() 
{

}

NETDVR_logSearchResult_t logResult={0};
void CMfcDemoDlg::OnLogSearch() 
{
	int ret;
	struct NETDVR_logSearchCondition_t s_condition;

	struct tm tmtemp;
	memset(&tmtemp, 0, sizeof(tmtemp));

	tmtemp.tm_year = 2009-1900;
	tmtemp.tm_mon = 0;
	tmtemp.tm_mday = 1;
	
	s_condition.start_time = mktime(&tmtemp);

	tmtemp.tm_year = 2010-1900;
	s_condition.end_time = mktime(&tmtemp);

	s_condition.startID = 1;
	s_condition.max_return = 12;
	s_condition.query_mode = NETDVR_LOGSEARCH_MODE_TYPE;
	s_condition.main_type = NETDVR_LOGSEARCH_MAIN_ALL;
	s_condition.slave_type = NETDVR_LOGSEARCH_REMOTEOP_ALL;

	
	if (logResult.sum)
	{
		NETDVR_logSearchClean(&logResult);
		memset(&logResult, 0, sizeof(logResult));
	}

	ret = NETDVR_logSearch(m_hDvr, &s_condition, &logResult);
	if (NETDVR_SUCCESS == ret)
	{

		NETDVR_logInfo_t* p_LogInfo = NULL;
		
		p_LogInfo = logResult.pH;
		CString szInfo;
		szInfo.Format("logSearch successfully!\n startID = %d, endID = %d, sum = %d\n",
			logResult.startID, logResult.endID, logResult.sum);
		for (int i=0; i<min(logResult.sum, s_condition.max_return); i++)
		{
			CString szLog;
			szLog.Format("main:%s, slave:%s, loginfo:%s\n", p_LogInfo->main, p_LogInfo->slave, p_LogInfo->loginfo);
			szInfo += szLog;
			p_LogInfo = p_LogInfo->pnext;
		}
		AfxMessageBox(szInfo,MB_ICONINFORMATION);

	}
	else
	{
		AfxMessageBox("logSearch failed!");
	}
	
}

void CMfcDemoDlg::OnLogClean() 
{
	if (logResult.sum)
	{
		NETDVR_logSearchClean(&logResult);
		memset(&logResult, 0, sizeof(logResult));
	}
}


NETDVR_UserNumber_t UserList={0};
void CMfcDemoDlg::OnGetUserList() 
{


	int ret = NETDVR_GetUserInfo(m_hDvr, &UserList);
	if (NETDVR_SUCCESS == ret)
	{
		NETDVR_userInfo_t* p_UserInfo = NULL;

		CString szInfo;
		for (int i=0; i<8; i++)
		{
			p_UserInfo = &UserList.userinfo[i];
			CString szUser;
			szUser.Format("name: %s, password: %s\n", p_UserInfo->name, p_UserInfo->password);
			szInfo += szUser;
			
		}
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	}
	else
	{
		AfxMessageBox("getUserList failed!");
	}
}

void CMfcDemoDlg::OnClearUserList() 
{

}

void CMfcDemoDlg::OnAddUser() 
{
	NETDVR_userInfo_t UserInfo;

	strcpy(UserInfo.name, "test");
	strcpy(UserInfo.password, "111");
	memset(UserInfo.mac_address, 0, sizeof(UserInfo.mac_address));

	UserInfo.rcamer = 1;
	UserInfo.rrec = 1;
	UserInfo.rplay = 1;
	UserInfo.rsetpara = 1;
	UserInfo.rlog = 1;
	UserInfo.rtool = 1;
	UserInfo.rpreview = 1;
	UserInfo.ralarm = 1;
	UserInfo.rvoip = 1;
	UserInfo.lcamer = 1;
	UserInfo.lrec = 1;
	UserInfo.lplay = 1;
	UserInfo.lsetpara = 1;
	UserInfo.llog = 1;
	UserInfo.ltool = 1;

	int ret = NETDVR_AddUserInfo(m_hDvr, &UserInfo);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("addUser failed!");
	}
	
}

void CMfcDemoDlg::OnEditUser() 
{
	NETDVR_userInfo_t UserInfo;
	
	strcpy(UserInfo.name, "test");
	strcpy(UserInfo.password, "222");

	memset(UserInfo.mac_address, 0, sizeof(UserInfo.mac_address));
	
	UserInfo.rcamer = 1;
	UserInfo.rrec = 1;
	UserInfo.rplay = 1;
	UserInfo.rsetpara = 1;
	UserInfo.rlog = 1;
	UserInfo.rtool = 1;
	UserInfo.rpreview = 1;
	UserInfo.ralarm = 1;
	UserInfo.rvoip = 1;
	UserInfo.lcamer = 1;
	UserInfo.lrec = 1;
	UserInfo.lplay = 1;
	UserInfo.lsetpara = 1;
	UserInfo.llog = 1;
	UserInfo.ltool = 1;
	
	int ret = NETDVR_EditUserInfo(m_hDvr, &UserInfo);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("editUser failed!");
	}
}

void CMfcDemoDlg::OnDelUser() 
{
	int ret = NETDVR_DelUserInfo(m_hDvr, "test");
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("deleteUser failed!");
	}
}

void CALLBACK ConnectLostCallBack(unsigned int dwContent)
{
	AfxMessageBox("Connect Lost!");
}

void CMfcDemoDlg::OnRegConnLost() 
{
	DWORD dwStart = GetTickCount();

	int ret = NETDVR_regCBMsgConnLost(m_hDvr, ConnectLostCallBack, 0);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox("regCBMsgConnLost failed!");
		return;
	}

	CString SZiNFO;
	SZiNFO.Format("Dur = %d",GetTickCount() - dwStart);
	AfxMessageBox(SZiNFO);

	
}

void CMfcDemoDlg::OnSDKcheck() 
{

}

void CMfcDemoDlg::OnGetConnTime() 
{
	unsigned int dwConnectTime;
	int ret = NETDVR_getConnectTimeout(&dwConnectTime);
	
	CString szInfo;
	szInfo.Format("Current Connect Time is %d ms!", dwConnectTime);
	AfxMessageBox(szInfo, MB_ICONINFORMATION);
}

void CMfcDemoDlg::OnSetConnTime() 
{
	AfxMessageBox("this sample will set connect time to 10000 ms!", MB_ICONINFORMATION);
	int ret = NETDVR_setConnectTimeout(10000);
}

void CALLBACK AlarmUploadCallBack(struct NETDVR_AlarmUploadState_t alarmstate, unsigned int dwContent)
{
	CString info;
	info.Format("type = %d, id = %d, state = %d, \ndwContent = 0x%x", 
		alarmstate.type, alarmstate.id, alarmstate.state, dwContent);
// 	::MessageBox(NULL, info, NULL,MB_OK);
}

void CMfcDemoDlg::OnRegAlarmUpload() 
{
	// TODO: Add your control notification handler code here
	int ret = NETDVR_regCBAlarmState(m_hDvr, AlarmUploadCallBack, (UINT)this);
	if (ret)
	{
		CString info;
		info.Format("NETDVR_regCBAlarmState, ret = %d", ret);
		::MessageBox(NULL, info, NULL,MB_OK);
		return;
	}

// 	CString info;
// 	info.Format("NETDVR_regCBAlarmState ok, dwContent = 0x%x", (UINT)this);
// 	::MessageBox(NULL, info, NULL,MB_OK);

	ret = NETDVR_SetAlarmUpload(m_hDvr, 1);
	if (ret)
	{
		CString info;
		info.Format("NETDVR_SetAlarmUpload, ret = %d", ret);
		::MessageBox(NULL, info, NULL,MB_OK);
	}
}

void CMfcDemoDlg::OnGetFileFrame() 
{
	// TODO: Add your control notification handler code here
	if (rec_desc.sum == 0)
	{
		AfxMessageBox("no rec file!");
		return;
	}


	bGetFileFrame = !bGetFileFrame;
	if (bGetFileFrame)
	{
		int ret;
#if 0
		//test the first file!
		PlayBackClientInfo_t pbci_t={0};
		pbci_t.sendmode = 1;
		pbci_t.pEncFrameCBFunc = get_encframe;
		pbci_t.dwEncFrameContent = 0;
		pbci_t.pDecFrameCBFunc = NULL;
		pbci_t.dwDecFrameContent = 0;
		pbci_t.pPlayOverCBFunc = playovermsg;
		pbci_t.dwPlayOverContent = 0;
		pbci_t.pProgressCBFunc = playprogress;
		pbci_t.dwProgressContent = 0;
		
		ret = NETDVR_startPlayByFile(m_hDvr, rec_desc.precInfo, &pbci_t, &m_GetFileFrmHandle);
		if (ret)
		{
			AfxMessageBox("openPlayer failed.");	
		}
	
#else
		NETDVR_TimePlayCond_t tps;
		tps.chn = 0;
		
		struct tm tmtemp;
		memset(&tmtemp, 0, sizeof(tmtemp));
		
		tmtemp.tm_year = 2010-1900;
		tmtemp.tm_mon = 0;
		tmtemp.tm_mday = 1;
		
		tps.start_time = mktime(&tmtemp); //2009-1-1 00:00:00
		
		tmtemp.tm_year = 2011-1900;
		tps.end_time = mktime(&tmtemp); //2010-1-1 00:00:00
		
		tps.type = NETDVR_REC_INDEX_ALL;
		
		PlayBackClientInfo_t pbci_t={0};
		pbci_t.sendmode = 1;
		pbci_t.pEncFrameCBFunc = get_encframe;
		pbci_t.dwEncFrameContent = 0;
		pbci_t.pDecFrameCBFunc = NULL;
		pbci_t.dwDecFrameContent = 0;
		pbci_t.pPlayOverCBFunc = playovermsg;
		pbci_t.dwPlayOverContent = 0;
		pbci_t.pProgressCBFunc = playprogress;
		pbci_t.dwProgressContent = 0;
		
		ret = NETDVR_startPlayByTime(m_hDvr, &tps, &pbci_t, &m_GetFileFrmHandle);
#endif

		GetDlgItem(IDC_GetFileFrame)->SetWindowText("StopGetFile");
	} 
	else
	{
		GetDlgItem(IDC_GetFileFrame)->SetWindowText("GetFileFrame");
		int ret = NETDVR_stopPlayBack(m_GetFileFrmHandle);
		if (NETDVR_SUCCESS != ret)
		{
			CString szErr;
			szErr.Format("stopFilePlay failed. %d", ret);
			AfxMessageBox(szErr);
			//AfxMessageBox("stopFilePlay failed.");
			return;
		}
	}
}

void CALLBACK SerialDataCallBack(int lSerialPort, unsigned char bySerialChannel, char *pRecvDataBuffer, unsigned int dwBufSize, unsigned int dwContent)
{

	FILE *fp = fopen("c:\\testserial.txt", "at");

	fprintf(fp, "SerialPort = %d, chn = %d, buf = %s, size = %d\n",
		lSerialPort, bySerialChannel, pRecvDataBuffer, dwBufSize);

	fflush(fp);
	fclose(fp);

}

int g_serialnPort = 2; //1-232  2-485

void CMfcDemoDlg::OnSerialport() 
{
	// TODO: Add your control notification handler code here
	char sendbuf[1024] = {0};
	sprintf(sendbuf, "This is a test for serialport\n");
	int ret = NETDVR_SerialSend(m_hDvr, g_serialnPort, 1, sendbuf, strlen(sendbuf)+1);
	if (ret)
	{
		CString szErr;
		szErr.Format("NETDVR_SerialStart failed. %d", ret);
		AfxMessageBox(szErr);
		return;
	}
	//NETDVR_SerialSend(m_hDvr, 2, 1, sendbuf, strlen(sendbuf)+1);

}

void CMfcDemoDlg::OnSerialParam() 
{
	// TODO: Add your control notification handler code here
	NETDVR_ComParam_t serialParam;
	memset(&serialParam, 0, sizeof(serialParam));
	
	//get alarmParam
	
	unsigned char port = 2;
	
	int ret = NETDVR_GetComParam(m_hDvr, port, &serialParam);
	if (NETDVR_SUCCESS == ret)
	{
		
		CString szInfo;
		//print some params
		szInfo.Format("NETDVR_GetComParam Success!\n com_baudrate: %d\n com_checkbit: %d",
			serialParam.com_baudrate, serialParam.com_checkbit);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("NETDVR_GetComParam  failed!");
		return;
	}
	
	//Set alarmParam
	serialParam.com_databit = YT_DATABIT_8;
	serialParam.com_checkbit = YT_CHECKBIT_NONE;
	serialParam.com_stopbit = YT_STOPBIT_1;
	serialParam.com_baudrate = 9600;
	ret = NETDVR_SetComParam(m_hDvr,&serialParam);
	if (NETDVR_SUCCESS == ret)
	{
		AfxMessageBox("NETDVR_SetComParam success",MB_ICONINFORMATION);
	} 
	else
	{
		AfxMessageBox("NETDVR_SetComParam failed");
	}	

	ret = NETDVR_GetComParam(m_hDvr, port, &serialParam);
	if (NETDVR_SUCCESS == ret)
	{
		
		CString szInfo;
		//print some params
		szInfo.Format("NETDVR_GetComParam Success!\n com_baudrate: %d\n com_checkbit: %d",
			serialParam.com_baudrate, serialParam.com_checkbit);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
	} 
	else
	{
		
		AfxMessageBox("NETDVR_GetComParam  failed!");
		return;
	}
}

void CMfcDemoDlg::OnSerialOpen() 
{
	// TODO: Add your control notification handler code here

	NETDVR_ComParam_t p_para;
	p_para.com_baudrate = 115200;
	p_para.com_checkbit = YT_CHECKBIT_NONE;
	p_para.com_databit =  YT_DATABIT_8;//数据位 8，7，6
	p_para.com_stopbit = YT_STOPBIT_1; //停止位 1，2
	p_para.serialport = 2;// 1:232;2:485

	NETDVR_SetComParam(m_hDvr,&p_para);
	int ret= NETDVR_SerialStart(m_hDvr, g_serialnPort,	SerialDataCallBack, 0);

	//NETDVR_SerialStart(m_hDvr, 2, SerialDataCallBack, 0);
	//bySerialChannel, 使用485串口时有效，从1开始；232串口作为透明通道时该值设置为0
	if (ret)
	{
		CString szErr;
		szErr.Format("NETDVR_SerialStart failed. %d", ret);
		AfxMessageBox(szErr);
		return;
	}
}

void CMfcDemoDlg::OnSerialClose() 
{
	// TODO: Add your control notification handler code here
 	NETDVR_SerialStop(m_hDvr, g_serialnPort);
	//NETDVR_SerialStop(m_hDvr, 2);
}

void CALLBACK DeviceReconnectCBFun(enum RECONNMSG msg, unsigned int context)
{
	switch (msg)
	{
	case RECONN_BEGIN:
		::MessageBox(NULL, "begin reconnect", NULL, MB_OK);
		break;
	case RECONN_SUCESS:
		::MessageBox(NULL, "reconnect SUCESS", NULL, MB_OK);
		break;
	case RECONN_FAILED:
		::MessageBox(NULL, "reconnect FAILED", NULL, MB_OK);
		break;
	}
}

void CMfcDemoDlg::OnSetReconnect() 
{
	// TODO: Add your control notification handler code here
	NETDVR_regCBMsgReconnect(m_hDvr, DeviceReconnectCBFun, 0);
	NETDVR_setReconnectFlag(m_hDvr, 1);
}

void CMfcDemoDlg::OnButtonPolling() 
{
	// TODO: Add your control notification handler code here
	for (int i=0; i<4; i++)
	{
		play[i].chn=i;
		play[i].pDlg=this;
		CreateThread(NULL, 0, _PlayThread_S, &play[i], 0, &id);
		palyid[i]=id;
	}
	CreateThread(NULL,0,_CtrlThread_S,this,0,&id);
	playluck=CreateEvent( NULL, TRUE, FALSE, NULL );
	
}

DWORD WINAPI CMfcDemoDlg::_PlayThread_S(LPVOID param)
{
	playinfo *p=(playinfo *)param;
	CMfcDemoDlg *pDlg=(CMfcDemoDlg *)p->pDlg;
	RealPlayClientInfo_t rpci;
	while (1)
	{
		MSG msg;
		PeekMessage(&msg, NULL, WM_USER+100, WM_USER+120, PM_NOREMOVE);
		
		if(GetMessage(&msg,0,0,0)) //get msg from message queue
		{
			if (msg.message==esc)
			{
				break;
			}
			switch(msg.message)
			{
			case start: 
				{
					rpci.rcv_chn = p->chn;
					rpci.streamtype = 0;
					rpci.pEncFrameCBFunc = NULL/*get_encframe*/;
					rpci.dwEncFrameContent = p->chn;
					rpci.pDecFrameCBFunc = DealDecVideoFrame;
					rpci.dwDecFrameContent = p->chn;
					int ret = NETDVR_StartRealPlay(pDlg->m_hDvr, &rpci, &pDlg->m_RealHandle[p->chn]);
					SetEvent(pDlg->playluck);
					if (NETDVR_SUCCESS != ret)
					{
						//       CString szErr;
						//       szErr.Format("NETDVR_StartRealPlay failed! ret = %d",ret);
						//       AfxMessageBox(szErr);
						FILE* pf=fopen("E:\\test_openinthread.txt","at");
						fprintf(pf,"open_realplay chn %d fialed  ret=%d\n",p->chn,ret);
						fflush(pf);
						fclose(pf);
						continue;
					}
					
				}
				break;
			case stop:
				{
					int ret = NETDVR_StopRealPlay(pDlg->m_RealHandle[p->chn]);
					pDlg->m_RealHandle[p->chn] = 0;
					SetEvent(pDlg->playluck);
					if (NETDVR_SUCCESS != ret)
					{
						//       CString szErr;
						//       szErr.Format("NETDVR_StopRealPlay failed! ret = %d",ret);
						//       AfxMessageBox(szErr);
						FILE* pf=fopen("E:\\test_stopinthread.txt","at");
						fprintf(pf,"stop_realplay chn %d fialed  ret=%d\n",p->chn,ret);
						fflush(pf);
						fclose(pf);
						continue;
					}

				}
				break;
			}
		}
		Sleep(1);
	}
	return 0;
}
DWORD WINAPI CMfcDemoDlg::_CtrlThread_S(LPVOID param)
{
	CMfcDemoDlg *pDlg=(CMfcDemoDlg *)param;
	int j=0;
	while (1)
	{
		if (pDlg->exit)
		{
			break;
		}
		for (int i=0;i<4;i++)
		{
			pDlg->curr=j;
			PostThreadMessage(pDlg->palyid[pDlg->curr], start, NULL, NULL);
			WaitForSingleObject(pDlg->playluck,INFINITE);
			ResetEvent(pDlg->playluck);
			j++;
		}
		Sleep(5000);
		for (i=0;i<4;i++)
		{
			PostThreadMessage(pDlg->palyid[pDlg->curr], stop, NULL, NULL);
			WaitForSingleObject(pDlg->playluck,INFINITE);
			ResetEvent(pDlg->playluck);
			pDlg->curr--;
		}
		if (j==4)
		{
			j=0;
		}
		Sleep(1);
	}
	return 0;
}

void CALLBACK callbackTimeDLProgress(struct NETDVR_TimeDLprogressParam_t progress, unsigned int dwContent)
{
	return;
}

void CALLBACK callbackTimeDLfilename(struct NETDVR_TimeDLfilename savepathname,char* s_save_filename, unsigned int dwContent)
{
	wsprintf(s_save_filename, TEXT("d:\\testtimedownload\\Chn%d_%d.ifv"), savepathname.channel_no,savepathname.currindex);
}

bool bTimeDownload = false;
void CMfcDemoDlg::OnButtonTimedownload() 
{
	// TODO: Add your control notification handler code here
	bTimeDownload = !bTimeDownload;
	TimeDownloadInfo_t timedlifo;
	timedlifo.dwErrContent=0;
	timedlifo.p_cb_err=callbackDLError;
	timedlifo.dwProgressContent=0;
	timedlifo.p_cb_progress=callbackTimeDLProgress;
	timedlifo.dwSaveContent=0;
	timedlifo.p_cb_save=callbackTimeDLfilename;
	timedlifo.streamtype=NETDVR_REC_INDEX_ALL;
	timedlifo.rcv_chn=6;
	struct tm tmtemp;
	memset(&tmtemp, 0, sizeof(tmtemp));
	
	tmtemp.tm_year = 2001-1900;
	tmtemp.tm_mon = 0;
	tmtemp.tm_mday = 1;
	
		timedlifo.startTime = mktime(&tmtemp); //2009-1-1 00:00:00
	//timedlifo.startTime = 1292976000;
	tmtemp.tm_year = 2013-1900;
		timedlifo.endTime = mktime(&tmtemp); //2011-1-1 00:00:00
	//timedlifo.endTime = 1293062399;
	
	if (bTimeDownload)
	{
		AfxMessageBox("this sample will download file and save to d:\\testtimedownload");
		
		int	ret = NETDVR_startTimeDownload(m_hDvr, &timedlifo, &dlhandle);
		if (NETDVR_SUCCESS != ret)
		{
			AfxMessageBox("NETDVR_startTimeDownload failed!");
		}
		GetDlgItem(IDC_BUTTON_TIMEDOWNLOAD)->SetWindowText("StopDL");
	} 
	else
	{
		GetDlgItem(IDC_BUTTON_TIMEDOWNLOAD)->SetWindowText("DownLoad");
		int ret;		
		ret = NETDVR_stopTimeDownload(dlhandle);
		if (NETDVR_SUCCESS != ret)
		{
			AfxMessageBox("NETDVR_stopTimeDownload failed!");
			return;
		}
		
	}
	
}

void CMfcDemoDlg::OntestLAS() 
{
	// TODO: Add your control notification handler code here
	NETDVR_LASServer_t LASinfo;
	memset(&LASinfo, 0, sizeof(LASinfo));
	
	int ret = NETDVR_getLASAFQQInfo(m_hDvr, &LASinfo);
	if (NETDVR_SUCCESS == ret)
	{
		CString szInfo;
		szInfo.Format("getLASInfo successfully!\n macaddr: %s, \n productcode: %s, \n sn: %s, \n",
			LASinfo.macaddr,
			LASinfo.productcode,
			LASinfo.sn
			);
		
		AfxMessageBox(szInfo,MB_ICONINFORMATION);
		
	}
	else
	{
		
		AfxMessageBox("getSysVerInfo  failed!");
		return;
	}
	int size = sizeof(LASinfo.sn);
	memset(LASinfo.sn,0,size);
	for(int i=0;i<16;i++)
	{
		LASinfo.sn[i]='0'+i;
	}
	memset(LASinfo.productcode,0,sizeof(LASinfo.productcode));
	for(i=0;i<16;i++)
	{
		LASinfo.productcode[i]='A'+i;
	}
	ret = NETDVR_setLASAFQQInfo(m_hDvr,&LASinfo);
	if (ret == NETDVR_SUCCESS)
	{
		CString szInfo;
		szInfo.Format("setLASinfo successfully!");
		AfxMessageBox(szInfo);
	}
	else
	{
		AfxMessageBox("setLASinfo  failed!");
		return;
	}
}

void CMfcDemoDlg::OnBUTTONGetAlarm() 
{
	// TODO: Add your control notification handler code here
	NETDVR_AlarmVal_t  m_getAlarm_val;
	memset(&m_getAlarm_val, 0, sizeof(m_getAlarm_val));
	UpdateData(TRUE);
	int	ret = NETDVR_getAlarmInVal(m_hDvr, m_nAlarmID, &m_getAlarm_val);
	if (NETDVR_SUCCESS != ret)
	{
		AfxMessageBox(_T("getAlarm err"));
		return;
	}
	CString szInfo;
	CString sztmp;
	sztmp.Format(_T("%d"), m_getAlarm_val.alarmid);
	szInfo+=sztmp;
	szInfo+=_T("|");
	sztmp.Empty();
	sztmp.Format(_T("%d"), m_getAlarm_val.val);
	szInfo+=sztmp;
	sztmp.Empty();
	AfxMessageBox(szInfo, MB_ICONINFORMATION);
	
}

void CMfcDemoDlg::OnBUTTONSetAlarm() 
{
	// TODO: Add your control notification handler code here
	NETDVR_AlarmVal_t  m_setAlarm_val;
	memset(&m_setAlarm_val,0,sizeof(NETDVR_AlarmVal_t));
	UpdateData(TRUE);
	m_setAlarm_val.alarmid=m_nAlarmID;
	m_setAlarm_val.val=m_nAlarmVal;
	
	int ret = NETDVR_setAlarmOutVal(m_hDvr, &m_setAlarm_val);
	if (ret)
	{
		CString info;
		info.Format("NETDVR_SetAlarmUpload, ret = %d", ret);
		::MessageBox(NULL, info, NULL,MB_OK);
	}
}
