//#include "Game.h"//←上の方追記
//#include "Renderer.h"
//#include "Input.h"
//#include"Controller.h"
//#include "Collision.h"
//#include <vector>
//#include <cmath>// ←——————————————————————————————————————————————— cosf/sinf で角度を計算します
//static void SpawnBullet(float x, float y);
//static void SpawnVulcan(float x, float y, int count, float SpreadDegree);
//
//struct VulcanBullet {
//	GameObject* obj;
//	float vx, vy;        // VanguardBulletの速度向量(每秒pixel)
//};
//std::vector<VulcanBullet> Spread;
//
//std::vector<GameObject*> bullets;     // 画面の中で飛んでいるの弾
//const float BULLET_SPEED = 2400.0f;    // 子彈速度(每秒像素)
//
//
//GameObject* objA = nullptr;
//GameObject* objB = nullptr;
//GameObject* Background_Layer1_far = nullptr;
//GameObject* Background_Layer2_mid = nullptr;
//GameObject* Background_Layer3_near = nullptr;
//
//float ax, ay;	//画面利用のためX軸をfloat xに設定します
//float bx;	//画面利用のためX軸をfloat xに設定します
//
//const float FIRE_INTERVAL = 0.06f;   // 每隔 0.12 秒射一粒(數字越細 = 射越密)
//float fireCooldown = 0.0f;           // 距離下一粒仲要等幾耐
//const float PLAYER_SPEED = 700.0f;   // 每秒像素;想快就改大
//
//void Game::InitializeSample()
//{
//	//----------------------------------------------//
//	//	↓InitializeSample（初期化関数）に記述		//
//	//----------------------------------------------//
//	Input::Bind(Command::Attack, Key::Space);   // 空白鍵 = 發射
//	ax = -576;
//	ay = 0;
//
//	bx = 400;
//
//	objA = AddObject(new GameObject);
//	objB = AddObject(new GameObject);
//
//	Background_Layer1_far = AddObject(new GameObject);
//	Background_Layer2_mid = AddObject(new GameObject);
//	Background_Layer3_near = AddObject(new GameObject);
//
//	// --- objA ---
//	objA->Activation();	//有効化
//	objA->Show();	//描画有効化
//	objA->GetSprite().Initialize();	//スプライト初期化
//	objA->GetSprite().SetPolygonSize(MakeFloat2(200.0f, 200.0f));	//サイズ指定
//	objA->GetSprite().LoadTexture("rom:/texture/Vanguard_Idle_sheet.tga");	//画像読み込み
//	objA->GetSprite().DivideAnimationCells(6, 6);	//テキスチャーが縦横何分割になっているか指定
//	objA->GetSprite().CreateAnimation("vanguard", 0, 35);	//アニメーションを作成（０フレイムなので１コマ）
//	objA->GetSprite().SetAnimation("vanguard");	//アニメーションを指定
//	objA->GetSprite().SetAnimationFrameTime("vanguard", 0.36f);	//アニメーション１週に何フレームかけるか
//
//	objA->GenerateRectangleCollision(1);	//自身に方形の当たり判定を生成（引数で渡した分だけ作成する）
//	objA->SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(40.0f, 70.0f));	//方形のパラメータをセット
//
//	objA->SetPosition(MakeFloat3(ax, ay, 0.0f));	//座標指定
//
//	// --- objB ---
//	objB->Activation();	//有効化
//	objB->Show();	//描画有効化
//	objB->GetSprite().Initialize();	//スプライト初期化
//	objB->GetSprite().LoadTexture("rom:/texture/car.tga");	//画像読み込み
//	objB->GetSprite().DivideAnimationCells(1, 1);	//テキスチャーが縦横何分割になっているか指定
//	objB->GetSprite().CreateAnimation("objAnim", 0, 0);	//アニメーションを作成（０フレイムなので１コマ）
//	objB->GetSprite().SetAnimation("objAnim");	//アニメーションを指定
//	objB->GetSprite().SetAnimationFrameTime("objAnim", 30.0f);	//アニメーション１週に何フレームかけるか
//
//	objB->GenerateCircleCollision(1);	//自身に円形の当たり判定を生成（引数で渡した分だけ作成する）
//
//	objB->SetCircleParameter(0, MakeFloat2(0, 0), 30.0f);	//円形のパラメータをセット
//	objB->SetPosition(MakeFloat3(bx, 0.0f, 0.0f));	//座標指定
//
//	// --- Background_Layer1_far ---
//	Background_Layer1_far->Activation();	//有効化
//	Background_Layer1_far->Show();	//描画有効化
//	Background_Layer1_far->GetSprite().Initialize();	//スプライト初期化
//	Background_Layer1_far->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
//	Background_Layer1_far->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
//	Background_Layer1_far->GetSprite().LoadTexture("rom:/texture/layer1_far_sheet.tga");	//画像読み込み
//	Background_Layer1_far->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
//	Background_Layer1_far->GetSprite().CreateAnimation("Background_Layer1_far", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）	
//	Background_Layer1_far->GetSprite().SetAnimation("Background_Layer1_far");	//アニメーションを指定
//	Background_Layer1_far->GetSprite().SetAnimationFrameTime("Background_Layer1_far", 0.24f);	//アニメーション１週に何フレームかけるか
//
//	// --- Background_Layer2_mid ---
//	Background_Layer2_mid->Activation();	//有効化
//	Background_Layer2_mid->Show();	//描画有効化
//	Background_Layer2_mid->GetSprite().Initialize();	//スプライト初期化
//	Background_Layer2_mid->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
//	Background_Layer2_mid->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
//	Background_Layer2_mid->GetSprite().LoadTexture("rom:/texture/layer2_mid_sheet.tga");	//画像読み込み
//	Background_Layer2_mid->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
//	Background_Layer2_mid->GetSprite().CreateAnimation("Background_Layer2_mid", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）	
//	Background_Layer2_mid->GetSprite().SetAnimation("Background_Layer2_mid");	//アニメーションを指定
//	Background_Layer2_mid->GetSprite().SetAnimationFrameTime("Background_Layer2_mid", 0.24f);	//アニメーション１週に何フレームかけるか
//
//	// --- Background_Layer3_near ---
//	Background_Layer3_near->Activation();	//有効化
//	Background_Layer3_near->Show();	//描画有効化
//	Background_Layer3_near->GetSprite().Initialize();	//スプライト初期化
//	Background_Layer3_near->SetPosition(MakeFloat3(0.0f, 0.0f, 0.0f));	//座標指定
//	Background_Layer3_near->GetSprite().SetPolygonSize(MakeFloat2(1920.0f, 1080.0f));	//サイズ指定
//	Background_Layer3_near->GetSprite().LoadTexture("rom:/texture/layer3_near_sheet.tga");	//画像読み込み
//	Background_Layer3_near->GetSprite().DivideAnimationCells(16, 15);	//テキスチャーが縦横何分割になっているか指定
//	Background_Layer3_near->GetSprite().CreateAnimation("Background_Layer3_near", 0, 239);	//アニメーションを作成（０フレイムなので１コマ）	
//	Background_Layer3_near->GetSprite().SetAnimation("Background_Layer3_near");	//アニメーションを指定
//	Background_Layer3_near->GetSprite().SetAnimationFrameTime("Background_Layer3_near", 0.24f);	//アニメーション１週に何フレームかけるか
//
//}
//
//void Game::UpdateSample(float dt)
//{
//	//----------------------------------------------//
//	//	↓UpdateSample（初期化関数）に記述			//
//	//----------------------------------------------//
//
//	//objA->objA->Update(dt);	//不要
//	if (objA != NULL && objB != NULL)
//	{
//		const Rectangle* colA = objA->GetRectangleCollision(0);	//当たり判定を取得
//		const Circle* colB = objB->GetCircleCollision(0);	//当たり判定を取得
//
//		if (CheckRectangleCircleCollision(*colA, *colB))	//(重なっているチェック
//		{
//			//--------------------------------------//
//			// オブジェクト削除	（方法一）			//
//			//--------------------------------------//
//
//			//バイブレーション
//			SetControllerLeftVibration(1);	//左ジョイコンを振動
//			if (objA != NULL)
//			{
//				DeleteObject(objA);	//オブジェクトを削除(deleteが行われる)
//				objA = NULL;
//			}
//			else
//			{
//				objA = AddObject(new GameObject);
//
//				objA->SetPosition(MakeFloat3(ax, ay, 0.0f));	//座標指定
//				objA->Activation();	//有効化
//				objA->Show();	//描画有効化
//				objA->GetSprite().Initialize();	//スプライト初期化
//				objA->GetSprite().LoadTexture("rom:/texture/Vanguard_Idle_sheet.tga");	//画像読み込み
//				objA->GetSprite().DivideAnimationCells(6, 6);	//テキスチャーが縦横何分割になっているか指定
//				objA->GetSprite().CreateAnimation("vanguard", 0, 35);	//アニメーションを作成（０フレイムなので１コマ）
//				objA->GetSprite().SetAnimation("vanguard");	//アニメーションを指定
//				objA->GetSprite().SetAnimationFrameTime("vanguard", 0.36f);	//アニメーション１週に何フレームかけるか
//
//				objA->GenerateRectangleCollision(1);	//自身に方形の当たり判定を生成（引数で渡した分だけ作成する）
//				objA->SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(40.0f, 70.0f));	//方形のパラメータをセット
//
//			}
//		}
//	}
//
//	//キー入力の例
//	if (Input::IsTrigger(Command::Confirm))
//	{
//		if (objA != NULL)
//		{
//			DeleteObject(objA);	//オブジェクトを削除(deleteが行われる)
//			objA = NULL;
//		}
//		else
//		{
//			objA = AddObject(new GameObject);
//
//			objA->SetPosition(MakeFloat3(ax, ay, 0.0f));	//座標指定
//			objA->Activation();	//有効化
//			objA->Show();	//描画有効化
//			objA->GetSprite().Initialize();	//スプライト初期化
//			objA->GetSprite().LoadTexture("rom:/texture/Vanguard_Idle_sheet.tga");	//画像読み込み
//			objA->GetSprite().DivideAnimationCells(6, 6);	//テキスチャーが縦横何分割になっているか指定
//			objA->GetSprite().CreateAnimation("vanguard", 0, 35);	//アニメーションを作成（０フレイムなので１コマ）
//			objA->GetSprite().SetAnimation("vanguard");	//アニメーションを指定
//			objA->GetSprite().SetAnimationFrameTime("vanguard", 0.36f);	//アニメーション１週に何フレームかけるか
//
//			objA->GenerateRectangleCollision(1);	//自身に方形の当たり判定を生成（引数で渡した分だけ作成する）
//			objA->SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(40.0f, 70.0f));	//方形のパラメータをセット
//
//		}
//	}
//
//	//==========================================================//
//	//					Bullets Shooting Test					
//	//==========================================================//
//
//	// === SpACEキー押し続け = 固定スピードで連発 ===
//	if (fireCooldown > 0.0f)
//		fireCooldown -= dt;                       // 每フレイムでカウントダウン
//
//	if (objA != NULL && Input::IsPress(Command::Attack) && fireCooldown <= 0.0f)
//	{
//		Float3 p = objA->GetPosition();
//		SpawnBullet(p.x-2, p.y-57 + 20.0f);
//		SpawnVulcan(p.x - 2, p.y - 57 + 20.0f, 5, 30.0f);   // 5 粒、30 度扇形
//		fireCooldown = FIRE_INTERVAL;             // 射完重設,等下一輪
//	}
//
//	// === 更新所有子彈;撞到 objB 就一齊消失 ===
//	for (int i = (int)bullets.size() - 1; i >= 0; i--)   // 由尾掃返轉頭,刪除先安全
//	{
//		GameObject* b = bullets[i];
//
//		// 1) 向上飛
//		Float3 bp = b->GetPosition();
//		bp.x += BULLET_SPEED * dt;        // +x = 向右;若飛錯方向就改成 -=
//		b->SetPosition(bp);
//		b->UpdateCollision();             // ★ 郁完一定要同步碰撞圈
//
//		bool remove = false;
//
//		// 2) 撞到 objB?
//		if (objB != NULL)
//		{
//			const Rectangle* cb = b->GetRectangleCollision(0);
//			const Circle* ct = objB->GetCircleCollision(0);
//			if (cb && ct && CheckRectangleCircleCollision(*cb, *ct))
//			{
//				DeleteObject(objB);       // objB 消失
//				objB = NULL;
//				remove = true;            // 子彈都一齊消失
//			}
//		}
//
//		// 3) 飛出畫面就回收(唔係條 list 會無限長)
//		if (b->GetPosition().x > 2000.0f) remove = true;
//
//		// 4) 真正刪除
//		if (remove)
//		{
//			DeleteObject(b);
//			bullets.erase(bullets.begin() + i);
//		}
//	}
//
//	//==========================================================//
//	//					Vulcan Bullets Test						//
//	//==========================================================//
//	for (int i = (int)Spread.size() - 1; i >= 0; i--)
//	{
//		VulcanBullet& bu = Spread[i];
//		GameObject* Vulcan = bu.obj;
//
//		Float3 bp = Vulcan->GetPosition();
//		bp.x += bu.vx * dt;               // ★ 各自方向
//		bp.y += bu.vy * dt;
//		Vulcan->SetPosition(bp);
//		Vulcan->UpdateCollision();
//
//		bool remove = false;
//
//		if (objB != NULL)
//		{
//			const Rectangle* cb = Vulcan->GetRectangleCollision(0);
//			const Circle* ct = objB->GetCircleCollision(0);
//			if (cb && ct && CheckRectangleCircleCollision(*cb, *ct))
//			{
//				DeleteObject(objB);
//				objB = NULL;
//				remove = true;
//			}
//		}
//
//		// 出界回收 —— 向右飛,所以主要睇 X 飛出右邊;順手連上下都檢查
//		Float3 q = Vulcan->GetPosition();
//		if (q.x > 1100.0f || q.y > 1400.0f || q.y < -1400.0f) remove = true;
//
//		if (remove)
//		{
//			DeleteObject(Vulcan);
//			Spread.erase(Spread.begin() + i);
//		}
//	}
//
//
//	//----------------------------------------------------------//
//	//	軸入力の例												//
//	//----------------------------------------------------------//
//	ax += Input::GetAxis(Command::MoveX) * PLAYER_SPEED * dt;
//	ay += Input::GetAxis(Command::MoveY) * PLAYER_SPEED * dt;
//
//	objA->SetPosition(MakeFloat3(ax, ay, 0.0f));	//座標指定
//
//}
//
//void Game::DrawSample()
//{
//	//----------------------------------------------//
//	//	↓DrawSample（初期化関数）に記述			//
//	//----------------------------------------------//
//
//}
//
//static void SpawnVulcan(float x, float y, int count, float SpreadDegree)
//{
//	// 向右飛 = 0 度。扇形以 0 度為中心上下分開。
//	float start = -SpreadDegree * 0.5f;                       // 扇形最上(或最下)一條
//	float step = (count > 1) ? SpreadDegree / (count - 1) : 0.0f;
//
//	for (int i = 0; i < count; i++)
//	{
//		float degree = start + step * i;                     // 呢粒嘅角度
//		float radian = degree * 3.14159265f / 180.0f;
//
//		float vx = cosf(radian) * BULLET_SPEED;              // 向右為主
//		float vy = sinf(radian) * BULLET_SPEED;              // 上下分散
//
//		GameObject* Vulcan = GameAPI.AddObject(new GameObject);
//		Vulcan->Activation();
//		Vulcan->Show();
//		Vulcan->GetSprite().Initialize();
//		Vulcan->GetSprite().LoadTexture("rom:/texture/Vulcan.tga");
//		Vulcan->GetSprite().DivideAnimationCells(1, 1);
//		Vulcan->GetSprite().CreateAnimation("Vulcan", 0, 0);
//		Vulcan->GetSprite().SetAnimation("Vulcan");
//		Vulcan->GetSprite().SetPolygonSize(MakeFloat2(32.0f, 15.0f));
//		Vulcan->GenerateRectangleCollision(1);
//		Vulcan->SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(32.0f, 7.5f));
//		Vulcan->SetPosition(MakeFloat3(x, y, 0.0f));
//
//		Spread.push_back({ Vulcan, vx, vy });                 // ★ 記住佢嘅方向
//	}
//}
//
//static void SpawnBullet(float x, float y)
//{
//	GameObject* b = GameAPI.AddObject(new GameObject);
//	b->Activation();
//	b->Show();
//	b->GetSprite().Initialize();
//	b->GetSprite().LoadTexture("rom:/texture/Vulcan.tga");
//	b->GetSprite().DivideAnimationCells(1, 1);
//	b->GetSprite().CreateAnimation("Vulcan", 0, 0);
//	b->GetSprite().SetAnimation("Vulcan");
//	b->GetSprite().SetPolygonSize(MakeFloat2(32.0f, 15.0f)); // Vulcan.tga 好大,縮細佢
//	b->GenerateRectangleCollision(1);
//	b->SetRectangleParameter(0, MakeFloat2(0, 0), MakeFloat2(32.0f, 7.5f));
//	b->SetPosition(MakeFloat3(x, y, 0.0f));
//	bullets.push_back(b);
//}

#include "Game.h"

void Game::InitializeSample()
{

}

void Game::UpdateSample(float dt)
{

}

void Game::DrawSample()
{

}