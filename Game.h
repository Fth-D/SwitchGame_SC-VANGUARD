#pragma once
#include "GameObject.h"
#include "Scene.h"
#include <set>
#include <utility>

class Game
{
private:
	//プライベートにコンストラクタを持つと、オブジェクトを生成できなくなる（何の意味があるの？って思うかもですが、これが重要です）
	Game();

	GameObject* playerObject = nullptr;

public:
	// デストラクタはpublicでも問題ない。
	~Game();

	// 実はprivateやpublicは複数回書いても問題ないんだ。（たまに役立つ）
private:
	// オブジェクトのポインタのポインタ。このサンプルは前期のCL23の総集編的な内容です。この変数にめちゃくちゃ注目しながらサンプルを読みましょう
	GameObject** objectList = nullptr;

	// ゲームで認めるオブジェクトの最大数
	int maxObject = 1000;

	// シーン内に許可するオブジェクトの数
	int maxObjectInScene = 0;

	// 今のオブジェクト数
	int currentObjectNum = 0;

	// フリーリスト
	int* freeSlots = nullptr;
	int freeSlotCount = 0;

	// 当たり判定のデバッグ描画
	bool debugDrawCollision = false;
	bool autoCollision = true;

	// Pairs that overlapped on the previous frame (used to derive Enter/Exit events).
	// Canonicalized so that pair::first < pair::second by pointer value.
	std::set<std::pair<GameObject*, GameObject*>> prevCollisionPairs;

	// Screen shake
	float shakeIntensity = 0.0f;
	float shakeTimer = 0.0f;

	// シーンを初期化。外からは呼ばれないようにprivate実装
	void InitializeNextScene();

	// シーンを終了
	void FinalizeCurrentScene();

	// シーン遷移処理
	void TransitNextScene();

	// ゲーム全体の更新処理
	void Update(float dt);

	// オブジェクトのアップデート
	void UpdateObject(float dt);

	// ゲーム全体の描画処理
	void Draw();

	// オブジェクトリストを整理
	void AdjustObjectList();
	void DrawCollisionDebug();

	//　タイトルの処理一式
	void InitializeTitle();
	void UpdateTitle(float dt);

	// ゲームシーン処理一式
	void InitializeGame();
	void UpdateGame(float dt);
	void DrawGame();

	// リザルト画面の処理一式
	void InitializeResult();
	void UpdateResult(float dt);

	// ゲームオーバー画面の処理一式
	void InitializeGameOver();
	void UpdateGameOver(float dt);

	// Sample
	void InitializeSample();
	void UpdateSample(float dt);
	void DrawSample();

	// オブジェクトリストを再作成
	void GenerateObjectList(int num);

public:
	// 現在のシーン
	Scene currentScene = GAME;

	// 次のシーン（予定）
	Scene nextScene = GAME;

	// ゲーム内にオブジェクトを追加
	GameObject* AddObject(GameObject* newObject = nullptr);

	// ゲーム内のオブジェクトを削除
	void DeleteObject(GameObject* deleteTarget = nullptr);

	// ゲーム内のオブジェクトをすべて削除
	void DeleteAllObject();

	// ゲーム全体の初期化
	void Initialize();

	// ゲーム全体の終了処理
	void Finalize();

	// ゲーム全体の処理全部を実行するラッパー（覆う）関数
	void Process();

	// 現在のシーンを確認
	Scene GetCurrentScene() { return currentScene; }

	// 次のシーン（予定）を確認
	Scene GetNextScene() { return nextScene; }

	// 次のシーンを設定
	void ChangeScene(Scene next) { nextScene = next; }

	void SetDebugDrawCollision(bool enabled) { debugDrawCollision = enabled; }
	void SetAutoCollision(bool enabled) { autoCollision = enabled; }
	void ShakeScreen(float intensity, float duration);
	bool GetDebugDrawCollision() { return debugDrawCollision; }

	bool CheckChangeScene() { return currentScene != nextScene; };

	// 衝突解決
	void ResolveCollision();

	// 敵の生成
	void GenerateNewEnemy();

	// 敵の数を取得
	int GetEnemyCount();

	// 敵がいないかどうかを確認
	bool IsNoEnemy();

	// 敵を倒した数を加算
	void AddBeatCount();

	// プレイヤーオブジェクトを取得
	GameObject* GetPlayerObject();
	GameObject* FindNearestObject(float x, float y, ObjectType type);

	/*重要：シングルトンパターンという実装について*/
	// コンストラクタがprivate、ということは、メンバ関数の中では実行できるということ。
	// メンバ変数の中でオブジェクトを作ることは可能。そのオブジェクトをstaticにしてあげることで、ここでしかオブジェクトを作れないという実装にする。
	// ゲーム内に1つしか存在しない（システム上の）オブジェクトはシングルトンパターンで実装するとよい。
	// コンストラクタをprivateにしながら、以下のように自クラスのオブジェクトのアドレスを返す関数を作成するとシングルトンパターンになる。
	static Game& GetInstance() { static Game game; return game; };


};

// ゲーム側の関数を使うときに使用するマクロ
// GameAPI.関数名　と使う。
#define GameAPI Game::GetInstance()