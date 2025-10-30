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

	// json適用
	ApplyJson();
}

void TitleController::Update() {

	background_->Update();
}

void TitleController::ImGui() {

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
		ImGui::EndTabBar();
	}
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
