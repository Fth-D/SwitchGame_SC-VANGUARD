#pragma once
#include <cmath>
#include "Platform/PlatformTypes.h"

//======================================================================================//
// UIEffects（UI共通エフェクト）														//
//--------------------------------------------------------------------------------------//
// テキストや数値表示など、複数のUI要素で使い回す小さな計算関数をまとめたもの。			//
// GameObjectやTextに依存しない、純粋な計算関数のみ（値を渡して値を受け取るだけ）。		//
//======================================================================================//

float GetFadeAlpha(float timer, float speed);
Float2 GetShakeOffset(float intensity);
Float3 LerpColor(Float3 colorA, Float3 colorB, float t);

// グリッチ・フリッカー：通常は1.0を返すが、稀にランダムで低輝度になる（信号不安定な演出）
float GetGlitchFlicker(float timer, float glitchChance);

// 数値の残像表示用：前回の値からどれだけ時間が経ったかを見て、フェードアウトする透明度を返す
float GetAfterimageAlpha(float timeSinceChange, float fadeDuration);