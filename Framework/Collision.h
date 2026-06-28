#pragma once
#include <iostream>
#include <cmath>
#include "Platform/PlatformTypes.h"

// 矩形を表す構造体
struct Rectangle 
{
    float x, y;             // 中心の座標
	float width, height;    // 幅と高さ
};

// 円を表す構造体
struct Circle 
{
    float x, y;      // 中心座標
	float radius;    // 半径
};

// 線分（始点と終点のある線のこと）を表す構造体
// (どこまでも伸びる一本の線のことは「直線」または「無限直線」といいます。基本は直線って言います。）
// (始点はあるが終点がない1本の線のことは「光線（レイ）」または「半直線」と言います。基本はレイっていいます)
struct Line
{
    float x1, y1;    // 始点の座標
    float x2, y2;    // 終点の座標
};

// 矩形同士の当たり判定
// 衝突していたら true を返す
bool CheckRectangleCollision(const Rectangle& rect1, const Rectangle& rect2);

// 円同士の当たり判定
// 衝突していたら true を返す
bool CheckCircleCollision(const Circle& circle1, const Circle& circle2);

// 矩形と円の当たり判定
// 衝突していたら true を返す
bool CheckRectangleCircleCollision(const Rectangle& rect, const Circle& circle);

// 線分同士の当たり判定
bool CheckLineCollision(const Line& line1, const Line& line2);

// 線分と矩形の当たり判定
bool CheckLineRectangleCollision(const Line& line, const Rectangle& rect);

// 線分と円の当たり判定
bool CheckLineCircleCollision(const Line& line, const Circle& circle);

// 線分に沿った移動ベクトルを計算（円形用）
Float2 CalculateMovementAlongLine(const Float2& movement, const Line& line, const Circle& character);

// 矩形用
Float2 CalculateMovementAlongLine(const Float2& movement, const Line& line, const Rectangle& character);

// 線分に沿った移動ベクトルを実際に計算している関数
Float2 CalculateMovementAlongLine(const Float2& movement, const Line& line, const Float3& currentPos, float characterRadius);

// 移動が線分に向かっているかを判定
bool IsMovingTowardLine(const Float3& position, const Float2& movement, const Line& line);