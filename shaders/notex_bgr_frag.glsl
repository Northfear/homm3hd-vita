float4 RGBA_BGRA(float4 rgba)
{
	return float4(rgba.bgr, rgba.a);
}

float4 main(
	float4 v_color : COLOR
) {
	float4 rgba = RGBA_BGRA(v_color);
	return rgba;
}

