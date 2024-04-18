#include "PipelineHelper.h"

#include <fstream>
#include <string>
#include <iostream>

void LoadShaders
(
	ID3D11Device* device, 
	ShaderD3D11* vShader, 
	ShaderD3D11* pShader, ShaderD3D11* pShaderDeferred, ShaderD3D11* pShader_cubeMap, 
	ShaderD3D11* cShader,
	std::string& vShaderByteCode)
{
	// -------------- Vertex Shaders ----------------
	vShader->Initialize(device, ShaderType::VERTEX_SHADER, L"VertexShader.cso");
	vShaderByteCode = std::string((char*)vShader->GetShaderByteData(), vShader->GetShaderByteSize());

	// -------------- Pixel Shaders ----------------

	pShader->Initialize(device, ShaderType::PIXEL_SHADER, L"PixelShader.cso");
	pShader_cubeMap->Initialize(device, ShaderType::PIXEL_SHADER, L"PixelShaderCubeMap.cso");
	pShaderDeferred->Initialize(device, ShaderType::PIXEL_SHADER, L"PixelShaderDeferred.cso");

	// -------------- Compute Shaders ----------------
	cShader->Initialize(device, ShaderType::COMPUTE_SHADER, L"ComputeShader.cso");
}

void CreateInputLayout(ID3D11Device* device, InputLayoutD3D11* inputLayout, const std::string& vShaderByteCode)
{
    std::string inputNames[3] = {"POSITION", "NORMAL", "UV"};

	inputLayout->AddInputElement(inputNames[0], DXGI_FORMAT_R32G32B32_FLOAT);
	inputLayout->AddInputElement(inputNames[1], DXGI_FORMAT_R32G32B32_FLOAT);
	inputLayout->AddInputElement(inputNames[2], DXGI_FORMAT_R32G32_FLOAT);
	inputLayout->FinalizeInputLayout(device, vShaderByteCode.c_str(), vShaderByteCode.length());
}

bool SetupPipeline(ID3D11Device* device, 
	ShaderD3D11* vShader, 
	ShaderD3D11* pShader, ShaderD3D11* pShaderDeferred, ShaderD3D11* pShader_cubeMap,
	ShaderD3D11* cShader, 
	InputLayoutD3D11* inputLayout)
{
	std::string vShaderByteCode;
	LoadShaders(device, vShader, pShader, pShaderDeferred, pShader_cubeMap, cShader, vShaderByteCode);
	CreateInputLayout(device, inputLayout, vShaderByteCode);

	return true;
}