#pragma once
#include "Platform/PlatformTypes.h"

// ================================================
// テキスト描画
//  TrueTypeフォント（.ttf / .ttc）を読み込み、
//  文字列を画面に描画する。
//
//  使い方:
//    Text::Initialize("C:/Windows/Fonts/msgothic.ttc");
//    Text::Draw("Hello World", 100.0f, 100.0f);
//    Text::Draw("Score: 42", 100.0f, 150.0f, 48.0f);
//    Text::SetColor(1.0f, 0.0f, 0.0f);  // 赤
// ================================================
namespace Text
{
	// --- システム ---

	// フォントを読み込む
	//  fontPath: フォントファイルのパス
	//    Win64: "C:/Windows/Fonts/msgothic.ttc" や "Assets/Font/myfont.ttf"
	//    NX64:  "rom:/font/myfont.ttf"
	//  bakeSize: フォントベイクサイズ（内部解像度。大きいほど綺麗だが重い）
	void Initialize(const char* fontPath, float bakeSize = 48.0f);
	void Finalize();

	// --- 描画 ---

	// テキストを描画する（画面座標、左上基準）
	void Draw(const char* text, float x, float y, float size = 32.0f);

	// 描画色を設定する
	void SetColor(float r, float g, float b, float a = 1.0f);
}
