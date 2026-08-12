#pragma once
#include <fstream>
#include <sstream>
#include <windows.h>
#include <timeapi.h>
#include <Tool/Logger/Logger.h>

/**
 * @enum FPSType
 * @brief FPSの種類を定義
 */
enum class FPSType {
    CPU,   ///< CPU処理のFPS
    GPU,   ///< GPU処理のFPS
    Count  ///< 種類数
};

/**
 * @class FPSObserver
 * @brief FPS（フレームレート）を監視・管理するクラス
 * 
 * CPUとGPUのフレームレートを個別に測定し、ターゲットFPSに合わせるための
 * 待機処理を行う。デルタタイムの管理とFPSの固定化を担当する。
 */
class FPSObserver {
public:

    /**
     * @brief FPS監視クラスを構築
     * @param isFix FPSを固定するかどうか
     * @param targetFps 目標FPS
     */
    FPSObserver(bool isFix = true, double targetFps = 60.0);
    
    /// @brief デストラクタ
    ~FPSObserver();

    /**
     * @brief FPS固定の有効/無効を設定
     * @param isFix FPSを固定するかどうか
     * @param type FPSの種類
     */
    void SetIsFix(bool isFix, FPSType type = FPSType::CPU) { isFix_[int(type)] = isFix; };
    
    /**
     * @brief 目標FPSを設定
     * @param targetfps 目標FPS
     * @param type FPSの種類
     */
    void SetTargetFPS(double targetfps, FPSType type = FPSType::CPU);
    
    /**
     * @brief デルタタイムを取得
     * @param type FPSの種類
     * @return デルタタイム（秒）
     */
    float GetDeltatime(FPSType type = FPSType::CPU) { return deltatime_[int(type)]; }

	float GetCPUTime() { return cpuTime_; }

    /**
     * @brief フレーム間の時間調整を行う
     * 
     * FPSを固定する場合、処理時間に余裕がある際に待機する。
     * @param type FPSの種類
     */
    void TimeAdjustment(FPSType type = FPSType::CPU);

private:
    /// @brief デルタタイム（秒）
    static std::vector<float> deltatime_;

    /// @brief 目標FPS
    std::vector<double> targetFPS_;
    /// @brief フレーム時間（秒）
    std::vector<double> frameTime_;

    /// @brief FPS固定の有効/無効
    std::vector<bool> isFix_;

    /// @brief 計測開始時間
    std::vector<LARGE_INTEGER> timeStart_{};
    /// @brief 計測終了時間  
    std::vector<LARGE_INTEGER> timeEnd_{};
    /// @brief 計測周波数
    std::vector<LARGE_INTEGER> timeFreq_{};

    //待機時間を含まないCPUの処理時間
    float cpuTime_ = 0.0f;

    /**
     * @brief 高精度な時間待機を行う
     * @param seconds 待機時間（秒）
     * @param type FPSの種類
     */
    void PreciseSleep(double seconds, FPSType type);

    /// @brief フレームカウント
	uint32_t frameCount_ = 0;
	// @brief フレームが連続で落ちた回数
	uint32_t droppedFrameCount_ = 0;

private: //ログ関連

    /// @brief ロガー
    Logger logger_;
    static inline const int recordNum_ = 16;
    static inline const int recordFrequency_ = 60;
    std::array<double, recordNum_> recordData_;
    int recordIndex_ = 0;
};
