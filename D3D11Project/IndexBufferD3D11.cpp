#include "indexbufferd3d11.h"
#include <iostream>

IndexBufferD3D11::IndexBufferD3D11(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
{
	Initialize(device, nrOfIndicesInBuffer, indexData);
}

IndexBufferD3D11::~IndexBufferD3D11()
{
	if (this->buffer != nullptr)
	{
		buffer->Release();
		buffer = nullptr;
	}
}

bool IndexBufferD3D11::Initialize(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
{
	nrOfIndices = nrOfIndicesInBuffer;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(uint32_t) * nrOfIndices;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = indexData;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, &this->buffer);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create index buffer" << std::endl;
		return false;
	}
	
	return true;
}


// ---------------- getters ----------------

size_t IndexBufferD3D11::GetNrOfIndices() const
{
	return nrOfIndices;
}

ID3D11Buffer* IndexBufferD3D11::GetBuffer() const
{
	return buffer;
}

