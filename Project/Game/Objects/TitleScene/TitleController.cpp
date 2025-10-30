#include "TitleController.h"

//============================================================================
//	TitleController classMethods
//============================================================================

void TitleController::Init() {

	background_ = std::make_unique<GameObject2D>();
	background_->Init("white", "background", "Title");
	background_->SetCenterTranslation();
	background_->SetWindowSize();
	background_->SetPostProcessMask(Bit_CRTDisplay);

	titleName_ = std::make_unique<GameObject2D>();
	titleName_->Init("titleName", "titleName", "Title");
	titleName_->SetCenterTranslation();
	titleName_->SetPostProcessMask(Bit_CRTDisplay);
}

void TitleController::Update() {

}

void TitleController::ImGui() {

	ImGui::Checkbox("Game Start", &isGameStart_);
	ImGui::Checkbox("Select Finish", &isSelectFinish_);
}