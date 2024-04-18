#pragma once

#include <d3d11_4.h>
#include <iostream>

class ConstantBufferD3D11
{
private:
	ID3D11Buffer* buffer = nullptr;
	size_t bufferSize = 0;

public:
	ConstantBufferD3D11() = default;
	ConstantBufferD3D11(ID3D11Device* device, size_t byteSize, void* initialData = nullptr);
	~ConstantBufferD3D11();
	ConstantBufferD3D11(const ConstantBufferD3D11& other) = delete;
	ConstantBufferD3D11& operator=(const ConstantBufferD3D11& other) = delete;
	ConstantBufferD3D11(ConstantBufferD3D11&& other) noexcept; // Move constructor
	ConstantBufferD3D11& operator=(ConstantBufferD3D11&& other) noexcept; // Move assignment operator

	bool Initialize(ID3D11Device* device, size_t byteSize, void* initialData = nullptr);

	size_t GetSize() const;
	ID3D11Buffer* GetBuffer() const;

	bool UpdateBuffer(ID3D11DeviceContext* context, void* data);
};