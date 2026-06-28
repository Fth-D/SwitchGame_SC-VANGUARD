#pragma once
#include <string>
#include "AnimationCell.h"

/**
 * 概要：1種類のアニメーション（歩く、走る、ジャンプなど）を管理するクラス
 *
 * 複数のAnimationCellで構成され、順番に再生することでアニメーションを表現する
 * ・以下の機能を持つ
 */
class Animation
{
private:
    // アニメーションデータ
    AnimationCell* cells;          // アニメーションセルの配列（動的確保）
    int cellCount;                 // 総セル数
    std::string animationName;     // アニメーション名（デバッグ用）

    // 現在の状態
    int currentCellIndex;          // 現在表示中のセル番号（0〜cellCount-1）
    int currentFrameTime;          // 現在のセルを表示している経過フレーム数
    Float2 currentUVCoord;         // 現在のUV座標（デバッグ・管理用）
    Float2 currentUVSize;          // 現在のUVサイズ（デバッグ・管理用）

    // アニメーション設定
    bool isLoop;                   // ループ再生するかどうか

private:
    // 内部ヘルパー関数
    // privateな関数は当然ながらクラス外から呼び出せない
    // (外から呼んでほしくない・呼ばれる想定ではない・呼ぶ意味がない関数はここにprivateなメンバ関数にするとよい)
    void SafeDeleteCells();  // セル配列を安全に削除する関数

public:
    // コンストラクタ・デストラクタ
    Animation();
    ~Animation();

    // 初期化・設定系
    void Initialize(const std::string& name, int count);
    void SetCell(int index, Float2 coord, Float2 size, int frame);
    void SetCellFrameTime(int index, int frame);
    void SetLoop(bool loop);

    // 更新・制御系
    void Update();                 // アニメーションを1フレーム進める
    void Reset();                  // 最初のコマから再生し直す
    void SetCurrentCell(int index); // 指定したセルに直接ジャンプ

    // 取得系
    Float2 GetCurrentUVCoord(); // 現在のセルのUVサイズを取得
    Float2 GetCurrentUVSize();  // 現在のセルのUVサイズを取得
    int GetCurrentCellIndex();  // 現在のセル番号を取得
    int GetCellCount();         // セル総数を取得
    bool IsLooping();           // ループするか
    bool IsAnimationEnd();      // アニメーションが終了したか

    // セル情報の取得
    AnimationCell* GetCell(int index);  // セルへのポインタを返す
    Float2 GetCellUVCoord(int index);   // 特定セルのUV座標を取得
    Float2 GetCellUVSize(int index);    // 特定セルのUVサイズを取得
    int GetCellFrameTime(int index);    // 特定セルのフレーム時間を取得

    // デバッグ用
    std::string GetDebugInfo(); // デバッグ情報を文字列で取得
};