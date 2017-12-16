#pragma once

#include <d3dx9.h>
#include "tlCSingleton.h"

class C3DGfx : public CSingleton<C3DGfx> {

public :

    C3DGfx ( ) ;
    ~C3DGfx ( ) ;

	BOOL Initialize ( HWND hWnd , DWORD dwWidth , DWORD dwHeight , D3DFORMAT d3dFormat , BOOL bFullScreen , DWORD dwDeviceId , BOOL bAntiAlias = FALSE ) ;
	BOOL CleanUp ( ) ;

	LPDIRECT3D9 GetD3D() { return m_pDirect3D ; }

	BOOL BeginFrame ( ) ;
	BOOL EndFrame ( ) ;
	BOOL ShowFrame ( RECT *prcSrc = NULL , RECT *prcDest = NULL, HWND hWnd = NULL ) ;

	BOOL Clear ( D3DCOLOR d3dColor , DWORD dwFlags = D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET ) ;

	D3DVIEWPORT9 GetFullscreenViewport () ;
	BOOL SaveScreenShot ( char *pszFileName ) ;

	BOOL EnableZBuffer ( ) ;
	BOOL DisableZBuffer ( ) ;
	BOOL DisableCull ( ) ;
	BOOL EnableCull ( DWORD dwCullMode = D3DCULL_CCW ) ;
	BOOL EnableLighting ( ) ;
	BOOL DisableLighting ( ) ;

	BOOL SetWorldMatrix ( const D3DXMATRIX &matWorld ) ;
	BOOL SetViewMatrix ( const D3DXMATRIX &matView ) ;
	BOOL SetProjectionMatrix ( const D3DXMATRIX &matProj ) ;

	BOOL IsInitialized() const { return m_bInit ; }
	LPDIRECT3DDEVICE9 GetDevice() const { return m_pd3dDevice ; }
	LPD3DXSPRITE GetD3DXSprite() const { return m_pd3dxSprite ; }
	LPD3DXLINE GetD3DXLine() const { return m_pd3dxLine ; }
	LPD3DXFONT GetD3DXFont() const { return m_pd3dxFont ; }
	DWORD GetWidth() const { return m_PresentParams.BackBufferWidth ; }
	DWORD GetHeight() const { return m_PresentParams.BackBufferHeight ; }
	DWORD GetFormat() const { return m_PresentParams.BackBufferFormat ; }
	BOOL IsAntiAliased() const { return m_PresentParams.MultiSampleType != D3DMULTISAMPLE_NONE ; }
	D3DPRESENT_PARAMETERS& GetPresentParams() { return m_PresentParams ; }
	LPD3DXFONT			m_pd3dxFont ;
	void EnterGfxCS () ;
	void LeaveGfxCS () ;

	BOOL Resize ( int iWidth, int iHeight );

	ID3DXEffectPool* GetEffectPool() { return m_pEffectPool ;  }

private :
	BOOL				m_bInit ;

	LPDIRECT3D9			m_pDirect3D ;
	LPDIRECT3DDEVICE9	m_pd3dDevice ;
	LPD3DXSPRITE		m_pd3dxSprite ;
	LPD3DXLINE			m_pd3dxLine ;
	ID3DXEffectPool*	m_pEffectPool ;
		
	D3DPRESENT_PARAMETERS m_PresentParams ;
	CRITICAL_SECTION	m_csDevice ;
} ;

