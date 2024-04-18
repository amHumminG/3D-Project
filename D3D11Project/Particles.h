#pragma once

#include <vector>
#include <DirectXMath.h>
#include <d3d11.h>

#include "StructuredBufferD3D11.h"
#include "ShaderD3D11.h"

struct Particle
{
	DirectX::XMFLOAT3 position;
	float padding1;
	DirectX::XMFLOAT3 startPosition;
	float padding2;
	//DirectX::XMFLOAT3 velocity;
	//DirectX::XMFLOAT3 color;
	//float size;
};


class Particles
{
private:

	std::vector<Particle> particles;

	StructuredBufferD3D11 particleBuffer;

public:
	
	Particles() = default;
	Particles
	(
		ID3D11Device* device,
		UINT nrOfParticles
	);
	~Particles() = default;
	Particles(const Particles& other) = delete;
	Particles& operator=(const Particles& other) = delete;
	Particles(Particles&& other) = default;
	Particles& operator=(Particles&& other) = delete;

	void Initialize(ID3D11Device* device, UINT nrOfParticles);

	ID3D11ShaderResourceView* GetParticleBufferSRV() const;
	ID3D11UnorderedAccessView* GetParticleBufferUAV() const;

	UINT GetNrOfParticles() const;

	void Update(ID3D11DeviceContext* immediateContext, ShaderD3D11* cShaderParticles);
};