#include "GameAPI.h"
#include "Platform/PlatformDefines.h"

//エントリポイント（ここから始まる）
#if PLATFORM_NX
extern "C" void nnMain()
#else
int main(int argc, char* argv[])
#endif
{
	Initialize();  //初期化

	Process();	//ゲームループ

	Finalize();  //後片付け

#if PLATFORM_NX
	return;
#else
	return 0;
#endif
}