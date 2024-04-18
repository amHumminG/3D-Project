#include "SamplerD3D11.h"

SamplerD3D11::SamplerD3D11(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode, std::optional<std::array<float, 4>> borderColour)
{
	Initialize(device, adressMode, borderColour);
}

SamplerD3D11::~SamplerD3D11()
{
	if (this->sampler != nullptr)
	{
		this->sampler->Release();
		this->sampler = nullptr;
	}
}

void SamplerD3D11::Initialize(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode, std::optional<std::array<float, 4>> borderColour)
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = adressMode;
	desc.AddressV = adressMode;
	desc.AddressW = adressMode;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	if (borderColour.has_value())
	{
		desc.BorderColor[0] = borderColour.value()[0];
		desc.BorderColor[1] = borderColour.value()[1];
		desc.BorderColor[2] = borderColour.value()[2];
		desc.BorderColor[3] = borderColour.value()[3];
	}

	HRESULT hr = device->CreateSamplerState(&desc, &this->sampler);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create sampler state!" << std::endl;
	}
}

ID3D11SamplerState* SamplerD3D11::GetSamplerState() const
{
	return this->sampler;
}


