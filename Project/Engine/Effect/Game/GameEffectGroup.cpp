#include "GameEffectGroup.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	GameEffectGroup classMethods
//============================================================================

void GameEffectGroup::Init(const std::string& name, const std::string& groupName) {

	// object作成
	objectId_ = objectManager_->CreateEffect(name, groupName);

	// data取得
	transform_ = objectManager_->GetData<EffectTransform>(objectId_);
	tag_ = objectManager_->GetData<ObjectTag>(objectId_);

	// editorに登録
	ImGuiObjectEditor::GetInstance()->Registerobject(objectId_, this);
}

void GameEffectGroup::Update() {

}

void GameEffectGroup::ImGui() {

	// 保存処理と読み込み処理
	SaveAndLoad();
}

void GameEffectGroup::SaveAndLoad() {

	// 保存処理呼び出し
	if (ImGui::Button("Save")) {

		jsonSaveState_.showPopup = true;
	}
	ImGui::SameLine();

	// 読み込んでデータを作成
	if (ImGui::Button("Load")) {

		std::string fileName;
		if (ImGuiHelper::OpenJsonDialog(fileName)) {

			LoadJson(fileName);
		}
	}
	// 保存処理
	{
		std::string fileName;
		if (ImGuiHelper::SaveJsonModal("Save CameraParam", baseJsonPath_.c_str(),
			baseJsonPath_.c_str(), jsonSaveState_, fileName)) {

			SaveJson(fileName);
		}
	}
	ImGui::Separator();
}

void GameEffectGroup::LoadJson(const std::string& fileName) {

	Json data;
	if (!JsonAdapter::LoadCheck(baseJsonPath_ + fileName, data)) {
		return;
	}

	// 読み込んだファイルからデータを作成
}

void GameEffectGroup::SaveJson(const std::string& filePath) {

	Json data;

	JsonAdapter::Save(filePath, data);
}