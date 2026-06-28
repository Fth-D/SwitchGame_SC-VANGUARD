#pragma once
#include "Platform/PlatformTypes.h"

class Camera
{
private:
	Float2 position;
	[[maybe_unused]] float screenWidth;
	[[maybe_unused]] float screenHeight;

	Camera();

public:
	~Camera();

	void SetPosition(Float2 newPos) { position = newPos; }
	Float2 GetPosition() { return position; }

	void Move(Float2 moveValue);

	Float2 WorldToScreen(Float3 objectPos);


	static Camera& GetInstance() { static Camera camera;return camera; }
};