#include "SpotLightCollectionD3D11.h"

void SpotLightCollectionD3D11::Initialize(ID3D11Device* device, const SpotLightData& lightInfo)
{
	UINT textureDimension = lightInfo.shadowMapInfo.textureDimension;

	for (int i = 0; i < lightInfo.perLightInfo.size(); i++)
	{
		using namespace DirectX;
		
		XMFLOAT3 initialPosition = lightInfo.perLightInfo[i].initialPosition;

		XMFLOAT3 direction = { 0.0f, 0.0f, 1.0f };
		XMMATRIX rotationX = XMMatrixRotationX(XMConvertToRadians(lightInfo.perLightInfo[i].rotationX));
		XMMATRIX rotationY = XMMatrixRotationY(XMConvertToRadians(lightInfo.perLightInfo[i].rotationY));
		XMVECTOR newDirection = XMVector3Transform(XMLoadFloat3(&direction), rotationX * rotationY);
		XMStoreFloat3(&direction, newDirection);

		XMFLOAT3 upDirection = { 0.0f, 1.0f, 0.0f };
		XMVECTOR newUpDirection = XMVector3Transform(XMLoadFloat3(&upDirection), rotationX * rotationY);
		XMStoreFloat3(&upDirection, newUpDirection);

		float aspectRatio = textureDimension / textureDimension;
		float nearZ = lightInfo.perLightInfo[i].projectionNearZ;
		float farZ = lightInfo.perLightInfo[i].projectionFarZ;
		float angle = XMConvertToRadians(lightInfo.perLightInfo[i].angle);

		// create shadow camera
		CameraD3D11 shadowCamera;
		ProjectionInfo projInfo = { angle * 2, aspectRatio, nearZ, farZ };
		shadowCamera.Initialize(device, projInfo, initialPosition);
		shadowCamera.RotateUp(lightInfo.perLightInfo[i].rotationY);
		shadowCamera.RotateRight(lightInfo.perLightInfo[i].rotationX); // rotations will change
		XMFLOAT4X4 vpMatrix = shadowCamera.GetViewProjectionMatrix();

		this->shadowCameras.push_back(std::move(shadowCamera));

		LightBuffer lightBuffer =
		{
			vpMatrix,
			lightInfo.perLightInfo[i].colour,
			direction,
			angle,
			initialPosition
		};

		this->bufferData.push_back(lightBuffer);
	}

	// create shadow map
	this->shadowMaps.Initialize(device, textureDimension, textureDimension, true, lightInfo.perLightInfo.size());

	// create structured buffer
	this->lightBuffer.Initialize(device, sizeof(LightBuffer), this->bufferData.size(), this->bufferData.data());
}

void SpotLightCollectionD3D11::UpdateLightBuffers(ID3D11DeviceContext* context)
{
	this->lightBuffer.UpdateBuffer(context, this->bufferData.data());
}

UINT SpotLightCollectionD3D11::GetNrOfLights() const
{
	return this->bufferData.size();
}

ID3D11DepthStencilView* SpotLightCollectionD3D11::GetShadowMapDSV(UINT lightIndex) const
{
	return this->shadowMaps.GetDSV(lightIndex);
}

ID3D11ShaderResourceView* SpotLightCollectionD3D11::GetShadowMapsSRV() const
{
	return this->shadowMaps.GetSRV();
}

ID3D11ShaderResourceView* SpotLightCollectionD3D11::GetLightBufferSRV() const
{
	return this->lightBuffer.GetSRV();
}

ID3D11Buffer* SpotLightCollectionD3D11::GetLightCameraConstantBuffer(UINT lightIndex) const
{
	return this->shadowCameras[lightIndex].GetPosConstantBuffer();
}

DirectX::XMFLOAT4X4 SpotLightCollectionD3D11::GetLightViewProjectionMatrix(UINT lightIndex) const
{
	return this->bufferData[lightIndex].vpMatrix;
}
