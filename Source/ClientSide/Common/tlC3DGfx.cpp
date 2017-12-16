#include "StdAfx.h"
#include "tlC3DGfx.h"
#include "tlMacros.h"


C3DGfx::C3DGfx ( )
{
	m_bInit		= FALSE ;

	m_pDirect3D	  = NULL ;
	m_pd3dDevice  = NULL ;
	m_pd3dxSprite = NULL ;
	m_pd3dxLine   = NULL ;
	m_pd3dxFont   = NULL ;
	m_pEffectPool = NULL ;

	ZeroMemory ( &m_PresentParams , sizeof(D3DPRESENT_PARAMETERS) ) ;
}

C3DGfx::~C3DGfx ( )
{
	CleanUp() ;
}

BOOL C3DGfx::Initialize( HWND hWnd , DWORD dwWidth , DWORD dwHeight , D3DFORMAT d3dFormat , BOOL bFullScreen , DWORD dwDeviceId , BOOL bAntiAlias /*= FALSE */ )
{
    if ( m_bInit )
        return FALSE ;

	//bAntiAlias = FALSE ;

	HRESULT hr ;

	m_pDirect3D = Direct3DCreate9 ( D3D_SDK_VERSION ) ;
	if ( !m_pDirect3D )
		return FALSE ;

	ZeroMemory ( &m_PresentParams , sizeof ( D3DPRESENT_PARAMETERS ) ) ;
	
	m_PresentParams.BackBufferWidth  = dwWidth ;
	m_PresentParams.BackBufferHeight = dwHeight ;
	m_PresentParams.BackBufferFormat = d3dFormat ;
	m_PresentParams.BackBufferCount  = 1 ;
	m_PresentParams.MultiSampleType  = D3DMULTISAMPLE_NONE ;
	if ( bAntiAlias )
		m_PresentParams.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES ;
	m_PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD ;
	m_PresentParams.hDeviceWindow = hWnd ;
	m_PresentParams.Windowed = !bFullScreen ;
	m_PresentParams.EnableAutoDepthStencil = TRUE ;
	m_PresentParams.AutoDepthStencilFormat = D3DFMT_D24S8 ;
	m_PresentParams.Flags = 0 ;
	m_PresentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT ;
	m_PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE ;

	
	hr = m_pDirect3D->CreateDevice ( dwDeviceId ,
									 D3DDEVTYPE_HAL ,
									 hWnd ,
									 D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
									 &m_PresentParams ,
									 &m_pd3dDevice ) ;

	if ( FAILED ( hr ) ) {
		hr = m_pDirect3D->CreateDevice ( dwDeviceId ,
										 D3DDEVTYPE_HAL ,
										 hWnd ,
										 D3DCREATE_SOFTWARE_VERTEXPROCESSING ,
										 &m_PresentParams ,
										 &m_pd3dDevice ) ;
	}
	if ( FAILED ( hr ) ) {
		hr = m_pDirect3D->CreateDevice ( dwDeviceId ,
										 D3DDEVTYPE_REF ,
										 hWnd ,
										 D3DCREATE_SOFTWARE_VERTEXPROCESSING ,
										 &m_PresentParams ,
										 &m_pd3dDevice ) ;
	}
	
	if ( FAILED ( hr ) ) {
		//CErrorLog::LogError( TRUE, 0.0f, "Unable to create D3D device", "", __LINE__, __FILE__ ) ;
		CleanUp ( ) ;
		return FALSE ;
	}

	/*
	hr = D3DXCreateSprite ( m_pd3dDevice , &m_pd3dxSprite ) ;
	if ( FAILED ( hr ) ) {
		CleanUp ( ) ;
		return FALSE ;
	}

	hr = D3DXCreateLine ( m_pd3dDevice , &m_pd3dxLine ) ;
	if ( FAILED ( hr ) ) {
		CleanUp ( ) ;
		return FALSE ;
	}

	//hr = D3DXCreateFont(m_pd3dDevice, 20, 0, 0, 1, FALSE, CP_ACP, 0, 0, 0, L"Consolas", &m_pd3dxFont);
	hr = D3DXCreateFont(m_pd3dDevice, 20, 0, 0, 1, FALSE, CP_ACP, 0, 0, 0, L"Consolas", &m_pd3dxFont);
	if ( FAILED ( hr ) ) {
		CleanUp ( ) ;
		return FALSE ;
	}

 	m_pd3dxFont->PreloadCharacters ( 0 , 65535 ) ;
 	m_pd3dxFont->PreloadGlyphs ( 0 , 65535 ) ;
	*/
	D3DXCreateEffectPool ( &m_pEffectPool ) ;

	InitializeCriticalSection ( &m_csDevice ) ;

    m_bInit = TRUE ;

    return TRUE ;
}

BOOL C3DGfx::BeginFrame ( )
{
	if ( !m_bInit )
		return FALSE ;

    m_pd3dDevice->SetRenderState ( D3DRS_LIGHTING , FALSE ) ;
    m_pd3dDevice->SetRenderState ( D3DRS_SPECULARENABLE  , FALSE ) ;

	HRESULT hr = m_pd3dDevice->BeginScene ( ) ;
	if ( FAILED ( hr ) )
		return FALSE ;

	return TRUE ;
}

BOOL C3DGfx::EndFrame ( )
{
	if ( !m_bInit )
		return FALSE ;

	HRESULT hr = m_pd3dDevice->EndScene ( ) ;
	if ( FAILED ( hr ) )
		return FALSE ;

	return TRUE ;
}

BOOL C3DGfx::ShowFrame ( RECT *prcSrc , RECT *prcDest , HWND hWnd )
{
	if ( !m_bInit )
		return FALSE ;

	HRESULT hr = m_pd3dDevice->Present ( prcSrc , prcDest , hWnd , NULL ) ;
	if ( FAILED ( hr ) ) {
		hr = m_pd3dDevice->Reset ( &m_PresentParams ) ;
		if ( FAILED ( hr ) )
			return FALSE ;
	}

	return TRUE ;
}

BOOL C3DGfx::CleanUp ( )
{
	TOCHAL_RELEASE ( m_pd3dxFont ) ;
	TOCHAL_RELEASE ( m_pd3dxLine ) ;
	TOCHAL_RELEASE ( m_pd3dxSprite ) ;
	TOCHAL_RELEASE ( m_pd3dDevice ) ;
	TOCHAL_RELEASE ( m_pDirect3D ) ;
	TOCHAL_RELEASE ( m_pEffectPool ) ;

	ZeroMemory ( &m_PresentParams , sizeof(D3DPRESENT_PARAMETERS) ) ;

	DeleteCriticalSection ( &m_csDevice ) ;

	m_bInit = FALSE ;

	return TRUE ;
}

BOOL C3DGfx::EnableZBuffer ( )
{
    if ( !m_bInit )
        return FALSE ;

    HRESULT hr ;
    hr = m_pd3dDevice->SetRenderState ( D3DRS_ZENABLE , D3DZB_TRUE ) ;
    if ( FAILED ( hr ) )
        return FALSE ;

    return TRUE ;
}

BOOL C3DGfx::DisableZBuffer ( )
{
    if ( !m_bInit )
        return FALSE ;

    HRESULT hr ;
    hr = m_pd3dDevice->SetRenderState ( D3DRS_ZENABLE , D3DZB_FALSE ) ;
    if ( FAILED ( hr ) )
        return FALSE ;

    return TRUE ;
}

BOOL C3DGfx::DisableCull ( )
{
	if ( !m_bInit )
		return FALSE ;

    HRESULT hr = m_pd3dDevice->SetRenderState ( D3DRS_CULLMODE , D3DCULL_NONE ) ;
    if ( FAILED ( hr ) )
        return FALSE ;

	return TRUE ;
}

BOOL C3DGfx::EnableCull ( DWORD dwCullMode )
{
	if ( !m_bInit )
		return FALSE ;

    HRESULT hr = m_pd3dDevice->SetRenderState ( D3DRS_CULLMODE , dwCullMode ) ;
    if ( FAILED ( hr ) )
        return FALSE ;

	return TRUE ;
}

BOOL C3DGfx::Clear ( D3DCOLOR d3dColor , DWORD dwFlags ) 
{
	if ( !m_bInit )
		return FALSE ;

    HRESULT hr = m_pd3dDevice->Clear ( 0L , NULL , dwFlags , d3dColor , 1.0f , 0L ) ;
    if ( FAILED ( hr ) )
        return FALSE ;

	return TRUE ;
}

BOOL C3DGfx::SaveScreenShot ( char *pszFileName ) 
{
	if ( !m_bInit )
		return FALSE ;
	if ( !pszFileName )
		return FALSE ;

	char szName [ MAX_PATH ] ;
	strcpy_s ( szName , MAX_PATH , pszFileName ) ;
	
	LPDIRECT3DSURFACE9 pSurface = NULL ;
	HRESULT hr = m_pd3dDevice->GetRenderTarget ( 0 , &pSurface ) ;
	if ( FAILED ( hr ) || !pSurface )
		return FALSE ;

	hr = D3DXSaveSurfaceToFileA ( szName , D3DXIFF_BMP , pSurface , NULL , NULL ) ;
	if ( FAILED ( hr ) )
		return FALSE ;

	return TRUE ;
}

BOOL C3DGfx::EnableLighting ( ) 
{
	if ( !m_bInit )
		return FALSE ;

	HRESULT hr = m_pd3dDevice->SetRenderState ( D3DRS_LIGHTING , TRUE ) ;
	if ( FAILED ( hr ) )
		return FALSE ;

	return TRUE ;
}

BOOL C3DGfx::DisableLighting ( ) 
{
	if ( !m_bInit )
		return FALSE ;

	HRESULT hr = m_pd3dDevice->SetRenderState ( D3DRS_LIGHTING , FALSE ) ;
	if ( FAILED ( hr ) )
		return FALSE ;

	return TRUE ;
}

D3DVIEWPORT9 C3DGfx::GetFullscreenViewport ()
{
	D3DVIEWPORT9 d3dVP ;
	d3dVP.Width = m_PresentParams.BackBufferWidth ;
	d3dVP.Height = m_PresentParams.BackBufferHeight ;
	d3dVP.X = 0 ;
	d3dVP.Y = 0 ;
	d3dVP.MinZ = 0.0f ;
	d3dVP.MaxZ = 1.0f ;

	return d3dVP ;
}

BOOL C3DGfx::SetWorldMatrix ( const D3DXMATRIX &matWorld )
{
	if ( !m_bInit )
		return FALSE ;

	m_pd3dDevice->SetTransform ( D3DTS_WORLD , &matWorld ) ;

	return TRUE ;
}

BOOL C3DGfx::SetViewMatrix ( const D3DXMATRIX &matView )
{
	if ( !m_bInit )
		return FALSE ;

	m_pd3dDevice->SetTransform ( D3DTS_VIEW , &matView ) ;

	return TRUE ;
}

BOOL C3DGfx::SetProjectionMatrix ( const D3DXMATRIX &matProj )
{
	if ( !m_bInit )
		return FALSE ;

	m_pd3dDevice->SetTransform ( D3DTS_PROJECTION , &matProj ) ;

	return TRUE ;
}

void C3DGfx::EnterGfxCS()
{
	EnterCriticalSection ( &m_csDevice ) ;
}

void C3DGfx::LeaveGfxCS()
{
	LeaveCriticalSection ( &m_csDevice ) ;
}

BOOL C3DGfx::Resize ( int iWidth, int iHeight )
{
	D3DPRESENT_PARAMETERS pp = m_PresentParams ;
	ZeroMemory ( &m_PresentParams, sizeof ( D3DPRESENT_PARAMETERS ) ) ;

	m_PresentParams.BackBufferWidth = iWidth ;
	m_PresentParams.BackBufferHeight = iHeight ;


// 	m_PresentParams.BackBufferWidth = dwWidth ;
// 	m_PresentParams.BackBufferHeight = dwHeight ;
	m_PresentParams.BackBufferFormat = D3DFMT_UNKNOWN ;
	m_PresentParams.BackBufferCount = 1 ;
	m_PresentParams.MultiSampleType = D3DMULTISAMPLE_NONE ;
	m_PresentParams.MultiSampleType = pp.MultiSampleType ;
	m_PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD ;
	m_PresentParams.hDeviceWindow = pp.hDeviceWindow ;
	m_PresentParams.Windowed = pp.Windowed ;
	m_PresentParams.EnableAutoDepthStencil = TRUE ;
	m_PresentParams.AutoDepthStencilFormat = D3DFMT_D24S8 ;
	m_PresentParams.Flags = 0 ;
	m_PresentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT ;
	m_PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE ;

// 	m_PresentParams.BackBufferCount = 0 ;
// 	m_PresentParams.BackBufferWidth = 0 ;
// 	m_PresentParams.BackBufferHeight = 0 ;
	//m_PresentParams.BackBufferFormat = D3DFMT_UNKNOWN ;

	HRESULT hr = m_pd3dDevice->Reset ( &m_PresentParams ) ;
	

	return TRUE ;
}
