#pragma once

#include <d3dx9.h>
#include "tlMathTypes.h"

using namespace tochal ;

class CCamera {

    public :

		enum MATRIX_TYPE	 { MAT_VIEW = 0x01 , MAT_PROJECTION = 0x02 } ;
		enum CAMERA_MODE	 { MODE_FREE , MODE_TARGET } ;
		enum PROJECTION_TYPE { PROJECTION_PERSPECTIVE , PROJECTION_ORTHO } ;
		enum PROJECTION_CENTER { CENTER_CENTERED , CENTER_OFFCENTER } ;
		
		struct CAMERA_DESC {

			CAMERA_MODE		  eMode ;
			PROJECTION_TYPE   eProjectionType ;
			PROJECTION_CENTER eProjectionCenter ;
			
			vector3 ptPos ;
			vector3 ptTarget ;
			
			float fYaw ;
			float fPitch ;
			float fRoll ;
			
			float fFovX ;
			float fFovY ;
			float fNear ;
			float fFar ;
			float fAspect ;

			float fLeft ;
			float fRight ;
			float fBottom ;
			float fTop ;

// 			struct OFF_CENTER_PARAMS {
// 				float l , r , b , t ;//, zn , zf ;
// 			} OffCenter ;
		} ;

		struct FRUSTUM {
			D3DXVECTOR3 ptHead ;
			D3DXVECTOR3 vNormalRight ;
			D3DXVECTOR3 vNormalTop ;
			D3DXVECTOR3 vNormalLeft ;
			D3DXVECTOR3 vNormalBottom ;
		} ;



//		BOOL m_bOffCenter ;
//		OFF_CENTER_PARAMS m_OffCenterParams ;

// 		D3DXVECTOR3 m_ptPos ;
// 		D3DXVECTOR3 m_ptTarget ;
		
// 		float m_fYaw ;
// 		float m_fPitch ;
// 		float m_fRoll ;
// 		float m_fFovX ;
// 		float m_fFovY ;
// 		float m_fNear ;
// 		float m_fFar ;
// 		float m_fAspect ;

//		DWORD m_dwMode ;


        CCamera() ;
        ~CCamera() ;

		BOOL Initialize ( float fFovX , float fAspect , float fNear , float fFar ) ;
		BOOL Initialize ( float fLeft , float fRight , float fBottom , float fTop , float fNear , float fFar ) ;
		BOOL InitializeOrtho ( float fWidth , float fHeight , float fNear , float fFar ) ;
		BOOL Kill () ;
		BOOL CleanUp () ;

		BOOL SetMode ( CAMERA_MODE eMode = MODE_FREE ) ;
		CAMERA_MODE GetMode () { return m_CameraDesc.eMode ; }
		BOOL SetProjectionType ( PROJECTION_TYPE eProjectionType ) ;
		PROJECTION_TYPE GetProjectionType () { return m_CameraDesc.eProjectionType ; }
		BOOL SetProjectionCenter ( PROJECTION_CENTER eProjectionCenter ) ;
		PROJECTION_CENTER GetProjectionCenter () { return m_CameraDesc.eProjectionCenter ; }
		BOOL SetAspect ( float fAspect , BOOL bUpdate = TRUE ) ;
		float GetAspect () { return m_CameraDesc.fAspect ; } 

		BOOL SetPosition ( const D3DXVECTOR3 &ptPos , BOOL bUpdate = TRUE ) ;
		BOOL SetPosition ( float fX , float fY , float fZ , BOOL bUpdate = TRUE ) ;
		const vector3& GetPosition () const { return m_CameraDesc.ptPos ; }
		BOOL SetTarget ( D3DXVECTOR3 ptTarget , BOOL bUpdate = TRUE ) ;
		BOOL SetTarget ( float fX , float fY , float fZ , BOOL bUpdate = TRUE ) ;
		const D3DXVECTOR3& GetTarget () const { return m_CameraDesc.ptTarget ; }

		BOOL SetOrientation ( float fYaw , float fPitch , float fRoll , BOOL bUpdate = TRUE ) ;
		BOOL SetYaw ( float fYaw , BOOL bUpdate = TRUE ) ;
		float GetYaw () { return m_CameraDesc.fYaw ; }
		BOOL SetPitch ( float fPitch , BOOL bUpdate = TRUE ) ;
		float GetPitch () { return m_CameraDesc.fPitch ; }
		BOOL SetRoll ( float fRoll , BOOL bUpdate = TRUE ) ;
		float GetRoll () { return m_CameraDesc.fRoll ; }

		BOOL SetFovX ( float fFovX , BOOL bUpdate = TRUE ) ;
		float GetFovX () { return m_CameraDesc.fFovX ; }
		BOOL SetFovY ( float fFovY , BOOL bUpdate = TRUE ) ;
		float GetFovY () { return m_CameraDesc.fFovY ; }
		BOOL SetNear ( float fNear , BOOL bUpdate = TRUE ) ;
		float GetNear () { return m_CameraDesc.fNear ; }
		BOOL SetFar ( float fFar , BOOL bUpdate = TRUE ) ;
		float GetFar () { return m_CameraDesc.fFar ; }
		BOOL SetViewRange ( float fNear , float fFar , BOOL bUpdate = TRUE ) ;
		
		BOOL SetCamera ( const CAMERA_DESC &CamDesc ) ;
		BOOL SetDistance ( float fDist , BOOL bUpdate = TRUE ) ;
		vector3 GetDirection () { return m_vDirection ; } ;
	
		BOOL UpdateMatrixes ( DWORD dwFlag = MAT_VIEW | MAT_PROJECTION ) ;
		BOOL UpdateViewMatrix ( ) ;
		BOOL UpdateProjectionMatrix ( ) ;
		BOOL UpdateFrustum ( ) ;

		BOOL IsInitialized () { return m_bInit ; }
		BOOL IsOrthoCamera () { return m_CameraDesc.eProjectionType == PROJECTION_ORTHO ; }
		BOOL IsPerspectiveCamera () { return m_CameraDesc.eProjectionType == PROJECTION_PERSPECTIVE ; }
		BOOL IsOffCenterCamera () { return m_CameraDesc.eProjectionCenter == CENTER_OFFCENTER ; }
		BOOL IsFreeCamera () { return m_CameraDesc.eMode == MODE_FREE ; }
		BOOL IsTargetCamera () { return m_CameraDesc.eMode == MODE_TARGET ; }

		const CAMERA_DESC& GetCamera () const { return m_CameraDesc ; }
		const D3DXMATRIX& GetViewMatrix () const { return m_matView ; }
		const D3DXMATRIX& GetProjectionMatrix () const { return m_matProj ; }
        
		const FRUSTUM& GetFrustum () const { return m_Frustum ; }
		BOOL SphereInFrustum ( D3DXMATRIX &matWorld , D3DXVECTOR3 &ptCenter , float fRadius ) ;

        BOOL SetD3DCamera ( LPDIRECT3DDEVICE9 pd3dDevice ) ;
		BOOL GetBillboardMatrix2D ( const D3DXVECTOR3 &ptPos , D3DXMATRIX &matBill ) ;
		BOOL GetBillboardMatrix3D ( const D3DXVECTOR3 &ptPos , D3DXMATRIX &matBillboard ) ;
		BOOL GetRayFromScreen ( POINT ptScreen , D3DXVECTOR3 *vRay , float fWidth , float fHeight ) ;
        DWORD GetOutCode ( const D3DXVECTOR3 &ptPoint ) ;

		vector3 GetRight();
		vector3 GetUp();

	protected:
        float PointToSurfaceDist ( D3DXVECTOR3 &ptPoint , D3DXVECTOR3 &ptCenter , D3DXVECTOR3 &vNormal ) ;
		void GetYawPitch ( const D3DXVECTOR3 &vVector, float &afYaw, float &afPitch );

		BOOL		m_bInit ;
		vector3		m_vDirection ;
		float		m_fDistance ;
		D3DXMATRIX	m_matView ;
		D3DXMATRIX	m_matProj ;

		CAMERA_DESC m_CameraDesc ;
		FRUSTUM		m_Frustum ; 

} ;


