
#include "main.h"
#include "controller.h"

//プロトタイプ宣言
Float3 GetControllerLeftRawAngle(); //左ジョイコンの角度生データ(1週で値は1増減する)
Float3 GetControllerRightRawAngle();//右ジョイコンの角度生データ(1週で値は1増減する)


// パッド変数
NpadJoyDualState g_OldPadState;
NpadJoyDualState g_PadState;


// 振動子変数
VibrationDeviceHandle g_VibrationDevice[2];
int g_VibrationLeftFrame;
int g_VibrationRightFrame;


// 6軸センサ変数
SixAxisSensorHandle g_SixAxisSensor[2];
SixAxisSensorState g_SixAxisSensorState[2] = {};


// タッチスクリーン変数
TouchScreenState<16> g_TouchScreenState;


//角度の基準
Float3 g_rightAngleBase;
Float3 g_leftAngleBase;


// 現在アクティブなスタイル (ホットプラグ検出用)
enum class ActiveNpadStyle { None, JoyDual, Handheld };
static ActiveNpadStyle g_ActiveStyle = ActiveNpadStyle::None;
static bool g_SixAxisStarted = false;
static bool g_NeedAngleBaseReset = false;

// 直近の Get*Handles で実際に取得できたハンドル数。
// NpadStyle やホットプラグの過渡状態によって 0/1/2 と変動するので、
// Start/Stop/Read もこの個数の範囲内だけで行う。
static int g_SixAxisHandleCount = 0;
static int g_VibrationHandleCount = 0;


// 現在有効なスタイルを判定する
static ActiveNpadStyle DetectActiveStyle()
{
	if (GetNpadStyleSet(NpadId::No1).Test<nn::hid::NpadStyleJoyDual>())
		return ActiveNpadStyle::JoyDual;
	if (GetNpadStyleSet(NpadId::Handheld).Test<nn::hid::NpadStyleHandheld>())
		return ActiveNpadStyle::Handheld;
	return ActiveNpadStyle::None;
}


// Joy-Con の装着/非装着切り替えでスタイルが変わると、
// 起動時に取った振動子・6軸センサのハンドルは古くなる。
// 毎フレーム呼んで、変化していたら取り直し＆再開させる。
//
// スタイルだけでなく「実際に取れたハンドル数」も監視する。
// 装着 → 非装着 → 装着 のような操作では、過渡フレームで片方の Joy-Con しか
// 認識されないことがあり、その状態で片方だけ Start すると、後で両方認識
// されるようになっても片方が動かないまま残るため。
static void RefreshDeviceHandlesIfChanged()
{
	ActiveNpadStyle now = DetectActiveStyle();
	if (now == ActiveNpadStyle::None) return; // どちらも未接続のフレームは何もしない

	NpadIdType id = (now == ActiveNpadStyle::JoyDual) ? NpadId::No1 : NpadId::Handheld;
	NpadStyleSet styleMask = (now == ActiveNpadStyle::JoyDual)
		? NpadStyleSet(NpadStyleJoyDual::Mask)
		: NpadStyleSet(NpadStyleHandheld::Mask);

	// 試しに新しいハンドルを取ってみて、スタイル or 個数が変化していたら反映する
	SixAxisSensorHandle newSixAxis[2] = {};
	int newSixAxisCount = GetSixAxisSensorHandles(newSixAxis, 2, id, styleMask);

	VibrationDeviceHandle newVibration[2] = {};
	int newVibrationCount = GetVibrationDeviceHandles(newVibration, 2, id, styleMask);

	bool changed = (now != g_ActiveStyle)
		|| (newSixAxisCount != g_SixAxisHandleCount)
		|| (newVibrationCount != g_VibrationHandleCount);
	if (!changed) return;

	// 既存の6軸センサを止める (有効だった数だけ)
	if (g_SixAxisStarted)
	{
		for (int i = 0; i < g_SixAxisHandleCount; i++)
		{
			StopSixAxisSensor(g_SixAxisSensor[i]);
		}
		g_SixAxisStarted = false;
	}

	// 取得結果を反映 (取れなかったスロットは空ハンドルで明示的にクリア)
	for (int i = 0; i < 2; i++)
	{
		g_SixAxisSensor[i]   = (i < newSixAxisCount)   ? newSixAxis[i]   : SixAxisSensorHandle{};
		g_VibrationDevice[i] = (i < newVibrationCount) ? newVibration[i] : VibrationDeviceHandle{};
	}
	g_SixAxisHandleCount   = newSixAxisCount;
	g_VibrationHandleCount = newVibrationCount;

	// 振動子初期化と6軸センサ開始 (実際に取れた個数だけ)
	for (int i = 0; i < g_VibrationHandleCount; i++)
	{
		InitializeVibrationDevice(g_VibrationDevice[i]);
	}
	for (int i = 0; i < g_SixAxisHandleCount; i++)
	{
		StartSixAxisSensor(g_SixAxisSensor[i]);
	}
	g_SixAxisStarted = (g_SixAxisHandleCount > 0);

	// 振動の残りフレームは古いハンドル宛だったのでキャンセル
	g_VibrationLeftFrame = 0;
	g_VibrationRightFrame = 0;

	// 新スタイルの最初のセンサ読み出し後に角度基準を取り直す
	g_NeedAngleBaseReset = true;

	g_ActiveStyle = now;
}


void InitController()
{

	// パッド初期化
	InitializeNpad();
	NpadIdType npadIds[] = { NpadId::No1,
							NpadId::Handheld };
	SetSupportedNpadStyleSet(NpadStyleJoyDual::Mask | NpadStyleHandheld::Mask);
	SetSupportedNpadIdType(npadIds, 2);

	g_VibrationLeftFrame = 0;
	g_VibrationRightFrame = 0;
	g_ActiveStyle = ActiveNpadStyle::None;
	g_SixAxisStarted = false;
	g_NeedAngleBaseReset = false;
	g_SixAxisHandleCount = 0;
	g_VibrationHandleCount = 0;

	// 振動子・6軸センサは Joy-Con の装着状態によって扱いが異なるので、
	// RefreshDeviceHandlesIfChanged() で動的に取得する。
	RefreshDeviceHandlesIfChanged();


	// タッチスクリーン初期化
	InitializeTouchScreen();

	//基準を初期化
	SetControllerLeftAngleBase();
	SetControllerRightAngleBase();

}



void UninitController()
{

}


void UpdateController()
{

	// Joy-Con の装着/非装着切り替えに追従して振動子・6軸センサのハンドルを更新する
	RefreshDeviceHandlesIfChanged();

	g_OldPadState = g_PadState;

	// パッド状態取得
	if (GetNpadStyleSet(NpadId::No1).Test<nn::hid::NpadStyleJoyDual>())
		GetNpadState(&g_PadState, NpadId::No1);
	else if (GetNpadStyleSet(NpadId::Handheld).Test<nn::hid::NpadStyleHandheld>())
		GetNpadState((NpadHandheldState*)&g_PadState, NpadId::Handheld);


	// 6軸センサ状態取得 (実際に取れたハンドル数の範囲のみ)
	if (g_SixAxisHandleCount > 0)
		GetSixAxisSensorState(&g_SixAxisSensorState[0], g_SixAxisSensor[0]);

	if (g_SixAxisHandleCount > 1)
		GetSixAxisSensorState(&g_SixAxisSensorState[1], g_SixAxisSensor[1]);
	else
		g_SixAxisSensorState[1] = g_SixAxisSensorState[0];

	// スタイル切り替え直後は新しいセンサで角度基準を取り直す
	if (g_NeedAngleBaseReset && g_SixAxisStarted)
	{
		SetControllerLeftAngleBase();
		SetControllerRightAngleBase();
		g_NeedAngleBaseReset = false;
	}


	// タッチスクリーン状態取得
	GetTouchScreenStates(&g_TouchScreenState, 1);


	// 振動子停止
	if (g_VibrationLeftFrame > 0)
	{
		g_VibrationLeftFrame--;

		if (g_VibrationLeftFrame == 0)
		{
			VibrationValue vibration = VibrationValue::Make();

			for (int i = 0; i < g_VibrationHandleCount; i++)
			{
				VibrationDeviceInfo info;
				GetVibrationDeviceInfo(&info, g_VibrationDevice[i]);
				if (info.position == VibrationDevicePosition_Left)
				{
					SendVibrationValue(g_VibrationDevice[i], vibration);
				}
			}
		}
	}

	if (g_VibrationRightFrame > 0)
	{
		g_VibrationRightFrame--;

		if (g_VibrationRightFrame == 0)
		{
			VibrationValue vibration = VibrationValue::Make();

			for (int i = 0; i < g_VibrationHandleCount; i++)
			{
				VibrationDeviceInfo info;
				GetVibrationDeviceInfo(&info, g_VibrationDevice[i]);
				if (info.position == VibrationDevicePosition_Right)
				{
					SendVibrationValue(g_VibrationDevice[i], vibration);
				}
			}
		}
	}


}




// Button::ID と NpadButton のビット位置が一致しないため変換が必要。
// NpadButton: 0=A 1=B 2=X 3=Y 4=StickL 5=StickR 6=L 7=R 8=ZL 9=ZR 10=Plus 11=Minus 12=Left 13=Up 14=Right 15=Down
static const int g_ButtonToNpadBit[Button::COUNT] = {
	0,   // A
	1,   // B
	2,   // X
	3,   // Y
	6,   // L
	7,   // R
	8,   // ZL
	9,   // ZR
	10,  // Plus
	11,  // Minus
	13,  // Up
	15,  // Down
	12,  // Left
	14,  // Right
	-1,  // MouseLeft (NX64では無効)
	-1,  // MouseRight
};

bool GetControllerPress(int button)
{
	// Key::ID (物理キー) は NX64 では無効
	if (button >= Key::OFFSET) return false;
	if (button < 0 || button >= Button::COUNT) return false;
	int bit = g_ButtonToNpadBit[button];
	if (bit < 0) return false;
	return g_PadState.buttons.Test(bit);
}

bool GetControllerTrigger(int button)
{
	if (button >= Key::OFFSET) return false;
	if (button < 0 || button >= Button::COUNT) return false;
	int bit = g_ButtonToNpadBit[button];
	if (bit < 0) return false;
	return ((g_PadState.buttons ^ g_OldPadState.buttons) & g_PadState.buttons).Test(bit);
}
bool GetControllerRelease(int button)
{
	if (button >= Key::OFFSET) return false;
	if (button < 0 || button >= Button::COUNT) return false;
	int bit = g_ButtonToNpadBit[button];
	if (bit < 0) return false;
	return ((g_PadState.buttons ^ g_OldPadState.buttons) & ~g_PadState.buttons).Test(bit);
}



Float2 GetControllerLeftStick()
{
	Float2 stick;
	stick.x = (float)g_PadState.analogStickL.x /  AnalogStickMax;
	stick.y = (float)g_PadState.analogStickL.y / -AnalogStickMax;
	return stick;
}

Float2 GetControllerRightStick()
{
	Float2 stick;
	stick.x = (float)g_PadState.analogStickR.x / AnalogStickMax;
	stick.y = (float)g_PadState.analogStickR.y / -AnalogStickMax;
	return stick;
}



//加速度===================================================

Float3 GetControllerLeftAcceleration()
{
	return g_SixAxisSensorState[0].acceleration;
}

Float3 GetControllerRightAcceleration()
{
	return g_SixAxisSensorState[1].acceleration;
}


//角度===================================================
Float3 GetControllerLeftRawAngle()
{
	return g_SixAxisSensorState[0].angle;
}

Float3 GetControllerRightRawAngle()
{
	return g_SixAxisSensorState[1].angle;
}
void SetControllerLeftAngleBase() {
	g_leftAngleBase = g_SixAxisSensorState[0].angle; //基準を初期化

}
void SetControllerRightAngleBase() {
	g_rightAngleBase = g_SixAxisSensorState[1].angle; //基準を初期化
}

Float3 GetControllerLeftAngle()
{
	Float3 now = GetControllerLeftRawAngle();
	Float3 res;
	res.x = (g_leftAngleBase.x - now.x) * 360;
	res.y = (g_leftAngleBase.y - now.y) * 360;
	res.z = (g_leftAngleBase.z - now.z) * 360;
	return res;
}

Float3 GetControllerRightAngle()
{
	Float3 now = GetControllerRightRawAngle();
	Float3 res;
	res.x = (g_rightAngleBase.x - now.x) * 360;
	res.y = (g_rightAngleBase.y - now.y) * 360;
	res.z = (g_rightAngleBase.z - now.z) * 360;
	return res;
}





//振動===================================================
void SetControllerLeftVibration(int _frame) {
	SetControllerLeftVibration(_frame, 0.0f, 10.0f, 0.5f, 320.0f);
}
void SetControllerLeftVibration(int _frame, float _power) {
	SetControllerLeftVibration(_frame, 0.0f, 10.0f, _power, 320.0f);
}
void SetControllerLeftVibration(int _frame, float _aLow, float _fLow, float aHigh, float fHigh)
{
	if (_aLow < 0)_aLow = 0;
	if (_aLow > 1)_aLow = 1;
	if (aHigh < 0)aHigh = 0;
	if (aHigh > 1)aHigh = 1;

	for (int i = 0; i < g_VibrationHandleCount; i++)
	{
		VibrationDeviceInfo info;
		GetVibrationDeviceInfo(&info, g_VibrationDevice[i]);
		if (info.position == VibrationDevicePosition_Left)
		{
			VibrationValue vibration = VibrationValue::Make(_aLow, _fLow, aHigh, fHigh);
			SendVibrationValue(g_VibrationDevice[i], vibration);
		}
	}

	g_VibrationLeftFrame = _frame;
}

void SetControllerRightVibration(int _frame) {
	SetControllerRightVibration(_frame, 0.0f, 10.0f, 0.5f, 320.0f);
}
void SetControllerRightVibration(int _frame, float _power) {
	SetControllerRightVibration(_frame, 0.0f, 10.0f, _power, 320.0f);
}
void SetControllerRightVibration(int _frame, float _aLow, float _fLow, float aHigh, float fHigh)
{
	if (_aLow < 0)_aLow = 0;
	if (_aLow > 1)_aLow = 1;
	if (aHigh < 0)aHigh = 0;
	if (aHigh > 1)aHigh = 1;

	for (int i = 0; i < g_VibrationHandleCount; i++)
	{
		VibrationDeviceInfo info;
		GetVibrationDeviceInfo(&info, g_VibrationDevice[i]);
		if (info.position == VibrationDevicePosition_Right)
		{
			VibrationValue vibration = VibrationValue::Make(_aLow, _fLow, aHigh, fHigh);
			SendVibrationValue(g_VibrationDevice[i], vibration);
		}
	}

	g_VibrationRightFrame = _frame;
}


//タッチスクリーン============================================

bool GetControllerTouchScreen()
{
	if (g_TouchScreenState.count > 0)
		return true;
	else
		return false;
}
//タッチされている数
int GetControllerTouchScreenCount() {

	return g_TouchScreenState.count;
}

Float2 GetControllerTouchScreenPosition(int _num)
{
	if (GetControllerTouchScreenCount() < _num) {
		return { 0,0 };
	}
	Float2 position;
	position.x = g_TouchScreenState.touches[_num].x * SCREEN_WIDTH / 1280.0f - SCREEN_WIDTH / 2.0f;
	position.y = g_TouchScreenState.touches[_num].y * SCREEN_HEIGHT / 720.0f - SCREEN_HEIGHT / 2.0f;

	return position;
}







