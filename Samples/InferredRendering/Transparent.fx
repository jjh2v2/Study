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

static const int NumLights = 4;

float4x4 WorldView;
float4x4 WorldViewProjection;
float4x4 ViewToLight;

float3 SpecularAlbedo;
float SpecularExponent;
float2 TexScale = {1.0f, 1.0f};
float Alpha = 0.5f;

float3 AmbientColor;
float  LightRanges[NumLights];
float3 LightColors[NumLights];
float3 LightPositionsVS[NumLights];

float3 LightDirVS;
float3 LightColor;

bool EnablePointLights : register(b0);

float2 ShadowMapDimensions;

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

texture2D DiffuseMap;
sampler2D DiffuseSampler = sampler_state
{
    Texture = <DiffuseMap>;
    MinFilter = ANISOTROPIC;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    MaxAnisotropy = 16;
    AddressU = Wrap;
    AddressV = Wrap;
	SRGBTexture = true;
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
	float4 PositionOS		: POSITION0;
	float3 NormalOS			: NORMAL0;
	float3 TangentOS		: TANGENT;
	float3 BinormalOS		: BINORMAL;
	float2 TexCoord			: TEXCOORD0;	
};

struct VSOutput
{
	float4 PositionCS		: POSITION0;
	float3 PositionVS		: TEXCOORD0;
	float3 NormalVS			: TEXCOORD1;
	float3 TangentVS		: TEXCOORD2;
	float3 BinormalVS		: TEXCOORD3;
	float2 TexCoord			: TEXCOORD4;
	float4 PositionLS		: TEXCOORD5;
};

VSOutput TransparentVS(in VSInput input)
{
	VSOutput output;
	output.PositionCS = mul(input.PositionOS, WorldViewProjection);
	output.PositionVS = mul(input.PositionOS, WorldView).xyz;
	output.PositionLS = mul(float4(output.PositionVS, 1.0f), ViewToLight);
	output.TexCoord = input.TexCoord * TexScale;

	// Transform the tangent basis to view space, so we can
	// transform the normal map normals to view space
	output.NormalVS = mul(input.NormalOS, (float3x3)WorldView);
	output.TangentVS = mul(input.TangentOS, (float3x3)WorldView);
	output.BinormalVS = mul(input.BinormalOS, (float3x3)WorldView);

	return output;
}

float4 TransparentPS(in VSOutput input, uniform bool UseNormalMap) : COLOR0
{    
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

	float3 diffuseAlbedo = tex2D(DiffuseSampler, input.TexCoord);
	float3 ambient = AmbientColor * diffuseAlbedo;
	float3 diffuse = 0;
	float3 specular = 0;

	if (EnablePointLights)
	{
		// Sum the point lights
		for (int i = 0; i < NumLights; i++)
		{
			float3 lightDirVS = LightPositionsVS[i] - input.PositionVS;
			float lightDist = length(lightDirVS);
			lightDirVS = normalize(lightDirVS);	
			float attenuation = max(0, (LightRanges[i] -lightDist) / LightRanges[i]);
			
			// Calculate lighting terms
			float3 V = -normalize(input.PositionVS);
			float3 H = normalize(V + lightDirVS);	 
			float NdotL = saturate(dot(normalVS, lightDirVS)) * attenuation;
			float NdotH = saturate(dot(normalVS, H));	
			
			diffuse += NdotL * LightColors[i];			
			specular += pow(NdotH, SpecularExponent)* ((SpecularExponent + 8.0f) / (8.0f * 3.14159265f)) * NdotL;
		}
	}

	// Add in the global directional light
	float3 V = -normalize(input.PositionVS);
	float3 H = normalize(V - LightDirVS);    
	float NdotL = saturate(dot(normalVS, -LightDirVS));
	float NdotH = saturate(dot(normalVS, H));
	float shadowOcclusion = GetShadowTerm(input.PositionLS, ShadowMapSampler, ShadowMapDimensions);
	diffuse += NdotL * LightColor * shadowOcclusion;
	specular += pow(NdotH, SpecularExponent)* ((SpecularExponent + 8.0f) / (8.0f * 3.14159265f)) * NdotL * shadowOcclusion;

	float3 color = saturate(ambient + diffuse * diffuseAlbedo + specular * SpecularAlbedo);

	return float4(color * Alpha, Alpha);
}

// Forward-rendered transparents with pre-multiplied alpha
technique Transparent
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 TransparentVS();
        PixelShader = compile ps_3_0 TransparentPS(false);
        
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

// Forward-rendered transparents with pre-multiplied alpha and normal mapping
technique TransparentNM
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 TransparentVS();
        PixelShader = compile ps_3_0 TransparentPS(true);
        
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