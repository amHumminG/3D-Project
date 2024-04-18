#include "RenderTargetD3D11.h"

RenderTargetD3D11::~RenderTargetD3D11()
{
	if (this->texture != nullptr)
	{
		this->texture->Release();
		this->texture = nullptr;
	}
	if (this->rtv != nullptr)
	{
		this->rtv->Release();
		this->rtv = nullptr;
	}
	if (this->srv != nullptr)
	{
		this->srv->Release();
		this->srv = nullptr;
	}
}

void RenderTargetD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, 
	DXGI_FORMAT format, bool hasSRV)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	device->CreateTexture2D(&desc, nullptr, &this->texture);
	device->CreateRenderTargetView(this->texture, nullptr, &this->rtv);

	if (hasSRV)
	{
		device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
	}
}

ID3D11RenderTargetView* RenderTargetD3D11::GetRTV() const
{
	return this->rtv;
}

ID3D11ShaderResourceView* RenderTargetD3D11::GetSRV() const
{
	return this->srv;
}
