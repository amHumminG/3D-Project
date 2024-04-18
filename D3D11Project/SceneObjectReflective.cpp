#include "SceneObjectReflective.h"


// Loads the mesh and material from an OBJ file
//void SceneObjectReflective::LoadMeshAndMaterial(ID3D11Device* device, const char* pathToOBJFile)
//{
//	// Load the model
//	objl::Loader loader;
//	if (!loader.LoadFile(pathToOBJFile))
//	{
//		std::cerr << "SceneObject::LoadMeshAndMaterial() Failed to load OBJ file!" << std::endl;
//	}
//
//	// data to be used for initializing the mesh
//	MeshData meshData;
//
//	// vertex buffer
//	std::vector<Vertex> vertices;
//	for (size_t i = 0; i < loader.LoadedVertices.size(); ++i)
//	{
//		Vertex vertex;
//
//		vertex.Position.x = loader.LoadedVertices[i].Position.X;
//		vertex.Position.y = loader.LoadedVertices[i].Position.Y;
//		vertex.Position.z = loader.LoadedVertices[i].Position.Z;
//
//		vertex.Normal.x = loader.LoadedVertices[i].Normal.X;
//		vertex.Normal.y = loader.LoadedVertices[i].Normal.Y;
//		vertex.Normal.z = loader.LoadedVertices[i].Normal.Z;
//
//		vertex.Uv.x = loader.LoadedVertices[i].TextureCoordinate.X;
//		vertex.Uv.y = -loader.LoadedVertices[i].TextureCoordinate.Y;
//
//		vertices.push_back(vertex);
//	}
//
//	meshData.vertexInfo = { sizeof(Vertex), vertices.size(), vertices.data() };
//
//	// index buffer in normal order
//	std::vector<unsigned int> indices;
//	for (size_t i = 0; i < loader.LoadedIndices.size(); ++i)
//	{
//		indices.push_back(loader.LoadedIndices[i]);
//	}
//
//	meshData.indexInfo = { indices.size(), indices.data() };
//
//
//	// submesh info
//	size_t indexOffset = 0;
//	for (size_t i = 0; i < loader.LoadedMeshes.size(); ++i)
//	{
//		size_t startIndexValue = indexOffset;
//		size_t nrOfIndicesInSubMesh = loader.LoadedMeshes[i].Indices.size();
//
//		// TODO: Load textures
//		ID3D11ShaderResourceView* ambientTextureSRV = nullptr;
//		ID3D11ShaderResourceView* diffuseTextureSRV = nullptr;
//		ID3D11ShaderResourceView* specularTextureSRV = nullptr;
//
//		meshData.subMeshInfo.push_back({ startIndexValue, nrOfIndicesInSubMesh, ambientTextureSRV, diffuseTextureSRV, specularTextureSRV });
//
//		indexOffset += loader.LoadedMeshes[i].Indices.size();
//	}
//
//	this->mesh.Initialize(device, meshData);
//
//	// -------------------------- MATERIAL --------------------------
//	// TODO: for now we only support one material per object but we should support one material per submesh
//	Material material;
//	material.ambient = loader.LoadedMaterials[0].Ka.X;
//	material.diffuse = loader.LoadedMaterials[0].Kd.X;
//	material.specular = loader.LoadedMaterials[0].Ks.X;
//	material.shininess = loader.LoadedMaterials[0].Ns;
//	this->materialConstantBuffer.Initialize(device, sizeof(Material), &material);
//}

void SceneObjectReflective::InitializeConstantBuffer(ID3D11Device* device)
{
	DirectX::XMFLOAT4X4 matrices[1] = { GetWorldMatrix() };
	this->constantBuffer.Initialize(device, sizeof(matrices), matrices);
}

void SceneObjectReflective::InitializeTextureCube(ID3D11Device* device, UINT textureDimension, DXGI_FORMAT format)
{
	// ---------------- CREATING THE TEXTURES ----------------
	bool hasSRV = true;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = textureDimension;
	desc.Height = textureDimension;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	//desc.BindFlags |= hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &this->texture);

	if (FAILED(hr))
	{
		std::cerr << "Could not create texture cube" << std::endl;
		return;
	}

	// ---------------- CREATING THE RESOURCE VIEWS ----------------

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < 6; i++)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		HRESULT hr = device->CreateRenderTargetView(this->texture, &rtvDesc, &this->rtv[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create texture cube rtv" << std::endl;
			std::cerr << "Error code: " << hr << std::endl;
		}
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.ArraySize = 1;
	uavDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < 6; i++)
	{
		uavDesc.Texture2DArray.FirstArraySlice = i;
		HRESULT hr = device->CreateUnorderedAccessView(this->texture, &uavDesc, &this->uav[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create texture cube uav" << std::endl;
			std::cerr << "Error code: " << hr << std::endl;
		}
	}

	bool needsSRV = true; // TEMPORARY
	if (needsSRV == true)
	{
		HRESULT hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);

		if (FAILED(hr))
		{
			std::cerr << "Could not create texture cube srv" << std::endl;
		}
	}

	// ---------------- ENVIRONEMNT CAMERAS ----------------

	ProjectionInfo projInfo;
	projInfo.fovAngleY = DirectX::XM_PIDIV2;
	projInfo.aspectRatio = 1.0f;
	projInfo.nearZ = 0.1f;
	projInfo.farZ = 100.0f;

	float upRotations[6];       // Rotations around local up vector/axis
	float rightRotations[6];    // Rotations around local right vecotor/axis

	// setting up the camera rotations
	upRotations[POSITIVE_X] = 90.0f;
	upRotations[NEGATIVE_X] = -90.0f;
	upRotations[POSITIVE_Y] = 0.0f;
	upRotations[NEGATIVE_Y] = 0.0f;
	upRotations[POSITIVE_Z] = 0.0f;
	upRotations[NEGATIVE_Z] = 180.0f;

	rightRotations[POSITIVE_X] = 0.0f;
	rightRotations[NEGATIVE_X] = 0.0f;
	rightRotations[POSITIVE_Y] = -90.0f;
	rightRotations[NEGATIVE_Y] = 90.0f;
	rightRotations[POSITIVE_Z] = 0.0f;
	rightRotations[NEGATIVE_Z] = 0.0f;

	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].Initialize(device, projInfo, this->position); // creates an internal buffer with necessary info about the camera
		this->cameras[i].RotateUp(upRotations[i]);
		this->cameras[i].RotateRight(rightRotations[i]);
	}

	// ---------------- DEPTH BUFFER ----------------

	this->depthBuffer.Initialize(device, textureDimension, textureDimension, false);


	// ---------------- VIEWPORT ----------------

	this->viewport.TopLeftX = 0;
	this->viewport.TopLeftY = 0;
	this->viewport.Width = textureDimension;
	this->viewport.Height = textureDimension;
	this->viewport.MinDepth = 0;
	this->viewport.MaxDepth = 1;
}

void SceneObjectReflective::Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& initialPosition, const float initialScale, UINT textureDimension,const char* filename)
{
	this->position = initialPosition;
	this->scale = initialScale;

	// Load the mesh
	std::string path = "Models/";
	path.append(filename);
	path.append(".obj");
	LoadMeshAndMaterial(device, path.c_str());

	// Initialize constant buffer
	InitializeConstantBuffer(device);

	// Initialize texture cube
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM; // DXGI_FORMAT_R32G32B32A32_FLOAT;
	InitializeTextureCube(device, textureDimension, format);
}


void SceneObjectReflective::MoveInDirection(float amount, const DirectX::XMFLOAT3& direction)
{
	DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
	DirectX::XMVECTOR movement = DirectX::XMVectorScale(dir, amount);
	DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(pos, movement);
	DirectX::XMStoreFloat3(&position, newPos);
}

void SceneObjectReflective::RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis)
{
	// convert amount to radians
	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&axis), DirectX::XMConvertToRadians(amount));

	DirectX::XMVECTOR newForward = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&this->forward), rotation);
	DirectX::XMVECTOR newRight = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&this->right), rotation);
	DirectX::XMVECTOR newUp = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&this->up), rotation);
	DirectX::XMStoreFloat3(&this->forward, newForward);
	DirectX::XMStoreFloat3(&this->right, newRight);
	DirectX::XMStoreFloat3(&this->up, newUp);


	// function to compare two vectors
	auto compare = [](const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) -> bool
		{
			return a.x == b.x && a.y == b.y && a.z == b.z;
		};

	if (compare(axis, this->right)) // x-axis
	{
		this->yaw -= amount;
	}
	else if (compare(axis, this->up)) // y-axis
	{
		this->pitch += amount;
	}
	else if (compare(axis, this->forward)) // z-axis
	{
		this->roll -= amount;
	}
}



SceneObjectReflective::SceneObjectReflective(ID3D11Device* device, const DirectX::XMFLOAT3& initialPosition, const float initialScale, UINT textureDimension, const char* filename)
{
	Initialize(device, initialPosition, initialScale, textureDimension, filename);
}

SceneObjectReflective::~SceneObjectReflective()
{
	if (this->texture)
	{
		this->texture->Release();
		this->texture = nullptr;
	}

	for (int i = 0; i < 6; ++i)
	{
		if (this->rtv[i])
		{
			this->rtv[i]->Release();
			this->rtv[i] = nullptr;
		}
	}

	if (this->srv)
	{
		this->srv->Release();
		this->srv = nullptr;
	}
}

// movements

void SceneObjectReflective::MoveForward(float amount)
{
	MoveInDirection(amount, forward);
	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].MoveForward(amount);
	}
}

void SceneObjectReflective::MoveRight(float amount)
{
	MoveInDirection(amount, right);
	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].MoveRight(amount);
	}
}

void SceneObjectReflective::MoveUp(float amount)
{
	MoveInDirection(amount, up);
	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].MoveUp(amount);
	}
}

// rotations

void SceneObjectReflective::RotateForward(float amount)
{
	RotateAroundAxis(amount, this->forward);
	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].RotateForward(amount);
	}
}

void SceneObjectReflective::RotateRight(float amount)
{
	RotateAroundAxis(amount, this->right);
	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].RotateRight(amount);
	}
}

void SceneObjectReflective::RotateUp(float amount)
{
	RotateAroundAxis(amount, this->up);
	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].RotateUp(amount);
	}
}


void SceneObjectReflective::ScaleUp(float amount)
{
	if (this->scale + amount > 0.1f)
		this->scale += amount;
}

// setters

void SceneObjectReflective::SetPosition(const DirectX::XMFLOAT3& newPosition)
{
	this->position = newPosition;
	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].SetPosition(newPosition);
	}
}

void SceneObjectReflective::SetScale(float newScale)
{
	if (newScale > 0.1f)
		this->scale = newScale;
}

// getters

const DirectX::XMFLOAT3& SceneObjectReflective::GetPosition() const
{
	return position;
}

const DirectX::XMFLOAT3& SceneObjectReflective::GetForward() const
{
	return forward;
}

const DirectX::XMFLOAT3& SceneObjectReflective::GetRight() const
{
	return right;
}

const DirectX::XMFLOAT3& SceneObjectReflective::GetUp() const
{
	return up;
}



ID3D11Buffer* SceneObjectReflective::GetConstantBuffer() const
{
	return this->constantBuffer.GetBuffer();
}

ID3D11Buffer* SceneObjectReflective::GetMaterialConstantBuffer() const
{
	return this->materialConstantBuffer.GetBuffer();
}

ID3D11ShaderResourceView* SceneObjectReflective::GetShaderResourceTexture() const
{
	return this->srv;
}

DirectX::XMFLOAT4X4 SceneObjectReflective::GetWorldMatrix() const
{
	using namespace DirectX;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(this->yaw), XMConvertToRadians(this->pitch), XMConvertToRadians(this->roll));
	XMMATRIX translationMatrix = XMMatrixTranslation(this->position.x, this->position.y, this->position.z);
	XMMATRIX scaleMatrix = XMMatrixScaling(this->scale, this->scale, this->scale);

	XMMATRIX world = XMMatrixTranspose(scaleMatrix * rotationMatrix * translationMatrix);

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, world);

	return worldMatrix;
}

ID3D11RenderTargetView* SceneObjectReflective::GetRTV(int index) const
{
	return this->rtv[index];
}

ID3D11UnorderedAccessView* SceneObjectReflective::GetUAV(int index) const
{
	return this->uav[index];
}

D3D11_VIEWPORT SceneObjectReflective::GetViewport() const
{
	return this->viewport;
}

ID3D11DepthStencilView* SceneObjectReflective::GetDepthBufferDSV() const
{
	return this->depthBuffer.GetDSV(0);
}

DirectX::XMFLOAT4X4 SceneObjectReflective::GetCameraVpMatrix(int index) const
{
	return this->cameras[index].GetViewProjectionMatrix();
}

ID3D11Buffer* SceneObjectReflective::GetCameraPosConstantBuffer(int index) const
{
	return this->cameras[index].GetPosConstantBuffer();
}

ID3D11Buffer* SceneObjectReflective::GetCameraVpConstantBuffer(int index) const
{
	return this->cameras[index].GetVpConstantBuffer();
}

DirectX::BoundingFrustum SceneObjectReflective::GetCameraFrustum(int index) const
{
	return this->cameras[index].GetFrustum();
}




void SceneObjectReflective::UpdateInternalConstantBuffer(ID3D11DeviceContext* immediateContext)
{
	DirectX::XMFLOAT4X4 matrices[1] = { GetWorldMatrix() };
	this->constantBuffer.UpdateBuffer(immediateContext, matrices);
}

void SceneObjectReflective::Update(ID3D11DeviceContext* immediateContext)
{
	UpdateInternalConstantBuffer(immediateContext);
	for (int i = 0; i < 6; ++i)
	{
		this->cameras[i].UpdateInternalConstantBuffer(immediateContext);
	}
}

void SceneObjectReflective::Draw(ID3D11DeviceContext* immediateContext, ID3D11Buffer* vpBuffer)
{
	this->mesh.BindMeshBuffers(immediateContext);

	// vertex shader
	std::vector<ID3D11Buffer*> VS_Cbuffers =
	{
		this->constantBuffer.GetBuffer()
	};
	immediateContext->VSSetConstantBuffers(0, VS_Cbuffers.size(), VS_Cbuffers.data());

	// domain shader
	std::vector<ID3D11Buffer*> DS_Cbuffers =
	{
		vpBuffer
	};
	immediateContext->DSSetConstantBuffers(0, DS_Cbuffers.size(), DS_Cbuffers.data());

	// pixel shader
	std::vector<ID3D11Buffer*> PS_Cbuffers =
	{
		this->materialConstantBuffer.GetBuffer()
	};
	immediateContext->PSSetConstantBuffers(0, PS_Cbuffers.size(), PS_Cbuffers.data());

	// shader resource view
	std::vector<ID3D11ShaderResourceView*> PS_SRVs =
	{
		GetShaderResourceTexture()
	};
	immediateContext->PSSetShaderResources(0, PS_SRVs.size(), PS_SRVs.data());

	// draw calls
	for (int i = 0; i < this->mesh.GetNrOfSubMeshes(); ++i)
	{
		this->mesh.PerformSubMeshDrawCall(immediateContext, i);
	}
}

void SceneObjectReflective::DrawShadowMap(ID3D11DeviceContext* immediateContext)
{
	this->mesh.BindMeshBuffers(immediateContext);

	// draw calls
	for (int j = 0; j < this->mesh.GetNrOfSubMeshes(); ++j)
	{
		this->mesh.PerformSubMeshDrawCall(immediateContext, j);
	}
}