uniform sampler2D u_s2dTexture;
uniform sampler2D u_s2dAlpha;

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

float4 RGBA_BGRA(float4 rgba)
{
	return float4(rgba.bgr, rgba.a);
}

float4 main(
	float4 v_color : COLOR,
	float2 v_texCoord : TEXCOORD0
) {
	float4 v4Color = tex2D(u_s2dTexture, v_texCoord);
	v4Color.a = tex2D(u_s2dAlpha, v_texCoord).r;
	
	float4 rgbaTex = v4Color * v_color;
	
	rgbaTex = outline(rgbaTex, u_s2dAlpha, v_texCoord, outlineColor, float2(1.0,1.0)/texSize);
	
	rgbaTex = RGBA_BGRA(rgbaTex);
	
	return rgbaTex;
}

