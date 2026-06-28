// ================================================
// Text.cpp
// TrueTypeフォント描画の実装
//
// stb_truetype を使ってフォントをビットマップにベイクし、
// OpenGLテクスチャとして描画する。
// ASCII・ひらがな・カタカナ・CJK句読点に対応。
// ================================================

#include "Text.h"
#include "Renderer.h"
#include "main.h"
#include "texture.h"
#include "Platform/PlatformDefines.h"

#include <cstdio>
#include <cstring>

#ifdef USE_DIRECTX11
extern ID3D11Device* g_Device;
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


// ================================================
// 内部定義
// ================================================
namespace
{
	// --- 文字範囲 ---
	struct CharRange
	{
		int firstCodepoint;
		int numChars;
		stbtt_packedchar* charData;
	};

	// サポートする文字範囲
	const int NUM_RANGES = 5;
	CharRange s_Ranges[NUM_RANGES] = {
		{ 32,     95,     nullptr },   // ASCII (スペース〜~)
		{ 0x3000, 64,     nullptr },   // CJK句読点（、。「」等）
		{ 0x3040, 96,     nullptr },   // ひらがな
		{ 0x30A0, 96,     nullptr },   // カタカナ
		{ 0x4E00, 20992,  nullptr },   // CJK統合漢字
	};

	// --- アトラス ---
	unsigned int s_AtlasTextureId = 0;
	const int ATLAS_SIZE = 4096;
	float s_BakedSize = 48.0f;

	// --- 描画色 ---
	Float4 s_Color = { 1.0f, 1.0f, 1.0f, 1.0f };


	// ================================================
	// UTF-8 デコード（1文字分）
	//  戻り値: Unicodeコードポイント
	//  bytesConsumed: 消費したバイト数
	// ================================================
	int DecodeUTF8(const char* text, int* bytesConsumed)
	{
		unsigned char c = (unsigned char)text[0];

		if (c < 0x80)
		{
			*bytesConsumed = 1;
			return c;
		}
		if ((c & 0xE0) == 0xC0)
		{
			*bytesConsumed = 2;
			return ((c & 0x1F) << 6) | (text[1] & 0x3F);
		}
		if ((c & 0xF0) == 0xE0)
		{
			*bytesConsumed = 3;
			return ((c & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
		}
		if ((c & 0xF8) == 0xF0)
		{
			*bytesConsumed = 4;
			return ((c & 0x07) << 18) | ((text[1] & 0x3F) << 12)
				| ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
		}

		*bytesConsumed = 1;
		return '?';
	}


	// ================================================
	// コードポイントから文字データを検索
	// ================================================
	const stbtt_packedchar* FindChar(int codepoint, int* charIndex)
	{
		for (int r = 0; r < NUM_RANGES; r++)
		{
			if (codepoint >= s_Ranges[r].firstCodepoint &&
				codepoint < s_Ranges[r].firstCodepoint + s_Ranges[r].numChars &&
				s_Ranges[r].charData != nullptr)
			{
				*charIndex = codepoint - s_Ranges[r].firstCodepoint;
				return s_Ranges[r].charData;
			}
		}
		return nullptr;
	}


	// ================================================
	// フォントファイルを読み込む（プラットフォーム別）
	// ================================================
	unsigned char* LoadFontFile(const char* path, int* outSize)
	{
#if PLATFORM_NX
		nn::fs::FileHandle file;
		nn::Result result = nn::fs::OpenFile(&file, path, nn::fs::OpenMode_Read);
		if (result.IsFailure())
		{
			PAL_LOG("Text: Failed to open font: %s\n", path);
			return nullptr;
		}

		int64_t fileSize = 0;
		nn::fs::GetFileSize(&fileSize, file);

		unsigned char* data = new unsigned char[fileSize];
		size_t readSize = 0;
		nn::fs::ReadFile(&readSize, file, 0, data, fileSize);
		nn::fs::CloseFile(file);

		*outSize = (int)fileSize;
		return data;
#else
		FILE* fp = fopen(path, "rb");
		if (!fp)
		{
			PAL_LOG("Text: Failed to open font: %s\n", path);
			return nullptr;
		}

		fseek(fp, 0, SEEK_END);
		int fileSize = (int)ftell(fp);
		fseek(fp, 0, SEEK_SET);

		unsigned char* data = new unsigned char[fileSize];
		fread(data, 1, fileSize, fp);
		fclose(fp);

		*outSize = fileSize;
		return data;
#endif
	}
}


// ================================================
// 初期化
// ================================================
void Text::Initialize(const char* fontPath, float bakeSize)
{
	s_BakedSize = bakeSize;

	// フォントファイルを読み込む
	int fontDataSize = 0;
	unsigned char* fontData = LoadFontFile(fontPath, &fontDataSize);
	if (!fontData) return;

	// 各範囲の文字データ領域を確保
	for (int r = 0; r < NUM_RANGES; r++)
	{
		s_Ranges[r].charData = new stbtt_packedchar[s_Ranges[r].numChars];
	}

	// ビットマップにベイク（複数のUnicode範囲をまとめて処理）
	unsigned char* bitmap = new unsigned char[ATLAS_SIZE * ATLAS_SIZE];
	memset(bitmap, 0, ATLAS_SIZE * ATLAS_SIZE);

	stbtt_pack_context packContext;
	stbtt_PackBegin(&packContext, bitmap, ATLAS_SIZE, ATLAS_SIZE, 0, 1, nullptr);
	stbtt_PackSetOversampling(&packContext, 2, 2);  // 2xオーバーサンプリングで品質向上

	stbtt_pack_range packRanges[NUM_RANGES];
	for (int r = 0; r < NUM_RANGES; r++)
	{
		packRanges[r].font_size = s_BakedSize;
		packRanges[r].first_unicode_codepoint_in_range = s_Ranges[r].firstCodepoint;
		packRanges[r].array_of_unicode_codepoints = nullptr;
		packRanges[r].num_chars = s_Ranges[r].numChars;
		packRanges[r].chardata_for_range = s_Ranges[r].charData;
		packRanges[r].h_oversample = 0;
		packRanges[r].v_oversample = 0;
	}

	stbtt_PackFontRanges(&packContext, fontData, 0, packRanges, NUM_RANGES);
	stbtt_PackEnd(&packContext);

	// 1チャンネル → RGBA に変換
	unsigned char* rgba = new unsigned char[ATLAS_SIZE * ATLAS_SIZE * 4];
	for (int i = 0; i < ATLAS_SIZE * ATLAS_SIZE; i++)
	{
		rgba[i * 4 + 0] = 255;
		rgba[i * 4 + 1] = 255;
		rgba[i * 4 + 2] = 255;
		rgba[i * 4 + 3] = bitmap[i];
	}

	// OpenGLテクスチャを作成
#ifdef USE_DIRECTX11
	printf("DX11 font atlas: %dx%d\n", ATLAS_SIZE, ATLAS_SIZE); fflush(stdout);
	// DX11: create texture for font atlas
	{
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = ATLAS_SIZE; td.Height = ATLAS_SIZE;
	td.MipLevels = 1; td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = rgba;
	initData.SysMemPitch = ATLAS_SIZE * 4;
	ID3D11Texture2D* fontTex = nullptr;
	HRESULT hr = g_Device->CreateTexture2D(&td, &initData, &fontTex);
	printf("CreateTexture2D: 0x%lx fontTex=%p\n", hr, fontTex); fflush(stdout);
	ID3D11ShaderResourceView* fontSRV = nullptr;
	if (fontTex) { g_Device->CreateShaderResourceView(fontTex, nullptr, &fontSRV); fontTex->Release(); }
	s_AtlasTextureId = RegisterSRV(fontSRV);
	printf("Atlas registered: %u\n", s_AtlasTextureId); fflush(stdout);
	}
#else
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE, ATLAS_SIZE,
		0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	glBindTexture(GL_TEXTURE_2D, 0);

	s_AtlasTextureId = texture;
#endif

	delete[] rgba;
	delete[] bitmap;
	delete[] fontData;
}


// ================================================
// 終了処理
// ================================================
void Text::Finalize()
{
	if (s_AtlasTextureId != 0)
	{
#ifdef USE_DIRECTX11
			ID3D11ShaderResourceView* srv = GetSRV(s_AtlasTextureId);
		if (srv) srv->Release();
#else
		glDeleteTextures(1, &s_AtlasTextureId);
#endif
		s_AtlasTextureId = 0;
	}

	for (int r = 0; r < NUM_RANGES; r++)
	{
		delete[] s_Ranges[r].charData;
		s_Ranges[r].charData = nullptr;
	}
}


// ================================================
// テキスト描画
// ================================================
void Text::Draw(const char* text, float x, float y, float size)
{
	if (s_AtlasTextureId == 0 || text == nullptr) return;

	// 呼び出し元のテクスチャ状態を退避（描画後に戻すため）
	unsigned int previousTextureId = Renderer::GetCurrentTexture();
	Renderer::SetTexture(s_AtlasTextureId);

	float scale = size / s_BakedSize;
	float cursorX = x;
	float cursorY = y;
	float baseline = cursorY + size;  // ベースライン（文字の下端基準）

	int i = 0;
	while (text[i] != '\0')
	{
		// 改行処理
		if (text[i] == '\n')
		{
			cursorX = x;
			cursorY += size;
			baseline = cursorY + size;
			i++;
			continue;
		}

		// UTF-8 デコード
		int bytesConsumed = 0;
		int codepoint = DecodeUTF8(&text[i], &bytesConsumed);
		i += bytesConsumed;

		// 文字データを検索
		int charIndex = 0;
		const stbtt_packedchar* charData = FindChar(codepoint, &charIndex);
		if (charData == nullptr) continue;

		const stbtt_packedchar& ch = charData[charIndex];

		// 文字の表示位置（ベースラインからのオフセット）
		float x0 = cursorX + ch.xoff * scale;
		float y0 = baseline + ch.yoff * scale;
		float x1 = cursorX + ch.xoff2 * scale;
		float y1 = baseline + ch.yoff2 * scale;

		// UV座標（アトラス内の文字位置）
		float u0 = (float)ch.x0 / ATLAS_SIZE;
		float v0 = (float)ch.y0 / ATLAS_SIZE;
		float u1 = (float)ch.x1 / ATLAS_SIZE;
		float v1 = (float)ch.y1 / ATLAS_SIZE;

		// 頂点データ
		VERTEX_3D vertex[4];

		vertex[0].Position = MakeFloat3(x0, y0, 0.0f);   // 左上
		vertex[1].Position = MakeFloat3(x1, y0, 0.0f);   // 右上
		vertex[2].Position = MakeFloat3(x0, y1, 0.0f);   // 左下
		vertex[3].Position = MakeFloat3(x1, y1, 0.0f);   // 右下

		for (int j = 0; j < 4; j++)
			vertex[j].Color = s_Color;

		vertex[0].TexCoord = MakeFloat2(u0, v0);
		vertex[1].TexCoord = MakeFloat2(u1, v0);
		vertex[2].TexCoord = MakeFloat2(u0, v1);
		vertex[3].TexCoord = MakeFloat2(u1, v1);

		Renderer::DrawVertices(vertex, 4, DrawMode::TriangleStrip);

		// カーソルを進める
		cursorX += ch.xadvance * scale;
	}

	// テクスチャ状態を復元
	Renderer::SetTexture(previousTextureId);
}


// ================================================
// 色設定
// ================================================
void Text::SetColor(float r, float g, float b, float a)
{
	s_Color = MakeFloat4(r, g, b, a);
}
