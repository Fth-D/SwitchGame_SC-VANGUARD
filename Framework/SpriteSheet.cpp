#include "SpriteSheet.h"
#include "Renderer.h"
#include "texture.h"
#include "main.h"
#include <cstdio>



/**
 * 概要：全てのアニメーションを安全に削除する内部関数
 */
void SpriteSheet::SafeDeleteAllAnimations()
{
    // 現在のアニメーションポインタをクリア
    currentAnimation = nullptr;

    // マップ内の全てのアニメーションを削除
    for (auto& pair : animations)
    {
        delete pair.second;
    }

    // マップをクリア
    animations.clear();
}

/**
 * 概要：コンストラクタ
 * 全メンバを初期化
 */
SpriteSheet::SpriteSheet()
{
    currentAnimation = nullptr;
    textureId = 0;
    filePath = "";
    divideX = 0;
    divideY = 0;
    position = MakeFloat3(0.0f, 0.0f, 0.0f);
    size = MakeFloat2(64.0f, 64.0f);
    angle = 0.0f;
}

/**
 * 概要：デストラクタ
 * テクスチャとアニメーションを解放
 */
SpriteSheet::~SpriteSheet()
{
    //UnloadTexture();//いらなくなりました
    SafeDeleteAllAnimations();  // ヘルパー関数を使用
}

/**
 * 概要：初期化処理
 * 全ての状態をリセット
 */
void SpriteSheet::Initialize()
{
    UnloadTexture();
    SafeDeleteAllAnimations();  // ヘルパー関数を使用

    currentAnimation = nullptr;
    divideX = 0;
    divideY = 0;
    position = MakeFloat3(0.0f, 0.0f, 0.0f);
    size = MakeFloat2(64.0f, 64.0f);
}

/**
 * 概要：テクスチャを読み込む
 * 引数：path テクスチャファイルのパス
 */
void SpriteSheet::LoadTexture(const std::string& path)
{
    // 既存のテクスチャがあれば解放
    //UnloadTexture();//いらなくなったよ

    // グローバル関数のLoadTextureを明示的に呼び出し
    textureId = ::LoadTexture(path.c_str());

    if (textureId > 0)
    {
        filePath = path;
    }
    else
    {
        textureId = 0;
        filePath = "";
        // エラーログ出力など
    }
}

/**
 * 概要：テクスチャを解放
 */
void SpriteSheet::UnloadTexture()
{
    if (textureId != 0)
    {
        ::UnloadTexture(textureId);
        textureId = 0;
        filePath = "";
    }
}

/**
 * 概要：スプライトシートを指定の数で分割
 * 引数：divX 横方向の分割数
 * 引数：divY 縦方向の分割数
 */
void SpriteSheet::DivideAnimationCells(int divX, int divY)
{
    divideX = divX;
    divideY = divY;
}

/**
 * 概要：インデックスからUV座標を計算
 * 引数：index セルのインデックス（0から始まる）
 * 返り値：UV座標（左上）
 */
Float2 SpriteSheet::CalculateUVCoord(int index)
{
    if (divideX <= 0 || divideY <= 0)
    {
        return MakeFloat2(0.0f, 0.0f);
    }

    int col = index % divideX;  // 列（X方向）
    int row = index / divideX;  // 行（Y方向）

    float u = (float)col / (float)divideX;
    float v = (float)row / (float)divideY;

    return MakeFloat2(u, v);
}

/**
 * 概要：新しいアニメーションを作成
 * 引数：name アニメーション名
 * 引数：startIndex 開始セルインデックス
 * 引数：endIndex 終了セルインデックス
 */
void SpriteSheet::CreateAnimation(const std::string& name,
    int startIndex, int endIndex)
{
    // 既存のアニメーションがあれば削除
    DeleteAnimation(name);

    // パラメータチェック
    int totalCells = divideX * divideY;
    if (startIndex < 0 || endIndex >= totalCells || startIndex > endIndex)
    {
        return; // エラー
    }

    // Animationインスタンスを作成
    Animation* newAnim = new Animation();
    int cellCount = endIndex - startIndex + 1;
    newAnim->Initialize(name, cellCount);

    // 各セルの情報を設定
    float cellWidth = 1.0f / (float)divideX;
    float cellHeight = 1.0f / (float)divideY;

    for (int i = 0; i < cellCount; i++)
    {
        int cellIndex = startIndex + i;
        Float2 uvCoord = CalculateUVCoord(cellIndex);
        Float2 uvSize = MakeFloat2(cellWidth, cellHeight);

        // デフォルトで5フレーム表示
        newAnim->SetCell(i, uvCoord, uvSize, 5);
    }

    // デフォルトでループ設定
    newAnim->SetLoop(true);

    // マップに追加
    animations[name] = newAnim;
}

/**
 * 概要：指定した名前のアニメーションを再生
 * 引数：name アニメーション名
 */
void SpriteSheet::SetAnimation(const std::string& name)
{
    auto it = animations.find(name);
    if (it != animations.end())
    {
        // 別のアニメーションに切り替える場合のみリセット
        if (currentAnimation != it->second)
        {
            currentAnimation = it->second;
            currentAnimation->Reset();
        }
    }
}

/**
 * 概要：アニメーションのループ設定
 * 引数：name アニメーション名
 * 引数：loop ループするかどうか
 */
void SpriteSheet::SetAnimationLoop(const std::string& name, bool loop)
{
    auto it = animations.find(name);
    if (it != animations.end())
    {
        it->second->SetLoop(loop);
    }
}

/**
 * 概要：アニメーションの全セルの表示時間を設定
 * 引数：name アニメーション名
 * 引数：frameTime 表示フレーム数
 */
void SpriteSheet::SetAnimationFrameTime(const std::string& name, int frameTime)
{
    auto it = animations.find(name);
    if (it != animations.end())
    {
        Animation* anim = it->second;
        for (int i = 0; i < anim->GetCellCount(); i++)
        {
            anim->SetCellFrameTime(i, frameTime);
        }
    }
}

/**
 * 概要：指定した名前のアニメーションを削除
 * 引数：name アニメーション名
 */
void SpriteSheet::DeleteAnimation(const std::string& name)
{
    auto it = animations.find(name);
    if (it != animations.end())
    {
        // 現在のアニメーションの場合はnullptrに
        if (currentAnimation == it->second)
        {
            currentAnimation = nullptr;
        }

        delete it->second;
        animations.erase(it);
    }
}

/**
 * 概要：全てのアニメーションを削除
 */
void SpriteSheet::ClearAnimations()
{
    SafeDeleteAllAnimations();
}

/**
 * 概要：特定のアニメーションの特定のセルの情報を設定
 * 引数：name アニメーション名
 * 引数：cellIndex セルのインデックス（0から始まる）
 * 引数：coord UV座標
 * 引数：size UVサイズ
 * 引数：frame 表示フレーム数
 */
void SpriteSheet::SetAnimationCell(const std::string& name, int cellIndex, Float2 coord, Float2 size, int frame)
{
    auto it = animations.find(name);
    if (it != animations.end())
    {
        it->second->SetCell(cellIndex, coord, size, frame);
    }
}

/**
 * 概要：特定のアニメーションの特定のセルの表示時間を設定
 * 引数：name アニメーション名
 * 引数：cellIndex セルのインデックス
 * 引数：frame 表示フレーム数
 */
void SpriteSheet::SetAnimationCellFrameTime(const std::string& name, int cellIndex, int frame)
{
    auto it = animations.find(name);
    if (it != animations.end())
    {
        it->second->SetCellFrameTime(cellIndex, frame);
    }
}

/**
 * 概要：特定のアニメーションの特定のセルのUV情報のみ設定
 * 引数：name アニメーション名
 * 引数：cellIndex セルのインデックス
 * 引数：coord UV座標
 * 引数：size UVサイズ
 */
void SpriteSheet::SetAnimationCellUV(const std::string& name, int cellIndex, Float2 coord, Float2 size)
{
    auto it = animations.find(name);
    if (it != animations.end())
    {
        // 現在のフレーム時間を保持したままUV情報だけ更新
        it->second->SetCell(cellIndex, coord, size, 5); // デフォルト値を使用
    }
}

/**
 * 概要：アニメーションを更新
 */
void SpriteSheet::Update()
{
    if (currentAnimation != nullptr)
    {
        currentAnimation->Update();
    }
}

/**
 * 概要：ポリゴンを時計回りに回す
 */
void SpriteSheet::Rotate(float rotateAngle)
{
    angle += rotateAngle;
    // 360度を超えて回転している場合は角度を丸め込み
    if (angle > 360.0f)
    {
        angle = angle - 360.0f;
    }
    else if (angle < 0.0f)
    {
        angle = 360 - angle;
    }
}

/**
 * 概要：ポリゴンを反時計回りに回す
 */
void SpriteSheet::ReverseRotate(float rotateAngle)
{
    angle -= rotateAngle;

    // 0度未満になったら丸め込み
    if (angle < 0.0f)
    {
        angle = 360 - angle;
    }
}

/**
 * 概要：ポリゴンの角度を設定する
 */
void SpriteSheet::SetRotate(float newRotate)
{
    angle = newRotate;
}

/**
 * 概要：ポリゴンを回転させる
 */
void SpriteSheet::RotatePolygon(Float2 pos,VERTEX_3D* vertex)
{
    // 斜辺の長さを計算
    double oblique = sqrt(pow(size.x/2.0f,2.0f) + pow(size.y/2.0f,2.0f));

    // 回転するラジアン角度
    double rad = 0.0;

    // 左上の頂点を回転して配置
    rad = (0 + atan((size.y / 2) / (size.x / 2)) * 180.0f / PI + angle) * PI / 180.0f;
    vertex[0].Position = MakeFloat3(pos.x - (float)(oblique * cos(rad)), pos.y - (float)(oblique * sin(rad)), 0.0f);

    // 右上
    rad = (180 - atan((size.y / 2) / (size.x / 2)) * 180.0f / PI + angle) * PI / 180.0f;
    vertex[1].Position = MakeFloat3(pos.x - (float)(oblique * cos(rad)), pos.y - (float)(oblique * sin(rad)), 0.0f);

    // 左下
    rad = (360 - atan((size.y / 2) / (size.x / 2)) * 180.0f / PI + angle) * PI / 180.0f;
    vertex[2].Position = MakeFloat3(pos.x - (float)(oblique * cos(rad)), pos.y - (float)(oblique * sin(rad)), 0.0f);

    // 右下
    rad = (180 + atan((size.y / 2) / (size.x / 2)) * 180.0f / PI + angle) * PI / 180.0f;
    vertex[3].Position = MakeFloat3(pos.x - (float)(oblique * cos(rad)), pos.y - (float)(oblique * sin(rad)), 0.0f);
}

/**
 * 概要：スプライトを描画
 */
void SpriteSheet::Render()
{
    // テクスチャやアニメーションがない場合は描画しない
    if (textureId == 0 || currentAnimation == nullptr)
    {
        return;
    }

    // 現在のアニメーションからUV情報を取得
    Float2 uvCoord = currentAnimation->GetCurrentUVCoord();
    Float2 uvSize = currentAnimation->GetCurrentUVSize();

    // テクスチャを設定
    Renderer::SetTexture(textureId);

    VERTEX_3D vertex[4];
    Float2 screenPosition = MakeFloat2(position.x, position.y);

    float left = screenPosition.x - size.x / 2.0f;
    float right = screenPosition.x + size.x / 2.0f;
    float top = screenPosition.y - size.y / 2.0f;
    float bottom = screenPosition.y + size.y / 2.0f;

    vertex[0].Position = MakeFloat3(left, top, 0.0f);
    vertex[1].Position = MakeFloat3(right, top, 0.0f);
    vertex[2].Position = MakeFloat3(left, bottom, 0.0f);
    vertex[3].Position = MakeFloat3(right, bottom, 0.0f);

    RotatePolygon(screenPosition, vertex);

    for (int i = 0; i < 4; i++)
    {
        vertex[i].Color = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    float u0 = uvCoord.x;
    float u1 = uvCoord.x + uvSize.x;
    float v0 = 1.0f - uvCoord.y;
    float v1 = 1.0f - uvCoord.y - uvSize.y;

    vertex[0].TexCoord = MakeFloat2(u0, v0);
    vertex[1].TexCoord = MakeFloat2(u1, v0);
    vertex[2].TexCoord = MakeFloat2(u0, v1);
    vertex[3].TexCoord = MakeFloat2(u1, v1);

    Renderer::DrawVertices(vertex, 4, DrawMode::TriangleStrip);
}

/**
 * 概要：表示位置を設定
 * 引数：pos 新しい位置
 */
void SpriteSheet::SetPolygonPosition(Float3 pos)
{
    position = pos;
}

/**
 * 概要：表示サイズを設定
 * 引数：s 新しいサイズ
 */
void SpriteSheet::SetPolygonSize(Float2 s)
{
    size = s;
}

/**
 * 概要：表示位置を取得
 * 返り値：現在の位置
 */
Float3 SpriteSheet::GetPolygonPosition()
{
    return position;
}

/**
 * 概要：表示サイズを取得
 * 返り値：現在のサイズ
 */
Float2 SpriteSheet::GetPolygonSize()
{
    return size;
}

/**
 * 概要：現在のアニメーション名を取得
 * 返り値：アニメーション名（設定されていない場合は空文字列）
 */
std::string SpriteSheet::GetCurrentAnimationName()
{
    if (currentAnimation == nullptr)
    {
        return "";
    }

    // マップから現在のアニメーションの名前を検索
    for (const auto& pair : animations)
    {
        if (pair.second == currentAnimation)
        {
            return pair.first;
        }
    }

    return "";
}

/**
 * 概要：指定した名前のアニメーションが存在するか
 * 引数：name アニメーション名
 * 返り値：存在する場合true
 */
bool SpriteSheet::HasAnimation(const std::string& name)
{
    return animations.find(name) != animations.end();
}

/**
 * 概要：デバッグ情報を取得
 * 返り値：デバッグ用の文字列
 */
std::string SpriteSheet::GetDebugInfo()
{
    char buffer[512];
    std::snprintf(buffer, sizeof(buffer),
        "=== SpriteSheet Debug Info ===\n"
        "Texture: %s (ID: %u)\n"
        "Division: %d x %d\n"
        "Position: (%.1f, %.1f, %.1f)\n"
        "Size: %.1f x %.1f\n"
        "Animations: %zu\n"
        "Current: %s\n",
        filePath.c_str(), textureId,
        divideX, divideY,
        position.x, position.y, position.z,
        size.x, size.y,
        animations.size(),
        GetCurrentAnimationName().c_str()
    );

    std::string result = buffer;

    // 現在のアニメーションの詳細情報を追加
    if (currentAnimation != nullptr)
    {
        result += "\n--- Current Animation ---\n";
        result += currentAnimation->GetDebugInfo();
    }

    return result;
}