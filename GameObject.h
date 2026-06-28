#pragma once
#include "SpriteSheet.h"
#include "Platform/PlatformTypes.h"
#include <string>

// 当たり判定用構造体の前方宣言（ヘッダにヘッダをインクルードしないための措置）
struct Rectangle;
struct Circle;

/**
* ゲームオブジェクトの種類
*/
enum ObjectType
{
	UI = 0,
	Player = 1,
	Enemy = 2,
	Bullet = 3,
	Other = 4,

	TYPE_OF_OBJECTTYPE,
};


/**
 * 概要：ゲームオブジェクトのクラス
 * ゲームオブジェクト1つ分を表現する
 */
class GameObject
{
protected:
	SpriteSheet sprite;
	Rectangle* rect = nullptr;		// 矩形の当たり判定を持つ場合、この変数に1つまたは2つ以上の当たり判定を動的生成する。nullptr = 何も指さないポインタ。
	unsigned int ownedRect = 0;				// 保持している矩形の当たり判定の数
	Circle* circle = nullptr;		// 同上。こちらは円形。
	unsigned int ownedCircle = 0;			// 保持している円形の当たり判定の数
	int objectId = 0;				// ゲームオブジェクトの通し番号
	bool isActive = false;			// ゲームオブジェクトの更新処理を実行するかどうか
	bool isRender = false;			// ゲームオブジェクトを描画するかどうか
	Float3 position = MakeFloat3(0.0f, 0.0f, 0.0f);	// オブジェクトのゲーム世界内での座標
	float angle = 0.0f;				// オブジェクトの回転角度（オイラー角）

	ObjectType type;				// オブジェクトごとに設定されるタイプ

public:
	GameObject();
	virtual ~GameObject();

	// ゲッター・セッター
	void SetObjectId(int newId);
	int GetObjectId();
	void SetObjectActiveState(bool newState);
	bool GetObjectActiveState();
	void SetObjectRenderState(bool newState);
	bool GetObjectRenderState();
	void SetObjectType(ObjectType newType);
	void SetFixedScreenPositionState(bool newState);
	bool GetFixedScreenPositionState();
	ObjectType GetObjectType();

	void Activation();     // オブジェクトを更新する状態にする
	void Deactivation();   // オブジェクトを更新しない状態にする
	void Show();           // オブジェクトを描画する状態にする
	void Hide();           // オブジェクトを描画しない状態にする

	// オブジェクトの座標を指定
	virtual void SetPosition(Float3 newPosition);
	// オブジェクトの座標を返す
	Float3 GetPosition();
	// オブジェクトを移動させる
	void Move(Float3 moveValue);

	// スプライトへのアクセス（これが主要なインターフェース）
	SpriteSheet& GetSprite();

	// 当たり判定管理
	void GenerateRectangleCollision(unsigned int num);   // 自身に矩形の当たり判定を生成（引数で渡した分だけ作成する）
	void GenerateCircleCollision(unsigned int num);      // 自身に円形の当たり判定を生成（引数で渡した分だけ作成する）
	void DeleteRectangleCollision();                     // 矩形の当たり判定を削除
	void DeleteCircleCollision();                        // 円形の当たり判定を削除
	void DeleteAllCollision();                           // すべての当たり判定を削除
	void SetRectanglePosition(unsigned int no, Float2 newCenter);
	void SetRectangleParameter(unsigned int no, Float2 newCenter, Float2 newSize);
	void SetCirclePosition(unsigned int no, Float2 newCenter);
	void SetCircleParameter(unsigned int no, Float2 newCenter, float newRadius);
	const Rectangle* GetRectangleCollision(int no);      // 特定の当たり判定のみを取得
	const Circle* GetCircleCollision(int no);            // 同上
	void UpdateCollision();

	// 描画処理
	virtual void Update(float dt);
	void Draw();
	virtual void Render();

	// ここから下の関数はオーバーライドして使う
	// 当たり判定が重なっている間、毎フレーム呼ばれる (Unity の OnCollisionStay 相当)
	virtual void CollisionReaction(GameObject* opponent) {};
	// 当たり判定が重なり始めたフレームに1回だけ呼ばれる (Unity の OnCollisionEnter 相当)
	virtual void CollisionReactionEnter(GameObject* opponent) {};
	// 当たり判定の重なりが解消したフレームに1回だけ呼ばれる (Unity の OnCollisionExit 相当)
	// 重なり中に相手オブジェクトが DeleteObject() で削除された場合も呼ばれる。
	virtual void CollisionReactionExit(GameObject* opponent) {};
};
