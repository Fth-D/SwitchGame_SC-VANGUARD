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

PlayerCharacter* Vanguard = nullptr;	//プレイヤー機体
GameObject* objB = nullptr;	//敵機体(テスト用)

GameObject* Background_Layer1_far = nullptr;	//背景レイヤー1（遠景）
GameObject* Background_Layer2_mid = nullptr;	//背景レイヤー2（中景）
GameObject* Background_Layer3_near = nullptr;	//背景レイヤー3（近景）

//==========================================================================================//
// FindNthNearestEnemy: 指定した順位で近い敵を探す											//
//------------------------------------------------------------------------------------------//
// order=0 なら最も近い敵、order=1 なら2番目に近い敵……という意味。						//
// 該当する敵がいなければ nullptr を返す(例えば敵が2体しかいないのにorder=3を指定した場合)。//
//==========================================================================================//
EnemyCharacter* FindNthNearestEnemy(float x, float y, int order)
{
	// 全ての生存中の敵と、その距離を集める
	std::vector<EnemyCharacter*> aliveEnemies;
	std::vector<float> distances;

	for (int i = 0; i < (int)Enemies.size(); i++)
	{
		EnemyCharacter* enemy = Enemies[i];
		if (enemy->GetObjectActiveState() == false) continue;

		Float3 pos = enemy->GetPosition();
		float dx = pos.x - x;
		float dy = pos.y - y;
		float dist = sqrtf(dx * dx + dy * dy);

		aliveEnemies.push_back(enemy);
		distances.push_back(dist);
	}

	if (aliveEnemies.empty()) return nullptr;	// 敵が一体もいない

	// 距離が近い順に並べ替える(単純な選択ソート、敵の数が少ないので十分)
	for (int i = 0; i < (int)distances.size() - 1; i++)
	{
		int minIndex = i;
		for (int j = i + 1; j < (int)distances.size(); j++)
		{
			if (distances[j] < distances[minIndex]) minIndex = j;
		}
		if (minIndex != i)
		{
			std::swap(distances[i], distances[minIndex]);
			std::swap(aliveEnemies[i], aliveEnemies[minIndex]);
		}
	}

	// order が敵の数を超えていたら、一番近い敵にフォールバックする
	if (order >= (int)aliveEnemies.size())
	{
		order = 0;
	}

	return aliveEnemies[order];
}

void Game::InitializeGame()			// ここにゲームシーン初期化コードを書く
{
	Log::SetScreenDisplay(true);	// 開啟畫面上顯示log

	//======================================================================================//
	// GameObjectの生成																		//
	//======================================================================================//

	// --- 背景	--- //
	Background_Layer1_far = AddObject(new GameObject);
	Background_Layer2_mid = AddObject(new GameObject);

	// --- プレイヤー機体の生成 --- //
	Vanguard = (PlayerCharacter*)AddObject(new PlayerCharacter); //プレイヤー機体を生成

	Background_Layer3_near = AddObject(new GameObject);

	// --- UI「最初に」生成 --- //
	UI_Frame = AddObject(new GameObject);


	//======================================================================================//
	// GameObjectの生成パラメータ															//
	//======================================================================================//

	// --- UI_Frame ---
	UI_Frame->Activation();	//有効化
	UI_Frame->Show();	//描画有効化
	UI_Frame->GetSprite().Initialize();	//スプライト初期化
	UI_Frame->SetObjectType(UI);	//オブジェクト分類
	UI_Frame->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	UI_Frame->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	UI_Frame->GetSprite().LoadTexture("rom:/texture/UI/UI_test.tga");	//画像読み込み
	UI_Frame->GetSprite().DivideAnimationCells(1, 1);	//テキスチャーが縦横何分割になっているか指定
	UI_Frame->GetSprite().CreateAnimation("UI_frame", 0, 0);	//アニメーションを作成（０フレイムなので１コマ）
	UI_Frame->GetSprite().SetAnimation("UI_frame");	//アニメーションを指定

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

	//// --- Background_Layer3_near ---
	//Background_Layer3_near->Activation();	//有効化
	//Background_Layer3_near->Show();	//描画有効化
	//Background_Layer3_near->GetSprite().Initialize();	//スプライト初期化
	//Background_Layer3_near->SetObjectType(Other);	//オブジェクト分類
	//Background_Layer3_near->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	//Background_Layer3_near->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	//Background_Layer3_near->GetSprite().LoadTexture("rom:/texture/Background/layer_3_near_sheet.tga");	//画像読み込み
	//Background_Layer3_near->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
	//Background_Layer3_near->GetSprite().CreateAnimation("Background_Layer3_near", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）
	//Background_Layer3_near->GetSprite().SetAnimation("Background_Layer3_near");	//アニメーションを指定
	//Background_Layer3_near->GetSprite().SetAnimationFrameTime("Background_Layer3_near", 0.24f);	//アニメーション１週に何フレーム


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
}

void Game::UpdateGame(float dt)		// ここにゲームシーン更新コードを書く
{

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

		Float3 bp = Vulacn->GetPosition();	//弾の現在座標を取得
		bp.x += bullet.vulcanVelocityX * dt;	//各弾それぞれの向きで移動
		bp.y += bullet.vulcanVelocityY * dt;
		Vulacn->SetPosition(bp);	//座標を更新する
		Vulacn->UpdateCollision();	//当たり判定を更新する

		bool remove = false;	//削除フラグ

		// ----- objB に当たった？ -----
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

		// ----- 画面外に出たら回収（右方向の一定距離離れると消える） -----
		if (bp.x > 1200.0f || bp.y > 1400.0f || bp.y < -1400.0f)
		{
			remove = true;
		}
		if (remove)
		{
			DeleteObject(Vulacn);	//弾を削除
			Spread.erase(Spread.begin() + i);	//弾リストからも削除
		}
	}

	//======================================================================================//
	// Laser弾の更新										 								//
	//--------------------------------------------------------------------------------------//
	// ① ボタンを押している間はY座標をプレイヤーの砲口に追従させる。ボタンを離した瞬間、	//
	//   以後は追従をやめて直進のみになる(isFollowingPlayerで判定)。						//
	// ② 敵に当たっても消えず、貫通したまま進み続ける。重なっている間は毎フレーム			//
	//   ダメージを与え続ける(秒間ダメージをdtで割って計算する)。							//
	//======================================================================================//
	bool isAttackHeld = Input::IsPress(Command::Attack);	// 射撃ボタンがまだ押されているか

	for (int i = (int)Straight.size() - 1; i >= 0; i--)
	{
		LaserBeam& beam = Straight[i];
		GameObject* Laser = beam.obj;

		// ボタンが離されたら、以後この発は追従をやめる(一度離れたら戻らない)
		if (beam.isFollowingPlayer && isAttackHeld == false)
		{
			beam.isFollowingPlayer = false;
		}

		Float3 beamPosition = Laser->GetPosition();

		if (beam.isFollowingPlayer)
		{
			// ボタンを押している間：Y座標をプレイヤーの砲口に同期させる
			Float3 playerPos = Vanguard->GetPosition();
			beamPosition.y = playerPos.y + Vanguard->GetLaserMuzzleOffsetY();
			beamPosition.x += beam.laserVelocityX * dt;
		}
		else
		{
			// ボタンが離された後：通常の直進のみ(追従しない)
			beamPosition.x += beam.laserVelocityX * dt;
			beamPosition.y += beam.laserVelocityY * dt;
		}

		Laser->SetPosition(beamPosition);
		Laser->UpdateCollision();

		bool remove = false;

		// ----- objB に当たった？(テスト用、そのまま残す) -----
		if (objB != NULL)
		{
			const Rectangle* bulletRect = Laser->GetRectangleCollision(0);
			const Circle* enemyCircle = objB->GetCircleCollision(0);
			if (bulletRect && enemyCircle && CheckRectangleCircleCollision(*bulletRect, *enemyCircle))
			{
				SetControllerLeftVibration(0.5f, 0.1f);
				DeleteObject(objB);
				objB = NULL;
			}
		}

		// ----- Enemies に当たった？(貫通:消えずに毎フレームダメージを与え続ける) -----
		for (int j = (int)Enemies.size() - 1; j >= 0; j--)
		{
			EnemyCharacter* enemy = Enemies[j];
			if (enemy->GetObjectActiveState() == false)
			{
				Enemies.erase(Enemies.begin() + j);	// ★ 死咗即刻剔走
				continue;
			}

			const Rectangle* bulletRect = Laser->GetRectangleCollision(0);
			const Rectangle* enemyRect = enemy->GetRectangleCollision(0);
			if (bulletRect && enemyRect && CheckRectangleCollision(*bulletRect, *enemyRect))
			{
				float laserDamagePerSecond = 60.0f;
				int damageThisFrame = (int)(laserDamagePerSecond * dt);
				if (damageThisFrame < 1) damageThisFrame = 1;

				enemy->TakeDamage(damageThisFrame);

				// ★ 傷害之後即刻檢查,死咗就即刻剔走
				if (enemy->GetObjectActiveState() == false)
				{
					Enemies.erase(Enemies.begin() + j);
				}
			}
		}

		// ----- 画面外に出たら回収 -----
		if (beamPosition.x > 1200.0f || beamPosition.y > 1400.0f || beamPosition.y < -1400.0f)
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
	// 発射から時間経過(homingTimer)によって3段階に分かれる：								//
	//   ① 彈出(だんしゅつ)段階：機体から飛び出す方向(モードによって上下が変わる)			//
	//   ② 推進段階：まっすぐ前(右)へ加速する												//
	//   ③ 追尾段階：最も近い敵に向かって少しずつ方向転換する								//
	//======================================================================================//
	for (int i = (int)Chase.size() - 1; i >= 0; i--)
	{
		HomingMissile& missile = Chase[i];
		GameObject* Homing = missile.obj;

		missile.homingTimer += dt;	// 発射からの経過時間を加算する

		Float3 myPos = Homing->GetPosition();


		if (missile.homingTimer < 0.15f)
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
				missile.homingVelocityY = 100.0f;	// 下方向へ飛び出す
			}
		}
		else if (missile.homingTimer < 0.35f)
		{
			//--------------------------------------------------------------------------//
			// ② 推進段階(0.15～0.4秒)：飛び出しが終わり、まっすぐ右へ加速する			//
			// この間はまだ敵を追わない(推進エンジンが安定するまでの演出)				//
			//--------------------------------------------------------------------------//

			missile.homingVelocityX = 370.0f;	// 右方向へ加速
			missile.homingVelocityY = 0.0f;	// 上下の動きは止める
		}
		else
		{
			//--------------------------------------------------------------------------//
			// ③ 追尾段階(0.4秒以降)：最も近い敵を探して、方向を少しずつそちらへ寄せる	//
			// 一気に向きを変えず、徐々に旋回することで「ミサイルらしい動き」になる		//
			//--------------------------------------------------------------------------//
			
			EnemyCharacter* target = FindNthNearestEnemy(myPos.x, myPos.y, missile.targetOrder);	// 最も近い敵を探す

			if (target != nullptr)	// 敵が見つかった場合のみ方向を調整する
			{
				Log::Info("Missile order=%d -> target at (%.0f, %.0f)", missile.targetOrder, target->GetPosition().x, target->GetPosition().y);

				Float3 targetPos = target->GetPosition();
				float dx = targetPos.x - myPos.x;	// 敵までのX方向の距離
				float dy = targetPos.y - myPos.y;	// 敵までのY方向の距離
				float angle = atan2f(dy, dx);		// 敵の方向の角度を求める

				//float turnRate = 3260.0f;	// 旋回の強さ(大きいほど急旋回になる)
				missile.homingVelocityX += cosf(angle) * missile.TurnRate;	// 敵の方向へ少し速度を足す
				missile.homingVelocityY += sinf(angle) * missile.TurnRate;

				// 速度ベクトルの長さを一定(homingSpeed)に揃え直す
				// これをしないと、旋回を繰り返すたびに速度がどんどん増えてしまう
				float speed = sqrtf(missile.homingVelocityX * missile.homingVelocityX +
					missile.homingVelocityY * missile.homingVelocityY);
				if (speed > 0.0f)
				{
					float homingSpeed = 1800.0f;	// ミサイルの飛行速度(固定)
					missile.homingVelocityX = missile.homingVelocityX / speed * homingSpeed;
					missile.homingVelocityY = missile.homingVelocityY / speed * homingSpeed;
				}
			}
			// target が nullptr の場合(敵が画面上にいない)：
			// 何もしないので、直前の速度のまま直進を続ける
		}

		// --- 求めた速度で実際に座標を移動させる ---
		Float3 bp = myPos;
		bp.x += missile.homingVelocityX * dt;
		bp.y += missile.homingVelocityY * dt;
		Homing->SetPosition(bp);
		Homing->UpdateCollision();

		bool remove = false;

		// ----- Enemies に当たった？ -----
		for (int j = (int)Enemies.size() - 1; j >= 0; j--)
		{
			EnemyCharacter* enemy = Enemies[j];
			if (enemy->GetObjectActiveState() == false) continue;

			const Rectangle* bulletRect = Homing->GetRectangleCollision(0);
			const Rectangle* enemyRect = enemy->GetRectangleCollision(0);
			if (bulletRect && enemyRect && CheckRectangleCollision(*bulletRect, *enemyRect))
			{
				enemy->TakeDamage(3);
				remove = true;
				break;
			}
		}

		// ----- 画面外に出たら回収 -----
		if (bp.x > 1200.0f || bp.y > 1400.0f || bp.y < -1400.0f)
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
		newEnemy->Setup(1000.0f, spawnY, 120.0f, 120.0f, 60.0f, 30.0f, 3, 1, -650.0f, 16, "rom:/texture/Character/Enemy/ENEMY_ReaperDrone.tga");

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

		// ---- Vulcan/Laser 弾に当たった？ ---- 
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
	Log::DrawScreen();
}