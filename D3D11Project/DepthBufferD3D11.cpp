#include "DepthBufferD3D11.h"

DepthBufferD3D11::DepthBufferD3D11(ID3D11Device* device, UINT width, UINT height, bool hasSRV)
{
	Initialize(device, width, height, hasSRV);
}

DepthBufferD3D11::~DepthBufferD3D11()
{
	if (this->texture != nullptr)
	{
		this->texture->Release();
		this->texture = nullptr;
	}

	for (int i = 0; i < this->depthStencilViews.size(); i++)
	{
		this->depthStencilViews[i]->Release();
		this->depthStencilViews[i] = nullptr;
	}

	if (this->srv != nullptr)
	{
		this->srv->Release();
		this->srv = nullptr;
	}
}

void DepthBufferD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, bool hasSRV, UINT arraysize)
{
	// ------------- creating texture -------------
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = arraysize;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	if (hasSRV)
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	else
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &this->texture);
	if (FAILED(hr))
	{
		std::cerr << "DepthBufferD3D11::Initialize() failed to create texture" << std::endl;
		return;
	}

	// ------------- creating depth stencil view -------------

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	if (arraysize == 1)
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	else
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 1;

	for (int i = 0; i < arraysize; i++)
	{
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		ID3D11DepthStencilView* dsv;
		hr = device->CreateDepthStencilView(this->texture, &dsvDesc, &dsv);
		if (FAILED(hr))
		{
			std::cerr << "DepthBufferD3D11::Initialize() failed to create depth stencil view" << i << std::endl;
		}
		this->depthStencilViews.push_back(dsv);
	}

	// ------------- creating shader resource view -------------

	if (hasSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		if (arraysize == 1)
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		else
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = arraysize;


		hr = device->CreateShaderResourceView(this->texture, &srvDesc, &this->srv);
		if (FAILED(hr))
		{
			std::cerr << "DepthBufferD3D11::Initialize() failed to create shader resource view" << std::endl;
		}
	}
}

ID3D11DepthStencilView* DepthBufferD3D11::GetDSV(UINT arrayIndex) const
{
	return this->depthStencilViews[arrayIndex];
}

ID3D11ShaderResourceView* DepthBufferD3D11::GetSRV() const
{
	return this->srv;
}
