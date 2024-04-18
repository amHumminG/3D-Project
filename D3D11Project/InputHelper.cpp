#include "InputHelper.h"

void ToggleLight(Scene* scene, float deltaTime)
{
	float waitTime = 0.2f;
	static float time = 0.0f;
	time -= deltaTime;
	 if (GetKeyState('H') & 0x8000)
	 {
	 	if (time < 0)
	 	{
	 		scene->ToggleLights();	
	 		time = waitTime;
	 	}
	 }
}

void HandleKeyInput(Scene* scene, float deltaTime)
{
	// Camera movement WASD, Space and Ctrl
	CameraD3D11* camera = scene->GetActiveCamera();

	float moveSpeed = 15.0f;
	float increment = moveSpeed * deltaTime;

	if (GetKeyState('W') & 0x8000)
		camera->MoveForward(increment);

	else if (GetKeyState('S') & 0x8000)
		camera->MoveForward(-increment);

	else if (GetKeyState('A') & 0x8000)
		camera->MoveRight(-increment);

	else if (GetKeyState('D') & 0x8000)
		camera->MoveRight(increment);

	if (GetKeyState(VK_CONTROL) & 0x8000)
		camera->MoveUp(-increment);

	if (GetKeyState(VK_SPACE) & 0x8000)
		camera->MoveUp(increment);

	ToggleLight(scene, deltaTime);
}

void HandleMouseInput(HWND& window, CameraD3D11* camera, float deltaTime)
{
	// get the current mouse position
	POINT mousePos;
	GetCursorPos(&mousePos);

	// get the window's position
	RECT windowRect;
	GetWindowRect(window, &windowRect);

	// get the window's center
	POINT windowCenter;

	windowCenter.x = windowRect.left + (windowRect.right - windowRect.left) / 2;
	windowCenter.y = windowRect.top + (windowRect.bottom - windowRect.top) / 2;

	// set the cursor position to the window's center
	SetCursorPos(windowCenter.x, windowCenter.y);

	// calculate the change in mouse position
	float dx = (float)(mousePos.x - windowCenter.x);
	float dy = (float)(mousePos.y - windowCenter.y);

	// rotate the camera
	float sensitivity = 14.0f;
	float increment = sensitivity * deltaTime;
	camera->RotateUp(dx * increment);
	camera->RotateRight(dy * increment);
}


void HandleInput(HWND& window, Scene* scene, float deltaTime)
{
	HandleKeyInput(scene, deltaTime);
	HandleMouseInput(window, scene->GetActiveCamera(), deltaTime);
}
