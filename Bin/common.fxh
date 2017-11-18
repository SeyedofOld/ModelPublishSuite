//--------------------------------------------------------------//
// Globals														//
//--------------------------------------------------------------//


#define ROW_LIGHT_DIRECTION	0
#define ROW_LIGHT_COLOR		1
#define ROW_LIGHT_AMBIENT	2



shared float4x4 g_matSunLight	: SUNLIGHT ;		// Sun Light characteristics
													// Row1 : Direction
													// Row2 : Diffuse
													// Row3 : Ambient
													// Row4 : Reserved

shared float4x4 g_matSkyRotation ;

shared float	g_fTime	: TIME = 0.0f ;

shared float	g_fEnvironmentTemperature = 0.0f ;

#ifndef _3DSMAX_

shared float4x4 g_matWorld		: WORLD ;			// World matrix for object
shared float4x4 g_matView		: VIEW ;			// View matrix for camera
shared float4x4 g_matProj		: PROJECTION ;		// Projection matrix for camera
shared float4x4 g_matViewProj	: VIEWPROJECTION ;	// View*Projection matrix for camera


shared float4	g_f4FogColor	: FOGCOLOR = float4 ( 245/255.0f, 245/255.0, 255/255.0f, 0.0f ) ;	// Fog Color (Diffuse)
shared float4	g_f4FogParams			   = float4 ( 100.0f, 3000.0f, 0.98f, 0 ) ;					// Fog Parameters ( Start, End, MaxFogPower, Reserved )



shared float	g_fTerrainSize	= 1.0f ;
shared texture  g_txLightMap ;
shared float3   g_f3LightMapPos ;
shared float4x4 g_matLightMap ;

shared texture	g_txShadowMap ;
shared float	g_fShadowIntensity = 1.0f ;

sampler2D SamplerShadowMap = sampler_state
{
	Texture = <g_txShadowMap>;
	magfilter = linear;
	minfilter = linear;
	mipfilter = none;
};

inline float GetShadowFactor ( float x, float z )
{
	float4 f2ShadowMapCoord = float4 ( x, z, 0.0f, 0.0f ) / g_fTerrainSize ;
	f2ShadowMapCoord.y = 1.0 - f2ShadowMapCoord.y ;
	
	return lerp ( 1.0f, 0.0f, ( 1.0f - tex2Dlod ( SamplerShadowMap, f2ShadowMapCoord ).r ) * g_fShadowIntensity ) ;
	//clamp ( tex2Dlod ( SamplerShadowMap, f2ShadowMapCoord ).r, 0.0f, 1.0f ) ;
	//return clamp ( tex2Dlod ( SamplerShadowMap, f2ShadowMapCoord ).r, 0.0f, 1.0f ) ;
}

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

float4 g_f4FogParams = float4 ( 1000,1000,0,0 ) ;
float4 g_f4FogColor  = float4 ( 1,1,1,0 ) ;


#endif

float GetFogPower ( float4 vPos )
{
	float fFogPower = ( vPos.w - g_f4FogParams.x ) / ( g_f4FogParams.y - g_f4FogParams.x ) * g_f4FogParams.z ;
	if ( fFogPower > g_f4FogParams.z )
		fFogPower = 0.0f ;

	//fFogPower = clamp ( fFogPower , 0.0f , g_f4FogParams.z ) ;
	
	return fFogPower ;
}

float GetFogPower ( float fz )
{
	float fFogPower = ( fz - g_f4FogParams.x ) / ( g_f4FogParams.y - g_f4FogParams.x ) * g_f4FogParams.z ;
	if ( fFogPower > g_f4FogParams.z )
		fFogPower = 0.0f ;
	//fFogPower = clamp ( fFogPower , 0.0f , g_f4FogParams.z ) ;
	
	return fFogPower ;
}

float4 GetThermalColor ( float fTemp )
{
	float fBlackTemp = -1.0f ;
	float fWhiteTemp = 40.0f ;

	float fIntensity = smoothstep ( fBlackTemp, fWhiteTemp, fTemp ) ;
//	float fI = 0.8316008316008316008316008316008 * fTemp * fTemp / 10000.0 + 0.015841995841995841995841995842 * fTemp ;
	return float4 ( fIntensity, fIntensity, fIntensity, 1.0f ) ;
}

