// ================================================
// Renderer.DX11.cpp
// DirectX 11 implementation of the Renderer interface
// Only compiled when USE_DIRECTX11 is defined
// ================================================
#ifdef USE_DIRECTX11

#include "Renderer.h"
#include "main.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// DX11 device/context (created by GraphicsHelper)
extern ID3D11Device* g_Device;
extern ID3D11DeviceContext* g_Context;
extern ID3D11RenderTargetView* g_RenderTargetView;
extern IDXGISwapChain* g_SwapChain;

// Texture handle table (defined in texture.Win64.DX11.cpp)
extern ID3D11ShaderResourceView* GetSRV(unsigned int handle);

namespace
{
	ID3D11VertexShader* s_VertexShader = nullptr;
	ID3D11PixelShader* s_PixelShader = nullptr;
	ID3D11InputLayout* s_InputLayout = nullptr;
	ID3D11Buffer* s_VertexBuffer = nullptr;
	ID3D11Buffer* s_ProjectionBuffer = nullptr;
	ID3D11Buffer* s_TextureFlagBuffer = nullptr;
	ID3D11BlendState* s_BlendState = nullptr;
	ID3D11SamplerState* s_SamplerState = nullptr;
	ID3D11ShaderResourceView* s_CurrentTextureSRV = nullptr;

	const int MAX_VERTICES = 256;
	float s_OffsetX = 0.0f;
	float s_OffsetY = 0.0f;

	// HLSL vertex shader
	const char* VSSource =
		"cbuffer Proj : register(b0) { row_major float4x4 uProjection; };\n"
		"struct VS_IN { float3 Pos : POSITION; float4 Col : COLOR; float2 UV : TEXCOORD; };\n"
		"struct VS_OUT { float4 Pos : SV_POSITION; float4 Col : COLOR; float2 UV : TEXCOORD; };\n"
		"VS_OUT main(VS_IN i) {\n"
		"  VS_OUT o;\n"
		"  o.Pos = mul(float4(i.Pos, 1.0), uProjection);\n"
		"  o.Col = i.Col;\n"
		"  o.UV = i.UV;\n"
		"  return o;\n"
		"}\n";

	// HLSL pixel shader
	const char* PSSource =
		"Texture2D tex : register(t0);\n"
		"SamplerState samp : register(s0);\n"
		"cbuffer TexFlag : register(b1) { int uTextureEnable; int3 pad; };\n"
		"struct PS_IN { float4 Pos : SV_POSITION; float4 Col : COLOR; float2 UV : TEXCOORD; };\n"
		"float4 main(PS_IN i) : SV_TARGET {\n"
		"  if (uTextureEnable) return i.Col * tex.Sample(samp, i.UV);\n"
		"  else return i.Col;\n"
		"}\n";

	D3D11_PRIMITIVE_TOPOLOGY ToDXTopology(DrawMode mode)
	{
		switch (mode)
		{
		case DrawMode::Triangles:     return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case DrawMode::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case DrawMode::Lines:         return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case DrawMode::LineLoop:      return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		default:                      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}
}


void Renderer::Initialize()
{
	// Compile vertex shader
	if (!g_Device) return;

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* errBlob = nullptr;
	D3DCompile(VSSource, strlen(VSSource), nullptr, nullptr, nullptr,
		"main", "vs_5_0", 0, 0, &vsBlob, &errBlob);
	if (errBlob) { OutputDebugStringA((char*)errBlob->GetBufferPointer()); errBlob->Release(); }
	if (!vsBlob) return;
	g_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &s_VertexShader);

	// Input layout (matches VERTEX_3D)
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,        0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	g_Device->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &s_InputLayout);
	vsBlob->Release();

	// Compile pixel shader
	ID3DBlob* psBlob = nullptr;
	D3DCompile(PSSource, strlen(PSSource), nullptr, nullptr, nullptr,
		"main", "ps_5_0", 0, 0, &psBlob, &errBlob);
	if (errBlob) { OutputDebugStringA((char*)errBlob->GetBufferPointer()); errBlob->Release(); }
	if (!psBlob) return;
	g_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &s_PixelShader);
	psBlob->Release();

	// Dynamic vertex buffer
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(VERTEX_3D) * MAX_VERTICES;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	g_Device->CreateBuffer(&vbDesc, nullptr, &s_VertexBuffer);

	// Projection constant buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(float) * 16;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	g_Device->CreateBuffer(&cbDesc, nullptr, &s_ProjectionBuffer);

	// Texture flag constant buffer
	cbDesc.ByteWidth = 16; // int + padding
	g_Device->CreateBuffer(&cbDesc, nullptr, &s_TextureFlagBuffer);

	// Set projection matrix
	// Orthographic: [0, SCREEN_WIDTH] x [0, SCREEN_HEIGHT], Y-down
	float w = (float)SCREEN_WIDTH;
	float h = (float)SCREEN_HEIGHT;
	float proj[16] = {
		2.0f / w, 0,        0, 0,
		0,       -2.0f / h, 0, 0,
		0,        0,        1, 0,
		-1.0f,    1.0f,     0, 1
	};
	D3D11_MAPPED_SUBRESOURCE mapped;
	g_Context->Map(s_ProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, proj, sizeof(proj));
	g_Context->Unmap(s_ProjectionBuffer, 0);

	// Blend state (alpha blending)
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_Device->CreateBlendState(&blendDesc, &s_BlendState);

	// Sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	g_Device->CreateSamplerState(&sampDesc, &s_SamplerState);
}


void Renderer::Finalize()
{
	if (s_SamplerState) s_SamplerState->Release();
	if (s_BlendState) s_BlendState->Release();
	if (s_TextureFlagBuffer) s_TextureFlagBuffer->Release();
	if (s_ProjectionBuffer) s_ProjectionBuffer->Release();
	if (s_VertexBuffer) s_VertexBuffer->Release();
	if (s_InputLayout) s_InputLayout->Release();
	if (s_PixelShader) s_PixelShader->Release();
	if (s_VertexShader) s_VertexShader->Release();
}


void Renderer::BeginFrame()
{
	float clearColor[4] = { 0, 0, 0, 1 };
	g_Context->ClearRenderTargetView(g_RenderTargetView, clearColor);
}

void Renderer::BeginFrame(float r, float g, float b)
{
	float clearColor[4] = { r, g, b, 1 };
	g_Context->ClearRenderTargetView(g_RenderTargetView, clearColor);
}

void Renderer::EndFrame()
{
	g_SwapChain->Present(0, 0);   // VSync OFF。フレームレートは Game::Process の TARGET_FPS で制御する
	glfwPollEvents();
}


void Renderer::DrawVertices(const VERTEX_3D* vertices, int count, DrawMode mode)
{
	// Apply screen offset
	static VERTEX_3D offsetVerts[256];
	int n = (count < MAX_VERTICES) ? count : MAX_VERTICES;
	for (int i = 0; i < n; i++)
	{
		offsetVerts[i] = vertices[i];
		offsetVerts[i].Position.x += s_OffsetX;
		offsetVerts[i].Position.y += s_OffsetY;
	}

	// Update vertex buffer
	D3D11_MAPPED_SUBRESOURCE mapped;
	g_Context->Map(s_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, offsetVerts, sizeof(VERTEX_3D) * n);
	g_Context->Unmap(s_VertexBuffer, 0);

	// Set pipeline state
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	g_Context->IASetVertexBuffers(0, 1, &s_VertexBuffer, &stride, &offset);
	g_Context->IASetInputLayout(s_InputLayout);
	g_Context->IASetPrimitiveTopology(ToDXTopology(mode));
	g_Context->VSSetShader(s_VertexShader, nullptr, 0);
	g_Context->PSSetShader(s_PixelShader, nullptr, 0);
	g_Context->VSSetConstantBuffers(0, 1, &s_ProjectionBuffer);
	g_Context->PSSetConstantBuffers(1, 1, &s_TextureFlagBuffer);
	g_Context->PSSetSamplers(0, 1, &s_SamplerState);
	g_Context->OMSetRenderTargets(1, &g_RenderTargetView, nullptr);

	float blendFactor[4] = { 0, 0, 0, 0 };
	g_Context->OMSetBlendState(s_BlendState, blendFactor, 0xFFFFFFFF);

	if (s_CurrentTextureSRV)
		g_Context->PSSetShaderResources(0, 1, &s_CurrentTextureSRV);

	// Draw
	g_Context->Draw(n, 0);
}


void Renderer::SetTexture(unsigned int textureId)
{
	// textureId is actually an SRV pointer cast to uint (set by texture loader)
	D3D11_MAPPED_SUBRESOURCE mapped;
	int flag[4] = { (textureId != 0) ? 1 : 0, 0, 0, 0 };
	g_Context->Map(s_TextureFlagBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, flag, 16);
	g_Context->Unmap(s_TextureFlagBuffer, 0);

	s_CurrentTextureSRV = GetSRV(textureId);
}


void Renderer::SetLineWidth(float width)
{
	// DX11 does not support line width (always 1px)
	(void)width;
}


void Renderer::SetScreenOffset(float x, float y)
{
	s_OffsetX = x;
	s_OffsetY = y;
}

#endif // USE_DIRECTX11
