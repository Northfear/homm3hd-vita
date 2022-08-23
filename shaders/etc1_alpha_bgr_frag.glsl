uniform sampler2D u_s2dTexture;
uniform sampler2D u_s2dAlpha;

float4 main(
	float4 v_color : COLOR,
	float2 v_v2TexCoord : TEXCOORD0
) {
	float4 v4Colour = tex2D(u_s2dTexture, v_v2TexCoord).bgra;
	v4Colour.a = tex2D(u_s2dAlpha, v_v2TexCoord).r;
	return v4Colour * v_color;
}

