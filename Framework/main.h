#pragma once

#include "system.h"
#include "Renderer.h"
#include "Platform/PlatformDefines.h"

#define PI 3.141592f

// Switch 携帯モードのネイティブ解像度に統一（Win64 でも同じ座標系で動かすため）
#define SCREEN_WIDTH  1920	//1280
#define SCREEN_HEIGHT 1080	//720

// 目標フレームレート。更新・描画ともこのレートに固定する（事実上の固定タイムステップ）。
// メインループが「前回処理から 1/TARGET_FPS 秒経過したら 1 フレーム処理」する。
// ここを変えるだけで全体のフレームレートが変わる。
#define TARGET_FPS 60
