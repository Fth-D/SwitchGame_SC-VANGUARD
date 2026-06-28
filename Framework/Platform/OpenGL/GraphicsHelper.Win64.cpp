#ifndef USE_DIRECTX11
#include "GraphicsHelper.h"
#include "main.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>

static GLFWwindow* g_Window = nullptr;

void GraphicsHelper::Initialize()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	g_Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SampleGame", nullptr, nullptr);
	if (!g_Window) {
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(g_Window);
	glfwSwapInterval(0);   // VSync OFF。フレームレートは Game::Process の TARGET_FPS で制御する
}

void GraphicsHelper::Finalize()
{
	if (g_Window) {
		glfwDestroyWindow(g_Window);
		g_Window = nullptr;
	}
	glfwTerminate();
}

void GraphicsHelper::SwapBuffers()
{
	if (g_Window) {
		glfwSwapBuffers(g_Window);
		glfwPollEvents();
	}
}

void GraphicsHelper::Resize(int width, int height)
{
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
}

// Utility for game loop
bool PlatformShouldClose()
{
	return g_Window ? glfwWindowShouldClose(g_Window) != 0 : true;
}

GLFWwindow* PlatformGetWindow()
{
	return g_Window;
}

#endif // !USE_DIRECTX11
