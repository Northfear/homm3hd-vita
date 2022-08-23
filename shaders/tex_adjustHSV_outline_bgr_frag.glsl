uniform sampler2D texSampler;
uniform float hue;
uniform float hue_adjust;
uniform float saturation_adjust;
uniform float value_adjust;
uniform float4 outlineColor;
uniform float2 texSize;

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

float3 RGBToHSV(float3 c)
{
	float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
	float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));
	
	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float3 HSVToRGB(float3 c)
{
	float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float4 AdjustHSV(float4 rgba, float _hue, float _hue_adjust, float _saturation_adjust, float _value_adjust)
{
	float3 hsv = RGBToHSV( rgba.rgb );

	// Modify hue
	if ( hue_adjust >= 0.0 )
	{
		float difference = hue - hsv.x;

		hsv.x += hue_adjust * ( difference );

		if ( abs( difference ) > 0.5 )
		{
			if ( difference > 0.0 )
				hsv.x += 1.0 - hue_adjust;
			else
				hsv.x +=  hue_adjust;

			if ( hsv.x >= 1.0 )
				hsv.x -= 1.0;
		}
	}

	// Modify saturation
	if ( saturation_adjust >= 0.0 )
	{
		if ( saturation_adjust <= 1.0 )
			hsv.y *= saturation_adjust;
		else
			hsv.y = 1.0 - ( 1.0 - hsv.y ) / saturation_adjust;
	}

	// Modify value
	if ( value_adjust >= 0.0 )
	{
		if ( value_adjust <= 1.0 )
			hsv.z *= value_adjust;
		else
			hsv.z = 1.0 - ( 1.0 - hsv.z ) / value_adjust;
	}

	// Convert to RGB
	float3 rgb = HSVToRGB(hsv);
	
	return float4(rgb, rgba.a);
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
	
	rgbaTex *= v_color;
	
	rgbaTex = AdjustHSV(rgbaTex, hue, hue_adjust, saturation_adjust, value_adjust);
	
	rgbaTex = outline(rgbaTex, texSampler, v_texCoord, outlineColor, float2(1.0,1.0)/texSize);
	
	rgbaTex = RGBA_BGRA(rgbaTex);
	
	return rgbaTex;
}

