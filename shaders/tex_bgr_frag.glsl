uniform sampler2D texSampler;

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
	
	rgbaTex = RGBA_BGRA(rgbaTex);
	
	return rgbaTex;
}

