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

float4x4 WorldView;
float4x4 WorldViewProjection;

float FarClip;
float SpecularExponent;
float InstanceID;
float2 TexScale;

float4 OutputMask;

texture2D NormalMap;
sampler2D NormalSampler = sampler_state
{
    Texture = <NormalMap>;
    MinFilter = ANISOTROPIC;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    MaxAnisotropy = 16;
    AddressU = Wrap;
    AddressV = Wrap;
	SRGBTexture = false;
};


struct VSInput
{
	float4 PositionOS		: POSITION0;
	float3 NormalOS			: NORMAL0;
	float3 TangentOS		: TANGENT;
	float3 BinormalOS		: BINORMAL;
	float2 TexCoord			: TEXCOORD0;	
};

struct VSOutput
{
	float4 PositionCS		: POSITION0;
	float  Depth			: TEXCOORD0;
	float3 NormalVS			: TEXCOORD1;
	float3 TangentVS		: TEXCOORD2;
	float3 BinormalVS		: TEXCOORD3;
	float2 TexCoord			: TEXCOORD4;
};


VSOutput GBufferVS(in VSInput input)
{
	VSOutput output;
	
	// Transform the vertex into clip space
    output.PositionCS = mul(input.PositionOS, WorldViewProjection);

	// Pass along view-space z to the pixel shader, so it
	// can output view-space depth.
	float3 positionVS = mul(input.PositionOS, WorldView);
	output.Depth.x = positionVS.z;
	
	// Transform the tangent basis to view space, so we can
	// transform the normal map normals to view space
	output.NormalVS = mul(input.NormalOS, (float3x3)WorldView);
	output.TangentVS = mul(input.TangentOS, (float3x3)WorldView);
	output.BinormalVS = mul(input.BinormalOS, (float3x3)WorldView);
	
	output.TexCoord = input.TexCoord * TexScale;
	
	return output;
}

GBufferOutput GBufferPS(in VSOutput input, 
						in float2 ScreenPos : VPOS,
						uniform bool UseNormalMap, 
						uniform bool UseOutputMask)
{
	GBufferOutput output;

	// We'll normalize our view-space depth by dividing by the far clipping plane.
	// We'll also negate it, since z will be negative in a right-handed coordinate system.
	float depth = -input.Depth.x / FarClip;
	output.DepthID = float4(depth, InstanceID, 1.0f, 1.0f);
    
	float3 normalVS;

	if (UseNormalMap)
	{
		// Use the tangent basis to transform normals from the normal map to view-space
		float3x3 tangentBasis = float3x3(normalize(input.TangentVS), 
											normalize(input.BinormalVS), 
											normalize(input.NormalVS));
		float3 normalTS = tex2D(NormalSampler, input.TexCoord).xyz * 2.0f - 1.0f;
		normalVS = mul(normalTS, tangentBasis);
	}
	else
		normalVS = normalize(input.NormalVS);

    output.NormalsSpecular = PackNormalsAndSpecular(normalVS, SpecularExponent);

	// For transparents we use an output masks so that we render a stipple pattern
	if (UseOutputMask)
	{
		float2 pos = fmod(ScreenPos, 2);
		float index = pos.x + pos.y * 2;
		clip(OutputMask[index]);
	}

	return output;
}

// Opaques, no normal mapping
technique GBuffer
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 GBufferVS();
        PixelShader = compile ps_3_0 GBufferPS(false, false);
        
        ZEnable = true;
        ZWriteEnable = true;
		ZFunc = LESSEQUAL;
        CullMode = CCW;
        FillMode = Solid;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
		StencilEnable = true;
		StencilRef = 1;
		StencilFunc = ALWAYS;
		StencilPass = REPLACE;
		StencilZFail = KEEP;
		SRGBWriteEnable = false;
    }
}

// Opaques, with normal mapping
technique GBufferNM
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 GBufferVS();
        PixelShader = compile ps_3_0 GBufferPS(true, false);
        
        ZEnable = true;
        ZWriteEnable = true;
		ZFunc = LESSEQUAL;
        CullMode = CCW;
        FillMode = Solid;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
		StencilEnable = true;
		StencilRef = 1;
		StencilFunc = ALWAYS;
		StencilPass = REPLACE;
		StencilZFail = KEEP;
		SRGBWriteEnable = false;
    }
}

// Stippled transparents, no normal mapping
technique GBufferMask
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 GBufferVS();
        PixelShader = compile ps_3_0 GBufferPS(false, true);
        
        ZEnable = true;
        ZWriteEnable = true;
		ZFunc = LESSEQUAL;
        CullMode = CCW;
        FillMode = Solid;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
		StencilEnable = true;
		StencilRef = 1;
		StencilFunc = ALWAYS;
		StencilPass = REPLACE;
		StencilZFail = KEEP;
		SRGBWriteEnable = false;
    }
}

// Stippled transparents, with normal mapping
technique GBufferMaskNM
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 GBufferVS();
        PixelShader = compile ps_3_0 GBufferPS(true, true);
        
        ZEnable = true;
        ZWriteEnable = true;
		ZFunc = LESSEQUAL;
        CullMode = CCW;
        FillMode = Solid;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
		StencilEnable = true;
		StencilRef = 1;
		StencilFunc = ALWAYS;
		StencilPass = REPLACE;
		StencilZFail = KEEP;
		SRGBWriteEnable = false;
    }
}



