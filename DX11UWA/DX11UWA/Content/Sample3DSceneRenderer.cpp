﻿#include "pch.h"

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
	float fovAngleY = 70.0f * XM_PI / 180.0f;

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
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

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
	UpdateCamera(timer, 1.0f, 0.75f);

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

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);

	////////////////////////////////////////////////////////
	//Cude Translated to the right and not moving
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(2.0f, 0.0f, 0.0f)));

	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	context->DrawIndexed(m_indexCount, 0, 0);

	/////////////////////////////////////////////////////////
	//Pyramid translated left and rotaing on the Z axis
	float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
	double totalRotation = Rtime * radiansPerSecond;
	float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose( XMMatrixMultiply( XMMatrixRotationZ(radians), XMMatrixTranslation(-2.0f, 0.0f, 0.0f))));

	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	context->IASetVertexBuffers(0, 1, p_vertexBuffer.GetAddressOf(), &stride, &offset);

	context->IASetIndexBuffer(p_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->DrawIndexed(p_indexCount, 0, 0);

	///////////////////////////////////////////////////////
	//Tower Model Translated 5 to the Right
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(5.0f, 0.0f, 0.0f)));

	context->UpdateSubresource1(Model_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	UINT M_stride = sizeof(VertexPositionUVNormal);
	UINT M_offset = 0;

	context->IASetVertexBuffers(0, 1, Model_vertexBuffer.GetAddressOf(), &M_stride, &M_offset);

	context->IASetIndexBuffer(Model_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(Model_inputLayout.Get());
	context->VSSetShader(Model_vertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, Model_constantBuffer.GetAddressOf(), nullptr, nullptr);
	context->PSSetShader(Model_pixelShader.Get(), nullptr, 0);

	context->DrawIndexed(Model_indexCount, 0, 0);

	///////////////////////////////////////////////////////
	// Obj 2 Header Pyramid Translated 5 to the Left
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(-5.0f, 0.0f, 0.0f)));

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

	context->DrawIndexed(O2H_indexCount, 0, 0);

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
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
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
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});

	// Load mesh vertices. Each vertex has a position and a color.
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

	// Load mesh indices. Each trio of indices represents
	// a triangle to be rendered on the screen.
	// For example: 0,2,1 means that the vertices with indexes
	// 0, 2 and 1 from the vertex buffer compose the 
	// first triangle of this mesh.
	static const unsigned short PyramidIndices[] =
	{
		0,1,2, // -x
		0,3,1,

		0,4,3, // +x
		0,2,4,

		1,4,2, // -y
		1,3,4,
	};

	p_indexCount = ARRAYSIZE(PyramidIndices);

	D3D11_SUBRESOURCE_DATA pindexBufferData = { 0 };
	pindexBufferData.pSysMem = PyramidIndices;
	pindexBufferData.SysMemPitch = 0;
	pindexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC pindexBufferDesc(sizeof(PyramidIndices), D3D11_BIND_INDEX_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&pindexBufferDesc, &pindexBufferData, &p_indexBuffer));

	auto loadnewVSTask = DX::ReadDataAsync(L"VertexShader.cso");
	auto loadnewPSTask = DX::ReadDataAsync(L"PixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createnewVSTask = loadnewVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &Model_vertexShader));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &O2H_vertexShader));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &Grid_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC newvertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(newvertexDesc, ARRAYSIZE(newvertexDesc), &fileData[0], fileData.size(), &Model_inputLayout));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(newvertexDesc, ARRAYSIZE(newvertexDesc), &fileData[0], fileData.size(), &O2H_inputLayout));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(newvertexDesc, ARRAYSIZE(newvertexDesc), &fileData[0], fileData.size(), &Grid_inputLayout));

	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createnewPSTask = loadnewPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &Model_pixelShader));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &O2H_pixelShader));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &Grid_pixelShader));

		CD3D11_BUFFER_DESC newconstantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&newconstantBufferDesc, nullptr, &Model_constantBuffer));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&newconstantBufferDesc, nullptr, &O2H_constantBuffer));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&newconstantBufferDesc, nullptr, &Grid_constantBuffer));

	});

	Loaded = LoadOBJFile("Assets/Tower1.obj", &FirstModel);

	if (Loaded)
	{
		unsigned int ModelSize = FirstModel.MeshVerts.size();
		VertexPositionUVNormal* ModelVertices = new VertexPositionUVNormal[ModelSize];

		for (unsigned int i = 0; i < ModelSize; i++)
		{
			ModelVertices[i].pos = FirstModel.MeshVerts[i].Position;
			ModelVertices[i].uv = FirstModel.MeshVerts[i].UVW;
			ModelVertices[i].normal = FirstModel.MeshVerts[i].Normals;
		}

		D3D11_SUBRESOURCE_DATA ModelvertexBufferData = { 0 };
		ModelvertexBufferData.pSysMem = ModelVertices;
		ModelvertexBufferData.SysMemPitch = 0;
		ModelvertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC ModelvertexBufferDesc(sizeof(VertexPositionUVNormal) * ModelSize, D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&ModelvertexBufferDesc, &ModelvertexBufferData, &Model_vertexBuffer));

		unsigned int ModelIndicesSize = FirstModel.MeshIndecies.size();
		unsigned short* ModelIndices = new unsigned short[ModelIndicesSize];

		for (unsigned int i = 0; i < ModelIndicesSize; i++)
		{
			ModelIndices[i] = FirstModel.MeshIndecies[i];
		}

		Model_indexCount = ModelIndicesSize;

		D3D11_SUBRESOURCE_DATA ModelindexBufferData = { 0 };
		ModelindexBufferData.pSysMem = ModelIndices;
		ModelindexBufferData.SysMemPitch = 0;
		ModelindexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC ModelindexBufferDesc(sizeof(unsigned short) * ModelIndicesSize, D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&ModelindexBufferDesc, &ModelindexBufferData, &Model_indexBuffer));

	}

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

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
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
}