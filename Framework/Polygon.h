#pragma once
#include "Renderer.h"

// ================================================
// ポリゴン構造体
//  位置・サイズ・色を指定するだけで四角形を描画できる。
//  Draw()を呼ぶと、内部でRenderer::DrawVertices()が呼ばれる。
// ================================================
struct Polygon
{
	Float3 position;    // 表示位置（中心座標）
	float  width;       // 横幅
	float  height;      // 縦幅
	Float4 color;       // 色（R, G, B, A）

	// コンストラクタ（初期値を設定する）
	Polygon();

	// 描画する
	void Draw();
};
