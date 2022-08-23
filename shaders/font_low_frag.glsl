uniform sampler2D texSampler;
uniform float4 fontColor;
uniform float enable_shadow;
uniform float enable_outline;

uniform float2 texSize;
uniform float2 screenSize;

static float neighborAlphaFactor = 0.5;

float4 outline(float4 _pixelColor, sampler2D _texSampler, float2 _texCoord, float4 _outlineColor, float2 _pxUnit) {
	float sumNeighbors = 0.0;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2(-_pxUnit.x,	 _pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( 0,			 _pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( _pxUnit.x,	 _pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2(-_pxUnit.x,	 0)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( _pxUnit.x,	 0)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2(-_pxUnit.x,	-_pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( 0,			-_pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( _pxUnit.x,	-_pxUnit.y)).r;
	float4 color = lerp(_outlineColor, _pixelColor, _pixelColor.a);
	color.a = sumNeighbors * neighborAlphaFactor;
	return color;
}

float4 RGBA_BGRA(in float4 rgba)
{
	return float4(rgba.bgr, rgba.a);
}

float4 main(
	float4 v_color : COLOR,
	float2 v_texCoord : TEXCOORD0
) {
	float4 rgbaTex = tex2D(texSampler, v_texCoord);

	if(enable_shadow == 1.0)
	{
		const float3 shadowColor = float3(0, 0, 0);
		const float2 shadowOffset = float2(-0.75, -0.75);
		float shadowAlpha = tex2D(texSampler, v_texCoord + shadowOffset / texSize).a;
		rgbaTex = lerp(float4(shadowColor, shadowAlpha), rgbaTex, rgbaTex.a * 2.0);
	}
	rgbaTex = clamp(rgbaTex, float4(0.0, 0.0, 0.0, 0.0), float4(1.0, 1.0, 1.0, 1.0));
	rgbaTex *= v_color;
	rgbaTex *= fontColor;

	if(enable_outline == 1.0)
	{
		rgbaTex = outline(rgbaTex, texSampler, v_texCoord, float4(0.0, 0.0, 0.0, 1.0), float2(1.0,1.0)/texSize);
	}

	rgbaTex = RGBA_BGRA(rgbaTex);

	return rgbaTex;
}

