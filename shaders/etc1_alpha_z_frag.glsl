uniform sampler2D u_s2dTexture;
uniform sampler2D u_s2dAlpha;

float4 main(
	float4 v_color : COLOR,
	float2 v_texCoord : TEXCOORD0
) {
	float4 v4Colour = tex2D(u_s2dTexture, v_texCoord);
	v4Colour.a = tex2D(u_s2dAlpha, v_texCoord).r;
	if( v4Colour.a <= 0.5 ){
     		discard;
	}	
	return v4Colour * v_color;
}

