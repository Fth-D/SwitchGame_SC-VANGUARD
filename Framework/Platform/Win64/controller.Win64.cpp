#include "main.h"
#include "controller.h"
#include <GLFW/glfw3.h>
#include <cstring>

// Forward declaration from GraphicsHelper.Win64.cpp
struct GLFWwindow;
GLFWwindow* PlatformGetWindow();

static bool g_KeyState[512] = {};
static bool g_KeyStatePrev[512] = {};
static bool g_MouseState[8] = {};
static bool g_MouseStatePrev[8] = {};

// Button::ID -> GLFW key mapping (-1 = mouse button, handled separately)
static int g_ButtonToKey[Button::COUNT] = {
	GLFW_KEY_ENTER,     // A
	GLFW_KEY_ESCAPE,    // B
	GLFW_KEY_Z,         // X
	GLFW_KEY_SPACE,     // Y
	GLFW_KEY_Q,         // L
	GLFW_KEY_E,         // R
	GLFW_KEY_1,         // ZL
	GLFW_KEY_3,         // ZR
	GLFW_KEY_P,         // Plus
	GLFW_KEY_O,         // Minus
	GLFW_KEY_UP,        // Up
	GLFW_KEY_DOWN,      // Down
	GLFW_KEY_LEFT,      // Left
	GLFW_KEY_RIGHT,     // Right
	-1,                 // MouseLeft (special handling)
	-2,                 // MouseRight (special handling)
};

// Mouse button index for Button::MouseLeft / MouseRight
static int g_ButtonToMouse[Button::COUNT] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	GLFW_MOUSE_BUTTON_LEFT,   // MouseLeft
	GLFW_MOUSE_BUTTON_RIGHT,  // MouseRight
};

void InitController() {}
void UninitController() {}

void UpdateController()
{
	memcpy(g_KeyStatePrev, g_KeyState, sizeof(g_KeyState));
	memcpy(g_MouseStatePrev, g_MouseState, sizeof(g_MouseState));
	GLFWwindow* win = PlatformGetWindow();
	if (!win) return;
	for (int i = 0; i < 512; i++) {
		g_KeyState[i] = (glfwGetKey(win, i) == GLFW_PRESS);
	}
	for (int i = 0; i < 8; i++) {
		g_MouseState[i] = (glfwGetMouseButton(win, i) == GLFW_PRESS);
	}
}

bool GetControllerPress(int button)
{
	// Key::ID (物理キー直指定) の場合は g_KeyState を直接参照
	if (button >= Key::OFFSET) {
		int k = button - Key::OFFSET;
		return (k >= 0 && k < 512) ? g_KeyState[k] : false;
	}
	if (button < 0 || button >= Button::COUNT) return false;
	if (g_ButtonToKey[button] < 0) {
		int mb = g_ButtonToMouse[button];
		return (mb >= 0) ? g_MouseState[mb] : false;
	}
	return g_KeyState[g_ButtonToKey[button]];
}

bool GetControllerTrigger(int button)
{
	if (button >= Key::OFFSET) {
		int k = button - Key::OFFSET;
		return (k >= 0 && k < 512) ? (g_KeyState[k] && !g_KeyStatePrev[k]) : false;
	}
	if (button < 0 || button >= Button::COUNT) return false;
	if (g_ButtonToKey[button] < 0) {
		int mb = g_ButtonToMouse[button];
		return (mb >= 0) ? (g_MouseState[mb] && !g_MouseStatePrev[mb]) : false;
	}
	int key = g_ButtonToKey[button];
	return g_KeyState[key] && !g_KeyStatePrev[key];
}

bool GetControllerRelease(int button)
{
	if (button >= Key::OFFSET) {
		int k = button - Key::OFFSET;
		return (k >= 0 && k < 512) ? (!g_KeyState[k] && g_KeyStatePrev[k]) : false;
	}
	if (button < 0 || button >= Button::COUNT) return false;
	if (g_ButtonToKey[button] < 0) {
		int mb = g_ButtonToMouse[button];
		return (mb >= 0) ? (!g_MouseState[mb] && g_MouseStatePrev[mb]) : false;
	}
	int key = g_ButtonToKey[button];
	return !g_KeyState[key] && g_KeyStatePrev[key];
}

Float2 GetControllerLeftStick()
{
	// WASD emulation
	float x = 0.0f, y = 0.0f;
	GLFWwindow* win = PlatformGetWindow();
	if (win) {
		if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) x -= 1.0f;
		if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) x += 1.0f;
		if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) y -= 1.0f;
		if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) y += 1.0f;
	}
	return MakeFloat2(x, y);
}

Float2 GetControllerRightStick() { return MakeFloat2(0.0f, 0.0f); }

// Vibration stubs
void SetControllerLeftVibration(int) {}
void SetControllerRightVibration(int) {}
void SetControllerLeftVibration(int, float) {}
void SetControllerRightVibration(int, float) {}
void SetControllerLeftVibration(int, float, float, float, float) {}
void SetControllerRightVibration(int, float, float, float, float) {}

// 6-axis stubs
Float3 GetControllerLeftAcceleration() { return MakeFloat3(0,0,0); }
Float3 GetControllerRightAcceleration() { return MakeFloat3(0,0,0); }
void SetControllerLeftAngleBase() {}
void SetControllerRightAngleBase() {}
Float3 GetControllerLeftAngle() { return MakeFloat3(0,0,0); }
Float3 GetControllerRightAngle() { return MakeFloat3(0,0,0); }

// Touch screen stubs (mouse emulation)
bool GetControllerTouchScreen() { return false; }
int GetControllerTouchScreenCount() { return 0; }
Float2 GetControllerTouchScreenPosition(int) { return MakeFloat2(0,0); }
