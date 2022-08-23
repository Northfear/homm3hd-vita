uniform sampler2D texSampler;
uniform float4 outlineColor;
uniform float2 texSize;

static float neighborAlphaFactor = 0.5;

float4 outline(float4 _pixelColor, sampler2D _texSampler, float2 _texCoord, float4 _outlineColor, float2 _pxUnit) {
	float sumNeighbors = 0.0;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2(-_pxUnit.x,	 _pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( 0,			 _pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( _pxUnit.x,	 _pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2(-_pxUnit.x,	 0)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( _pxUnit.x,	 0)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2(-_pxUnit.x,	-_pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( 0,			-_pxUnit.y)).r;
	sumNeighbors += tex2D(_texSampler, _texCoord + float2( _pxUnit.x,	-_pxUnit.y)).r;
	float4 color = lerp(_outlineColor, _pixelColor, _pixelColor.a);
	color.a = sumNeighbors * neighborAlphaFactor;
	return color;
}

float4 main(
	float4 v_color : COLOR,
	float2 v_texCoord : TEXCOORD0
) {
	float4 rgbaTex = tex2D(texSampler, v_texCoord);
	
	rgbaTex *= v_color;
	
	rgbaTex = outline(rgbaTex, texSampler, v_texCoord, outlineColor, float2(1.0,1.0)/texSize);
	
	return rgbaTex;
}
