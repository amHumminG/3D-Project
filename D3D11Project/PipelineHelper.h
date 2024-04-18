#pragma once

#include "InputLayoutD3D11.h"
#include "shaderD3D11.h"

#include <array>
#include <d3d11.h>


bool SetupPipeline(ID3D11Device* device,
	ShaderD3D11* vShader,
	ShaderD3D11* pShader, ShaderD3D11* pShaderDeferred, ShaderD3D11* pShader_cubeMap,
	ShaderD3D11* cShader,
	InputLayoutD3D11* inputLayout);
