//#include "common.fxh"

float4x4  g_mProj ;
texture g_txDiffuse = NULL ;

sampler2D SamplerDiffuse = sampler_state
{
	Texture = <g_txDiffuse>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Point ;
	ADDRESSU = Wrap;
	ADDRESSV = Wrap;
};

struct PS_INPUT_STRUCT
{
	float4 f4Pos		:	POSITION ;
	float4 f4Color		:	COLOR0 ;
	float2 f2TexCoord0	:	TEXCOORD0 ;
};

PS_INPUT_STRUCT vs_main ( float4 vPos : POSITION,
						 float4 f4Color : COLOR0,
						 float2 vTex0 : TEXCOORD0 )
{
	vPos.w = 1.0f ;
	vPos = mul ( vPos, g_mProj ) ;

	PS_INPUT_STRUCT psIn ;

	psIn.f4Pos = vPos ;
	psIn.f2TexCoord0 = vTex0 ;
	psIn.f4Color = f4Color ;

	return psIn ;
}

float4 ps_main ( PS_INPUT_STRUCT psIn ) : COLOR0
{
	float4 f4Texture	= tex2D ( SamplerDiffuse, psIn.f2TexCoord0 ) ;
	float4 f4Modulate	= float4 ( psIn.f4Color.r * f4Texture.x, psIn.f4Color.g * f4Texture.y, psIn.f4Color.b * f4Texture.z, 1.0f ) ;
	float4 f4Result		= float4 ( f4Modulate.xyz, f4Texture.a * psIn.f4Color.a ) ;
	//float4 f4Result		= float4 ( psIn.f4Color.rgb, f4Texture.a * psIn.f4Color.a ) ;
	//return psIn.f4Color ;

	return f4Result ;
}

//////// techniques ////////////////////////////

technique Gui
{
	pass p0
	{
		VertexShader = compile vs_3_0 vs_main();
        PixelShader  = compile ps_3_0 ps_main();
		AlphaBlendEnable	= True ;
		SrcBlend			= SrcAlpha ;
		DestBlend			= InvSrcAlpha ;
		BlendOp				= Add ;
		CullMode			= None ;
		ZEnable				= False ;
		ScissorTestEnable	= True ;
	} 
}

