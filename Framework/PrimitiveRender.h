#pragma once
#include "Collision.h"  // Rectangle, Circle, Line構造体の定義
#include "Platform/PlatformTypes.h"

// 矩形の描画
void RenderRectangle(const Rectangle& rect, Float4 color = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f));
void RenderRectangleWireframe(const Rectangle& rect, Float4 color = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f));

// 円の描画
void RenderCircle(const Circle& circle, Float4 color = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f), int segments = 32);
void RenderCircleWireframe(const Circle& circle, Float4 color = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f), int segments = 32);

// 線分の描画
void RenderLine(const Line& line, Float4 color = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f), float width = 1.0f);
void RenderThickLine(const Line& line, float thickness, Float4 color = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f));
