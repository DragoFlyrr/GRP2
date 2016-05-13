struct INPUT_PIXEL	
{
	float2 uv : TEXCOORD;
	float4 color : COLOR;
	float4 projectedCoordinate : SV_POSITION;
};

texture2D baseTexture : register(t0);


SamplerState filters[2] : register(s0);

float4 main( INPUT_PIXEL input ) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filters[0], input.uv);
	float4 newColor;
	newColor.a = baseColor.b;
	newColor.r = baseColor.g;
	newColor.g = baseColor.r;
	newColor.b = baseColor.a;

	return newColor;
}
//float4 main(INPUT_PIXEL input) : SV_TARGET
//{
//	return float4(input.uvout, 1);
//}