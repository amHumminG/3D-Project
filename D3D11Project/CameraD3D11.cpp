#include "CameraD3D11.h"

void CameraD3D11::MoveInDirection(float amount, const DirectX::XMFLOAT3& direction)
{
	DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
	DirectX::XMVECTOR movement = DirectX::XMVectorScale(dir, amount);
	DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(pos, movement);
	DirectX::XMStoreFloat3(&position, newPos);
}

void CameraD3D11::RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis)
{
	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&axis), DirectX::XMConvertToRadians(amount));

	DirectX::XMVECTOR newForward = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&this->forward), rotation);
	DirectX::XMVECTOR newRight = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&this->right), rotation);
	DirectX::XMVECTOR newUp = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&this->up), rotation);
	DirectX::XMStoreFloat3(&this->forward, newForward);
	DirectX::XMStoreFloat3(&this->right, newRight);
	DirectX::XMStoreFloat3(&this->up, newUp);
}                        

CameraD3D11::CameraD3D11(ID3D11Device* device, const ProjectionInfo& projectionInfo, const DirectX::XMFLOAT3& initialPosition)
{
	Initialize(device, projectionInfo, initialPosition);
}

bool CameraD3D11::Initialize(ID3D11Device* device, const ProjectionInfo& projectionInfo, const DirectX::XMFLOAT3& initialPosition)
{
	this->projInfo = projectionInfo;
	this->position = initialPosition;
	if (!cameraPosBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4), &this->position))
	{
		std::cerr << "Failed to initialize cameraPos buffer" << std::endl;
		return false;
	}

	DirectX::XMFLOAT4X4 vpMatrix = GetViewProjectionMatrix();
	if (!cameraVpBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4X4), &vpMatrix))
	{
		std::cerr << "Failed to initialize cameraVp buffer" << std::endl;
		return false;
	}

	// transpose vpMatrix back to normal for frustum creation
	DirectX::XMMATRIX vpMatrixXM = DirectX::XMLoadFloat4x4(&vpMatrix);
	vpMatrixXM = DirectX::XMMatrixTranspose(vpMatrixXM);
	DirectX::BoundingFrustum::CreateFromMatrix(this->frustum, vpMatrixXM);

	return true;
}

// movements
void CameraD3D11::MoveForward(float amount)
{
	MoveInDirection(amount, forward);
}

void CameraD3D11::MoveRight(float amount)
{
	MoveInDirection(amount, right);
}

void CameraD3D11::MoveUp(float amount)
{
	MoveInDirection(amount, up);
}

void CameraD3D11::SetPosition(const DirectX::XMFLOAT3& position)
{
	this->position = position;
}


// rotations
void CameraD3D11::RotateForward(float amount)
{
	RotateAroundAxis(amount, forward);
}

void CameraD3D11::RotateRight(float amount)
{
	RotateAroundAxis(amount, right);
}

void CameraD3D11::RotateUp(float amount)
{
	RotateAroundAxis(amount, up);
}


// getters
const DirectX::XMFLOAT3& CameraD3D11::GetPosition() const
{
	return position;
}

const DirectX::XMFLOAT3& CameraD3D11::GetForward() const
{
	return forward;
}

const DirectX::XMFLOAT3& CameraD3D11::GetRight() const
{
	return right;
}

const DirectX::XMFLOAT3& CameraD3D11::GetUp() const
{
	return up;
}

bool CameraD3D11::UpdateInternalConstantBuffer(ID3D11DeviceContext* context)
{
	if (!cameraPosBuffer.UpdateBuffer(context, &this->position))
	{
		std::cerr << "Failed to update camera buffer" << std::endl;
		return false;
	}

	DirectX::XMFLOAT4X4 vpMatrix = GetViewProjectionMatrix();
	if (!cameraVpBuffer.UpdateBuffer(context, &vpMatrix))
	{
		std::cerr << "Failed to update camera buffer" << std::endl;
		return false;
	}

	// transpose vpMatrix back to normal for frustum creation
	DirectX::XMMATRIX vpMatrixXM = DirectX::XMLoadFloat4x4(&vpMatrix);
	vpMatrixXM = DirectX::XMMatrixTranspose(vpMatrixXM);
	// Update the frustum
	DirectX::BoundingFrustum::CreateFromMatrix(this->frustum, vpMatrixXM);

	//this->frustum.Transform(this->frustum, vpMatrixXM);
	//this->frustum.Origin = DirectX::XMFLOAT3(position.x, position.y, position.z);
	//this->frustum.Orientation = DirectX::XMFLOAT4(forward.x, forward.y, forward.z, 1.0f);
	//this->frustum.RightSlope = projInfo.aspectRatio;
	//this->frustum.LeftSlope = -projInfo.aspectRatio;
	//this->frustum.TopSlope = projInfo.fovAngleY / 2.0f;
	//this->frustum.BottomSlope = -projInfo.fovAngleY / 2.0f;
	//this->frustum.Near = projInfo.nearZ;
	//this->frustum.Far = projInfo.farZ;

	return true;
}

// more getters
ID3D11Buffer* CameraD3D11::GetPosConstantBuffer() const
{
	return cameraPosBuffer.GetBuffer();
}

ID3D11Buffer* CameraD3D11::GetVpConstantBuffer() const
{
	return cameraVpBuffer.GetBuffer();
}

DirectX::XMFLOAT4X4 CameraD3D11::GetViewProjectionMatrix() const
{
	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&position), DirectX::XMLoadFloat3(&forward), DirectX::XMLoadFloat3(&up));
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(projInfo.fovAngleY, projInfo.aspectRatio, projInfo.nearZ, projInfo.farZ);

	DirectX::XMFLOAT4X4 viewProj;
	DirectX::XMStoreFloat4x4(&viewProj, DirectX::XMMatrixTranspose(view * proj)); // Transpose for HLSL	

	return viewProj;
}

const DirectX::BoundingFrustum& CameraD3D11::GetFrustum() const
{
	return this->frustum;
}

std::vector<DirectX::XMFLOAT3> CameraD3D11::GetFrustumCorners() const
{
	std::vector<DirectX::XMFLOAT3> corners;
	DirectX::XMFLOAT3 cornersArray[8];
	this->frustum.GetCorners(cornersArray);
	for (int i = 0; i < 8; ++i)
	{
		corners.push_back(cornersArray[i]);
	}

	return corners;
}

