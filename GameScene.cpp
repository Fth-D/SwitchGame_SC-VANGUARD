#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "Controller.h"
#include "Collision.h"
#include "GameScene.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "Log.h"
#include <cstdlib>
#include <vector>	// rand() を使うため
#include <cmath>	// Homingのどこで使う

////============================================================================================//
//// 位置測定用																					//
////============================================================================================//
//#if !PLATFORM_NX
//#include <GLFW/glfw3.h>
//#include <cstdio>
//
//// GraphicsHelper.Win64 側で定義済み（controller.Win64.cpp と同じ流用）
//GLFWwindow* PlatformGetWindow();
//
//// マウス座標をウィンドウのタイトルバーに表示する（x64ビルドのデバッグ用）
//static void ShowCursorPositionInWindowTitle()
//{
//	GLFWwindow* window = PlatformGetWindow();
//	if (window == nullptr) return;
//
//	double cursorX = 0.0;
//	double cursorY = 0.0;
//	glfwGetCursorPos(window, &cursorX, &cursorY);	// ウィンドウ左上が原点(0,0)のピクセル座標
//	int worldX = (int)cursorX - 960;	// 1920/2
//	int worldY = (int)cursorY - 540;	// 1080/2
//	char windowTitle[128];
//	snprintf(windowTitle, sizeof(windowTitle),
//		"Stellar Chronicle: VANGUARD   X= %d, Y= %d", (int)cursorX, (int)cursorY);
//	glfwSetWindowTitle(window, windowTitle);
//}
//#endif


//======================================================================================//
// 弾リストの実体																		//
//--------------------------------------------------------------------------------------//
// Player.hでは extern 宣言しているので、ここで実体を作る必要がある						//
// Player が spawn し、このSceneが更新．削除する										//
//======================================================================================//
std::vector<VulcanBullet> Spread;
std::vector<LaserBeam> Straight;
std::vector<HomingMissile> Chase;

std::vector<EnemyCharacter*>Enemies;	// 敵リスト
float enemySpawnTimer = 2.0f;	// 次の敵出現までの時間（最初の１体は２秒後）

GameState gameState = PLAYING;

GameObject* UI_Frame = nullptr;	//UIフレーム

GameObject* HP_Bar = nullptr;	// HP Bar

PlayerCharacter* Vanguard = nullptr;	//プレイヤー機体
GameObject* objB = nullptr;	//敵機体(テスト用)

GameObject* Background_Layer1_far = nullptr;	//背景レイヤー1（遠景）
GameObject* Background_Layer2_mid = nullptr;	//背景レイヤー2（中景）
GameObject* Background_Layer3_near = nullptr;	//背景レイヤー3（近景）

float HP_Leungh = 563.0f;


void Game::InitializeGame()			// ここにゲームシーン初期化コードを書く
{
	//======================================================================================//
	// GameObjectの生成																		//
	//======================================================================================//

	// --- 背景	--- //
	Background_Layer1_far = AddObject(new GameObject);
	Background_Layer2_mid = AddObject(new GameObject);

	// --- プレイヤー機体の生成 --- //
	Vanguard = (PlayerCharacter*)AddObject(new PlayerCharacter); //プレイヤー機体を生成

	Background_Layer3_near = AddObject(new GameObject);

	//======================================================================================//
	// GameObjectの生成パラメータ															//
	//======================================================================================//

	// --- UI_Frame ---
	UI_Frame = new GameObject;
	UI_Frame->Activation();	//有効化
	UI_Frame->Show();	//描画有効化
	UI_Frame->GetSprite().Initialize();	//スプライト初期化
	UI_Frame->SetObjectType(UI);	//オブジェクト分類
	UI_Frame->SetFixedScreenPositionState(true);	// 画面に固定するかどうか（UIなどはtrueにするといいよ）
	UI_Frame->SetPosition(MakeFloat3(960.0f, 540.0f, 0.0f));	//座標指定	(画面に固定する場合、画面左上が0，0なので)
	UI_Frame->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	// サイズ指定
	UI_Frame->GetSprite().LoadTexture("rom:/texture/UI/UI_Frame_Pixel.tga");	// 画像読み込み
	UI_Frame->GetSprite().DivideAnimationCells(1, 1);	// テキスチャーが縦横何分割になっているか指定
	UI_Frame->GetSprite().CreateAnimation("UI_frame", 0, 0);	// アニメーションを作成（０フレイムなので１コマ）
	UI_Frame->GetSprite().SetAnimation("UI_frame");	// アニメーションを指定

	HP_Bar = new GameObject;
	HP_Bar->Activation();	//有効化
	HP_Bar->Show();	//描画有効化
	HP_Bar->GetSprite().Initialize();	//スプライト初期化
	HP_Bar->SetObjectType(UI);	//オブジェクト分類
	HP_Bar->SetFixedScreenPositionState(true);	// 画面に固定するかどうか（UIなどはtrueにするといいよ）
	HP_Bar->SetPosition(MakeFloat3(371.0f, 152.5f, 0.0f));	// 座標指定
	HP_Bar->GetSprite().SetPolygonSize(MakeFloat2(HP_Leungh, 100.0f));	//サイズ指定
	HP_Bar->GetSprite().LoadTexture("rom:/texture/UI/HP_Bar.tga");	//画像読み込み
	HP_Bar->GetSprite().DivideAnimationCells(1, 1);	//テキスチャーが縦横何分割になっているか指定
	HP_Bar->GetSprite().CreateAnimation("HP_Bar", 0, 0);	//アニメーションを作成（０フレイムなので１コマ）
	HP_Bar->GetSprite().SetAnimation("HP_Bar");	//アニメーションを指定

	// --- Vanguard ---
	Vanguard->Setup(-430.0f, 0.0f);	//初期化（画像、当たり判定、位置）

	// --- Background_Layer1_far ---
	Background_Layer1_far->Activation();	//有効化
	Background_Layer1_far->Show();	//描画有効化
	Background_Layer1_far->GetSprite().Initialize();	//スプライト初期化
	Background_Layer1_far->SetObjectType(Other);	//オブジェクト分類
	Background_Layer1_far->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	Background_Layer1_far->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	Background_Layer1_far->GetSprite().LoadTexture("rom:/texture/Background/layer1_far_sheet.tga");	//画像読み込み
	Background_Layer1_far->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
	Background_Layer1_far->GetSprite().CreateAnimation("Background_Layer1_far", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）
	Background_Layer1_far->GetSprite().SetAnimation("Background_Layer1_far");	//アニメーションを指定
	Background_Layer1_far->GetSprite().SetAnimationFrameTime("Background_Layer1_far", 0.24f);	//アニメーション１週に何フレーム

	// --- Background_Layer2_mid ---
	Background_Layer2_mid->Activation();	//有効化
	Background_Layer2_mid->Show();	//描画有効化
	Background_Layer2_mid->GetSprite().Initialize();	//スプライト初期化
	Background_Layer2_mid->SetObjectType(Other);	//オブジェクト分類
	Background_Layer2_mid->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	Background_Layer2_mid->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	Background_Layer2_mid->GetSprite().LoadTexture("rom:/texture/Background/layer2_mid_sheet.tga");	//画像読み込み
	Background_Layer2_mid->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
	Background_Layer2_mid->GetSprite().CreateAnimation("Background_Layer2_mid", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）
	Background_Layer2_mid->GetSprite().SetAnimation("Background_Layer2_mid");	//アニメーションを指定
	Background_Layer2_mid->GetSprite().SetAnimationFrameTime("Background_Layer2_mid", 0.24f);	//アニメーション１週に何フレーム

	// --- Background_Layer3_near ---
	Background_Layer3_near->Activation();	//有効化
	Background_Layer3_near->Show();	//描画有効化
	Background_Layer3_near->GetSprite().Initialize();	//スプライト初期化
	Background_Layer3_near->SetObjectType(Other);	//オブジェクト分類
	Background_Layer3_near->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	Background_Layer3_near->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	Background_Layer3_near->GetSprite().LoadTexture("rom:/texture/Background/layer_3_near_sheet.tga");	//画像読み込み
	Background_Layer3_near->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
	Background_Layer3_near->GetSprite().CreateAnimation("Background_Layer3_near", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）
	Background_Layer3_near->GetSprite().SetAnimation("Background_Layer3_near");	//アニメーションを指定
	Background_Layer3_near->GetSprite().SetAnimationFrameTime("Background_Layer3_near", 0.24f);	//アニメーション１週に何フレーム

	/*
	//----------------------------------------------//
	// 敵機体の生成（テスト用）						//
	//----------------------------------------------//
	objB = AddObject(new GameObject);	//敵機体を生成
	objB->Activation();	//有効化
	objB->Show();	//描画有効化
	objB->GetSprite().Initialize();	//スプライト初期化
	objB->SetObjectType(Enemy);	//オブジェクト分類
	objB->GetSprite().LoadTexture("rom:/texture/Character/Enemy/ENEMY_ReaperDrone.tga");	//画像読み込み
	objB->GetSprite().DivideAnimationCells(3, 1);	//テキスチャーが縦横何分割になっているか指定
	objB->GetSprite().CreateAnimation("objAnim", 0, 2);	//アニメーションを作成（０フレイムなので１コマ）
	objB->GetSprite().SetAnimation("objAnim");	//アニメーションを指定
	objB->GenerateCircleCollision(1);	//自身に円形の当たり判定を生成（引数で渡した分だけ作成する）
	objB->SetCircleParameter(0, MakeFloat2(0, 0), 30.0f);	//円形のパラメータをセット
	objB->SetPosition(MakeFloat3(400.0f, 0.0f, 0.0f));	//座標指定
	*/

}

void Game::UpdateGame(float dt)		// ここにゲームシーン更新コードを書く
{

	//#if !PLATFORM_NX
	//	ShowCursorPositionInWindowTitle();
	//#endif

		//----------------------------------------------------------------------------------//
		//	プレイヤーの更新																//
		//----------------------------------------------------------------------------------//
		//	Vanguard は AddObject でエンジンに登録済み。									//
		//	→ エンジンが毎フレーム自動で Update() を呼んでくれる。							//
		//	  （移動・変形・射撃は PlayerCharacter::Update に書いてあり、それが自動で動く）	//
		//----------------------------------------------------------------------------------//

		//==============================================//
		// Vulcan弾の更新								//
		//==============================================//
	for (int i = (int)Spread.size() - 1; i >= 0; i--)
	{
		VulcanBullet& bullet = Spread[i];
		GameObject* Vulacn = bullet.obj;

		Float3 bulletPosition = Vulacn->GetPosition();	//弾の現在座標を取得
		bulletPosition.x += bullet.vulcanVelocityX * dt;	//各弾それぞれの向きで移動
		bulletPosition.y += bullet.vulcanVelocityY * dt;
		Vulacn->SetPosition(bulletPosition);	//座標を更新する
		Vulacn->UpdateCollision();	//当たり判定を更新する

		bool remove = false;	//削除フラグ

		/*
		// ----- objB に当たった？ ----- （弾のテスト用オブジェクト）
		if (objB != NULL)	//弾が敵機体に当たったら
		{
			const Rectangle* bulletRect = Vulacn->GetRectangleCollision(0);	//弾の当たり判定を取得（方形）
			const Circle* enemyCircle = objB->GetCircleCollision(0);	//敵（円形）
			if (bulletRect && enemyCircle && CheckRectangleCircleCollision(*bulletRect, *enemyCircle))	//重なりチェック
			{
				SetControllerLeftVibration(0.5f, 0.1f);	//バイブレーション
				DeleteObject(objB);	//敵機体を削除
				objB = NULL;	//ポインタをNULLにする
				remove = true;	//弾も削除する
			}
		}
		*/

		// ----- 画面外に出たら回収（右方向の一定距離離れると消える） -----
		if (bulletPosition.x > 1200.0f || bulletPosition.y > 1400.0f || bulletPosition.y < -1400.0f)
		{
			remove = true;
		}
		if (remove)
		{
			DeleteObject(Vulacn);	//弾を削除
			Spread.erase(Spread.begin() + i);	//弾リストからも削除
		}
	}

	//==============================================//
	// Laser弾の更新								//
	//==============================================//
	bool AttackHeld = Input::IsPress(Command::Attack);	//射撃ボタンが押されている

	for (int i = (int)Straight.size() - 1; i >= 0; i--)
	{
		LaserBeam& beam = Straight[i];
		GameObject* Laser = beam.obj;

		// ボタンが押されていない時にはプレイヤーのY位置に追従しない
		if (beam.FollowPlayer_posY && AttackHeld == false)
		{
			beam.FollowPlayer_posY = false;
		}

		Float3 bulletPosition = Laser->GetPosition();	// 弾の現在座標を取得

		if (beam.FollowPlayer_posY)
		{
			Float3 playerPos = Vanguard->GetPosition();
			bulletPosition.y = playerPos.y + Vanguard->GetLaserMuzzleOffsetY();
			bulletPosition.x += beam.laserVelocityX * dt;
		}
		else
		{
			bulletPosition.x += beam.laserVelocityX * dt;	// 速度分だけX座標を進める
			bulletPosition.y += beam.laserVelocityY * dt;	// 速度分だけY座標を進める
		}

		Laser->SetPosition(bulletPosition);				// 計算した新しい座標を実際に反映する
		Laser->UpdateCollision();						// 当たり判定の位置も新しい座標に合わせる

		bool remove = false;

		/*
		// ----- objB に当たった？ ----- （弾のテスト用オブジェクト）
		if (objB != NULL)
		{
			const Rectangle* bulletRect = Laser->GetRectangleCollision(0);
			const Circle* enemyCircle = objB->GetCircleCollision(0);
			if (bulletRect && enemyCircle && CheckRectangleCircleCollision(*bulletRect, *enemyCircle))
			{
				SetControllerLeftVibration(0.5f, 0.1f);
				DeleteObject(objB);
				objB = NULL;
				remove = true;
			}
		}
		*/

		// ----- 画面外に出たら回収 -----
		if (bulletPosition.x > 1200.0f || bulletPosition.y > 1400.0f || bulletPosition.y < -1400.0f)
		{
			remove = true;
		}
		if (remove)
		{
			DeleteObject(Laser);
			Straight.erase(Straight.begin() + i);
		}
	}

	//======================================================================================//
	// Homing弾の更新										 								//
	//--------------------------------------------------------------------------------------//
	// 発射から時間経過(phaseTimer)によって3段階に分かれる：								//
	//   ① 彈出(だんしゅつ)段階：機体から飛び出す方向(モードによって上下が変わる)			//
	//   ② 推進段階：まっすぐ前(右)へ加速する												//
	//   ③ 追尾段階：最も近い敵に向かって少しずつ方向転換する								//
	//======================================================================================//
	for (int i = (int)Chase.size() - 1; i >= 0; i--)
	{
		HomingMissile& missile = Chase[i];
		GameObject* Homing = missile.obj;

		missile.phaseTimer += dt;	// 発射からの経過時間を加算する

		Float3 myPos = Homing->GetPosition();

		if (missile.phaseTimer < 0.15f)
		{
			//------------------------------------------------------------------------------//
			// ① 彈出段階(0～0.15秒)：まだ敵を追わず、機体から飛び出す方向のみ決める		//
			// モードによって上下が逆になる(ロボット＝上向き、ファイター＝下向き)			//
			// ※ ロボット/ファイターどちらが上/下に飛ぶかは実機で見た目を確認して決めた	//
			//------------------------------------------------------------------------------//
			if (Vanguard->GetMode() == PlayerCharacter::ROBOT)
			{
				missile.homingVelocityX = -80.0f;	// わずかに左へ流れる(演出上の動き)
				missile.homingVelocityY = -500.0f;	// 上方向へ飛び出す
			}
			else if (Vanguard->GetMode() == PlayerCharacter::FIGHTER)
			{
				missile.homingVelocityX = -80.0f;
				missile.homingVelocityY = 500.0f;	// 下方向へ飛び出す
			}
		}
		else if (missile.phaseTimer < 0.4f)
		{
			//--------------------------------------------------------------------------//
			// ② 推進段階(0.15～0.4秒)：飛び出しが終わり、まっすぐ右へ加速する			//
			// この間はまだ敵を追わない(推進エンジンが安定するまでの演出)				//
			//--------------------------------------------------------------------------//

			missile.homingVelocityX = 200.0f;	// 右方向へ加速
			missile.homingVelocityY = 0.0f;	// 上下の動きは止める
		}
		else
		{
			//--------------------------------------------------------------------------//
			// ③ 追尾段階(0.4秒以降)：最も近い敵を探して、方向を少しずつそちらへ寄せる	//
			// 一気に向きを変えず、徐々に旋回することで「ミサイルらしい動き」になる		//
			//--------------------------------------------------------------------------//

			GameObject* target = FindNearestObject(myPos.x, myPos.y, Enemy);	// 最も近い敵を探す

			if (target != nullptr)	// 敵が見つかった場合のみ方向を調整する
			{
				Float3 targetPos = target->GetPosition();
				float dx = targetPos.x - myPos.x;	// 敵までのX方向の距離
				float dy = targetPos.y - myPos.y;	// 敵までのY方向の距離
				float angle = atan2f(dy, dx);		// 敵の方向の角度を求める

				float turnRate = 1480.0f;	// 旋回の強さ(大きいほど急旋回になる)
				missile.homingVelocityX += cosf(angle) * turnRate;	// 敵の方向へ少し速度を足す
				missile.homingVelocityY += sinf(angle) * turnRate;

				// 速度ベクトルの長さを一定(homingSpeed)に揃え直す
				// これをしないと、旋回を繰り返すたびに速度がどんどん増えてしまう
				float speed = sqrtf(missile.homingVelocityX * missile.homingVelocityX +
					missile.homingVelocityY * missile.homingVelocityY);
				if (speed > 0.0f)
				{
					float homingSpeed = 4200.0f;	// ミサイルの飛行速度(固定)
					missile.homingVelocityX = missile.homingVelocityX / speed * homingSpeed;
					missile.homingVelocityY = missile.homingVelocityY / speed * homingSpeed;
				}
			}
			// target が nullptr の場合(敵が画面上にいない)：
			// 何もしないので、直前の速度のまま直進を続ける
		}

		// --- 求めた速度で実際に座標を移動させる ---
		Float3 bulletPosition = myPos;
		bulletPosition.x += missile.homingVelocityX * dt;
		bulletPosition.y += missile.homingVelocityY * dt;
		Homing->SetPosition(bulletPosition);
		Homing->UpdateCollision();

		bool remove = false;

		/*
		// ----- Enemies に当たった？ -----
		for (int j = (int)Enemies.size() - 1; j >= 0; j--)
		{
			EnemyCharacter* enemy = Enemies[j];
			if (enemy->GetObjectActiveState() == false) continue;

			const Rectangle* bulletRect = Homing->GetRectangleCollision(0);
			const Rectangle* enemyRect = enemy->GetRectangleCollision(0);
			if (bulletRect && enemyRect && CheckRectangleCollision(*bulletRect, *enemyRect))
			{
				enemy->TakeDamage(1);
				remove = true;
				break;
			}
		}
		*/

		// ----- 画面外に出たら回収 -----
		if (bulletPosition.x > 1200.0f || bulletPosition.y > 1400.0f || bulletPosition.y < -1400.0f)
		{
			remove = true;
		}
		if (remove)
		{
			DeleteObject(Homing);
			Chase.erase(Chase.begin() + i);
		}
	}


	if (gameState == GAME_OVER)
	{
		return;
	}

	//======================================================================================//
	// 敵の出現タイマー																		//
	//--------------------------------------------------------------------------------------//
	// タイマーが0以下になったら、1体生成してタイマーをリセットする							//
	//======================================================================================//
	enemySpawnTimer -= dt;
	if (enemySpawnTimer <= 0.0f)
	{
		float spawnY = -300.0f + (float)(rand() % 600);	//

		// 敵位置指定（X,Y）、敵サイズ指定（X,Y）、敵当たり判定（X,Y）、画像分割（X,Y）、敵初期化数値（速度・HP）,敵の画像読み込み
		EnemyCharacter* newEnemy = (EnemyCharacter*)AddObject(new EnemyCharacter);
		newEnemy->Setup(1000.0f, spawnY, 180.0f, 180.0f, 50.0f, 50.0f, 3, 1, -522.0f, 16, "rom:/texture/Character/Enemy/ENEMY_ReaperDrone.tga");

		Enemies.push_back(newEnemy);

		enemySpawnTimer = 0.5f;
	}

	//======================================================================================//
	// 敵の更新・当たり判定																	//
	//======================================================================================//
	for (int i = (int)Enemies.size() - 1; i >= 0; i--)
	{
		EnemyCharacter* enemy = Enemies[i];

		if (enemy->GetObjectActiveState() == false)
		{
			Enemies.erase(Enemies.begin() + i);
			continue;
		}

		//==================================================================================================//
		// Boolで敵の生存状態でアップデートする																//
		//--------------------------------------------------------------------------------------------------//
		// 敵が死んだかどうか一度 SYSTEM 内に状態アップデートして、死んだままの敵のオブジェクトを使い続ける	//
		// になると、Deactivation()の物体、つまり無効化されたのオブジェクトを使うことになる。				//
		// このこと自体が不具合に発生をさせるの原因になります。												//
		//==================================================================================================//
		bool enemyStillAlive = true;	//今、敵が生きている(True)

		// ---- Vulcan 弾に当たった？ ----
		for (int j = (int)Spread.size() - 1; j >= 0; j--)
		{
			VulcanBullet& bullet = Spread[j];
			const Rectangle* bulletRect = bullet.obj->GetRectangleCollision(0);
			const Rectangle* enemyRect = enemy->GetRectangleCollision(0);

			if (bulletRect && enemyRect && CheckRectangleCollision(*bulletRect, *enemyRect))
			{
				enemy->TakeDamage(1);
				DeleteObject(bullet.obj);
				Spread.erase(Spread.begin() + j);
				enemyStillAlive = enemy->GetObjectActiveState();	// TakeDamageで死んだかもしれないので、状態を確認し直す(ダメージ後、敵の生死を確認する)
				break;
			}
		}

		// ---- Laser 弾に当たった？（貫通：remove しない、毎フレーム判定し続ける） ----
		if (enemyStillAlive == true)
		{
			for (int j = (int)Straight.size() - 1; j >= 0; j--)
			{
				LaserBeam& beam = Straight[j];
				const Rectangle* bulletRect = beam.obj->GetRectangleCollision(0);
				const Rectangle* enemyRect = enemy->GetRectangleCollision(0);

				if (bulletRect && enemyRect && CheckRectangleCollision(*bulletRect, *enemyRect))
				{
					enemy->TakeDamage(1);
					enemyStillAlive = enemy->GetObjectActiveState();
					if (enemyStillAlive == false)break;
				}
			}
		}

		// ---- Homing 弾に当たった？ ----
		if (enemyStillAlive == true)
		{
			for (int j = (int)Chase.size() - 1; j >= 0; j--)
			{
				HomingMissile& missile = Chase[j];
				const Rectangle* bulletRect = missile.obj->GetRectangleCollision(0);
				const Rectangle* enemyRect = enemy->GetRectangleCollision(0);

				if (bulletRect && enemyRect && CheckRectangleCollision(*bulletRect, *enemyRect))
				{
					enemy->TakeDamage(5);
					DeleteObject(missile.obj);
					Chase.erase(Chase.begin() + j);
					enemyStillAlive = enemy->GetObjectActiveState();
					break;
				}
			}
		}

		// ----- プレイヤーに当たった？（敵がまだ生きている時だけチェックする） -----
		if (enemyStillAlive == true)
		{
			const Rectangle* playerRect = Vanguard->GetRectangleCollision(0);
			const Rectangle* enemyRect = enemy->GetRectangleCollision(0);

			if (playerRect && enemyRect && CheckRectangleCollision(*playerRect, *enemyRect))
			{
				SetControllerLeftVibration(0.5f, 0.1f);

				enemy->TakeDamage(999);
				Vanguard->TakeDamage(34);

				if (Vanguard->IsGameOver())
				{
					gameState = GAME_OVER;
				}

				enemyStillAlive = enemy->GetObjectActiveState();	// プレイヤーとの衝突後、敵の生存状態を確認し直す
				//------------------------------------------------------------------------------//		 ⇑
				// プレイヤーとの衝突後(TakeDamageが呼ばれたので)、敵の生存状態を確認し直す。	//		 ║
				// これがないと、enemy->TakeDamage(999) によって「二度死亡」の状態になり、		//=======╝
				// 既に無効化された敵のデータを使い続けてバグが発生してしまう					//
				//------------------------------------------------------------------------------//
			}
		}

		if (enemyStillAlive == false)
		{
			Enemies.erase(Enemies.begin() + i);	//すでに死んだ敵に Enemies と言うリストから外しなさい
		}
	}
}

void Game::DrawGame()				// ここにゲームシーン描画コードを書く
{
	Log::SetScreenDisplay(true);	// デバッグログを画面に表示する

	//--------------------------------------------------------------------------------------//
	// UIの描画（objectListとは別管理）														//
	//--------------------------------------------------------------------------------------//
	// UI_FrameはAddObjectを使わず、new GameObjectで直接生成している。						//
	// このDrawGame()は、Draw()内でobjectList全体(敵・弾・プレイヤー)が描画された後に		//
	// 呼ばれるため、ここで手動でDraw()すれば、常にobjectListの全オブジェクトより			//
	// 手前に表示される。									 								//
	// これにより、AddObject/DeleteObjectのスロット再利用（LIFO）による						//
	// 描画順の不安定さを回避できる。														//
	//--------------------------------------------------------------------------------------//


	if (HP_Bar != nullptr)
	{
		HP_Bar->Draw();
	}

	if (UI_Frame != nullptr)
	{
		UI_Frame->Draw();
	}


	//--------------------------------------------------------------------------------------//
	// 今後ここに追加予定のもの（同じ仕組みを使う）											//
	//--------------------------------------------------------------------------------------//
	// if (WeaponIcon != nullptr) WeaponIcon->Draw();		// 武器アイコンのアニメーション
	// DrawScoreText();									// スコア等の数字表示（Text使用時）
	// if (FadeOverlay != nullptr) FadeOverlay->Draw();	// フェード用の黒幕（一番最後に描画する）
}
