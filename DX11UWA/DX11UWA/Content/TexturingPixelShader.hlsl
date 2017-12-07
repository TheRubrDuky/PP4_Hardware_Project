#define LIGHT_AMOUNT 3
#define DIR_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct Lights
{
	float4 pos;
	float4 direction;
	float4 color;

	float4 angle;
	float2 angleratio;
	float2 C_att;
	float2 L_att;
	float2 Q_att;

	int2 type;
	int2 enabled;
	int4 padding;
};

cbuffer LightProp : register (b1)
{
	float4 CameraPos;
	Lights LightArray[LIGHT_AMOUNT];
}

texture2D ModelTexture : register(t0);

SamplerState StateBeingUsed : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 norm : NORMAL;
	float4 posWS : POSITIONWS;
};

float Attenuation(Lights light, float distance)
{
	float Att = 1.0f / (light.C_att.x + (light.L_att.x * distance) + (light.Q_att.x * distance * distance));

	return Att;
}

float4 Directional_Light(Lights light, float3 Norm, float4 SurfaceColor)
{
	float LightRatio = clamp(dot(-normalize(light.direction), Norm), 0, 1);

	float4 result = LightRatio * light.color * SurfaceColor;

	return result;
}

float4 Point_Light(Lights light, float3 Norm, float4 SurfaceColor, float4 Position)
{
	float3 LightDir = normalize(light.pos - Position);

	float LightRatio = clamp(dot(LightDir, Norm), 0, 1);

	float distance = length((light.pos - Position).xyz);

	float att = Attenuation(light, distance);

	float4 result = LightRatio * light.color * SurfaceColor * att;

	return result;
}

float4 Spot_Light(Lights light, float3 Norm, float4 SurfaceColor, float4 Position)
{
	float3 LightDir = normalize(light.pos - Position);

	float SurfaceRatio = clamp(dot(-LightDir, normalize(light.angle)), 0, 1);

	float SpotFactor = 1.0f - (clamp(((light.angleratio.x - SurfaceRatio) / (light.angleratio.x - light.angleratio.y)), 0, 1));

	float LightRatio = clamp(dot(LightDir, Norm), 0, 1);

	float distance = length((light.pos - Position).xyz);

	float att = Attenuation(light, distance);

	float4 result = SpotFactor * LightRatio * light.color * SurfaceColor * att;

	return result;
}

float4 TotalLighting(float4 WSPos, float3 Norm, float4 SurfaceColor)
{
	float3 view = normalize(CameraPos - WSPos).xyz;

	float4 result = { 0.0f, 0.0f, 0.0f, 0.0f };

	[unroll]
	for (unsigned int i = 0; i < LIGHT_AMOUNT; i++)
	{
		float4 TempResult = { 0.0f, 0.0f, 0.0f, 0.0f };

		if (LightArray[i].enabled.x == 0) continue;

		switch (LightArray[i].type.x)
		{
		case DIR_LIGHT:
			{
				TempResult = Directional_Light(LightArray[i], Norm, SurfaceColor);
			}
			break;
		case POINT_LIGHT:
			{
				TempResult = Point_Light(LightArray[i], Norm, SurfaceColor, WSPos);
			}
			break;
		case SPOT_LIGHT:
			{
				TempResult = Spot_Light(LightArray[i], Norm, SurfaceColor, WSPos);
			}
			break;
		}

		result += TempResult;
	}

	result = saturate(result);

	return result;
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 Color = ModelTexture.Sample(StateBeingUsed, input.uv);

	float4 FinalColor = TotalLighting(input.posWS, normalize(input.norm), Color);

	float4 Ambiance = { 0.20f, 0.20f, 0.20f, 1.0f };

	Ambiance = Ambiance * Color;

	FinalColor = FinalColor + Ambiance;

	FinalColor = saturate(FinalColor);

	return FinalColor;
}