#include "ConstantBufferD3D11.h"

ConstantBufferD3D11::ConstantBufferD3D11(ID3D11Device* device, size_t byteSize, void* initialData)
{
	Initialize(device, byteSize, initialData);
}

ConstantBufferD3D11::~ConstantBufferD3D11()
{
	if (buffer != nullptr)
	{
		buffer->Release();
		buffer = nullptr;
	}
}

ConstantBufferD3D11::ConstantBufferD3D11(ConstantBufferD3D11&& other) noexcept
{
	buffer = other.buffer;
	bufferSize = other.bufferSize;

	other.buffer = nullptr;
	other.bufferSize = 0;
}

ConstantBufferD3D11& ConstantBufferD3D11::operator=(ConstantBufferD3D11&& other) noexcept
{
	if (this != &other)
	{
		if (buffer != nullptr)
		{
			buffer->Release();
		}

		buffer = other.buffer;
		bufferSize = other.bufferSize;

		other.buffer = nullptr;
		other.bufferSize = 0;
	}

	return *this;
}

bool ConstantBufferD3D11::Initialize(ID3D11Device* device, size_t byteSize, void* initialData)
{
	if (buffer != nullptr)
	{
		buffer->Release();
		buffer = nullptr;
	}

	bufferSize = byteSize;

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = static_cast<UINT>(byteSize);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	if (initialData != nullptr)
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = initialData;
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&desc, &data, &buffer);

		if (FAILED(hr))
		{
			std::cerr << "Could not create constant buffer" << std::endl;
			return false;
		}
	}
	else
	{
		HRESULT hr = device->CreateBuffer(&desc, nullptr, &buffer);

		if (FAILED(hr))
		{
			std::cerr << "Could not create constant buffer" << std::endl;
			return false;
		}
	}

	return true;
}

size_t ConstantBufferD3D11::GetSize() const
{
	return bufferSize;
}

ID3D11Buffer* ConstantBufferD3D11::GetBuffer() const
{
	return buffer;
}

bool ConstantBufferD3D11::UpdateBuffer(ID3D11DeviceContext* immediateContext, void* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	HRESULT hr = immediateContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		std::cerr << "Constant buffer::UpdateBuffer() failed to map buffer" << std::endl;
		std::cerr << "Could not map constant buffer" << std::endl;
		return false;
	}
	memcpy(mappedResource.pData, data, bufferSize);
	immediateContext->Unmap(buffer, 0);

	return true;
}