#pragma once
#include "GameObject.h"

//==========================================================================================//
// Enemy（敵機）																			//
//------------------------------------------------------------------------------------------//
// 画面右から出現、左方向へ飛ぶ																//
// GameObject を継承している、自動でUpdate()で呼ぶ											//
// 自分の速度(vx)で左へ飛び、画面外に出たら自分で無効化する（プレイヤーの視界から消える）	//
// 弾に当たった時のダメージ判定・削除は GameSceneで管理する									//
//==========================================================================================//

class EnemyCharacter :public GameObject
{
	float velocityX = 0.0f;	// X方向の速度（毎秒ピクセル）
	int hp = 1;			// 体力（0以下になったら死亡）

public:
	void Setup(float Enemy_posX, float Enemy_posY, float Enemy_sizX, float Enemy_sizeY, float Enemy_collisionX, float Enemy_collisionY, float cellX, float cellY, float initialVelocityX, int initialHp, const char* texturePath);
	void Update(float dt)override;	//毎フレーム：左へ飛ぶ＋画面外なら自分で消える

	void TakeDamage(int damage);			// ダメージを受ける（）
	//bool IsDead() const { return hp <= 0; }	// 死亡判定
};