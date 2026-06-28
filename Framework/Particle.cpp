// ================================================
// Particle.cpp
// 軽量パーティクルシステム
//
// 仕組み:
//   固定長配列で粒子を管理する（new/deleteしない）。
//   各粒子は「位置・速度・色・寿命・サイズ」を持つ。
//   Emit()で空きスロットに粒子を書き込み、
//   Update()で全粒子を一括更新し、
//   Draw()で生存中の粒子をまとめて描画する。
//
//   粒子が寿命を迎えると active=false になり、
//   次のEmit()でスロットが再利用される。
//
//   描画はRenderer::DrawVertices()で行う。
//   テクスチャを使わず、色とアルファだけで表現する。
//   寿命に応じてアルファが減衰し、フェードアウトする。
// ================================================

#include "Particle.h"
#include "Renderer.h"
#include "main.h"
#include "CameraAPI.h"
#include <cstdlib>
#include <cmath>

namespace
{
	// 1粒子のデータ
	struct ParticleData
	{
		float x, y;           // 位置（worldSpace=true:ワールド座標 / false:スクリーン座標）
		float vx, vy;         // 速度（px/秒）
		Float4 color;         // 基本色
		float life;           // 残り寿命（秒）
		float lifeMax;        // 初期寿命（フェード計算用）
		float size;           // 大きさ（px）
		bool active;          // 生存中か
		bool worldSpace;      // 位置をワールド座標として扱うか（Draw時にWorldToScreen変換）
	};

	ParticleData* s_Particles = nullptr;
	int s_MaxParticles = 0;
	int s_NextSlot = 0;       // 次に書き込むスロット（リングバッファ的に回す）


	// ランダム float（min〜max）
	float RandRange(float minVal, float maxVal)
	{
		float t = (float)(rand() % 10001) / 10000.0f;  // 0.0〜1.0
		return minVal + t * (maxVal - minVal);
	}
}


// ================================================
// 初期化
// ================================================
void Particle::Initialize(int maxParticles)
{
	s_MaxParticles = maxParticles;
	s_Particles = new ParticleData[maxParticles];
	s_NextSlot = 0;

	for (int i = 0; i < maxParticles; i++)
		s_Particles[i].active = false;
}


// ================================================
// 終了
// ================================================
void Particle::Finalize()
{
	delete[] s_Particles;
	s_Particles = nullptr;
	s_MaxParticles = 0;
}


// ================================================
// 更新
//  全粒子の位置を速度で移動し、寿命を減らす。
//  寿命が0以下になったら非アクティブにする。
// ================================================
void Particle::Update(float dt)
{
	if (s_Particles == nullptr) return;

	for (int i = 0; i < s_MaxParticles; i++)
	{
		if (!s_Particles[i].active) continue;

		// 位置を更新
		s_Particles[i].x += s_Particles[i].vx * dt;
		s_Particles[i].y += s_Particles[i].vy * dt;

		// 重力（下方向に加速）
		s_Particles[i].vy += 100.0f * dt;

		// 寿命を減らす
		s_Particles[i].life -= dt;
		if (s_Particles[i].life <= 0.0f)
			s_Particles[i].active = false;
	}
}


// ================================================
// 描画
//  生存中の粒子をRenderer::DrawVerticesでまとめて描画する。
//  寿命の残り割合に応じてアルファを減衰させ、フェードアウトする。
// ================================================
void Particle::Draw()
{
	if (s_Particles == nullptr) return;

	Renderer::SetTexture(0);

	for (int i = 0; i < s_MaxParticles; i++)
	{
		if (!s_Particles[i].active) continue;

		ParticleData& p = s_Particles[i];

		// 寿命の残り割合（1.0 = 生まれた直後, 0.0 = 消滅直前）
		float lifeRatio = p.life / p.lifeMax;

		// アルファをフェードアウト
		Float4 color = p.color;
		color.w = lifeRatio;

		// 小さな四角形（size x size）を描画
		float half = p.size * 0.5f;

		// ワールド座標の粒子は描画時にスクリーン座標へ変換する（カメラに追従）
		Float2 pos = p.worldSpace
			? WorldToScreen(MakeFloat3(p.x, p.y, 0.0f))
			: MakeFloat2(p.x, p.y);

		VERTEX_3D v[4];
		v[0].Position = MakeFloat3(pos.x - half, pos.y - half, 0.0f);
		v[1].Position = MakeFloat3(pos.x + half, pos.y - half, 0.0f);
		v[2].Position = MakeFloat3(pos.x - half, pos.y + half, 0.0f);
		v[3].Position = MakeFloat3(pos.x + half, pos.y + half, 0.0f);

		for (int j = 0; j < 4; j++)
		{
			v[j].Color = color;
			v[j].TexCoord = MakeFloat2(0.0f, 0.0f);
		}

		Renderer::DrawVertices(v, 4, DrawMode::TriangleStrip);
	}
}


// ================================================
// 粒子の放出
//  指定位置から count 個の粒子を全方向にランダムに放出する。
//
//  速度の決め方:
//    角度をランダム（0〜360度）に決め、
//    速さをspeedMin〜speedMaxの範囲でランダムに決める。
//    vx = cos(angle) * speed
//    vy = sin(angle) * speed
// ================================================
void Particle::Emit(float x, float y, int count,
                    Float4 color,
                    float speedMin, float speedMax,
                    float lifeMin, float lifeMax,
                    float sizeMin, float sizeMax,
                    bool screenSpace)
{
	if (s_Particles == nullptr) return;

	for (int n = 0; n < count; n++)
	{
		// リングバッファ的にスロットを回す（古い粒子を上書き）
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;
		p.x = x;
		p.y = y;

		// ランダムな方向に飛ばす
		float angle = RandRange(0.0f, 6.2831853f);  // 0〜2*PI
		float speed = RandRange(speedMin, speedMax);
		p.vx = cosf(angle) * speed;
		p.vy = sinf(angle) * speed;

		p.color   = color;
		p.life    = RandRange(lifeMin, lifeMax);
		p.lifeMax = p.life;
		p.size    = RandRange(sizeMin, sizeMax);
	}
}


// ================================================
// Spark: 全方向ランダム放出
//  爆発やヒットエフェクトに使う。
//  速度・寿命・サイズがランダムなので、自然な散らばりになる。
// ================================================
void Particle::Spark(float x, float y, int count, Float4 color, bool screenSpace)
{
	Emit(x, y, count, color,
		50.0f, 250.0f,     // 速度: 50〜250 px/秒
		0.3f, 0.8f,        // 寿命: 0.3〜0.8秒
		2.0f, 6.0f,
		screenSpace);       // サイズ: 2〜6px
}


// ================================================
// Circle: 円形に均等拡散
//  全粒子を360度均等に配置し、同じ速度で外側に広がる。
//  衝撃波やバリアの展開演出に使う。
//
//  radius / life で速度を決める。
//  例: 200px を 0.4秒で広がる → 速度 = 500px/秒
// ================================================
void Particle::Circle(float x, float y, int count, Float4 color,
                      float radius, float life, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	// 全粒子が寿命内にradiusまで届く速度
	float speed = radius / life;

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;
		p.x = x;
		p.y = y;

		// 均等に360度分配（ランダムではなく等間隔）
		float angle = (float)n / (float)count * 6.2831853f;
		p.vx = cosf(angle) * speed;
		p.vy = sinf(angle) * speed;

		p.color   = color;
		p.life    = life;
		p.lifeMax = life;
		p.size    = 3.0f;
	}
}


// ================================================
// Fountain: 上方向に噴出して落下
//  花火や噴水の演出に使う。
//  角度を上向き（-60度〜-120度）に制限し、重力で放物線を描く。
//  Updateの重力（+100px/秒^2）が自然な落下を生む。
// ================================================
void Particle::Fountain(float x, float y, int count, Float4 color, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;
		p.x = x;
		p.y = y;

		// 上向き（-60度〜-120度 = 上方向の60度範囲）
		// -PI/2 が真上。そこから±30度の範囲
		float angle = RandRange(-2.094f, -1.047f);  // -120度〜-60度（ラジアン）
		float speed = RandRange(200.0f, 500.0f);
		p.vx = cosf(angle) * speed;
		p.vy = sinf(angle) * speed;   // 負の値 = 上向き

		p.color   = color;
		p.life    = RandRange(0.8f, 1.5f);  // 長めの寿命（落下を見せる）
		p.lifeMax = p.life;
		p.size    = RandRange(3.0f, 6.0f);
	}
}


// ================================================
// Trail: 指定方向に細く流れる
//  弾の軌跡やジェット噴射の演出に使う。
//  方向ベクトルを中心に、狭い角度範囲（±15度）で拡散する。
//  速度は遅め、寿命は短めで、細い尾を引く。
// ================================================
void Particle::Trail(float x, float y, float dirX, float dirY, int count,
                     Float4 color, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	// 方向ベクトルを正規化
	float len = sqrtf(dirX * dirX + dirY * dirY);
	if (len < 0.001f) return;
	dirX /= len;
	dirY /= len;

	// 基準角度
	float baseAngle = atan2f(dirY, dirX);

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;
		p.x = x;
		p.y = y;

		// 基準角度 ± 15度（0.26ラジアン）の範囲で散らす
		float angle = baseAngle + RandRange(-0.26f, 0.26f);
		float speed = RandRange(80.0f, 180.0f);
		p.vx = cosf(angle) * speed;
		p.vy = sinf(angle) * speed;

		p.color   = color;
		p.life    = RandRange(0.1f, 0.3f);  // 短い寿命で細い軌跡
		p.lifeMax = p.life;
		p.size    = RandRange(1.0f, 3.0f);  // 小さめ
	}
}


// ================================================
// Rise: ゆっくり上昇
//  煙、ほこり、回復エフェクトに使う。
//  横方向にわずかに揺れながら、ゆっくり上昇する。
//  寿命が長く、大きめの粒子でふわっとした印象を出す。
//  ※ 重力（下方向+100px/秒^2）を打ち消す程度の上昇速度にする。
// ================================================
void Particle::Rise(float x, float y, int count, Float4 color, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;
		// 出現位置を少しばらけさせる
		p.x = x + RandRange(-20.0f, 20.0f);
		p.y = y + RandRange(-5.0f, 5.0f);

		// ゆっくり上昇 + 微量の横揺れ
		// 上昇速度 -120〜-180 で、重力 +100 を差し引くと実質 -20〜-80px/秒
		p.vx = RandRange(-15.0f, 15.0f);
		p.vy = RandRange(-180.0f, -120.0f);

		p.color   = color;
		p.life    = RandRange(1.0f, 2.0f);   // 長い寿命
		p.lifeMax = p.life;
		p.size    = RandRange(4.0f, 10.0f);  // 大きめ
	}
}


// ================================================
// Explosion: 大きく激しい爆発
//  赤〜橙〜黄のランダムなグラデーションで、
//  Sparkより高速・大粒・多量に放出する。
//  ボス撃破や大ダメージの演出に使う。
// ================================================
void Particle::Explosion(float x, float y, int count, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;
		p.x = x + RandRange(-10.0f, 10.0f);  // 少しばらけた出現位置
		p.y = y + RandRange(-10.0f, 10.0f);

		float angle = RandRange(0.0f, 6.2831853f);
		float speed = RandRange(100.0f, 500.0f);  // 高速
		p.vx = cosf(angle) * speed;
		p.vy = sinf(angle) * speed;

		// 赤〜黄のランダムグラデーション
		float r = RandRange(0.8f, 1.0f);
		float g = RandRange(0.2f, 0.9f);
		p.color = MakeFloat4(r, g, 0.1f, 1.0f);

		p.life    = RandRange(0.3f, 1.0f);
		p.lifeMax = p.life;
		p.size    = RandRange(4.0f, 12.0f);  // 大粒
	}
}


// ================================================
// Confetti: カラフルな紙吹雪
//  ランダムな明るい色の粒子がゆっくり落下する。
//  祝福やステージクリアの演出に使う。
//  重力で自然に落ちるが、横方向にもばらける。
// ================================================
void Particle::Confetti(float x, float y, int count, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;
		p.x = x + RandRange(-100.0f, 100.0f);  // 広い範囲に散らす
		p.y = y + RandRange(-50.0f, 50.0f);

		// 上に少し飛んでから落ちてくる
		p.vx = RandRange(-80.0f, 80.0f);
		p.vy = RandRange(-200.0f, -50.0f);

		// ランダムに明るい色を選ぶ（R/G/Bのうち1つを強くする）
		float r = RandRange(0.3f, 1.0f);
		float g = RandRange(0.3f, 1.0f);
		float b = RandRange(0.3f, 1.0f);
		// 1チャンネルを必ず明るくする
		int ch = rand() % 3;
		if (ch == 0) r = 1.0f;
		else if (ch == 1) g = 1.0f;
		else b = 1.0f;
		p.color = MakeFloat4(r, g, b, 1.0f);

		p.life    = RandRange(1.5f, 3.0f);    // 長寿命（ゆっくり消える）
		p.lifeMax = p.life;
		p.size    = RandRange(4.0f, 8.0f);
	}
}


// ================================================
// Spiral: 渦を巻くように拡散
//  円周上に粒子を配置し、接線方向の速度を与える。
//  接線方向 + 外向き方向を合成して、渦巻き状の軌道になる。
//  魔法やワープの演出に使う。
//
//  計算:
//    外向き: (cos(a), sin(a)) × outSpeed
//    接線  : (-sin(a), cos(a)) × tanSpeed
//    合成  : 外向き + 接線
// ================================================
void Particle::Spiral(float x, float y, int count, Float4 color, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;

		// 円周上の位置に配置
		float angle = (float)n / (float)count * 6.2831853f;
		float spawnRadius = RandRange(5.0f, 20.0f);
		p.x = x + cosf(angle) * spawnRadius;
		p.y = y + sinf(angle) * spawnRadius;

		// 外向き + 接線方向の合成速度
		float outSpeed = RandRange(50.0f, 120.0f);   // 外向きの速度
		float tanSpeed = RandRange(100.0f, 250.0f);   // 接線方向の速度
		float outX = cosf(angle);
		float outY = sinf(angle);
		float tanX = -sinf(angle);  // 接線 = 法線を90度回転
		float tanY = cosf(angle);

		p.vx = outX * outSpeed + tanX * tanSpeed;
		p.vy = outY * outSpeed + tanY * tanSpeed;

		p.color   = color;
		p.life    = RandRange(0.5f, 1.0f);
		p.lifeMax = p.life;
		p.size    = RandRange(2.0f, 5.0f);
	}
}


// ================================================
// Charge: 周囲から中心に収束
//  radius の距離に粒子を生成し、中心に向かって移動させる。
//  チャージ攻撃や吸収の演出に使う。
//
//  速度 = radius / life で、寿命内にちょうど中心に届くように計算。
// ================================================
void Particle::Charge(float x, float y, int count, Float4 color, float radius, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;

		// 中心からradius離れた位置に出現
		float angle = (float)n / (float)count * 6.2831853f;
		p.x = x + cosf(angle) * radius;
		p.y = y + sinf(angle) * radius;

		// 中心に向かう速度（寿命内に到達）
		float life = RandRange(0.3f, 0.6f);
		float speed = radius / life;
		p.vx = -cosf(angle) * speed;  // 中心方向 = 外向きの逆
		p.vy = -sinf(angle) * speed;

		p.color   = color;
		p.life    = life;
		p.lifeMax = life;
		p.size    = RandRange(2.0f, 5.0f);
	}
}


// ================================================
// Flash: 一瞬だけ光る閃光
//  大きな粒子が高速で広がり、すぐ消える。
//  マズルフラッシュや着弾の瞬間の光に使う。
//  寿命が極端に短い（0.05〜0.15秒）ので一瞬だけ見える。
// ================================================
void Particle::Flash(float x, float y, int count, Float4 color, bool screenSpace)
{
	if (s_Particles == nullptr) return;

	for (int n = 0; n < count; n++)
	{
		ParticleData& p = s_Particles[s_NextSlot];
		s_NextSlot = (s_NextSlot + 1) % s_MaxParticles;

		p.active = true;
		p.worldSpace = !screenSpace;
		p.x = x;
		p.y = y;

		float angle = RandRange(0.0f, 6.2831853f);
		float speed = RandRange(300.0f, 800.0f);  // 高速で飛散
		p.vx = cosf(angle) * speed;
		p.vy = sinf(angle) * speed;

		p.color   = color;
		p.life    = RandRange(0.05f, 0.15f);   // 極短寿命
		p.lifeMax = p.life;
		p.size    = RandRange(6.0f, 16.0f);    // 大きめ（光の表現）
	}
}
