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


//struct Vertex
//{
//	DirectX::XMFLOAT3 Position;
//	DirectX::XMFLOAT3 Normal;
//	DirectX::XMFLOAT2 Uv;
//};
//
//struct Material
//{
//	float ambient;
//	float diffuse;
//	float specular;
//	float shininess;
//};

enum TEXTURE_CUBE_FACE_INDEX
{
	POSITIVE_X = 0,
	NEGATIVE_X = 1,
	POSITIVE_Y = 2,
	NEGATIVE_Y = 3,
	POSITIVE_Z = 4,
	NEGATIVE_Z = 5
};

class SceneObjectReflective : private Object
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

	// Cube map 
	ID3D11Texture2D* texture = nullptr;
	ID3D11RenderTargetView* rtv[6];
	ID3D11UnorderedAccessView* uav[6];
	ID3D11ShaderResourceView* srv = nullptr;

	DepthBufferD3D11 depthBuffer;
	D3D11_VIEWPORT viewport;
	CameraD3D11 cameras[6];

	//void LoadMeshAndMaterial(ID3D11Device* device, const char* filename);
	void InitializeConstantBuffer(ID3D11Device* device);
	void InitializeTextureCube(ID3D11Device* device, UINT textureDimension, DXGI_FORMAT format);

	void MoveInDirection(float amount, const DirectX::XMFLOAT3& direction);
	void RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis);

public:
	SceneObjectReflective() = default;
	SceneObjectReflective
	(
		ID3D11Device* device,
		const DirectX::XMFLOAT3& initialPosition,
		const float initialScale,
		UINT textureDimension,
		const char* filename
	);
	~SceneObjectReflective();
	SceneObjectReflective(const SceneObjectReflective& other) = delete;
	SceneObjectReflective& operator=(const SceneObjectReflective& other) = delete;
	SceneObjectReflective(SceneObjectReflective&& other) = default;
	SceneObjectReflective& operator=(SceneObjectReflective&& other) = default;

	void Initialize
	(
		ID3D11Device* device,
		const DirectX::XMFLOAT3& initialPosition,
		const float initialScale,
		UINT textureDimension,
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

	ID3D11Buffer* GetConstantBuffer() const;
	ID3D11Buffer* GetMaterialConstantBuffer() const;
	ID3D11ShaderResourceView* GetShaderResourceTexture() const;

	DirectX::XMFLOAT4X4 GetWorldMatrix() const;

	// Cube map related
	D3D11_VIEWPORT GetViewport() const;
	ID3D11DepthStencilView* GetDepthBufferDSV() const;
	DirectX::XMFLOAT4X4 GetCameraVpMatrix(int index) const;
	ID3D11Buffer* GetCameraPosConstantBuffer(int index) const;
	ID3D11Buffer* GetCameraVpConstantBuffer(int index) const;
	DirectX::BoundingFrustum GetCameraFrustum(int index) const;

	ID3D11RenderTargetView* GetRTV(int index) const;
	ID3D11UnorderedAccessView* GetUAV(int index) const;
	ID3D11DepthStencilView* GetDSV() const;

	void UpdateInternalConstantBuffer(ID3D11DeviceContext* immediateContext);

	void Update(ID3D11DeviceContext* immediateContext);
	void Draw(ID3D11DeviceContext* immediateContext, ID3D11Buffer* vpBuffer);
	void DrawShadowMap(ID3D11DeviceContext* immediateContext);
};

