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

//銃口の座標 offset（胸のバルカンの位置、実機で微調整する）
static const float VULCAN_MUZZLE_OFFSET_X = -2.0f;		//銃口のX座標 offset
static const float VULCAN_MUZZLE_OFFSET_Y = -37.0f;	//銃口のY座標 offset

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
		bulletSpeed = 2600.0f;	// ロボットモードの弾の速度
		SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(70.0f, 40.0f));	// ファイターモードの当たり判定

		// --- ファイター用シートに切り替え ---
		GetSprite().LoadTexture("rom:/texture/FighterMode_sheet.tga");
		GetSprite().DivideAnimationCells(6, 6);
		GetSprite().CreateAnimation("FighterMode", 0, 35);
		GetSprite().SetAnimationFrameTime("FighterMode", 0.36f);
		GetSprite().SetAnimation("FighterMode");	// ファイターモードの画像
	}
	else if (mode == ROBOT)
	{
		speed = 750.0f;			// ファイターモードの移動速度
		bulletSpeed = 2000.0f;	// ファイターモードの弾の速度
		SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(40.0f, 70.0f));	// ロボットモードの当たり判定

		// --- ロボット用シートに切り替え ---
		GetSprite().LoadTexture("rom:/texture/Vanguard_Idle_sheet.tga");
		GetSprite().DivideAnimationCells(6, 6);
		GetSprite().CreateAnimation("RobotMode_Idle", 0, 35);
		GetSprite().SetAnimationFrameTime("RobotMode_Idle", 0.36f);
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
#if PLATFORM_NX
	// --- Switch (NX64) ---
	Input::Bind(Command::Attack, Button::ZR);		// ZR = 射撃
	Input::Bind(Command::SubAction, Button::Minus);	// Minus(-) = 変形
	Input::Bind(Command::NewCommand, Button::ZL);	// ZL = 
#else
	// --- PC (x64) ---
	Input::Bind(Command::Attack,	 Key::Space);	// スペースキー = 射撃
	Input::Bind(Command::SubAction,  Key::LShift);	// 左Shiftキー = 変形
	Input::Bind(Command::SubAction,  Key::RShift);	// 右Shiftキー = 変形
	Input::Bind(Command::NewCommand, Key::Tab);		// TAB = 武器切り替え
	Input::Bind(Command::ShoulderR,  Key::Right);	// → 右キー = 次の武器
	Input::Bind(Command::ShoulderL,  Key::Left);	// ← 左キー = 前の武器

#endif // PLATFORM_NX



	// --- スプライト初期化 ---
	Activation();	//有効化
	Show();	//描画有効化
	GetSprite().Initialize();	//スプライト初期化
	GetSprite().SetPolygonSize(MakeFloat2(200.0f, 200.0f));	//サイズ指定

	// --- 当たり判定初期化 ---
	GenerateRectangleCollision(1);	//自身に方形の当たり判定を生成（引数で渡した分だけ作成する）
	SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(67.0f, 40.0f));	//方形のパラメータをセット

	// --- 座標初期化 ---
	SetPosition(MakeFloat3(x, y, 0.0f));	//座標指定

	//--------------------------------------------------------------//
	// 武器 × 形態 の数値表を埋める								//
	//--------------------------------------------------------------//
	
	// --- 射速表 [武器][形態] ---
	fireIntervalTable[WEAPON_VULCAN][FIGHTER] = 0.05f;	// ファイターモードでバルカンの発射間隔
	fireIntervalTable[WEAPON_VULCAN][ROBOT] = 0.09f;	// ロボットモードでバルカンの発射間隔
	
	fireIntervalTable[WEAPON_LASER][FIGHTER] = 0.02f;	// ファイターモードでビームキャノンの発射間隔
	fireIntervalTable[WEAPON_LASER][ROBOT] = 0.03f;		// ロボットモードでビームキャノンの発射間隔

	fireIntervalTable[WEAPON_HOMING][FIGHTER] = 0.10f;	// ファイターモードでホーミングミサイルの発射間隔
	fireIntervalTable[WEAPON_HOMING][ROBOT] = 0.10f;	// ロボットモードでホーミングミサイルのの発射間隔

	// --- 炮口offset表 [武器][形態] ---
	muzzleTable[WEAPON_VULCAN][FIGHTER] = MakeFloat2(-2, -37);
	muzzleTable[WEAPON_VULCAN][ROBOT] = MakeFloat2(5, -30);

	muzzleTable[WEAPON_LASER][FIGHTER] = MakeFloat2(10, -20);
	muzzleTable[WEAPON_LASER][ROBOT] = MakeFloat2(15, -10);
	
	muzzleTable[WEAPON_HOMING][FIGHTER] = MakeFloat2(0, 0);
	muzzleTable[WEAPON_HOMING][ROBOT] = MakeFloat2(0, 0);

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
	Float3 player_position = GetPosition();	// 現在の座標を取得
	player_position.x += Input::GetAxis(Command::MoveX) * speed * dt;	// 左右移動
	player_position.y += Input::GetAxis(Command::MoveY) * speed * dt;	// 上下移動

	// 画面内の活動範囲に納める
	if (player_position.y > BOUND_TOP) player_position.y = BOUND_TOP;			// 上端で止める
	if (player_position.y < BOUND_BOTTOM) player_position.y = BOUND_BOTTOM;		// 下端で止める
	if (player_position.x < BOUND_LEFT) player_position.x = BOUND_LEFT;			// 左端で止める
	if (player_position.x > BOUND_RIGHT) player_position.x = BOUND_RIGHT;		// 右端で止める
	
	SetPosition(player_position);	// 座標を更新する

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
		// 砲口 offset を表から取得（ 武器×形態 ）
		Float2 muzzle = muzzleTable[currentWeapon][mode];	//砲口 offset
		float muzzleX = player_position.x + muzzle.x;		//実際の砲口X座標
		float muzzleY = player_position.y + muzzle.y;		//実際の砲口Y座標

		//　武器ごとに発射
		switch (currentWeapon)
		{
		case WEAPON_VULCAN:
			FireVulcan(muzzleX, muzzleY, 5, 20.0f);	//バルカン発射
			break;
		case WEAPON_LASER:
			FireLaser(muzzleX, muzzleY);		//	
			break;
		case WEAPON_HOMING:
			break;
		case WEAPON_COUNT:
			break;
		default:
			break;
		}

		//射速を表から所得してリセット（武器×形態）
		fireCooldown = fireIntervalTable[currentWeapon][mode];
	}

	//------------------------------------------------------------------------------//
	// 3）武器切り替え: [ TAB ] で Vulcan ⇔ Laser ⇔ Homing						//
	//------------------------------------------------------------------------------//
	
	// ZL / Tab = 快速切り替え
	if (Input::IsTrigger(Command::NewCommand))	// TABが押された瞬間
	{
		if (currentWeapon < 2)
		{
			currentWeapon = (WeaponType)(currentWeapon + 1);
		}
		else
		{
			currentWeapon = (WeaponType)(currentWeapon - 2);
		}
	}

	// R = 次の武器
	if (Input::IsTrigger(Command::ShoulderR))	// TABが押された瞬間
	{
		if (currentWeapon < 2)
		{
			currentWeapon = (WeaponType)(currentWeapon + 1);
		}
		else
		{
			currentWeapon = (WeaponType)(currentWeapon - 2);
		}
	}

	// L = 前の武器
	if (Input::IsTrigger(Command::ShoulderL))	// TABが押された瞬間
	{
		if (currentWeapon > 0)
		{
			currentWeapon = (WeaponType)(currentWeapon - 1);
		}
		else
		{
			currentWeapon = (WeaponType)(currentWeapon + 2);
		}
	}

	//--------------------------------------------------------------------------//
	// 4）基底Update()を呼ぶ													//
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

		GameObject* Vulcan = GameAPI.AddObject(new GameObject);	// 弾の GameObject を生成
		Vulcan->Activation();	// 有効化
		Vulcan->Show();	// 描画有効化
		Vulcan->GetSprite().Initialize();	// スプライト初期化
		Vulcan->GetSprite().LoadTexture("rom:/texture/Vulcan.tga");	// 画像読み込み
		Vulcan->GetSprite().DivideAnimationCells(1, 1);	// テキスチャーが縦横何分割になっているか指定
		Vulcan->GetSprite().CreateAnimation("Vulcan", 0, 0);	// アニメーションを作成（０フレイムなので１コマ）
		Vulcan->GetSprite().SetAnimation("Vulcan");	// アニメーションを指定

		Vulcan->GetSprite().SetPolygonSize(MakeFloat2(20.0f, 20.0f));	// サイズ指定

		Vulcan->GenerateRectangleCollision(1);	// 自身に方形の当たり判定を生成（引数で渡した分だけ作成する）
		Vulcan->SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(20.0f, 20.0f));	// 方形のパラメータをセット

		Vulcan->SetPosition(MakeFloat3(x, y, 0.0f));	// 座標指定

		Spread.push_back({ Vulcan, vx, vy });	// 弾リストに追加(速度も一緒に覚える)
	}
}

//======================================================================================//
//	FireLaser: 一直線に貫通する															//
//======================================================================================//

void PlayerCharacter::FireLaser(float x, float y)
{
	float velocityX = bulletSpeed;	// まっすぐ右へ（bulletSpeed はモードで変わる）
	float velocityY = 0.0f;	// 上下には散らさない

	GameObject* Laser = GameAPI.AddObject(new GameObject);	// 弾の GameObject を生成
	Laser->Activation();	// 有効化
	Laser->Show();	// 描画有効化
	Laser->GetSprite().Initialize();	// スプライト初期化
	Laser->GetSprite().LoadTexture("rom:/texture/Laser.tga");	// 画像読み込み
	Laser->GetSprite().DivideAnimationCells(6, 1);	// テキスチャーが縦横何分割になっているか指定
	Laser->GetSprite().CreateAnimation("Laser", 0, 5);	// アニメーションを作成（０フレイムなので１コマ）
	Laser->GetSprite().SetAnimation("Laser");	// アニメーションを指定
	Laser->GetSprite().SetAnimationFrameTime("Laser", 0.48f);	//

	Laser->GetSprite().SetPolygonSize(MakeFloat2(300.0f, 150.0f));	// サイズ指定

	Laser->GenerateRectangleCollision(1);	// 自身に方形の当たり判定を生成（引数で渡した分だけ作成する）
	Laser->SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(100.0f, 40.0f));	// 方形のパラメータをセット

	Laser->SetPosition(MakeFloat3(x, y, 0.0f));	//座標指定

	Spread.push_back({ Laser,velocityX,velocityY });	// 弾リストに追加(速度も一緒に覚える)
}


void PlayerCharacter::CollisionReaction(GameObject* opponent)
{
	//TODO: 衝突時の反応をここに書く
}