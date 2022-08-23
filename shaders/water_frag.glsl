uniform sampler2D bgTexSampler;
uniform sampler2D wavesTexSampler;

uniform float4 mapParams;		//x=map_origin.x, y=map_origin.y, z=baseX, w=baseY
uniform float4 waterParams;	//x=counter, y=direction(0/1)
uniform float2 bgTexSize;
uniform float2 wavesTexSize;

float4 main(
	float2 v_texCoord : TEXCOORD0,
	float2 v_vertexCoord : TEXCOORD1
) {
	float2 tmp = float2(mapParams.z, mapParams.w); //baseX base on map_origin.x + counter, same for baseY
	tmp += v_texCoord;
	float2 bgTexCoord = tmp/bgTexSize;

	float wavMvt = fmod(2.0*waterParams.x, 180.0);
	wavMvt = sin(radians(wavMvt))*0.15;
	float2 wavesTexCoord = (tmp/wavesTexSize) + float2(wavMvt, wavMvt);

	float4 bgCol = tex2D(bgTexSampler, bgTexCoord*float2(2.0,2.0));
	float4 wavesCol = tex2D(wavesTexSampler, wavesTexCoord*float2(2.0,2.0));
	float4 comb = bgCol * 0.3 + wavesCol * 0.6;
	return float4(comb.b, comb.g, comb.r, 1);
}
