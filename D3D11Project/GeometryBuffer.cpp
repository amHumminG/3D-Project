#include "GeometryBuffer.h"

GeometryBufferD3D11::GeometryBufferD3D11(ID3D11Device* device, UINT windowWidth, UINT windowHeight)
{
	Initialize(device, windowWidth, windowHeight);
}

GeometryBufferD3D11::~GeometryBufferD3D11()
{
	if (this->texture)
	{
		this->texture->Release();
		this->texture = nullptr;
	}

	if (this->srv)
	{
		this->srv->Release();
		this->srv = nullptr;
	}

	if (this->rtv)
	{
		this->rtv->Release();
		this->rtv = nullptr;
	}
}

void GeometryBufferD3D11::Initialize(ID3D11Device* device, UINT windowWidth, UINT windowHeight)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = windowWidth;
	desc.Height = windowHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &this->texture);
	if (FAILED(hr))
	{
		std::cerr << "GeometryBufferD3D11: failed to create texture" << std::endl;
		return;
	}

	hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
	if (FAILED(hr))
	{
		std::cerr << "GeometryBufferD3D11: failed to create shader resource view" << std::endl;
	}

	hr = device->CreateRenderTargetView(this->texture, nullptr, &this->rtv);
	if (FAILED(hr))
	{
		std::cerr << "GeometryBufferD3D11: failed to create render target view" << std::endl;
	}
}

// getters

ID3D11ShaderResourceView* GeometryBufferD3D11::GetSRV() const
{
	return this->srv;
}

ID3D11RenderTargetView* GeometryBufferD3D11::GetRTV() const
{
	return this->rtv;
}
