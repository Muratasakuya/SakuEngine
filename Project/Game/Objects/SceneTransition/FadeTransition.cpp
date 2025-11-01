#include "FadeTransition.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FadeTransition classMethods
//============================================================================

void FadeTransition::Init() {

	fadeSprite_ = std::make_unique<GameObject2D>();
	fadeSprite_->Init("white", "transitionSprite", "Scene");
	fadeSprite_->SetPostProcessMask(Bit_CRTDisplay);
	// シーンが切り替わっても破棄しない
	fadeSprite_->SetDestroyOnLoad(false);
	fadeSprite_->SetSpriteLayerIndex(SpriteLayerIndex::SceneTransition, 0);

	// fade初期設定
	fadeSprite_->SetCenterTranslation();
	fadeSprite_->SetSize(Vector2(Config::kWindowWidthf, Config::kWindowHeightf));
	fadeSprite_->SetColor(Color::Convert(0x04040400));

	loadSprite_ = std::make_unique<GameObject2D>();
	loadSprite_->Init("nowLoading", "nowLoading", "Scene");
	// シーンが切り替わっても破棄しない
	loadSprite_->SetDestroyOnLoad(false);
	loadSprite_->SetSpriteLayerIndex(SpriteLayerIndex::SceneTransition, 1);
	// 初期設定
	loadSprite_->SetCenterTranslation();
	loadSprite_->SetAlpha(0.0f);

	// json適応
	ApplyJson();
}

void FadeTransition::Update() {}

void FadeTransition::BeginUpdate() {

	beginTimer_.Update();
	fadeSprite_->SetAlpha(std::lerp(0.0f, 1.0f, beginTimer_.easedT_));

	if (beginTimer_.IsReached()) {

		fadeSprite_->SetAlpha(1.0f);

		// 次に進める
		state_ = TransitionState::Load;
		beginTimer_.Reset();
	}
}

void FadeTransition::LoadUpdate() {

	loadSprite_->SetAlpha(1.0f);

	// 読み込み完了後次に進む
	if (loadingFinished_) {

		state_ = TransitionState::LoadEnd;
	}
}

void FadeTransition::LoadEndUpdate() {

	waitTimer_.Update();
	loadSprite_->SetAlpha(1.0f);
	if (waitTimer_.IsReached()) {

		// 次に進める
		state_ = TransitionState::End;
		waitTimer_.Reset();
	}
}

void FadeTransition::EndUpdate() {

	endTimer_.Update();
	fadeSprite_->SetAlpha(std::lerp(1.0f, 0.0f, endTimer_.easedT_));
	loadSprite_->SetAlpha(0.0f);
	if (endTimer_.IsReached()) {

		fadeSprite_->SetAlpha(0.0f);

		// 遷移終了
		state_ = TransitionState::Begin;
		endTimer_.Reset();
		loadingFinished_ = false;
	}
}

void FadeTransition::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	beginTimer_.ImGui("Begin");
	waitTimer_.ImGui("Wait");
	endTimer_.ImGui("End");
}

void FadeTransition::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Scene/Transition/fadeTransition.json", data)) {
		return;
	}

	beginTimer_.FromJson(data["beginTimer_"]);
	waitTimer_.FromJson(data["waitTimer_"]);
	endTimer_.FromJson(data["endTimer_"]);
}

void FadeTransition::SaveJson() {

	Json data;

	beginTimer_.ToJson(data["beginTimer_"]);
	waitTimer_.ToJson(data["waitTimer_"]);
	endTimer_.ToJson(data["endTimer_"]);

	JsonAdapter::Save("Scene/Transition/fadeTransition.json", data);
}