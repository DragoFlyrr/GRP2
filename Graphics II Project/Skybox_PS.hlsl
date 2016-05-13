struct INPUT_PIXEL	
{
	float3 uv : TEXCOORD;
	float4 color : COLOR;
	float4 projectedCoordinate : SV_POSITION;
};

TextureCube baseTexture : register(t0);

sampler cubeSampler = sampler_state {
	texture = <tex>;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

float4 main( INPUT_PIXEL input ) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(cubeSampler, input.uv);
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