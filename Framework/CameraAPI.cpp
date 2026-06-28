#include "Camera.h"
#include "CameraAPI.h"
// PlatformTypes.h included via CameraAPI.h

void MoveCamera(Float2 moveValue)
{
	Camera::GetInstance().Move(moveValue);
}

Float2 WorldToScreen(Float3 objectPos)
{
	return Camera::GetInstance().WorldToScreen(objectPos);
}

void ResetCamera()
{
	Camera::GetInstance().SetPosition(MakeFloat2(0.0f, 0.0f));
}