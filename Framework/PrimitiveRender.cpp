#include "PrimitiveRender.h"
#include "Renderer.h"
#include <cmath>

/**
 * 矩形を塗りつぶして描画
 */
void RenderRectangle(const Rectangle& rect, Float4 color)
{
    Renderer::SetTexture(0);

    VERTEX_3D vertex[4];

    float left = rect.x - rect.width / 2.0f;
    float right = rect.x + rect.width / 2.0f;
    float top = rect.y - rect.height / 2.0f;
    float bottom = rect.y + rect.height / 2.0f;

    vertex[0].Position = MakeFloat3(left, top, 0.0f);
    vertex[1].Position = MakeFloat3(right, top, 0.0f);
    vertex[2].Position = MakeFloat3(left, bottom, 0.0f);
    vertex[3].Position = MakeFloat3(right, bottom, 0.0f);

    for (int i = 0; i < 4; i++)
    {
        vertex[i].Color = color;
        vertex[i].TexCoord = MakeFloat2(0.0f, 0.0f);
    }

    Renderer::DrawVertices(vertex, 4, DrawMode::TriangleStrip);
}

/**
 * 矩形の枠線を描画
 */
void RenderRectangleWireframe(const Rectangle& rect, Float4 color)
{
    Renderer::SetTexture(0);

    VERTEX_3D vertex[4];

    float left = rect.x - rect.width / 2.0f;
    float right = rect.x + rect.width / 2.0f;
    float top = rect.y - rect.height / 2.0f;
    float bottom = rect.y + rect.height / 2.0f;

    vertex[0].Position = MakeFloat3(left, top, 0.0f);
    vertex[1].Position = MakeFloat3(right, top, 0.0f);
    vertex[2].Position = MakeFloat3(right, bottom, 0.0f);
    vertex[3].Position = MakeFloat3(left, bottom, 0.0f);

    for (int i = 0; i < 4; i++)
    {
        vertex[i].Color = color;
        vertex[i].TexCoord = MakeFloat2(0.0f, 0.0f);
    }

    Renderer::DrawVertices(vertex, 4, DrawMode::LineLoop);
}

/**
 * 円を塗りつぶして描画
 */
void RenderCircle(const Circle& circle, Float4 color, int segments)
{
    Renderer::SetTexture(0);

    if (segments < 3) segments = 3;

    float angleStep = 2.0f * 3.14159265f / segments;

    for (int i = 0; i < segments; i++)
    {
        VERTEX_3D vertex[3];

        // 中心点
        vertex[0].Position = MakeFloat3(circle.x, circle.y, 0.0f);
        vertex[0].Color = color;
        vertex[0].TexCoord = MakeFloat2(0.0f, 0.0f);

        // 現在の角度の点
        float angle1 = i * angleStep;
        vertex[1].Position = MakeFloat3(
            circle.x + circle.radius * cos(angle1),
            circle.y + circle.radius * sin(angle1),
            0.0f
        );
        vertex[1].Color = color;
        vertex[1].TexCoord = MakeFloat2(0.0f, 0.0f);

        // 次の角度の点
        float angle2 = (i + 1) * angleStep;
        vertex[2].Position = MakeFloat3(
            circle.x + circle.radius * cos(angle2),
            circle.y + circle.radius * sin(angle2),
            0.0f
        );
        vertex[2].Color = color;
        vertex[2].TexCoord = MakeFloat2(0.0f, 0.0f);

        Renderer::DrawVertices(vertex, 3);
    }
}

/**
 * 円の枠線を描画（ワイヤー）
 */
void RenderCircleWireframe(const Circle& circle, Float4 color, int segments)
{
    Renderer::SetTexture(0);

    if (segments < 3) segments = 3;

    float angleStep = 2.0f * 3.14159265f / segments;

    for (int i = 0; i < segments; i++)
    {
        VERTEX_3D vertex[2];

        // 現在の点
        float angle1 = i * angleStep;
        vertex[0].Position = MakeFloat3(
            circle.x + circle.radius * cos(angle1),
            circle.y + circle.radius * sin(angle1),
            0.0f
        );
        vertex[0].Color = color;
        vertex[0].TexCoord = MakeFloat2(0.0f, 0.0f);

        // 次の点
        float angle2 = ((i + 1) % segments) * angleStep;
        vertex[1].Position = MakeFloat3(
            circle.x + circle.radius * cos(angle2),
            circle.y + circle.radius * sin(angle2),
            0.0f
        );
        vertex[1].Color = color;
        vertex[1].TexCoord = MakeFloat2(0.0f, 0.0f);

        Renderer::DrawVertices(vertex, 2, DrawMode::Lines);
    }
}

/**
 * 線を描画
 */
void RenderLine(const Line& line, Float4 color, float width)
{
    Renderer::SetTexture(0);
    Renderer::SetLineWidth(width);

    VERTEX_3D vertex[2];

    vertex[0].Position = MakeFloat3(line.x1, line.y1, 0.0f);
    vertex[0].Color = color;
    vertex[0].TexCoord = MakeFloat2(0.0f, 0.0f);

    vertex[1].Position = MakeFloat3(line.x2, line.y2, 0.0f);
    vertex[1].Color = color;
    vertex[1].TexCoord = MakeFloat2(0.0f, 0.0f);

    Renderer::DrawVertices(vertex, 2, DrawMode::Lines);

    Renderer::SetLineWidth(1.0f);
}

/**
 * 太い線を矩形として描画
 */
void RenderThickLine(const Line& line, float thickness, Float4 color)
{
    Renderer::SetTexture(0);

    float dx = line.x2 - line.x1;
    float dy = line.y2 - line.y1;
    float length = sqrt(dx * dx + dy * dy);

    if (length == 0) return;

    // 正規化
    dx /= length;
    dy /= length;

    // 法線ベクトル
    float nx = -dy * thickness / 2.0f;
    float ny = dx * thickness / 2.0f;

    VERTEX_3D vertex[4];

    vertex[0].Position = MakeFloat3(line.x1 - nx, line.y1 - ny, 0.0f);
    vertex[1].Position = MakeFloat3(line.x1 + nx, line.y1 + ny, 0.0f);
    vertex[2].Position = MakeFloat3(line.x2 - nx, line.y2 - ny, 0.0f);
    vertex[3].Position = MakeFloat3(line.x2 + nx, line.y2 + ny, 0.0f);

    for (int i = 0; i < 4; i++)
    {
        vertex[i].Color = color;
        vertex[i].TexCoord = MakeFloat2(0.0f, 0.0f);
    }

    Renderer::DrawVertices(vertex, 4, DrawMode::TriangleStrip);
}
