#include "ShaderD3D11.h"

#include <d3dcompiler.h>
#include <fstream>
ShaderD3D11::~ShaderD3D11()
{
	if (this->shaderBlob != nullptr)
	{
		this->shaderBlob->Release();
		this->shaderBlob = nullptr;
	}
	switch (this->type)
	{
	case ShaderType::VERTEX_SHADER:
		if (this->shader.vertex != nullptr)
		{
			this->shader.vertex->Release();
			this->shader.vertex = nullptr;
		}
		break;

	case ShaderType::HULL_SHADER:
		if (this->shader.hull != nullptr)
		{
			this->shader.hull->Release();
			this->shader.hull = nullptr;
		}
		break;

	case ShaderType::DOMAIN_SHADER:
		if (this->shader.domain != nullptr)
		{
			this->shader.domain->Release();
			this->shader.domain = nullptr;
		}
		break;

	case ShaderType::GEOMETRY_SHADER:
		if (this->shader.geometry != nullptr)
		{
			this->shader.geometry->Release();
			this->shader.geometry = nullptr;
		}
		break;

	case ShaderType::PIXEL_SHADER:
		if (this->shader.pixel != nullptr)
		{
			this->shader.pixel->Release();
			this->shader.pixel = nullptr;
		}
		break;

	case ShaderType::COMPUTE_SHADER:
		if (this->shader.compute != nullptr)
		{
			this->shader.compute->Release();
			this->shader.compute = nullptr;
		}
		break;
	}
}

ShaderD3D11::ShaderD3D11(ID3D11Device* device, ShaderType shaderType, const void* dataPtr, size_t dataSize)
{
	Initialize(device, shaderType, dataPtr, dataSize);
}

ShaderD3D11::ShaderD3D11(ID3D11Device* device, ShaderType shaderType, LPCWSTR csoPath)
{
	Initialize(device, shaderType, csoPath);
}

void ShaderD3D11::Initialize(ID3D11Device* device, ShaderType shaderType, const void* dataPtr, size_t dataSize)
{
	this->type = shaderType;
	HRESULT hr = S_OK;
	switch (this->type)
	{
	case ShaderType::VERTEX_SHADER:
		hr = device->CreateVertexShader(dataPtr, dataSize, nullptr, &this->shader.vertex);
		break;

	case ShaderType::HULL_SHADER:
		hr = device->CreateHullShader(dataPtr, dataSize, nullptr, &this->shader.hull);
		break;

	case ShaderType::DOMAIN_SHADER:
		hr = device->CreateDomainShader(dataPtr, dataSize, nullptr, &this->shader.domain);
		break;

	case ShaderType::GEOMETRY_SHADER:
		hr = device->CreateGeometryShader(dataPtr, dataSize, nullptr, &this->shader.geometry);
		break;

	case ShaderType::PIXEL_SHADER:
		hr = device->CreatePixelShader(dataPtr, dataSize, nullptr, &this->shader.pixel);
		break;

	case ShaderType::COMPUTE_SHADER:
		hr = device->CreateComputeShader(dataPtr, dataSize, nullptr, &this->shader.compute);
		break;
	}
	if (FAILED(hr))
	{
		std::cerr << "Failed to create shader!" << std::endl;
	}
}

void ShaderD3D11::Initialize(ID3D11Device* device, ShaderType shaderType, LPCWSTR csoPath)
{
	this->type = shaderType;
	HRESULT hr = S_OK;

	hr = D3DReadFileToBlob(csoPath, &this->shaderBlob);
	if (FAILED(hr))
	{
		std::cerr << "Failed to read file to blob!" << std::endl;
	}

	switch (this->type)
	{
	case ShaderType::VERTEX_SHADER:
		hr = device->CreateVertexShader(this->shaderBlob->GetBufferPointer(), this->shaderBlob->GetBufferSize(), nullptr, &this->shader.vertex);
		break;

	case ShaderType::HULL_SHADER:
		hr = device->CreateHullShader(this->shaderBlob->GetBufferPointer(), this->shaderBlob->GetBufferSize(), nullptr, &this->shader.hull);
		break;

	case ShaderType::DOMAIN_SHADER:
		hr = device->CreateDomainShader(this->shaderBlob->GetBufferPointer(), this->shaderBlob->GetBufferSize(), nullptr, &this->shader.domain);
		break;

	case ShaderType::GEOMETRY_SHADER:
		hr = device->CreateGeometryShader(this->shaderBlob->GetBufferPointer(), this->shaderBlob->GetBufferSize(), nullptr, &this->shader.geometry);
		break;

	case ShaderType::PIXEL_SHADER:
		hr = device->CreatePixelShader(this->shaderBlob->GetBufferPointer(), this->shaderBlob->GetBufferSize(), nullptr, &this->shader.pixel);
		break;

	case ShaderType::COMPUTE_SHADER:
		hr = device->CreateComputeShader(this->shaderBlob->GetBufferPointer(), this->shaderBlob->GetBufferSize(), nullptr, &this->shader.compute);
		break;
	}
}

const void* ShaderD3D11::GetShaderByteData() const
{
	return this->shaderBlob->GetBufferPointer();
}

size_t ShaderD3D11::GetShaderByteSize() const
{
	return this->shaderBlob->GetBufferSize();
}

void ShaderD3D11::BindShader(ID3D11DeviceContext* context) const
{
	switch (this->type)
	{
	case ShaderType::VERTEX_SHADER:
		context->VSSetShader(this->shader.vertex, nullptr, 0);
		break;

	case ShaderType::HULL_SHADER:
		context->HSSetShader(this->shader.hull, nullptr, 0);
		break;

	case ShaderType::DOMAIN_SHADER:
		context->DSSetShader(this->shader.domain, nullptr, 0);
		break;

	case ShaderType::GEOMETRY_SHADER:
		context->GSSetShader(this->shader.geometry, nullptr, 0);
		break;

	case ShaderType::PIXEL_SHADER:
		context->PSSetShader(this->shader.pixel, nullptr, 0);
		break;

	case ShaderType::COMPUTE_SHADER:
		context->CSSetShader(this->shader.compute, nullptr, 0);
		break;
	}
}

