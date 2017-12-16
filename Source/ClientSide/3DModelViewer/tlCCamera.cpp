#include "StdAfx.h"
#include "tlCCamera.h"

#ifdef TOCHAL_USE_MEM_MANAGER
#define new new(__FILE__, __LINE__)
#endif // TOCHAL_USE_MEM_MANAGER


CCamera::CCamera ( )
{
	m_bInit = FALSE ;
	
	m_vDirection = D3DXVECTOR3 ( 0.0f , 0.0f , 1.0f ) ;
	m_fDistance  = 1.0f ;

	ZeroMemory ( &m_CameraDesc , sizeof (CAMERA_DESC) ) ;
	ZeroMemory ( &m_Frustum , sizeof(FRUSTUM) ) ;

	D3DXMatrixIdentity ( &m_matView ) ;
	D3DXMatrixIdentity ( &m_matProj ) ;
}

CCamera::~CCamera ( )
{
	CleanUp() ;
}

BOOL CCamera::CleanUp ( )
{
	m_bInit = FALSE ;

	m_vDirection = D3DXVECTOR3 ( 0.0f , 0.0f , 1.0f ) ;
	m_fDistance  = 1.0f ;

	ZeroMemory ( &m_CameraDesc , sizeof (CAMERA_DESC) ) ;
	ZeroMemory ( &m_Frustum , sizeof(FRUSTUM) ) ;

	D3DXMatrixIdentity ( &m_matView ) ;
	D3DXMatrixIdentity ( &m_matProj ) ;

	return TRUE ;
}

BOOL CCamera::Initialize ( float fFovX , float fAspect , float fNear , float fFar ) 
{
	m_CameraDesc.fNear = fNear ;
	m_CameraDesc.fFar  = fFar ;

	m_CameraDesc.fFovX	 = fFovX ;
	m_CameraDesc.fAspect = fAspect ;
	m_CameraDesc.fFovY	 = 2.0f * atanf ( tanf ( fFovX / 2.0f ) / fAspect ) ;
	
	{ // Calculate additional parameters
		m_CameraDesc.fRight = fNear * tanf ( m_CameraDesc.fFovX / 2.0f ) ;
		m_CameraDesc.fLeft = -m_CameraDesc.fRight ;

		m_CameraDesc.fTop = fNear * tanf ( m_CameraDesc.fFovY / 2.0f ) ;
		m_CameraDesc.fBottom = - m_CameraDesc.fTop ;
	}

	m_CameraDesc.eProjectionType = PROJECTION_PERSPECTIVE ;
	m_CameraDesc.eProjectionCenter = CENTER_CENTERED ;

	m_bInit = TRUE;

	UpdateProjectionMatrix() ;
	D3DXMatrixIdentity ( &m_matView ) ;

    return TRUE ;
}

BOOL CCamera::Initialize ( float fLeft , float fRight , float fBottom , float fTop , float fNear , float fFar )
{
	m_CameraDesc.fLeft	 = fLeft ;
	m_CameraDesc.fRight  = fRight ;
	m_CameraDesc.fBottom = fBottom ;
	m_CameraDesc.fTop	 = fTop ;
	m_CameraDesc.fNear	 = fNear ;
	m_CameraDesc.fFar	 = fFar ;
	
	{ // Calculate additional parameters
		m_CameraDesc.fAspect = ( fRight - fLeft ) / ( fTop - fBottom ) ;

		float fFovRight = 2.0f * atanf ( fRight / fNear ) ;
		float fFovLeft = 2.0f * atanf ( -fLeft / fNear ) ;
		m_CameraDesc.fFovX = fFovLeft + fFovRight ;

		float fFovBottom = 2.0f * atanf ( -fBottom / fNear ) ;
		float fFovTop = 2.0f * atanf ( fTop / fNear ) ;
		m_CameraDesc.fFovY = fFovBottom + fFovTop ;
	}

	m_CameraDesc.eProjectionType = PROJECTION_PERSPECTIVE ;
	m_CameraDesc.eProjectionCenter = CENTER_OFFCENTER ;

	m_bInit = TRUE;

	UpdateProjectionMatrix() ;
	D3DXMatrixIdentity ( &m_matView ) ;

	return TRUE ;
}

BOOL CCamera::InitializeOrtho ( float fWidth , float fHeight , float fNear , float fFar )
{
	m_CameraDesc.fNear = fNear ;
	m_CameraDesc.fFar  = fFar ;

	m_CameraDesc.fAspect = fWidth / fHeight ;
	m_CameraDesc.fFovX	 = 2.0f * atanf ( fWidth / 2.0f / fNear ) ;
	m_CameraDesc.fFovY	 = 2.0f * atanf ( fHeight / 2.0f / fNear ) ;

	{ // Calculate additional parameters
		m_CameraDesc.fRight = fWidth / 2.0f ;
		m_CameraDesc.fLeft = -fWidth / 2.0f ;

		m_CameraDesc.fTop = fHeight / 2.0f ;
		m_CameraDesc.fBottom = -fHeight / 2.0f ;
	}

	m_CameraDesc.eProjectionType = PROJECTION_ORTHO ;
	m_CameraDesc.eProjectionCenter = CENTER_CENTERED ;

	m_bInit = TRUE;

	UpdateProjectionMatrix() ;
	D3DXMatrixIdentity ( &m_matView ) ;

	return TRUE ;
}

BOOL CCamera::Kill ( ) 
{
	if ( !m_bInit )
		return FALSE ;

	BOOL bResult = CleanUp ( ) ;

	return bResult ;
}

BOOL CCamera::SetMode ( CAMERA_MODE eMode )
{
	if ( !m_bInit )
		return ( FALSE ) ;

//     if ( eMode == CAMERA_FREE && m_dwMode == CAMERA_TARGET ) {
//         
//     } else {
//         if ( eMode == CAMERA_FREE && m_dwMode == CAMERA_TARGET ) {
//         }
//     }

	m_CameraDesc.eMode = eMode ;
//	m_dwMode = eMode ;

	UpdateMatrixes () ;

	return TRUE ;
}

BOOL CCamera::SetProjectionType ( PROJECTION_TYPE eProjectionType )
{
	m_CameraDesc.eProjectionType = eProjectionType ;

	UpdateMatrixes () ;

	return TRUE ;
}

BOOL CCamera::SetProjectionCenter ( PROJECTION_CENTER eProjectionCenter )
{
	m_CameraDesc.eProjectionCenter = eProjectionCenter ;

	UpdateMatrixes () ;

	return TRUE ;
}

BOOL CCamera::SetAspect ( float fAspect , BOOL bUpdate )
{
	if ( !m_bInit )
		return FALSE ;

	if ( m_CameraDesc.eProjectionType == PROJECTION_PERSPECTIVE ) {
		
		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			m_CameraDesc.fAspect = fAspect ;
			m_CameraDesc.fFovY = 2.0f * atanf ( tanf ( m_CameraDesc.fFovX / 2.0f ) / m_CameraDesc.fAspect ) ;

			m_CameraDesc.fTop = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovY / 2.0f ) ;
			m_CameraDesc.fBottom = -m_CameraDesc.fTop ;
		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			return FALSE ;
		}

	} else if ( m_CameraDesc.eProjectionCenter == PROJECTION_ORTHO ) {

		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			return FALSE ;
		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			return FALSE ;
		}
	}

	m_CameraDesc.fAspect = fAspect ;

	if ( bUpdate )
		UpdateProjectionMatrix ( ) ;

	return TRUE ;
}

BOOL CCamera::SetPosition ( const D3DXVECTOR3 &ptPos , BOOL bUpdate)
{
	if ( !m_bInit )
		return FALSE ;

	//m_ptPos = ptPos ;

    if ( m_CameraDesc.eMode == MODE_TARGET ) {

		m_CameraDesc.ptPos = ptPos ;
		D3DXVECTOR3 vDist = m_CameraDesc.ptPos - m_CameraDesc.ptTarget ;
		m_fDistance = D3DXVec3Length ( &vDist ) ;

//         m_fDistance = sqrt ( ( m_ptPos.x - m_ptTarget.x ) * ( m_ptPos.x - m_ptTarget.x ) +
//                              ( m_ptPos.y - m_ptTarget.y ) * ( m_ptPos.y - m_ptTarget.y ) +
//                              ( m_ptPos.z - m_ptTarget.z ) * ( m_ptPos.z - m_ptTarget.z ) ) ;

        D3DXVECTOR3 vDir = m_CameraDesc.ptTarget - m_CameraDesc.ptPos ;
        D3DXVec3Normalize ( &vDir , &vDir ) ;
        m_vDirection = vDir ;

        if ( vDir.x == 0.0f && vDir.z == 0.0f )
            m_CameraDesc.fYaw = 0.0f ;
        else
            m_CameraDesc.fYaw = atan2f ( vDir.x , vDir.z ) ;

        D3DXMATRIX matRot ;
        D3DXMatrixRotationYawPitchRoll ( &matRot , -m_CameraDesc.fYaw , 0 , 0 ) ;
        D3DXVec3TransformCoord ( &vDir , &vDir , &matRot ) ;
        if ( vDir.y == 0.0f && vDir.z == 0.0f )
            m_CameraDesc.fPitch = 0.0f ;
        else
            m_CameraDesc.fPitch = -atan2f ( vDir.y , vDir.z ) ;

    } else {

		m_CameraDesc.ptPos = ptPos ;

		D3DXMATRIX matRot ;
		D3DXVECTOR3 vDir ( 0.0f , 0.0f , 1.0f ) ;

		D3DXMatrixRotationYawPitchRoll ( &matRot , m_CameraDesc.fYaw , m_CameraDesc.fPitch , m_CameraDesc.fRoll ) ;
		D3DXVec3TransformCoord ( &vDir , &vDir , &matRot ) ;

		m_vDirection = vDir ;
		m_CameraDesc.ptTarget = m_CameraDesc.ptPos + m_fDistance * m_vDirection ;
//         m_ptTarget.x = m_vDirection.x * m_fDistance + m_ptPos.x ;
//         m_ptTarget.y = m_vDirection.y * m_fDistance + m_ptPos.y ;
//         m_ptTarget.z = m_vDirection.z * m_fDistance + m_ptPos.z ;
    }


	if ( bUpdate )
		UpdateViewMatrix ( ) ;

	return TRUE ;
}

BOOL CCamera::SetPosition ( float fX , float fY , float fZ , BOOL bUpdate ) 
{
	if ( !m_bInit )
		return FALSE ;

	D3DXVECTOR3 ptPos ( fX , fY , fZ ) ;
	SetPosition ( ptPos , bUpdate ) ;

	return TRUE ;
}

BOOL CCamera::SetTarget ( D3DXVECTOR3 ptTarget , BOOL bUpdate )
{
	if ( !m_bInit )
		return ( FALSE ) ;
    
	if ( m_CameraDesc.eMode == MODE_FREE )
		return FALSE ;

	m_CameraDesc.ptTarget = ptTarget ;

    if ( m_CameraDesc.eMode == MODE_TARGET ) {
		
		D3DXVECTOR3 vDist = m_CameraDesc.ptPos - m_CameraDesc.ptTarget ;
		m_fDistance = D3DXVec3Length ( &vDist ) ;
		
//         m_fDistance = sqrt ( ( m_ptPos.x - m_ptTarget.x ) * ( m_ptPos.x - m_ptTarget.x ) +
//                              ( m_ptPos.y - m_ptTarget.y ) * ( m_ptPos.y - m_ptTarget.y ) +
//                              ( m_ptPos.z - m_ptTarget.z ) * ( m_ptPos.z - m_ptTarget.z ) ) ;
// 
        D3DXVECTOR3 vDir = m_CameraDesc.ptTarget - m_CameraDesc.ptPos ;
        D3DXVec3Normalize ( &vDir , &vDir ) ;
        m_vDirection = vDir ;

        if ( vDir.x == 0.0f && vDir.z == 0.0f )
            m_CameraDesc.fYaw = 0.0f ;
        else
            m_CameraDesc.fYaw = atan2f ( vDir.x , vDir.z ) ;
        
		D3DXMATRIX matRot ;
        D3DXMatrixRotationYawPitchRoll ( &matRot , -m_CameraDesc.fYaw , 0.0f , 0.0f ) ;
        D3DXVec3TransformCoord ( &vDir , &vDir , &matRot ) ;
        if ( vDir.y == 0.0f && vDir.z == 0.0f )
            m_CameraDesc.fPitch = 0.0f ;
        else
            m_CameraDesc.fPitch = -atan2f ( vDir.y , vDir.z ) ;

    } else {
    }

	if ( bUpdate )
		UpdateViewMatrix ( ) ;

	return ( TRUE ) ;
}

BOOL CCamera::SetTarget ( float fX , float fY , float fZ , BOOL bUpdate )
{
	if ( !m_bInit )
		return FALSE ;

    if ( m_CameraDesc.eMode == MODE_FREE )
        return FALSE ;

	D3DVECTOR ptTarget = { fX , fY , fZ } ;
	SetTarget ( ptTarget , bUpdate ) ;

	return TRUE ;
}

BOOL CCamera::SetOrientation ( float fYaw , float fPitch , float fRoll , BOOL bUpdate )
{
	if ( !m_bInit )
		return ( FALSE ) ;

	m_CameraDesc.fYaw = fYaw ;
	m_CameraDesc.fPitch = fPitch ;
	m_CameraDesc.fRoll = fRoll ;

    if ( m_CameraDesc.eMode == MODE_TARGET ) {

		D3DXMATRIX matRot ;
		D3DXVECTOR3 vDir ( 0.0f , 0.0f , m_fDistance ) ;

		D3DXMatrixRotationYawPitchRoll ( &matRot , m_CameraDesc.fYaw , m_CameraDesc.fPitch , m_CameraDesc.fRoll ) ;
		D3DXVec3TransformCoord ( &vDir , &vDir , &matRot ) ;

        m_CameraDesc.ptPos = m_CameraDesc.ptTarget - vDir ; 

        D3DXVec3Normalize ( &vDir , &vDir ) ;
        m_vDirection = vDir ;

    } else if ( m_CameraDesc.eMode == MODE_FREE ) {

		D3DXMATRIX matRot ;
		D3DXVECTOR3 vDir ( 0.0f , 0.0f , 1.0 ) ;

		D3DXMatrixRotationYawPitchRoll ( &matRot , m_CameraDesc.fYaw , m_CameraDesc.fPitch , m_CameraDesc.fRoll ) ;
		D3DXVec3TransformCoord ( &vDir , &vDir , &matRot ) ;
        m_vDirection = vDir ;

		m_CameraDesc.ptTarget = m_CameraDesc.ptPos + m_vDirection * m_fDistance ;
// 		m_ptTarget.x = m_ptPos.x + m_fDistance * m_vDirection.x ;
//         m_ptTarget.y = m_ptPos.y + m_fDistance * m_vDirection.y ;
//         m_ptTarget.z = m_ptPos.z + m_fDistance * m_vDirection.z ;
    }

	if ( bUpdate )
		UpdateViewMatrix ( ) ;

	return TRUE ;
}

BOOL CCamera::SetYaw ( float fYaw , BOOL bUpdate /*= TRUE */ )
{
	return SetOrientation ( fYaw , m_CameraDesc.fPitch , m_CameraDesc.fRoll , bUpdate ) ;
}

BOOL CCamera::SetPitch ( float fPitch , BOOL bUpdate /*= TRUE */ )
{
	return SetOrientation ( m_CameraDesc.fYaw , fPitch , m_CameraDesc.fRoll , bUpdate ) ;
}

BOOL CCamera::SetRoll ( float fRoll , BOOL bUpdate /*= TRUE */ )
{
	return SetOrientation ( m_CameraDesc.fYaw , m_CameraDesc.fPitch , fRoll , bUpdate ) ;
}

BOOL CCamera::SetFovX ( float fFovX , BOOL bUpdate ) 
{
	if ( !m_bInit )
		return ( FALSE ) ;


	if ( m_CameraDesc.eProjectionType == PROJECTION_PERSPECTIVE ) {

		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			m_CameraDesc.fFovX = fFovX ;
			m_CameraDesc.fFovY = 2.0f * atanf ( tanf ( fFovX / 2.0f ) / m_CameraDesc.fAspect ) ;

			m_CameraDesc.fRight = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovX / 2.0f ) ;
			m_CameraDesc.fLeft = -m_CameraDesc.fRight ;

			m_CameraDesc.fTop = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovY / 2.0f ) ;
			m_CameraDesc.fBottom = - m_CameraDesc.fTop ;

		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			return FALSE ;
		}

	} else if ( m_CameraDesc.eProjectionCenter == PROJECTION_ORTHO ) {

		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			
			m_CameraDesc.fFovX = fFovX ;
			m_CameraDesc.fFovY = 2.0f * atanf ( tanf ( fFovX / 2.0f ) / m_CameraDesc.fAspect ) ;

			m_CameraDesc.fRight = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovX / 2.0f ) ;
			m_CameraDesc.fLeft = -m_CameraDesc.fRight ;

			m_CameraDesc.fTop = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovY / 2.0f ) ;
			m_CameraDesc.fBottom = - m_CameraDesc.fTop ;

		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			return FALSE ;
		}
	}

	if ( bUpdate )
		UpdateProjectionMatrix ( ) ;

	return TRUE ;
}

BOOL CCamera::SetFovY ( float fFovY , BOOL bUpdate ) 
{
	if ( !m_bInit )
		return ( FALSE ) ;

	if ( m_CameraDesc.eProjectionType == PROJECTION_PERSPECTIVE ) {

		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			m_CameraDesc.fFovY = fFovY ;
			m_CameraDesc.fFovX = 2.0f * atanf ( tanf ( fFovY / 2.0f ) * m_CameraDesc.fAspect ) ;

			m_CameraDesc.fRight = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovX / 2.0f ) ;
			m_CameraDesc.fLeft = -m_CameraDesc.fRight ;

			m_CameraDesc.fTop = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovY / 2.0f ) ;
			m_CameraDesc.fBottom = - m_CameraDesc.fTop ;

		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			return FALSE ;
		}

	} else if ( m_CameraDesc.eProjectionCenter == PROJECTION_ORTHO ) {

		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			m_CameraDesc.fFovY = fFovY ;
			m_CameraDesc.fFovX = 2.0f * atanf ( tanf ( fFovY / 2.0f ) * m_CameraDesc.fAspect ) ;

			m_CameraDesc.fRight = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovX / 2.0f ) ;
			m_CameraDesc.fLeft = -m_CameraDesc.fRight ;

			m_CameraDesc.fTop = m_CameraDesc.fNear * tanf ( m_CameraDesc.fFovY / 2.0f ) ;
			m_CameraDesc.fBottom = - m_CameraDesc.fTop ;

		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			return FALSE ;
		}
	}
	
	if ( bUpdate )
		UpdateProjectionMatrix ( ) ;

	return TRUE ;
}

BOOL CCamera::SetNear ( float fNear , BOOL bUpdate /*= TRUE */ )
{
	return SetViewRange ( fNear , m_CameraDesc.fFar ) ;
}

BOOL CCamera::SetFar ( float fFar , BOOL bUpdate /*= TRUE */ )
{
	return SetViewRange ( m_CameraDesc.fNear , fFar ) ;
}

BOOL CCamera::SetViewRange ( float fNear , float fFar , BOOL bUpdate )
{
	if ( !m_bInit )
		return FALSE ;

	if ( m_CameraDesc.eProjectionType == PROJECTION_PERSPECTIVE ) {

		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {

			m_CameraDesc.fNear = fNear ;
			m_CameraDesc.fFar = fFar ;

		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {

			m_CameraDesc.fNear = fNear ;
			m_CameraDesc.fFar = fFar ;

			float fFovRight = 2.0f * atanf ( m_CameraDesc.fRight / fNear ) ;
			float fFovLeft = 2.0f * atanf ( -m_CameraDesc.fLeft / fNear ) ;
			m_CameraDesc.fFovX = fFovLeft + fFovRight ;

			float fFovBottom = 2.0f * atanf ( -m_CameraDesc.fBottom / fNear ) ;
			float fFovTop = 2.0f * atanf ( m_CameraDesc.fTop / fNear ) ;
			m_CameraDesc.fFovY = fFovBottom + fFovTop ;

		}

	} else if ( m_CameraDesc.eProjectionCenter == PROJECTION_ORTHO ) {

		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			
			m_CameraDesc.fNear = fNear ;
			m_CameraDesc.fFar = fFar ;

		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			m_CameraDesc.fNear = fNear ;
			m_CameraDesc.fFar = fFar ;
		}
	}
	
	
// 	m_fNear = fNear ;
// 	m_fFar = fFar ;
	if ( bUpdate )
		UpdateProjectionMatrix ( ) ;

	return TRUE ;
}

BOOL CCamera::SetDistance ( float fDist , BOOL bUpdate ) 
{
	if ( !m_bInit )
		return FALSE ;

	if ( m_CameraDesc.eMode == MODE_TARGET ) {

		m_fDistance = fDist ;

		D3DXMATRIX matRot ;
		D3DXVECTOR3 vDir ( 0.0f , 0.0f , m_fDistance ) ;

		D3DXMatrixRotationYawPitchRoll ( &matRot , m_CameraDesc.fYaw , m_CameraDesc.fPitch , 0 ) ;
		D3DXVec3TransformCoord ( &vDir , &vDir , &matRot ) ;

		m_CameraDesc.ptPos = m_CameraDesc.ptTarget - vDir ;

		if ( bUpdate )
			UpdateViewMatrix ( ) ;
	} else {

		m_fDistance = fDist ;

		m_CameraDesc.ptTarget = m_CameraDesc.ptPos + m_vDirection * m_fDistance ;
	}

	return TRUE ;
}

BOOL CCamera::UpdateMatrixes ( DWORD dwFlag )
{
	if ( !m_bInit )
		return FALSE ;

	if ( dwFlag | MAT_VIEW )
		UpdateViewMatrix ( ) ;
	if ( dwFlag | MAT_PROJECTION )
		UpdateProjectionMatrix ( ) ;

	return TRUE ;
}

BOOL CCamera::UpdateViewMatrix ( ) 
{
	if ( !m_bInit )
		return FALSE ;


	if ( m_CameraDesc.eMode == MODE_FREE ) {
		D3DXMATRIX matRot ;
		D3DXVECTOR3 vUpVec ( 0.0f , 1000.0f , 0.0f ) ;
		D3DXVECTOR3 vEyePt ;
		D3DXVECTOR3 vLookatPt ;
		D3DXVECTOR3 vDir ( 0.0f , 0.0f , 1000.0f ) ;
		vEyePt = m_CameraDesc.ptPos ;


		D3DXMatrixRotationYawPitchRoll ( &matRot , m_CameraDesc.fYaw , m_CameraDesc.fPitch , m_CameraDesc.fRoll ) ;
		D3DXVec3TransformCoord ( &vDir , &vDir , &matRot ) ;
		m_vDirection = vDir / 1000.0f ;

		vLookatPt.x = vEyePt.x + vDir.x ;
		vLookatPt.y = vEyePt.y + vDir.y ;
		vLookatPt.z = vEyePt.z + vDir.z ;

		D3DXVec3TransformCoord ( &vUpVec , &vUpVec , &matRot ) ;
		D3DXMatrixLookAtLH ( &m_matView , &vEyePt , &vLookatPt , &vUpVec ) ;
	}

	if ( m_CameraDesc.eMode == MODE_TARGET ) {

		D3DXMATRIX matRot ;
		D3DXMATRIX matView ;
		D3DXVECTOR3 vUpVec ( 0.0f , 1000.0f , 0.0f ) ;
		D3DXVECTOR3 vEyePt ;
		D3DXVECTOR3 vLookatPt ;
		vEyePt = m_CameraDesc.ptPos ;
		vLookatPt = m_CameraDesc.ptTarget ;
		D3DXMatrixRotationZ ( &matRot , m_CameraDesc.fRoll ) ;
		D3DXVec3TransformCoord ( &vUpVec , &vUpVec , &matRot ) ;
		D3DXMatrixLookAtLH ( &m_matView , &vEyePt , &vLookatPt , &vUpVec ) ;
	}

    UpdateFrustum ( ) ;

	return TRUE ;
}

BOOL CCamera::UpdateProjectionMatrix ( )
{
	if ( !m_bInit )
		return FALSE ;

	if ( m_CameraDesc.eProjectionType == PROJECTION_PERSPECTIVE ) {
		
		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			D3DXMatrixPerspectiveFovLH ( &m_matProj , m_CameraDesc.fFovY , m_CameraDesc.fAspect , m_CameraDesc.fNear , m_CameraDesc.fFar ) ;
		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			D3DXMatrixPerspectiveOffCenterLH ( &m_matProj , m_CameraDesc.fLeft , m_CameraDesc.fRight , m_CameraDesc.fBottom , m_CameraDesc.fTop , m_CameraDesc.fNear , m_CameraDesc.fFar ) ;
		}

	} else if ( m_CameraDesc.eProjectionType == PROJECTION_ORTHO ) {

		if ( m_CameraDesc.eProjectionCenter == CENTER_CENTERED ) {
			D3DXMatrixOrthoLH ( &m_matProj , m_CameraDesc.fRight - m_CameraDesc.fLeft , m_CameraDesc.fTop - m_CameraDesc.fBottom , m_CameraDesc.fNear , m_CameraDesc.fFar ) ;
		} else if ( m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ) {
			D3DXMatrixOrthoOffCenterLH ( &m_matProj , m_CameraDesc.fLeft , m_CameraDesc.fRight , m_CameraDesc.fBottom , m_CameraDesc.fTop , m_CameraDesc.fNear , m_CameraDesc.fFar ) ;
		}

	}

// 	if( m_bOffCenter ) {
// 	}
// 	else {
// 		//	D3DXMatrixPerspectiveFovLH ( &m_matProj , m_fFovX / m_fAspect , m_fAspect , m_fNear , m_fFar ) ;
// 		//float fFovY = 2.0f * atan ( tan ( m_fFovX / 2.0f ) / m_fAspect ) ;
// 		//	D3DXMatrixPerspectiveFovLH ( &m_matProj , fFOVH , m_fAspect , m_fNear , m_fFar ) ;
// 		D3DXMatrixPerspectiveFovLH ( &m_matProj , m_fFovY, m_fAspect , m_fNear , m_fFar ) ;
// 	}

    UpdateFrustum () ;

	return TRUE ;
}

BOOL CCamera::SetCamera ( const CAMERA_DESC &CamDesc )
{
	m_bInit = TRUE ;

	m_CameraDesc = CamDesc ;

	UpdateMatrixes() ;

	return TRUE ;
}

BOOL CCamera::UpdateFrustum ()
{
    if ( !m_bInit )
        return FALSE ;

    D3DXVECTOR3 vRight ( -1.0f , 0 , 0 ) ;
    D3DXVECTOR3 vLeft  ( +1.0f , 0 , 0 ) ;
    D3DXVECTOR3 vTop    ( 0 , -1.0f , 0 ) ;
    D3DXVECTOR3 vBottom ( 0 , +1.0f , 0 ) ;

    D3DXMATRIX matYaw ;
    D3DXMATRIX matPitch ;

    D3DXMatrixRotationY ( &matYaw , -m_CameraDesc.fFovX / 2.0f ) ;
    D3DXVec3TransformCoord ( &vLeft , &vLeft , &matYaw ) ;

    D3DXMatrixRotationY ( &matYaw , m_CameraDesc.fFovX / 2.0f ) ;
    D3DXVec3TransformCoord ( &vRight , &vRight , &matYaw ) ;

//    float fFovY = 2.0f * atan ( tan ( m_C / 2.0f ) / m_fAspect ) ;
	//D3DXMatrixPerspectiveFovLH ( &m_matProj , fFovY , m_fAspect , m_fNear , m_fFar ) ;

    D3DXMatrixRotationX ( &matPitch , -m_CameraDesc.fFovY / 2.0f ) ;
    D3DXVec3TransformCoord ( &vTop , &vTop , &matPitch ) ;

    D3DXMatrixRotationX ( &matPitch , m_CameraDesc.fFovY / 2.0f ) ;
    D3DXVec3TransformCoord ( &vBottom , &vBottom , &matPitch ) ;

    D3DXMATRIX matRoll ;
    D3DXMATRIX matRot ;

    D3DXMatrixRotationY ( &matYaw   , m_CameraDesc.fYaw   ) ;
    D3DXMatrixRotationX ( &matPitch , m_CameraDesc.fPitch ) ;
    D3DXMatrixRotationZ ( &matRoll  , m_CameraDesc.fRoll  ) ;

    matRot = matRoll * matPitch * matYaw ;
    D3DXVec3TransformCoord ( &vLeft , &vLeft , &matRot ) ;
    D3DXVec3TransformCoord ( &vRight , &vRight , &matRot ) ;
    D3DXVec3TransformCoord ( &vTop , &vTop , &matRot ) ;
    D3DXVec3TransformCoord ( &vBottom , &vBottom , &matRot ) ;

/*    D3DXVECTOR4 vLeft2 ;
    D3DXVECTOR4 vRight2 ;
    D3DXVECTOR4 vTop2 ;
    D3DXVECTOR4 vBottom2 ;
    D3DXVec3Transform ( &vLeft2 , &vLeft , &matRot ) ;
    D3DXVec3Transform ( &vRight2 , &vRight , &matRot ) ;
    D3DXVec3Transform ( &vTop2 , &vTop , &matRot ) ;
    D3DXVec3Transform ( &vBottom2 , &vBottom , &matRot ) ;

    vLeft.x = vLeft2.x ;
    vLeft.y = vLeft2.y ;
    vLeft.z = vLeft2.z ;
    vRight.x = vRight2.x ;
    vRight.y = vRight2.y ;
    vRight.z = vRight2.z ;
    vTop.x = vTop2.x ;
    vTop.y = vTop2.y ;
    vTop.z = vTop2.z ;
    vBottom.x = vBottom2.x ;
    vBottom.y = vBottom2.y ;
    vBottom.z = vBottom2.z ;

    D3DXVec3Normalize ( &vLeft , &vLeft ) ;
    D3DXVec3Normalize ( &vRight , &vRight ) ;
    D3DXVec3Normalize ( &vTop , &vTop ) ;
    D3DXVec3Normalize ( &vBottom , &vBottom ) ;*/

    m_Frustum.ptHead = m_CameraDesc.ptPos ;
    m_Frustum.vNormalLeft = vLeft ;
    m_Frustum.vNormalRight = vRight ;
    m_Frustum.vNormalTop = vTop ;
    m_Frustum.vNormalBottom = vBottom ;

    return TRUE ;
}

BOOL CCamera::SphereInFrustum ( D3DXMATRIX &matWorld , D3DXVECTOR3 &ptCenter , float fRadius )
{
    if ( !m_bInit )
        return FALSE ;

	D3DXMATRIX matInverse = matWorld ;
//	D3DXMatrixInverse ( &matInverse , NULL , matWorld ) ;

	D3DXVECTOR3 ptCenterTransformed ;
	D3DXVec3TransformCoord ( &ptCenterTransformed , &ptCenter , &matInverse ) ;
    if ( PointToSurfaceDist ( ptCenterTransformed , m_Frustum.ptHead , m_Frustum.vNormalLeft ) < -fRadius )
        return FALSE ;

    if ( PointToSurfaceDist ( ptCenterTransformed , m_Frustum.ptHead , m_Frustum.vNormalRight ) < -fRadius )
        return FALSE ;

    if ( PointToSurfaceDist ( ptCenterTransformed , m_Frustum.ptHead , m_Frustum.vNormalTop ) < -fRadius )
        return FALSE ;

    if ( PointToSurfaceDist ( ptCenterTransformed , m_Frustum.ptHead , m_Frustum.vNormalBottom ) < -fRadius )
        return FALSE ;

    return TRUE ;
}

BOOL CCamera::SetD3DCamera ( LPDIRECT3DDEVICE9 pd3dDevice )
{
    if ( !m_bInit )
        return FALSE ;
    if ( !pd3dDevice )
        return FALSE ;

    pd3dDevice->SetTransform ( D3DTS_VIEW       , &m_matView ) ;
    pd3dDevice->SetTransform ( D3DTS_PROJECTION , &m_matProj ) ;

    return TRUE ;
}

BOOL CCamera::GetBillboardMatrix2D ( const D3DXVECTOR3 &ptPos , D3DXMATRIX &matBill )
{
	if ( !m_bInit )
		return FALSE ;

    D3DXMATRIX matTrans ;
    D3DXMATRIX matRot ;
    D3DXMATRIX matRotRoll ;
    D3DXMATRIX matRotPitch ;
    D3DXMATRIX matRotYaw ;
    D3DXMATRIX matResult ;

	D3DXVECTOR3 vTarget = ptPos ;
	D3DXVECTOR3 vSource = m_CameraDesc.ptPos ;
	D3DXVECTOR3 vDir = vTarget - vSource ;

    if( vDir.x > 0.0f )
        D3DXMatrixRotationY( &matRotYaw, -atanf(vDir.z/vDir.x)/*-D3DX_PI/2*/ );
    else
        D3DXMatrixRotationY( &matRotYaw, -atanf(vDir.z/vDir.x)/*+D3DX_PI/2*/ );

    D3DXMatrixTranslation ( &matTrans , ptPos.x , ptPos.y , ptPos.z ) ;

    D3DXMatrixRotationZ ( &matRotRoll , 0 ) ;
    D3DXMatrixRotationX ( &matRotPitch , 0 ) ;
    //D3DXMatrixRotationY ( &matRotYaw , m_Yaw ) ;

    D3DXMatrixMultiply ( &matRot , &matRotPitch , &matRotRoll ) ;
    D3DXMatrixMultiply ( &matRot , &matRotYaw , &matRot ) ;

//    D3DXMatrixMultiply ( &matResult , &matRot , &matTrans ) ;
	matResult = matRot ;

    matBill = matResult ;

	return TRUE ;
}

BOOL CCamera::GetBillboardMatrix3D ( const D3DXVECTOR3 &ptPos , D3DXMATRIX &matBillboard )
{
	if ( ! m_bInit )
		return FALSE ;

	D3DXVECTOR3 vDir = ptPos - m_CameraDesc.ptPos ;

	float fYaw, fPitch ;
	GetYawPitch ( vDir, fYaw, fPitch ) ;

	D3DXMATRIX matYaw , matPitch ;
	D3DXMatrixRotationX ( &matPitch, fPitch ) ;
	D3DXMatrixRotationY ( &matYaw, fYaw ) ;

	matBillboard = matPitch * matYaw ;

	return TRUE ;
}

BOOL CCamera::GetRayFromScreen ( POINT ptScreen , D3DXVECTOR3 *vRay , float fScreenWidth , float fScreenHeight )
{
    if ( !m_bInit )
        return FALSE ;
    if ( !vRay )
        return FALSE ;

    D3DXMATRIX matInvView ;
    D3DXMatrixInverse ( &matInvView , NULL , &m_matView ) ;

    D3DXVECTOR3 v ;
    v.x =  ( ( ( 2.0f * ptScreen.x ) / fScreenWidth  ) - 1 ) / m_matProj._11 ;
    v.y = -( ( ( 2.0f * ptScreen.y ) / fScreenHeight ) - 1 ) / m_matProj._22 ;
    v.z =  1.0f;

    vRay->x  = v.x * matInvView._11 + v.y * matInvView._21 + v.z * matInvView._31 ;
    vRay->y  = v.x * matInvView._12 + v.y * matInvView._22 + v.z * matInvView._32 ;
    vRay->z  = v.x * matInvView._13 + v.y * matInvView._23 + v.z * matInvView._33 ;

    return TRUE ;
}

DWORD CCamera::GetOutCode ( const D3DXVECTOR3 &ptPoint )
{
    if ( !m_bInit )
        return 0 ;

    DWORD dwOutCode = 0 ;
    float fLeft = m_Frustum.vNormalLeft.x * ( ptPoint.x - m_CameraDesc.ptPos.x ) +
                  m_Frustum.vNormalLeft.y * ( ptPoint.y - m_CameraDesc.ptPos.y ) +
                  m_Frustum.vNormalLeft.z * ( ptPoint.z - m_CameraDesc.ptPos.z ) ;
    if ( fLeft < 0 )
        dwOutCode |= ( 1 << 0 ) ;

    float fRight = m_Frustum.vNormalRight.x * ( ptPoint.x - m_CameraDesc.ptPos.x ) +
                   m_Frustum.vNormalRight.y * ( ptPoint.y - m_CameraDesc.ptPos.y ) +
                   m_Frustum.vNormalRight.z * ( ptPoint.z - m_CameraDesc.ptPos.z ) ;
    if ( fRight < 0 )
        dwOutCode |= ( 1 << 1 ) ;

    float fTop = m_Frustum.vNormalTop.x * ( ptPoint.x - m_CameraDesc.ptPos.x ) +
                   m_Frustum.vNormalTop.y * ( ptPoint.y - m_CameraDesc.ptPos.y ) +
                   m_Frustum.vNormalTop.z * ( ptPoint.z - m_CameraDesc.ptPos.z ) ;
    if ( fTop < 0 )
        dwOutCode |= ( 1 << 2 ) ;

    float fBottom = m_Frustum.vNormalBottom.x * ( ptPoint.x - m_CameraDesc.ptPos.x ) +
                   m_Frustum.vNormalBottom.y * ( ptPoint.y - m_CameraDesc.ptPos.y ) +
                   m_Frustum.vNormalBottom.z * ( ptPoint.z - m_CameraDesc.ptPos.z ) ;
    if ( fBottom < 0 )
        dwOutCode |= ( 1 << 3 ) ;

    return dwOutCode ;
}

float CCamera::PointToSurfaceDist ( D3DXVECTOR3 &ptPoint , D3DXVECTOR3 &ptCenter , D3DXVECTOR3 &vNormal )
{
	float fDist ;
	fDist = vNormal.x * ( ptPoint.x - ptCenter.x ) +
		vNormal.y * ( ptPoint.y - ptCenter.y ) +
		vNormal.z * ( ptPoint.z - ptCenter.z ) ;

	//    fDist /= sqrt ( vNormal.x * vNormal.x + vNormal.y * vNormal.y + vNormal.z * vNormal.z ) ;

	return fDist ;
}

void CCamera::GetYawPitch ( const D3DXVECTOR3 &vVector, float &afYaw, float &afPitch ) 
{
	float fRxy , fYaw, fPitch;

	fRxy = sqrtf( vVector.x * vVector.x + vVector.z * vVector.z ) ;
	fYaw = 0 ;
	if ( vVector.x != 0 || vVector.z != 0 )
		fYaw = atan2f ( vVector.x , vVector.z ) ;
	fPitch = 0.0f ;
	if ( vVector.y != 0 || fRxy != 0 )
		fPitch = -atan2f ( vVector.y , fRxy ) ;

	afYaw = fYaw ;
	afPitch = fPitch ;
}

vector3 CCamera::GetRight()
{
	return vector3(m_matView._11, m_matView._21, m_matView._31);
}

vector3 CCamera::GetUp()
{
	return vector3(m_matView._12, m_matView._22, m_matView._32);
}

