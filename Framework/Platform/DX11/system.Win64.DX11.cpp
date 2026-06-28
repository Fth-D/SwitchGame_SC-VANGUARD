// ================================================
// system.Win64.DX11.cpp
// System init for DirectX 11 (no GLAD/GL)
// Only compiled when USE_DIRECTX11 is defined
// ================================================
#ifdef USE_DIRECTX11

#include "main.h"
#include <cstdio>

GraphicsHelper g_GraphicsHelper;

void InitSystem()
{
	
	g_GraphicsHelper.Initialize();
	
	
}

void UninitSystem()
{
	g_GraphicsHelper.Finalize();
}

void SwapBuffers()
{
	g_GraphicsHelper.SwapBuffers();
}

#endif // USE_DIRECTX11
