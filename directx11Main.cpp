#include "pch.h"
#include "directx11Main.h"
#include "Common\DirectXHelper.h"

using namespace directx11;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// 응용 프로그램이 로드되면 응용 프로그램 자산을 로드하고 초기화합니다.
directx11Main::directx11Main(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	Initialize(1600,900,true,true,0.0f,0.0f,deviceResources);

	// 장치가 손실되었거나 다시 만들어지는 경우 알림을 등록합니다.
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: 이 항목을 앱 콘텐츠 초기화로 대체합니다.
	
	
	// TODO: 기본 가변 timestep 모드 외에 다른 설정을 하려면 타이머 설정을 변경합니다.
	// 예: 60FPS 고정 timestep 업데이트 논리일 경우 다음을 호출합니다.
	
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	
}
bool directx11Main::Initialize(int screenWidth, int screenHeight, bool vsync, bool fullscreen, float screenDepth, float screenNear, const std::shared_ptr<DX::DeviceResources>& deviceResources)
{

	m_videoCardMemory = 0;//메모리 넣기
	m_videoCardDescription[0] = '\0';//설명 넣기
	m_swapChain = nullptr;
	m_device = nullptr;
	m_deviceContext = nullptr;
	m_renderTargetView = nullptr;
	m_depthStencilBuffer = nullptr;
	m_depthStencilState = nullptr;
	m_depthStencilView = nullptr;
	m_rasterState = nullptr;
	m_projectionMatrix = DirectX::XMFLOAT4X4(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	);
	m_worldMatrix = DirectX::XMFLOAT4X4(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	);;
	m_orthoMatrix = DirectX::XMFLOAT4X4(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	);

	HRESULT result = 0;
	IDXGIFactory* factory = nullptr;
	IDXGIAdapter* adapter = nullptr;
	IDXGIOutput* adapterOutput = nullptr;
	unsigned int numModes, i, numerator, denominator, stringLength;
	DXGI_MODE_DESC* displayModeList = nullptr;
	DXGI_ADAPTER_DESC adapterDesc;
	int error = 0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr = nullptr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	//vsync( 수직동기화) 설정 저장
	m_vsync_enabled = vsync;

	// DirectX 그래픽 인터페이스 팩토리를 만듭니다.

	result = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**) factory);
	if (FAILED(result))
	{
		return false;

	}
	// 팩토리 객체를 사용하여 첫번째 그래픽 카드 인터페이스에 대한 아답터를 만듭니다.
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}
	// 출력(모니터)에 대한 첫번째 아답터를 나열합니다.
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}
	// DXGI_FORMAT_R8G8B8A8_UNORM 모니터 출력 디스플레이 포맷에 맞는 모드의 개수를 구합니다.

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}
	// 가능한 모든 모니터와 그래픽카드 조합을 저장할 리스트를 생성합니다
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}
	// 이제 모든 디스플레이 모드에 대해 화면 너비/높이에 맞는 디스플레이 모드를 찾습니다. 
	// 적합한 것을 찾으면 모니터의 새로고침 비율의 분모와 분자 값을 저장합니다.

	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth);
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight);
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}
	// 어댑터(그래픽카드)의 description을 가져옵니다.

	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}
	// 현재 그래픽카드의 메모리 용량을 메가바이트 단위로 저장합니다. 
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024); 
	// 그래픽카드의 이름을 char형 문자열 배열로 바꾼 뒤 저장합니다.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0) { return false; }
	// 디스플레이 모드 리스트의 할당을 해제합니다.
	delete[] displayModeList;
	displayModeList = 0;
	// 출력 아답터를 할당 해제합니다.
	adapterOutput->Release();
	adapter = 0;
	// 아답터를 할당 해제합니다.
	adapter->Release();
	adapter = 0;
	// 팩토리 객체를 할당 해제합니다.
	factory->Release();
	factory = 0;
	// 스왑 체인 description을 초기화합니다.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	// 하나의 백버퍼만을 사용하도록 합니다.
	swapChainDesc.BufferCount = 1;
	// 백버퍼의 너비와 높이를 설정합니다.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	// 백버퍼로 일반적인 32bit의 서페이스를 지정합니다.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// 백버퍼의 새로고침 비율을 설정합니다.
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	// 백버퍼의 용도를 설정합니다.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// 렌더링이 이루어질 윈도우의 핸들을 설정합니다.
	swapChainDesc.OutputWindow = deviceResources->GetSwapChain()->GetHwnd;
	// 멀티샘플링을 끕니다.
	// 윈도우 모드 또는 풀스크린 모드를 설정합니다.
	// 스캔라인의 정렬과 스캔라이닝을 지정되지 않음으로(unspecified) 설정합니다.
	// 출력된 이후의 백버퍼의 내용을 버리도록 합니다.
	// 추가 옵션 플래그를 사용하지 않습니다.

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	
	result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	
	if (FAILED(result))
	{
		return false;
	}

	// Get the pointer to the back buffer.
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState);

	// Setup the viewport for rendering.
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &viewport);

	// Setup the projection matrix.
	fieldOfView = (float)XM_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	XMStoreFloat4x4(&m_projectionMatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth));

	// Initialize the world matrix to the identity matrix.
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());

	// Create an orthographic projection matrix for 2D rendering.
	XMStoreFloat4x4(&m_orthoMatrix, XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth));

	return true;
}
directx11Main::~directx11Main()
{
	// 장치 알림 등록 취소
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// 창 크기가 변경되면 응용 프로그램 상태를 업데이트합니다(예: 장치 방향 변경).
void directx11Main::CreateWindowSizeDependentResources() 
{
	// TODO: 이 항목을 앱 콘텐츠의 크기 종속 초기화로 대체합니다.
	
}

// 프레임당 한 번 응용 프로그램 상태를 업데이트합니다.
void directx11Main::Update() 
{
	
	
	//// 장면 개체를 업데이트합니다.
	//m_timer.Tick([&]()
	//{
	//	// TODO: 이 항목을 앱 콘텐츠 업데이트 함수로 대체합니다.
	//	
	//});
}

// 현재 응용 프로그램 상태에 따라 현재 프레임을 렌더링합니다.
// 프레임이 렌더링되어 표시할 준비가 되면 true를 반환합니다.
bool directx11Main::Render() 
{
	// 처음 업데이트하기 전에 아무 것도 렌더링하지 마세요.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// 전체 화면을 대상으로 하도록 뷰포트를 다시 설정합니다.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// 렌더링 대상을 화면에 대해 다시 설정합니다.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// 백 버퍼 및 깊이 스텐실 뷰를 지웁니다.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 장면 개체를 렌더링합니다.
	// TODO: 이 항목을 앱 콘텐츠 렌더링 함수로 대체합니다.
	

	return true;
}

// 릴리스가 필요한 장치 리소스를 렌더러에 알립니다.
void directx11Main::OnDeviceLost()
{
	
}

// 장치 리소스가 이제 다시 만들어질 수 있음을 렌더러에 알립니다.
void directx11Main::OnDeviceRestored()
{

	CreateWindowSizeDependentResources();
}
