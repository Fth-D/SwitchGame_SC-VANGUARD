#include "Polygon.h"


// ================================================
// コンストラクタ
// ================================================
Polygon::Polygon()
{
	position = MakeFloat3(0.0f, 0.0f, 0.0f);
	width    = 100.0f;
	height   = 100.0f;
	color    = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f);  // 白
}


// ================================================
// 描画
//  position を中心に、width x height の四角形を描画する。
//  内部では4つの頂点データを作成し、TriangleStripで描画している。
// ================================================
void Polygon::Draw()
{
	// テクスチャを使わない
	Renderer::SetTexture(0);

	// 4つの頂点データを用意する
	VERTEX_3D vertex[4];

	// 四角形の四隅の座標を計算する
	float left   = position.x - width  / 2.0f;
	float right  = position.x + width  / 2.0f;
	float top    = position.y - height / 2.0f;
	float bottom = position.y + height / 2.0f;

	// 頂点座標（左上、右上、左下、右下）
	vertex[0].Position = MakeFloat3(left,  top,    0.0f);
	vertex[1].Position = MakeFloat3(right, top,    0.0f);
	vertex[2].Position = MakeFloat3(left,  bottom, 0.0f);
	vertex[3].Position = MakeFloat3(right, bottom, 0.0f);

	// 色とテクスチャ座標を設定
	for (int i = 0; i < 4; i++)
	{
		vertex[i].Color    = color;
		vertex[i].TexCoord = MakeFloat2(0.0f, 0.0f);
	}

	// TriangleStripで描画する（4頂点で四角形になる）
	Renderer::DrawVertices(vertex, 4, DrawMode::TriangleStrip);
}
