// ================================================
// sound.Win64.cpp
// XAudio2 によるサウンド実装（Win64）
//
// NX64 では nn::atk を使うが、Win64 では XAudio2 を使用する。
// sound.h のインターフェースは共通。
// ================================================

#include "sound.h"
#include "../PlatformDefines.h"
#include <windows.h>
#include <xaudio2.h>
#include <cstdio>
#include <cstring>

#pragma comment(lib, "xaudio2.lib")


// ================================================
// 内部定義
// ================================================
namespace
{
	// --- WAVデータ ---
	struct WavData
	{
		WAVEFORMATEX format;
		BYTE* data;
		DWORD dataSize;
	};

	// --- XAudio2 ---
	IXAudio2* g_XAudio2 = nullptr;
	IXAudio2MasteringVoice* g_MasterVoice = nullptr;

	// --- BGM ---
	IXAudio2SourceVoice* g_BGMVoice = nullptr;

	// --- SE ボイスプール ---
	static const int MAX_SE_VOICES = 16;
	IXAudio2SourceVoice* g_SEVoices[MAX_SE_VOICES] = {};

	// --- サウンドデータ ---
	static const int MAX_SOUNDS = 16;
	WavData g_SoundData[MAX_SOUNDS] = {};

	// SoundId → WAVファイルパス
	// NX64 では SoundData.fsid が ID を定義する。
	// Win64 では sound.h の #define に対応するパスをここで定義する。
	const char* g_SoundPaths[MAX_SOUNDS] = {
		"SoundMakerProject/asset/SE/hit0.wav",      // SE_HIT    = 0
		"SoundMakerProject/asset/BGM/sample000.wav", // TITLE_BGM = 1
	};


	// ================================================
	// WAVファイル読み込み
	// ================================================
	bool LoadWav(const char* path, WavData* out)
	{
		FILE* fp = fopen(path, "rb");
		if (!fp)
		{
			printf("Sound: Failed to open %s\n", path);
			return false;
		}

		// RIFFヘッダ確認
		char riff[4], wave[4];
		DWORD fileSize;
		fread(riff, 1, 4, fp);
		fread(&fileSize, 4, 1, fp);
		fread(wave, 1, 4, fp);

		if (memcmp(riff, "RIFF", 4) != 0 || memcmp(wave, "WAVE", 4) != 0)
		{
			printf("Sound: Not a valid WAV file: %s\n", path);
			fclose(fp);
			return false;
		}

		// fmt / data チャンクを探す
		bool foundFmt = false, foundData = false;
		memset(&out->format, 0, sizeof(out->format));
		out->data = nullptr;
		out->dataSize = 0;

		while (!feof(fp))
		{
			char chunkId[4];
			DWORD chunkSize;
			if (fread(chunkId, 1, 4, fp) != 4) break;
			if (fread(&chunkSize, 4, 1, fp) != 1) break;

			if (memcmp(chunkId, "fmt ", 4) == 0)
			{
				// fmt チャンク（PCM: 16バイト）
				DWORD readSize = (chunkSize < sizeof(WAVEFORMATEX)) ? chunkSize : sizeof(WAVEFORMATEX);
				fread(&out->format, 1, readSize, fp);
				if (chunkSize > readSize)
					fseek(fp, chunkSize - readSize, SEEK_CUR);
				foundFmt = true;
			}
			else if (memcmp(chunkId, "data", 4) == 0)
			{
				// data チャンク
				out->dataSize = chunkSize;
				out->data = new BYTE[chunkSize];
				fread(out->data, 1, chunkSize, fp);
				foundData = true;
			}
			else
			{
				// 未知のチャンクはスキップ
				fseek(fp, chunkSize, SEEK_CUR);
			}

			if (foundFmt && foundData) break;
		}

		fclose(fp);

		if (!foundFmt || !foundData)
		{
			printf("Sound: Incomplete WAV file: %s\n", path);
			delete[] out->data;
			out->data = nullptr;
			return false;
		}

		return true;
	}


	// ================================================
	// 再生済みSEボイスを解放（ボイスプール管理）
	// ================================================
	int FindFreeSESlot()
	{
		for (int i = 0; i < MAX_SE_VOICES; i++)
		{
			if (g_SEVoices[i] == nullptr)
				return i;

			// 再生が終わったボイスを回収
			XAUDIO2_VOICE_STATE state;
			g_SEVoices[i]->GetState(&state);
			if (state.BuffersQueued == 0)
			{
				g_SEVoices[i]->DestroyVoice();
				g_SEVoices[i] = nullptr;
				return i;
			}
		}
		return -1;  // 全スロット使用中
	}
}


// ================================================
// 初期化
// ================================================
void InitSound()
{
	// COM 初期化（XAudio2 に必要）
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// XAudio2 インスタンス作成
	HRESULT hr = XAudio2Create(&g_XAudio2);
	if (FAILED(hr))
	{
		printf("XAudio2Create failed: 0x%lx\n", hr);
		return;
	}

	// マスタリングボイス作成（最終出力先）
	hr = g_XAudio2->CreateMasteringVoice(&g_MasterVoice);
	if (FAILED(hr))
	{
		printf("CreateMasteringVoice failed\n");
		return;
	}

	// WAVファイルをプリロード
	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		if (g_SoundPaths[i] != nullptr)
		{
			LoadWav(g_SoundPaths[i], &g_SoundData[i]);
		}
	}

	printf("Sound: XAudio2 initialized\n");
}


// ================================================
// 終了処理
// ================================================
void UninitSound()
{
	// BGM停止
	StopBGM();

	// SEボイスを全て破棄
	for (int i = 0; i < MAX_SE_VOICES; i++)
	{
		if (g_SEVoices[i])
		{
			g_SEVoices[i]->Stop();
			g_SEVoices[i]->DestroyVoice();
			g_SEVoices[i] = nullptr;
		}
	}

	// サウンドデータ解放
	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		delete[] g_SoundData[i].data;
		g_SoundData[i].data = nullptr;
	}

	// XAudio2 解放
	if (g_MasterVoice)
	{
		g_MasterVoice->DestroyVoice();
		g_MasterVoice = nullptr;
	}
	if (g_XAudio2)
	{
		g_XAudio2->Release();
		g_XAudio2 = nullptr;
	}

	CoUninitialize();
}


// ================================================
// 更新（XAudio2は内部でスレッド管理するため最小限）
// ================================================
void UpdateSound()
{
}


// ================================================
// BGM 再生（ループ）
// ================================================
void PlayBGM(SoundId soundId)
{
	if (!g_XAudio2 || soundId >= MAX_SOUNDS || !g_SoundData[soundId].data)
		return;

	// 既存のBGMを停止
	StopBGM();

	WavData& wav = g_SoundData[soundId];

	// ソースボイス作成
	HRESULT hr = g_XAudio2->CreateSourceVoice(&g_BGMVoice, &wav.format);
	if (FAILED(hr) || !g_BGMVoice) return;

	// バッファ設定（ループ再生）
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = wav.dataSize;
	buffer.pAudioData = wav.data;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

	g_BGMVoice->SubmitSourceBuffer(&buffer);
	g_BGMVoice->Start();
}


// ================================================
// BGM 停止
// ================================================
void StopBGM()
{
	if (g_BGMVoice)
	{
		g_BGMVoice->Stop();
		g_BGMVoice->DestroyVoice();
		g_BGMVoice = nullptr;
	}
}


// ================================================
// BGM 音量設定
// ================================================
void SetBGMVolume(float vol, int delay)
{
	(void)delay;  // Win64版では即時反映
	if (g_BGMVoice)
	{
		if (vol < 0.0f) vol = 0.0f;
		g_BGMVoice->SetVolume(vol);
	}
}


// ================================================
// SE 再生（ワンショット）
//  ボイスプールから空きスロットを取得して再生する。
//  同時再生数は MAX_SE_VOICES まで。
// ================================================
void PlaySE(SoundId soundId)
{
	if (!g_XAudio2 || soundId >= MAX_SOUNDS || !g_SoundData[soundId].data)
		return;

	// 空きスロットを探す
	int slot = FindFreeSESlot();
	if (slot < 0) return;

	WavData& wav = g_SoundData[soundId];

	// ソースボイス作成
	HRESULT hr = g_XAudio2->CreateSourceVoice(&g_SEVoices[slot], &wav.format);
	if (FAILED(hr) || !g_SEVoices[slot]) return;

	// バッファ設定（1回再生）
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = wav.dataSize;
	buffer.pAudioData = wav.data;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	g_SEVoices[slot]->SubmitSourceBuffer(&buffer);
	g_SEVoices[slot]->Start();
}
