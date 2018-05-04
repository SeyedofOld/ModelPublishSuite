#include "common.fxh"


#ifdef _3DSMAX_
#else
#endif 

float4 g_f4AmbientColor = { 0.577, 0.577, 0.577, 1.0 } ;
float4 g_f4DiffuseColor = { 1, 1, 1, 1 } ;

float g_fTransparency		= 1.0f ;
float g_fAlphaTest			= 0.1f ;
float g_fGlossiness			= 100.0f ;
float g_fSpecularIntensity	= 1.0f ;
float g_fReflectionFactor	= 1.0f ;

texture g_txDiffuse ;
texture g_txAlpha ;
texture g_txNormal ;
texture g_txSpecular ;
texture g_txReflection ;

sampler2D SamplerDiffuse = sampler_state
{
	Texture = <g_txDiffuse>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	ADDRESSU = WRAP;
	ADDRESSV = WRAP;
};

sampler2D SamplerAlpha = sampler_state
{
	Texture = <g_txAlpha>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	ADDRESSU = WRAP;
	ADDRESSV = WRAP;
};

sampler2D SamplerNormal = sampler_state
{
	Texture = <g_txNormal>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	ADDRESSU = WRAP;
	ADDRESSV = WRAP;
};

sampler2D SamplerSpecular = sampler_state
{
	Texture = <g_txSpecular>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	ADDRESSU = WRAP;
	ADDRESSV = WRAP;
};

sampler2D SamplerReflection = sampler_state
{
	Texture = <g_txReflection>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	ADDRESSU = WRAP;
	ADDRESSV = WRAP;
};


struct PS_INPUT_STRUCT
{
	float4	f4Pos		:	POSITION ;
	float3	f3Normal	:	NORMAL ;
	float2	f2TexCoord0	:	TEXCOORD0 ;
};

PS_INPUT_STRUCT vs_main ( float3 vPos : POSITION,
						  float3 vNormal : NORMAL,
						  float2 vTex0 : TEXCOORD0)
{
	PS_INPUT_STRUCT psIn ;

	float4 oPos ;
	oPos = mul ( float4(vPos,1), g_matWorld ) ;
	oPos = mul ( oPos, g_matView ) ;
	oPos = mul ( oPos, g_matProj );

	psIn.f3Normal = mul ( float4(vNormal, 0), g_matWorld ).xyz ;

	psIn.f2TexCoord0 = vTex0 ;
	psIn.f4Pos = oPos ;

	return psIn ;
}

bool g_bHasDiffTex	= false ;
bool g_bHasAlphaTex = false ;
bool g_bHasNormTex	= false ;
bool g_bHasSpecTex	= false ;
bool g_bHasReflTex	= false ;
bool g_bHasUv		= false ;
bool g_bHasNormal	= false ;

/*#define HAS_DIFF_TEX
#define HAS_NORMAL
#define HAS_SPEC_TEX
#define HAS_ALPHA_TEX
#define HAS_REFL_TEX*/

float4 ps_main ( PS_INPUT_STRUCT psIn ) : COLOR0
{
	float4 f4Albedo = g_f4DiffuseColor ;
	if ( g_bHasDiffTex && g_bHasUv )
		f4Albedo *= tex2D ( SamplerDiffuse, psIn.f2TexCoord0 ) ;

	float fNDotL = 1.0f ;
	if ( g_bHasNormal ) {
		psIn.f3Normal = normalize ( psIn.f3Normal ) ;
		fNDotL = max ( dot ( -g_vSunLightDir.xyz, psIn.f3Normal ), 0.0f ) ;
	}

	//float4 f4LightFactor = fNDotL * g_f4SunLightDiffuse ;
	//float4 f4AmbientFactor = g_f4SunLightAmbient * g_f4AmbientColor ;

	float4 f4DiffuseLit = f4Albedo ;// *( f4LightFactor + f4AmbientFactor ) ;

	float4 f4Result = f4DiffuseLit ;

	if ( g_bHasNormal ) {
		float4 f4Specular = 0 ;
		f4Specular = pow ( max ( dot ( float4(psIn.f3Normal, 0), - g_vSunLightDir ), 0.0f ), g_fGlossiness ) * g_f4SunLightDiffuse ;

		f4Specular *= g_fSpecularIntensity ;

		if ( g_bHasSpecTex && g_bHasUv ) 
			f4Specular *= tex2D ( SamplerSpecular, psIn.f2TexCoord0 ) ;

		f4Result = f4Result + f4Specular ;
	}

	f4Result.a = g_fTransparency ;
	if ( g_bHasAlphaTex )
		f4Result.a *= tex2D ( SamplerAlpha, psIn.f2TexCoord0 ).a ;

//	if ( f4Result.a < g_fAlphaTest )
		//discard ;

	return f4Result ;
}


//////// techniques ////////////////////////////

technique UberTechnique
{
	pass p0
	{
		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
		AlphaBlendEnable = True ;
		SrcBlend		= SrcAlpha ;
		DestBlend		= InvSrcAlpha ;
		CullMode		= None ;
	}
}

