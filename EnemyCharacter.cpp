#include "EnemyCharacter.h"

//==============================================================================//
// 敵が消える範囲（この外に出たら自分で回収する）								//
//==============================================================================//

static const float ENEMY_DESPAWN = -900.0f;	// これより左(X座標)に出たら自分で消える（despawn する）

//==============================================================================//
// Setup：敵の初期化（画像・当たり判定・位置・速度・HP）						//
//==============================================================================//
void EnemyCharacter::Setup(float Enemy_posX, float Enemy_posY, float Enemy_sizeX, float Enemy_sizeY, float Enemy_collisionX, float Enemy_collisionY, float Colums, float Rows, float initialVelocityX, int initialHp, const char* texturePath)
{
	velocityX = initialVelocityX; //速度を覚える
	hp = initialHp; //体力を覚える

	Activation();	//有効化
	Show();	//描画有効化
	GetSprite().Initialize();	// スプライト初期化
	SetObjectType(Enemy);	//オブジェクト分類

	GetSprite().LoadTexture(texturePath);	//画像読み込む
	GetSprite().DivideAnimationCells(Colums, Rows); //分割
	GetSprite().CreateAnimation("EnemyAnim", 0, Colums-1); //アニメーションを作成
	GetSprite().SetAnimationFrameTime("EnemyAnim", (Colums/10.0f));
	GetSprite().SetAnimation("EnemyAnim"); //アニメーションを指定

	GetSprite().SetPolygonSize(MakeFloat2(Enemy_sizeX, Enemy_sizeY));	//サイズ

	GenerateRectangleCollision(1);	//方形の当たり判定を生成
	SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(Enemy_collisionX, Enemy_collisionY));	//判定サイズ

	SetPosition(MakeFloat3(Enemy_posX, Enemy_posY, 0.0f));	//出現位置を指定する
}

//======================================================================================//
// Update：毎フレーム、左へ飛ぶ＋画面外なら自分で消える									//
//--------------------------------------------------------------------------------------//
// 敵との当たり判定（弾・プレイヤー）はGameScene 側のループが担当する（今はそのまま）	//
//======================================================================================//
void EnemyCharacter::Update(float dt)
{
	// 1) 自分の速度で移動（左へ）
	Float3 enemyPosition = GetPosition();
	enemyPosition.x += velocityX * dt;
	SetPosition(enemyPosition);

	// 2）画面外に出たら自分で消える
	if (enemyPosition.x < ENEMY_DESPAWN)
	{
		Hide();
		Deactivation();
		return;
	}

	// 3）基底Update：当たり判定の同期＆アニメーション更新
	GameObject::Update(dt);
}

//======================================================================================//
//	TakeDamage：当たり判定の同期＋アニメーション更新									//
//======================================================================================//
void EnemyCharacter::TakeDamage(int damage)
{
	hp -= damage;
	if (hp <= 0)
	{
		Hide();	// 描画無効化
		Deactivation();	// 死亡→無効化（リストから外す）
	}
}