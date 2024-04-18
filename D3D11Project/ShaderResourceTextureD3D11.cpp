#include "ShaderResourceTextureD3D11.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ShaderResourceTextureD3D11::ShaderResourceTextureD3D11(ID3D11Device* device, UINT width, UINT height, void* textureData)
{
	Initialize(device, width, height, textureData);
}

ShaderResourceTextureD3D11::ShaderResourceTextureD3D11(ID3D11Device* device, const char* pathToTextureFile)
{
	Initialize(device, pathToTextureFile);
}

ShaderResourceTextureD3D11::~ShaderResourceTextureD3D11()
{
	if (this->texture != nullptr)
	{
		this->texture->Release();
		this->texture = nullptr;
	}
	if (this->srv != nullptr)
	{
		this->srv->Release();
		this->srv = nullptr;
	}
}

void ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, void* textureData)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = textureData;
	data.SysMemPitch = width * 4;

	HRESULT hr = device->CreateTexture2D(&desc, &data, &this->texture);
	if (FAILED(hr))
	{
		std::cerr << "ShaderResourceTexture::Initialize() Failed to create texture2D!" << std::endl;
		return;
	}

	hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
	if (FAILED(hr))
	{
		std::cerr << "ShaderResourceTexture::Initialize() Failed to create shader resource view!" << std::endl;
	}
}

void ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, const char* pathToTextureFile)
{
	int width = 0;
	int height = 0;
	int componentsPerPixel = 0;

	// loading the texture
	unsigned char* textureData = stbi_load(pathToTextureFile, &width, &height, &componentsPerPixel, 4);

	if (textureData == NULL)
	{
		std::cerr << "ShaderResourceTexture::Initialize() Failed to load textureData!" << std::endl;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = textureData;
	data.SysMemPitch = width * 4;

	HRESULT hr = device->CreateTexture2D(&desc, &data, &this->texture);
	if (FAILED(hr))
	{
		std::cerr << "ShaderResourceTexture::Initialize() Failed to create texture2D!" << std::endl;
		return;
	}

	hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
	if (FAILED(hr))
	{
		std::cerr << "ShaderResourceTexture::Initialize() Failed to create shader resource view!" << std::endl;
	}

	stbi_image_free(textureData);
}

ID3D11ShaderResourceView* ShaderResourceTextureD3D11::GetSRV() const
{
	return this->srv;
}
