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

	float screenHRatio = clamp((1.0-(screenSize.y-600.0)/600.0), 0.0, 1.0); //screenHRatio == 1 when height == 600, 0 when 1200 or higher
	float stepMin = 0.15 * screenHRatio;
	float stepMax = 1.0 - 0.84 * screenHRatio;
	
	//rgbaTex.a = step(stepMax, rgbaTex.a);
	rgbaTex.a = smoothstep(stepMin, stepMax, rgbaTex.a);

	if(enable_shadow == 1.0)
	{
		const float3 shadowColor = float3(0, 0, 0);
		const float2 shadowOffset1 = float2(-1, -1);
		const float2 shadowOffset2 = float2(-2, -2);
		float shadowAlpha = tex2D(texSampler, v_texCoord+shadowOffset1/texSize).a;
		rgbaTex = lerp(float4(shadowColor, shadowAlpha), rgbaTex, rgbaTex.a*2.0);
		shadowAlpha = tex2D(texSampler, v_texCoord+shadowOffset2/texSize).a;
		rgbaTex = lerp(float4(shadowColor, shadowAlpha), rgbaTex, rgbaTex.a*2.0);
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

