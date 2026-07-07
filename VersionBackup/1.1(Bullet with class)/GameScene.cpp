#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "Controller.h"
#include "Collision.h"
#include "PlayerCharacter.h"
#include <vector>

//======================================================================================//
// 弾リストの実体																		//
//--------------------------------------------------------------------------------------//
// Player.hではextern宣言しているので、ここで実体を作る必要がある						//
// Player が spawn し、このSceneが更新．削除する										//
//======================================================================================//

GameObject* UI_Frame = nullptr;	//UIフレーム

PlayerCharacter* Vanguard = nullptr;	//プレイヤー機体
GameObject* objB = nullptr;	//敵機体(テスト用)

std::vector<Projectile*> Spread;

GameObject* Background_Layer1_far = nullptr;	//背景レイヤー1（遠景）
GameObject* Background_Layer2_mid = nullptr;	//背景レイヤー2（中景）
GameObject* Background_Layer3_near = nullptr;	//背景レイヤー3（近景）


void Game::InitializeGame()			// ここにゲームシーン初期化コードを書く
{
	//======================================================================================//
	// GameObjectの生成																		//
	//======================================================================================//
	
	// --- UI「最初に」生成 --- //
	UI_Frame = AddObject(new GameObject);

	// --- プレイヤー機体の生成 --- //
	Vanguard = (PlayerCharacter*)AddObject(new PlayerCharacter); //プレイヤー機体を生成

	// --- 背景	--- //
	Background_Layer1_far = AddObject(new GameObject);
	Background_Layer2_mid = AddObject(new GameObject);
	Background_Layer3_near = AddObject(new GameObject);

	objB = AddObject(new GameObject);	//敵機体(テスト用)


	//======================================================================================//
	// GameObjectの生成パラメータ															//
	//======================================================================================//

	// --- UI_Frame ---
	UI_Frame->Activation();	//有効化
	UI_Frame->Show();	//描画有効化
	UI_Frame->GetSprite().Initialize();	//スプライト初期化
	UI_Frame->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	UI_Frame->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	UI_Frame->GetSprite().LoadTexture("rom:/texture/UI_test.tga");	//画像読み込み
	UI_Frame->GetSprite().DivideAnimationCells(1, 1);	//テキスチャーが縦横何分割になっているか指定
	UI_Frame->GetSprite().CreateAnimation("UI_frame", 0, 0);	//アニメーションを作成（０フレイムなので１コマ）
	UI_Frame->GetSprite().SetAnimation("UI_frame");	//アニメーションを指定

	// --- Vanguard ---
	Vanguard->Setup(-540.0f, 0.0f);	//初期化（画像、当たり判定、位置）

	// --- Background_Layer1_far ---
	Background_Layer1_far->Activation();	//有効化
	Background_Layer1_far->Show();	//描画有効化
	Background_Layer1_far->GetSprite().Initialize();	//スプライト初期化
	Background_Layer1_far->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	Background_Layer1_far->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	Background_Layer1_far->GetSprite().LoadTexture("rom:/texture/layer1_far_sheet.tga");	//画像読み込み
	Background_Layer1_far->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
	Background_Layer1_far->GetSprite().CreateAnimation("Background_Layer1_far", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）
	Background_Layer1_far->GetSprite().SetAnimation("Background_Layer1_far");	//アニメーションを指定
	Background_Layer1_far->GetSprite().SetAnimationFrameTime("Background_Layer1_far", 0.24f);	//アニメーション１週に何フレーム

	// --- Background_Layer2_mid ---
	Background_Layer2_mid->Activation();	//有効化
	Background_Layer2_mid->Show();	//描画有効化
	Background_Layer2_mid->GetSprite().Initialize();	//スプライト初期化
	Background_Layer2_mid->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	Background_Layer2_mid->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	Background_Layer2_mid->GetSprite().LoadTexture("rom:/texture/layer2_mid_sheet.tga");	//画像読み込み
	Background_Layer2_mid->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
	Background_Layer2_mid->GetSprite().CreateAnimation("Background_Layer2_mid", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）
	Background_Layer2_mid->GetSprite().SetAnimation("Background_Layer2_mid");	//アニメーションを指定
	Background_Layer2_mid->GetSprite().SetAnimationFrameTime("Background_Layer2_mid", 0.24f);	//アニメーション１週に何フレーム

	// --- Background_Layer3_near ---
	Background_Layer3_near->Activation();	//有効化
	Background_Layer3_near->Show();	//描画有効化
	Background_Layer3_near->GetSprite().Initialize();	//スプライト初期化
	Background_Layer3_near->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
	Background_Layer3_near->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
	Background_Layer3_near->GetSprite().LoadTexture("rom:/texture/layer_3_near_sheet.tga");	//画像読み込み
	Background_Layer3_near->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
	Background_Layer3_near->GetSprite().CreateAnimation("Background_Layer3_near", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）
	Background_Layer3_near->GetSprite().SetAnimation("Background_Layer3_near");	//アニメーションを指定
	Background_Layer3_near->GetSprite().SetAnimationFrameTime("Background_Layer3_near", 0.24f);	//アニメーション１週に何フレーム


	//----------------------------------------------//
	// 敵機体の生成（テスト用）						//
	//----------------------------------------------//
	objB->Activation();	//有効化
	objB->Show();	//描画有効化
	objB->GetSprite().Initialize();	//スプライト初期化
	objB->GetSprite().LoadTexture("rom:/texture/car.tga");	//画像
	objB->GetSprite().DivideAnimationCells(1, 1);	//テキスチャーが縦横何分割になっているか指定
	objB->GetSprite().CreateAnimation("objAnim", 0, 0);	//アニメーションを作成（０フレイムなので１コマ）
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
		Projectile*shot = Spread[i];
		//GameObject* Vulacn = bullet.obj;
		
		// 自分で消えた（画面外）なら、リストから外すだけ
		if (shot->GetObjectActiveState() == false)
		{
			Spread.erase(Spread.begin() + i);
			continue;
		}

		//Float3 bp = Vulacn->GetPosition();	//弾の現在座標を取得
		//bp.x += bullet.vx * dt;	//各弾それぞれの向きで移動
		//bp.y += bullet.vy * dt;
		//Vulacn->SetPosition(bp);	//座標を更新する
		//Vulacn->UpdateCollision();	//当たり判定を更新する

		//bool remove = false;	//削除フラグ

		// ----- objB に当たった？ -----
		if (objB != NULL)	//弾が敵機体に当たったら
		{
			const Rectangle* bulletRect = shot->GetRectangleCollision(0);	//弾の当たり判定を取得（方形）
			const Circle* enemyCircle = objB->GetCircleCollision(0);	//敵（円形）
			if (bulletRect && enemyCircle && CheckRectangleCircleCollision(*bulletRect, *enemyCircle))	//重なりチェック
			{
				SetControllerLeftVibration(0.5f, 0.1f);	//バイブレーション
				DeleteObject(objB);	//敵機体を削除
				objB = NULL;	//ポインタをNULLにする
				//remove = true;	//弾も削除する
				shot->Hide();	//弾を非表示にする
				shot->Deactivation();	//弾を無効化する
				Spread.erase(Spread.begin() + i);	//弾リストからも削除
			}
		}

		//// ----- 画面外に出たら回収（右方向の一定距離離れると消える） -----
		//if (bp.x > 1200.0f || bp.y > 1400.0f || bp.y < -1400.0f)
		//{
		//	remove = true;
		//}
		//if (remove)
		//{
		//	DeleteObject(Vulacn);	//弾を削除
		//	Spread.erase(Spread.begin() + i);	//弾リストからも削除
		//}
	}
}

void Game::DrawGame()				// ここにゲームシーン描画コードを書く
{

}