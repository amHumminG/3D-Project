#pragma once

#include <d3d11_4.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>

#include "ConstantBufferD3D11.h"

struct ProjectionInfo
{
	float fovAngleY = 0.0f;
	float aspectRatio = 0.0f;
	float nearZ = 0.0f;
	float farZ = 0.0f;
};

class CameraD3D11
{
private:
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 forward = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 right = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	ProjectionInfo projInfo;

	ConstantBufferD3D11 cameraPosBuffer;
	ConstantBufferD3D11 cameraVpBuffer;

	DirectX::BoundingFrustum frustum;

	void MoveInDirection(float amount, const DirectX::XMFLOAT3& direction);
	void RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis);

public:
	CameraD3D11() = default;
	CameraD3D11(ID3D11Device* device, const ProjectionInfo& projectionInfo,
		const DirectX::XMFLOAT3& initialPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	~CameraD3D11() = default;
	CameraD3D11(const CameraD3D11& other) = delete;
	CameraD3D11& operator=(const CameraD3D11& other) = default;
	CameraD3D11(CameraD3D11&& other) = default;
	CameraD3D11& operator=(CameraD3D11&& other) = default;

	bool Initialize(ID3D11Device* device, const ProjectionInfo& projectionInfo,
		const DirectX::XMFLOAT3& initialPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

	void MoveForward(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);
	void SetPosition(const DirectX::XMFLOAT3& position);

	void RotateForward(float amount);
	void RotateRight(float amount);
	void RotateUp(float amount);

	const DirectX::XMFLOAT3& GetPosition() const;
	const DirectX::XMFLOAT3& GetForward() const;
	const DirectX::XMFLOAT3& GetRight() const;
	const DirectX::XMFLOAT3& GetUp() const;

	bool UpdateInternalConstantBuffer(ID3D11DeviceContext* context);

	ID3D11Buffer* GetPosConstantBuffer() const;
	ID3D11Buffer* GetVpConstantBuffer() const;

	DirectX::XMFLOAT4X4 GetViewProjectionMatrix() const;
	const DirectX::BoundingFrustum& GetFrustum() const;
	std::vector<DirectX::XMFLOAT3> GetFrustumCorners() const;
};