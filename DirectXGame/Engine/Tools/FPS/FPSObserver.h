#pragma once
#include <fstream>
#include <sstream>
#include <windows.h>
#include <timeapi.h>
#include <Logger/Logger.h>

class FPSObserver {
public:

    /// <summary>
    /// fpsを管理するクラス
    /// </summary>
    /// <param name="isfix">fpsを固定するかどうか</param>
    /// <param name="targetfps">固定するときの目標fps</param>
    FPSObserver(bool isfix = true, double targetfps = 60.0);
    ~FPSObserver();

    void SetIsFix(bool isfix) { isFix_ = isfix; };
    void SetTargetFPS(double targetfps);
    float GetDeltatime() { return deltatime_; }

    // FPSを固定するため処理時間に余裕がある場合その分待つ
    void TimeAdjustment();

private:
    static float deltatime_;

    double targetFPS_;
    double frameTime_;

    static bool isFix_;

    // 計測開始時間
    LARGE_INTEGER timeStart_{};
    // 計測終了時間  
    LARGE_INTEGER timeEnd_{};
    // 計測周波数
    LARGE_INTEGER timeFreq_{};

    // より精密な時間待機
    void PreciseSleep(double seconds);

    std::shared_ptr<spdlog::logger> logger_;
};
