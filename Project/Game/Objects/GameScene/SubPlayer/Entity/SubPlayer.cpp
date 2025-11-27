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

	// 手の親は体
	rightHand_->SetParent(body_->GetTransform());
	leftHand_->SetParent(body_->GetTransform());

	// 影無効(1.0fで影は映らなくなる)
	body_->SetShadowRate(1.0f);
	rightHand_->SetShadowRate(1.0f);
	leftHand_->SetShadowRate(1.0f);
}

void SubPlayer::InitState() {

	// 状態管理クラスの初期化
	stateController_ = std::make_unique<SubPlayerStateController>();
	stateController_->Init();
	// 各パーツを状態管理クラスに設定
	stateController_->SetParts(body_.get(), rightHand_.get(), leftHand_.get());
}

void SubPlayer::SetPartsTransform(GameObject3D* parts, const Transform3D& transform) {

	// SRTを設定
	parts->SetTranslation(transform.translation);
	parts->SetRotation(transform.rotation);
	parts->SetScale(transform.scale);
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

	// 状態の更新
	stateController_->Update();
}

void SubPlayer::ImGui() {

	if (ImGui::Button("Save Json")) {
		SaveJson();
	}

	if (ImGui::BeginTabBar("SubPlayer")) {
		if (ImGui::BeginTabItem("Parts")) {

			// 体
			if (ImGui::CollapsingHeader("Body")) {
				if (initBodyTransform_.ImGui(itemWidth_)) {

					SetPartsTransform(body_.get(), initBodyTransform_);
				}
			}
			// 右手
			if (ImGui::CollapsingHeader("RightHand")) {
				if (initRightHandTransform_.ImGui(itemWidth_)) {

					SetPartsTransform(rightHand_.get(), initRightHandTransform_);
				}
			}
			// 左手
			if (ImGui::CollapsingHeader("LeftHand")) {
				if (initLeftHandTransform_.ImGui(itemWidth_)) {

					SetPartsTransform(leftHand_.get(), initLeftHandTransform_);
				}
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("State")) {

			ImGui::SetWindowFontScale(0.8f);

			stateController_->ImGui();
			ImGui::SetWindowFontScale(1.0f);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void SubPlayer::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("SubPlayer/mainParameter.json", data)) {
		return;
	}

	// トランスフォーム
	{
		// パーツ別の初期トランスフォーム
		initBodyTransform_.FromJson(data["InitBodyTransform"]);
		initRightHandTransform_.FromJson(data["InitRightHandTransform"]);
		initLeftHandTransform_.FromJson(data["InitLeftHandTransform"]);

		// 各パーツにトランスフォームをセット
		SetPartsTransform(body_.get(), initBodyTransform_);
		SetPartsTransform(rightHand_.get(), initRightHandTransform_);
		SetPartsTransform(leftHand_.get(), initLeftHandTransform_);
	}
}

void SubPlayer::SaveJson() {

	Json data;

	// パーツ別の初期トランスフォーム
	initBodyTransform_.ToJson(data["InitBodyTransform"]);
	initRightHandTransform_.ToJson(data["InitRightHandTransform"]);
	initLeftHandTransform_.ToJson(data["InitLeftHandTransform"]);

	JsonAdapter::Save("SubPlayer/mainParameter.json", data);
}