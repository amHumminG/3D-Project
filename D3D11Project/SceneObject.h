#pragma once

#include <vector>
#include <DirectXMath.h>
#include <d3d11.h>

#include "Object.h"

#include "MeshD3D11.h"
#include "ConstantBufferD3D11.h"
#include "ShaderResourceTextureD3D11.h"
#include "CameraD3D11.h"
#include "SpotLightCollectionD3D11.h"



struct BoundingVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Color;
};

class SceneObject : private Object
{

private:
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 forward = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 right = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	float scale = 1.0f;

	float yaw = 0.0f;
	float pitch = 0.0f;
	float roll = 0.0f;

	//MeshD3D11 mesh;
	//ConstantBufferD3D11 materialConstantBuffer;
	ConstantBufferD3D11 constantBuffer;
	ShaderResourceTextureD3D11 shaderResourceTexture;

	// DEBUG
	std::string name;
	VertexBufferD3D11 boundingBoxVertexBuffer;

	void InitializeShaderResourceTexture(ID3D11Device* device, const char* filename);
	void InitializeConstantBuffer(ID3D11Device* device);

	void MoveInDirection(float amount, const DirectX::XMFLOAT3& direction);
	void RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis);

public:
	SceneObject() = default;
	SceneObject
	(
		ID3D11Device* device, 
		const DirectX::XMFLOAT3& initialPosition, 
		const float initialScale,
		const char* filename
	);
	~SceneObject() = default;
	SceneObject(const SceneObject& other) = delete;
	SceneObject& operator=(const SceneObject& other) = delete;
	SceneObject(SceneObject&& other) = default;
	SceneObject& operator=(SceneObject&& other) = default;

	void Initialize
	(
		ID3D11Device* device,
		const DirectX::XMFLOAT3& initialPosition,
		const float initialScale,
		const char* filename
	);

	// movement, rotations and scale
	void MoveForward(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);

	void RotateForward(float amount);
	void RotateRight(float amount);
	void RotateUp(float amount);

	void ScaleUp(float amount);

	void SetPosition(const DirectX::XMFLOAT3& newPosition);
	void SetScale(float newScale);

	// getters
	const DirectX::XMFLOAT3& GetPosition() const;
	const DirectX::XMFLOAT3& GetForward() const;
	const DirectX::XMFLOAT3& GetRight() const;
	const DirectX::XMFLOAT3& GetUp() const;

	const std::string& GetName() const;


	ID3D11Buffer* GetConstantBuffer() const;
	ID3D11Buffer* GetMaterialConstantBuffer() const;
	ID3D11ShaderResourceView* GetShaderResourceTexture() const;

	DirectX::XMFLOAT4X4 GetWorldMatrix() const;
	const DirectX::BoundingBox& GetBoundingBox() const;
	std::vector<DirectX::XMFLOAT3> GetBoundingBoxCorners() const;
	std::vector<BoundingVertex> GetVertices(std::vector<DirectX::XMFLOAT3> corners, DirectX::XMFLOAT3 color) const;


	void UpdateInternalConstantBuffer(ID3D11DeviceContext* immediateContext);

	void Update(ID3D11DeviceContext* immediateContext);
	void Draw(ID3D11DeviceContext* immediateContext, ID3D11Buffer* vpBuffer) const;
	void DrawShadowMap(ID3D11DeviceContext* immediateContext);
	void DrawBoundingBox(ID3D11DeviceContext* immediateContext, ID3D11Buffer* vpBuffer) const;
};