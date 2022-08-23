uniform sampler2D texSampler;

float4 main(
	float4 v_color : COLOR,
	float2 v_texCoord : TEXCOORD0
) {
    float4 rgbaTex = tex2D(texSampler, v_texCoord);
    
    rgbaTex *= v_color;
   
	if( rgbaTex.a <= 0.5 ){
     		discard;
	}   
   
    return rgbaTex;
}

