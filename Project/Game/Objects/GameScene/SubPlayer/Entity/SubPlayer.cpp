#include "SubPlayer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	SubPlayer classMethods
//============================================================================

void SubPlayer::InitParts() {

	// 各パーツの初期化
	// 体
	body_ = std::make_unique<GameObject3D>();
	body_->Init("subPlayerBody", "subPlayerBody", "SubPlayer");
	// 右手
	rightHand_ = std::make_unique<GameObject3D>();
	rightHand_->Init("subPlayerHand", "subPlayerRightHand", "SubPlayer");
	// 左手
	leftHand_ = std::make_unique<GameObject3D>();
	leftHand_->Init("subPlayerHand", "subPlayerLeftHand", "SubPlayer");
}

void SubPlayer::InitState() {

	// 状態管理クラスの初期化
	stateController_ = std::make_unique<SubPlayerStateController>();
	stateController_->Init();
	// 各パーツを状態管理クラスに設定
	stateController_->SetParts(body_.get(), rightHand_.get(), leftHand_.get());
}

void SubPlayer::Init() {

	// 各パーツの初期化
	InitParts();
	// 状態管理の初期化
	InitState();

	// json適用
	ApplyJson();
}

void SubPlayer::Update() {


}

void SubPlayer::ImGui() {

	if (ImGui::Button("Save Json")) {
		SaveJson();
	}
}

void SubPlayer::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("SubPlayer/mainParameter.json", data)) {
		return;
	}
}

void SubPlayer::SaveJson() {

	Json data;

	JsonAdapter::Save("SubPlayer/mainParameter.json", data);
}