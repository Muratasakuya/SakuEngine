#include "TitleController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	TitleController classMethods
//============================================================================

void TitleController::Init() {

	// 背景初期化
	background_ = std::make_unique<TitleBackground>();
	background_->Init();

	// 選択UIナビゲーター初期化
	selectUINavigator_ = std::make_unique<GameUIFocusNavigator>();
	selectUINavigator_->Init("TitleSelectUI");

	// json適用
	ApplyJson();
}

void TitleController::Update() {

	// 背景更新
	background_->Update();

	// 入力チェック
	CheckInputFromUINavigator();

	// 選択UIナビゲーター更新
	// まだフラグなにもtrueになっていなければ入力を受け付けさせる
	selectUINavigator_->Update(!isGameStart_ && !isSelectFinish_);
}

void TitleController::CheckInputFromUINavigator() {

	// どちらかがtrueならチェックしない
	if (isGameStart_ || isSelectFinish_) {
		return;
	}

	// ゲーム開始フラグ
	if (selectUINavigator_->IsDecidedUI("StartUI")) {

		isGameStart_ = true;
	}
	// ゲーム終了フラグ
	if (selectUINavigator_->IsDecidedUI("FinishUI")) {

		isSelectFinish_ = true;
	}
}

void TitleController::ImGui() {

	ImGui::SetWindowFontScale(0.72f);

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	if (ImGui::BeginTabBar("TitleController")) {
		if (ImGui::BeginTabItem("Runtime")) {

			ImGui::Checkbox("Game Start", &isGameStart_);
			ImGui::Checkbox("Select Finish", &isSelectFinish_);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Background")) {

			background_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("SelectUI")) {

			selectUINavigator_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::SetWindowFontScale(1.0f);
}

void TitleController::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Title/titleController.json", data)) {
		return;
	}

	background_->FromJson(data["Background"]);
}

void TitleController::SaveJson() {

	Json data;

	background_->ToJson(data["Background"]);

	JsonAdapter::Save("Title/titleController.json", data);
}