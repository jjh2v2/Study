// Converts a normalized cartesian direction vector 
// to spherical coordinates.
float2 CartesianToSpherical(float3 cartesian)
{
	float2 spherical;
	
	spherical.x = atan2(cartesian.y, cartesian.x) / 3.14159f;
	spherical.y = cartesian.z;
	
	return spherical * 0.5f + 0.5f;
}

// Converts a spherical coordinate to a normalized
// cartesian direction vector.
float3 SphericalToCartesian(float2 spherical)
{
	float2 sinCosTheta, sinCosPhi;
	
	spherical = spherical * 2.0f - 1.0f;
	sincos(spherical.x * 3.14159f, sinCosTheta.x, sinCosTheta.y);
	sinCosPhi = float2(sqrt(1.0 - spherical.y * spherical.y), spherical.y);
	
	return float3(sinCosTheta.y * sinCosPhi.x, sinCosTheta.x * sinCosPhi.x, sinCosPhi.y);	
}

// Lambert Azimuthal Equal-Area projection
// See http://aras-p.info/texts/CompactNormalStorage.html#method6-lambert
float2 CartesianToLambertAzimuthal(float3 cartesian)
{
	float f = sqrt(8 * cartesian.z + 8);
	return cartesian.xy / f + 0.5;
}

float3 LambertAzimuthalToCartesian(float2 la)
{
	float2 fenc = la * 4 - 2;
	float f = dot(fenc , fenc);
	float g = sqrt(1 - f / 4);
	float3 n;
	n.xy = fenc * g;
	n.z = 1 - f / 2;
	return n;
}

float2 TexCoordFromVPOS (float2 VPOS, float2 sourceDimensions)
{
	return (VPOS + 0.5f) / sourceDimensions;
}

// Reconstruct position from a linear depth buffer
float3 PositionFromDepth(sampler2D depthSampler, float2 texCoord, float3 frustumRay)
{	
	float pixelDepth = tex2D(depthSampler, texCoord).x;
	return pixelDepth * frustumRay;
}

float4 PackNormalsAndSpecular(float3 normal, float specular)
{
	float4 output;
	output.xy = CartesianToLambertAzimuthal(normal);    
	output.z = specular / 512.0f;
    output.w = 1.0f;
    
    return output;
}

void UnpackNormalsAndSpecular(in sampler2D normalSpecSampler, in float2 texCoord, 
							  out float3 normals, out float specular)
{
	float4 packedNormalsAndSpec = tex2D(normalSpecSampler, texCoord);
	normals = LambertAzimuthalToCartesian(packedNormalsAndSpec.xy);
	specular = packedNormalsAndSpec.z * 512.0f;
}		

// Converts from linear RGB space to sRGB.
float3 LinearToSRGB(in float3 color)
{
	return pow(color, 1/2.2f);
}

// Converts from sRGB space to linear RGB.
float3 SRGBToLinear(in float3 color)
{
	return pow(color, 2.2f);
}

bool FuzzyEquals(in float a, in float b, in float epsilon)
{
	return abs(a - b) < epsilon;
}

float4 SampleDSF(in sampler2D samp, in sampler2D depthIDSamp, in float2 texCoord, 
				 in float2 texSize, in float instanceID, in float depthVS, 
				 in float2 texScale, in float2 offset, in float2 filterScale)
{
	// Scale out into pixel space
	float2 unormSamplePos = texSize * texCoord;

	// Determine the lerp amounts
	float2 lerps = (frac(unormSamplePos) - (0.5f * texScale) - offset) / filterScale ;
	float lerpAmount[3];
	lerpAmount[0] = lerps.x;
	lerpAmount[1] = lerps.x;
	lerpAmount[2] = lerps.y;

	// Get the upper left position
	float2 lerpPos = floor(unormSamplePos) + offset + 0.5f;
	lerpPos /= texSize;

	// Figure out our 4 sample points
	float2 samplePoints[4];
	samplePoints[0] = lerpPos;
	samplePoints[1] = lerpPos + float2(filterScale.x / texSize.x, 0);
	samplePoints[2] = lerpPos + float2(0, filterScale.y / texSize.y);
	samplePoints[3] = lerpPos + float2(filterScale.x / texSize.x, filterScale.y / texSize.y);

	// Take the 4 samples, and compute an additional weight for
	// each sample based on comparison with the DepthID buffer
	float4 samples[4];
	float weights[4];
	for (int i = 0; i < 4; i++)
	{
		samples[i] = tex2D(samp, samplePoints[i]);
		float2 depthID = tex2D(depthIDSamp, samplePoints[i]).xy;
		weights[i] = FuzzyEquals(depthID.y, instanceID, 0.01f)
						&& FuzzyEquals(depthID.x, depthVS, 0.1f);
	};

	// We'll bias our lerp weights based on our additional DepthID
	// weights.  This will filter out "bad" samples that are across
	// discontinuities .
	lerpAmount[0] = saturate(lerpAmount[0] - weights[0] + weights[1]);
	lerpAmount[1] = saturate(lerpAmount[1] - weights[2] + weights[3]);

	float topWeight = (weights[0] + weights[1]) * 0.5f;
	float bottomWeight = (weights[2] + weights[3]) * 0.5f;
	lerpAmount[2] = saturate(lerpAmount[2] - topWeight + bottomWeight);

	// Perform the bilinear filtering with our new weights
	return lerp(lerp(samples[0], samples[1], lerpAmount[0]),
				   lerp(samples[2], samples[3],lerpAmount[1]),
				   lerpAmount[2]);
}

float GetShadowTerm(in float4 pixelPosLS, in sampler2D shadowMapSampler, in float2 shadowMapDimensions)
{
	static const float Bias = 0.002f;
	float2 shadowTexCoord = pixelPosLS.xy * 0.5f + 0.5f;
	shadowTexCoord.y = 1.0f - shadowTexCoord.y;
	float pixelDepthLS = pixelPosLS.z / pixelPosLS.w - Bias;
	
	// 2x2 PCF
	float2 shadowMapCoord = shadowMapDimensions * shadowTexCoord;        
	float2 lerps = frac(shadowMapCoord);
	float2 samplePoints[4];
	samplePoints[0] = shadowTexCoord;
	samplePoints[1] = shadowTexCoord + float2(1.0f / shadowMapDimensions.x, 0);
	samplePoints[2] = shadowTexCoord + float2(0, 1.0f / shadowMapDimensions.y);
	samplePoints[3] = shadowTexCoord + float2(1.0f / shadowMapDimensions.x, 1.0f / shadowMapDimensions.y);
	
	float samples[4];
	for (int i = 0; i < 4; i++)
		samples[i] = tex2D(shadowMapSampler, samplePoints[i]).x >= pixelDepthLS;
	
	return lerp(lerp(samples[0], samples[1], lerps.x), lerp( samples[2], samples[3], lerps.x), lerps.y);
}

struct GBufferOutput
{
	float4 DepthID			: COLOR0;
	float4 NormalsSpecular	: COLOR1;
};