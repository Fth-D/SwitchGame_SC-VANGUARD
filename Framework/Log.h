#pragma once

// ================================================
// ログシステム + 画面表示
//  コンソール・ファイル出力に加え、画面上にログを表示できる。
//  Text描画と連携して、デバッグ情報をリアルタイムで確認できる。
//
//  使い方:
//    Log::Info("HP: %d", hp);
//    Log::SetScreenDisplay(true);   // 画面表示ON
//    Log::SetLevel(Log::LV_INFO);   // TRACE以下を非表示
// ================================================
namespace Log
{
	enum Level
	{
		LV_TRACE,
		LV_INFO,
		LV_WARNING,
		LV_ERROR,
		LV_NONE
	};

	void Initialize(const char* logFile = nullptr);
	void Finalize();

	void SetLevel(Level minLevel);
	Level GetLevel();

	void Trace(const char* fmt, ...);
	void Info(const char* fmt, ...);
	void Warning(const char* fmt, ...);
	void Error(const char* fmt, ...);

	// --- 画面表示 ---
	void SetScreenDisplay(bool enabled);    // 画面上のログ表示ON/OFF
	void SetScreenPosition(float x, float y); // 表示位置（デフォルト: 10, 10 = 画面左上）
	void SetScreenFontSize(float size);     // フォントサイズ（デフォルト: 18）
	void SetScreenLines(int maxLines);      // 表示行数（デフォルト: 16）
	void SetScreenLifetime(float seconds);  // メッセージ表示時間（デフォルト: 5秒）
	void DrawScreen();                      // Game::Drawから呼ぶ
}
