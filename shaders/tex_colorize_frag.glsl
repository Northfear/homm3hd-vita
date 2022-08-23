uniform sampler2D texSampler;
uniform float2 HS;

/*vec4 colorize(in vec4 rgba, in vec2 _HS)
{
	vec3 pixelRGB = rgba.rgb;
	float v = max(max(pixelRGB.r, pixelRGB.g), pixelRGB.b);
	float f = mod( _HS.x *6.0, 1.0 );
	float p = v * ( 1.0 - _HS.y );
	float q = v * ( 1.0 - _HS.y * f );
	float t = v * ( 1.0 - _HS.y * ( 1.0 - f ) );
	
	int hi = int(_HS.x * 6.0);
	if (hi == 0) {
		pixelRGB = vec3(v, t, p);
	} else if (hi == 1) {
		pixelRGB = vec3(q, v, p);
	} else if (hi == 2) {
		pixelRGB = vec3(p, v, t);
	} if(hi == 3) {
		pixelRGB = vec3(p, q, v);
	} else if (hi == 4) {
		pixelRGB = vec3(t, p, v);
	} else {
		pixelRGB = vec3(v, p, q);
	}
	
	return vec4(pixelRGB, rgba.a);
}*/

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

float4 setHS(float4 rgba, float2 _HS)
{
	float3 hsv = RGBToHSV( rgba.rgb );

	hsv.x = _HS.x;
	hsv.y = _HS.y;
	
	// Convert to RGB
	float3 rgb = HSVToRGB(hsv);
	
	return float4(rgb, rgba.a);
}

float4 main(
	float4 v_color : COLOR,
	float2 v_texCoord : TEXCOORD0
) {
	float4 rgbaTex = tex2D(texSampler, v_texCoord);
	
	rgbaTex *= v_color;
	
	rgbaTex = setHS(rgbaTex, HS);
	
	return rgbaTex;
}
