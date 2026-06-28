#include "GameTime.h"
#include <chrono>

// ================================================
// 内部変数
// ================================================
namespace
{
	using Clock = std::chrono::high_resolution_clock;

	Clock::time_point s_StartTime;
	Clock::time_point s_LastTime;
	float s_DeltaTime = 0.0f;
	float s_ElapsedTime = 0.0f;
}


// ================================================
// 初期化
// ================================================
void Time::Initialize()
{
	s_StartTime = Clock::now();
	s_LastTime  = s_StartTime;
	s_DeltaTime = 0.0f;
	s_ElapsedTime = 0.0f;
}


// ================================================
// 更新（毎フレーム呼ぶ）
// ================================================
void Time::Update()
{
	auto now = Clock::now();

	s_DeltaTime   = std::chrono::duration<float>(now - s_LastTime).count();
	s_ElapsedTime = std::chrono::duration<float>(now - s_StartTime).count();

	s_LastTime = now;
}


// ================================================
// 現在の経過秒（即時値）
//  Update() のキャッシュを介さず、その場で計測する。
//  メインループのフレームレート制御（経過時間ゲート）で使う。
// ================================================
float Time::NowSeconds()
{
	return std::chrono::duration<float>(Clock::now() - s_StartTime).count();
}


// ================================================
// 前フレームからの経過時間（秒）
// ================================================
float Time::GetDeltaTime()
{
	return s_DeltaTime;
}


// ================================================
// ゲーム開始からの経過時間（秒）
// ================================================
float Time::GetElapsedTime()
{
	return s_ElapsedTime;
}
