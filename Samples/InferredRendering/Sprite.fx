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

texture2D SpriteTexture;
sampler2D SpriteSampler = sampler_state
{
    Texture = <SpriteTexture>;
    MinFilter = point;
    MagFilter = point;
    MipFilter = point;
    MaxAnisotropy = 1;
	SRGBTexture = true;
};

float4 SpritePS(float4 colorMask : COLOR0, float2 texCoord : TEXCOORD0) : COLOR0
{
    float4 color = tex2D(SpriteSampler, texCoord) * colorMask;
	return color;
}

// Used with SpriteBatch...does everything normally except applies gamma-correction
technique Sprite
{
    pass Pass1
    {
        PixelShader = compile ps_2_0 SpritePS();

		SRGBWriteEnable = true;
    }
}
