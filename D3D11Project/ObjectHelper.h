#pragma once

#include <array>
#include <d3d11.h>
#include <DirectXMath.h>

#include "CameraD3D11.h"
#include "VertexBufferD3D11.h"
#include "MeshD3D11.h"
#include "ShaderResourceTextureD3D11.h"
#include "SamplerD3D11.h"

bool SetupObjects
(
	ID3D11Device* device,

	// cameras
	CameraD3D11* camera1,

	// screen dimensions
	const UINT width,
	const UINT height
);