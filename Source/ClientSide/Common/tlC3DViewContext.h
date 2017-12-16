#pragma once

#include "tlMathTypes.h"

class CCamera ;

class C3DViewContext 
{
    public :

		struct VIEW_DESC {
			D3DVIEWPORT9		Viewport ;
			HWND				hWnd ;
			IDirect3DSurface9*	pRenderTarget ;
			IDirect3DSurface9*  pDepthBuffer ;
			CCamera*			pCamera ;
			IDirect3DSwapChain9* pSwapChain ;
			bool				bOffScreen ;
			wchar_t				szName [ 20 ] ;
			bool				bOwnCamera ;
			bool				bOwnRenderTarget ;
			void*				pUserData ;
			IDirect3DTexture9*	pRenderTexture ;
			VIEW_DESC()
			{
				ZeroMemory ( this , sizeof(VIEW_DESC) ) ;
			}
		} ;
		
		C3DViewContext ( const wchar_t* pszName, const HWND hWnd, const D3DVIEWPORT9& rViewport, const IDirect3DSurface9* pRenderTarget, const CCamera* pCamera, bool bOffScreen, bool bOwnCamera, bool bOwnTarget ) ;
		~C3DViewContext() ;

		D3DVIEWPORT9 GetViewport() const { return m_Desc.Viewport; }
		void SetViewport(D3DVIEWPORT9 val) { m_Desc.Viewport = val; }
		HWND GetHWnd() const { return m_Desc.hWnd; }
		void SetHWnd(HWND val) { m_Desc.hWnd = val; }
		IDirect3DSurface9* GetRenderTarget() const { return m_Desc.pRenderTarget; }
		void SetRenderTarget(IDirect3DSurface9* val) { m_Desc.pRenderTarget = val; }
		CCamera* GetCamera() const { return m_Desc.pCamera; }
		void SetCamera(CCamera* val) { m_Desc.pCamera = val; }

 		const wchar_t* GetName() const { return m_Desc.szName; }
 		void SetName(wchar_t* pszName) { wcscpy_s ( m_Desc.szName , 20 , pszName ) ; }

        void CleanUp () ;

		bool CreateRenderTarget ( int nWidth, int nHeight, bool bRenderToTexrure = false ) ;
		bool SelectView() ;
		void RestoreRenderTarget() ;

		bool ShowFrame ( RECT *prcSrc = NULL , RECT *prcDest = NULL, HWND hWnd = NULL ) ;

		void* GetUserData() { return m_Desc.pUserData ; }
		void SetUserData ( void* pUserData ) { m_Desc.pUserData = pUserData ; }

		VIEW_DESC& GetViewDesc() { return m_Desc; }

		friend class CViewManager ;

private:
	VIEW_DESC m_Desc ;
	IDirect3DSurface9* m_pPrevTarget ;
} ;