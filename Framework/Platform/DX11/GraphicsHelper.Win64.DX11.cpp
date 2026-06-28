// ================================================
// GraphicsHelper.Win64.DX11.cpp
// DX11 device, swap chain, render target creation
// Only compiled when USE_DIRECTX11 is defined
// ================================================
#ifdef USE_DIRECTX11

#include "GraphicsHelper.h"
#include "main.h"
#include <d3d11.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <cstdio>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Global DX11 objects (used by Renderer, texture, etc.)
ID3D11Device* g_Device = nullptr;
ID3D11DeviceContext* g_Context = nullptr;
IDXGISwapChain* g_SwapChain = nullptr;
ID3D11RenderTargetView* g_RenderTargetView = nullptr;

static GLFWwindow* g_Window = nullptr;

void GraphicsHelper::Initialize()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // No OpenGL context
	g_Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SampleGame (DX11)", nullptr, nullptr);
	if (!g_Window) { printf("Failed to create window\n"); return; }

	HWND hwnd = glfwGetWin32Window(g_Window);

	// Create device + swap chain
	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount = 1;
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hwnd;
	scd.SampleDesc.Count = 1;
	scd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		0, nullptr, 0,
		D3D11_SDK_VERSION, &scd, &g_SwapChain,
		&g_Device, &featureLevel, &g_Context);

	if (FAILED(hr)) { printf("D3D11CreateDeviceAndSwapChain failed: 0x%lx\n", hr); return; }

	// Create render target view
	ID3D11Texture2D* backBuffer = nullptr;
	g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (backBuffer)
	{
		g_Device->CreateRenderTargetView(backBuffer, nullptr, &g_RenderTargetView);
		backBuffer->Release();
	}

	// Set viewport
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)SCREEN_WIDTH;
	vp.Height = (float)SCREEN_HEIGHT;
	vp.MaxDepth = 1.0f;
	g_Context->RSSetViewports(1, &vp);

	printf("DirectX 11 initialized (Feature Level: 0x%x)\n", featureLevel);
}

void GraphicsHelper::Finalize()
{
	if (g_RenderTargetView) g_RenderTargetView->Release();
	if (g_SwapChain) g_SwapChain->Release();
	if (g_Context) g_Context->Release();
	if (g_Device) g_Device->Release();
	if (g_Window) glfwDestroyWindow(g_Window);
	glfwTerminate();
}

void GraphicsHelper::SwapBuffers()
{
	// Swap is handled by Renderer::EndFrame via g_SwapChain->Present
	glfwPollEvents();
}

void GraphicsHelper::Resize(int width, int height)
{
	(void)width; (void)height;
}

bool PlatformShouldClose()
{
	return g_Window ? glfwWindowShouldClose(g_Window) != 0 : true;
}

GLFWwindow* PlatformGetWindow()
{
	return g_Window;
}

#endif // USE_DIRECTX11
