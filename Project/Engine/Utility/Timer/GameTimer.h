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
#include <algorithm>

//============================================================================
//	GameTimer class
//	deltaTimeの取得や、時間のスケーリングを行う
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

	// ヒットストップ発生呼び出し
	static void StartHitStop(float duration, float timeScale = 0.0f);
	// スローモーション発生呼び出し
	static void StartSlowMotion(float timeScale, float duration,
		float fadeOut = 0.0f, EasingType easingType = EasingType::Linear);

	//------ measurement -----------------------------------------------------

	// 全体のフレーム時間計測
	static void BeginFrameCount();
	static void EndFrameCount();
	// 更新処理時間計測
	static void BeginUpdateCount();
	static void EndUpdateCount();
	// 描画処理時間計測
	static void BeginDrawCount();
	static void EndDrawCount();

	//--------- accessor -----------------------------------------------------

	// スケーリングされていないdeltaTimeを取得
	static float GetDeltaTime() { return deltaTime_; }
	// スケーリングされたdeltaTimeを取得
	static float GetScaledDeltaTime() { return deltaTime_ * timeScale_; }
	// 現在のtimeScaleを取得
	static float GetTimeScale() { return timeScale_; }
	
	// 起動してからの合計時間を取得
	static float GetTotalTime();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ヒットストップ情報
	struct HitStop {

		bool active = false;    // ヒットストップ中か
		float remaining = 0.0f; // 残り時間
		float scale = 0.0f;     // スケール値
	};

	// スローモーション情報
	struct SlowMotion {

		bool active = false;      // スローモーション中か
		float elapsed = 0.0f;     // 経過時間
		float duration = 0.0f;    // 持続時間
		float fadeOut = 0.0f;     // フェードアウト時間
		float startScale = 1.0f;  // 開始スケール値
		float targetScale = 1.0f; // 目標スケール値
		EasingType easing = EasingType::Linear;
	};

	// 計測情報
	struct Measurement {

		std::chrono::time_point<std::chrono::high_resolution_clock> start; // 開始時間
		std::chrono::time_point<std::chrono::high_resolution_clock> end;   // 終了時間
		std::chrono::duration<float, std::milli> resultSeconds; // 実行時間
	};

	//--------- variables ----------------------------------------------------

	// デルタタイム
	static float deltaTime_; // ゲームにおける1フレームの時間
	static float timeScale_; // 時間スケール

	// ヒットストップ情報
	static HitStop hitStop_;
	// スローモーション情報
	static SlowMotion slowMotion_;

	static std::chrono::steady_clock::time_point startTime_;
	static std::chrono::steady_clock::time_point lastFrameTime_;

	static Measurement allMeasure_;
	static Measurement updateMeasure_;
	static Measurement drawMeasure_;

	// 直近30フレームの平均を取得
	static constexpr size_t kSmoothingSample_ = 8;

	static std::vector<float> allTimes_;
	static std::vector<float> updateTimes_;
	static std::vector<float> drawTimes_;

	//--------- functions ----------------------------------------------------

	// ヒットストップ、スローモーションの更新
	static void UpdateTimeScale();

	static void AddMeasurement(std::vector<float>& buffer, float value);

	static float GetSmoothedTime(const std::vector<float>& buffer);
	static float GetSmoothedFrameTime();
	static float GetSmoothedUpdateTime();
	static float GetSmoothedDrawTime();
};