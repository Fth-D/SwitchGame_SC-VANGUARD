
#include <nn/atk.h>
#include <../Samples/Include/nns/atk/atk_SampleCommon.h>

#include "main.h"
#include "sound.h"



namespace
{
    const char ArchiveRelativePath[] = "SoundData.bfsar";

    const int SoundHeapSize = 4 * 1024 * 1024;

    nn::atk::SoundHeap          g_SoundHeap;
    nn::atk::FsSoundArchive     g_SoundArchive;
    nn::atk::SoundArchivePlayer g_SoundArchivePlayer;
    nn::atk::SoundDataManager   g_SoundDataManager;

    nn::audio::MemoryPoolType   g_MemoryPool;

    void* g_pMemoryForSoundSystem;
    void* g_pMemoryForSoundHeap;
    void* g_pMemoryForInfoBlock;
    void* g_pMemoryForSoundDataManager;
    void* g_pMemoryForSoundArchivePlayer;
    void* g_pMemoryForStreamBuffer;

    nn::atk::SoundHandle        g_SoundHandleBGM;
    nn::atk::SoundHandle        g_SoundHandleSE;
}



void InitSound()
{

 	// SoundSystem初期化
	{
		nns::atk::InitializeHeap();
		nns::atk::InitializeFileSystem();


		bool isSuccess = true;

		nn::atk::SoundSystem::SoundSystemParam param;
		std::size_t memSizeForSoundSystem = nn::atk::SoundSystem::GetRequiredMemSize(param);
		g_pMemoryForSoundSystem = nns::atk::Allocate(memSizeForSoundSystem, nn::atk::SoundSystem::WorkMemoryAlignSize);
		isSuccess = nn::atk::SoundSystem::Initialize(
			param,
			reinterpret_cast<uintptr_t>(g_pMemoryForSoundSystem),
			memSizeForSoundSystem);
		NN_ABORT_UNLESS(isSuccess, "cannot initialize SoundSystem");

		// SoundHeap の初期化
		g_pMemoryForSoundHeap = nns::atk::Allocate(SoundHeapSize);
		isSuccess = g_SoundHeap.Create(g_pMemoryForSoundHeap, SoundHeapSize);
		NN_ABORT_UNLESS(isSuccess, "cannot create SoundHeap");

		// SoundArchive の初期化
		const char* archiveAbsolutePath = nns::atk::GetAbsolutePath(ArchiveRelativePath);
		isSuccess = g_SoundArchive.Open(archiveAbsolutePath);
		NN_ABORT_UNLESS(isSuccess, "cannot open SoundArchive(%s)\n", archiveAbsolutePath);

		// SoundArchive のパラメータ情報をメモリにロード
		std::size_t infoBlockSize = g_SoundArchive.GetHeaderSize();
		g_pMemoryForInfoBlock = nns::atk::Allocate(infoBlockSize, nn::atk::FsSoundArchive::BufferAlignSize);
		isSuccess = g_SoundArchive.LoadHeader(g_pMemoryForInfoBlock, infoBlockSize);
		NN_ABORT_UNLESS(isSuccess, "cannot load InfoBlock");

		// SoundDataManager の初期化
		std::size_t memSizeForSoundDataManager = g_SoundDataManager.GetRequiredMemSize(&g_SoundArchive);
		g_pMemoryForSoundDataManager = nns::atk::Allocate(memSizeForSoundDataManager, nn::atk::SoundDataManager::BufferAlignSize);
		isSuccess = g_SoundDataManager.Initialize(
			&g_SoundArchive,
			g_pMemoryForSoundDataManager,
			memSizeForSoundDataManager);
		NN_ABORT_UNLESS(isSuccess, "cannot initialize SoundDataManager");

		// SoundArchivePlayer で用いるストリームバッファの初期化
		// ストリームバッファはメモリプール管理されているヒープから確保する必要があります。
		std::size_t memSizeForStreamBuffer = g_SoundArchivePlayer.GetRequiredStreamBufferSize(&g_SoundArchive);
		g_pMemoryForStreamBuffer = nns::atk::AllocateForMemoryPool(memSizeForStreamBuffer);

		// 専用のヒープをメモリプールにアタッチ
		nn::atk::SoundSystem::AttachMemoryPool(&g_MemoryPool, nns::atk::GetPoolHeapAddress(), nns::atk::GetPoolHeapSize());

		// SoundArchivePlayer の初期化
		std::size_t memSizeForSoundArchivePlayer = g_SoundArchivePlayer.GetRequiredMemSize(&g_SoundArchive);
		g_pMemoryForSoundArchivePlayer = nns::atk::Allocate(memSizeForSoundArchivePlayer, nn::atk::SoundArchivePlayer::BufferAlignSize);
		isSuccess = g_SoundArchivePlayer.Initialize(
			&g_SoundArchive,
			&g_SoundDataManager,
			g_pMemoryForSoundArchivePlayer, memSizeForSoundArchivePlayer,
			g_pMemoryForStreamBuffer, memSizeForStreamBuffer);
		NN_ABORT_UNLESS(isSuccess, "cannot initialize SoundArchivePlayer");
	}



	// SoundData読み込み
	{
		bool isSuccess = true;
		isSuccess = g_SoundDataManager.LoadData(SE_HIT, &g_SoundHeap);
		NN_ABORT_UNLESS(isSuccess, "LoadData failed.");

		/*
		isSuccess = g_SoundDataManager.LoadData(WSD_DECISION24, &g_SoundHeap);
		NN_ABORT_UNLESS(isSuccess, "LoadData failed.");

		isSuccess = g_SoundDataManager.LoadData(WSD_STONE_BREAK1, &g_SoundHeap);
		NN_ABORT_UNLESS(isSuccess, "LoadData failed.");

		isSuccess = g_SoundDataManager.LoadData(WSD_WATER_DROP1, &g_SoundHeap);
		NN_ABORT_UNLESS(isSuccess, "LoadData failed.");

		isSuccess = g_SoundDataManager.LoadData(WSD_WATER_DROP2, &g_SoundHeap);
		NN_ABORT_UNLESS(isSuccess, "LoadData failed.");
		*/
	}

}



void UninitSound()
{
    g_SoundArchivePlayer.Finalize();

    // 専用のヒープをメモリプールからデタッチ
    nn::atk::SoundSystem::DetachMemoryPool(&g_MemoryPool);

    g_SoundDataManager.Finalize();
    g_SoundArchive.Close();
    g_SoundHeap.Destroy();
    nn::atk::SoundSystem::Finalize();

    nns::atk::FreeForMemoryPool(g_pMemoryForStreamBuffer);
    nns::atk::Free(g_pMemoryForSoundArchivePlayer);
    nns::atk::Free(g_pMemoryForSoundDataManager);
    nns::atk::Free(g_pMemoryForInfoBlock);
    nns::atk::Free(g_pMemoryForSoundHeap);
    nns::atk::Free(g_pMemoryForSoundSystem);


	nns::atk::FinalizeFileSystem();
	nns::atk::FinalizeHeap();

}


void UpdateSound()
{
    g_SoundArchivePlayer.Update();
}




void PlayBGM(nn::atk::SoundArchive::ItemId _soundId)
{
	g_SoundArchivePlayer.StartSound(&g_SoundHandleBGM, _soundId);
}

void StopBGM()
{
	g_SoundHandleBGM.Stop(0);
}

void SetBGMVolume(float _vol, int _delay)
{
	if (_vol < 0)_vol = 0;
	g_SoundHandleBGM.SetVolume(_vol, _delay);
}

void PlaySE(nn::atk::SoundArchive::ItemId _soundId)
{
	g_SoundArchivePlayer.StartSound(&g_SoundHandleSE, _soundId);
}

