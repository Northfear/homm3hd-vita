void main(
	float2 a_vertex,
	float2 a_texCoord,
	float4 a_color,
	float4 out gl_Position : POSITION,
	float2 out v_v2TexCoord : TEXCOORD0,
	float4 out v_color : COLOR
) {
	gl_Position = float4(a_vertex, 0.0f, 1.0f);
	v_color = a_color;
	v_v2TexCoord = a_texCoord;
}

