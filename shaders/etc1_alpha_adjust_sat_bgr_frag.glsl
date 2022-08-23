uniform sampler2D u_s2dTexture;
uniform sampler2D u_s2dAlpha;

uniform float sat_amount;

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

float4 AdjustSat(float4 rgba, float _sat_amount)
{
	float3 hsv = RGBToHSV( rgba.rgb );

	// Modify saturation
	if ( _sat_amount <= 1.0 )
		hsv.y *= _sat_amount;
	else
		hsv.y = 1.0 - ( 1.0 - hsv.y ) / _sat_amount;

	// Convert to RGB
	float3 rgb = HSVToRGB(hsv);
	
	return float4(rgb, rgba.a);
}

float4 RGBA_BGRA(float4 rgba)
{
	return float4(rgba.bgr, rgba.a);
}

float4 main(
	float4 v_color : COLOR,
	float2 v_texCoord : TEXCOORD0
) {
	float4 v4Color = tex2D(u_s2dTexture, v_texCoord);
	v4Color.a = tex2D(u_s2dAlpha, v_texCoord).r;
	
	float4 rgbaTex = v4Color * v_color;
	
	rgbaTex = AdjustSat(rgbaTex, sat_amount);
	
	rgbaTex = RGBA_BGRA(rgbaTex);
	
	return rgbaTex;
}

