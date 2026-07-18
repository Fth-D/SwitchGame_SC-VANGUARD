#include"sound.h"
#include "Platform/PlatformDefines.h"
#include "Renderer.h"
#include "GameTime.h"
#include "main.h"
#include "Text.h"
#include "Input.h"
#include "Log.h"
#include "Collision.h"
#include "Particle.h"
#include "PrimitiveRender.h"
#include "CameraAPI.h"
#include <cstdlib>
#include "Game.h"
#include "GameObject.h"
#include "system.h"
#include "controller.h"
#include "texture.h"
#include "CameraAPI.h"
#include <cstdlib>
#include "PrimitiveRender.h"
#include <utility>

// 2つの GameObject* を「小さい方が first」の順で組にして返す。
// std::set のキーとして使うときに (A,B) と (B,A) を同一視するための正規化。
static std::pair<GameObject*, GameObject*> MakeOrderedPair(GameObject* a, GameObject* b)
{
	return (a < b) ? std::make_pair(a, b) : std::make_pair(b, a);
}

Game::Game()
{
	objectList = nullptr;
	[[maybe_unused]] int maxObject = 1000;
	[[maybe_unused]] int currentObjectNum = 0;

	[[maybe_unused]] Scene currentScene = TITLE;
	[[maybe_unused]] Scene nextScene = TITLE;

}

Game::~Game()
{
	DeleteAllObject();
}

GameObject* Game::AddObject(GameObject* newObject)
{
	if (newObject == nullptr)
		return nullptr;

	// ゲーム全体で許容するより多くのオブジェクトを生成しない
	if (currentObjectNum >= maxObject)
		return nullptr;

	// シーン全体で許容するより多くのオブジェクトを生成しない
	if (currentObjectNum >= maxObjectInScene)
		return nullptr;

	// オブジェクトリストの空いているスロットを探してオブジェクトを追加
	if (freeSlotCount <= 0)
	{
		delete newObject;
		return nullptr;
	}

	freeSlotCount--;
	int slot = freeSlots[freeSlotCount];

	objectList[slot] = newObject;
	objectList[slot]->SetObjectId(slot);
	currentObjectNum++;
	return newObject;
}

void Game::DeleteObject(GameObject* deleteTarget)
{
	if (deleteTarget == nullptr)
		return;

	int targetId = deleteTarget->GetObjectId();
	if (targetId < 0 || targetId >= maxObjectInScene)
		return;

	if (objectList[targetId] == nullptr)
		return;

	// 重なり中だった全ての相手に CollisionReactionExit を発火してから削除する。
	// 削除されるオブジェクト自身には Exit を通知しない (Unity の挙動に合わせる)。
	for (auto it = prevCollisionPairs.begin(); it != prevCollisionPairs.end(); )
	{
		if (it->first == deleteTarget || it->second == deleteTarget)
		{
			GameObject* opponent = (it->first == deleteTarget) ? it->second : it->first;
			if (opponent != nullptr) opponent->CollisionReactionExit(deleteTarget);
			it = prevCollisionPairs.erase(it);
		}
		else
		{
			++it;
		}
	}

	delete objectList[targetId];
	objectList[targetId] = nullptr;
	currentObjectNum--;

	freeSlots[freeSlotCount] = targetId;
	freeSlotCount++;

	// ただし、シンプルにこの関数を使用すると多分バグるぞ！なんでか調べてみよう！
}

void Game::DeleteAllObject()
{
	if (objectList == nullptr)
		return;

	playerObject = nullptr;

	// シーン破棄/総入替時はオブジェクトが全部消えるので Exit は発火させず、ペア集合だけクリアする。
	prevCollisionPairs.clear();

	for (int i = 0; i <maxObjectInScene;i++)
	{
		if (objectList[i] == nullptr)
			continue;

		delete objectList[i];
		objectList[i] = nullptr;
	}

	currentObjectNum = 0;
	delete[] objectList;
	objectList = nullptr;
	delete[] freeSlots;
	freeSlots = nullptr;
	freeSlotCount = 0;
}

void Game::GenerateObjectList(int num)
{
	objectList = new GameObject * [num];
	for (int i = 0;i < num;i++)
	{
		// オブジェクトリストに明示的にnullptrを入れ、何も入っていない空、という状態にしておく。
		objectList[i] = nullptr;
	}

	freeSlots = new int[num];
	freeSlotCount = num;
	for (int i = 0; i < num; i++)
	{
		freeSlots[i] = num - 1 - i;
	}

	maxObjectInScene = num;
}

void Game::Process()
{
	// 目標フレームレートで更新・描画する（事実上の固定タイムステップ）。
	// 前回処理を始めた時刻を覚えておき、そこから 1/TARGET_FPS 秒たったら 1 フレーム処理する。
	// 経過するまでは何もせず周回するだけ。VSync を切っているので、ここがフレームレートを決める。
	const float step = 1.0f / (float)TARGET_FPS;   // 1 フレームの長さ（秒）
	float lastProcessTime = Time::NowSeconds();    // 前回処理を始めた時刻

	while (true)
	{
#if PLATFORM_WIN64
		{ extern bool PlatformShouldClose(); if (PlatformShouldClose()) break; }
#endif
		// 前回処理から step 秒たっていなければ、まだ処理しない
		float now = Time::NowSeconds();
		if (now - lastProcessTime < step)
			continue;

		lastProcessTime = now;   // 今回の処理開始時刻を記録

		Time::Update();            // 実測の経過時間を更新（FPS 表示などの計測用）
		Update(step);              // ゲーム更新には固定の step を渡す（毎フレーム同じ進み幅）
		Draw();
		AdjustObjectList();

		if (CheckChangeScene() == true)
		{
			TransitNextScene();
		}
	}
}

void Game::Update(float dt)
{
	// dt はメインループから渡される固定ステップ（1/TARGET_FPS 秒）

	// サウンドのアップデート
	UpdateSound();

	// コントローラのアップデート
	UpdateController();
	Input::Update(dt);

	// ゲームオブジェクトのアップデート
	UpdateObject(dt);

	if (autoCollision) ResolveCollision();

	Particle::Update(dt);

	// Screen shake decay
	if (shakeTimer > 0.0f) shakeTimer -= dt;

	// シーン固有の処理
	switch (currentScene)
	{
	case TITLE:
		UpdateTitle(dt);
		break;

	case GAME:
		UpdateGame(dt);
		break;

	case RESULT:
		UpdateResult(dt);
		break;

	case GAMEOVER:
		UpdateGameOver(dt);
		break;

	case SAMPLE:
		UpdateSample(dt);
		break;

	default:
		break;
	}
}

void Game::UpdateObject(float dt)
{
	if (objectList == nullptr)
		return;

	for (int i = 0;i < maxObjectInScene;i++)
	{
		if (objectList[i] == nullptr)
			continue;
		bool isUpdatable = (objectList[i] != nullptr || objectList[i]->GetObjectActiveState() == true);
		if (isUpdatable == false)
			continue;

		objectList[i]->Update(dt);
	}
}

void Game::Draw()
{
	if (objectList == nullptr)
		return;

	// 背景色でクリア
	Renderer::BeginFrame();

	// Screen shake
	if (shakeTimer > 0.0f)
	{
		float sx = (float)(rand() % 200 - 100) / 100.0f * shakeIntensity;
		float sy = (float)(rand() % 200 - 100) / 100.0f * shakeIntensity;
		Renderer::SetScreenOffset(sx, sy);
	}
	else
	{
		Renderer::SetScreenOffset(0.0f, 0.0f);
	}

	for (int i = 0; i < maxObjectInScene; i++)
	{
		if (objectList[i] == nullptr) continue;
		if (!objectList[i]->GetObjectActiveState()) continue;
		objectList[i]->Draw();
	}

	DrawCollisionDebug();
	Particle::Draw();


	// Scene-specific draw
	switch (currentScene)
	{
	case GAME:
		DrawGame();
		break;
	case SAMPLE:
		DrawSample();
		break;
	default:
		break;
	}

	Log::DrawScreen();

	Renderer::EndFrame();
}

// ResolveCollision (Framework)
// 当たり判定を解決し、Enter / Stay / Exit のコールバックをディスパッチする。
//   - 今フレーム重なっている && 前フレームは重なっていなかった -> CollisionReactionEnter
//   - 今フレーム重なっている && 前フレームも重なっていた       -> CollisionReaction (Stay)
//   - 今フレーム重なっていない && 前フレームは重なっていた     -> CollisionReactionExit
void Game::ResolveCollision()
{
	if (objectList == nullptr) return;

	std::set<std::pair<GameObject*, GameObject*>> currentPairs;

	for (int i = 0; i < maxObjectInScene; i++)
	{
		if (objectList[i] == nullptr) continue;
		if (!objectList[i]->GetObjectActiveState()) continue;

		for (int j = i + 1; j < maxObjectInScene; j++)
		{
			if (objectList[j] == nullptr) continue;
			if (!objectList[j]->GetObjectActiveState()) continue;

			const Circle* ci = objectList[i]->GetCircleCollision(0);
			const Circle* cj = objectList[j]->GetCircleCollision(0);
			const Rectangle* ri = objectList[i]->GetRectangleCollision(0);
			const Rectangle* rj = objectList[j]->GetRectangleCollision(0);

			if (!ci && !ri) continue;
			if (!cj && !rj) continue;

			bool hit = false;

			if (ci && cj) hit = CheckCircleCollision(*ci, *cj);
			if (!hit && ri && rj) hit = CheckRectangleCollision(*ri, *rj);
			if (!hit && ri && cj) hit = CheckRectangleCircleCollision(*ri, *cj);
			if (!hit && rj && ci) hit = CheckRectangleCircleCollision(*rj, *ci);

			if (hit)
			{
				auto p = MakeOrderedPair(objectList[i], objectList[j]);
				currentPairs.insert(p);

				if (prevCollisionPairs.find(p) == prevCollisionPairs.end())
				{
					// 今フレームから新規に重なり始めた
					objectList[i]->CollisionReactionEnter(objectList[j]);
					objectList[j]->CollisionReactionEnter(objectList[i]);
				}
				else
				{
					// 前フレームから引き続き重なっている
					objectList[i]->CollisionReaction(objectList[j]);
					objectList[j]->CollisionReaction(objectList[i]);
				}
			}
		}
	}

	// 前フレームに重なっていて、今フレームは重なっていないペア -> Exit
	for (const auto& p : prevCollisionPairs)
	{
		if (currentPairs.find(p) != currentPairs.end()) continue;
		// 両端ともまだ生きているはず (途中で削除されたペアは DeleteObject() 内で取り除き済み)
		p.first->CollisionReactionExit(p.second);
		p.second->CollisionReactionExit(p.first);
	}

	prevCollisionPairs = std::move(currentPairs);
}

// DrawCollisionDebug
void Game::DrawCollisionDebug()
{
	if (!debugDrawCollision) return;
	if (objectList == nullptr) return;

	Float4 color = MakeFloat4(0.0f, 1.0f, 0.0f, 0.5f);

	for (int i = 0; i < maxObjectInScene; i++)
	{
		if (objectList[i] == nullptr) continue;
		if (!objectList[i]->GetObjectActiveState()) continue;

		Float3 wpos = objectList[i]->GetPosition();
		Float2 spos = objectList[i]->GetFixedScreenPositionState()
			? MakeFloat2(wpos.x, wpos.y)
			: WorldToScreen(wpos);

		const Circle* c = objectList[i]->GetCircleCollision(0);
		if (c)
		{
			Circle sc;
			sc.x = spos.x; sc.y = spos.y; sc.radius = c->radius;
			RenderCircleWireframe(sc, color);
		}

		const Rectangle* r = objectList[i]->GetRectangleCollision(0);
		if (r)
		{
			Rectangle sr;
			sr.x = spos.x; sr.y = spos.y;
			sr.width = r->width; sr.height = r->height;
			RenderRectangleWireframe(sr, color);
		}
	}
}

void Game::ShakeScreen(float intensity, float duration)
{
	shakeIntensity = intensity;
	shakeTimer = duration;
}

GameObject* Game::FindNearestObject(float x, float y, ObjectType type)
{
	if (objectList == nullptr) return nullptr;
	GameObject* nearest = nullptr;
	float nearestDistSq = 99999999.0f;
	for (int i = 0; i < maxObjectInScene; i++)
	{
		if (objectList[i] == nullptr) continue;
		if (!objectList[i]->GetObjectActiveState()) continue;
		if (objectList[i]->GetObjectType() != type) continue;
		Float3 pos = objectList[i]->GetPosition();
		float dx = pos.x - x;
		float dy = pos.y - y;
		float distSq = dx * dx + dy * dy;
		if (distSq < nearestDistSq) { nearestDistSq = distSq; nearest = objectList[i]; }
	}
	return nearest;
}

void Game::AdjustObjectList()
{
	// 更新および描画の両方を止めたオブジェクトをリストから削除
	for (int i = 0;i < maxObjectInScene;i++)
	{
		if (objectList[i] == nullptr)
			continue;
		if (objectList[i]->GetObjectActiveState() == true)
			continue;
		if (objectList[i]->GetObjectRenderState() == true)
			continue;

		DeleteObject(objectList[i]);
	}
}

void Game::Initialize()
{
	InitSound();
	Log::Initialize();
	Log::SetScreenDisplay(true);
	Log::Info("Game::Initialize start");
	InitSystem();
	Log::Info("InitSystem done");
	Renderer::Initialize();
	Log::Info("Renderer done");
	Log::Trace("Time::Initialize...");
	Time::Initialize();
	Log::Trace("Text::Initialize...");
#if PLATFORM_NX
	Text::Initialize("rom:/Fonts/ROG_regular.ttc");
#else
	Text::Initialize("C:/Windows/Fonts/ROG_regular.ttc");
#endif
	InitController();
	Input::Initialize();
	Particle::Initialize();
	SetControllerLeftAngleBase();
	SetControllerRightAngleBase();

	FinalizeCurrentScene();
	InitializeNextScene();

	playerObject = nullptr;
}

void Game::Finalize()
{
	Particle::Finalize();
	Text::Finalize();
	Renderer::Finalize();
	UninitSystem();
	UninitController();

	// テクスチャ解放処理はここにまとめます
	UnloadTextures();
	UninitSound();
}


// シーンを初期化。外からは呼ばれないようにprivateにしている
void Game::InitializeNextScene()
{
	// 登録されているオブジェクトをすべて削除
	DeleteAllObject();
	GenerateObjectList(maxObject);

	// シーンごとの初期化開始
	switch (nextScene)
	{
	case TITLE:
		InitializeTitle();
		break;

	case GAME:
		InitializeGame();
		break;

	case RESULT:
		InitializeResult();
		break;

	case GAMEOVER:
		InitializeGameOver();
		break;

	case SAMPLE:
		InitializeSample();
		break;

	default:
		break;
	}
}

// シーンを終了
void Game::FinalizeCurrentScene()
{
	DeleteAllObject();
}

// シーン遷移処理
void Game::TransitNextScene()
{
	FinalizeCurrentScene();
	InitializeNextScene();
	ResetCamera();
	currentScene = nextScene;
}
