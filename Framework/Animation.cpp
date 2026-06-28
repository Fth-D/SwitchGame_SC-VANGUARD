#include "Animation.h"
#include <cstdio>
#include <algorithm>

/**
 * 概要：セル配列を安全に削除する内部関数
 *
 * nullチェックを行い、配列を削除してnullptrを設定
 * 関連する状態変数もリセットする
 */
void Animation::SafeDeleteCells()
{
    if (cells != nullptr)
    {
        delete[] cells;
        cells = nullptr;
    }

    // セル配列に関連する変数もリセット
    cellCount = 0;
    currentCellIndex = 0;
    currentFrameTime = 0;
    currentUVCoord = MakeFloat2(0.0f, 0.0f);
    currentUVSize = MakeFloat2(0.0f, 0.0f);
}

/**
 * 概要：コンストラクタ
 * 全てのメンバ変数を初期化
 */
Animation::Animation()
{
    cells = nullptr;
    cellCount = 0;
    animationName = "";
    currentCellIndex = 0;
    currentFrameTime = 0;
    isLoop = false;
    currentUVCoord = MakeFloat2(0.0f, 0.0f);
    currentUVSize = MakeFloat2(0.0f, 0.0f);
}

/**
 * 概要：デストラクタ
 * 動的確保したセル配列を解放
 */
Animation::~Animation()
{
    SafeDeleteCells();  // 安全な削除関数を使用
}

/**
 * 概要：アニメーションの初期化
 * 引数：name アニメーション名（"walk", "jump"など）
 * 引数：count セル数（コマ数）
 */
void Animation::Initialize(const std::string& name, int count)
{
    // 既存のセルがあれば解放
    SafeDeleteCells();  // 安全な削除関数を使用

    animationName = name;
    cellCount = count;

    // セル配列を動的確保
    if (cellCount > 0)
    {
        cells = new AnimationCell[cellCount];

        // 最初のセルが作成されたので、現在の状態を初期化
        currentCellIndex = 0;
        currentFrameTime = 0;
    }
    else
    {
        cells = nullptr;
    }
}

/**
 * 概要：指定インデックスのセル情報を設定
 * 引数：index セルのインデックス（0〜cellCount-1）
 * 引数：coord UV座標
 * 引数：size UVサイズ
 * 引数：frame 表示フレーム数
 */
void Animation::SetCell(int index, Float2 coord, Float2 size, int frame)
{
    if (index < 0 || index >= cellCount || cells == nullptr)
    {
        return;
    }

    cells[index].uvCoord = coord;
    cells[index].uvSize = size;
    cells[index].continueFrame = frame;

    // 最初のセルの場合は現在の状態も更新
    if (index == 0)
    {
        currentUVCoord = coord;
        currentUVSize = size;
    }
}

/**
 * 概要：指定インデックスのセルの表示時間を設定
 * 引数：index セルのインデックス
 * 引数：frame 表示フレーム数
 */
void Animation::SetCellFrameTime(int index, int frame)
{
    if (index < 0 || index >= cellCount || cells == nullptr)
    {
        return;
    }

    cells[index].continueFrame = frame;
}

/**
 * 概要：ループ設定を変更
 * 引数：loop ループするかどうか
 */
void Animation::SetLoop(bool loop)
{
    isLoop = loop;
}

/**
 * 概要：アニメーションを更新（1フレーム進める）
 *
 * 現在のセルの表示時間が経過したら次のセルへ移行
 * ループ設定に応じて最初に戻るか最後で停止する
 */
void Animation::Update()
{
    if (cellCount <= 0 || cells == nullptr)
    {
        return;
    }

    // 現在のフレームカウントを増加
    currentFrameTime++;

    // 表示時間が経過したら次のセルへ
    if (currentFrameTime >= cells[currentCellIndex].continueFrame)
    {
        currentFrameTime = 0;
        currentCellIndex++;

        // 最後のセルを超えた場合の処理
        if (currentCellIndex >= cellCount)
        {
            if (isLoop)
            {
                // ループする場合は最初に戻る
                currentCellIndex = 0;
            }
            else
            {
                // ループしない場合は最後のセルで停止
                currentCellIndex = cellCount - 1;
                currentFrameTime = cells[currentCellIndex].continueFrame;
            }
        }
    }

    // 現在のUV情報を更新（デバッグ・管理用）
    currentUVCoord = cells[currentCellIndex].uvCoord;
    currentUVSize = cells[currentCellIndex].uvSize;
}

/**
 * 概要：アニメーションをリセット（最初から再生）
 */
void Animation::Reset()
{
    currentCellIndex = 0;
    currentFrameTime = 0;

    if (cells != nullptr && cellCount > 0)
    {
        currentUVCoord = cells[0].uvCoord;
        currentUVSize = cells[0].uvSize;
    }
}

/**
 * 概要：指定したセルに直接ジャンプ
 * 引数：index ジャンプ先のセルインデックス
 */
void Animation::SetCurrentCell(int index)
{
    if (index < 0 || index >= cellCount || cells == nullptr)
    {
        return;
    }

    currentCellIndex = index;
    currentFrameTime = 0;
    currentUVCoord = cells[currentCellIndex].uvCoord;
    currentUVSize = cells[currentCellIndex].uvSize;
}

/**
 * 概要：現在のUV座標を取得
 * 返り値：現在のUV座標
 */
Float2 Animation::GetCurrentUVCoord()
{
    return currentUVCoord;
}

/**
 * 概要：現在のUVサイズを取得
 * 返り値：現在のUVサイズ
 */
Float2 Animation::GetCurrentUVSize()
{
    return currentUVSize;
}

/**
 * 概要：現在のセルインデックスを取得
 * 返り値：現在のセルインデックス
 */
int Animation::GetCurrentCellIndex()
{
    return currentCellIndex;
}

/**
 * 概要：総セル数を取得
 * 返り値：総セル数
 */
int Animation::GetCellCount()
{
    return cellCount;
}

/**
 * 概要：ループ設定を取得
 * 返り値：ループするかどうか
 */
bool Animation::IsLooping()
{
    return isLoop;
}

/**
 * 概要：アニメーションが終了したかチェック
 * 返り値：ループしない設定で最後のセルの表示が終了したらtrue
 */
bool Animation::IsAnimationEnd()
{
    if (isLoop)
    {
        return false; // ループする場合は終了しない
    }

    // ループしない　かつ　今のアニメーションが終了していればtrueを返す
    return (currentCellIndex == cellCount - 1) && (currentFrameTime >= cells[currentCellIndex].continueFrame);
}

/**
 * 概要：指定インデックスのセルへのポインタを取得
 * 引数：index セルのインデックス
 * 返り値：セルへのポインタ（無効な場合はnullptr）
 */
AnimationCell* Animation::GetCell(int index)
{
    if (index < 0 || index >= cellCount || cells == nullptr)
    {
        return nullptr;
    }
    return &cells[index];
}

/**
 * 概要：指定インデックスのセルのUV座標を取得
 * 引数：index セルのインデックス
 * 返り値：UV座標（無効な場合は(0,0)）
 */
Float2 Animation::GetCellUVCoord(int index)
{
    if (index < 0 || index >= cellCount || cells == nullptr)
    {
        return MakeFloat2(0.0f, 0.0f);
    }
    return cells[index].uvCoord;
}

/**
 * 概要：指定インデックスのセルのUVサイズを取得
 * 引数：index セルのインデックス
 * 返り値：UVサイズ（無効な場合は(0,0)）
 */
Float2 Animation::GetCellUVSize(int index)
{
    if (index < 0 || index >= cellCount || cells == nullptr)
    {
        return MakeFloat2(0.0f, 0.0f);
    }
    return cells[index].uvSize;
}

/**
 * 概要：指定インデックスのセルのフレーム時間を取得
 * 引数：index セルのインデックス
 * 返り値：フレーム時間（無効な場合は0）
 */
int Animation::GetCellFrameTime(int index)
{
    if (index < 0 || index >= cellCount || cells == nullptr)
    {
        return 0;
    }
    return cells[index].continueFrame;
}

/**
 * 概要：デバッグ情報を文字列で取得
 * 返り値：現在の状態を表す文字列
 */
std::string Animation::GetDebugInfo()
{
    char buffer[512];
    int continueFrame = 0;

    if (cells && currentCellIndex < cellCount)
    {
        continueFrame = cells[currentCellIndex].continueFrame;
    }

    std::snprintf(buffer, sizeof(buffer),
        "Animation: %s\n"
        "  Cell: %d/%d (Frame: %d/%d)\n"
        "  UV Coord: (%.3f, %.3f)\n"
        "  UV Size: (%.3f, %.3f)\n"
        "  Loop: %s\n"
        "  Status: %s",
        animationName.c_str(),
        currentCellIndex + 1, cellCount,
        currentFrameTime, continueFrame,
        currentUVCoord.x, currentUVCoord.y,
        currentUVSize.x, currentUVSize.y,
        isLoop ? "Yes" : "No",
        IsAnimationEnd() ? "Finished" : "Playing"
    );

    return std::string(buffer);
}