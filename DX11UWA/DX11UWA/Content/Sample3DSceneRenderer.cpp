#include "pch.h"

#include "Sample3DSceneRenderer.h"

using namespace DX11UWA;
using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
		Rtime = timer.GetTotalSeconds();
	}


	// Update or move camera here
	UpdateCamera(timer, 2.0f, 0.75f);

}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}

	if (m_kbuttons['V'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		fovAngleY = fovAngleY + (delta_time);

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		if (fovAngleY >= (150.0f * XM_PI / 180.0f))
		{
			fovAngleY = (150.0f * XM_PI / 180.0f);
		}

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['B'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		fovAngleY = fovAngleY - (delta_time);

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		if (fovAngleY <= (30.0f * XM_PI / 180.0f))
		{
			fovAngleY = (30.0f * XM_PI / 180.0f);
		}

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['C'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		fovAngleY = (70.0f * XM_PI / 180.0f);

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['I'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		NearPlane = NearPlane - 0.1f;

		if (NearPlane <= 0.01f)
		{
			NearPlane = 0.01f;
		}

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['O'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		NearPlane = NearPlane + 0.1f;

		if (NearPlane >= 50.0f)
		{
			NearPlane = 50.0f;
		}

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['K'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		FarPlane = FarPlane - 0.1f;

		if (FarPlane <= 10.0f)
		{
			FarPlane = 10.0f;
		}

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['L'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		FarPlane = FarPlane + 0.1f;

		if (FarPlane >= 100.0f)
		{
			FarPlane = 100.0f;
		}

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['U'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		NearPlane = 0.01f;

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['J'])
	{
		Size outputSize = m_deviceResources->GetOutputSize();

		float aspectRatio = outputSize.Width / outputSize.Height;

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		FarPlane = 100.0f;

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, NearPlane, FarPlane);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

	if (m_kbuttons['1'])
	{
		DLight = 1;
	}

	if (m_kbuttons['2'])
	{
		PLight = 1;
	}

	if (m_kbuttons['3'])
	{
		SLight = 1;
	}

	if (m_kbuttons['4'])
	{
		DLight = 0;
	}

	if (m_kbuttons['5'])
	{
		PLight = 0;
	}

	if (m_kbuttons['6'])
	{
		SLight = 0;
	}

	if (m_currMousePos) 
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}

	if (LightProperties.LightArray[0].direction.x >= 7.5f)
	{
		LightProperties.LightArray[0].direction.x = -7.5f;
	}
	LightProperties.LightArray[0].direction.x = (LightProperties.LightArray[0].direction.x + (delta_time * 2));

	if (LightProperties.LightArray[1].pos.x <= -10.0f && LightProperties.LightArray[1].pos.z <= -10.0f)
	{
		LightProperties.LightArray[1].pos.x = 10.0f;
		LightProperties.LightArray[1].pos.z = 10.0f;
	}

	LightProperties.LightArray[1].pos.x = (LightProperties.LightArray[1].pos.x - (delta_time));
	LightProperties.LightArray[1].pos.z = (LightProperties.LightArray[1].pos.x - (delta_time));

	if (LightProperties.LightArray[2].angle.z <= -2.0f)
	{
		LightProperties.LightArray[2].angle.z = 2.0f;
	}

	LightProperties.LightArray[2].angle.z = LightProperties.LightArray[2].angle.z - (delta_time * 0.5f);

	m_time = (float)timer.GetElapsedSeconds();
}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(void)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	ID3D11ShaderResourceView* TextureArray[] = { Moss_SRV, MossNorm_SRV, Concrete_SRV, Rock_SRV, RockNormal_SRV, SkyboxTexture_SRV, Pebble_SRV, PebbleNorm_SRV };
	
	D3D11_SAMPLER_DESC Sample1;
	ZeroMemory(&Sample1, sizeof(D3D11_SAMPLER_DESC));
	Sample1.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	Sample1.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	Sample1.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	Sample1.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	Sample1.MipLODBias = 0.0f;
	Sample1.MaxAnisotropy = 1;
	Sample1.ComparisonFunc = D3D11_COMPARISON_NEVER;
	Sample1.BorderColor[0] = 1.0f;
	Sample1.BorderColor[1] = 1.0f;
	Sample1.BorderColor[2] = 1.0f;
	Sample1.BorderColor[3] = 1.0f;
	Sample1.MinLOD = -FLT_MAX;
	Sample1.MaxLOD = FLT_MAX;

	m_deviceResources->GetD3DDevice()->CreateSamplerState(&Sample1, &WrapState);

	ID3D11SamplerState* SampleStates[] = { WrapState };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	LightProperties.CameraPos = { m_camera._41, m_camera._42, m_camera._43, m_camera._44 };

	XMVECTOR CameraPos = XMLoadFloat4(&LightProperties.CameraPos);

	LightProperties.LightArray[0].enabled.x = DLight;

	LightProperties.LightArray[1].enabled.x = PLight;

	LightProperties.LightArray[2].enabled.x = SLight;

	context->UpdateSubresource1(Lights_constantBuffer.Get(), 0, NULL, &LightProperties, 0, 0, 0);

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixIdentity()));
	
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	/////////////////////////////////////////////////////
	//Skybox
	ID3D11ShaderResourceView* SkyboxTextureArray[] = { SkyboxTexture_SRV };

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslationFromVector(CameraPos)));

	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	UINT m_stride = sizeof(VertexPositionColor);
	UINT m_offset = 0;

	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);

	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, SkyboxTextureArray);
	context->PSSetSamplers(0, 1, &SampleStates[0]);

	context->DrawIndexed(m_indexCount, 0, 0);

	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, NULL);

	/////////////////////////////////////////////////////
	//Spyro Model
	ID3D11ShaderResourceView* SpyroTextureArray[] = { Concrete_SRV };

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(180)), XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), XMMatrixTranslation(0.0f, 0.0f, 3.0f)))));

	context->UpdateSubresource1(SModel_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	UINT SM_stride = sizeof(VertexPositionUVNormal);
	UINT SM_offset = 0;

	context->IASetVertexBuffers(0, 1, SModel_vertexBuffer.GetAddressOf(), &SM_stride, &SM_offset);

	context->IASetIndexBuffer(SModel_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(SModel_inputLayout.Get());

	context->VSSetShader(SModel_vertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, SModel_constantBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(SModel_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, SpyroTextureArray);
	context->PSSetSamplers(0, 1, &SampleStates[0]);

	context->PSSetConstantBuffers(1, 1, Lights_constantBuffer.GetAddressOf());

	context->DrawIndexed(SModel_indexCount, 0, 0);

	////////////////////////////////////////////////////////////
	////Model
	ID3D11ShaderResourceView* ModelTextureArray[] = { Rock_SRV, RockNormal_SRV };

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationZ(XMConvertToRadians(0)), XMMatrixMultiply(XMMatrixScaling(1.0f, 1.0f, 1.0f), XMMatrixTranslation(15.0f, 0.0f, 0.0f)))));

	context->UpdateSubresource1(Model_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	UINT M_stride = sizeof(VertexPositionUVNormalTan);
	UINT M_offset = 0;

	context->IASetVertexBuffers(0, 1, Model_vertexBuffer.GetAddressOf(), &M_stride, &M_offset);

	context->IASetIndexBuffer(Model_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(Model_inputLayout.Get());
	context->VSSetShader(Model_vertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, Model_constantBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(Model_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 2, ModelTextureArray);
	context->PSSetSamplers(0, 1, &SampleStates[0]);

	context->DrawIndexed(Model_indexCount, 0, 0);

	///////////////////////////////////////////////////////
	// Obj 2 Header Pyramid Translated 5 to the Left
	ID3D11ShaderResourceView* O2HTextureArray[] = { Concrete_SRV };

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(-5.0f, 0.5f, 0.0f)));

	context->UpdateSubresource1(O2H_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	UINT M2_stride = sizeof(VertexPositionUVNormal);
	UINT M2_offset = 0;

	context->IASetVertexBuffers(0, 1, O2H_vertexBuffer.GetAddressOf(), &M2_stride, &M2_offset);

	context->IASetIndexBuffer(O2H_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(O2H_inputLayout.Get());
	context->VSSetShader(O2H_vertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, O2H_constantBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(O2H_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, O2HTextureArray);
	context->PSSetSamplers(0, 1, &SampleStates[0]);

	context->DrawIndexed(O2H_indexCount, 0, 0);

	/////////////////////////////////////////////////////
	//Grid
	ID3D11ShaderResourceView* GridTextureArray[] = { Pebble_SRV, PebbleNorm_SRV };

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(-50.0f, -0.10f, 50.0f)));

	context->UpdateSubresource1(Grid_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	UINT M3_stride = sizeof(VertexPositionUVNormalTan);
	UINT M3_offset = 0;

	context->IASetVertexBuffers(0, 1, Grid_vertexBuffer.GetAddressOf(), &M3_stride, &M3_offset);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(Grid_inputLayout.Get());

	context->VSSetShader(Grid_vertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, Grid_constantBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(Grid_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 2, GridTextureArray);
	context->PSSetSamplers(0, 1, &SampleStates[0]);

	context->Draw(Grid_indexCount, 0);
}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));

		auto loadnewVSTask = DX::ReadDataAsync(L"TexturingVertexShader.cso");
		auto loadnewPSTask = DX::ReadDataAsync(L"TexturingPixelShader.cso");

		// After the vertex shader file is loaded, create the shader and input layout.
		auto createnewVSTask = loadnewVSTask.then([this](const std::vector<byte>& fileData)
		{
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &O2H_vertexShader));
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &SModel_vertexShader));

			static const D3D11_INPUT_ELEMENT_DESC newvertexDesc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(newvertexDesc, ARRAYSIZE(newvertexDesc), &fileData[0], fileData.size(), &O2H_inputLayout));
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(newvertexDesc, ARRAYSIZE(newvertexDesc), &fileData[0], fileData.size(), &SModel_inputLayout));

		});

		// After the pixel shader file is loaded, create the shader and constant buffer.
		auto createnewPSTask = loadnewPSTask.then([this](const std::vector<byte>& fileData)
		{
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &O2H_pixelShader));
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &SModel_pixelShader));


			CD3D11_BUFFER_DESC newconstantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&newconstantBufferDesc, nullptr, &O2H_constantBuffer));
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&newconstantBufferDesc, nullptr, &SModel_constantBuffer));

			CD3D11_BUFFER_DESC LightsconstantBufferDesc(sizeof(LightProp), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&LightsconstantBufferDesc, nullptr, &Lights_constantBuffer));

		});

		HRESULT result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/OutputCube1.dds", (ID3D11Resource**)&SkyboxTexture, &SkyboxTexture_SRV);

		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/156_WM.dds", (ID3D11Resource**)&MossTexture, &Moss_SRV);
		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/156_norm.dds", (ID3D11Resource**)&MossNormTexture, &MossNorm_SRV);

		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/169_WithMips.dds", (ID3D11Resource**)&ConcreteTexture, &Concrete_SRV);

		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/Castle_Medium.dds", (ID3D11Resource**)&RockTexture, &Rock_SRV);
		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/CastleNorm.dds", (ID3D11Resource**)&RockNormalTexture, &RockNormal_SRV);

		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/188.dds", (ID3D11Resource**)&PebbleTexture, &Pebble_SRV);
		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/188_norm.dds", (ID3D11Resource**)&PebbleNormTexture, &PebbleNorm_SRV);

		auto createtheTask = (createnewVSTask && createnewPSTask).then([this]()
		{
#pragma region Object-2-Header Pyramid
			VertexPositionUVNormal O2HVertices[768];

			for (unsigned int i = 0; i < 768; i++)
			{
				O2HVertices[i].pos.x = test_pyramid_data[i].pos[0];
				O2HVertices[i].pos.y = test_pyramid_data[i].pos[1];
				O2HVertices[i].pos.z = test_pyramid_data[i].pos[2];

				O2HVertices[i].uv.x = test_pyramid_data[i].uvw[0];
				O2HVertices[i].uv.y = test_pyramid_data[i].uvw[1];
				O2HVertices[i].uv.z = test_pyramid_data[i].uvw[2];


				O2HVertices[i].normal.x = test_pyramid_data[i].nrm[0];
				O2HVertices[i].normal.y = test_pyramid_data[i].nrm[1];
				O2HVertices[i].normal.z = test_pyramid_data[i].nrm[2];
			}

			D3D11_SUBRESOURCE_DATA O2HvertexBufferData = { 0 };
			O2HvertexBufferData.pSysMem = O2HVertices;
			O2HvertexBufferData.SysMemPitch = 0;
			O2HvertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC O2HvertexBufferDesc(sizeof(O2HVertices), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&O2HvertexBufferDesc, &O2HvertexBufferData, &O2H_vertexBuffer));

			unsigned short O2HIndices[1674];

			for (unsigned int i = 0; i < 1674; i++)
			{
				O2HIndices[i] = 0;

				O2HIndices[i] = (unsigned short)(test_pyramid_indicies[i]);
			}

			O2H_indexCount = ARRAYSIZE(O2HIndices);

			D3D11_SUBRESOURCE_DATA O2HindexBufferData = { 0 };
			O2HindexBufferData.pSysMem = O2HIndices;
			O2HindexBufferData.SysMemPitch = 0;
			O2HindexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC O2HindexBufferDesc(sizeof(O2HIndices), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&O2HindexBufferDesc, &O2HindexBufferData, &O2H_indexBuffer));
#pragma endregion

#pragma region Spyro Model

			SpyroLoaded = LoadOBJFile("Assets/spyro1.obj", &SpyroModel);

			if (SpyroLoaded)
			{
				unsigned int SModelSize = SpyroModel.MeshVerts.size();
				VertexPositionUVNormal* SModelVertices = new VertexPositionUVNormal[SModelSize];

				for (unsigned int i = 0; i < SModelSize; i++)
				{
					SModelVertices[i].pos = SpyroModel.MeshVerts[i].Position;
					SModelVertices[i].uv = SpyroModel.MeshVerts[i].UVW;
					SModelVertices[i].normal = SpyroModel.MeshVerts[i].Normals;
				}

				D3D11_SUBRESOURCE_DATA SModelvertexBufferData = { 0 };
				SModelvertexBufferData.pSysMem = SModelVertices;
				SModelvertexBufferData.SysMemPitch = 0;
				SModelvertexBufferData.SysMemSlicePitch = 0;
				CD3D11_BUFFER_DESC SModelvertexBufferDesc(sizeof(VertexPositionUVNormal) * SModelSize, D3D11_BIND_VERTEX_BUFFER);
				DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&SModelvertexBufferDesc, &SModelvertexBufferData, &SModel_vertexBuffer));

				unsigned int SModelIndicesSize = SpyroModel.MeshIndecies.size();
				unsigned short* SModelIndices = new unsigned short[SModelIndicesSize];

				for (unsigned int i = 0; i < SModelIndicesSize; i++)
				{
					SModelIndices[i] = SpyroModel.MeshIndecies[i];
				}

				SModel_indexCount = SModelIndicesSize;

				D3D11_SUBRESOURCE_DATA SModelindexBufferData = { 0 };
				SModelindexBufferData.pSysMem = SModelIndices;
				SModelindexBufferData.SysMemPitch = 0;
				SModelindexBufferData.SysMemSlicePitch = 0;
				CD3D11_BUFFER_DESC SModelindexBufferDesc(sizeof(unsigned short) * SModelIndicesSize, D3D11_BIND_INDEX_BUFFER);
				DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&SModelindexBufferDesc, &SModelindexBufferData, &SModel_indexBuffer));

			}

#pragma endregion

#pragma region Lights
			DirLight.enabled = { 0, 0 };
			DirLight.pos = { 0.0f, 0.0f, 0.0f, 1.0f };
			DirLight.direction = { 0.0f, -1.0f, 0.0f, 1.0f };
			DirLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			DirLight.angle = { 0.0f, 0.0f, 0.0f, 0.0f };
			DirLight.angleratio = { 0.0f, 0.0f };
			DirLight.C_att = { 1.0f, 0.0f };
			DirLight.L_att = { 1.0f, 0.0f };
			DirLight.Q_att = { 0.0f, 0.0f };
			DirLight.type = { 0, 0 };
			DirLight.padding = { 0, 0, 0, 0 };

			PointLight.enabled = { 0, 0 };
			PointLight.pos = { 0.0f, 2.0f, 0.0f, 1.0f };
			PointLight.direction = { -5.0f, 5.0f, 0.0f, 0.0f };
			PointLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			PointLight.angle = { 0.0f, 0.0f, 0.0f, 0.0f };
			PointLight.angleratio = { 0.0f, 0.0f };
			PointLight.C_att = { 0.25f, 0.0f };
			PointLight.L_att = { 0.0f, 0.0f };
			PointLight.Q_att = { 0.1f, 0.0f };
			PointLight.type = { 1, 0 };
			PointLight.padding = { 0, 0, 0, 0 };

			SpotLight.enabled = { 0, 0 };
			SpotLight.pos = { 0.0f, 10.0f, 0.0f, 1.0f };
			SpotLight.direction = { 0.0f, 0.0f, 0.0f, 0.0f };
			SpotLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			SpotLight.angle = { 0.0f, -2.0f, 1.0f, 0.0f };
			SpotLight.angleratio = { 0.98f, 0.95f };
			SpotLight.C_att = { 1.0f, 0.0f };
			SpotLight.L_att = { 0.0f, 0.0f };
			SpotLight.Q_att = { 0.0f, 0.0f };
			SpotLight.type = { 2, 0 };
			SpotLight.padding = { 0, 0, 0, 0 };

			LightProperties.LightArray[0] = DirLight;
			LightProperties.LightArray[1] = PointLight;
			LightProperties.LightArray[2] = SpotLight;
#pragma endregion

		});

		auto VSTask = DX::ReadDataAsync(L"NormalTexturingVertexShader.cso");
		auto PSTask = DX::ReadDataAsync(L"NormalTexturingPixelShader.cso");

		// After the vertex shader file is loaded, create the shader and input layout.
		auto cVSTask = VSTask.then([this](const std::vector<byte>& fileData)
		{
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &Model_vertexShader));
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &Grid_vertexShader));


			static const D3D11_INPUT_ELEMENT_DESC vertDesc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertDesc, ARRAYSIZE(vertDesc), &fileData[0], fileData.size(), &Model_inputLayout));
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertDesc, ARRAYSIZE(vertDesc), &fileData[0], fileData.size(), &Grid_inputLayout));

		});

		// After the pixel shader file is loaded, create the shader and constant buffer.
		auto cPSTask = PSTask.then([this](const std::vector<byte>& fileData)
		{
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &Model_pixelShader));
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &Grid_pixelShader));


			CD3D11_BUFFER_DESC newconstantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&newconstantBufferDesc, nullptr, &Model_constantBuffer));
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&newconstantBufferDesc, nullptr, &Grid_constantBuffer));

		});

		auto createTask = (cVSTask && cPSTask).then([this]()
		{

#pragma region Model
			Loaded = LoadOBJFile("Assets/Hyrule_Castle1.obj", &FirstModel);

			if (Loaded)
			{
				unsigned int ModelSize = FirstModel.MeshVerts.size();
				VertexPositionUVNormalTan* ModelVertices = new VertexPositionUVNormalTan[ModelSize];

				for (unsigned int i = 0; i < ModelSize; i++)
				{
					ModelVertices[i].pos = FirstModel.MeshVerts[i].Position;
					ModelVertices[i].uv = FirstModel.MeshVerts[i].UVW;
					ModelVertices[i].normal = FirstModel.MeshVerts[i].Normals;
					ModelVertices[i].tangent = { 0.0f, 0.0f, 0.0f };
				}

				unsigned int ModelIndicesSize = FirstModel.MeshIndecies.size();
				unsigned short* ModelIndices = new unsigned short[ModelIndicesSize];

				for (unsigned int i = 0; i < ModelIndicesSize; i++)
				{
					ModelIndices[i] = FirstModel.MeshIndecies[i];
				}

				for (unsigned int i = 0; i < ModelIndicesSize; i += 3)
				{
					float x1 = ModelVertices[ModelIndices[i + 1]].pos.x - ModelVertices[ModelIndices[i]].pos.x;
					float x2 = ModelVertices[ModelIndices[i + 2]].pos.x - ModelVertices[ModelIndices[i]].pos.x;
					float y1 = ModelVertices[ModelIndices[i + 1]].pos.y - ModelVertices[ModelIndices[i]].pos.y;
					float y2 = ModelVertices[ModelIndices[i + 2]].pos.y - ModelVertices[ModelIndices[i]].pos.y;
					float z1 = ModelVertices[ModelIndices[i + 1]].pos.z - ModelVertices[ModelIndices[i]].pos.z;
					float z2 = ModelVertices[ModelIndices[i + 2]].pos.z - ModelVertices[ModelIndices[i]].pos.z;

					float s1 = ModelVertices[ModelIndices[i + 1]].uv.x - ModelVertices[ModelIndices[i]].uv.x;
					float s2 = ModelVertices[ModelIndices[i + 2]].uv.x - ModelVertices[ModelIndices[i]].uv.x;
					float t1 = ModelVertices[ModelIndices[i + 1]].uv.y - ModelVertices[ModelIndices[i]].uv.y;
					float t2 = ModelVertices[ModelIndices[i + 2]].uv.y - ModelVertices[ModelIndices[i]].uv.y;

					float r = 1.0f / ((s1 * t2) - (s2 * t1));

					//XMFLOAT3 Sdir = { (((t2 * x1) - (s2 * x2)) * r), (((t2 * y1) - (s2 * y2)) * r), (((t2 * z1) - (s2 * z2)) * r) };
					//XMFLOAT3 Tdir = { (((s1 * x2) - (t1 * x1)) * r), (((s1 * y2) - (t1 * y1)) * r), (((s1 * z2) - (t1 * z1)) * r) };

					XMFLOAT3 Sdir = { (((t2 * x1) - (t1 * x2)) * r), (((t2 * y1) - (t1 * y2)) * r), (((t2 * z1) - (t1 * z2)) * r) };
					XMFLOAT3 Tdir = { (((s1 * x2) - (s2 * x1)) * r), (((s1 * y2) - (s2 * y1)) * r), (((s1 * z2) - (s2 * z1)) * r) };

					XMVECTOR S = XMLoadFloat3(&Sdir);
					XMVECTOR T = XMLoadFloat3(&Tdir);
					XMVECTOR N1 = XMLoadFloat3(&ModelVertices[ModelIndices[i]].normal);
					XMVECTOR N2 = XMLoadFloat3(&ModelVertices[ModelIndices[i + 1]].normal);
					XMVECTOR N3 = XMLoadFloat3(&ModelVertices[ModelIndices[i + 2]].normal);

					XMVECTOR Tangent1 = XMLoadFloat3(&ModelVertices[ModelIndices[i]].tangent);
					XMVECTOR Tangent2 = XMLoadFloat3(&ModelVertices[ModelIndices[i + 1]].tangent);
					XMVECTOR Tangent3 = XMLoadFloat3(&ModelVertices[ModelIndices[i + 2]].tangent);

					XMVECTOR Tan1 = XMVector3Cross(S, N1) + Tangent1;
					XMVECTOR Tan2 = XMVector3Cross(S, N2) + Tangent2;
					XMVECTOR Tan3 = XMVector3Cross(S, N3) + Tangent3;

					XMStoreFloat3(&ModelVertices[ModelIndices[i]].tangent, Tan1);
					XMStoreFloat3(&ModelVertices[ModelIndices[i + 1]].tangent, Tan2);
					XMStoreFloat3(&ModelVertices[ModelIndices[i + 2]].tangent, Tan3);
				}

				D3D11_SUBRESOURCE_DATA ModelvertexBufferData = { 0 };
				ModelvertexBufferData.pSysMem = ModelVertices;
				ModelvertexBufferData.SysMemPitch = 0;
				ModelvertexBufferData.SysMemSlicePitch = 0;
				CD3D11_BUFFER_DESC ModelvertexBufferDesc(sizeof(VertexPositionUVNormalTan) * ModelSize, D3D11_BIND_VERTEX_BUFFER);
				DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&ModelvertexBufferDesc, &ModelvertexBufferData, &Model_vertexBuffer));

				Model_indexCount = ModelIndicesSize;

				D3D11_SUBRESOURCE_DATA ModelindexBufferData = { 0 };
				ModelindexBufferData.pSysMem = ModelIndices;
				ModelindexBufferData.SysMemPitch = 0;
				ModelindexBufferData.SysMemSlicePitch = 0;
				CD3D11_BUFFER_DESC ModelindexBufferDesc(sizeof(unsigned short) * ModelIndicesSize, D3D11_BIND_INDEX_BUFFER);
				DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&ModelindexBufferDesc, &ModelindexBufferData, &Model_indexBuffer));
			}
#pragma endregion

#pragma region Grid

			VertexPositionUVNormalTan GridVertices[600];
			unsigned int gridverts = 0;
			for (unsigned int z = 0; z < 10; z++)
			{
				for (unsigned int x = 0; x < 10; x++)
				{
					GridVertices[gridverts].pos.x = 0.0f + ((float)x * 10.0f);
					GridVertices[gridverts].pos.y = 0.0f;
					GridVertices[gridverts].pos.z = (0.0f + ((float)z * -10.0f));

					GridVertices[gridverts].uv.x = 0.0f;
					GridVertices[gridverts].uv.y = 0.0f;
					GridVertices[gridverts].uv.z = 0.0f;

					GridVertices[gridverts].normal.x = 0.0f;
					GridVertices[gridverts].normal.y = 1.0f;
					GridVertices[gridverts].normal.z = 0.0f;

					GridVertices[gridverts].tangent.x = 0.0f;
					GridVertices[gridverts].tangent.y = 0.0f;
					GridVertices[gridverts].tangent.z = 0.0f;

					++gridverts;

					GridVertices[gridverts].pos.x = 10.0f + ((float)x* 10.0f);
					GridVertices[gridverts].pos.y = 0.0f;
					GridVertices[gridverts].pos.z = (-10.0f + ((float)z * -10.0f));

					GridVertices[gridverts].uv.x = 1.0f;
					GridVertices[gridverts].uv.y = 1.0f;
					GridVertices[gridverts].uv.z = 0.0f;

					GridVertices[gridverts].normal.x = 0.0f;
					GridVertices[gridverts].normal.y = 1.0f;
					GridVertices[gridverts].normal.z = 0.0f;

					GridVertices[gridverts].tangent.x = 0.0f;
					GridVertices[gridverts].tangent.y = 0.0f;
					GridVertices[gridverts].tangent.z = 0.0f;

					++gridverts;

					GridVertices[gridverts].pos.x = 0.0f + ((float)x * 10.0f);
					GridVertices[gridverts].pos.y = 0.0f;
					GridVertices[gridverts].pos.z = (-10.0f + ((float)z * -10.0f));

					GridVertices[gridverts].uv.x = 0.0f;
					GridVertices[gridverts].uv.y = 1.0f;
					GridVertices[gridverts].uv.z = 0.0f;

					GridVertices[gridverts].normal.x = 0.0f;
					GridVertices[gridverts].normal.y = 1.0f;
					GridVertices[gridverts].normal.z = 0.0f;

					GridVertices[gridverts].tangent.x = 0.0f;
					GridVertices[gridverts].tangent.y = 0.0f;
					GridVertices[gridverts].tangent.z = 0.0f;

					float x1 = GridVertices[gridverts - 1].pos.x - GridVertices[gridverts - 2].pos.x;
					float x2 = GridVertices[gridverts].pos.x - GridVertices[gridverts - 2].pos.x;
					float y1 = GridVertices[gridverts - 1].pos.y - GridVertices[gridverts - 2].pos.y;
					float y2 = GridVertices[gridverts].pos.y - GridVertices[gridverts - 2].pos.y;
					float z1 = GridVertices[gridverts - 1].pos.z - GridVertices[gridverts - 2].pos.z;
					float z2 = GridVertices[gridverts].pos.z - GridVertices[gridverts - 2].pos.z;

					float s1 = GridVertices[gridverts - 1].uv.x - GridVertices[gridverts - 2].uv.x;
					float s2 = GridVertices[gridverts].uv.x - GridVertices[gridverts - 2].uv.x;
					float t1 = GridVertices[gridverts - 1].uv.y - GridVertices[gridverts - 2].uv.y;
					float t2 = GridVertices[gridverts].uv.y - GridVertices[gridverts - 2].uv.y;

					float r = 1.0f / ((s1 * t2) - (s2 * t1));

					XMFLOAT3 Sdir = { (((t2 * x1) - (t1 * x2)) * r), (((t2 * y1) - (t1 * y2)) * r), (((t2 * z1) - (t1 * z2)) * r) };
					XMFLOAT3 Tdir = { (((s1 * x2) - (s2 * x1)) * r), (((s1 * y2) - (s2 * y1)) * r), (((s1 * z2) - (s2 * z1)) * r) };

					XMVECTOR S = XMLoadFloat3(&Sdir);
					XMVECTOR T = XMLoadFloat3(&Tdir);
					XMVECTOR N1 = XMLoadFloat3(&GridVertices[gridverts - 2].normal);
					XMVECTOR N2 = XMLoadFloat3(&GridVertices[gridverts - 1].normal);
					XMVECTOR N3 = XMLoadFloat3(&GridVertices[gridverts].normal);

					XMVECTOR Tangent1 = XMLoadFloat3(&GridVertices[gridverts - 2].tangent);
					XMVECTOR Tangent2 = XMLoadFloat3(&GridVertices[gridverts - 1].tangent);
					XMVECTOR Tangent3 = XMLoadFloat3(&GridVertices[gridverts].tangent);

					XMVECTOR Tan1 = XMVector3Cross(S, N1) + Tangent1;
					XMVECTOR Tan2 = XMVector3Cross(S, N2) + Tangent2;
					XMVECTOR Tan3 = XMVector3Cross(S, N3) + Tangent3;

					XMStoreFloat3(&GridVertices[gridverts - 2].tangent, Tan1);
					XMStoreFloat3(&GridVertices[gridverts - 1].tangent, Tan2);
					XMStoreFloat3(&GridVertices[gridverts].tangent, Tan3);

					++gridverts;

					GridVertices[gridverts].pos.x = 0.0f + ((float)x * 10.0f);
					GridVertices[gridverts].pos.y = 0.0f;
					GridVertices[gridverts].pos.z = (0.0f + ((float)z * -10.0f));

					GridVertices[gridverts].uv.x = 0.0f;
					GridVertices[gridverts].uv.y = 0.0f;
					GridVertices[gridverts].uv.z = 0.0f;

					GridVertices[gridverts].normal.x = 0.0f;
					GridVertices[gridverts].normal.y = 1.0f;
					GridVertices[gridverts].normal.z = 0.0f;

					GridVertices[gridverts].tangent.x = 0.0f;
					GridVertices[gridverts].tangent.y = 0.0f;
					GridVertices[gridverts].tangent.z = 0.0f;

					++gridverts;

					GridVertices[gridverts].pos.x = 10.0f + ((float)x * 10.0f);
					GridVertices[gridverts].pos.y = 0.0f;
					GridVertices[gridverts].pos.z = (0.0f + ((float)z * -10.0f));

					GridVertices[gridverts].uv.x = 1.0f;
					GridVertices[gridverts].uv.y = 0.0f;
					GridVertices[gridverts].uv.z = 0.0f;

					GridVertices[gridverts].normal.x = 0.0f;
					GridVertices[gridverts].normal.y = 1.0f;
					GridVertices[gridverts].normal.z = 0.0f;

					GridVertices[gridverts].tangent.x = 0.0f;
					GridVertices[gridverts].tangent.y = 0.0f;
					GridVertices[gridverts].tangent.z = 0.0f;

					++gridverts;

					GridVertices[gridverts].pos.x = 10.0f + ((float)x * 10.0f);
					GridVertices[gridverts].pos.y = 0.0f;
					GridVertices[gridverts].pos.z = (-10.0f + ((float)z * -10.0f));

					GridVertices[gridverts].uv.x = 1.0f;
					GridVertices[gridverts].uv.y = 1.0f;
					GridVertices[gridverts].uv.z = 0.0f;

					GridVertices[gridverts].normal.x = 0.0f;
					GridVertices[gridverts].normal.y = 1.0f;
					GridVertices[gridverts].normal.z = 0.0f;

					GridVertices[gridverts].tangent.x = 0.0f;
					GridVertices[gridverts].tangent.y = 0.0f;
					GridVertices[gridverts].tangent.z = 0.0f;

					x1 = GridVertices[gridverts - 1].pos.x - GridVertices[gridverts - 2].pos.x;
					x2 = GridVertices[gridverts].pos.x - GridVertices[gridverts - 2].pos.x;
					y1 = GridVertices[gridverts - 1].pos.y - GridVertices[gridverts - 2].pos.y;
					y2 = GridVertices[gridverts].pos.y - GridVertices[gridverts - 2].pos.y;
					z1 = GridVertices[gridverts - 1].pos.z - GridVertices[gridverts - 2].pos.z;
					z2 = GridVertices[gridverts].pos.z - GridVertices[gridverts - 2].pos.z;

					s1 = GridVertices[gridverts - 1].uv.x - GridVertices[gridverts - 2].uv.x;
					s2 = GridVertices[gridverts].uv.x - GridVertices[gridverts - 2].uv.x;
					t1 = GridVertices[gridverts - 1].uv.y - GridVertices[gridverts - 2].uv.y;
					t2 = GridVertices[gridverts].uv.y - GridVertices[gridverts - 2].uv.y;

					r = 1.0f / ((s1 * t2) - (s2 * t1));

					Sdir = { (((t2 * x1) - (t1 * x2)) * r), (((t2 * y1) - (t1 * y2)) * r), (((t2 * z1) - (t1 * z2)) * r) };
					Tdir = { (((s1 * x2) - (s2 * x1)) * r), (((s1 * y2) - (s2 * y1)) * r), (((s1 * z2) - (s2 * z1)) * r) };

					S = XMLoadFloat3(&Sdir);
					T = XMLoadFloat3(&Tdir);
					N1 = XMLoadFloat3(&GridVertices[gridverts - 2].normal);
					N2 = XMLoadFloat3(&GridVertices[gridverts - 1].normal);
					N3 = XMLoadFloat3(&GridVertices[gridverts].normal);

					Tangent1 = XMLoadFloat3(&GridVertices[gridverts - 2].tangent);
					Tangent2 = XMLoadFloat3(&GridVertices[gridverts - 1].tangent);
					Tangent3 = XMLoadFloat3(&GridVertices[gridverts].tangent);

					Tan1 = XMVector3Cross(S, N1) + Tangent1;
					Tan2 = XMVector3Cross(S, N2) + Tangent2;
					Tan3 = XMVector3Cross(S, N3) + Tangent3;

					XMStoreFloat3(&GridVertices[gridverts - 2].tangent, Tan1);
					XMStoreFloat3(&GridVertices[gridverts - 1].tangent, Tan2);
					XMStoreFloat3(&GridVertices[gridverts].tangent, Tan3);

					++gridverts;
				}
			}

			Grid_indexCount = gridverts;

			D3D11_SUBRESOURCE_DATA GridvertexBufferData = { 0 };
			GridvertexBufferData.pSysMem = GridVertices;
			GridvertexBufferData.SysMemPitch = 0;
			GridvertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC GridvertexBufferDesc(sizeof(GridVertices), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&GridvertexBufferDesc, &GridvertexBufferData, &Grid_vertexBuffer));
#pragma endregion
		});
	});


	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
#pragma region Starter Cube
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			2,1,0, // -x
			2,3,1,

			5,6,4, // +x
			7,6,5,

			1,5,0, // -y
			5,4,0,

			6,7,2, // +y
			7,3,2,

			4,6,0, // -z
			6,2,0,

			3,7,1, // +z
			7,5,1,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
#pragma endregion

#pragma region Pyramid
		static const VertexPositionColor PyramidVertices[] =
		{
			{ XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  -0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  -0.5f, 0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  -0.5f,  -0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		};

		D3D11_SUBRESOURCE_DATA pvertexBufferData = { 0 };
		pvertexBufferData.pSysMem = PyramidVertices;
		pvertexBufferData.SysMemPitch = 0;
		pvertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC pvertexBufferDesc(sizeof(PyramidVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pvertexBufferDesc, &pvertexBufferData, &p_vertexBuffer));

		static const unsigned short PyramidIndices[] =
		{
			0,1,2,
			0,3,1,

			0,4,3,
			0,2,4,

			1,4,2,
			1,3,4,
		};

		p_indexCount = ARRAYSIZE(PyramidIndices);

		D3D11_SUBRESOURCE_DATA pindexBufferData = { 0 };
		pindexBufferData.pSysMem = PyramidIndices;
		pindexBufferData.SysMemPitch = 0;
		pindexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC pindexBufferDesc(sizeof(PyramidIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pindexBufferDesc, &pindexBufferData, &p_indexBuffer));
#pragma endregion

		
	});

	createCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});


}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	loadingcomplete = false;
	tloadingcomplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();

	p_indexBuffer.Reset();
	p_vertexBuffer.Reset();

	Model_inputLayout.Reset();
	Model_vertexBuffer.Reset();
	Model_indexBuffer.Reset();
	Model_vertexShader.Reset();
	Model_pixelShader.Reset();
	Model_constantBuffer.Reset();

	O2H_inputLayout.Reset();
	O2H_vertexBuffer.Reset();
	O2H_indexBuffer.Reset();
	O2H_vertexShader.Reset();
	O2H_pixelShader.Reset();
	O2H_constantBuffer.Reset();

	SModel_inputLayout.Reset();
	SModel_vertexBuffer.Reset();
	SModel_indexBuffer.Reset();
	SModel_vertexShader.Reset();
	SModel_pixelShader.Reset();
	SModel_constantBuffer.Reset();

	Lights_constantBuffer.Reset();

	MossTexture->Release();
	Moss_SRV->Release();

	MossNormTexture->Release();
	MossNorm_SRV->Release();

	WrapState->Release();

	ConcreteTexture->Release();
	Concrete_SRV->Release();

	RockTexture->Release();
	Rock_SRV->Release();

	RockNormalTexture->Release();
	RockNormal_SRV->Release();

	SkyboxTexture->Release();
	SkyboxTexture_SRV->Release();
}