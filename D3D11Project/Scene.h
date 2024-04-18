#pragma once

#include <vector>
#include <iostream>

#include "SceneObject.h"
#include "SpotLightCollectionD3D11.h"
#include "CameraD3D11.h"
#include "SceneObjectReflective.h"
#include "Particles.h"
#include "QuadTree.h"

struct lightInfo
{
	float nrOfLights;
	float maxDistance;
	bool lightsOn;

	char padding[12];
};

class Scene
{
private:

	// spatial trees
	QuadTree<SceneObject> objectQuadTree;
	
	// objects
	std::vector<SceneObject*> objects;
	std::vector<SceneObjectReflective*> reflectiveObjects;

	// cameras
	std::vector<CameraD3D11*> cameras;
	UINT activeCameraIndex;
	ConstantBufferD3D11 vpBuffer;
	VertexBufferD3D11 frustumVertexBuffer;


	// lights
	lightInfo lightInfoData;
	ConstantBufferD3D11 lightInfoBuffer;
	SpotLightCollectionD3D11* spotLightCollection;

	// particles
	Particles* particles;

public:

	Scene()
	{
		activeCameraIndex = 0;
	}

	Scene(ID3D11Device* device, CameraD3D11*& camera)
	{
		Initialize(device, camera);
	}

	~Scene()
	{
		for (auto& o : objects)

		for (auto& o : reflectiveObjects)
			delete o;

		for (auto& c : cameras)
			delete c;

		if (spotLightCollection)
			delete spotLightCollection;

		if (particles)
			delete particles;
	}

	void Initialize(ID3D11Device* device, CameraD3D11*& camera)
	{
		activeCameraIndex = 0;
		AddCamera(camera);

		std::vector<DirectX::XMFLOAT4X4> vpMatrices = { GetActiveCamera()->GetViewProjectionMatrix()};
		this->vpBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4X4), vpMatrices.data());

		// Initialize the frustum vertex buffer
		std::vector<DirectX::XMFLOAT3> corners = GetActiveCamera()->GetFrustumCorners();
		std::vector<BoundingVertex> vertices = GetVertices(corners, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));	// green vertuces
		this->frustumVertexBuffer.Initialize(device, sizeof(BoundingVertex), vertices.size(), vertices.data());

		// Initialize the quadtree
		float extent = 100.0f;
		this->objectQuadTree.initialize(extent);
	}

	void Finalize()
	{
		int counter = 0;
		for (auto& o : objects)
		{
			counter++;
			std::cerr << "Trying to add SceneObject nr: " << counter << std::endl;
			this->objectQuadTree.AddElement(o, o->GetBoundingBox());
			std::cerr << "Added object to quadtree" << std::endl;
			std::cerr << std::endl;
		}
	}

	// Update the VP buffer with the active camera's VP matrix6
	void UpdateVPBuffer(ID3D11DeviceContext* immediateContext)
	{
		std::vector<DirectX::XMFLOAT4X4> vpMatrices = { GetActiveCamera()->GetViewProjectionMatrix()};
		this->vpBuffer.UpdateBuffer(immediateContext, vpMatrices.data());
	}

	// Update the VP buffer with the given matrix
	void UpdateVPBuffer(ID3D11DeviceContext* immediateContext, DirectX::XMFLOAT4X4& vpMatrix)
	{
		std::vector<DirectX::XMFLOAT4X4> vpMatrices = { vpMatrix };
		this->vpBuffer.UpdateBuffer(immediateContext, vpMatrices.data());
	}

	void UpdateLightInfoBuffer(ID3D11DeviceContext* immediateContext)
	{
		this->lightInfoBuffer.UpdateBuffer(immediateContext, &this->lightInfoData);
	}

	// Update all scene objects and the vpBuffer with the active camera's VP matrix
	void Update(ID3D11DeviceContext* immediateContext, ShaderD3D11* cShaderParticles, ID3D11Device* device)
	{
		for (auto& o : objects)
			o->Update(immediateContext);

		for (auto& o : reflectiveObjects)
			o->Update(immediateContext);

		this->particles->Update(immediateContext, cShaderParticles);

		GetActiveCamera()->UpdateInternalConstantBuffer(immediateContext);
		UpdateVPBuffer(immediateContext);
		UpdateLightInfoBuffer(immediateContext);

		// update the frustum vertex buffer
		std::vector<DirectX::XMFLOAT3> corners = GetActiveCamera()->GetFrustumCorners();
		std::vector<BoundingVertex> vertices = GetVertices(corners, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));	// green vertuces
		this->frustumVertexBuffer.Initialize(device, sizeof(BoundingVertex), vertices.size(), vertices.data());
	}

	// Update all scene objects and the vpBuffer with the given matrix
	void Update(ID3D11DeviceContext* immediateContext, ShaderD3D11* cShaderParticles, DirectX::XMFLOAT4X4& vpMatrix)
	{
		for (auto& o : objects)
			o->Update(immediateContext);

		for (auto& o : reflectiveObjects)
			o->Update(immediateContext);

		this->particles->Update(immediateContext, cShaderParticles);

		UpdateVPBuffer(immediateContext, vpMatrix);
		UpdateLightInfoBuffer(immediateContext);
	}

	void Draw(ID3D11DeviceContext* immediateContext, DirectX::BoundingFrustum& frustum)
	{
		DrawObjects(immediateContext, frustum);
		DrawReflectiveObjects(immediateContext);
	}

	// Draw all scene objects
	void DrawObjects(ID3D11DeviceContext* immediateContext, const DirectX::BoundingFrustum& frustum)
	{
		std::vector<const SceneObject*> objectsInFrustum = this->objectQuadTree.CheckTree(frustum);
		std::cerr << "Nr of objects in frustum: " << objectsInFrustum.size() << std::endl;

		for (auto& o : objectsInFrustum)
		{
			std::cerr << "Drawing " << o->GetName() << std::endl;
			o->Draw(immediateContext, this->vpBuffer.GetBuffer());
			std::cerr << "Done drawing object" << std::endl;
		}

		//for (auto& o : objects)
		//{
		//	o->Draw(immediateContext, this->vpBuffer.GetBuffer());
		//}
	}

	// Draw all reflective objects
	void DrawReflectiveObjects(ID3D11DeviceContext* immediateContext)
	{
		for (auto& o : reflectiveObjects)
			o->Draw(immediateContext, this->vpBuffer.GetBuffer());
	}

	void DrawShadowMaps(ID3D11DeviceContext* immediateContext)
	{
		for (int lightIndex = 0; lightIndex < this->spotLightCollection->GetNrOfLights(); ++lightIndex)
		{
			ID3D11DepthStencilView* dsView = this->spotLightCollection->GetShadowMapDSV(lightIndex);
			immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
			immediateContext->OMSetRenderTargets(0, nullptr, dsView);
			DirectX::XMFLOAT4X4 vpMatrix = this->spotLightCollection->GetLightViewProjectionMatrix(lightIndex);
			UpdateVPBuffer(immediateContext, vpMatrix);	// update the VP buffer with the light's VP matrix
			for (auto& o : objects)
			{
				std::vector<ID3D11Buffer*> VS_Cbuffers = { this->vpBuffer.GetBuffer(), o->GetConstantBuffer()};
				immediateContext->VSSetConstantBuffers(0, VS_Cbuffers.size(), VS_Cbuffers.data());
				o->DrawShadowMap(immediateContext);
			}
			for (auto& o : reflectiveObjects)
			{
				std::vector<ID3D11Buffer*> VS_Cbuffers = { this->vpBuffer.GetBuffer(), o->GetConstantBuffer() };
				immediateContext->VSSetConstantBuffers(0, VS_Cbuffers.size(), VS_Cbuffers.data());
				o->DrawShadowMap(immediateContext);
			}
		}

		immediateContext->OMSetRenderTargets(0, nullptr, nullptr);	// unbind the last dsView
	}

	void DrawBoundingBoxes(ID3D11DeviceContext* immediateContext)
	{
		for (auto& o : objects)
			o->DrawBoundingBox(immediateContext, this->vpBuffer.GetBuffer());

		DrawCameraFrustum(immediateContext);
	}

	void DrawCameraFrustum(ID3D11DeviceContext* immediateContext)
	{
		UINT offset = 0;
		UINT stride = this->frustumVertexBuffer.GetVertexSize();
		ID3D11Buffer* IA_VertexBuffers[1] = { this->frustumVertexBuffer.GetBuffer() };
		immediateContext->IASetVertexBuffers(0, 1, IA_VertexBuffers, &stride, &offset);

		// vertex shader
		std::vector<ID3D11Buffer*> VS_Cbuffers =
		{
			this->vpBuffer.GetBuffer()
		};
		immediateContext->VSSetConstantBuffers(0, VS_Cbuffers.size(), VS_Cbuffers.data());

		// draw call
		immediateContext->Draw(24, 0);
	}


	void AddObject(SceneObject*& object)
	{
		objects.push_back(object);
	}

	void AddReflectiveObject(SceneObjectReflective*& object)
	{
		reflectiveObjects.push_back(object);
	}

	void AddCamera(CameraD3D11*& camera)
	{
		cameras.push_back(camera);
	}

	void SetSpotLightCollection(ID3D11Device* device, SpotLightCollectionD3D11*& spotLightCollection)
	{
		this->spotLightCollection = spotLightCollection;

		this->lightInfoData = { (float)this->spotLightCollection->GetNrOfLights(), 1000.0f, true };
		this->lightInfoBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4), &lightInfoData);
	}

	void SetParticles(Particles*& particles)
	{
		this->particles = particles;
	}

	void ToggleLights()
	{
		this->lightInfoData.lightsOn = !this->lightInfoData.lightsOn;
	}

	void SetActiveCamera(UINT index)
	{
		this->activeCameraIndex = index;
	}

	SpotLightCollectionD3D11* GetSpotLightCollection()
	{
		return this->spotLightCollection;
	}

	CameraD3D11* GetActiveCamera()
	{
		return this->cameras[activeCameraIndex];
	}

	UINT GetNrOfObjects()
	{
		return this->objects.size();
	}

	UINT GetNrOfReflectiveObjects()
	{
		return this->reflectiveObjects.size();
	}

	SceneObjectReflective* GetReflectiveObject(UINT index)
	{
		return this->reflectiveObjects[index];
	}

	ID3D11Buffer* GetVPBuffer()
	{
		return this->vpBuffer.GetBuffer();
	}

	ID3D11Buffer* GetLightInfoBuffer()
	{
		return this->lightInfoBuffer.GetBuffer();
	}

	Particles* GetParticles() const
	{
		return this->particles;
	}

	std::vector<BoundingVertex> GetVertices(std::vector<DirectX::XMFLOAT3> corners, DirectX::XMFLOAT3 color) const
	{

		std::vector<BoundingVertex> vertices;
		BoundingVertex vertex;
		vertex.Color = color;

		vertex.Position = corners[0]; vertices.push_back(vertex); vertex.Position = corners[1]; vertices.push_back(vertex);
		vertex.Position = corners[1]; vertices.push_back(vertex); vertex.Position = corners[2]; vertices.push_back(vertex);
		vertex.Position = corners[2]; vertices.push_back(vertex); vertex.Position = corners[3]; vertices.push_back(vertex);
		vertex.Position = corners[3]; vertices.push_back(vertex); vertex.Position = corners[0]; vertices.push_back(vertex);

		vertex.Position = corners[0]; vertices.push_back(vertex); vertex.Position = corners[4]; vertices.push_back(vertex);
		vertex.Position = corners[1]; vertices.push_back(vertex); vertex.Position = corners[5]; vertices.push_back(vertex);
		vertex.Position = corners[2]; vertices.push_back(vertex); vertex.Position = corners[6]; vertices.push_back(vertex);
		vertex.Position = corners[3]; vertices.push_back(vertex); vertex.Position = corners[7]; vertices.push_back(vertex);

		vertex.Position = corners[4]; vertices.push_back(vertex); vertex.Position = corners[5]; vertices.push_back(vertex);
		vertex.Position = corners[5]; vertices.push_back(vertex); vertex.Position = corners[6]; vertices.push_back(vertex);
		vertex.Position = corners[6]; vertices.push_back(vertex); vertex.Position = corners[7]; vertices.push_back(vertex);
		vertex.Position = corners[7]; vertices.push_back(vertex); vertex.Position = corners[4]; vertices.push_back(vertex);

		return vertices;
	}
};