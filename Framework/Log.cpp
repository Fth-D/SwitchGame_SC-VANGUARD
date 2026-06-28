#include "Log.h"
#include "Text.h"
#include "GameTime.h"
#include "Platform/PlatformDefines.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>

#if PLATFORM_WIN64 || defined(USE_DIRECTX11)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif
#include <windows.h>
#endif

namespace
{
	// --- 基本設定 ---
	Log::Level s_MinLevel = Log::LV_TRACE;
	FILE* s_LogFile = nullptr;
	const char* s_Names[] = { "TRACE", "INFO", "WARN", "ERROR" };

	// --- 画面表示用リングバッファ ---
	struct ScreenMessage
	{
		char text[256];
		Log::Level level;
		float timestamp;   // Time::GetElapsedTime() at log time
		bool used;
	};

	static const int MAX_SCREEN_MESSAGES = 64;
	ScreenMessage s_Messages[MAX_SCREEN_MESSAGES];
	int s_WriteIndex = 0;
	bool s_ScreenEnabled = false;
	float s_ScreenX = 10.0f;
	float s_ScreenY = 10.0f;
	float s_FontSize = 18.0f;
	int s_MaxLines = 16;
	float s_Lifetime = 5.0f;

	// --- 出力 ---
	void Output(Log::Level level, const char* fmt, va_list args)
	{
		if (level < s_MinLevel) return;

		char buf[512];
		int off = snprintf(buf, sizeof(buf), "[%s] ", s_Names[level]);
		vsnprintf(buf + off, sizeof(buf) - off, fmt, args);
		int len = (int)strlen(buf);
		if (len > 0 && buf[len-1] != '\n') { buf[len] = '\n'; buf[len+1] = '\0'; }

#if PLATFORM_NX
		PAL_LOG("%s", buf);
#else
		printf("%s", buf);
		fflush(stdout);
#if PLATFORM_WIN64 || defined(USE_DIRECTX11)
		OutputDebugStringA(buf);
#endif
#endif
		if (s_LogFile) { fprintf(s_LogFile, "%s", buf); fflush(s_LogFile); }

		// Screen buffer (without newline)
		if (s_ScreenEnabled)
		{
			ScreenMessage& msg = s_Messages[s_WriteIndex];
			// Copy without trailing newline
			int copyLen = (int)strlen(buf);
			if (copyLen > 0 && buf[copyLen-1] == '\n') copyLen--;
			if (copyLen > 255) copyLen = 255;
			memcpy(msg.text, buf, copyLen);
			msg.text[copyLen] = '\0';
			msg.level = level;
			msg.timestamp = Time::GetElapsedTime();
			msg.used = true;
			s_WriteIndex = (s_WriteIndex + 1) % MAX_SCREEN_MESSAGES;
		}
	}
}


void Log::Initialize(const char* logFile)
{
	s_MinLevel = Log::LV_TRACE;
	s_WriteIndex = 0;
	s_ScreenEnabled = false;
	for (int i = 0; i < MAX_SCREEN_MESSAGES; i++)
		s_Messages[i].used = false;
	if (logFile) s_LogFile = fopen(logFile, "w");
}

void Log::Finalize()
{
	if (s_LogFile) { fclose(s_LogFile); s_LogFile = nullptr; }
}

void Log::SetLevel(Level minLevel) { s_MinLevel = minLevel; }
Log::Level Log::GetLevel() { return s_MinLevel; }

void Log::Trace(const char* fmt, ...)   { va_list a; va_start(a, fmt); Output(LV_TRACE, fmt, a);   va_end(a); }
void Log::Info(const char* fmt, ...)    { va_list a; va_start(a, fmt); Output(LV_INFO, fmt, a);    va_end(a); }
void Log::Warning(const char* fmt, ...) { va_list a; va_start(a, fmt); Output(LV_WARNING, fmt, a); va_end(a); }
void Log::Error(const char* fmt, ...)   { va_list a; va_start(a, fmt); Output(LV_ERROR, fmt, a);   va_end(a); }

void Log::SetScreenDisplay(bool enabled) { s_ScreenEnabled = enabled; }
void Log::SetScreenPosition(float x, float y) { s_ScreenX = x; s_ScreenY = y; }
void Log::SetScreenFontSize(float size) { s_FontSize = size; }
void Log::SetScreenLines(int maxLines) { s_MaxLines = maxLines; }
void Log::SetScreenLifetime(float seconds) { s_Lifetime = seconds; }


// ================================================
// 画面描画
//  最新のメッセージを画面右側に表示する。
//  レベルに応じた色で表示し、古いメッセージはフェードアウトする。
// ================================================
void Log::DrawScreen()
{
	if (!s_ScreenEnabled) return;

	float now = Time::GetElapsedTime();
	float fontSize = s_FontSize;
	float x = s_ScreenX;
	float y = s_ScreenY;
	int drawn = 0;

	// 新しいメッセージから遡って表示
	for (int n = 0; n < MAX_SCREEN_MESSAGES && drawn < s_MaxLines; n++)
	{
		int idx = (s_WriteIndex - 1 - n + MAX_SCREEN_MESSAGES) % MAX_SCREEN_MESSAGES;
		ScreenMessage& msg = s_Messages[idx];
		if (!msg.used) continue;

		float age = now - msg.timestamp;
		if (age > s_Lifetime) continue;

		// フェードアウト（残り1秒でアルファ減少）
		float alpha = 1.0f;
		if (age > s_Lifetime - 1.0f)
			alpha = (s_Lifetime - age);

		// レベル別の色
		float r = 1, g = 1, b = 1;
		switch (msg.level)
		{
		case LV_TRACE:   r = 0.6f; g = 0.6f; b = 0.6f; break;
		case LV_INFO:    r = 0.7f; g = 1.0f; b = 0.7f; break;
		case LV_WARNING: r = 1.0f; g = 0.9f; b = 0.3f; break;
		case LV_ERROR:   r = 1.0f; g = 0.3f; b = 0.3f; break;
		default: break;
		}

		// 背景（半透明の暗い帯）
		// ※ Polygon を使うと Log.cpp が Polygon.h に依存するので、
		//    ここではテキストのみで表示する。

		Text::SetColor(r * alpha, g * alpha, b * alpha, alpha);
		Text::Draw(msg.text, x, y + drawn * (fontSize + 2.0f), fontSize);
		drawn++;
	}
}
