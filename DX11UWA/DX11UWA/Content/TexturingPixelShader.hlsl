// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 norm : NORMAL;
	float4 posWS : POSITIONWS;
};

texture2D ModelTexture : register(t0);

SamplerState StateBeingUsed : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 Color = ModelTexture.Sample(StateBeingUsed, input.uv);

	return Color;
}