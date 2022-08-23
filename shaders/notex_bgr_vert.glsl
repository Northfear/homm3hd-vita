void main(
	float2 a_vertex,
	float4 a_color,
	float4 out gl_Position : POSITION,
	float4 out v_color : COLOR
) {
	gl_Position = float4(a_vertex, 0.0f, 1.0f);
	v_color = a_color;
}

