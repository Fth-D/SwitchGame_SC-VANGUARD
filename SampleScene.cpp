#include "Game.h"//←上の方追記
#include "Renderer.h"
#include "Input.h"
#include"Controller.h"
#include "Collision.h"

GameObject* objA = nullptr;
GameObject* objB = nullptr;

float ax;	//画面利用のためX軸をfloat xに設定します
float bx;	//画面利用のためX軸をfloat xに設定します

void Game::InitializeSample()
{
	//----------------------------------------------//
	//	↓InitializeSample（初期化関数）に記述		//
	//----------------------------------------------//

	ax = 0;
	bx = 400;

	objA = AddObject(new GameObject);
	objB = AddObject(new GameObject);

	// --- objA ---
	objA->Activation();	//有効化
	objA->Show();	//描画有効化
	objA->GetSprite().Initialize();	//スプライト初期化
	objA->GetSprite().LoadTexture("rom:/texture/dora01.tga");	//画像読み込み
	objA->GetSprite().DivideAnimationCells(3, 4);	//テキスチャーが縦横何分割になっているか指定
	objA->GetSprite().CreateAnimation("objAnim", 0, 2);	//アニメーションを作成（０フレイムなので１コマ）
	objA->GetSprite().SetAnimation("objAnim");	//アニメーションを指定
	objA->GetSprite().SetAnimationFrameTime("objAnim", 30.0f);	//アニメーション１週に何フレームかけるか

	objA->GenerateCircleCollision(1);	//自身に円形の当たり判定を生成（引数で渡した分だけ作成する）
	objA->SetCircleParameter(0, MakeFloat2(0, 0), 30.0f);	//円形のパラメータをセット

	objA->SetPosition(MakeFloat3(ax, 0.0f, 0.0f));	//座標指定

	// --- objB ---
	objB->Activation();	//有効化
	objB->Show();	//描画有効化
	objB->GetSprite().Initialize();	//スプライト初期化
	objB->GetSprite().LoadTexture("rom:/texture/car.tga");	//画像読み込み
	objB->GetSprite().DivideAnimationCells(1, 1);	//テキスチャーが縦横何分割になっているか指定
	objB->GetSprite().CreateAnimation("objAnim", 0, 0);	//アニメーションを作成（０フレイムなので１コマ）
	objB->GetSprite().SetAnimation("objAnim");	//アニメーションを指定
	objB->GetSprite().SetAnimationFrameTime("objAnim", 30.0f);	//アニメーション１週に何フレームかけるか

	objB->GenerateCircleCollision(1);	//自身に円形の当たり判定を生成（引数で渡した分だけ作成する）

	objB->SetCircleParameter(0, MakeFloat2(0, 0), 30.0f);	//円形のパラメータをセット
	objB->SetPosition(MakeFloat3(bx, 0.0f, 0.0f));	//座標指定

}

void Game::UpdateSample(float dt)
{
	//----------------------------------------------//
	//	↓UpdateSample（初期化関数）に記述			//
	//----------------------------------------------//

	//objA->objA->Update(dt);	//不要
	if (objA != NULL && objB != NULL)
	{
		const Circle* colA = objA->GetCircleCollision(0);	//当たり判定を取得
		const Circle* colB = objB->GetCircleCollision(0);	//当たり判定を取得

		if (CheckCircleCollision(*colA, *colB))	//(重なっているチェック
		{
			//--------------------------------------//
			// オブジェクト削除	（方法一）			//
			//--------------------------------------//

			//バイブレーション
			SetControllerLeftVibration(1);	//左ジョイコンを振動
			if (objA != NULL)
			{
				DeleteObject(objA);	//オブジェクトを削除(deleteが行われる)
				objA = NULL;
			}
			else
			{
				objA = AddObject(new GameObject);

				objA->SetPosition(MakeFloat3(ax, 0.0f, 0.0f));	//座標指定
				objA->GetSprite().SetPolygonSize(MakeFloat2(200.0f, 200.0f));	//サイズ指定
				objA->Activation();	//有効化
				objA->Show();	//描画有効化
				objA->GetSprite().Initialize();	//スプライト初期化
				objA->GetSprite().LoadTexture("rom:/texture/dora01.tga");	//画像読み込み
				objA->GetSprite().DivideAnimationCells(3, 4);	//テキスチャーが縦横何分割になっているか指定
				objA->GetSprite().CreateAnimation("objAnim", 0, 2);	//アニメーションを作成（０フレイムなので１コマ）
				objA->GetSprite().SetAnimation("objAnim");	//アニメーションを指定
				objA->GetSprite().SetAnimationFrameTime("objAnim", 30.0f);	//アニメーション１週に何フレームかけるか

				objA->GenerateCircleCollision(1);	//自身に円形の当たり判定を生成（引数で渡した分だけ作成する）
				objA->SetCircleParameter(0, MakeFloat2(0, 0), 100.0f);	//円形のパラメータをセット

			}
		}
	}

	//キー入力の例
	if (Input::IsTrigger(Command::Confirm))
	{
		if (objA != NULL)
		{
			DeleteObject(objA);	//オブジェクトを削除(deleteが行われる)
			objA = NULL;
		}
		else
		{
			objA = AddObject(new GameObject);

			objA->SetPosition(MakeFloat3(ax, 0.0f, 0.0f));	//座標指定
			objA->GetSprite().SetPolygonSize(MakeFloat2(200.0f, 200.0f));	//サイズ指定
			objA->Activation();	//有効化
			objA->Show();	//描画有効化
			objA->GetSprite().Initialize();	//スプライト初期化
			objA->GetSprite().LoadTexture("rom:/texture/dora01.tga");	//画像読み込み
			objA->GetSprite().DivideAnimationCells(3, 4);	//テキスチャーが縦横何分割になっているか指定
			objA->GetSprite().CreateAnimation("objAnim", 3, 5);	//アニメーションを作成（０フレイムなので１コマ）
			objA->GetSprite().SetAnimation("objAnim");	//アニメーションを指定
			objA->GetSprite().SetAnimationFrameTime("objAnim", 30.0f);	//アニメーション１週に何フレームかけるか

			objA->GenerateCircleCollision(1);	//自身に円形の当たり判定を生成（引数で渡した分だけ作成する）
			objA->SetCircleParameter(0, MakeFloat2(0, 0), 100.0f);	//円形のパラメータをセット

		}
	}

	//----------------------------------------------------------//
	//	軸入力の例												//
	//----------------------------------------------------------//
	ax += Input::GetAxis(Command::MoveX);

	//画面タッチ	
	if (GetControllerTouchScreen())	//タッチされているている判断
	{
		Float2 pos = GetControllerTouchScreenPosition();	//タッチされている座標を取得
		objA->SetPosition(MakeFloat3(pos.x, pos.y, 0.0f));	//座標指定

		//バイブレーション
		SetControllerLeftVibration(1);	//左ジョイコンを振動
	}

	//ボタンを押すして角度をリセット
	if (Input::IsPress(Command::Confirm))
	{
		SetControllerLeftAngleBase();	//左ジョイコンの角度を取得
	}

	Float3 angle = GetControllerLeftAngle();	//左ジョイコンの角度を取得
	ax += angle.z;
	objA->SetPosition(MakeFloat3(ax, 0.0f, 0.0f));	//座標指定

	//------------------------------//
	//	加速度取得の例				//
	//------------------------------//
	Float3 acceleration = GetControllerLeftAcceleration();	//左ジョイコンの加速度を取得
	if (acceleration.y > 3 || acceleration.y < -3)	//Y軸方向の加速度が一定以上なら
	{
		ax++;
	}
	objA->SetPosition(MakeFloat3(ax, 0.0f, 0.0f));	//座標指定

}

void Game::DrawSample()
{
	//----------------------------------------------//
	//	↓DrawSample（初期化関数）に記述			//
	//----------------------------------------------//

}	