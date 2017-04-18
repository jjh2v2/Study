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
float4x4 ViewToLight;

float  LightRange;
float3 LightColor;
float3 LightPosVS;
float3 LightDirVS;

float FarClip;

float2 GBufferDimensions;
float2 ShadowMapDimensions;
float3 FrustumCornersVS[4];

texture2D DepthIDTexture;
sampler2D DepthIDSampler = sampler_state
{
    Texture = <DepthIDTexture>;
    MinFilter = point;
    MagFilter = point;
    MipFilter = point;
    MaxAnisotropy = 1;
	SRGBTexture = false;
};

texture2D NormalSpecTexture;
sampler2D NormalSpecSampler = sampler_state
{
    Texture = <NormalSpecTexture>;
    MinFilter = point;
    MagFilter = point;
    MipFilter = point;
    MaxAnisotropy = 1;
	SRGBTexture = false;
};

texture2D ShadowMapTexture;
sampler2D ShadowMapSampler = sampler_state
{
    Texture = <ShadowMapTexture>;
    MinFilter = point;
    MagFilter = point;
    MipFilter = point;
    MaxAnisotropy = 1;
	SRGBTexture = false;
};


struct VSInput
{
	float4 PositionOS			: POSITION0;
};

struct VSOutput
{
	float4 PositionCS			: POSITION0;
	float3 PositionVS			: TEXCOORD0;
};


VSOutput PointLightVS(in VSInput input)
{
	VSOutput output;

	// Figure out the position of the vertex in clip space, and in view space
    output.PositionCS = mul(input.PositionOS, WorldViewProjection);    
    output.PositionVS = mul(input.PositionOS, WorldView);
	
    return output;
}

float4 PointLightPS(	in float3 VertexPositionVS : TEXCOORD0,
						in float2 ScreenPos : VPOS	)	: COLOR0
{	
	float2 texCoord = TexCoordFromVPOS(ScreenPos, GBufferDimensions);

	// Reconstruct view-space position from the depth buffer
    float3 frustumRayVS = VertexPositionVS.xyz * (FarClip/-VertexPositionVS.z);
	float3 pixelPositionVS = PositionFromDepth(DepthIDSampler, texCoord, frustumRayVS);
	
	// Get normals and specular exponent from the G-Buffer
	float3 normalVS;
	float specExponent; 
	UnpackNormalsAndSpecular(NormalSpecSampler, texCoord, normalVS, specExponent);
		
	float3 lightDirVS = LightPosVS - pixelPositionVS;
	float lightDist = length(lightDirVS);
	lightDirVS = normalize(lightDirVS);	
	float attenuation = max(0, (LightRange -lightDist) / LightRange);
	
	// Calculate lighting terms
	float3 V = -normalize(pixelPositionVS);
	float3 H = normalize(V + lightDirVS);	
    float NdotL = saturate(dot(normalVS, lightDirVS)) * attenuation;
    float NdotH = saturate(dot(normalVS, H));	
    float specular = pow(NdotH, specExponent)* ((specExponent + 8.0f) / (8.0f * 3.14159265f));

	return float4(	LightColor.x * NdotL,
					LightColor.y * NdotL,
					LightColor.z * NdotL,
					specular * NdotL);
}

void DirectionalLightVS (	in float3 PositionOS				: POSITION,
							in float3 TexCoordAndCornerIndex	: TEXCOORD0,					
							out float4 PositionCS				: POSITION,
							out float2 TexCoord					: TEXCOORD0,
							out float3 FrustumRayVS				: TEXCOORD1	)
{
	// Offset the position by half a pixel to correctly align texels to pixels
	PositionCS.x = PositionOS.x - (1.0f / GBufferDimensions.x);
	PositionCS.y = PositionOS.y + (1.0f / GBufferDimensions.y);
	PositionCS.z = PositionOS.z;
	PositionCS.w = 1.0f;
	
	// Pass along the texture coordinate and the position of the frustum corner
	TexCoord = TexCoordAndCornerIndex.xy;
	FrustumRayVS = FrustumCornersVS[TexCoordAndCornerIndex.z];
}	

float4 DirectionalLightPS(	in float2 TexCoord			: TEXCOORD0,
							in float3 FrustumRayVS	: TEXCOORD1)	: COLOR0
{	
	// Reconstruct view-space position from the depth buffer
	float3 pixelPositionVS = PositionFromDepth(DepthIDSampler, TexCoord, FrustumRayVS);
	
	// Get normals and specular exponent from the G-Buffer
	float3 normalVS;
	float specExponent; 
	UnpackNormalsAndSpecular(NormalSpecSampler, TexCoord, normalVS, specExponent);
	
	// Calculate lighting terms
	float3 V = -normalize(pixelPositionVS);
	float3 H = normalize(V - LightDirVS);
    float NdotL = saturate(dot(normalVS, -LightDirVS));
	float NdotH = saturate(dot(normalVS, H));
    float specular = pow(NdotH, specExponent)* ((specExponent + 8.0f) / (8.0f * 3.14159265f));

	// Apply shadow mapping
	float4 pixelPosLS = mul(float4(pixelPositionVS, 1.0f), ViewToLight);
	float shadowOcclusion = GetShadowTerm(pixelPosLS, ShadowMapSampler, ShadowMapDimensions);
	NdotL *= shadowOcclusion;	
		
	return float4(	LightColor.x * NdotL,
					LightColor.y * NdotL,
					LightColor.z * NdotL,
					specular * NdotL);
}

// Point light with bounding volume, drawing back-faces only
technique PointLightBack
{
    pass p0
    {
		ZEnable = true;
		ZWriteEnable = false;
		ZFunc = GREATEREQUAL;
		AlphaBlendEnable = true;
		SrcBlend = ONE;
		DestBlend = ONE;
		CullMode = CW;
		AlphaTestEnable = false;
		StencilEnable = true;
		StencilFunc = EQUAL;
		StencilRef = 1;	
		StencilPass = REPLACE;
		StencilZFail = KEEP;
		SRGBWriteEnable = false;
    
        VertexShader = compile vs_3_0 PointLightVS();
        PixelShader = compile ps_3_0 PointLightPS();
    }
}

// Point light with bounding volume, drawing front-faces only
technique PointLightFront
{
    pass p0
    {
		ZEnable = true;
		ZWriteEnable = false;
		ZFunc = LESSEQUAL;
		AlphaBlendEnable = true;
		SrcBlend = ONE;
		DestBlend = ONE;
		CullMode = CCW;
		AlphaTestEnable = false;
		StencilEnable = true;
		StencilFunc = EQUAL;
		StencilRef = 1;	
		StencilPass = REPLACE;
		StencilZFail = KEEP;
		SRGBWriteEnable = false;
    
        VertexShader = compile vs_3_0 PointLightVS();
        PixelShader = compile ps_3_0 PointLightPS();
    }
}

// Directional light with a full-screen quad
technique DirectionalLight
{
    pass p0
    {
		ZEnable = false;
		ZWriteEnable = false;
		ZFunc = LESSEQUAL;
		AlphaBlendEnable = true;
		SrcBlend = ONE;
		DestBlend = ONE;
		CullMode = NONE;
		AlphaTestEnable = false;
		StencilEnable = true;
		StencilFunc = EQUAL;
		StencilRef = 1;	
		StencilPass = REPLACE;
		StencilZFail = KEEP;
		SRGBWriteEnable = false;
    
        VertexShader = compile vs_3_0 DirectionalLightVS();
        PixelShader = compile ps_3_0 DirectionalLightPS();
    }
}



