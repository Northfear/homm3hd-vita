uniform sampler2D texSampler;
uniform float2 rt_sz; // render target size
uniform float2 pixel_sz; // pixel size

float4 main(
	float4 v_color : COLOR,
	float2 v_texCoord : TEXCOORD0
) {
	float2 d = pixel_sz*(float2(1.0, 1.0)/rt_sz);
    float2 coord = float2(d.x*(floor(v_texCoord.x/d.x)+d.x/2.0),
                      d.y*(floor(v_texCoord.y/d.y)+d.y/2.0));
	float4 rgbaTex = tex2D(texSampler, coord);

	rgbaTex *= v_color;
	
	return rgbaTex;
}
