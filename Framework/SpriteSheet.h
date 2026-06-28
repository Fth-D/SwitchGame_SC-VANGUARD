#pragma once
#include <string>
#include <map>
#include "Animation.h"
#include "main.h"

/**
 * 概要：スプライトシートを管理し、複数のアニメーションを制御するクラス
 * アニメーションを名前で管理できるようにしています。
 * 詳細な使い方はmain.cppを参照してね
 * ・以下の機能を持つ
 * - テクスチャの読み込みと管理
 * - スプライトシートの分割
 * - 複数のアニメーションの管理
 * - 描画処理
 */
class SpriteSheet
{
private:
    // アニメーション管理
    std::map<std::string, Animation*> animations;   // 名前でアニメーションを管理
                                                    // std::map ⇒ 2つの識別子を一組の要素として扱える配列…のようなもの。
                                                    // 連想配列、テーブルなんて呼ばれ方をしている。
                                                    // 今回の場合はstring(文字列)とアニメーション1種類あたりの情報の2つを一つの要素として扱っている。

    Animation* currentAnimation;                    // 現在再生中のアニメーションの情報
    std::string currentAnimationName = "NotPlaying";// 現在再生中のアニメーションの名前

    // テクスチャ情報
    unsigned int textureId;                         // OpenGL用のテクスチャID（DirectXでも使えるように改造したい場合はどうすれば良いか考えると楽しいかもね）
    std::string filePath;                           // ファイルパス
    int divideX, divideY;                           // 分割数（横、縦）

    // 描画情報
    Float3 position;                                // ポリゴンの表示位置
    Float2 size;                                    // ポリゴンの表示サイズ

    float angle;                                   // 回転角度（時計回り）
    
    bool isScreenPosition = false;                 // trueの場合、スクリーン座標に固定される。

private:
    // 内部ヘルパー関数
    Float2 CalculateUVCoord(int index);
    void SafeDeleteAllAnimations();  // 全アニメーションを安全に削除
    void RotatePolygon(Float2 pos,VERTEX_3D* vertex);  // 実際にポリゴンを回転させている関数（意図しない呼び出しを避けるためにprivate実装）

public:
    // コンストラクタ・デストラクタ
    SpriteSheet();
    ~SpriteSheet();

    // 初期化
    void Initialize();

    // テクスチャ管理
    void LoadTexture(const std::string& path);
    void UnloadTexture();

    // スプライトシート分割
    void DivideAnimationCells(int divX, int divY);

    // アニメーション管理
    void CreateAnimation(const std::string& name, int startIndex, int endIndex);
    void SetAnimation(const std::string& name);
    void SetAnimationLoop(const std::string& name, bool loop);
    void SetAnimationFrameTime(const std::string& name, int frameTime);
    void DeleteAnimation(const std::string& name);
    void ClearAnimations();

    // アニメーション管理（既存の部分の後に追加）
    // セル単位の操作
    void SetAnimationCell(const std::string& name, int cellIndex, Float2 coord, Float2 size, int frame);
    void SetAnimationCellFrameTime(const std::string& name, int cellIndex, int frame);
    void SetAnimationCellUV(const std::string& name, int cellIndex, Float2 coord, Float2 size);

    // ポリゴンを回転させるための関数
    void Rotate(float rotateAngle);         // ポリゴンを回転させる
    void ReverseRotate(float rotateAngle);  // ポリゴンを逆回転させる
    void SetRotate(float newRotate);        // ポリゴンの角度を設定する

    // 更新・描画
    void Update();
    void Render();

    // 描画設定
    void SetPolygonPosition(Float3 pos);
    void SetPolygonSize(Float2 size);
    Float3 GetPolygonPosition();
    Float2 GetPolygonSize();
    void SetFixedScreenPosition(bool fix) { isScreenPosition = fix; };    //　スクリーン座標に固定するかどうかを設定
    bool GetFixedScreenPosition() { return isScreenPosition; };

    // 情報取得
    std::string GetCurrentAnimationName();
    bool HasAnimation(const std::string& name);
    std::string GetDebugInfo();

    // テクスチャID関係
    int GetTextureID() { return textureId; }
    void SetTextureID(int id) { textureId = id; }
};