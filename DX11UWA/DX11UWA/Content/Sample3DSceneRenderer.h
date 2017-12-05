#pragma once

#include "OBJModelLoader.h"
#include "test pyramid.h"
#include "Common\DDSTextureLoader.h"

namespace DX11UWA
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources(void);
		void CreateWindowSizeDependentResources(void);
		void ReleaseDeviceDependentResources(void);
		void Update(DX::StepTimer const& timer);
		void Render(void);
		void StartTracking(void);
		void TrackingUpdate(float positionX);
		void StopTracking(void);
		inline bool IsTracking(void) { return m_tracking; }

		double Rtime = 0.0;

		// Helper functions for keyboard and mouse input
		void SetKeyboardButtons(const char* list);
		void SetMousePosition(const Windows::UI::Input::PointerPoint^ pos);
		void SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos);


	private:
		void Rotate(float radians);
		void UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		ID3D11Texture2D* MossTexture = nullptr;
		ID3D11ShaderResourceView* Moss_SRV = nullptr;
		ID3D11SamplerState* WrapState = nullptr;

		ID3D11Texture2D* ConcreteTexture = nullptr;
		ID3D11ShaderResourceView* Concrete_SRV = nullptr;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// Direct3D resources for Pyramid geometry.
		Microsoft::WRL::ComPtr<ID3D11Buffer>		p_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		p_indexBuffer;

		// System resources for Pyramid geometry.
		uint32	p_indexCount;

		// Direct3D resources for Model geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	Model_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		Model_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		Model_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	Model_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	Model_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		Model_constantBuffer;

		// System resources for Model geometry.
		uint32	Model_indexCount;
		ObjectData FirstModel;
		bool Loaded;

		// Direct3D resources for Obj2Header Pyramid geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	O2H_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		O2H_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		O2H_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	O2H_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	O2H_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		O2H_constantBuffer;

		// System resources for Obj2Header Pyramid geometry.
		uint32	O2H_indexCount;

		// Direct3D resources for Obj2Header Pyramid geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	Grid_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		Grid_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		Grid_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	Grid_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	Grid_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		Grid_constantBuffer;

		// System resources for Obj2Header Pyramid geometry.
		uint32	Grid_indexCount;

		// Direct3D resources for Spyro Model geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	SModel_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		SModel_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		SModel_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	SModel_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	SModel_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		SModel_constantBuffer;

		// System resources for Spyro Model geometry.
		uint32	SModel_indexCount;
		ObjectData SpyroModel;
		bool SpyroLoaded;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		// Data members for keyboard and mouse input
		char	m_kbuttons[256];
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;
	};
}

