#pragma once
#include "GameObject.h"
#include <vector>

enum WeaponType	// 武器の種類
{
	WEAPON_VULCAN,	// バルカン				（ 扇形射撃 ）
	WEAPON_LASER,	// ビームキャノン		（ 直線射撃 ）
	WEAPON_HOMING,	// ホーミングミサイル	（ 追尾射撃 ）
	WEAPON_COUNT,	// ←--- 武器の総数		（ count 用 ）
};

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

//======================================================================================//
//  各武器の発射物構造体 ------> 武器ごとに独立した「飛んでいる1発」のデータ			//
//--------------------------------------------------------------------------------------//
// 各武器の弾は挙動が異なる(Vulcan/Laserは直進のみ、Homingは追尾ロジックが必要)ため、	//
// 共通のstructにまとめず、武器ごとに専用のstructで管理する。							//
// これにより、ある武器の仕様変更が他の武器に影響しない。								//
//======================================================================================//

struct VulcanBullet
{
	GameObject* obj;						// 弾の GameObject
	float vulcanVelocityX, vulcanVelocityY;	// 弾の速度ベクトル（毎秒ピクセル）
};
extern std::vector<VulcanBullet> Spread;	// 弾リストの実体は GameScene.cpp にあります。

struct LaserBeam
{
	GameObject* obj;						// 弾のGameObject
	float laserVelocityX, laserVelocityY;	// レーザーの速度ベクトル(毎秒ピクセル)
};
extern std::vector<LaserBeam> Straight;		// レーザーリストの実体は GameScene.cpp にあります

struct HomingMissile
{
	GameObject* obj;						// 弾のGameObject
	float omingVelocityX, homingVelocityY;	// ホーミングミサイルの速度ベクトル(毎秒ピクセル)
	float homingTimer = 0.0f;				// 発射からの経過時間(段階判定に使う)
};
extern std::vector<HomingMissile> Chase;	// ホーミングリストの実体は GameScene.cpp にあります。

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
		MODE_COUNT	// ← 形態の総数
	};

private:
	Mode mode = FIGHTER; // 初期モードはファイター
	WeaponType currentWeapon = WEAPON_VULCAN;	// 今の武器
	float fireCooldown = 0.0f; // 射撃冷却時間、次の弾まであと何秒待つか

	//--- 武器 の表(武器だけで決まる、形態と無関係) ---
	float bulletSpeedTable[WEAPON_COUNT];	// 弾速(武器ごと)

	//--- 武器 × 形態 の表 [武器][形態] ---
	float  fireIntervalTable[WEAPON_COUNT][MODE_COUNT];	// 射速
	Float2 muzzleTable[WEAPON_COUNT][MODE_COUNT];		// 炮口offset

	//--------------------------------------------------------------//
	// ↓この3つは「モードで変わる」ので const じゃない				//
	// （変形する＝値が変わる、だから変数で待つ必要がある）			//
	//--------------------------------------------------------------//
	float speed = 700.0f;			// 移動速度
	float fireInterval = 0.06f;		// 発射間隔（小さいほどで連射が密になる）
	float invincibleTimer = 0.0f;	// 無敵時間計算（秒）

	int HP = 100;			// プレイヤーのHP初期値
	int max_HP = 100;		// プレイヤーのHP最大値
	int repairCore = 3;		// リペアコア初期持ち数
	int max_repairCore = 6;	// リペアコア最大持ち数

	void ApplyMode();				// モードに合わせてパラメータを適用する(値を一式セット)
	void FireVulcan(float x, float y, int count, float spreadDegree, float bulletSpeed);	//扇形に弾を発射する
	void FireLaser(float x, float y, float bulletSpeed);	// 直線にレーザーを発射する
	void FireHoming(float x, float y, float bulletSpeed);	// 追尾ミサイルを発射する
	void UseRepairCore();	// リペアコアを1つ消費してHP全回復＋無敵時間を得る
public:
	void Setup(float x, float y); // 初期化（画像、当たり判定、位置）
	void Update(float dt) override; // 每フレーム：移動+変形+射撃（自動で呼ばれる）
	void ToggleMode(); // 人形 ⇔ ファイター を切り替える
	void CollisionReaction(GameObject* opponent) override; // 當碰撞發生時的反應
	void TakeDamage(int damage);	// ダメージを受ける(無敵中は無効。HPが0以下でリペアコアを自動使用)
	bool IsGameOver()const;	// HPが0以下、かつリペアコアも尽きたかどうか
};