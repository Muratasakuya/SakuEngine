#include "GrayscaleUpdater.h"

//============================================================================
//	GrayscaleUpdater classMethods
//============================================================================

void GrayscaleUpdater::Init() {

	// 最初の状態
	currentState_ = State::None;
	// グレー値0.0fで初期化
	bufferData_.rate = 0.0f;

	// json適応
	ApplyJson();
}

void GrayscaleUpdater::Update() {

	// 状態に応じて更新
	switch (currentState_) {
	case GrayscaleUpdater::State::None:
		break;
	case GrayscaleUpdater::State::Updating:

		// 補間更新
		rateAnimation_.LerpValue(bufferData_.rate);

		// 補間終了したら次の状態へ
		if (rateAnimation_.IsFinished()) {

			currentState_ = State::WaitGray;
		}
		break;
	case GrayscaleUpdater::State::WaitGray:

		// 時間を更新
		waitGrayTimer_.Update(std::nullopt, false);

		// 時間経過後グレースケールを元に戻す
		if (waitGrayTimer_.IsReached()) {

			currentState_ = State::Return;

			// アニメーションを逆補間
			rateAnimation_.Reset();
			rateAnimation_.Start();
			rateAnimation_.SetAnimationType(SimpleAnimationType::Return);
		}
		break;
	case GrayscaleUpdater::State::Return:

		// 補間更新
		rateAnimation_.LerpValue(bufferData_.rate);

		// 補間終了したら処理を終了する
		if (rateAnimation_.IsFinished()) {

			currentState_ = State::None;
		}
		break;
	case GrayscaleUpdater::State::Constant:

		// グレースケールを固定
		bufferData_.rate = 1.0f;
		break;
	}
}

void GrayscaleUpdater::Start() {

	// リセット
	rateAnimation_.Reset();
	waitGrayTimer_.Reset();

	// 補間処理を開始させる
	rateAnimation_.Start();
	rateAnimation_.SetAnimationType(SimpleAnimationType::None);
	currentState_ = State::Updating;
}

void GrayscaleUpdater::ImGui() {

	SaveButton();
	ImGui::Separator();

	if (ImGui::Button("Start")) {

		Start();
	}

	rateAnimation_.ImGui("Rate Animation", false);
	waitGrayTimer_.ImGui("WaitGrayTimer", true);
}

void GrayscaleUpdater::ApplyJson() {

	Json data;
	if (!LoadFile(data)) {
		return;
	}

	rateAnimation_.FromJson(data["RateAnimation"]);
	waitGrayTimer_.FromJson(data["WaitGrayTimer"]);
}

void GrayscaleUpdater::SaveJson() {

	Json data;

	rateAnimation_.ToJson(data["RateAnimation"]);
	waitGrayTimer_.ToJson(data["WaitGrayTimer"]);

	SaveFile(data);
}