#pragma once
#include "GameObject.h"
#include "Projectile.h"
#include <vector>

//======================================================================================//
//  弾（VulcanBullet）構造体 ------> 弾（Vulcan）1発分のデータ							//
//--------------------------------------------------------------------------------------//
// VulcanBullet は「扇形に飛ぶ弾」の情報をまとめた構造体です。							//
// 弾は GameObject で表現されますが、GameObject には「速度」という概念がないため、		//
// 弾の速度を別途管理する必要があります。												//
// 弾は１発ずつ自分の速度ベクトル（vx, vy）を持つので、									//
// Update() の中で毎フレーム vx, vy に従って座標を更新する必要があります。				//
// こうすれば扇形.斜めに飛ぶ弾をまとめて扱える											//
// 弾リストの実体は SampleScene.cpp にあります。										//
//======================================================================================//

extern std::vector<Projectile*> Spread;	// 弾リストの実体は SampleScene.cpp にあります。
/*
struct VulcanBullet
{
	GameObject* obj;	// 弾の GameObject
	float vx, vy;		// 弾の速度ベクトル（毎秒ピクセル）
};
extern std::vector<VulcanBullet> Spread;	// 弾リストの実体は SampleScene.cpp にあります。
*/

//======================================================================================//
//  Player Class ---> Player: プレイヤー機体											//
//--------------------------------------------------------------------------------------//
// プレイヤー機体は「ロボットモード」と「ファイターモード」の2種類のモードを持つ。		//
// モードによって移動速度、射撃間隔、弾の速度が変化する。								//
// プレイヤー機体は「射撃冷却時間」を持つ。												//
// 射撃冷却時間が0以下のときに射撃可能。射撃すると射撃冷却時間がリセットされる。		//
// GameObject を継承（＝自分自身が、GameObject)											//
// エンジン毎フレイムUpdate()を自動で呼ぶ												//
// 変形出来る：ROBOT（人形ロボット）⇔ FIGHTER（ファイター）							//
//======================================================================================//
class PlayerCharacter : public GameObject
{
public:
	enum Mode	//モードの種類
	{
		ROBOT,
		FIGHTER,
	};

private:
	Mode mode = FIGHTER; // 初期モードはファイター
	float fireCooldown = 0.0f; // 射撃冷却時間、次の弾まであと何秒待つか

	//--------------------------------------------------------------//
	// ↓この3つは「モードで変わる」ので const じゃない				//
	// （変形する＝値が変わる、だから変数で待つ必要がある）			//
	//--------------------------------------------------------------//
	float speed = 700.0f;			// 移動速度
	float fireInterval = 0.06f;		// 発射間隔（小さいほどで連射が密になる）
	float bulletSpeed = 2400.0f;	// 弾の速度

	void ApplyMode();				// モードに合わせてパラメータを適用する(値を一式セット)
	void FireVulcan(float x, float y, int count, float spreadDegree);	//扇形に弾を発射する

public:
	void Setup(float x, float y); // 初期化（画像、当たり判定、位置）
	void Update(float dt) override; // 每フレーム：移動+変形+射撃（自動で呼ばれる）
	void ToggleMode(); // 人形 ⇔ ファイター を切り替える
	void CollisionReaction(GameObject* opponent) override; // 當碰撞發生時的反應
};