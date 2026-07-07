#include "PlayerCharacter.h"
#include "Game.h"
#include "Input.h"
#include "Controller.h"
#include <cmath>// ←————————————————————————————————— cosf/sinf で角度を計算します

//======================================================================================//
//  モードで変わらない固定パラメータ（const）											//
//--------------------------------------------------------------------------------------//
// 移動できる範囲の制限（画面、UI内に収めるため）										//
//======================================================================================//

static const float BOUND_TOP = 320.0f;		//プレイヤーの上画面活動限界
static const float BOUND_BOTTOM = -300.0f;	//プレイヤーの下画面活動限界
static const float BOUND_LEFT = -605.0f;	//プレイヤーの左画面活動限界
static const float BOUND_RIGHT = 825.0f;	//プレイヤーの右画面活動限界

//銃口の座標offset（胸のバルカンの位置、実機で微調整する）
static const float VULCAN_MUZZLE_OFFSET_X = -2.0f;		//銃口のX座標offset
static const float VULCAN_MUZZLE_OFFSET_Y = -37.0f;	//銃口のY座標offset

//======================================================================================//
// ApplyMode() ---> モードに合わせてパラメータを適用する(値を一式セット)				//
//--------------------------------------------------------------------------------------//
// プレイヤー機体のモードに合わせて、移動速度、射撃間隔、弾の速度をセットする			//
// 変形の心臓部、速度、武装の発射間隔、弾速、ColliderBoxを１か所でまとめて切り替える	//
//　こうすれば「速度は変わったのに box は人形のまま」みたいな半端な状態が起きない。		//
//======================================================================================//

void PlayerCharacter::ApplyMode()
{
	if (mode == FIGHTER)
	{
		speed = 1200.0f;		// ロボットモードの移動速度
		fireInterval = 0.05f;	// ロボットモードの発射間隔
		bulletSpeed = 2600.0f;	// ロボットモードの弾の速度
		SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(70.0f, 40.0f));	// ファイターモードの当たり判定
		GetSprite().SetAnimation("FighterMode");	// ファイターモードの画像
	}
	else if (mode == ROBOT)
	{
		speed = 750.0f;			// ファイターモードの移動速度
		fireInterval = 0.09f;	// ファイターモードの発射間隔
		bulletSpeed = 2000.0f;	// ファイターモードの弾の速度
		SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(40.0f, 70.0f));	// ロボットモードの当たり判定
		GetSprite().SetAnimation("RobotMode_Idle");	// ロボットモードの画像
	}
}

//======================================================================================//
// ToggleMode() ---> 人形 ⇔ ファイター を切り替える									//
//--------------------------------------------------------------------------------------//
void PlayerCharacter::ToggleMode()
{
	if (mode == FIGHTER)
	{
		mode = ROBOT;
	}
	else if (mode == ROBOT)//elseだけでもできる
	{
		mode = FIGHTER;
	}
	ApplyMode(); // 1モードに合わせてパラメータを適用する
}

//======================================================================================//
// Setup() ---> 初期化（画像、当たり判定、位置）										//
//======================================================================================//
void PlayerCharacter::Setup(float x, float y)
{
	// --- キー入力のバインド ---
	Input::Bind(Command::Attack, Key::Space);	//スペースキー = 射撃
	Input::Bind(Command::SubAction, Key::LShift);	// 左Shiftキー = 変形
	Input::Bind(Command::SubAction, Key::RShift);	// 右Shiftキー = 変形

	// --- スプライト初期化 ---
	Activation();	//有効化
	Show();	//描画有効化
	GetSprite().Initialize();	//スプライト初期化
	GetSprite().SetPolygonSize(MakeFloat2(200.0f, 200.0f));	//サイズ指定
	GetSprite().LoadTexture("rom:/texture/Vanguard_SpriteSheet.tga");	//画像読み込み
	GetSprite().DivideAnimationCells(36, 2);	//テキスチャーが縦横何分割になっているか指定
	GetSprite().CreateAnimation("RobotMode_Idle", 0, 35);	//アニメーションを作成（０フレイムなので１コマ）
	GetSprite().SetAnimation("RobotMode_Idle");	//アニメーションを指定
	GetSprite().SetAnimationFrameTime("RobotMode_Idle", 0.36f);	//アニメーション１週に何フレームかけるか
	GetSprite().CreateAnimation("FighterMode", 36, 71);	//アニメーションを作成（０フレイムなので１コマ）
	GetSprite().SetAnimation("FighterMode");	//アニメーションを指定
	GetSprite().SetAnimationFrameTime("FighterMode", 0.36f);	//アニメーション１週に何フレームかけるか

	// --- 当たり判定初期化 ---
	GenerateRectangleCollision(1);	//自身に方形の当たり判定を生成（引数で渡した分だけ作成する）
	SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(67.0f, 40.0f));	//方形のパラメータをセット

	// --- 座標初期化 ---
	SetPosition(MakeFloat3(x, y, 0.0f));	//座標指定

	mode = FIGHTER;		// 初期モードはファイター
	ApplyMode();		// モードに合わせてパラメータを適用する
}

//======================================================================================//
// Update() ---> 每フレーム：移動+変形+射撃（自動で呼ばれる）							//
//--------------------------------------------------------------------------------------//
// プレイヤー機体の毎フレーム処理（移動、変形、射撃）									//
// 毎フレーム自動で呼び出す																//
// 0）変形 → 1）移動 → 2）射撃 → 3）基底Updateの順									//
//======================================================================================//
void PlayerCharacter::Update(float dt)
{
	//--------------------------------------------------------------------------//
	// 0）変形: [ Shift キー ]で　人形⇔ファイター								//
	//--------------------------------------------------------------------------//
	if (Input::IsTrigger(Command::SubAction))	//Shiftキーが押された瞬間
	{
		ToggleMode();	// モードチェンジ
	}

	//--------------------------------------------------------------------------//
	// 1）移動 + 画面内の活動範囲の納め											//
	//--------------------------------------------------------------------------//

	// 移動制御
	Float3 p = GetPosition();	// 現在の座標を取得
	p.x += Input::GetAxis(Command::MoveX) * speed * dt;	// 左右移動
	p.y += Input::GetAxis(Command::MoveY) * speed * dt;	// 上下移動

	// 画面内の活動範囲に納める
	if (p.y > BOUND_TOP) p.y = BOUND_TOP;			// 上端で止める
	if (p.y < BOUND_BOTTOM) p.y = BOUND_BOTTOM;		// 下端で止める
	if (p.x < BOUND_LEFT) p.x = BOUND_LEFT;			// 左端で止める
	if (p.x > BOUND_RIGHT) p.x = BOUND_RIGHT;		// 右端で止める
	SetPosition(p);	// 座標を更新する

	//--------------------------------------------------------------------------//
	// 2）射撃: [ Space キー ]で　バルカン発射 							   		//
	// （FireVulcan() はモードで変わる	 　　　　　　　　　　　　　　　　		//
	//--------------------------------------------------------------------------//
	if (fireCooldown > 0.0f)	// 冷却中ならカウントダウン
	{
		fireCooldown -= dt;	// 毎フレームで減していく
	}

	if (Input::IsPress(Command::Attack) && fireCooldown <= 0.0f)	//押していて＆冷却時間が0以下なら
	{
		Float3 p = GetPosition();	// 現在の座標を取得
		FireVulcan(p.x + VULCAN_MUZZLE_OFFSET_X, p.y + VULCAN_MUZZLE_OFFSET_Y, 5, 20.0f);	// バルカン発射
		fireCooldown = fireInterval;	// 冷却時間をリセット
	}

	//--------------------------------------------------------------------------//
	// 3）基底Update()を呼ぶ													//
	// 当たり判定の同期＋アニメーションを進める（呼ばないとアニメが止める		//
	//--------------------------------------------------------------------------//
	GameObject::Update(dt);
}

//======================================================================================//
// FireVulcan: 扇形に count 発撃つ														//
//--------------------------------------------------------------------------------------//
// x, y = 発射位置（銃口の座標）														//
// 右向き＝0度。0度を中心に上下広げる													//
// 三角関数で各弾の vx と vy を出す（速さは全部同じ）									//
//======================================================================================//
void PlayerCharacter::FireVulcan(float x, float y, int count, float spreadDegree)
{
	float start = -spreadDegree * 0.5f;								// 扇の一番端の角度
	float step = (count > 1) ? spreadDegree / (count - 1) : 0.0f;	// 扇の角度を count 等分する（1発ごとの角度差）

	for (int i = 0; i < count; i++) //count発ぶん繰り返す
	{
		float degree = start + step * i;	// この弾の角度
		float radian = degree * 3.14159265f / 180.0f;	// 角度 → ラジアンに変換

		float vx = cosf(radian) * bulletSpeed;	// 発散方向は右で（bulletSpeed はモードで変わる）
		float vy = sinf(radian) * bulletSpeed;	// 上下に散らす

		Projectile* shot = (Projectile*)GameAPI.AddObject(new Projectile);	// 弾の GameObject を生成
		shot->Activation();	// 有効化
		shot->Show();	// 描画有効化
		shot->GetSprite().Initialize();	// スプライト初期化
		shot->GetSprite().LoadTexture("rom:/texture/Vulcan.tga");	// 画像読み込み
		shot->GetSprite().DivideAnimationCells(1, 1);	// テキスチャーが縦横何分割になっているか指定
		shot->GetSprite().CreateAnimation("Vulcan", 0, 0);	// アニメーションを作成（０フレイムなので１コマ）
		shot->GetSprite().SetAnimation("Vulcan");	// アニメーションを指定
		shot->GetSprite().SetPolygonSize(MakeFloat2(20.0f, 20.0f));	// サイズ指定
		shot->GenerateRectangleCollision(1);	// 自身に方形の当たり判定を生成（引数で渡した分だけ作成する）
		shot->SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(20.0f, 20.0f));	// 方形のパラメータをセット
		shot->SetPosition(MakeFloat3(x, y, 0.0f));	// 座標指定
		shot->SetVelocity(vx, vy);

		Spread.push_back(shot);	// 弾リストに追加(速度も一緒に覚える)
	}
}

void PlayerCharacter::CollisionReaction(GameObject* opponent)
{
	//TODO: 衝突時の反応をここに書く
}