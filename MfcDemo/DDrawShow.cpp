// DDrawShow.cpp: implementation of the DDrawShow class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DDrawShow.h"
#include <stdio.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDDrawShow::CDDrawShow()
{
	m_lpDD=NULL;    
	m_lpDDSPrimary=NULL;  
	m_lpDDSOffScr=NULL;  
  	m_dwWidth = 0;
	m_dwHeight = 0;
	m_hPlayWnd = NULL;
}

CDDrawShow::~CDDrawShow()
{
	DestroyDDrawObj();
	m_dwWidth = 0;
	m_dwHeight = 0;
	m_hPlayWnd = NULL;
}



BOOL CDDrawShow::CreateDDrawObj(enum IMAGETYPE imagetype, HWND hPlayWnd, DWORD width, DWORD height)
{
	if (hPlayWnd && hPlayWnd!=INVALID_HANDLE_VALUE)
	{
		m_hPlayWnd = hPlayWnd;

	}
	else
	{
		return FALSE;
	}
	// 创建DirectCraw对象
	HRESULT hr = DirectDrawCreateEx(NULL, (VOID**)&m_lpDD, IID_IDirectDraw7, NULL);
	if (hr != DD_OK) 
	{
		//MessageBox("Error Create DDraw.");
		return FALSE;
	}
	// 设置协作层
	hr = m_lpDD->SetCooperativeLevel(m_hPlayWnd,
//		DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT
		DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES
		);
	if (hr != DD_OK)
	{
		//MessageBox("Error Create Level.", s);
		return FALSE;
	}
	

	// 创建主表面
	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);
	m_ddsd.dwFlags = DDSD_CAPS;
	m_ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (m_lpDD->CreateSurface(&m_ddsd, &m_lpDDSPrimary, NULL) != DD_OK)
	{
		//MessageBox("Error Create Primary Surface.", s);
		return FALSE;
	}

	
	LPDIRECTDRAWCLIPPER  pcClipper;   // Cliper
	if( m_lpDD->CreateClipper( 0, &pcClipper, NULL ) != DD_OK )
		return FALSE;
	
	if( pcClipper->SetHWnd( 0, m_hPlayWnd ) != DD_OK )
	{
		pcClipper->Release();
		return FALSE;
	}
	
	if( m_lpDDSPrimary->SetClipper( pcClipper ) != DD_OK )
	{
		pcClipper->Release();
		return FALSE;
	}
	
	// Done with clipper
	pcClipper->Release();

	
	// 创建draw表面 
	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);
	m_ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

	
	m_dwWidth = width;
	m_dwHeight = height;
	m_ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	m_ddsd.dwWidth = m_dwWidth;
	m_ddsd.dwHeight = m_dwHeight;
	m_ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

	switch(imagetype)
	{
	case YUY2:
		
		break;

	case RGB32:
		
	    break;

	case YV12:
		{
			m_ddsd.ddpfPixelFormat.dwFlags  = DDPF_FOURCC /*| DDPF_YUV*/ ;
			m_ddsd.ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y','V', '1', '2');
			m_ddsd.ddpfPixelFormat.dwYUVBitCount = 12;
		}
		break;

	default:
	    break;
	}

	hr = m_lpDD->CreateSurface(&m_ddsd, &m_lpDDSOffScr, NULL);

	if (hr != DD_OK)
	{
		return FALSE;
	}
	return TRUE;
}

void CDDrawShow::DestroyDDrawObj()
{
	if(m_lpDD != NULL)
	{
		if(m_lpDDSPrimary != NULL)
		{
			m_lpDDSPrimary->Release();
			m_lpDDSPrimary = NULL;
		}
		if(m_lpDDSOffScr != NULL)
		{
			m_lpDDSOffScr->Release();
			m_lpDDSOffScr = NULL;
		}
		m_lpDD->Release();
		m_lpDD = NULL;
	}
}

 



BOOL CDDrawShow::DrawYV12byDraw7(LPBYTE lpY, LPBYTE lpV, LPBYTE lpU, DWORD width, DWORD height)
{
	if ((m_dwHeight != height) || (m_dwWidth != width))
	{
		DestroyDDrawObj();
		CreateDDrawObj(YV12, m_hPlayWnd, width, height);
	}

	if (!m_lpDDSOffScr || !m_lpDDSPrimary || !m_lpDD )
	{
		return S_FALSE;
	}
	
	if (!lpY || !lpV || !lpU)
	{
		return S_FALSE;
	}
	
	//将解码得到的YUV数据拷贝到YUV表面
	HRESULT					ddRval;				// DirectDraw 函数返回值
	RECT					rctDest;			// 目标区域
	RECT					rctSour;			// 源区域
	


	ddRval = m_lpDDSOffScr->Lock(NULL,&m_ddsd,/*DDLOCK_DONOTWAIT*/DDLOCK_WAIT | DDLOCK_WRITEONLY,NULL);
	
// 	while(ddRval == DDERR_WASSTILLDRAWING);
	if( ddRval == DDERR_SURFACELOST )
	{
		//MessageBox("DDERR_SURFACELOST");
		ddRval = m_lpDDSOffScr->Restore();
		ddRval = m_lpDDSOffScr->Lock(NULL, &m_ddsd, DDLOCK_WAIT|DDLOCK_WRITEONLY, NULL);
	}
	

	if(ddRval != DD_OK)
		return FALSE;
	

	// 填充离屏表面
	LPBYTE lpSurf = (LPBYTE)m_ddsd.lpSurface;
	if(lpSurf)
	{
		unsigned int i = 0;
		
		for (i=0;i<m_ddsd.dwHeight;i++)
		{
			memcpy(lpSurf, lpY, m_ddsd.dwWidth);
			lpY += m_dwWidth;
			lpSurf += m_ddsd.lPitch;
		}
		
		for (i=0;i<m_ddsd.dwHeight/2;i++)
		{
			memcpy(lpSurf, lpV, m_ddsd.dwWidth/2);
			lpV += m_dwWidth/2; 
			lpSurf += m_ddsd.lPitch/2;
		}
		for (i=0;i<m_ddsd.dwHeight/2;i++)
		{
			memcpy(lpSurf, lpU, m_ddsd.dwWidth/2);
			lpU += m_dwWidth/2;
			lpSurf += m_ddsd.lPitch/2;
		}
	}

	m_lpDDSOffScr->Unlock(NULL);
	
	//YUV表面的显示
	rctSour.left = 0;
	rctSour.top = 0;
	rctSour.right = m_ddsd.dwWidth;
	rctSour.bottom = m_ddsd.dwHeight;
	::GetClientRect(m_hPlayWnd,&rctDest);
	::ClientToScreen(m_hPlayWnd, (LPPOINT)&rctDest.left);
	::ClientToScreen(m_hPlayWnd, (LPPOINT)&rctDest.right);
	

	ddRval = m_lpDDSPrimary->Blt(&rctDest, m_lpDDSOffScr, &rctSour, /*DDBLT_DONOTWAIT*/DDBLT_WAIT, NULL);
	// while(ddRval == DDERR_WASSTILLDRAWING);
	if(ddRval != DD_OK)
	{
//		MessageBox(NULL,"blt error",NULL,MB_OK);
		return FALSE;
	}
	

	
	return TRUE;
}

