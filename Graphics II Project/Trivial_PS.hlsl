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

cbuffer LIGHTING : register(b1)
{
	float3 lightingbools;
	float padding;

	float3 directinoallightdir;
	float padding2;

	float4 directionallightcolor;

	float3 pointlightpos;
	float padding3;

	float3 spotlightpos;
	float padding4;

	float3 conedir;
	float coneratio;
}

float4 main( INPUT_PIXEL input ) : SV_TARGET
{
	float4 newColor = baseTexture.Sample(filters[0], input.uv);
	//float4 newColor;
	//newColor.a = baseColor.b;
	//newColor.r = baseColor.g;
	//newColor.g = baseColor.r;
	//newColor.b = baseColor.a;
	float4 tempcolor = newColor;
	if (tempcolor.w < 1)
		tempcolor.w = .1;
	float3 lightdir = -normalize(directinoallightdir);

	//directionallight
	float4 lightcolor = newColor * directionallightcolor;
	float3 normal = normalize(input.normal);
	float4 directionlight = clamp((dot(lightdir, normal) * newColor) + (newColor + lightcolor), 0, 1);
	
	
	float3 specularviewdir = normalize(input.CameraCoord - input.worldcoord);
	float3 halfvector = normalize((-lightdir) * specularviewdir);
	float intensity = max(clamp(dot(input.normal, normalize(halfvector)),0, 1),1);

	directionlight *= intensity *= newColor;

	//pointlight
	float3 pointlightdir = normalize(pointlightpos - input.worldcoord);
	float4 pointlight = clamp((dot(pointlightdir, normal) * newColor), 0, 1);
	//float Attenuation = 1.0 - clamp(MAGNITUDE(pointlightpos - input.worldcoord) / lightradius, 0, 1);
	//pointlight *= Attenuation;
	specularviewdir = normalize(input.CameraCoord - input.worldcoord);
	halfvector = normalize((-pointlightdir) * specularviewdir);
	intensity = max(clamp(dot(input.normal, normalize(halfvector)), 0, 1), 1);
	
	pointlight *= intensity *= newColor;

	//spotlight

	float3 spotlightdir = normalize(spotlightpos - input.worldcoord);   
	float surfaceratio = clamp(dot(-spotlightdir, normalize(conedir)),0,1);
	float spotfactor = (surfaceratio > coneratio) ? 1 : 0;
	float lightratio = clamp(dot(spotlightdir, input.normal),0,1);
	float4 spotlight = spotfactor * lightratio;
	//Attenuation = 1.0 - clamp(magnitude(spotlightpos - input.worldcoord) / lightradius, 0, 1);
	//Attneuation *= 1.0 - clamp((coneratio - surfaceratio)/(coneratio
	//spotlight *= Attenuation;
	specularviewdir = normalize(input.CameraCoord - input.worldcoord);
	halfvector = normalize((-spotlightdir) * specularviewdir);
	intensity = max(clamp(dot(input.normal, normalize(halfvector)), 0, 1), 1);
	spotlight *= intensity *= newColor;
	

	if (newColor.a <= 0.1f)
		discard;
	else
		newColor.w = tempcolor.w;
	if (lightingbools[0] == 1 && lightingbools[1] == 1 && lightingbools[2] == 1)
		return clamp(directionlight + pointlight + spotlight, 0,1);
	else if (lightingbools[0] == 1 && lightingbools[1] == 1)
		return clamp(directionlight + pointlight, 0, 1);
	else if (lightingbools[0] == 1 && lightingbools[2] == 1)
		return clamp(directionlight + spotlight, 0, 1);
	else if (lightingbools[1] == 1 && lightingbools[2] == 1)
	{
		pointlight = clamp(pointlight + (newColor + lightcolor), 0, 1);
			return clamp(pointlight + spotlight, 0, 1);
	}
	else if (lightingbools[0] == 1)
		return clamp(directionlight, 0, 1);
	else if (lightingbools[1] == 1)
	{
		pointlight = clamp(pointlight + (newColor + lightcolor), 0, 1);
		return clamp(pointlight, 0, 1);
	}
	else if (lightingbools[2] == 1)
	{
		spotlight = clamp(spotlight + (newColor + lightcolor), 0, 1);
			return clamp(spotlight, 0, 1);
	}
	return 0;
}
//float4 main(INPUT_PIXEL input) : SV_TARGET
//{
//	return float4(input.uvout, 1);
//}