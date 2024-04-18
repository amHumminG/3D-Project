#include "D3D11Helper.h"

bool CreateInterfaces(ID3D11Device*& device, ID3D11DeviceContext*& immediateContext, IDXGISwapChain*& swapChain, UINT width, UINT height, HWND window)
{
	UINT flags = 0;
	if (_DEBUG)
		flags = D3D11_CREATE_DEVICE_DEBUG;

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Default
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &immediateContext);

	return !(FAILED(hr));
}

bool CreateRtvAndGetBackBuffer(ID3D11Device* device, IDXGISwapChain* swapChain, ID3D11Texture2D*& backBuffer, 
	ID3D11RenderTargetView*& rtv)
{
	// get the address of the back buffer
	if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
	{
		std::cerr << "Failed to get back buffer!" << std::endl;
		return false;
	}

	// use the back buffer address to create the render target
	// null as description to base it on the backbuffers values
	HRESULT hr = device->CreateRenderTargetView(backBuffer, NULL, &rtv);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create render target view!" << std::endl;
		return false;
	}

	return true;
}

bool CreateOutputTextureAndUav
(
	ID3D11Device* device, 
	ID3D11Texture2D*& outputTexture, ID3D11UnorderedAccessView*& outputUAV, ID3D11RenderTargetView*& outputRTV,
	UINT width, UINT height
)
{	
	D3D11_TEXTURE2D_DESC outputTextureDesc;
	outputTextureDesc.Width = width;
	outputTextureDesc.Height = height;
	outputTextureDesc.MipLevels = 1;
	outputTextureDesc.ArraySize = 1;
	outputTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	outputTextureDesc.SampleDesc.Count = 1;
	outputTextureDesc.SampleDesc.Quality = 0;
	outputTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	outputTextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	outputTextureDesc.CPUAccessFlags = 0;
	outputTextureDesc.MiscFlags = 0;
	HRESULT hr = device->CreateTexture2D(&outputTextureDesc, nullptr, &outputTexture);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create output texture!" << std::endl;
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	hr = device->CreateUnorderedAccessView(outputTexture, &uavDesc, &outputUAV);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create output UAV!" << std::endl;
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateRenderTargetView(outputTexture, &rtvDesc, &outputRTV);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create output RTV!" << std::endl;
		return false;
	}

	return true;
}

bool CreateDepthStencil(ID3D11Device* device, UINT width, UINT height, ID3D11Texture2D*& dsTexture, ID3D11DepthStencilView*& dsView)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &dsTexture)))
	{
		std::cerr << "Failed to create depth stencil texture!" << std::endl;
		return false;
	}

	HRESULT hr = device->CreateDepthStencilView(dsTexture, 0, &dsView);
	return !(FAILED(hr));
}

void SetViewport(D3D11_VIEWPORT& viewport, UINT width, UINT height)
{
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

bool SetupD3D11
(
	UINT width, UINT height, HWND window, 
	ID3D11Device*& device, ID3D11DeviceContext*& immediateContext, 
	IDXGISwapChain*& swapChain, ID3D11Texture2D*& backBuffer,
	ID3D11RenderTargetView*& rtv, 
	ID3D11Texture2D*& outputTexture, ID3D11UnorderedAccessView*& outputUAV, ID3D11RenderTargetView*& outputRTV,
	ID3D11Texture2D*& dsTexture, ID3D11DepthStencilView*& dsView, D3D11_VIEWPORT& viewport, D3D11_VIEWPORT& shadowViewport, UINT shadowDimension
)
{
	if (!CreateInterfaces(device, immediateContext, swapChain, width, height, window))
	{
		std::cerr << "Error creating interfaces!" << std::endl;
		return false;
	}

	if (!CreateRtvAndGetBackBuffer(device, swapChain, backBuffer, rtv))
	{
		std::cerr << "Error creating RTV and getting back buffer!" << std::endl;
		return false;
	}

	if (!CreateOutputTextureAndUav(device, outputTexture, outputUAV, outputRTV, width, height))
	{
		std::cerr << "Error creating output texture, UAV and RTV!" << std::endl;
		return false;
	}

	if (!CreateDepthStencil(device, width, height, dsTexture, dsView))
	{
		std::cerr << "Error creating depth stencil view!" << std::endl;
		return false;
	}

	SetViewport(viewport, width, height);
	SetViewport(shadowViewport, shadowDimension, shadowDimension);

	return true;
}