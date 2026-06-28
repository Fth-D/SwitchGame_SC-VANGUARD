// ================================================
// Input.cpp
// コマンドベース入力システムの実装
//
// ゲームコマンド（Confirm, Attack 等）を物理ボタンに対応づける。
// 内部では controller.h の関数を呼び出している。
//
// デフォルトのキーバインドはプラットフォームごとに設定される。
// NX64: Joy-Con のボタン配置に準拠
// Win64: キーボードの一般的なゲーム操作に準拠
//
// Bind() を使えば実行時にバインドを変更できる（キーコンフィグ）。
// ================================================

#include "Input.h"
#include "controller.h"
#include "Platform/PlatformDefines.h"

namespace
{
	// 1コマンドにつき最大4つまでボタン/キーを割り当てられる
	constexpr int MAX_BINDINGS_PER_COMMAND = 4;

	// コマンドごとのボタン割り当て（2次元配列）
	int s_Bindings[Command::COUNT][MAX_BINDINGS_PER_COMMAND];
	int s_BindingCount[Command::COUNT] = {};  // 各コマンドに登録された個数

	// リピート用の状態
	float s_HoldTime[Command::COUNT] = {};
	float s_RepeatTimer[Command::COUNT] = {};
	bool  s_RepeatEnabled[Command::COUNT] = {};  // コマンド単位のON/OFF
	bool  s_RepeatGlobal   = true;   // 一括ON/OFF
	float s_RepeatDelay    = 0.4f;   // 長押しからリピート開始までの時間
	float s_RepeatInterval = 0.5f;  // リピートの間隔


	// ================================================
	// デフォルトバインドを設定
	// ================================================
	void SetDefaultBindings()
	{
		// --- 両プラットフォーム共通の初期値 ---
		//
		// controller.h の Button::ID を使って割り当てる。
		// Button::ID は内部で物理キーに変換される。
		//   NX64:  Button::A → Joy-Con A ボタン
		//   Win64: Button::A → Enter キー
		//
		// プラットフォームごとに変えたい場合は #if で分岐する。
		//
		// Input::Bind() は「追加」セマンティクスなので、複数回呼べばサブキーが増える。

		Input::Bind(Command::Confirm,   Button::A);
		Input::Bind(Command::Cancel,    Button::B);
		Input::Bind(Command::Pause,     Button::Plus);
		Input::Bind(Command::ShoulderL, Button::L);
		Input::Bind(Command::ShoulderR, Button::R);

		// 攻撃・サブアクションはプラットフォームで変えたい例
#if PLATFORM_NX
		Input::Bind(Command::Attack,    Button::Y);
		Input::Bind(Command::SubAction, Button::X);
#else
		Input::Bind(Command::Attack,    Button::MouseLeft);   // Win64: left click
		Input::Bind(Command::Cancel,    Button::MouseRight);  // Win64: right click
		Input::Bind(Command::SubAction, Button::X);           // Win64: Z
#endif
	}
}


// ================================================
// 初期化
// ================================================
void Input::Initialize()
{
	// バインドを一旦すべて空にしてからデフォルト設定を流し込む
	for (int i = 0; i < Command::COUNT; i++) s_BindingCount[i] = 0;

	SetDefaultBindings();

	for (int i = 0; i < Command::COUNT; i++)
	{
		s_HoldTime[i] = 0.0f;
		s_RepeatTimer[i] = 0.0f;
		s_RepeatEnabled[i] = true;
	}
	s_RepeatGlobal = true;
}


// ================================================
// 更新（毎フレーム呼ぶ）
// ================================================
void Input::Update(float dt)
{
	// 各コマンドの長押し時間を追跡（割り当てられたボタンのいずれかが押されていればOK）
	for (int i = 0; i < Command::COUNT; i++)
	{
		bool anyPressed = false;
		for (int k = 0; k < s_BindingCount[i]; k++)
		{
			if (GetControllerPress(s_Bindings[i][k])) { anyPressed = true; break; }
		}

		if (anyPressed)
		{
			s_HoldTime[i] += dt;
			s_RepeatTimer[i] += dt;
		}
		else
		{
			s_HoldTime[i] = 0.0f;
			s_RepeatTimer[i] = 0.0f;
		}
	}
}


// ================================================
// 入力判定
// ================================================
bool Input::IsPress(int command)
{
	if (command < 0 || command >= Command::COUNT) return false;
	for (int k = 0; k < s_BindingCount[command]; k++)
	{
		if (GetControllerPress(s_Bindings[command][k])) return true;
	}
	return false;
}

bool Input::IsTrigger(int command)
{
	if (command < 0 || command >= Command::COUNT) return false;
	for (int k = 0; k < s_BindingCount[command]; k++)
	{
		if (GetControllerTrigger(s_Bindings[command][k])) return true;
	}
	return false;
}

bool Input::IsRelease(int command)
{
	if (command < 0 || command >= Command::COUNT) return false;
	for (int k = 0; k < s_BindingCount[command]; k++)
	{
		if (GetControllerRelease(s_Bindings[command][k])) return true;
	}
	return false;
}


// ================================================
// 軸入力の取得
//  Command::Axis で指定した軸の値を返す（-1.0〜1.0）
//  内部でコントローラのスティックやキーボードに対応する。
// ================================================
float Input::GetAxis(int axis)
{
	switch (axis)
	{
	case Command::MoveX: return GetControllerLeftStick().x;
	case Command::MoveY: return GetControllerLeftStick().y;
	case Command::LookX: return GetControllerRightStick().x;
	case Command::LookY: return GetControllerRightStick().y;
	default: return 0.0f;
	}
}


// ================================================
// キーバインド追加
//  command: Command::ID
//  button:  Button::ID（controller.h） または Key::ID
//  既存のバインドはそのままに、もう1つ追加する。
//  1コマンドにつき最大 MAX_BINDINGS_PER_COMMAND 個までで、
//  すでに上限まで埋まっている場合は無視される。
// ================================================
void Input::Bind(int command, int button)
{
	if (command < 0 || command >= Command::COUNT) return;
	// Key::ID (0x1000以上) または Button::ID (0〜Button::COUNT-1) のみ受け付ける
	if (button < Key::OFFSET && (button < 0 || button >= Button::COUNT)) return;
	// 上限まで埋まっていれば追加しない
	if (s_BindingCount[command] >= MAX_BINDINGS_PER_COMMAND) return;

	s_Bindings[command][s_BindingCount[command]] = button;
	s_BindingCount[command]++;
}

// ================================================
// 指定コマンドのバインドをすべて解除
//  キーコンフィグで張り直すときに使う。
// ================================================
void Input::ClearBindings(int command)
{
	if (command < 0 || command >= Command::COUNT) return;
	s_BindingCount[command] = 0;
}


// ================================================
// キーリピート
//  押した瞬間（Trigger）で true。
//  そのまま押し続けると、RepeatDelay 経過後に
//  RepeatInterval ごとに true を返す。
// ================================================
bool Input::IsRepeat(int command)
{
	if (command < 0 || command >= Command::COUNT) return false;

	// 押した瞬間は常に返す
	if (IsTrigger(command))
	{
		s_RepeatTimer[command] = 0.0f;
		return true;
	}

	// リピートが無効なら Trigger のみ
	if (!s_RepeatGlobal || !s_RepeatEnabled[command])
		return false;

	// 長押し中: 初期遅延を超えたらリピート
	if (s_HoldTime[command] >= s_RepeatDelay &&
		s_RepeatTimer[command] >= s_RepeatInterval)
	{
		s_RepeatTimer[command] -= s_RepeatInterval;
		return true;
	}

	return false;
}


// ================================================
// リピート設定
// ================================================
bool Input::IsRepeating(int command)
{
	if (command < 0 || command >= Command::COUNT) return false;
	return s_HoldTime[command] >= s_RepeatDelay;
}

void Input::SetRepeatEnabled(bool enabled)
{
	s_RepeatGlobal = enabled;
}

void Input::SetRepeatEnabled(int command, bool enabled)
{
	if (command >= 0 && command < Command::COUNT)
		s_RepeatEnabled[command] = enabled;
}

void Input::SetRepeatDelay(float seconds)
{
	if (seconds >= 0.0f) s_RepeatDelay = seconds;
}

void Input::SetRepeatInterval(float seconds)
{
	if (seconds > 0.0f) s_RepeatInterval = seconds;
}
