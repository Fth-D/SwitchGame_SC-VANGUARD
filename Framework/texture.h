#pragma once

unsigned int LoadTexture(const char *FileName);
void UnloadTexture(unsigned int Texture);
void UnloadTextures();

#ifdef USE_DIRECTX11
struct ID3D11ShaderResourceView;
unsigned int RegisterSRV(ID3D11ShaderResourceView* srv);
ID3D11ShaderResourceView* GetSRV(unsigned int handle);
#endif
