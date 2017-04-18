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

float3 SpecularAlbedo;
float3 AmbientColor;
float2 GBufferDimensions;
float2 RTDimensions;
float InstanceID;
float FarClip;
float2 TexScale = {1.0f, 1.0f};
float Alpha = 0.5f;
float4 XFilterOffsets;
float4 YFilterOffsets;

texture2D LightTexture;
sampler2D LightSampler = sampler_state
{
    Texture = <LightTexture>;
    MinFilter = point;
    MagFilter = point;
    MipFilter = point;
    MaxAnisotropy = 1;
	SRGBTexture = false;
};

texture2D DiffuseMap;
sampler2D DiffuseSampler = sampler_state
{
    Texture = <DiffuseMap>;
    MinFilter = anisotropic;
    MagFilter = linear;
    MipFilter = linear;
    MaxAnisotropy = 16;
    AddressU = Wrap;
    AddressV = Wrap;
	SRGBTexture = true;
};

texture2D	DepthIDTexture;
sampler2D	DepthIDSampler = sampler_state
{
    Texture = <DepthIDTexture>;
    MinFilter = point;
    MagFilter = point;
    MipFilter = point;
    MaxAnisotropy = 1;
	SRGBTexture = false;
};

struct VSInput 
{
	float4 PositionOS	: POSITION0;
	float2 TexCoord		: TEXCOORD0;
};

struct VSOutput 
{
	float4 PositionCS	: POSITION0;
	float2 TexCoord		: TEXCOORD0;
	float DepthVS		: TEXCOORD1;
};

VSOutput CompositeVS(in VSInput input)
{
	VSOutput output;
	output.PositionCS = mul(input.PositionOS, WorldViewProjection);
	output.TexCoord = input.TexCoord * TexScale;
	output.DepthVS = -mul(input.PositionOS, WorldView).z;
	return output;
}

float4 CompositePS(in float2 TexCoord	: TEXCOORD0, 
				   in float DepthVS		: TEXCOORD1,
				   in float2 ScreenPos	: VPOS,
				   uniform bool Filter,
				   uniform bool Transparent)	: COLOR0
{	    
	float2 screenTexCoord = TexCoordFromVPOS(ScreenPos, RTDimensions);

	// Get our diffuse albedo
	float3 diffuseAlbedo = tex2D(DiffuseSampler, TexCoord).rgb;
	
	// Start adding up the color
	float3 color = 0;
	
	// Ambient light
	color += AmbientColor * diffuseAlbedo;
	
    // Sample lights from the L-Buffer
	float normalizedDepthVS = DepthVS / FarClip;
	float4 lightValue;
	if (Filter)
	{
		float2 offset = 0;
		float2 filterScale = float2(1.0f, 1.0f);
		if (Transparent)
		{
			// For transparents we adjust our filtering so that we grab
			// the nearest 4 samples according to the output mask we used
			float2 gBufferPos = screenTexCoord * GBufferDimensions;
			float2 pos = floor(fmod(gBufferPos, 2));
			float index = pos.x + pos.y * 2;

			offset.x = XFilterOffsets[index];
			offset.y = YFilterOffsets[index];
			filterScale = float2(2.0f, 2.0f);
		}

		float2 texScale = GBufferDimensions / RTDimensions;
		lightValue = SampleDSF(LightSampler, DepthIDSampler, screenTexCoord, 
								GBufferDimensions, InstanceID, normalizedDepthVS, 
								texScale, offset, filterScale);		
	}
	else
		lightValue = tex2D(LightSampler, screenTexCoord);
	
	// Apply our albedos to diffuse and specular
	color += (lightValue.xyz * diffuseAlbedo) + (lightValue.www * SpecularAlbedo);
    
	return float4(color * Alpha, Alpha);
}

// Renders opaques with no filtering (standard Light Prepass method)
technique CompositeOpaque
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 CompositeVS();
        PixelShader = compile ps_3_0 CompositePS(false, false);
        
        ZEnable = true;
        ZWriteEnable = true;
		ZFunc = LESSEQUAL;
        CullMode = CCW;
        FillMode = Solid;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
		StencilEnable = false;
		SRGBWriteEnable = true;
    }
}

// Renders opaques with filtering, used when deferred transparency is being used
// or when using a downsized G-Buffer + L-Buffer
technique CompositeOpaqueFiltered
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 CompositeVS();
        PixelShader = compile ps_3_0 CompositePS(true, false);
        
        ZEnable = true;
        ZWriteEnable = true;
		ZFunc = LESSEQUAL;
        CullMode = CCW;
        FillMode = Solid;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
		StencilEnable = false;
		SRGBWriteEnable = true;
    }
}

// Renders deferred transparents with adjusted filtering
technique CompositeTransparent
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 CompositeVS();
        PixelShader = compile ps_3_0 CompositePS(true, true);
        
        ZEnable = true;
        ZWriteEnable = false;
		ZFunc = LESSEQUAL;
        CullMode = CCW;
        FillMode = Solid;
        AlphaBlendEnable = true;
		SrcBlend = ONE;
		DestBlend = INVSRCALPHA;
        AlphaTestEnable = true;
		StencilEnable = false;
		SRGBWriteEnable = true;
    }
}