#include "AnimationCell.h"

/**
 * 概要：デフォルトコンストラクタ
 * すべてのメンバを初期値で初期化
 */
AnimationCell::AnimationCell()
{
    uvCoord = MakeFloat2(0.0f, 0.0f);
    uvSize = MakeFloat2(0.0f, 0.0f);
    continueFrame = 1;
}

/**
 * 概要：値を指定するコンストラクタ
 * 引数：coord UV座標
 * 引数：size UVサイズ
 * 引数：frame 表示フレーム数
 */
AnimationCell::AnimationCell(Float2 coord, Float2 size, int frame)
{
    uvCoord = coord;
    uvSize = size;
    continueFrame = frame;
}