#include "GlitchUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	GlitchUpdater classMethods
//============================================================================

void GlitchUpdater::Init() {

	// 初期化値
	currentState_ = State::None;

	// json適応
	ApplyJson();
}

void GlitchUpdater::Start() {

	// 収束時間を設定
	convergenceTimer_.target_ = maxRandomCount_ * timer_.target_;

	// 全ての値をリセット
	currentCount_ = 0;
	convergenceTimer_.Reset();
	bufferData_.time = 0.0f;
	bufferData_.intensity = 0.0f;

	// Updateに設定
	currentState_ = State::Updating;
}

void GlitchUpdater::Update() {

	switch (currentState_) {
	case GlitchUpdater::State::Updating:

		// 時間を進める
		timer_.Update();
		bufferData_.time += timer_.current_;

		// 強度の範囲を0に収束させる
		convergenceTimer_.Update();
		intencityRange_ = std::lerp(startIntencityRange_, 0.0f, convergenceTimer_.t_);

		// 時間経過後リセットしてまた発生させる
		if (timer_.IsReached()) {

			// ランダムな値で強度を設定する
			bufferData_.intensity = RandomGenerator::Generate(
				-intencityRange_, intencityRange_);

			// リセットして進める
			timer_.Reset();
			++currentCount_;

			// 最大数を超えたら終了
			if (maxRandomCount_ < currentCount_) {

				// 更新しない状態にする
				bufferData_.intensity = 0.0f;
				currentState_ = State::None;
			}
		}
		break;
	}
}

void GlitchUpdater::ImGui() {

	SaveButton();

	// 値操作する状態を選択
	if (EnumAdapter<State>::Combo("State", &currentState_)) {
		if (currentState_ == State::Updating) {

			Start();
		}
	}

	ImGui::SeparatorText("Runtime");

	ImGui::Text("time:  %.3f", bufferData_.time);
	ImGui::Text("intensity:  %.3f", bufferData_.intensity);
	ImGui::Text("intencityRange:  %.3f", intencityRange_);
	ImGui::Text("currentCount:  %d / %d", currentCount_, maxRandomCount_);

	ImGui::SeparatorText("Edit");

	timer_.ImGui("Timer");
	convergenceTimer_.ImGui("ConvergenceTimer");

	ImGui::Separator();

	ImGui::DragInt("maxRandomCount", &maxRandomCount_, 1);
	ImGui::DragFloat("startIntencityRange", &startIntencityRange_, 0.01f);

	ImGui::Separator();


	ImGui::DragFloat("blockSize", &bufferData_.blockSize, 0.01f);
	ImGui::DragFloat("colorOffset", &bufferData_.colorOffset, 0.01f);
	ImGui::DragFloat("noiseStrength", &bufferData_.noiseStrength, 0.01f);
	ImGui::DragFloat("lineSpeed", &bufferData_.lineSpeed, 0.01f);
}

void GlitchUpdater::ApplyJson() {

	Json data;
	if (!LoadFile(data)) {
		return;
	}

	timer_.FromJson(data["Timer"]);
	convergenceTimer_.FromJson(data["ConvergenceTimer"]);

	maxRandomCount_ = data.value("maxRandomCount_", 8);
	startIntencityRange_ = data.value("startIntencityRange_", 8.0f);

	bufferData_.blockSize = data.value("blockSize", 32.0f);
	bufferData_.colorOffset = data.value("blockSize", 4.0f);
	bufferData_.noiseStrength = data.value("noiseStrength", 0.2f);
	bufferData_.lineSpeed = data.value("lineSpeed", 1.5f);
}

void GlitchUpdater::SaveJson() {

	Json data;

	timer_.ToJson(data["Timer"]);
	convergenceTimer_.ToJson(data["ConvergenceTimer"]);

	data["maxRandomCount_"] = maxRandomCount_;
	data["startIntencityRange_"] = startIntencityRange_;

	data["blockSize"] = bufferData_.blockSize;
	data["colorOffset"] = bufferData_.colorOffset;
	data["noiseStrength"] = bufferData_.noiseStrength;
	data["lineSpeed"] = bufferData_.lineSpeed;

	SaveFile(data);
}