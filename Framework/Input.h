#pragma once
#include "Platform/PlatformTypes.h"

// ================================================
// コマンド定義
//  ゲーム内で使用する操作の一覧。
//  ゲームコードでは物理キー（AボタンやEnterキー）ではなく、
//  コマンド（Confirm, Attack 等）で入力を判定する。
//  どのキーがどのコマンドに対応するかは、Input が内部で管理する。
//
//  この仕組みはゲーム業界で「アクションマッピング」と呼ばれ、
//  キーコンフィグ対応やプラットフォーム間の差異吸収に使われる。
// ================================================
namespace Command
{
	enum ID
	{
		// --- ボタンコマンド ---
		Confirm,        // 決定
		Cancel,         // キャンセル
		Attack,         // 攻撃
		SubAction,      // サブアクション
		Pause,          // ポーズ / メニュー
		ShoulderL,      // Lボタン相当
		ShoulderR,      // Rボタン相当
		Jump,
		NewCommand,		// 新しいコマンドを追加する例
		COUNT
	};

	// --- 軸コマンド ---
	enum Axis
	{
		MoveX,          // 移動 横方向（-1.0〜1.0）
		MoveY,          // 移動 縦方向（-1.0〜1.0）
		LookX,          // 視点 横方向（-1.0〜1.0）
		LookY,          // 視点 縦方向（-1.0〜1.0）

		AXIS_COUNT
	};
}

// ================================================
// 入力インターフェース
//  ゲームコードはこの namespace の関数だけを使って入力を取得する。
//  物理キーの違い（NX/Win64）を意識する必要はない。
//
//  使い方:
//    if (Input::Trigger(Command::Confirm))  // 決定ボタンが押された瞬間
//    if (Input::Press(Command::Attack))     // 攻撃ボタンを押し続けている
//    float x = Input::GetAxis(Command::MoveX); // 軸入力（-1.0〜1.0）
// ================================================
namespace Input
{
	// --- システム ---
	void Initialize();
	void Update(float dt);

	// --- 入力判定 ---
	bool IsPress(int command);       // 押している間 true
	bool IsTrigger(int command);     // 押した瞬間だけ true
	bool IsRelease(int command);     // 離した瞬間だけ true
	bool IsRepeat(int command);      // Trigger + 長押しでリピート
	bool IsRepeating(int command);   // リピート状態に入っているか

	// --- 軸入力 ---
	float GetAxis(int axis);   // 軸コマンドの値（-1.0〜1.0）

	// --- キーバインド ---
	//  指定コマンドに、ボタン/キーを「追加」で割り当てる。
	//  1コマンドにつき最大 4 個まで保持できる（超過分は無視）。
	//  button は controller.h の Button::ID または Key::ID を指定する。
	void Bind(int command, int button);

	//  指定コマンドのバインドをすべて解除する（キーコンフィグで張り直すときに使う）。
	void ClearBindings(int command);

	// --- リピート設定 ---
	void SetRepeatEnabled(bool enabled);             // 全コマンドのリピートON/OFF
	void SetRepeatEnabled(int command, bool enabled); // 特定コマンドのリピートON/OFF
	void SetRepeatDelay(float seconds);               // 長押しからリピート開始までの時間（デフォルト 0.4秒）
	void SetRepeatInterval(float seconds);            // リピートの間隔（デフォルト 0.05秒）
}
