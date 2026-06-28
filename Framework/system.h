#pragma once

#include "Platform/PlatformDefines.h"
#include "Platform/PlatformTypes.h"

#if PLATFORM_NX
  #if defined(NN_BUILD_CONFIG_OS_HORIZON)
  #include <nn/oe.h>
  #endif
  #include <nn/os.h>
  #include <nn/hid.h>
  #include <nn/fs.h>
  #include <nn/gll.h>
  #include "GraphicsHelper.h"
#elif defined(USE_DIRECTX11)
  #define NOGDI           // Windows GDI の Rectangle() 関数との名前衝突を回避
  #define NOMINMAX        // min/max マクロを無効化
  #include <d3d11.h>
  #include <GLFW/glfw3.h>
  #include "GraphicsHelper.h"
#else
  #include <glad/glad.h>
  #include <GLFW/glfw3.h>
  #include "GraphicsHelper.h"
#endif


void InitSystem();
void UninitSystem();

void SwapBuffers();
