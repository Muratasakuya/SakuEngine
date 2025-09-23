#include "FadeSprite.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Utility/EnumAdapter.h>
#include <Engine/Utility/JsonAdapter.h>

//============================================================================
//	FadeSprite classMethods
//============================================================================

void FadeSprite::DerivedInit() {

	// 初期設定
	SetCenterTranslation();
	SetSize(Vector2(Config::kWindowWidthf, Config::kWindowHeightf));
	SetColor(Color::Convert(0x02020200));

	// json適応
	ApplyJson();
	Reset();
}

bool FadeSprite::IsFinished() const {

	// フラグがたっていれば遷移させない
	if (disableTransition_) {
		return false;
	}
	return currentState_ == State::Wait;
}

void FadeSprite::Update() {

	switch (currentState_) {
	case FadeSprite::State::None:

		// 全てのタイマーをリセット
		beginTimer_.Reset();
		waitTimer_.Reset();
		endTimer_.Reset();
		break;
	case FadeSprite::State::Begin:

		beginTimer_.Update();
		material_->material.color.a = std::lerp(0.0f, 1.0f, beginTimer_.easedT_);
		if (beginTimer_.IsReached()) {

			material_->material.color.a = 1.0f;
			currentState_ = State::Wait;
		}
		break;
	case FadeSprite::State::Wait:

		waitTimer_.Update();
		if (waitTimer_.IsReached()) {

			currentState_ = State::End;
		}
		break;
	case FadeSprite::State::End:

		endTimer_.Update();
		material_->material.color.a = std::lerp(1.0f, 0.0f, endTimer_.easedT_);
		if (endTimer_.IsReached()) {

			material_->material.color.a = 0.0f;
			currentState_ = State::None;
		}
		break;
	}
}

void FadeSprite::Start() {

	if (currentState_ != State::None) {
		return;
	}

	// 更新開始
	currentState_ = State::Begin;
}

void FadeSprite::Reset() {

	// 状態を最初に戻す
	currentState_ = State::None;
}

void FadeSprite::DerivedImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	EnumAdapter<State>::Combo("currentState", &currentState_);
	ImGui::Checkbox("disableTransition", &disableTransition_);

	beginTimer_.ImGui("Begin");
	waitTimer_.ImGui("Wait");
	endTimer_.ImGui("End");
}

void FadeSprite::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("SpriteEffect/fadeSprite.json", data)) {
		return;
	}

	beginTimer_.FromJson(data["beginTimer_"]);
	waitTimer_.FromJson(data["waitTimer_"]);
	endTimer_.FromJson(data["endTimer_"]);
}

void FadeSprite::SaveJson() {

	Json data;

	beginTimer_.ToJson(data["beginTimer_"]);
	waitTimer_.ToJson(data["waitTimer_"]);
	endTimer_.ToJson(data["endTimer_"]);

	JsonAdapter::Save("SpriteEffect/fadeSprite.json", data);
}