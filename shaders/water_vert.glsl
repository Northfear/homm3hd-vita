void main(
	float2 a_vertex,
	float2 a_texCoord,
	float4 out gl_Position : POSITION,
	float2 out v_texCoord : TEXCOORD0,
	float2 out v_vertexCoord : TEXCOORD1
) {
	gl_Position = float4(a_vertex, 0.0f, 1.0f);
	v_vertexCoord = a_vertex;
	v_texCoord = a_texCoord;
}
