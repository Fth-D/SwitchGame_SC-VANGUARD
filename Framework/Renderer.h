#pragma once
#include "Platform/PlatformTypes.h"

// ================================================
// 頂点情報
// ================================================
struct VERTEX_3D
{
	Float3 Position;  // 座標
	Float4 Color;     // 色
	Float2 TexCoord;  // テクスチャ座標
};

// ================================================
// 描画モード
// ================================================
enum class DrawMode
{
	Triangles,      // 三角形
	TriangleStrip,  // 三角形ストリップ
	Lines,          // 線
	LineLoop,       // 線ループ（閉じた線）
};

// ================================================
// 描画インターフェース
//  ゲームコードではOpenGLの関数を直接呼ばず、
//  このnamespace内の関数を使って描画すること。
// ================================================
namespace Renderer
{
	// --- システム（触らなくてよい） ---
	void Initialize();
	void Finalize();

	// --- フレーム制御 ---
	void BeginFrame();                               // 黒背景でクリア
	void BeginFrame(float r, float g, float b);      // 背景色を指定してクリア
	void EndFrame();                                  // 画面に反映

	// --- 描画 ---
	void DrawVertices(const VERTEX_3D* vertices,      // 頂点配列
	                  int count,                      // 頂点数
	                  DrawMode mode = DrawMode::Triangles);  // 描画モード

	// --- テクスチャ ---
	void SetTexture(unsigned int textureId);          // テクスチャを使う（0で解除）
	unsigned int GetCurrentTexture();                 // 現在のテクスチャIDを取得（保存・復元用）

	// --- その他 ---
	void SetLineWidth(float width);                   // 線の太さ
	void SetScreenOffset(float x, float y);           // 画面全体のオフセット（シェイク用）
}
