#pragma once

#include <d3d11_4.h>
#include <iostream>

class GeometryBufferD3D11
{
private:
	ID3D11Texture2D* texture = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;                            
	ID3D11RenderTargetView* rtv = nullptr;

public:
	GeometryBufferD3D11() = default;
	GeometryBufferD3D11(ID3D11Device* device, UINT width, UINT height);
	~GeometryBufferD3D11();
	GeometryBufferD3D11(const GeometryBufferD3D11& other) = delete;
	GeometryBufferD3D11& operator=(const GeometryBufferD3D11& other) = delete;
	GeometryBufferD3D11(GeometryBufferD3D11&& other) = delete;
	GeometryBufferD3D11& operator=(GeometryBufferD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, UINT width, UINT height);

	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11RenderTargetView* GetRTV() const;

	// create a clear function
};