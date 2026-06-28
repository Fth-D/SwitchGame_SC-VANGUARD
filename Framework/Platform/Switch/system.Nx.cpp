#include "main.h"


char* g_CacheBuffer = NULL;

GraphicsHelper g_GraphicsHelper;


void DebugCallbackFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	NN_UNUSED(length);
	NN_UNUSED(userParam);

	NN_LOG("GL Debug Callback:\n");
	NN_LOG("  source:       0x%08x\n", source);
	NN_LOG("  type:         0x%08x\n", type);
	NN_LOG("  id:           0x%08x\n", id);
	NN_LOG("  severity:     0x%08x\n", severity);
	NN_LOG("  message:      %s\n", message);

	NN_ASSERT(type != GL_DEBUG_TYPE_ERROR, "GL Error occurs.");
}



void InitSystem()
{

	// ファイルシステムマウント
	{
		nn::Result result;
		size_t cacheSize = 0;

		nn::fs::QueryMountRomCacheSize(&cacheSize);

		g_CacheBuffer = new char[cacheSize];
		if (g_CacheBuffer == NULL)
		{
			NN_ASSERT(false, "Cache buffer is null.\n");
			return;
		}

		result = nn::fs::MountRom("rom", g_CacheBuffer, cacheSize);
		NN_ABORT_UNLESS_RESULT_SUCCESS(result);
	}



	// OpenGL初期化
	{
		g_GraphicsHelper.Initialize();

		nngllResult gllResult = nngllInitializeGl();
		NN_UNUSED(gllResult);
		NN_ASSERT_EQUAL(gllResult, nngllResult_Succeeded);

		NN_LOG("GL_VERSION: %s\n", glGetString(GL_VERSION));
		NN_LOG("GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		glDebugMessageCallback(DebugCallbackFunc, NULL);

		g_GraphicsHelper.Resize(SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	// シェーダ・VAO・VBO・レンダーステートの初期化は Renderer::Initialize() で行う
}




void UninitSystem()
{
	// シェーダ・VAO・VBO の解放は Renderer::Finalize() で行う

	g_GraphicsHelper.Finalize();


	// ファイルシステムアンマウント
	{
		nn::fs::Unmount("rom");

		delete[] g_CacheBuffer;
		g_CacheBuffer = NULL;
	}
}


void SwapBuffers()
{
	g_GraphicsHelper.SwapBuffers();
}
