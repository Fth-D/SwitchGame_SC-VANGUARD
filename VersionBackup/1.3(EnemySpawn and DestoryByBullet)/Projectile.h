#pragma once
#include "GameObject.h"

//======================================================================================//
//  Projectile（発射物）：Vulcan / Laser / Homing 共通の「飛んでいる1発」				//
//--------------------------------------------------------------------------------------//
// GameObject を継承しているので、エンジンが毎フレーム自動で Update() を呼ぶ。			//
// 自分の速度（vx, vy）で前へ飛び、画面外に出たら自分で無効化する（＝自分で消える）。	//
// 見た目（画像・アニメ・サイズ）は生成する側（FireVulcan など）が設定する。			//
// 敵との当たり判定は GameScene 側のループが担当する（今は手動のまま）。				//
//======================================================================================//
class Projectile : public GameObject
{
	float velocityX = 0.0f;	// X方向の速度（毎秒ピクセル）
	float velocityY = 0.0f;	// Y方向の速度（毎秒ピクセル）

public:
	void SetVelocity(float x, float y) { velocityX = x; velocityY = y; }	// 速度をセット
	void Update(float dt) override;							// 毎フレーム：飛ぶ＋画面外なら消える
};