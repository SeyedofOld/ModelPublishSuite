#include "StdAfx.h"
#include "tlC3DViewContext.h"
#include "tlC3DGfx.h"
#include "tlMacros.h"
#include "tlCCamera.h"


C3DViewContext::C3DViewContext( const wchar_t* pszName, const HWND hWnd, const D3DVIEWPORT9& rViewport, const IDirect3DSurface9* pRenderTarget, const CCamera* pCamera, bool bOffScreen, bool bOwnCamera, bool bOwnTarget )
{
//	bOffScreen = false;

	m_Desc.hWnd				= hWnd ;
	m_Desc.Viewport			= rViewport ;
	m_Desc.pRenderTarget	= (IDirect3DSurface9*)pRenderTarget ;
	m_Desc.pCamera			= (CCamera*)pCamera ;
	m_Desc.bOffScreen		= bOffScreen ;
	m_Desc.bOwnCamera		= bOwnCamera ;
	m_Desc.bOwnRenderTarget = bOwnTarget ;
	wcscpy_s(m_Desc.szName, 20, pszName);

	m_pPrevTarget			= NULL ;

	HRESULT hr ;
	if ( ! pRenderTarget ) {
		C3DGfx::GetInstance()->GetDevice()->GetRenderTarget ( 0 , &m_Desc.pRenderTarget ) ;
	} else {
		D3DPRESENT_PARAMETERS pp = C3DGfx::GetInstance()->GetPresentParams() ;
		hr = C3DGfx::GetInstance()->GetDevice()->CreateAdditionalSwapChain ( &pp , &m_Desc.pSwapChain ) ;
	}

	C3DGfx::GetInstance()->GetDevice()->GetDepthStencilSurface ( &m_Desc.pDepthBuffer ) ;
}

C3DViewContext::~C3DViewContext()
{
	CleanUp() ;
}

void C3DViewContext::CleanUp()
{
	if ( m_Desc.bOwnCamera ) 
		TOCHAL_DELETE ( m_Desc.pCamera ) ;
	if ( m_Desc.bOwnRenderTarget )
		TOCHAL_RELEASE ( m_Desc.pRenderTarget ) ;

	ZeroMemory ( &m_Desc , sizeof(VIEW_DESC) ) ;
}

bool C3DViewContext::CreateRenderTarget ( int nWidth, int nHeight, bool bRenderToTexture /*= false*/ )
{
	if ( !m_Desc.bOffScreen && bRenderToTexture )
		return false ;

	HRESULT hr ;
	
	if ( m_Desc.bOffScreen ) {

		if ( bRenderToTexture ) {
			hr = C3DGfx::GetInstance()->GetDevice()->CreateTexture(nWidth,
				nHeight,
				1,
				D3DUSAGE_RENDERTARGET,
				C3DGfx::GetInstance()->GetPresentParams().BackBufferFormat,
				D3DPOOL_DEFAULT,
				&m_Desc.pRenderTexture,
				NULL);

			if (FAILED(hr))
				return false;

			m_Desc.pRenderTexture->GetSurfaceLevel(0, &m_Desc.pRenderTarget);

		} 
		else {

			hr = C3DGfx::GetInstance()->GetDevice()->CreateRenderTarget(nWidth,
				nHeight,
				C3DGfx::GetInstance()->GetPresentParams().BackBufferFormat,
				C3DGfx::GetInstance()->GetPresentParams().MultiSampleType,
				C3DGfx::GetInstance()->GetPresentParams().MultiSampleQuality,
				FALSE,
				&m_Desc.pRenderTarget,
				NULL);

			if (FAILED(hr))
				return false;
		}

		D3DPRESENT_PARAMETERS pp = C3DGfx::GetInstance()->GetPresentParams() ;
		pp.BackBufferWidth = nWidth ;
		pp.BackBufferHeight = nHeight ;
		hr = C3DGfx::GetInstance()->GetDevice()->CreateAdditionalSwapChain ( &pp, &m_Desc.pSwapChain ) ;

	} 
	else {
		D3DPRESENT_PARAMETERS pp = C3DGfx::GetInstance()->GetPresentParams();
		pp.BackBufferWidth = nWidth;
		pp.BackBufferHeight = nHeight;
		hr = C3DGfx::GetInstance()->GetDevice()->CreateAdditionalSwapChain(&pp, &m_Desc.pSwapChain);

		if (bRenderToTexture) {
			hr = m_Desc.pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_Desc.pRenderTarget);
		}
		else {
			hr = m_Desc.pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_Desc.pRenderTarget);
		}
	}
	
	hr = C3DGfx::GetInstance()->GetDevice()->CreateDepthStencilSurface ( nWidth , 
		nHeight , 
		C3DGfx::GetInstance()->GetPresentParams().AutoDepthStencilFormat ,
		C3DGfx::GetInstance()->GetPresentParams().MultiSampleType ,
		C3DGfx::GetInstance()->GetPresentParams().MultiSampleQuality ,
		TRUE ,
		&m_Desc.pDepthBuffer ,
		NULL ) ;

	if ( FAILED(hr) )
		return false ;

	return true ;
}

bool C3DViewContext::SelectView()
{
	HRESULT hr ;

	hr = C3DGfx::GetInstance()->GetDevice()->GetRenderTarget ( 0 , &m_pPrevTarget ) ;

	hr = C3DGfx::GetInstance()->GetDevice()->SetRenderTarget ( 0 , m_Desc.pRenderTarget ) ;
	hr = C3DGfx::GetInstance()->GetDevice()->SetDepthStencilSurface ( m_Desc.pDepthBuffer ) ;

	hr = C3DGfx::GetInstance()->GetDevice()->SetViewport ( &m_Desc.Viewport ) ;
	if ( FAILED(hr) )
		return false ;

	if ( m_Desc.pCamera ) {
		m_Desc.pCamera->SetD3DCamera ( C3DGfx::GetInstance()->GetDevice() ) ;
	}

// 	if ( m_Desc.pCamera ) {
// 		CGlobalShaderParams::GetInstance()->SetView( m_Desc.pCamera->GetViewMatrix() );
// 		CGlobalShaderParams::GetInstance()->SetProj( m_Desc.pCamera->GetProjectionMatrix() );
// 	}
	return true ;
}

bool C3DViewContext::ShowFrame ( RECT *prcSrc, RECT *prcDest, HWND hWnd )
{
	if ( m_Desc.pSwapChain ) {

		HRESULT hr = m_Desc.pSwapChain->Present ( prcSrc , prcDest , hWnd , NULL , 0 ) ;

		if ( FAILED ( hr ) ) {
			hr = C3DGfx::GetInstance()->GetDevice()->Reset ( &C3DGfx::GetInstance()->GetPresentParams() ) ;
			if ( FAILED ( hr ) )
				return false ;
			else
				return true ;
		}

	} else {
		//HRESULT hr;
		//IDirect3DSurface9* pBackBuf ;
		//hr = C3DGfx::GetInstance()->GetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuf);
		
		//C3DGfx::GetInstance()->GetDevice()->StretchRect(m_Desc.pRenderTarget, NULL, pBackBuf, NULL, D3DTEXF_NONE);

		return ( C3DGfx::GetInstance()->ShowFrame ( prcSrc , prcDest , hWnd ) == TRUE );
	}

	return true ;
}

void C3DViewContext::RestoreRenderTarget()
{
	//HRESULT hr ;

	//hr = C3DGfx::GetInstance()->GetDevice()->SetRenderTarget ( 0 , m_pPrevTarget ) ;
	//hr = hr ;
}



