#include "TitleController.h"

//============================================================================
//	TitleController classMethods
//============================================================================

void TitleController::Init() {


}

void TitleController::Update() {


}

void TitleController::ImGui() {

	ImGui::Checkbox("Game Start", &isGameStart_);
	ImGui::Checkbox("Select Finish", &isSelectFinish_);
}