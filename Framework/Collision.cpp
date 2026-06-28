#include "Collision.h"
#include <cmath>
using namespace std;

// 矩形同士の当たり判定
bool CheckRectangleCollision(const Rectangle& rect1, const Rectangle& rect2)
{
    // AABB (Axis-Aligned Bounding Box) 判定
    // AABBとは、軸回転のない矩形同士の衝突判定方法です。
    // Rectangle.x, y は中心座標なので、左右上下の辺を計算してから比較する。

    float rect1Left   = rect1.x - rect1.width  / 2.0f;
    float rect1Right  = rect1.x + rect1.width  / 2.0f;
    float rect1Top    = rect1.y - rect1.height / 2.0f;
    float rect1Bottom = rect1.y + rect1.height / 2.0f;

    float rect2Left   = rect2.x - rect2.width  / 2.0f;
    float rect2Right  = rect2.x + rect2.width  / 2.0f;
    float rect2Top    = rect2.y - rect2.height / 2.0f;
    float rect2Bottom = rect2.y + rect2.height / 2.0f;

    if (rect1Left   >= rect2Right  ||    // rect1が完全にrect2の右側
        rect1Right  <= rect2Left   ||    // rect1が完全にrect2の左側
        rect1Top    >= rect2Bottom ||    // rect1が完全にrect2の下側
        rect1Bottom <= rect2Top)         // rect1が完全にrect2の上側
    {
        return false;  // 当たっていない
    }
    return true;  // 当たっている
}

// 円同士の当たり判定
bool CheckCircleCollision(const Circle& circle1, const Circle& circle2)
{
    // 円の中心間の距離を計算
    float dx = circle1.x - circle2.x;   // X（横）方向の距離
    float dy = circle1.y - circle2.y;   // Y（縦）方向の距離
    float distanceSquared = dx * dx + dy * dy;  // 距離の二乗を計算（平方根を取らないための最適化）

    // 半径の合計の二乗を計算
    float radiusSumSquared = (circle1.radius + circle2.radius) * (circle1.radius + circle2.radius); // 半径の合計を二乗

    // 距離の二乗が半径の合計の二乗より小さい場合は衝突
    return distanceSquared < radiusSumSquared;
}

/**
 * 概要：線分と線分の当たり判定
 *
 * 考え方：
 * 1. 線分ABと線分CDが交差するには、
 *    - 点Aと点Bが線分CDの反対側にある
 *    - 点Cと点Dが線分ABの反対側にある
 * 2. 外積を使って点が線分のどちら側にあるか判定
 * 3. 細かいところはGPTにシュゥゥゥーッ！
 *
 * 引数：line1, line2 判定する2つの線分
 * 返り値：交差している場合true
 */
bool CheckLineCollision(const Line& line1, const Line& line2)
{
    // ベクトルの外積を計算する補助関数
    auto cross = [](float ax, float ay, float bx, float by) {
        return ax * by - ay * bx;
        };

    // line1の方向ベクトル
    float d1x = line1.x2 - line1.x1;
    float d1y = line1.y2 - line1.y1;

    // line2の各点からline1の始点へのベクトル
    float v1x = line2.x1 - line1.x1;
    float v1y = line2.y1 - line1.y1;
    float v2x = line2.x2 - line1.x1;
    float v2y = line2.y2 - line1.y1;

    // 外積を計算（line2の両端点がline1のどちら側にあるか）
    float cross1 = cross(d1x, d1y, v1x, v1y);
    float cross2 = cross(d1x, d1y, v2x, v2y);

    // 同じ側にある場合は交差しない
    if (cross1 * cross2 > 0) return false;

    // line2の方向ベクトル
    float d2x = line2.x2 - line2.x1;
    float d2y = line2.y2 - line2.y1;

    // line1の各点からline2の始点へのベクトル
    float v3x = line1.x1 - line2.x1;
    float v3y = line1.y1 - line2.y1;
    float v4x = line1.x2 - line2.x1;
    float v4y = line1.y2 - line2.y1;

    // 外積を計算（line1の両端点がline2のどちら側にあるか）
    float cross3 = cross(d2x, d2y, v3x, v3y);
    float cross4 = cross(d2x, d2y, v4x, v4y);

    // 同じ側にある場合は交差しない
    if (cross3 * cross4 > 0) return false;

    return true;
}

/**
 * 概要：矩形と円の当たり判定
 * 引数：rect 矩形（中心座標基準）
 * 引数：circle 円（中心座標基準）
 * 返り値：衝突している場合true
 */
bool CheckRectangleCircleCollision(const Rectangle& rect, const Circle& circle)
{
    // 矩形の各辺の座標を計算（中心座標から）
    float rectLeft = rect.x - rect.width / 2.0f;
    float rectRight = rect.x + rect.width / 2.0f;
    float rectTop = rect.y - rect.height / 2.0f;
    float rectBottom = rect.y + rect.height / 2.0f;

    // 円の中心から矩形に最も近い点を見つける
    float closestX = max(rectLeft, min(circle.x, rectRight));
    float closestY = max(rectTop, min(circle.y, rectBottom));

    // 円の中心からその最近点までの距離を計算
    float dx = circle.x - closestX;
    float dy = circle.y - closestY;
    float distanceSquared = dx * dx + dy * dy;

    // 距離が半径より小さい場合は衝突
    return distanceSquared < (circle.radius * circle.radius);
}

/**
 * 概要：線分と矩形の当たり判定
 *
 * 考え方：
 * 1. 線分の端点が矩形内にあるかチェック
 * 2. 矩形の4つの辺を線分として、元の線分と交差するかチェック
 * 3. 細かいところはGPTにシュゥゥゥーッ！（プログラマは原理を知る必要はないです。使えてれば良いのです）
 *
 * 引数：line 線分, rect 矩形（中心座標基準）
 * 返り値：衝突している場合true
 */
bool CheckLineRectangleCollision(const Line& line, const Rectangle& rect)
{
    // 矩形の境界を計算
    float left = rect.x - rect.width / 2.0f;
    float right = rect.x + rect.width / 2.0f;
    float top = rect.y - rect.height / 2.0f;
    float bottom = rect.y + rect.height / 2.0f;

    // 1. 線分の端点が矩形内にあるかチェック
    auto isPointInRect = [](float px, float py, float l, float r, float t, float b) {
        return px >= l && px <= r && py >= t && py <= b;
        };

    if (isPointInRect(line.x1, line.y1, left, right, top, bottom) ||
        isPointInRect(line.x2, line.y2, left, right, top, bottom))
    {
        return true;
    }

    // 2. 矩形の4辺との交差判定
    // 上辺
    Line topEdge = { left, top, right, top };
    if (CheckLineCollision(line, topEdge)) return true;

    // 右辺
    Line rightEdge = { right, top, right, bottom };
    if (CheckLineCollision(line, rightEdge)) return true;

    // 下辺
    Line bottomEdge = { right, bottom, left, bottom };
    if (CheckLineCollision(line, bottomEdge)) return true;

    // 左辺
    Line leftEdge = { left, bottom, left, top };
    if (CheckLineCollision(line, leftEdge)) return true;

    return false;
}

/**
 * 概要：線分と円の当たり判定
 *
 * 考え方：
 * 1. 線分を方向ベクトルとして表現
 * 2. 円の中心から線分への垂線の足を求める
 * 3. 垂線の足が線分上にあるかチェック
 * 4. 最短距離を計算して半径と比較
 * 5. 細かいところはGPTにシュゥゥゥーッ！（プログラマは原理を知る必要はないです。使えてれば良いのです）
 *
 * 引数：line 線分, circle 円
 * 返り値：衝突している場合true
 */
bool CheckLineCircleCollision(const Line& line, const Circle& circle)
{
    // 線分の方向ベクトル
    float dx = line.x2 - line.x1;
    float dy = line.y2 - line.y1;

    // 線分の長さの2乗
    float lineLength2 = dx * dx + dy * dy;

    // 線分が点の場合
    if (lineLength2 == 0)
    {
        float dist2 = (circle.x - line.x1) * (circle.x - line.x1) +
            (circle.y - line.y1) * (circle.y - line.y1);
        return dist2 <= circle.radius * circle.radius;
    }

    // 始点から円の中心へのベクトル
    float fx = circle.x - line.x1;
    float fy = circle.y - line.y1;

    // 内積を計算（射影）
    float dot = fx * dx + fy * dy;

    // tは線分上の位置を表すパラメータ（0～1）
    float t = dot / lineLength2;

    // 最近点の座標
    float nearestX, nearestY;

    if (t < 0)
    {
        // 始点が最近点
        nearestX = line.x1;
        nearestY = line.y1;
    }
    else if (t > 1)
    {
        // 終点が最近点
        nearestX = line.x2;
        nearestY = line.y2;
    }
    else
    {
        // 線分上の点が最近点
        nearestX = line.x1 + t * dx;
        nearestY = line.y1 + t * dy;
    }

    // 円の中心から最近点までの距離の2乗
    float distX = circle.x - nearestX;
    float distY = circle.y - nearestY;
    float distance2 = distX * distX + distY * distY;

    // 距離が半径以下なら衝突
    return distance2 <= circle.radius * circle.radius;
}

/**
 * 概要：線分との衝突時の移動ベクトル調整
 *
 * 端点を円として扱い、完全な衝突判定を行う
 *
 * 引数：
 * - movement: 元の移動ベクトル
 * - line: 衝突している線分
 * - currentPos: 現在のキャラクター位置
 * - characterRadius: キャラクターの半径（円の場合は半径、矩形の場合は対角線の半分など）
 *
 * 戻り値：調整後の移動ベクトル
 */
Float2 CalculateMovementAlongLine(const Float2& movement, const Line& line, const Float3& currentPos, float characterRadius)
{
    // 移動が極小の場合はそのまま返す
    float moveLength2 = movement.x * movement.x + movement.y * movement.y;
    if (moveLength2 < 0.0001f)
    {
        return movement;
    }

    Float2 adjustedMovement = movement;

    // 1. 端点との衝突チェック（端点を円として扱う）
    Circle startPoint = { line.x1, line.y1, 0.1f }; // 端点の半径（小さな値）
    Circle endPoint = { line.x2, line.y2, 0.1f };

    // 移動後の位置
    Circle movedCharacter = {
        currentPos.x + movement.x,
        currentPos.y + movement.y,
        characterRadius
    };

    // 始点との衝突チェック
    if (CheckCircleCollision(movedCharacter, startPoint))
    {
        // 始点から押し出す方向を計算
        float dx = currentPos.x - line.x1;
        float dy = currentPos.y - line.y1;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist > 0.0001f)
        {
            // 正規化して必要な距離だけ押し出し
            float pushDistance = characterRadius + startPoint.radius - dist;
            if (pushDistance > 0)
            {
                dx /= dist;
                dy /= dist;

                // 移動を制限
                float dotProduct = movement.x * dx + movement.y * dy;
                if (dotProduct < 0) // 端点に向かっている場合
                {
                    // 端点に向かう成分を除去
                    adjustedMovement.x = movement.x - dotProduct * dx;
                    adjustedMovement.y = movement.y - dotProduct * dy;
                }
            }
        }
    }

    // 終点との衝突チェック
    if (CheckCircleCollision(movedCharacter, endPoint))
    {
        // 終点から押し出す方向を計算
        float dx = currentPos.x - line.x2;
        float dy = currentPos.y - line.y2;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist > 0.0001f)
        {
            // 正規化して必要な距離だけ押し出し
            float pushDistance = characterRadius + endPoint.radius - dist;
            if (pushDistance > 0)
            {
                dx /= dist;
                dy /= dist;

                // 移動を制限
                float dotProduct = adjustedMovement.x * dx + adjustedMovement.y * dy;
                if (dotProduct < 0) // 端点に向かっている場合
                {
                    // 端点に向かう成分を除去
                    adjustedMovement.x = adjustedMovement.x - dotProduct * dx;
                    adjustedMovement.y = adjustedMovement.y - dotProduct * dy;
                }
            }
        }
    }

    // 2. 線分本体との衝突処理（壁ずり）
    float lineDirX = line.x2 - line.x1;
    float lineDirY = line.y2 - line.y1;
    float lineLength2 = lineDirX * lineDirX + lineDirY * lineDirY;

    if (lineLength2 < 0.0001f)
    {
        return { 0.0f, 0.0f };
    }

    // 現在位置の線分上での位置
    float toStartX = currentPos.x - line.x1;
    float toStartY = currentPos.y - line.y1;
    float t = (toStartX * lineDirX + toStartY * lineDirY) / lineLength2;

    // 線分の範囲内（端点の間）にいる場合のみ壁ずり処理
    if (t >= 0.0f && t <= 1.0f)
    {
        // 法線ベクトル
        float lineLength = sqrt(lineLength2);
        float normalX = -lineDirY / lineLength;
        float normalY = lineDirX / lineLength;

        // 法線方向の距離
        float normalDist = toStartX * normalX + toStartY * normalY;

        // キャラクターが線分に十分近い場合
        if (abs(normalDist) < characterRadius + 1.0f)
        {
            // 移動の法線成分
            float normalComponent = adjustedMovement.x * normalX + adjustedMovement.y * normalY;

            // 線分に向かう移動の場合
            if (normalDist * normalComponent < 0)
            {
                // 法線成分を除去（壁ずり）
                adjustedMovement.x -= normalComponent * normalX;
                adjustedMovement.y -= normalComponent * normalY;
            }
        }
    }

    return adjustedMovement;
}
/**
 * 概要：円形キャラクター用の移動ベクトル調整
 *
 * 引数：
 * - movement: 元の移動ベクトル
 * - line: 衝突している線分
 * - character: 円形キャラクター
 *
 * 戻り値：調整後の移動ベクトル
 */
Float2 CalculateMovementAlongLine(const Float2& movement, const Line& line, const Circle& character)
{
    Float3 currentPos = { character.x, character.y,0.0f };
    return CalculateMovementAlongLine(movement, line, currentPos, character.radius);
}

/**
 * 概要：矩形キャラクター用の移動ベクトル調整
 *
 * 引数：
 * - movement: 元の移動ベクトル
 * - line: 衝突している線分
 * - character: 矩形キャラクター（中心座標基準）
 *
 * 戻り値：調整後の移動ベクトル
 */
Float2 CalculateMovementAlongLine(const Float2& movement, const Line& line, const Rectangle& character)
{
    Float3 currentPos = { character.x, character.y, 0.0f};

    // 矩形の場合、より精密な処理を行う
    // 移動が極小の場合はそのまま返す
    float moveLength2 = movement.x * movement.x + movement.y * movement.y;
    if (moveLength2 < 0.0001f)
    {
        return movement;
    }

    Float2 adjustedMovement = movement;

    // 矩形の四隅を計算
    float halfWidth = character.width * 0.5f;
    float halfHeight = character.height * 0.5f;

    Float2 corners[4] = {
        {character.x - halfWidth, character.y - halfHeight}, // 左上
        {character.x + halfWidth, character.y - halfHeight}, // 右上
        {character.x + halfWidth, character.y + halfHeight}, // 右下
        {character.x - halfWidth, character.y + halfHeight}  // 左下
    };

    // 各角が線分と衝突するかチェック
    bool cornerCollision = false;
    [[maybe_unused]] Float2 collisionNormal = { 0, 0 };

    for (int i = 0; i < 4; i++)
    {
        // 角の移動線分
        Line cornerMovement = {
            corners[i].x,
            corners[i].y,
            corners[i].x + movement.x,
            corners[i].y + movement.y
        };

        // 線分との交差判定
        if (CheckLineCollision(cornerMovement, line))
        {
            cornerCollision = true;

            // 交点を計算して衝突法線を求める
            float x1 = corners[i].x;
            float y1 = corners[i].y;
            float x2 = corners[i].x + movement.x;
            float y2 = corners[i].y + movement.y;
            float x3 = line.x1;
            float y3 = line.y1;
            float x4 = line.x2;
            float y4 = line.y2;

            float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

            if (abs(denom) > 0.0001f)
            {
                float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;

                if (t > 0.0f && t <= 1.0f)
                {
                    // 移動を制限
                    adjustedMovement.x *= t * 0.95f; // 少し手前で止める
                    adjustedMovement.y *= t * 0.95f;
                    return adjustedMovement;
                }
            }
        }
    }

    // 角の衝突がない場合は通常の処理（中心点での判定）
    if (!cornerCollision)
    {
        // 矩形の実効半径（対角線の半分）
        float effectiveRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
        return CalculateMovementAlongLine(movement, line, currentPos, effectiveRadius);
    }

    return adjustedMovement;
}