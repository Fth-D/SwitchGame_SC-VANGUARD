#pragma once
#include "Platform/PlatformTypes.h"

// ================================================
// パーティクルシステム
//  テクスチャを使わず、色付きの小さな四角形で粒子を表現する。
//  用途に応じた5種類のプリセットを提供する。
//
//  使い方:
//    Particle::Spark(x, y, 30);                  // 爆発・ヒット
//    Particle::Circle(x, y, 24);                 // 衝撃波
//    Particle::Fountain(x, y, 20);               // 花火・噴水
//    Particle::Trail(x, y, dirX, dirY, 5);       // 弾の軌跡
//    Particle::Rise(x, y, 10);                   // 煙・回復
//    Particle::Emit(x, y, 30, color, ...);       // 詳細カスタム
//
//  座標系（各放出関数の最後の引数 screenSpace で指定）:
//    省略時(false) = ワールド座標系。カメラに追従し、ワールド上の位置に留まる。
//    true        = スクリーン座標系。画面に固定（UI演出向き）。
//    例) Particle::Spark(x, y, 30, color, true);  // スクリーン座標で放出
// ================================================
namespace Particle
{
	// --- システム ---
	void Initialize(int maxParticles = 2048);
	void Finalize();
	void Update(float dt);
	void Draw();

	// --- 汎用放出（全パラメータ指定） ---
	void Emit(float x, float y, int count,
	          Float4 color    = MakeFloat4(1.0f, 0.9f, 0.3f, 1.0f),
	          float speedMin  = 50.0f,
	          float speedMax  = 200.0f,
	          float lifeMin   = 0.3f,
	          float lifeMax   = 0.8f,
	          float sizeMin   = 2.0f,
	          float sizeMax   = 6.0f,
	          bool screenSpace = false);

	// --- プリセット ---

	// Spark: 全方向にランダム放出（爆発、ヒットエフェクト）
	void Spark(float x, float y, int count = 20,
	           Float4 color = MakeFloat4(1.0f, 0.9f, 0.3f, 1.0f),
	           bool screenSpace = false);

	// Circle: 円形に均等拡散（衝撃波、バリア展開）
	void Circle(float x, float y, int count = 24,
	            Float4 color = MakeFloat4(1.0f, 1.0f, 1.0f, 1.0f),
	            float radius = 200.0f,
	            float life   = 0.4f,
	            bool screenSpace = false);

	// Fountain: 上方向に噴出して落下（花火、噴水）
	void Fountain(float x, float y, int count = 20,
	              Float4 color = MakeFloat4(1.0f, 0.5f, 0.1f, 1.0f),
	              bool screenSpace = false);

	// Trail: 指定方向に細く流れる（弾の軌跡、ジェット噴射）
	//  dirX, dirY: 流れる方向（正規化不要、内部で正規化する）
	void Trail(float x, float y, float dirX, float dirY, int count = 5,
	           Float4 color = MakeFloat4(0.5f, 0.8f, 1.0f, 1.0f),
	           bool screenSpace = false);

	// Rise: ゆっくり上昇（煙、ほこり、回復エフェクト）
	void Rise(float x, float y, int count = 10,
	          Float4 color = MakeFloat4(0.7f, 0.7f, 0.7f, 1.0f),
	          bool screenSpace = false);

	// Explosion: 大きく激しい爆発（赤〜橙〜黄のグラデーション）
	void Explosion(float x, float y, int count = 60,
	               bool screenSpace = false);

	// Confetti: カラフルな紙吹雪（祝福、ステージクリア）
	void Confetti(float x, float y, int count = 40,
	              bool screenSpace = false);

	// Spiral: 渦を巻くように拡散（魔法、ワープ）
	void Spiral(float x, float y, int count = 30,
	            Float4 color = MakeFloat4(0.6f, 0.3f, 1.0f, 1.0f),
	            bool screenSpace = false);

	// Charge: 周囲から中心に収束（チャージ、吸収）
	void Charge(float x, float y, int count = 24,
	            Float4 color = MakeFloat4(0.3f, 1.0f, 0.6f, 1.0f),
	            float radius = 150.0f,
	            bool screenSpace = false);

	// Flash: 一瞬だけ光る（マズルフラッシュ、着弾閃光）
	void Flash(float x, float y, int count = 15,
	           Float4 color = MakeFloat4(1.0f, 1.0f, 0.9f, 1.0f),
	           bool screenSpace = false);
}
