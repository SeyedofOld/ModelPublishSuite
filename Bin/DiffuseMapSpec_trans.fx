#include "common.fxh"


#ifdef _3DSMAX_
#else
#endif 

float4 g_f4AmbientColor : Ambient <
	string UIName = "Ambient Color" ; 
> = { 0.577, 0.577, 0.577, 1.0 } ;

float4 g_f4DiffuseColor : Diffuse <
	string UIName = "Diffuse Color" ; 
> = { 1, 1, 1, 1 } ;

texture g_txDiffuse : DiffuseMap <
	string name = "seafloor.dds"; 
	string UIName = "Diffuse Texture";
>;

float g_fTransparency : Alpha <
	string UIName = "Transparency Value" ; 
> = 1.0f ;

float g_fAlphaTest <
	string UIName = "Alpha Test" ; 
> = 0.1f ;

float g_fTemperature <
	string UIName = "Temperature" ; 
> = 1.0f ;

float g_fGlossiness <
	string UIName = "Glossiness" ;
    string UIWidget = "FloatSpinner";
	float UIMin = 0.0 ;
	float UIMax = 1000.0 ;
	float UIStep = 1.0 ;
> ;

float g_fSpecularIntensity <
	string UIName = "Specular Intensity" ;
    string UIWidget = "FloatSpinner";
	float UIMin = 0.0 ;
	float UIMax = 100.0 ;
	float UIStep = 0.1 ;
> ;

sampler2D SamplerDiffuse = sampler_state
{
	Texture = <g_txDiffuse>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	ADDRESSU = WRAP;
	ADDRESSV = WRAP;
};

//sampler2D SamplerShadowMap = sampler_state
//{
//	Texture = <g_txShadowMap>;
//	magfilter = linear;
//	minfilter = linear;
//	mipfilter = none;
//};

struct PS_INPUT_STRUCT
{
	float4	f4Pos		:	POSITION ;
	float3	f3Normal	:	NORMAL ;
	float2	f2TexCoord0	:	TEXCOORD0 ;
	float	fFogPower	:	TEXCOORD1 ;
	float	fShadow		:	TEXCOORD2 ;
};

PS_INPUT_STRUCT vs_main ( float4 vPos : POSITION,
						 float3 vNormal : NORMAL,
						 float2 vTex0 : TEXCOORD0)
{
	vPos.w = 1.0f ;

	PS_INPUT_STRUCT psIn ;

	float4 oPos ;

#ifdef _3DSMAX_
	psIn.fShadow = 1.0f ;
#else
	//float4 f4ShadowMapCoord = float4 ( g_matWorld._41 , g_matWorld._43 , 0 , 0 ) / g_fTerrainSize ;
	//f4ShadowMapCoord.y = 1 - f4ShadowMapCoord.y ;
	//psIn.fShadow = clamp ( tex2Dlod ( SamplerShadowMap , f4ShadowMapCoord ).r * 1.5f , 0.2f , 1.0f ) ;
#endif


	oPos = mul( vPos, g_matWorld );
#ifndef _3DSMAX_
	psIn.fShadow = GetShadowFactor ( oPos.x, oPos.z ) ;
#endif

	psIn.f3Normal = normalize ( mul (vNormal , g_matWorld) ).xyz ;
	oPos = mul( oPos, g_matView );
	float fDist = oPos.z ;
	oPos = mul( oPos, g_matProj );

	psIn.f2TexCoord0 = vTex0 ;

	psIn.f4Pos = oPos ;


	psIn.fFogPower = GetFogPower ( oPos ) ;

	return psIn ;
}

float4 ps_main ( PS_INPUT_STRUCT psIn ) : COLOR0
{
	//return float4(1,0,0,0.1) ;

	// Base color
	float4 f4BaseClr	= tex2D ( SamplerDiffuse , psIn.f2TexCoord0 ) * g_f4DiffuseColor ;

	psIn.f3Normal = normalize ( psIn.f3Normal ) ;

	psIn.fShadow = 1;

#ifdef _3DSMAX_
	g_matSunLight [ ROW_LIGHT_DIRECTION ] = normalize ( float4(g_LightDir,0) ) ;
	g_matSunLight [ ROW_LIGHT_COLOR		] = g_LightColor ;
	g_matSunLight [ ROW_LIGHT_AMBIENT	] = float4(0.5,0.5,0.5,0) ;
#endif

	float fNDotL = max ( dot ( -g_matSunLight [ ROW_LIGHT_DIRECTION ].xyz , psIn.f3Normal ) , 0.0f ) ;

	float4 f4DiffuseFactor = fNDotL * g_matSunLight [ ROW_LIGHT_COLOR ] * psIn.fShadow ;
	float4 f4AmbientFactor = g_matSunLight [ ROW_LIGHT_AMBIENT ] * g_f4AmbientColor ;


	float4 f4Result = f4BaseClr ;

#ifdef _3DSMAX_

#else
	f4Result.rgb = lerp ( f4Result.rgb , g_f4FogColor.rgb * g_matSunLight [ 1 ].rgb , psIn.fFogPower ) ;
	//f4Result = lerp ( f4Result , g_f4FogColor * g_matSunLight [ 1 ] , psIn.fFogPower ) ;
	//f4Result *= g_matSunLight [ 1 ] ;
#endif
	f4Result *= ( f4DiffuseFactor + f4AmbientFactor ) ;

	float4 f4Specular = 0 ;

	f4Specular = pow ( max ( dot ( psIn.f3Normal, - g_matSunLight [ ROW_LIGHT_DIRECTION ] ), 0.0f ) , g_fGlossiness ) * g_fSpecularIntensity * g_matSunLight [ ROW_LIGHT_COLOR ] ;

	f4Result = f4Result + f4Specular ;

	f4Result.a = f4BaseClr.a * g_fTransparency ;
	if ( f4Result.a < g_fAlphaTest )
		discard ;

	return f4Result ;
}

float4 ps_main_night ( PS_INPUT_STRUCT psIn ) : COLOR0
{
	// Base color
	float4 f4BaseClr	= tex2D ( SamplerDiffuse , psIn.f2TexCoord0 ).yyyy ;


	// New new
	//f4BaseClr = GetThermalColor ( g_fTemperature + g_fEnvironmentTemperature + (f4BaseClr.g - 0.5f) * 0.2f ) ;

//	psIn.f3Normal = normalize ( psIn.f3Normal ) ;

// #ifdef _3DSMAX_
// 	g_matSunLight [ 0 ] = -normalize ( float4(g_LightDir,0) ) ;
// 	g_matSunLight [ 1 ] = g_LightColor ;
// 	g_matSunLight [ 2 ] = float4(0.5,0.5,0.5,0) ;
// #endif

//	float fNDotL = clamp ( dot ( -g_matSunLight [ 0 ].xyz , psIn.f3Normal ) , 0 , 2 ) ;

	float4 f4DiffuseFactor = 1.0f ;
	float4 f4AmbientFactor = 1.0f ;

	float4 f4Result = f4BaseClr ;

#ifdef _3DSMAX_
#else
	f4Result = lerp ( f4Result , g_f4FogColor * g_matSunLight [ 1 ] , psIn.fFogPower ) ;
	//f4Result *= g_matSunLight [ 1 ] ;
#endif

	f4Result *= f4DiffuseFactor + f4AmbientFactor ;
	
	float fColorTemp = 1.0 - dot ( f4Result, float4(0.2f, 0.7f, 0.1f, 0.0f) ) ;
	float fTemp = fColorTemp * 3.0f + g_fEnvironmentTemperature + g_fTemperature ;

	f4Result = GetThermalColor ( fTemp ) ;

	f4Result.a = 1.0f ;

	return f4Result ;
}

//////// techniques ////////////////////////////

technique DiffuseMapSpecTrans
{
	pass p0
	{
		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
		AlphaBlendEnable = True ;
		SrcBlend			= SrcAlpha ;
		DestBlend			= InvSrcAlpha ;
		CullMode = None ;
	}

}

technique DiffuseMapSpecTransNight
{
	pass p0
	{
		VertexShader = compile vs_3_0 vs_main();
        PixelShader  = compile ps_3_0 ps_main_night();
		AlphaBlendEnable = True ;
		SrcBlend			= SrcAlpha ;
		DestBlend			= InvSrcAlpha ;
		CullMode = None ;
	}
}

