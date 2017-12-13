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

		float fovAngleY = 0.0f;
		float NearPlane = 0.01f;
		float FarPlane = 100.0f;

		float m_time;

		ID3D11SamplerState* WrapState = nullptr;

		//Ground Texture
		ID3D11Texture2D* GroundTexture = nullptr;
		ID3D11Texture2D* GroundNormalTexture = nullptr;

		ID3D11ShaderResourceView* Ground_SRV = nullptr;
		ID3D11ShaderResourceView* GroundNormal_SRV = nullptr;

		//Barn A Texture
		ID3D11Texture2D* BarnATexture = nullptr;
		ID3D11Texture2D* BarnANormalTexture = nullptr;

		ID3D11ShaderResourceView* BarnA_SRV = nullptr;
		ID3D11ShaderResourceView* BarnANormal_SRV = nullptr;

		//SkyBox Texture
		ID3D11Texture2D* SkyboxTexture = nullptr;
		ID3D11ShaderResourceView* SkyboxTexture_SRV = nullptr;

		//Lights
		Lights DirLight;
		Lights SpotLight;
		Lights PointLight;

		unsigned int DLight = 0;
		unsigned int PLight = 0;
		unsigned int SLight = 0;

		LightProp LightProperties;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		Lights_constantBuffer;

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

		// Direct3D resources for Model geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	BarnAModel_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		BarnAModel_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		BarnAModel_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	BarnAModel_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	BarnAModel_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		BarnAModel_constantBuffer;

		// System resources for Model geometry.
		uint32	BarnAModel_indexCount;
		ObjectData BarnAModel;
		bool BarnALoaded;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		bool	loadingcomplete;
		bool	tloadingcomplete;
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

