#include "GameObject.h"
#include "Collision.h"
#include "CameraAPI.h"

/**
 * 概要：コンストラクタ
 * 内部状態を初期値にそろえる
 */
GameObject::GameObject()
{
	rect = nullptr;       // 矩形の当たり判定を持つ場合に動的生成する。nullptr = 何も指さないポインタ。
	ownedRect = 0;        // 保持している矩形の当たり判定の数
	circle = nullptr;     // 同上。こちらは円形。
	ownedCircle = 0;      // 保持している円形の当たり判定の数
	objectId = 0;         // ゲームオブジェクトの通し番号
	isActive = false;     // ゲームオブジェクトの更新処理を実行するかどうか
	isRender = false;     // ゲームオブジェクトを描画するかどうか
}

/**
 * 概要：デストラクタ
 * 当たり判定の解放などを行う
 */
GameObject::~GameObject()
{
	DeleteAllCollision();
	//sprite.UnloadTexture(); は不要になりました。
}

/**
 * 概要：オブジェクトIDを設定
 * 引数：newId 新しいID
 */
void GameObject::SetObjectId(int newId)
{
	objectId = newId;
}

/**
 * 概要：オブジェクトIDを取得
 * 返り値：オブジェクトID
 */
int GameObject::GetObjectId()
{
	return objectId;
}

/**
 * 概要：オブジェクトのアクティブ状態を設定
 * 引数：newState 新しい状態
 */
void GameObject::SetObjectActiveState(bool newState)
{
	isActive = newState;
}

/**
 * 概要：オブジェクトのアクティブ状態を取得
 * 返り値：アクティブ状態
 */
bool GameObject::GetObjectActiveState()
{
	return isActive;
}

/**
 * 概要：オブジェクトの描画状態を設定
 * 引数：newState 新しい状態
 */
void GameObject::SetObjectRenderState(bool newState)
{
	isRender = newState;
}

/**
 * 概要：オブジェクトの描画状態を取得
 * 返り値：描画状態
 */
bool GameObject::GetObjectRenderState()
{
	return isRender;
}

void GameObject::SetObjectType(ObjectType newType)
{
	type = newType;
}

ObjectType GameObject::GetObjectType()
{
	return type;
}

void GameObject::SetFixedScreenPositionState(bool newState)
{
	sprite.SetFixedScreenPosition(newState);
}

bool GameObject::GetFixedScreenPositionState()
{
	return sprite.GetFixedScreenPosition();
}

/**
 * 概要：オブジェクトの更新処理を有効にする
 */
void GameObject::Activation()
{
	isActive = true;
}

/**
 * 概要：オブジェクトの更新処理を無効にする
 */
void GameObject::Deactivation()
{
	isActive = false;
}

/**
 * 概要：オブジェクトの描画処理を有効にする
 */
void GameObject::Show()
{
	isRender = true;
}

/**
 * 概要：オブジェクトの描画処理を無効にする
 */
void GameObject::Hide()
{
	isRender = false;
}

/**
 * 概要：オブジェクトを移動させる
 * 引数：moveValue 移動量
 */
void GameObject::Move(Float3 moveValue)
{
	Float3 newPosition = MakeFloat3(position.x + moveValue.x, position.y + moveValue.y, position.z + moveValue.z);
	SetPosition(newPosition);
}

/**
 * 概要：オブジェクトの座標を指定
 * 引数：newPosition 新しい座標
 */
void GameObject::SetPosition(Float3 newPosition)
{
	position = newPosition;
}

/**
 * 概要：オブジェクトの座標を取得
 * 返り値：現在の座標
 */
Float3 GameObject::GetPosition()
{
	return position;
}

/**
 * 概要：スプライトへの参照を取得
 * 返り値：スプライトへの参照
 */
SpriteSheet& GameObject::GetSprite()
{
	return sprite;
}

/**
 * 概要：自身に矩形の当たり判定を生成（引数で渡した分だけ作成する）
 * 引数：num 生成する数（1以上）
 */
void GameObject::GenerateRectangleCollision(unsigned int num)
{
	if (num <= 0)
		return;

	// 一度当たり判定を削除
	DeleteRectangleCollision();

	// 常に配列として作成（1つでも配列）
	rect = new Rectangle[num];
	ownedRect = num;
}

/**
 * 概要：自身に円形の当たり判定を生成（引数で渡した分だけ作成する）
 * 引数：num 生成する数（1以上）
 */
void GameObject::GenerateCircleCollision(unsigned int num)
{
	if (num <= 0)
		return;

	DeleteCircleCollision();

	// 常に配列として作成（1つでも配列）
	circle = new Circle[num];
	ownedCircle = num;
}

/**
 * 概要：矩形の当たり判定を削除
 */
void GameObject::DeleteRectangleCollision()
{
	if (rect == nullptr)
		return;

	delete[] rect;
	rect = nullptr;
	ownedRect = 0;
}

/**
 * 概要：円形の当たり判定を削除
 */
void GameObject::DeleteCircleCollision()
{
	if (circle == nullptr)
		return;

	delete[] circle;
	circle = nullptr;
	ownedCircle = 0;
}

/**
 * 概要：すべての当たり判定を削除
 */
void GameObject::DeleteAllCollision()
{
	DeleteRectangleCollision();
	DeleteCircleCollision();
}

/**
 * 概要：特定の矩形当たり判定を取得
 * 引数：no 当たり判定の番号
 * 返り値：当たり判定へのポインタ
 */
const Rectangle* GameObject::GetRectangleCollision(int no)
{
	if (rect == nullptr || (unsigned int)no >= ownedRect)
		return nullptr;

	return &rect[no];
}

/**
 * 概要：特定の円形当たり判定を取得
 * 引数：no 当たり判定の番号
 * 返り値：当たり判定へのポインタ
 */
const Circle* GameObject::GetCircleCollision(int no)
{
	if (circle == nullptr || (unsigned int)no >= ownedCircle)
		return nullptr;

	return &circle[no];
}

/**
 * 概要：矩形の当たり判定の位置を設定
 * 引数：no 当たり判定の番号
 * 引数：newCenter 新しい中心座標
 */
void GameObject::SetRectanglePosition(unsigned int no, Float2 newCenter)
{
	if (rect == nullptr || no >= ownedRect)
		return;

	rect[no].x = newCenter.x; // Rectangle構造体の実装に依存
	rect[no].y = newCenter.y; // Rectangle構造体の実装に依存
}

/**
 * 概要：矩形の当たり判定のパラメータを設定
 * 引数：no 当たり判定の番号
 * 引数：newCenter 新しい中心座標
 * 引数：newSize 新しいサイズ
 */
void GameObject::SetRectangleParameter(unsigned int no, Float2 newCenter, Float2 newSize)
{
	if (rect == nullptr || no >= ownedRect)
		return;

	rect[no].x = newCenter.x; // Rectangle構造体の実装に依存
	rect[no].y = newCenter.y;
	rect[no].width = newSize.x;
	rect[no].height = newSize.y;
}

/**
 * 概要：円形の当たり判定の位置を設定
 * 引数：no 当たり判定の番号
 * 引数：newCenter 新しい中心座標
 */
void GameObject::SetCirclePosition(unsigned int no, Float2 newCenter)
{
	if (circle == nullptr || no >= ownedCircle)
		return;

	circle[no].x = newCenter.x; // Circle構造体の実装に依存
	circle[no].y = newCenter.y;
}

/**
 * 概要：円形の当たり判定のパラメータを設定
 * 引数：no 当たり判定の番号
 * 引数：newCenter 新しい中心座標
 * 引数：newRadius 新しい半径
 */
void GameObject::SetCircleParameter(unsigned int no, Float2 newCenter, float newRadius)
{
	if (circle == nullptr || no >= ownedCircle)
		return;

	circle[no].x = newCenter.x; // Circle構造体の実装に依存
	circle[no].y = newCenter.y; // Circle構造体の実装に依存
	circle[no].radius = newRadius;
}

/**
 * 概要：当たり判定をオブジェクト自身の座標と連動させて更新
 */
void GameObject::UpdateCollision()
{
	for (unsigned int i = 0; i < ownedRect; i++)
	{
		SetRectanglePosition(i, MakeFloat2(position.x, position.y));
	}

	for (unsigned int i = 0; i < ownedCircle; i++)
	{
		SetCirclePosition(i, MakeFloat2(position.x, position.y));
	}
}

/**
 * 概要：ゲームオブジェクトの更新処理
 */
void GameObject::Update(float dt)
{
	if (isActive == false)
		return;

	UpdateCollision();
	sprite.Update();  // アニメーション更新
}

/**
 * 概要：スプライトの描画処理
 */
void GameObject::Render()
{
	sprite.Render();
}

/**
 * 概要：ゲームオブジェクトの描画処理
 */
void GameObject::Draw()
{
	if (isRender == false)
		return;

	if (sprite.GetFixedScreenPosition() == false)
	{
		Float2 screenPos = WorldToScreen(position);
		sprite.SetPolygonPosition(MakeFloat3(screenPos.x, screenPos.y, position.z));
	}
	else
	{
		sprite.SetPolygonPosition(position);
	}

	Render();
}
