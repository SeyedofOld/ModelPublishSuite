//--------------------------------------------------------------//
// Globals														//
//--------------------------------------------------------------//


/*#define ROW_LIGHT_DIRECTION	0
#define ROW_LIGHT_COLOR		1
#define ROW_LIGHT_AMBIENT	2*/



/*shared float4x4 g_matSunLight	: SUNLIGHT ;		// Sun Light characteristics
													// Row1 : Direction
													// Row2 : Diffuse
													// Row3 : Ambient
													// Row4 : Reserved*/
shared float4 g_vSunLightDir ;
shared float4 g_f4SunLightDiffuse ;
shared float4 g_f4SunLightAmbient ;


shared float	g_fTime	: TIME = 0.0f ;

#ifndef _3DSMAX_

	shared float4x4 g_matWorld		: WORLD ;			// World matrix for object
	shared float4x4 g_matView		: VIEW ;			// View matrix for camera
	shared float4x4 g_matProj		: PROJECTION ;		// Projection matrix for camera
	shared float4x4 g_matViewProj	: VIEWPROJECTION ;	// View*Projection matrix for camera

#else

	shared float4x4 g_matWorld		: 	World ;
	shared float4x4 g_matView		: 	View ;
	shared float4x4 g_matProj		: 	Projection ;

	float3 g_LightDir : Direction <
		string UIName = "Light Direction" ;
		string Object = "TargetLight" ;
		string Space = "World" ;
		int RefID = 0 ;
		//int LightRef = 0;
		//string UIWidget = "None";
	> ;

	float4 g_LightColor : LightColor
	<
		int LightRef = 0;
		//string UIName = "Light Color" ;
	> ;

#endif

