#include "FPSObserver.h"
#pragma comment(lib,"winmm.lib")
#include <imgui/imgui.h>

std::vector<float> FPSObserver::deltatime_;

FPSObserver::FPSObserver(bool isFix, double targetFps) {
    for(int i =0; i < static_cast<int>(FPSType::Count); ++i) {
        timeFreq_.emplace_back();
        timeStart_.emplace_back();
        timeEnd_.emplace_back();
        // 周波数取得
        QueryPerformanceFrequency(&timeFreq_[i]);
        // 計測開始時間の初期化
        QueryPerformanceCounter(&timeStart_[i]);
	}

	targetFPS_.resize(static_cast<int>(FPSType::Count), targetFps);
	frameTime_.resize(static_cast<int>(FPSType::Count), 1.0 / targetFps);
	deltatime_.resize(static_cast<int>(FPSType::Count), static_cast<float>(1.0 / targetFps));
	isFix_.resize(static_cast<int>(FPSType::Count), isFix);

    // 高精度タイマーの設定
    timeBeginPeriod(1);

    logger_ = getLogger("FPS", LoggerFlag::UseDebugString);
}

FPSObserver::~FPSObserver() {
    // 高精度タイマーの解除
    timeEndPeriod(1);
}

void FPSObserver::SetTargetFPS(double targetFps, FPSType type) {
    targetFPS_[int(type)] = targetFps;
	frameTime_[int(type)] = 1.0 / targetFps;
}

void FPSObserver::TimeAdjustment(FPSType type) {
	LARGE_INTEGER& timeStart = timeStart_[int(type)];
    LARGE_INTEGER timeEnd{};
	LARGE_INTEGER& timeFreq = timeFreq_[int(type)];
    // フレーム終了時間を取得
    QueryPerformanceCounter(&timeEnd);

    // 経過時間を計算（秒単位）
    double frameTime = static_cast<double>(timeEnd.QuadPart - timeStart.QuadPart) /
        static_cast<double>(timeFreq.QuadPart);

    if (isFix_[int(type)]) {

        // 目標フレーム時間まで待機
        if (frameTime < frameTime_[int(type)]) {
            double remainingTime = frameTime_[int(type)] - frameTime;
            PreciseSleep(remainingTime, type);

            // 待機後の実際の時間を再測定
            QueryPerformanceCounter(&timeEnd);
            frameTime = static_cast<double>(timeEnd.QuadPart - timeStart.QuadPart) /
                static_cast<double>(timeFreq.QuadPart);
        }

    }

    deltatime_[int(type)] = static_cast<float>(frameTime);
    frameCount_++;

    if (deltatime_[int(type)] > 20.0 / targetFPS_[int(type)]) {
		droppedFrameCount_++;
        if (droppedFrameCount_ >= 5) {
            logger_->warn("FPS Drop Detected: frameCount = {}  nowFPS = {:.2f}  TargetFPS = {:.2f}", frameCount_, 1.0f / deltatime_[int(type)], float(targetFPS_[int(type)]));
        }
    }

    if (deltatime_[int(type)] > 0.1f) {
        deltatime_[int(type)] = 0.1f; // 最大値を設定して極端な値を防ぐ
	}

    // 次のフレームの開始時間を設定
    timeStart = timeEnd;
}

void FPSObserver::PreciseSleep(double seconds, FPSType type) {
    if (seconds <= 0.0) return;

    LARGE_INTEGER sleepStart, sleepCurrent;
    QueryPerformanceCounter(&sleepStart);

    double targetTicks = seconds * static_cast<double>(timeFreq_[int(type)].QuadPart);

    // 粗い待機
    if (seconds > 0.002) {
        DWORD sleepMs = static_cast<DWORD>((seconds - 0.001) * 1000.0);
        Sleep(sleepMs);
    }

    // 精密待機
    do {
        QueryPerformanceCounter(&sleepCurrent);
    } while ((sleepCurrent.QuadPart - sleepStart.QuadPart) < targetTicks);
}
