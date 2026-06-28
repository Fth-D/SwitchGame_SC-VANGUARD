#pragma once
#include "Platform/PlatformTypes.h"

/**
 * 概要：アニメーションの1コマ（1フレーム）の情報を保持する構造体
 *
 * スプライトシート内の1つのセルに対応し、
 * そのセルのUV座標、サイズ、表示時間を管理する
 */
struct AnimationCell
{
	Float2 uvCoord;        // UV座標（左上基準）0.0〜1.0の範囲
	Float2 uvSize;         // UVサイズ（幅と高さ）0.0〜1.0の範囲
	int continueFrame;     // このコマを表示し続けるフレーム数

	// デフォルトコンストラクタ
	AnimationCell();

	// 値を指定するコンストラクタ
	AnimationCell(Float2 coord, Float2 size, int frame);
};