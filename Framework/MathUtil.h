#pragma once

#include <random>
#include <chrono>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <algorithm>
#include "Platform/PlatformTypes.h"

/**
 * 球面線形補間による確率抽選関数
 * @param current 第一引数（現在値）
 * @param target 第二引数（目標値）
 * @return 当選した場合true、そうでなければfalse
 */
static bool SlerpProbabilityDraw(double current, double target)
{
    // 第一引数が第二引数以上の場合は強制的にtrue
    if (current >= target)
    {
        return true;
    }

    // 静的な乱数生成器（関数呼び出し間で状態を保持）
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);

    // 差分を計算（必ず正の値）
    double difference = target - current;

    // 球面線形補間による確率計算
    // 差分が0に近いほど確率が1.0（100%）に近づき、
    // 差分が大きくなるほど0.1（10%）に漸近する

    // 指数的減衰パラメータ（調整可能）
    constexpr double decayRate = 1.0;

    // 基本的な指数減衰: e^(-difference * decayRate)
    double baseDecay = std::exp(-difference * decayRate);

    // 球面線形補間のための角度計算
    // baseDecayを角度空間でさらに滑らかに補間
    double angle = baseDecay * (M_PI / 2.0);  // 0〜π/2の範囲

    // sin関数による滑らかな補間（球面線形補間の近似）
    double smoothFactor = std::sin(angle);

    // 最終確率: 10% + (滑らかな係数 * 90%)
    constexpr double minProbability = 0.1;
    constexpr double maxProbabilityRange = 0.9;
    double probability = minProbability + (smoothFactor * maxProbabilityRange);

    // 0から1の範囲で乱数を生成して抽選
    double randomValue = dist(rng);

    return randomValue <= probability;
}

/**
 * 指定範囲内のfloat乱数生成関数
 * @param minValue 最小値（この値を含む）
 * @param maxValue 最大値（この値を含む）
 * @return 指定範囲内のfloat値
 */
static float RandomFloat(float minValue, float maxValue)
{
    // 引数の順序チェック：minValue > maxValueの場合は入れ替え
    if (minValue > maxValue)
    {
        std::swap(minValue, maxValue);
    }

    // 静的な乱数生成器（関数呼び出し間で状態を保持）
    static thread_local std::mt19937 rng(std::random_device{}());

    // float型用の均等分布生成器
    static thread_local std::uniform_real_distribution<float> dist;

    // 分布パラメータを動的に設定
    typename std::uniform_real_distribution<float>::param_type params(minValue, maxValue);

    // 指定範囲内の乱数を生成
    return dist(rng, params);
}

/**
 * オーバーロード版：引数1つの場合は0.0f〜指定値の範囲
 * @param maxValue 最大値（0.0f〜maxValueの範囲）
 * @return 0.0f以上maxValue以下のfloat値
 */
static float RandomFloat(float maxValue)
{
    return RandomFloat(0.0f, maxValue);
}

/**
 * 引数なし版：0.0f〜1.0fの範囲
 * @return 0.0f以上1.0f以下のfloat値
 */
static float RandomFloat()
{
    return RandomFloat(0.0f, 1.0f);
}

/**
 * 時計回り2次元単位ベクトル生成関数
 * 関数内で時間を計測し、10秒で1周する時計回りの単位ベクトルを返す
 * @return 時計回りに回転する2次元単位ベクトル（Float2型）
 */
static Float2 GetClockwiseUnitVector()
{
    // 高精度時間計測用の静的変数
    static std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    // 現在時刻を取得
    std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

    // 経過時間を秒単位で計算
    std::chrono::duration<double> elapsedDuration = currentTime - startTime;
    double elapsedSeconds = elapsedDuration.count();

    // 10秒で1周するため、周期で正規化
    constexpr double rotationPeriod = 10.0;
    double normalizedTime = fmod(elapsedSeconds, rotationPeriod);

    // 時計回りの角度計算（0秒で上向き、時計回りに回転）
    // 通常の数学座標系では反時計回りが正なので、角度を反転
    double angle = -2.0 * M_PI * (normalizedTime / rotationPeriod);

    // 時計回りの単位ベクトル計算
    // 0秒時点で上向き（0, 1）から開始するため、π/2オフセット
    double adjustedAngle = angle + (M_PI / 2.0);

    float vectorX = static_cast<float>(cos(adjustedAngle));
    float vectorY = static_cast<float>(sin(adjustedAngle));

    // 浮動小数点誤差による単位ベクトルからの微小なずれを手動で正規化
    float magnitude = sqrt(vectorX * vectorX + vectorY * vectorY);
    if (magnitude > 0.0f)
    {
        vectorX /= magnitude;
        vectorY /= magnitude;
    }

    return MakeFloat2(vectorX, vectorY);
}