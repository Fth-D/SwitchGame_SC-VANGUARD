#pragma once


#include "Platform/PlatformTypes.h"

#if PLATFORM_NX
  #include <nn/hid/hid_NpadJoy.h>
  #include <nn/hid/hid_Vibration.h>
  #include <nn/hid/hid_NpadSixAxisSensor.h>
  #include <nn/hid/hid_TouchScreen.h>
  using namespace nn::hid;
#endif

// Abstract button IDs
namespace Button {
  enum ID { A, B, X, Y, L, R, ZL, ZR, Plus, Minus, Up, Down, Left, Right, MouseLeft, MouseRight, COUNT };
}

// 物理キー識別子 (Win64用)
//   Input::Bind の第2引数として Button::ID の代わりに指定できる。
//   例: Input::Bind(Command::Jump, Key::Space);
//   値は GLFW のキーコードにオフセット 0x1000 を加算。
//   (NX64 では物理キー入力は無いので常に false を返す)
namespace Key {
  enum ID : int {
    OFFSET = 0x1000,
    Space = OFFSET + 32, Enter = OFFSET + 257, Escape = OFFSET + 256,
    Tab = OFFSET + 258, Backspace = OFFSET + 259,
    A = OFFSET + 65, B = OFFSET + 66, C = OFFSET + 67, D = OFFSET + 68,
    E = OFFSET + 69, F = OFFSET + 70, G = OFFSET + 71, H = OFFSET + 72,
    I = OFFSET + 73, J = OFFSET + 74, K = OFFSET + 75, L = OFFSET + 76,
    M = OFFSET + 77, N = OFFSET + 78, O = OFFSET + 79, P = OFFSET + 80,
    Q = OFFSET + 81, R = OFFSET + 82, S = OFFSET + 83, T = OFFSET + 84,
    U = OFFSET + 85, V = OFFSET + 86, W = OFFSET + 87, X = OFFSET + 88,
    Y = OFFSET + 89, Z = OFFSET + 90,
    Num0 = OFFSET + 48, Num1 = OFFSET + 49, Num2 = OFFSET + 50,
    Num3 = OFFSET + 51, Num4 = OFFSET + 52, Num5 = OFFSET + 53,
    Num6 = OFFSET + 54, Num7 = OFFSET + 55, Num8 = OFFSET + 56, Num9 = OFFSET + 57,
    Up = OFFSET + 265, Down = OFFSET + 264, Left = OFFSET + 263, Right = OFFSET + 262,
    LShift = OFFSET + 340, LCtrl = OFFSET + 341, LAlt = OFFSET + 342,
    RShift = OFFSET + 344, RCtrl = OFFSET + 345, RAlt = OFFSET + 346,
    F1 = OFFSET + 290, F2 = OFFSET + 291, F3 = OFFSET + 292,
    F4 = OFFSET + 293, F5 = OFFSET + 294, F6 = OFFSET + 295,
    F7 = OFFSET + 296, F8 = OFFSET + 297, F9 = OFFSET + 298,
    F10 = OFFSET + 299, F11 = OFFSET + 300, F12 = OFFSET + 301,
  };
}


//基本の関数==============================================================
void InitController();
void UninitController();
void UpdateController();

//ボタン入力・スティック傾きを取得===========================
bool GetControllerPress(int button); //press
bool GetControllerTrigger(int button); //trigger
bool GetControllerRelease(int button); //release
Float2 GetControllerLeftStick();
Float2 GetControllerRightStick();

//joy-conの振動==================================================
//↓とりあえず簡単に振動させたい場合(frameは振動させる時間(秒))
void SetControllerLeftVibration(int _frame); //左ジョイコンを振動
void SetControllerRightVibration(int _frame); //右ジョイコンを振動

//↓振動の強さを変えたい場合(powerは0〜1)で指定
void SetControllerLeftVibration(int _frame, float _power); //左ジョイコンを振動
void SetControllerRightVibration(int _frame, float _power); //右ジョイコンを振動

//↓振動の質を詳細指定したい場合
//　aLow  低帯域の振幅(0〜1)
//　fow   低帯域の周波数(40Hz〜640Hz)
//　aHigh 高帯域の振幅(0〜1)
//　fHigh 高帯域の周波数(80Hz〜1280Hz)
void SetControllerLeftVibration(int _frame, float _aLow, float _fLow, float aHigh, float fHigh); //左ジョイコンを振動
void SetControllerRightVibration(int _frame, float _aLow, float _fLow, float aHigh, float fHigh); //左ジョイコンを振動



//joy-conの加速度==================================================
Float3 GetControllerLeftAcceleration();//左ジョイコンの加速度
Float3 GetControllerRightAcceleration();//右ジョイコンの加速度



//joy-conの角度==================================================
void SetControllerLeftAngleBase(); //左ジョイコンの角度をリセット
void SetControllerRightAngleBase(); //右ジョイコンの角度をリセット
Float3 GetControllerLeftAngle(); //左ジョイコンの角度を取得
Float3 GetControllerRightAngle(); //右ジョイコンの角度を取得



//タッチスクリーン=================================================
bool GetControllerTouchScreen(); //タッチされているか否か
int GetControllerTouchScreenCount(); //タッチされている数(最大16点)
Float2 GetControllerTouchScreenPosition(int _num = 0); //タッチされている座標を取得(引数は取得したいタッチの番号)






