#pragma once

#include <vector>
#include <DirectXMath.h>
#include <d3d11.h>
#include <DirectXCollision.h>

#include "MeshD3D11.h"
#include "ConstantBufferD3D11.h"

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 Uv;
};

struct Material
{
	float ambient;
	float diffuse;
	float specular;
	float shininess;
};

class Object
{
private:

public:
	Object() = default;
	Object(const Object& other) = delete;
	Object& operator=(const Object& other) = delete;
	Object(Object&& other) = default;
	Object& operator=(Object&& other) = default;
	~Object() = default;

	MeshD3D11 mesh;
	ConstantBufferD3D11 materialConstantBuffer;
	DirectX::BoundingBox boundingBox;

	DirectX::XMVECTOR minV;
	DirectX::XMVECTOR maxV;

	void LoadMeshAndMaterial(ID3D11Device* device, const char* pathToOBJFile);
};