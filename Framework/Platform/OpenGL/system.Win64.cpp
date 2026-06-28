#ifndef USE_DIRECTX11
#include "main.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>

GraphicsHelper g_GraphicsHelper;


static void DebugCallbackFunc(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	(void)length; (void)userParam;
	printf("GL Debug: source=0x%x type=0x%x id=0x%x severity=0x%x msg=%s\n",
		source, type, id, severity, message);
}


void InitSystem()
{
	// ウィンドウ生成
	g_GraphicsHelper.Initialize();

	// OpenGL関数ロード
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		return;
	}

	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
	printf("GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// シェーダ・VAO・VBO・レンダーステートの初期化は Renderer::Initialize() で行う
}


void UninitSystem()
{
	// シェーダ・VAO・VBO の解放は Renderer::Finalize() で行う

	g_GraphicsHelper.Finalize();
}


void SwapBuffers()
{
	g_GraphicsHelper.SwapBuffers();
}

#endif // !USE_DIRECTX11
