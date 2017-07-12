// DDrawShow.h: interface for the DDrawShow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DDRAWSHOW_H__5DD0D61D_3729_4F52_9533_1D0968104452__INCLUDED_)
#define AFX_DDRAWSHOW_H__5DD0D61D_3729_4F52_9533_1D0968104452__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ddraw.h"
#pragma comment( lib, "ddraw.lib" )
#pragma comment( lib, "dxguid.lib" )
  
typedef enum IMAGETYPE{YUY2,RGB32,YV12};

class CDDrawShow  
{
public:
	CDDrawShow();
	virtual ~CDDrawShow();
	BOOL CreateDDrawObj(enum IMAGETYPE imagetype, HWND hPlayWnd, DWORD width, DWORD height);
	void DestroyDDrawObj();
	BOOL DrawYV12byDraw7(LPBYTE lpY, LPBYTE lpV, LPBYTE lpU, DWORD width, DWORD height);

private:

	LPDIRECTDRAW7           m_lpDD;    // DirectDraw 对象指针
	LPDIRECTDRAWSURFACE7    m_lpDDSPrimary;  // DirectDraw 主表面指针
	LPDIRECTDRAWSURFACE7    m_lpDDSOffScr;  // DirectDraw 离屏表面指针
	DDSURFACEDESC2   m_ddsd;    // DirectDraw 表面描述
	DWORD	m_dwWidth;
	DWORD	m_dwHeight;
	HWND	m_hPlayWnd;
};

#endif // !defined(AFX_DDRAWSHOW_H__5DD0D61D_3729_4F52_9533_1D0968104452__INCLUDED_)
