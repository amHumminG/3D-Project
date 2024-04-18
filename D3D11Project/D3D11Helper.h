#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <iostream>

bool SetupD3D11
(
	UINT width, UINT height, HWND window,
	ID3D11Device*& device, ID3D11DeviceContext*& immediateContext,
	IDXGISwapChain*& swapChain, ID3D11Texture2D*& backBuffer,
	ID3D11RenderTargetView*& rtv, ID3D11Texture2D*& outputTexture, ID3D11UnorderedAccessView*& outputUAV, ID3D11RenderTargetView*& outputRTV,
	ID3D11Texture2D*& dsTexture, ID3D11DepthStencilView*& dsView, D3D11_VIEWPORT& viewport, D3D11_VIEWPORT& shadowViewport, UINT shadowDimension
);