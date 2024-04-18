#include "SceneObject.h"

void SceneObject::InitializeConstantBuffer(ID3D11Device* device)
{
	DirectX::XMFLOAT4X4 matrices[1] = { GetWorldMatrix() };
	this->constantBuffer.Initialize(device, sizeof(matrices), matrices);
}

void SceneObject::InitializeShaderResourceTexture(ID3D11Device* device, const char* pathToTextureFile)
{
	this->shaderResourceTexture.Initialize(device, pathToTextureFile);
}

void SceneObject::MoveInDirection(float amount, const DirectX::XMFLOAT3& direction)
{
	DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
	DirectX::XMVECTOR movement = DirectX::XMVectorScale(dir, amount);
	DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(pos, movement);
	DirectX::XMStoreFloat3(&position, newPos);
}

void SceneObject::RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis)
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

SceneObject::SceneObject(ID3D11Device* device, const DirectX::XMFLOAT3& initialPosition, const float initialScale, const char* filename)
{
	Initialize(device, initialPosition, initialScale, filename);
}

void SceneObject::Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& initialPosition, const float initialScale, const char* filename)
{
	this->position = initialPosition;
	this->scale = initialScale;
	this->name = filename;

	// Load the mesh
	std::string path = "Models/";
	path.append(filename);
	path.append(".obj");
	LoadMeshAndMaterial(device, path.c_str());

	// Create bounding box using minV = (minX, minY, minZ) and maxV = (maxX, maxY, maxZ)
	DirectX::FXMMATRIX translationMatrix = DirectX::XMMatrixTranslation(this->position.x, this->position.y, this->position.z);
	DirectX::XMVECTOR minV = DirectX::XMVector3Transform(this->minV, translationMatrix);	// offset the minV by the position
	DirectX::XMVECTOR maxV = DirectX::XMVector3Transform(this->maxV, translationMatrix);	// offset the maxV by the position
	DirectX::BoundingBox::CreateFromPoints(this->boundingBox, minV, maxV);



	std::vector<DirectX::XMFLOAT3> corners = GetBoundingBoxCorners();
	std::vector<BoundingVertex> vertices = GetVertices(corners, {1.0f, 0.0f, 0.0f});

	this->boundingBoxVertexBuffer.Initialize(device, sizeof(BoundingVertex), vertices.size(), vertices.data());

	// Initialize constant buffer
	InitializeConstantBuffer(device);

	// Initialize shaderResourceTexture
	path = "Textures/";
	path.append(filename);
	path.append(".png");
	InitializeShaderResourceTexture(device, path.c_str());
}

// movements

void SceneObject::MoveForward(float amount)
{
	MoveInDirection(amount, forward);
}

void SceneObject::MoveRight(float amount)
{
	MoveInDirection(amount, right);
}

void SceneObject::MoveUp(float amount)
{
	MoveInDirection(amount, up);
}

// rotations

void SceneObject::RotateForward(float amount)
{
	RotateAroundAxis(amount, this->forward);
}

void SceneObject::RotateRight(float amount)
{
	RotateAroundAxis(amount, this->right);
}

void SceneObject::RotateUp(float amount)
{
	RotateAroundAxis(amount, this->up);
}

void SceneObject::ScaleUp(float amount)
{
	if (this->scale + amount > 0.1f)
		this->scale += amount;
}

// setters

void SceneObject::SetPosition(const DirectX::XMFLOAT3& newPosition)
{
	this->position = newPosition;
}

void SceneObject::SetScale(float newScale)
{
	if (newScale > 0.1f)
		this->scale = newScale;
}

// getters

const DirectX::XMFLOAT3& SceneObject::GetPosition() const
{
	return position;
}

const DirectX::XMFLOAT3& SceneObject::GetForward() const
{
	return forward;
}

const DirectX::XMFLOAT3& SceneObject::GetRight() const
{
	return right;
}

const DirectX::XMFLOAT3& SceneObject::GetUp() const
{
	return up;
}

const std::string& SceneObject::GetName() const
{
	return name;
}

std::vector<DirectX::XMFLOAT3> SceneObject::GetBoundingBoxCorners() const
{
	std::vector<DirectX::XMFLOAT3> corners;
	DirectX::XMFLOAT3 cornersArray[8];
	this->boundingBox.GetCorners(cornersArray);
	for (int i = 0; i < 8; ++i)
	{
		corners.push_back(cornersArray[i]);
	}

	return corners;
}

std::vector<BoundingVertex> SceneObject::GetVertices(std::vector<DirectX::XMFLOAT3> corners, DirectX::XMFLOAT3 color) const
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


ID3D11Buffer* SceneObject::GetConstantBuffer() const
{
	return this->constantBuffer.GetBuffer();
}

ID3D11Buffer* SceneObject::GetMaterialConstantBuffer() const
{
	return this->materialConstantBuffer.GetBuffer();
}

ID3D11ShaderResourceView* SceneObject::GetShaderResourceTexture() const
{
	return this->shaderResourceTexture.GetSRV();
}



DirectX::XMFLOAT4X4 SceneObject::GetWorldMatrix() const
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

const DirectX::BoundingBox& SceneObject::GetBoundingBox() const
{
	return this->boundingBox;
}

void SceneObject::UpdateInternalConstantBuffer(ID3D11DeviceContext* immediateContext)
{
	DirectX::XMFLOAT4X4 matrices[1] = { GetWorldMatrix() };
	this->constantBuffer.UpdateBuffer(immediateContext, matrices);
}

void SceneObject::Update(ID3D11DeviceContext* immediateContext)
{
	UpdateInternalConstantBuffer(immediateContext);
}

void SceneObject::Draw(ID3D11DeviceContext* immediateContext, ID3D11Buffer* vpBuffer) const
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
		this->shaderResourceTexture.GetSRV()
	};
	immediateContext->PSSetShaderResources(0, PS_SRVs.size(), PS_SRVs.data());

	// draw calls
	for (int i = 0; i < this->mesh.GetNrOfSubMeshes(); ++i)
	{
		this->mesh.PerformSubMeshDrawCall(immediateContext, i);
	}
}

void SceneObject::DrawShadowMap(ID3D11DeviceContext* immediateContext)
{
	this->mesh.BindMeshBuffers(immediateContext);

	// draw calls
	for (int j = 0; j < this->mesh.GetNrOfSubMeshes(); ++j)
	{
		this->mesh.PerformSubMeshDrawCall(immediateContext, j);
	}
}

void SceneObject::DrawBoundingBox(ID3D11DeviceContext* immediateContext, ID3D11Buffer* vpBuffer) const
{
	UINT offset = 0;	// Offset in bytes from the start of the buffer to the first element to use
	UINT stride = this->boundingBoxVertexBuffer.GetVertexSize();	// The size of a single vertex in the buffer
	ID3D11Buffer* IA_VertexBuffers[1] = { this->boundingBoxVertexBuffer.GetBuffer() };
	immediateContext->IASetVertexBuffers(0, 1, IA_VertexBuffers, &stride, &offset);

	// vertex shader
	std::vector<ID3D11Buffer*> VS_Cbuffers =
	{
		vpBuffer
	};
	immediateContext->VSSetConstantBuffers(0, VS_Cbuffers.size(), VS_Cbuffers.data());

	// draw call
	immediateContext->Draw(24, 0);
}