#include "InputLayoutD3D11.h"
#include <iostream>

InputLayoutD3D11::~InputLayoutD3D11()
{
	if (this->inputLayout != nullptr)
	{
		this->inputLayout->Release();
		this->inputLayout = nullptr;
	}
}

void InputLayoutD3D11::AddInputElement(const std::string& semanticName, DXGI_FORMAT format)
{
	// Add the semantic name to the list of semantic names
	this->semanticNames.push_back(semanticName);

	D3D11_INPUT_ELEMENT_DESC elementDesc;
	elementDesc.SemanticName = semanticName.c_str();
	elementDesc.SemanticIndex = 0;
	elementDesc.Format = format;
	elementDesc.InputSlot = 0;
	elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	elementDesc.InstanceDataStepRate = 0;


	this->elements.push_back(elementDesc);
}

void InputLayoutD3D11::FinalizeInputLayout(ID3D11Device* device, const void* vsDataPtr, size_t vsDataSize)
{
    HRESULT hr = device->CreateInputLayout(this->elements.data(), static_cast<UINT>(this->elements.size()), vsDataPtr, static_cast<UINT>(vsDataSize), &this->inputLayout);
    if (FAILED(hr))
    {
        std::cerr << "InputLayoutD3D11::FinalizeInputLayout() failed to create input layout" << std::endl;
    }
}

ID3D11InputLayout* InputLayoutD3D11::GetInputLayout() const
{
	return this->inputLayout;
}
