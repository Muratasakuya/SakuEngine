#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/Easing.h>

// c++
#include <cstdint>
#include <chrono>
#include <vector>
#include <numeric>

//============================================================================
//	GameTimer class
//============================================================================
class GameTimer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameTimer() = default;
	~GameTimer() = default;

	static void Update();

	static void ImGui();

	static void BeginFrameCount();
	static void EndFrameCount();

	static void BeginUpdateCount();
	static void EndUpdateCount();

	static void BeginDrawCount();
	static void EndDrawCount();

	//--------- accessor -----------------------------------------------------

	static void SetTimeScale(float timeScale, EasingType easing = EasingType::EaseOutExpo);
	static void SetWaitTime(float waitTime, bool isReset = true);
	static void SetLerpSpeed(float lerpSpeed) { lerpSpeed_ = lerpSpeed; }
	static void SetReturnScaleEnable(bool enable) { returnScaleEnable_ = enable; }

	static float GetDeltaTime() { return deltaTime_; }
	static float GetScaledDeltaTime() { return deltaTime_ * timeScale_; }

	static float GetTotalTime();
	static float GetTimeScale() { return timeScale_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	struct Measurement {

		std::chrono::time_point<std::chrono::high_resolution_clock> start; // 開始時間
		std::chrono::time_point<std::chrono::high_resolution_clock> end;   // 終了時間
		std::chrono::duration<float, std::milli> resultSeconds; // 実行時間
	};

	//--------- variables ----------------------------------------------------

	static std::chrono::steady_clock::time_point startTime_;
	static std::chrono::steady_clock::time_point lastFrameTime_;

	static Measurement allMeasure_;
	static Measurement updateMeasure_;
	static Measurement drawMeasure_;

	static float deltaTime_;

	static float timeScale_;
	static EasingType easing_;

	static float lerpSpeed_;

	static float waitTimer_;
	static float waitTime_;

	static bool returnScaleEnable_;

	// 直近30フレームの平均を取得
	static constexpr size_t kSmoothingSample_ = 8;

	static std::vector<float> allTimes_;
	static std::vector<float> updateTimes_;
	static std::vector<float> drawTimes_;

	//--------- functions ----------------------------------------------------

	static void AddMeasurement(std::vector<float>& buffer, float value);

	static float GetSmoothedTime(const std::vector<float>& buffer);
	static float GetSmoothedFrameTime();
	static float GetSmoothedUpdateTime();
	static float GetSmoothedDrawTime();
};