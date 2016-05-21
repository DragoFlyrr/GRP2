struct INPUT_PIXEL
{
	float2 uv : TEXCOORD;
	float4 color : COLOR;
	float3 normal : NORMAL;
	float3 worldcoord : POSITION;
	float4 projectedCoordinate : SV_POSITION;
	float4 CameraCoord : COORD;
};

texture2D baseTexture : register(t0);

SamplerState filters[2] : register(s0);

float4 main(INPUT_PIXEL input) : SV_TARGET
{
	return baseTexture.Sample(filters[0], input.uv);
}