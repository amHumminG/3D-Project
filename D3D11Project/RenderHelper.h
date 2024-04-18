#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include "ObjectHelper.h"
#include "InputLayoutD3D11.h"
#include "ShaderD3D11.h"
#include "Scene.h"
#include "SpotLightCollectionD3D11.h"
#include "GeometryBuffer.h"

#include "Particles.h"



void Render
(
	ID3D11DeviceContext* immediateContext,
	ID3D11RasterizerState* rsState,
	Scene* scene,

	// Output
	ID3D11Texture2D* outputTexture, ID3D11Texture2D* backBuffer,
	ID3D11UnorderedAccessView* outputUAV, ID3D11RenderTargetView* outputRTV,

	// Gbuffers
	std::vector<GeometryBufferD3D11*>Gbuffers, std::vector<GeometryBufferD3D11*> GbuffersCubeMap,

	// Viewports and depth buffer
	ID3D11DepthStencilView* dsView, D3D11_VIEWPORT& viewport, D3D11_VIEWPORT& shadowViewport,

	// Shaders
	ShaderD3D11* vShader, InputLayoutD3D11* inputLayout,
	ShaderD3D11* hShaderLOD, ShaderD3D11* dShaderLOD,
	ShaderD3D11* pShaderDeferred, ShaderD3D11* pShaderCubeMap,
	ShaderD3D11* cShader, std::vector<UINT> dispatchThreads,

	ShaderD3D11* vShaderParticles, ShaderD3D11* pShaderParticles,
	ShaderD3D11* gShaderParticles, ShaderD3D11* cShaderParticles,

	ShaderD3D11* vShaderBB, ShaderD3D11* pShaderBB,

	// Samplers
	SamplerD3D11* sampler, SamplerD3D11* shadowSampler
);