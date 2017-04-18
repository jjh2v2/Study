//========================================================================
//
//	Inferred Rendering Sample
//
//		by MJP  (mpettineo@gmail.com)
//      http://mynameismjp.wordpress.com
//		01/09/2010      
//
//========================================================================

#include "Common.fxh"

float4x4 WorldViewProjection;

void ShadowMapVS(in float4 PositionOS : POSITION0,
				   out float4 PositionCS : POSITION0,
				   out float2 DepthCS : TEXCOORD0)
{
	PositionCS = mul(PositionOS, WorldViewProjection);
	DepthCS = PositionCS.zw;
}

float4 ShadowMapPS(in float2 DepthCS : TEXCOORD0) : COLOR0
{
	return DepthCS.x / DepthCS.y;
}

// Vanilla shadow mapping
technique ShadowMap
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 ShadowMapVS();
        PixelShader = compile ps_3_0 ShadowMapPS();
        
        ZEnable = true;
        ZWriteEnable = true;
		ZFunc = LESSEQUAL;
        CullMode = CCW;
        FillMode = Solid;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
		StencilEnable = false;
		SRGBWriteEnable = false;
    }
}





