#include "GameTimer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/Easing.h>

// imgui
#include <imgui.h>

//============================================================================
//	GameTimer classMethods
//============================================================================

float GameTimer::deltaTime_ = 0.0f;
float GameTimer::timeScale_ = 1.0f;
GameTimer::HitStop GameTimer::hitStop_{};
GameTimer::SlowMotion GameTimer::slowMotion_{};

std::chrono::steady_clock::time_point GameTimer::startTime_ = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point GameTimer::lastFrameTime_ = std::chrono::steady_clock::now();
GameTimer::Measurement GameTimer::allMeasure_ = {};
GameTimer::Measurement GameTimer::updateMeasure_ = {};
GameTimer::Measurement GameTimer::drawMeasure_ = {};

std::vector<float> GameTimer::allTimes_ = {};
std::vector<float> GameTimer::updateTimes_ = {};
std::vector<float> GameTimer::drawTimes_ = {};

void GameTimer::StartHitStop(float duration, float timeScale) {

	// ヒットストップ情報セット
	hitStop_.active = true;
	hitStop_.scale = timeScale;
	hitStop_.remaining = std::max(hitStop_.remaining, duration);
}

void GameTimer::StartSlowMotion(float timeScale, float duration, float fadeOut, EasingType easingType) {

	// スローモーション情報セット
	slowMotion_.active = true;
	slowMotion_.elapsed = 0.0f;
	slowMotion_.duration = duration;
	slowMotion_.fadeOut = fadeOut;
	slowMotion_.startScale = 1.0f;
	slowMotion_.targetScale = timeScale;
	slowMotion_.easing = easingType;
}

void GameTimer::Update() {

	// ΔTime計算
	auto currentFrameTime = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsedTime = currentFrameTime - lastFrameTime_;
	deltaTime_ = elapsedTime.count();
	lastFrameTime_ = currentFrameTime;

	// ヒットストップ、スローモーションの更新
	UpdateTimeScale();
}

void GameTimer::UpdateTimeScale() {

	//============================================================================
	//	ヒットストップの時間更新
	//============================================================================

	// アクティブの時
	if (hitStop_.active) {

		hitStop_.remaining -= deltaTime_;
		// 0.0f以下になったら終了
		if (hitStop_.remaining <= 0.0f) {

			// リセット
			hitStop_.active = false;
			hitStop_.remaining = 0.0f;
		}
	}

	//============================================================================
	//	スローモーションの時間更新
	//============================================================================

	// アクティブの時
	if (slowMotion_.active) {

		slowMotion_.elapsed += deltaTime_;
		// 経過時間が持続時間+フェードアウト時間を超えたら終了
		if (slowMotion_.duration + slowMotion_.fadeOut <= slowMotion_.elapsed) {

			// 終了
			slowMotion_.active = false;
			// リセット
			slowMotion_.elapsed = 0.0f;
			slowMotion_.easing = EasingType::Linear;
		}
	}

	//============================================================================
	//	スケーリング値の更新
	//============================================================================

	float scale = 1.0f;
	if (slowMotion_.active) {

		float lerpScale = 1.0f;
		if (slowMotion_.elapsed <= slowMotion_.duration) {

			// 1.0fからtargetScaleに補間
			float lerpT = slowMotion_.elapsed / (std::max)(slowMotion_.duration, 0.0001f);
			float easedT = EasedValue(slowMotion_.easing, lerpT);
			lerpScale = std::lerp(slowMotion_.startScale, slowMotion_.targetScale, easedT);
		} else {

			// targetScaleから1.0fに補間
			float lerpT = (slowMotion_.elapsed - slowMotion_.duration) / (std::max)(slowMotion_.fadeOut, 0.0001f);
			float easedT = EasedValue(slowMotion_.easing, std::clamp(lerpT, 0.0f, 1.0f));
			lerpScale = std::lerp(slowMotion_.targetScale, 1.0f, easedT);
		}
		scale = (std::min)(scale, lerpScale);
	}

	// ヒットストップ有効時はそのスケール値を渡す
	if (hitStop_.active) {

		scale = (std::min)(scale, hitStop_.scale);
	}

	// スケールを渡す
	timeScale_ = scale;
}

void GameTimer::ImGui() {

	ImGui::SeparatorText("Performance");
	ImGui::Text("frameRate:       %.2f fps", ImGui::GetIO().Framerate); //* フレームレート情報
	ImGui::Text("deltaTime:       %.3f s", deltaTime_);                 //* ΔTime
	ImGui::Text("scaledDeltaTime: %.3f s", GetScaledDeltaTime());       //* ScaledΔTime
	ImGui::Text("totalTime:       %.3f s", GetTotalTime());             //* 合計時間

	ImGui::Text("frameTime:       %.2f ms", GetSmoothedFrameTime());  // ループにかかった時間
	ImGui::Text("updateTime:      %.2f ms", GetSmoothedUpdateTime()); // 更新処理にかかった時間
	ImGui::Text("drawTime:        %.2f ms", GetSmoothedDrawTime());   // 描画処理にかかった時間
}

void GameTimer::BeginFrameCount() {

	allMeasure_.start = std::chrono::high_resolution_clock::now();
}

void GameTimer::EndFrameCount() {

	allMeasure_.end = std::chrono::high_resolution_clock::now();
	allMeasure_.resultSeconds = allMeasure_.end - allMeasure_.start;

	AddMeasurement(allTimes_, allMeasure_.resultSeconds.count());
}

void GameTimer::BeginUpdateCount() {

	updateMeasure_.start = std::chrono::high_resolution_clock::now();
}

void GameTimer::EndUpdateCount() {

	updateMeasure_.end = std::chrono::high_resolution_clock::now();
	updateMeasure_.resultSeconds = updateMeasure_.end - updateMeasure_.start;

	AddMeasurement(updateTimes_, updateMeasure_.resultSeconds.count());
}

void GameTimer::BeginDrawCount() {

	drawMeasure_.start = std::chrono::high_resolution_clock::now();
}

void GameTimer::EndDrawCount() {

	drawMeasure_.end = std::chrono::high_resolution_clock::now();
	drawMeasure_.resultSeconds = drawMeasure_.end - drawMeasure_.start;

	AddMeasurement(drawTimes_, drawMeasure_.resultSeconds.count());
}

float GameTimer::GetTotalTime() {

	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - startTime_;
	return elapsed.count();
}

void GameTimer::AddMeasurement(std::vector<float>& buffer, float value) {

	buffer.push_back(value);
	if (buffer.size() > kSmoothingSample_) {

		// 古いデータを削除
		buffer.erase(buffer.begin());
	}
}

float GameTimer::GetSmoothedTime(const std::vector<float>& buffer) {

	if (buffer.empty()) return 0.0f;
	float sum = std::accumulate(buffer.begin(), buffer.end(), 0.0f);
	return sum / buffer.size();
}

float GameTimer::GetSmoothedFrameTime() {

	return GetSmoothedTime(allTimes_);
}

float GameTimer::GetSmoothedUpdateTime() {

	return GetSmoothedTime(updateTimes_);
}

float GameTimer::GetSmoothedDrawTime() {

	return GetSmoothedTime(drawTimes_);
}