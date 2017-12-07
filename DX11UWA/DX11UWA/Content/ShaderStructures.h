#pragma once

#define LIGHT_AMOUNT 3

namespace DX11UWA
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct Lights
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 direction;
		DirectX::XMFLOAT4 color;

		DirectX::XMFLOAT4 angle;
		DirectX::XMFLOAT2 angleratio;
		DirectX::XMFLOAT2 C_att;
		DirectX::XMFLOAT2 L_att;
		DirectX::XMFLOAT2 Q_att;

		DirectX::XMINT2 type;
		DirectX::XMINT2 enabled;
		DirectX::XMINT4 padding;
	};

	struct LightProp
	{
		DirectX::XMFLOAT4 CameraPos;
		Lights LightArray[LIGHT_AMOUNT];
	};

	struct VertexPosition
	{
		DirectX::XMFLOAT3 pos;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexPositionUVNormal
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 uv;
		DirectX::XMFLOAT3 normal;
	};
}