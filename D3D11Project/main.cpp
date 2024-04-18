#include <Windows.h>
#include <iostream>
#include <fstream>
#include <d3d11.h>
#include <DirectXMath.h>
#include <chrono>

// helper includes
#include "WindowHelper.h"
#include "D3D11Helper.h"
#include "PipelineHelper.h"
#include "ObjectHelper.h"
#include "InputHelper.h"
#include "RenderHelper.h"

// class includes
#include "ShaderD3D11.h"
#include "SpotLightCollectionD3D11.h"
#include "SceneObjectReflective.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	// Redirect std::cerr to the file
	const char* fileName = "error_log.txt";
	std::ofstream fileStream(fileName);
	std::cerr.rdbuf(fileStream.rdbuf());

	const UINT WIDTH = 1280;
	const UINT HEIGHT = 720;
	const UINT shadowTextureDimension = 1024;
	const UINT reflectionCubeMapDimension = 1280;
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}

	ID3D11Device* device;
	ID3D11DeviceContext* immediateContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* dsTexture;
	ID3D11DepthStencilView* dsView;
	D3D11_VIEWPORT viewport;
	D3D11_VIEWPORT shadowViewport;

	// output related
	ID3D11Texture2D* backBuffer;
	ID3D11Texture2D* outputTexture;
	ID3D11UnorderedAccessView* outputUAV;
	ID3D11RenderTargetView* outputRTV;

	// shaders
	ShaderD3D11* vShader = new ShaderD3D11();
	ShaderD3D11* pShader = new ShaderD3D11();
	ShaderD3D11* pShaderDeferred = new ShaderD3D11();
	ShaderD3D11* pShaderCubeMap = new ShaderD3D11();
	ShaderD3D11* cShader = new ShaderD3D11();

	// input layouts
	InputLayoutD3D11* inputLayout = new InputLayoutD3D11();

	if (!SetupD3D11
	(
		WIDTH, HEIGHT, window, 
		device, immediateContext, 
		swapChain, backBuffer,
		rtv, outputTexture, outputUAV, outputRTV,
		dsTexture, dsView, viewport, shadowViewport, shadowTextureDimension
	))
	{
		std::cerr << "Failed to setup d3d11!" << std::endl;
		return -1;
	}
	
	// TODO: initialize cShaderParticles
	if (!SetupPipeline(device, vShader, pShader, pShaderDeferred, pShaderCubeMap, cShader, inputLayout))
	{
		std::cerr << "Failed to setup pipeline!" << std::endl;
		return -1;
	}

	// TODO: move this to a helper function
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(rsDesc));
	rsDesc.FillMode = D3D11_FILL_SOLID; // fill solid or wireframe
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	ID3D11RasterizerState* rsState;
	device->CreateRasterizerState(&rsDesc, &rsState);
	immediateContext->RSSetState(rsState);	// set as default

	// Particles shaders
	ShaderD3D11* vShaderParticles = new ShaderD3D11(device, ShaderType::VERTEX_SHADER, L"VertexShaderParticles.cso");
	ShaderD3D11* gShaderParticles = new ShaderD3D11(device, ShaderType::GEOMETRY_SHADER, L"GeometryShaderParticles.cso");
	ShaderD3D11* pShaderParticles = new ShaderD3D11(device, ShaderType::PIXEL_SHADER, L"PixelShaderParticles.cso");
	ShaderD3D11* cShaderParticles = new ShaderD3D11(device, ShaderType::COMPUTE_SHADER, L"ComputeShaderParticles.cso");

	// LOD shaders
	ShaderD3D11* hShaderLOD = new ShaderD3D11(device, ShaderType::HULL_SHADER, L"HullShaderLOD.cso");
	ShaderD3D11* dShaderLOD = new ShaderD3D11(device, ShaderType::DOMAIN_SHADER, L"DomainShaderLOD.cso");

	// Bounding box shaders
	ShaderD3D11* vShaderBB = new ShaderD3D11(device, ShaderType::VERTEX_SHADER, L"VertexShaderBB.cso");
	ShaderD3D11* pShaderBB = new ShaderD3D11(device, ShaderType::PIXEL_SHADER, L"PixelShaderBB.cso");

	// samplers
	SamplerD3D11* sampler = new SamplerD3D11(device, D3D11_TEXTURE_ADDRESS_WRAP, std::nullopt);
	std::array<float, 4> borderColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	SamplerD3D11* shadowSampler = new SamplerD3D11(device, D3D11_TEXTURE_ADDRESS_BORDER, borderColor);

	// Gbuffers for screen
	GeometryBufferD3D11* Gbuffer1 = new GeometryBufferD3D11(device, WIDTH, HEIGHT);
	GeometryBufferD3D11* Gbuffer2 = new GeometryBufferD3D11(device, WIDTH, HEIGHT);
	GeometryBufferD3D11* Gbuffer3 = new GeometryBufferD3D11(device, WIDTH, HEIGHT);
	GeometryBufferD3D11* Gbuffer4 = new GeometryBufferD3D11(device, WIDTH, HEIGHT);

	// Gbuffers for cube map
	GeometryBufferD3D11* GbufferCubeMap1 = new GeometryBufferD3D11(device, reflectionCubeMapDimension, reflectionCubeMapDimension);
	GeometryBufferD3D11* GbufferCubeMap2 = new GeometryBufferD3D11(device, reflectionCubeMapDimension, reflectionCubeMapDimension);
	GeometryBufferD3D11* GbufferCubeMap3 = new GeometryBufferD3D11(device, reflectionCubeMapDimension, reflectionCubeMapDimension);
	GeometryBufferD3D11* GbufferCubeMap4 = new GeometryBufferD3D11(device, reflectionCubeMapDimension, reflectionCubeMapDimension);

	// lights
	SpotLightCollectionD3D11* spotLightCollection = new SpotLightCollectionD3D11();
	SpotLightData spotLightData;
	spotLightData.shadowMapInfo = { shadowTextureDimension };
	spotLightData.perLightInfo.push_back(
	{
		DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),		// color
		90.0f,										// rotationX
		0.0f,										// rotationY
		40.0f,										// angle
		0.1f,										// projectionNearZ
		1000.0f,									// projectionFarZ
		DirectX::XMFLOAT3(0.0f, 30.0f, 0.0f)		// initialPosition
	});
	//spotLightData.perLightInfo.push_back(
	//{
	//	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),		// color
	//	60.0f,										// rotationX
	//	180.0f,										// rotationY
	//	40.0f,										// angle
	//	0.1f,										// projectionNearZ
	//	1000.0f,									// projectionFarZ
	//	DirectX::XMFLOAT3(5.0f, 20.0f, 20.0f)		// initialPosition
	//});
	spotLightCollection->Initialize(device, spotLightData);


	ProjectionInfo projectionInfo = 
	{
		DirectX::XMConvertToRadians(50),	// fovAngleY
		(float)WIDTH / (float)HEIGHT,		// aspectRatio
		0.1f,								// nearZ
		100.0f								// farZ
	};
	CameraD3D11* camera1 = new CameraD3D11(device, projectionInfo, DirectX::XMFLOAT3(5.0f, 5.0f, -30.0f));

	// scene creation
	Scene* scene = new Scene(device, camera1);	// TODO: make spotlightCollection part of initialization
	scene->SetSpotLightCollection(device, spotLightCollection);

	// objects

	// axis
	SceneObject* xaxis = new SceneObject(device, DirectX::XMFLOAT3(0, 0, 0), 1.0f, "xaxis");
	SceneObject* yaxis = new SceneObject(device, DirectX::XMFLOAT3(0, 0, 0), 1.0f, "yaxis");
	SceneObject* zaxis = new SceneObject(device, DirectX::XMFLOAT3(0, 0, 0), 1.0f, "zaxis");
	//zaxis->RotateUp(180.0f);
	scene->AddObject(xaxis);
	scene->AddObject(yaxis);
	scene->AddObject(zaxis);

	SceneObject* HippoGray = new SceneObject(device, DirectX::XMFLOAT3(20, 0, 20), 1.f, "HippoGray");
	scene->AddObject(HippoGray);

	SceneObject* Bear = new SceneObject(device, DirectX::XMFLOAT3(20, 0, -20), 1.0f, "Bear");
	scene->AddObject(Bear);

	SceneObject* Penguin = new SceneObject(device, DirectX::XMFLOAT3(-20, 1.4, -20), 1.0f, "Penguin");
	scene->AddObject(Penguin);

	SceneObject* Horse = new SceneObject(device, DirectX::XMFLOAT3(-20, 0, 20), 1.0f, "Horse");
	scene->AddObject(Horse);

	SceneObject* cheeseTriangle = new SceneObject(device, DirectX::XMFLOAT3(20, 15, 20), 1.0f, "cheesetriangle");
	scene->AddObject(cheeseTriangle);

	//SceneObject* floor = new SceneObject(device, DirectX::XMFLOAT3(0, 0, 0), 3.0f, "floor");
	//scene->AddObject(floor);

	//SceneObject* Rock = new SceneObject(device, DirectX::XMFLOAT3(0, 0, 0), 1.0f, "rock");
	//scene->AddObject(Rock);

	// reflective objects
	SceneObjectReflective* reflectiveCube = new SceneObjectReflective(device, DirectX::XMFLOAT3(0, 0, 0), 5.0f, reflectionCubeMapDimension, "sphere");
	//scene->AddReflectiveObject(reflectiveCube);

	// particles
	Particles* particles = new Particles(device, 2560);
	scene->SetParticles(particles);

	scene->Finalize();

	static auto previousTime = std::chrono::high_resolution_clock::now();

	MSG msg = { };
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// DeltaTime calculation
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();
		previousTime = currentTime;

		// hotkeys are only allowed if the window is in focus
		if (GetActiveWindow() == window)
		{
			ShowCursor(false);
			HandleInput(window, scene, deltaTime);
		}

		//cheeseTriangle->RotateUp(30.0f * deltaTime);

		scene->Update(immediateContext, cShaderParticles, device);

		// Render the scene

		Render
		(
			immediateContext,
			rsState,
			scene,
			outputTexture, backBuffer, outputUAV, outputRTV,
			{ Gbuffer1, Gbuffer2, Gbuffer3, Gbuffer4 },
			{ GbufferCubeMap1, GbufferCubeMap2, GbufferCubeMap3, GbufferCubeMap4 },
			dsView, viewport, shadowViewport,
			vShader, inputLayout,
			hShaderLOD, dShaderLOD,
			pShaderDeferred, pShaderCubeMap,
			cShader, { 64, 36, 1 },
			vShaderParticles, gShaderParticles, 
			pShaderParticles, cShaderParticles,
			vShaderBB, pShaderBB,
			sampler, shadowSampler
		);

		swapChain->Present(0, 0);
	}

	// ------ CLEANUP -------

	// cameras
	delete camera1;

	// scenes
	delete scene;

	// lights
	delete spotLightCollection;

	// Gbuffers
	delete Gbuffer1;
	delete Gbuffer2;
	delete Gbuffer3;
	delete Gbuffer4;

	delete GbufferCubeMap1;
	delete GbufferCubeMap2;
	delete GbufferCubeMap3;
	delete GbufferCubeMap4;

	// other resources
	delete inputLayout;
	delete sampler;
	delete shadowSampler;

	// shaders
	delete vShader;
	delete pShaderDeferred;
	delete pShaderCubeMap;
	delete cShader;

	delete vShaderParticles;
	delete gShaderParticles;
	delete pShaderParticles;
	delete cShaderParticles;



	// --- D3D11 ---

	// output related
	outputTexture->Release();
	outputUAV->Release();
	backBuffer->Release();
	rtv->Release();
	swapChain->Release();

	// other
	dsView->Release();
	dsTexture->Release();
	immediateContext->Release();
	device->Release();

	fileStream.close(); // error_log.txt

	return 0;
}