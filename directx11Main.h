#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include "MoveLookController.h"

// 화면의 Direct2D 및 3D 콘텐츠를 렌더링합니다.
namespace directx11
{
	class directx11Main : public DX::IDeviceNotify
	{
	public:
		directx11Main(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~directx11Main();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		void Grid_KeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);

		void Grid_KeyUp(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e);

	private:
		// 장치 리소스에 대한 캐시된 포인터입니다.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: 사용자 콘텐츠 렌더러로 대체합니다.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		// 렌더링 루프 타이머입니다.
		DX::StepTimer m_timer;

		InputController^ m_controller;
	};
}