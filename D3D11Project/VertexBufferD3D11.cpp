#include "VertexBufferD3D11.h"

VertexBufferD3D11::VertexBufferD3D11(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData)
{
	std::cerr << "initialize vertex buffer" << std::endl;
	Initialize(device, sizeOfVertex, nrOfVerticesInBuffer, vertexData);
}

bool VertexBufferD3D11::Initialize(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData)
{
	this->nrOfVertices = nrOfVerticesInBuffer;
	this->vertexSize = sizeOfVertex;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeOfVertex * nrOfVerticesInBuffer;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = vertexData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, &buffer);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create vertex buffer" << std::endl;
		return false;
	}

	return true;
}

VertexBufferD3D11::~VertexBufferD3D11()
{
	if (this->buffer != nullptr)
	{
		buffer->Release();
		buffer = nullptr;
	}
}

// getters
UINT VertexBufferD3D11::GetNrOfVertices() const
{
	return nrOfVertices;
}

UINT VertexBufferD3D11::GetVertexSize() const
{
	return vertexSize;
}

ID3D11Buffer* VertexBufferD3D11::GetBuffer() const
{
	return buffer;
}


