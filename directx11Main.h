#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Controller.h"
#include "..\Common\DirectXHelper.h"
#include <DirectXMath.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")



//////////////
// INCLUDES //
//////////////
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
using namespace DirectX;

// 화면의 Direct2D 및 3D 콘텐츠를 렌더링합니다.
namespace directx11
{
	class directx11Main : public DX::IDeviceNotify
	{
	public:
		directx11Main(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		bool Initialize(int screenWidth, int screenHeight, bool vsync, bool fullscreen, float screenDepth, float screenNear, const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~directx11Main();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		

	private:
		// 장치 리소스에 대한 캐시된 포인터입니다.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: 사용자 콘텐츠 렌더러로 대체합니다.

		// 렌더링 루프 타이머입니다.
		DX::StepTimer m_timer;


		bool m_vsync_enabled;//수직동기화
		int m_videoCardMemory;
		char m_videoCardDescription[256];
		IDXGISwapChain* m_swapChain;
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;
		ID3D11RenderTargetView* m_renderTargetView;
		ID3D11Texture2D* m_depthStencilBuffer;
		ID3D11DepthStencilState* m_depthStencilState;//깊이값
		ID3D11DepthStencilView* m_depthStencilView;//깊이값
		ID3D11RasterizerState* m_rasterState;
		XMFLOAT4X4 m_projectionMatrix;
		XMFLOAT4X4 m_worldMatrix;
		XMFLOAT4X4 m_orthoMatrix;


	};
}