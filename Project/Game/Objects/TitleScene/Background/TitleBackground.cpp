#include "TitleBackground.h"

//============================================================================
//	TitleBackground classMethods
//============================================================================

void TitleBackground::Init() {

	// 使用するスプライトの初期化

	// 背景
	background_ = std::make_unique<GameObject2D>();
	background_->Init("white", "background", "Title");

	// タイトルの名前
	titleName_ = std::make_unique<GameObject2D>();
	titleName_->Init("titleName", "titleName", "Title");
}

void TitleBackground::Update() {


}

void TitleBackground::ImGui() {

	if (ImGui::BeginTabBar("TitleBackground")) {
		if (ImGui::BeginTabItem("Sprite")) {
			if (ImGui::CollapsingHeader("Background")) {

				background_->ImGui();
			}
			if (ImGui::CollapsingHeader("TitleName")) {

				titleName_->ImGui();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void TitleBackground::FromJson(const Json& json) {

	background_->ApplyJson(json["BackgroundSprite"]);
	titleName_->ApplyJson(json["TitleNameSprite"]);
}

void TitleBackground::ToJson(Json& json) const {

	background_->SaveJson(json["BackgroundSprite"]);
	titleName_->SaveJson(json["TitleNameSprite"]);
}