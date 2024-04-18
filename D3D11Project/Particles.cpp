#include "Particles.h"

Particles::Particles(ID3D11Device* device, UINT nrOfParticles)
{
	Initialize(device, nrOfParticles);
}

void Particles::Initialize(ID3D11Device* device, UINT nrOfParticles)
{
	Particle p;
	for (int i = 0; i < nrOfParticles; i++)
	{
		// randomize x and z position between -100 and 100 and y position between 0 and 40
		float xPos = (rand() % 200) - 100.0f;
		float zPos = (rand() % 200) - 100.0f;

		// randomize y position between 0 and 40
		float yStartPos = 150;
		float yPos = rand() % (int)yStartPos;

		p.position = DirectX::XMFLOAT3(xPos, yPos, zPos);
		p.startPosition = DirectX::XMFLOAT3(xPos, yStartPos, zPos);
		
		this->particles.push_back(p);
	}

	this->particleBuffer.Initialize(device, sizeof(Particle), this->particles.size(), this->particles.data(), false, true, true);
}

void Particles::Update(ID3D11DeviceContext* immediateContext, ShaderD3D11* cShaderParticles)
{
	cShaderParticles->BindShader(immediateContext);

	ID3D11UnorderedAccessView* uav = this->particleBuffer.GetUAV();
	immediateContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
	immediateContext->Dispatch(std::ceil(particleBuffer.GetNrOfElements() / 32.0f), 1, 1);


	ID3D11UnorderedAccessView* nullUAV = nullptr;
	immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
}

ID3D11ShaderResourceView* Particles::GetParticleBufferSRV() const
{
	return this->particleBuffer.GetSRV();
}

ID3D11UnorderedAccessView* Particles::GetParticleBufferUAV() const
{
	return this->particleBuffer.GetUAV();
}

UINT Particles::GetNrOfParticles() const
{
	return this->particleBuffer.GetNrOfElements();
}