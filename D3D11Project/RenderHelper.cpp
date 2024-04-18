#include "RenderHelper.h"

void ClearShaders(ID3D11DeviceContext* immediateContext)
{
	immediateContext->VSSetShader(nullptr, nullptr, 0);
	immediateContext->HSSetShader(nullptr, nullptr, 0);
	immediateContext->DSSetShader(nullptr, nullptr, 0);
	immediateContext->GSSetShader(nullptr, nullptr, 0);
	immediateContext->PSSetShader(nullptr, nullptr, 0);
	immediateContext->CSSetShader(nullptr, nullptr, 0);
}

void RenderParticles
(
	Particles* particles,
	ID3D11DeviceContext* immediateContext,
	Scene* scene,
	ShaderD3D11* vShader,
	ShaderD3D11* gShader,
	ShaderD3D11* pShader,
	ShaderD3D11* cShader,
	D3D11_VIEWPORT& viewport, ID3D11Buffer* posBuffer, ID3D11Buffer* vpBuffer,
	ID3D11RenderTargetView* outputRTV, ID3D11DepthStencilView* dsView
)
{
	ClearShaders(immediateContext);

	immediateContext->IASetInputLayout(nullptr);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);


	// Vertex shader
	vShader->BindShader(immediateContext);
	ID3D11ShaderResourceView* ParticleSRV = particles->GetParticleBufferSRV();
	immediateContext->VSSetShaderResources(0, 1, &ParticleSRV);

	// Geometry shader
	gShader->BindShader(immediateContext);
	std::vector<ID3D11Buffer*> gsCBuffers =
	{
		posBuffer,
		vpBuffer
	};
	immediateContext->GSSetConstantBuffers(0, gsCBuffers.size(), gsCBuffers.data());

	immediateContext->RSSetViewports(1, &viewport);

	// Pixel shader
	pShader->BindShader(immediateContext);
	immediateContext->OMSetRenderTargets(1, &outputRTV, dsView);

	immediateContext->Draw(particles->GetNrOfParticles(), 0);

	// Unbind resources
	immediateContext->GSSetShader(nullptr, nullptr, 0);
	ID3D11ShaderResourceView* nullSRV = nullptr;
	immediateContext->VSSetShaderResources(0, 1, &nullSRV);
}


void RenderShadowMaps
(
	ID3D11DeviceContext* immediateContext,
	Scene* scene,
	InputLayoutD3D11* inputLayout,
	ShaderD3D11* vShader,
	D3D11_VIEWPORT& shadowViewport
)
{
	ClearShaders(immediateContext);

	// ----------- INPUT ASSEMBLER ------------
	immediateContext->IASetInputLayout(inputLayout->GetInputLayout());
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ----------- SHADOW PASS ------------

	immediateContext->RSSetViewports(1, &shadowViewport); // viewport with matching shadow map dimensions

	vShader->BindShader(immediateContext);
	scene->DrawShadowMaps(immediateContext);
}




void RenderDeferred
(
	ID3D11DeviceContext* immediateContext,
	Scene* scene, ID3D11Buffer* cameraPositionBuffer, DirectX::XMFLOAT4X4 vpMatrix, const DirectX::BoundingFrustum& frustum,
	ID3D11UnorderedAccessView* outputUAV, ID3D11RenderTargetView* outputRTV,
	std::vector<GeometryBufferD3D11*>Gbuffers,
	ID3D11DepthStencilView* dsView, D3D11_VIEWPORT& viewport, D3D11_VIEWPORT& shadowViewport,

	ShaderD3D11* vShader, InputLayoutD3D11* inputLayout,
	ShaderD3D11* hShaderLOD, ShaderD3D11* dShaderLOD,
	ShaderD3D11* pShaderDeferred, ShaderD3D11* pShaderCubeMap,
	ShaderD3D11* cShader, std::vector<UINT> dispatchThreads,

	ShaderD3D11* vShaderParticles, ShaderD3D11* gShaderParticles,
	ShaderD3D11* pShaderParticles, ShaderD3D11* cShaderParticles,

	SamplerD3D11* sampler, SamplerD3D11* shadowSampler
)
{
	ClearShaders(immediateContext);

	float Black[4] = { 0, 0, 0, 0 };	// black

	for (auto& g : Gbuffers)
	{
		immediateContext->ClearRenderTargetView(g->GetRTV(), Black);
	}
	immediateContext->ClearUnorderedAccessViewFloat(outputUAV, Black);
	immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);


	// ----------- SHADOW PASS ------------
	RenderShadowMaps(immediateContext, scene, inputLayout, vShader, shadowViewport);
	scene->UpdateVPBuffer(immediateContext, vpMatrix); // reset VP buffer to scenes active camera's VP matrix

	immediateContext->IASetInputLayout(inputLayout->GetInputLayout());
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST); // previously D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST	


	// ----------- GENERAL ------------


	immediateContext->RSSetViewports(1, &viewport);

	ID3D11SamplerState* samplerState = sampler->GetSamplerState();
	immediateContext->PSSetSamplers(0, 1, &samplerState);

	// ----------- GEOMETRY PASS ------------

	vShader->BindShader(immediateContext);
	hShaderLOD->BindShader(immediateContext);
	dShaderLOD->BindShader(immediateContext);
	pShaderDeferred->BindShader(immediateContext);
	cShader->BindShader(immediateContext);

	std::vector<ID3D11RenderTargetView*> rtvs;
	for (auto& g : Gbuffers)
	{
		rtvs.push_back(g->GetRTV());
	}

	immediateContext->OMSetRenderTargets(rtvs.size(), rtvs.data(), dsView);
	immediateContext->HSSetConstantBuffers(0, 1, &cameraPositionBuffer);

	// Drawing objects
	scene->DrawObjects(immediateContext, frustum);


	// Switching to cube map shader and drawing reflective objects
	pShaderCubeMap->BindShader(immediateContext);
	immediateContext->PSSetConstantBuffers(1, 1, &cameraPositionBuffer);
	scene->DrawReflectiveObjects(immediateContext);

	immediateContext->OMSetRenderTargets(0, nullptr, nullptr); // unbind all render targets

	// ----------- LIGHT PASS ------------

	// bind Gbuffer textures

	std::vector<ID3D11ShaderResourceView*> srvs;
	for (auto& g : Gbuffers)
		srvs.push_back(g->GetSRV());

	srvs.push_back(scene->GetSpotLightCollection()->GetLightBufferSRV());
	srvs.push_back(scene->GetSpotLightCollection()->GetShadowMapsSRV());

	immediateContext->CSSetShaderResources(0, srvs.size(), srvs.data());

	// set samplers
	ID3D11SamplerState* CSsampler = shadowSampler->GetSamplerState();
	immediateContext->CSSetSamplers(0, 1, &CSsampler);

	// bind Cbuffers
	std::vector<ID3D11Buffer*> CSCbuffers = { cameraPositionBuffer, scene->GetLightInfoBuffer() };
	immediateContext->CSSetConstantBuffers(0, CSCbuffers.size(), CSCbuffers.data());

	// Bind the unordered access view to the compute shader
	immediateContext->CSSetUnorderedAccessViews(0, 1, &outputUAV, nullptr);

	// Dispatch the compute shader
	immediateContext->Dispatch(dispatchThreads[0], dispatchThreads[1], dispatchThreads[2]);


	// Unbind Gbuffers
	ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	immediateContext->CSSetShaderResources(0, 6, nullSRVs);

	// Unbind the unordered access view
	ID3D11UnorderedAccessView* nullUAVs[] = { nullptr };
	immediateContext->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullptr);

	// ----------- PARTICLES ------------
	scene->UpdateVPBuffer(immediateContext, vpMatrix); // update VP buffer to scenes active camera's VP matrix
	RenderParticles
	(
		scene->GetParticles(), immediateContext, scene,
		vShaderParticles, gShaderParticles,
		pShaderParticles, cShaderParticles,
		viewport, cameraPositionBuffer, scene->GetActiveCamera()->GetVpConstantBuffer(),
		outputRTV, dsView
	);
	scene->UpdateVPBuffer(immediateContext); // reset VP buffer to scenes active camera's VP matrix
}

void RenderReflectiveObject(SceneObjectReflective* reflectiveObject,
	ID3D11DeviceContext* immediateContext, Scene* scene,
	std::vector<GeometryBufferD3D11*>GbuffersCubeMap,
	D3D11_VIEWPORT& shadowViewport,

	ShaderD3D11* vShader, InputLayoutD3D11* inputLayout,
	ShaderD3D11* hShaderLOD, ShaderD3D11* dShaderLOD,
	ShaderD3D11* pShaderDeferred, ShaderD3D11* pShaderCubeMap,
	ShaderD3D11* cShader, std::vector<UINT> dispatchThreads,

	ShaderD3D11* vShaderParticles, ShaderD3D11* gShaderParticles,
	ShaderD3D11* pShaderParticles, ShaderD3D11* cShaderParticles,

	SamplerD3D11* sampler, SamplerD3D11* shadowSampler)
{
	ID3D11DepthStencilView* textureCubeDSV = reflectiveObject->GetDepthBufferDSV();
	D3D11_VIEWPORT textureCubeViewport = reflectiveObject->GetViewport();

	for (int i = 0; i < 6; ++i)
	{
		ID3D11UnorderedAccessView* textureCubeUAV = reflectiveObject->GetUAV(i);
		ID3D11RenderTargetView* textureCubeRTV = reflectiveObject->GetRTV(i);
		ID3D11Buffer* cameraPosConstantBuffer = reflectiveObject->GetCameraPosConstantBuffer(i);

		// update scene with the vpMatrix corresponding to the current environment camera
		DirectX::XMFLOAT4X4 vpMatrix = reflectiveObject->GetCameraVpMatrix(i);
		ID3D11Buffer* vpBuffer = reflectiveObject->GetCameraVpConstantBuffer(i);
		const DirectX::BoundingFrustum frustum = reflectiveObject->GetCameraFrustum(i);

		RenderShadowMaps(immediateContext, scene, inputLayout, vShader, shadowViewport);

		RenderDeferred
		(
			immediateContext,
			scene, cameraPosConstantBuffer, vpMatrix, frustum,	// TODO: use a vpBuffer instead and change the logic in renderShadowmaps aswell
			textureCubeUAV, textureCubeRTV,
			GbuffersCubeMap,
			textureCubeDSV, textureCubeViewport, shadowViewport,
			vShader, inputLayout,
			hShaderLOD, dShaderLOD,
			pShaderDeferred, pShaderCubeMap,
			cShader, dispatchThreads,
			vShaderParticles, gShaderParticles,
			pShaderParticles, cShaderParticles,
			sampler, shadowSampler
		);
	}

	scene->UpdateVPBuffer(immediateContext); // reset VP buffer to scenes active camera's VP matrix
}

void RenderBoundingBoxes
(
	ID3D11DeviceContext* immediateContext,
	Scene* scene, 
	ShaderD3D11* vShader, ShaderD3D11* pShader,
	ID3D11RenderTargetView* outputRTV, ID3D11DepthStencilView* dsView
)
{
	ClearShaders(immediateContext);

	// ----------- INPUT ASSEMBLER ------------
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// ----------- SHADERS ------------
	vShader->BindShader(immediateContext);
	pShader->BindShader(immediateContext);

	immediateContext->OMSetRenderTargets(1, &outputRTV, dsView);

	// ----------- DRAWING ------------
	scene->DrawBoundingBoxes(immediateContext);
}


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
)
{
	//scene->Update(immediateContext, cShaderParticles);
	immediateContext->RSSetState(rsState);


	// RENDER REFLECTIVE OBJECTS
	UINT nrOfReflectiveObjects = scene->GetNrOfReflectiveObjects();
	for (int i = 0; i < nrOfReflectiveObjects; ++i)
	{
		RenderReflectiveObject
		(
			scene->GetReflectiveObject(i),
			immediateContext, scene,
			GbuffersCubeMap,
			shadowViewport,
			vShader, inputLayout,
			hShaderLOD, dShaderLOD,
			pShaderDeferred, pShaderCubeMap,
			cShader, { 64, 64, 1 },
			vShaderParticles, gShaderParticles,
			pShaderParticles, cShaderParticles,
			sampler, shadowSampler
		);
	}

	// RENDER SCENE
	std::cerr << "--------------------- Render Scene -------------------" << std::endl;
	RenderDeferred
	(
		immediateContext,
		scene, 
		scene->GetActiveCamera()->GetPosConstantBuffer(), 
		scene->GetActiveCamera()->GetViewProjectionMatrix(),
		scene->GetActiveCamera()->GetFrustum(),
		outputUAV, outputRTV,
		Gbuffers,
		dsView, viewport, shadowViewport,
		vShader, inputLayout,
		hShaderLOD, dShaderLOD,
		pShaderDeferred, pShaderCubeMap,
		cShader, { 64, 36, 1 },
		vShaderParticles, gShaderParticles,
		pShaderParticles, cShaderParticles,
		sampler, shadowSampler
	);

	RenderBoundingBoxes(immediateContext, scene, vShaderBB, pShaderBB, outputRTV, dsView);

	

	DirectX::BoundingFrustum frustum = scene->GetActiveCamera()->GetFrustum();
	std::cerr << "Frustum Origin: " << frustum.Origin.x << " " << frustum.Origin.y << " " << frustum.Origin.z << std::endl;
	std::cerr << "Frustum Orientation: " << frustum.Orientation.x << " " << frustum.Orientation.y << " " << frustum.Orientation.z << std::endl;
	std::cerr << "Frustum Right: " << frustum.RightSlope << std::endl;
	std::cerr << "Frustum Left: " << frustum.LeftSlope << std::endl;
	std::cerr << "Frustum Top: " << frustum.TopSlope << std::endl;
	std::cerr << "Frustum Bottom: " << frustum.BottomSlope << std::endl;
	std::cerr << "Frustum Near: " << frustum.Near << std::endl;
	std::cerr << "Frustum Far: " << frustum.Far << std::endl;


	// Copy the output texture to the back buffer (screen)
	immediateContext->CopyResource(backBuffer, outputTexture);
}