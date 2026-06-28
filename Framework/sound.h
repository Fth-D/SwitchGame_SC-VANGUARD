#pragma once

#include "Platform/PlatformDefines.h"

#if PLATFORM_NX
  #include <nn/atk.h>
  #include "SoundData.fsid"
#else
  #define SE_HIT    0
  #define TITLE_BGM 1
#endif

typedef unsigned int SoundId;

// 基本関数=============================
void InitSound();
void UninitSound();
void UpdateSound();



// BGM=============================
void PlayBGM(SoundId _soundId); // 再生
void StopBGM(); // 停止
void SetBGMVolume(float _vol, int _delay=0); // ボリューム調整
	// _vol: 音量（倍率）
	// _delay: 何フレームかけてボリュームを調整するか(0〜)



// 効果音=============================
void PlaySE(SoundId _soundId); // 再生
