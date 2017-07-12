// MfcDemo.h : main header file for the MFCDEMO application
//

#if !defined(AFX_MFCDEMO_H__0232B0AE_A4C5_4711_87B1_E94A2C76EA56__INCLUDED_)
#define AFX_MFCDEMO_H__0232B0AE_A4C5_4711_87B1_E94A2C76EA56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMfcDemoApp:
// See MfcDemo.cpp for the implementation of this class
//

class CMfcDemoApp : public CWinApp
{
public:
	CMfcDemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMfcDemoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMfcDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCDEMO_H__0232B0AE_A4C5_4711_87B1_E94A2C76EA56__INCLUDED_)
