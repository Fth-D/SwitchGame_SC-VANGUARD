// ================================================
// texture.Win64.DX11.cpp
// TGA texture loader for DirectX 11
// Returns SRV pointer cast to unsigned int
// Only compiled when USE_DIRECTX11 is defined
// ================================================
#ifdef USE_DIRECTX11

#include "texture.h"
#include <d3d11.h>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

extern ID3D11Device* g_Device;

static std::map<std::string, unsigned int> loadedTexture;

// Handle table: store SRV pointers, return index as handle
static const int MAX_TEXTURES = 256;
static ID3D11ShaderResourceView* g_SRVTable[MAX_TEXTURES] = {};
static int g_NextSRVSlot = 1;  // 0 = no texture

unsigned int RegisterSRV(ID3D11ShaderResourceView* srv)
{
	if (g_NextSRVSlot >= MAX_TEXTURES) return 0;
	int slot = g_NextSRVSlot++;
	g_SRVTable[slot] = srv;
	return (unsigned int)slot;
}

ID3D11ShaderResourceView* GetSRV(unsigned int handle)
{
	if (handle == 0 || handle >= MAX_TEXTURES) return nullptr;
	return g_SRVTable[handle];
}


static std::string ResolvePath(const char* filename)
{
	std::string path(filename);
	if (path.rfind("rom:/", 0) == 0)
		path = "rom/" + path.substr(5);
	return path;
}

unsigned int LoadTexture(const char* FileName)
{
	std::string resolved = ResolvePath(FileName);

	auto it = loadedTexture.find(resolved);
	if (it != loadedTexture.end())
		return it->second;

	FILE* fp = fopen(resolved.c_str(), "rb");
	if (!fp) { printf("Failed to open texture: %s\n", resolved.c_str()); return 0; }

	unsigned char header[18];
	fread(header, 1, 18, fp);

	int width = header[12] | (header[13] << 8);
	int height = header[14] | (header[15] << 8);
	int bpp = header[16] / 8;

	int imageSize = width * height * bpp;
	unsigned char* imageData = new unsigned char[imageSize];
	fread(imageData, 1, imageSize, fp);
	fclose(fp);

	// Swap R and B (TGA is BGR)
	for (int i = 0; i < imageSize; i += bpp)
	{
		unsigned char tmp = imageData[i];
		imageData[i] = imageData[i + 2];
		imageData[i + 2] = tmp;
	}

	// Convert to RGBA if needed
	unsigned char* rgbaData = nullptr;
	if (bpp == 3)
	{
		rgbaData = new unsigned char[width * height * 4];
		for (int i = 0; i < width * height; i++)
		{
			rgbaData[i * 4 + 0] = imageData[i * 3 + 0];
			rgbaData[i * 4 + 1] = imageData[i * 3 + 1];
			rgbaData[i * 4 + 2] = imageData[i * 3 + 2];
			rgbaData[i * 4 + 3] = 255;
		}
	}
	else
	{
		rgbaData = imageData;
	}

	// Create DX11 texture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = rgbaData;
	initData.SysMemPitch = width * 4;

	ID3D11Texture2D* texture = nullptr;
	g_Device->CreateTexture2D(&texDesc, &initData, &texture);

	ID3D11ShaderResourceView* srv = nullptr;
	if (texture)
	{
		g_Device->CreateShaderResourceView(texture, nullptr, &srv);
		texture->Release();
	}

	if (bpp == 3) delete[] rgbaData;
	delete[] imageData;

	unsigned int handle = RegisterSRV(srv);
	loadedTexture[resolved] = handle;
	return handle;
}

void UnloadTexture(unsigned int texture)
{
	if (texture != 0)
	{
		ID3D11ShaderResourceView* srv = GetSRV(texture);
		if (srv) srv->Release();
		g_SRVTable[texture] = nullptr;
	}
}

void UnloadTextures()
{
	for (auto& pair : loadedTexture)
	{
		ID3D11ShaderResourceView* srv = GetSRV(pair.second);
		if (srv) srv->Release();
	}
	loadedTexture.clear();
}

#endif // USE_DIRECTX11
