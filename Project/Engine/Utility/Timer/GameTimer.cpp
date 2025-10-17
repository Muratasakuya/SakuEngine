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

std::chrono::steady_clock::time_point GameTimer::startTime_ = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point GameTimer::lastFrameTime_ = std::chrono::steady_clock::now();
GameTimer::Measurement GameTimer::allMeasure_ = {};
GameTimer::Measurement GameTimer::updateMeasure_ = {};
GameTimer::Measurement GameTimer::drawMeasure_ = {};

std::vector<float> GameTimer::allTimes_ = {};
std::vector<float> GameTimer::updateTimes_ = {};
std::vector<float> GameTimer::drawTimes_ = {};

float GameTimer::deltaTime_ = 0.0f;
float GameTimer::timeScale_ = 1.0f;
EasingType GameTimer::easing_ = EasingType::EaseOutExpo;
float GameTimer::lerpSpeed_ = 8.0f;
float GameTimer::waitTimer_ = 0.0f;
float GameTimer::waitTime_ = 0.08f;
bool GameTimer::returnScaleEnable_ = true;

void GameTimer::Update() {

	auto currentFrameTime = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsedTime = currentFrameTime - lastFrameTime_;
	deltaTime_ = elapsedTime.count();

	lastFrameTime_ = currentFrameTime;

	if (returnScaleEnable_) {
		// timeScaleを1.0fに戻す処理
		if (timeScale_ != 1.0f) {

			// 硬直させる
			waitTimer_ += deltaTime_;
			if (waitTimer_ >= waitTime_) {

				float t = lerpSpeed_ * deltaTime_;
				float easedT = EasedValue(easing_, t);

				timeScale_ += (1.0f - timeScale_) * easedT;
				if (std::fabs(1.0f - timeScale_) < 0.01f) {
					timeScale_ = 1.0f;

					waitTimer_ = 0.0f;
				}
			}
		}
	}
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

void GameTimer::SetTimeScale(float timeScale, EasingType easing) {

	timeScale_ = timeScale;
	easing_ = easing;
}

void GameTimer::SetWaitTime(float waitTime, bool isReset) {

	waitTime_ = waitTime;
	waitTimer_ = isReset ? 0.0f : waitTimer_;
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