#pragma pack_matrix(row_major)
struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float3 uv : TEXCOORD;
	float3 normal : NORMAL;
};

struct OUTPUT_VERTEX
{
	//float4 colorOut : COLOR;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
	float3 normal : NORMAL;
	float3 worldcoord : POSITION;
	float4 projectedCoordinate : SV_POSITION;
	float4 CameraCoord : COORD;
};



cbuffer OBJECT : register(b1)
{
	float4x4 worldMatrix;
}

cbuffer SCENE : register(b2)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}


OUTPUT_VERTEX main( INPUT_VERTEX fromVertexBuffer )
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
		
	// TODO : PART 4 STEP 4
	//float4 spot;
	float4 local = float4 (fromVertexBuffer.coordinate.xyz, 1);
	local = mul(local, worldMatrix);
	sendToRasterizer.worldcoord = local;
	local = mul(local, viewMatrix);
	local = mul(local, projectionMatrix);
	sendToRasterizer.projectedCoordinate = local;
	sendToRasterizer.normal = fromVertexBuffer.normal;
	
	float4x4 temp = transpose(viewMatrix);
	sendToRasterizer.CameraCoord.x = temp[3][0];
	sendToRasterizer.CameraCoord.y = temp[3][1];
	sendToRasterizer.CameraCoord.z = temp[3][2];
	sendToRasterizer.CameraCoord.w = temp[3][3];

	// TODO : PART 3 STEP 7
	//sendToRasterizer.colorOut = fromVertexBuffer.color;
	sendToRasterizer.uv = fromVertexBuffer.uv.xy;
	// END PART 3

	return sendToRasterizer;
}