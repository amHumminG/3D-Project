#include "ObjectHelper.h"

#include <fstream>
#include <string>
#include <iostream>
#include <chrono>

bool CreateCamera(ID3D11Device* device, CameraD3D11* camera, DirectX::XMFLOAT3 initialPosition, float width, float height)
{
	ProjectionInfo projInfo =
	{	
		DirectX::XMConvertToRadians(50),	// fovAngleY
		width / height,						// aspectRatio
		0.1f,								// nearZ
		1000.0f								// farZ
	};
	return camera->Initialize(device, projInfo, initialPosition);
}

bool SetupObjects
(
	ID3D11Device* device,

	// cameras
	CameraD3D11* camera,

	// screen dimensions
	const UINT width,
	const UINT height
)
{
	if (!CreateCamera(device, camera, DirectX::XMFLOAT3(5.0f, 5.0f, 30.0f), width, height))
	{
		std::cerr << "Error creating camera!" << std::endl;
		return false;
	}

    return true;
}
