// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 uv : UV;
	float3 norm : NORMAL;
	float3 tan : TANGENT;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 norm : NORMAL;
	float3 tan : TANGENT;
	float4 posWS : POSITIONWS;
	float4x4 tbn : TBN;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);

	output.posWS = pos;

	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.uv = input.uv;

	float3 normWS = input.norm;
	normWS = mul(normWS, model);
	//normWS = normalize(normWS);
	output.norm = normWS;

	float3 tanWS = input.tan;
	tanWS = mul(tanWS, model);
	//tanWS = normalize(tanWS);
	output.tan = tanWS;

	float3 bitWS = cross(normWS, tanWS);

	float4x4 TBN = { tanWS.x, tanWS.y, tanWS.z, 0.0f,
					bitWS.x, bitWS.y, bitWS.z, 0.0f,
					normWS.x, normWS.y, normWS.z, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f };

	//TBN = transpose(TBN);

	output.tbn = TBN;

	return output;
}