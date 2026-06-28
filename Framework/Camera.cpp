#include "Camera.h"
#include "main.h"
#include "Platform/PlatformTypes.h"

Camera::Camera()
{
	position = MakeFloat2(0.0f, 0.0f);
}

Camera::~Camera()
{

}

void Camera::Move(Float2 moveValue)
{
	Float2 currentPos = GetPosition();
	Float2 newPos = MakeFloat2(currentPos.x + moveValue.x, currentPos.y + moveValue.y);

	SetPosition(newPos);
}

Float2 Camera::WorldToScreen(Float3 objectPos)
{
	Float2 screenPos;
	Float2 cameraPos = GetPosition();

	// 射影行列を Win64/Switch 両方で同じ「左上原点」に統一したので、
	// スクリーン中央までのオフセットを常に加算する
	float horizontalOffset = SCREEN_WIDTH / 2.0f;
	float verticalOffset = SCREEN_HEIGHT / 2.0f;

	screenPos.x = objectPos.x - cameraPos.x + horizontalOffset;
	screenPos.y = objectPos.y - cameraPos.y + verticalOffset;

	return screenPos;
}