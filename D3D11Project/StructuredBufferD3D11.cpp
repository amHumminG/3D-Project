#include "StructuredBufferD3D11.h"

StructuredBufferD3D11::StructuredBufferD3D11(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer, void* bufferData, bool dynamic, bool hasSRV, bool hasUAV)
{
	Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic, hasSRV, hasUAV);
}

StructuredBufferD3D11::~StructuredBufferD3D11()
{
	if (this->buffer)
	{
		this->buffer->Release();
		this->buffer = nullptr;
	}
}

void StructuredBufferD3D11::Initialize(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer, void* bufferData, bool dynamic, bool hasSRV, bool hasUAV)
{
	this->elementSize = sizeOfElement;
	this->nrOfElements = nrOfElementsInBuffer;

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = sizeOfElement * nrOfElementsInBuffer;
	bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT; // cookbook uses D3D11_USAGE_IMMUTABLE for some reason?
	bufferDesc.BindFlags = hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0;
	bufferDesc.BindFlags |= hasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0;
	bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeOfElement;

	HRESULT hr = S_OK;

	if (bufferData != nullptr)
	{
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = bufferData;

		hr = device->CreateBuffer(&bufferDesc, &initData, &this->buffer);
	}
	else
	{
		hr = device->CreateBuffer(&bufferDesc, nullptr, &this->buffer);
	}

	if (FAILED(hr))
	{
		std::cerr << "Structured buffer::Initialize() failed to create buffer" << std::endl;
		std::cerr << "Error code: " << hr << std::endl;
		return;
	}


	if (hasSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.ElementWidth = nrOfElementsInBuffer;

		hr = device->CreateShaderResourceView(this->buffer, &srvDesc, &this->srv);
		if (FAILED(hr))
		{
			std::cerr << "Structured buffer::Initialize() failed to create shader resource view" << std::endl;
		}
	}

	if (hasUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = nrOfElementsInBuffer;	// maybe take times 3 for xyz?	
		uavDesc.Buffer.Flags = 0;

		hr = device->CreateUnorderedAccessView(this->buffer, &uavDesc, &this->uav);
		if (FAILED(hr))
		{
			std::cerr << "Structured buffer::Initialize() failed to create unordered access view" << std::endl;
		}
	}
}

void StructuredBufferD3D11::UpdateBuffer(ID3D11DeviceContext* context, void* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HRESULT hr = context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		std::cerr << "Structured buffer::UpdateBuffer() failed to map buffer" << std::endl;
	}

	size_t bufferSize = this->elementSize * this->nrOfElements;
	memcpy(mappedResource.pData, data, bufferSize);
	context->Unmap(buffer, 0);
}

UINT StructuredBufferD3D11::GetElementSize() const
{
	return this->elementSize;
}

size_t StructuredBufferD3D11::GetNrOfElements() const
{
	return this->nrOfElements;
}

ID3D11ShaderResourceView* StructuredBufferD3D11::GetSRV() const
{
	return this->srv;
}

ID3D11UnorderedAccessView* StructuredBufferD3D11::GetUAV() const
{
	return this->uav;
}