//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject
{
	float4x4 gWorldViewProj; 
};

struct VertexIn
{
	float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

struct PSOutput
{
	float4 Color : SV_Target0;
	float4 Normal0 : SV_Target1;
	float4 Normal1 : SV_Target2;
	float4 Normal2 : SV_Target3;
};

PSOutput PS(VertexOut pin)
{
	PSOutput output;
	output.Color = pin.Color * 4;
	output.Normal0 = pin.Color * 1;
	output.Normal1 = pin.Color * 2;
	output.Normal2 = pin.Color * 3;

	return output;
}

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
