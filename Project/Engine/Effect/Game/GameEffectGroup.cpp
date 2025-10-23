#include "GameEffectGroup.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Effect/Particle/Core/ParticleManager.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

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

	// レイアウト設定を初期化
	ApplyLayout();
}

void GameEffectGroup::Update() {

}

void GameEffectGroup::ImGui() {

	EditLayout();

	ImGui::SetWindowFontScale(0.64f);

	if (ImGui::BeginTabBar("GameEffectGroup")) {
		if (ImGui::BeginTabItem("Info")) {

			// エフェクトの情報を表示
			DisplayInformation();
			// 保存処理と読み込み処理
			SaveAndLoad();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Transform")) {

			transform_->ImGui(itemWidth_);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Effect")) {

			// 左側の枠
			EditLeftChild();

			ImGui::SameLine();

			// 右側の枠
			EditRightChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::SetWindowFontScale(1.0f);
}

void GameEffectGroup::EditLeftChild() {

	ImGui::BeginChild("##EditLeftChild", ImVec2(leftChildWidth_, 0.0f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollbar);

	// パーティクルシステムの追加
	AddParticleSystem();

	ImGui::SeparatorText("Select");

	// パーティクルシステムの選択
	SelectParticleSystem();

	ImGui::EndChild();
}

void GameEffectGroup::AddParticleSystem() {

	// フォルダから追加するパーティクルシステムを選択する
	if (ImGui::Button("Load")) {

		std::string fileName;
		if (ImGuiHelper::OpenJsonDialog(fileName)) {

			GroupData group{};

			// システムを追加
			group.system = ParticleManager::GetInstance()->CreateParticleSystem(fileName);

			// ファイル名を取得
			std::filesystem::path pathfileName = fileName;
			group.name = pathfileName.stem().string();

			// グループ追加
			groups_.push_back(group);
		}
	}
}

void GameEffectGroup::SelectParticleSystem() {

	std::vector<std::string> groupNames{};
	for (const auto& group : groups_) {

		groupNames.emplace_back(group.name);
	}

	// グループをすべて表示
	ImGuiHelper::SelectableListFromStrings("", &selectGroupIndex_,
		groupNames, displaySystemCount_);
}

void GameEffectGroup::EditRightChild() {

	ImGui::BeginChild("##EditRightChild", ImVec2(0.0f, 0.0f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

	// グループの値操作
	EditGroup();

	ImGui::EndChild();
}

void GameEffectGroup::EditGroup() {

	// 範囲外アクセス
	if (groups_.empty() || selectGroupIndex_ < 0) {
		return;
	}

	// インデックスの範囲外制御
	selectGroupIndex_ = std::clamp(selectGroupIndex_, 0, static_cast<int>(groups_.size() - 1));

	GroupData& group = groups_[selectGroupIndex_];

	ImGui::SeparatorText(group.name.c_str());

	if (ImGui::CollapsingHeader("Runtime",
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf)) {

		ImGui::Text("emitted:   %d", group.runtime.emitted);
		ImGui::Text("timer:     %.3f", group.runtime.timer);
		ImGui::Text("emitTimer: %.3f", group.runtime.emitTimer);
		ImGui::Separator();

		ImGui::Checkbox("pending", &group.runtime.pending);
		ImGui::Checkbox("active", &group.runtime.active);
	}
	if (ImGui::CollapsingHeader("EmitSetting",
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf)) {

		EnumAdapter<EmitMode>::Combo("EmitMode", &group.emit.mode);
		ImGui::DragInt("count", &group.emit.count);
		ImGui::DragFloat("delay", &group.emit.delay, 0.01f);
		ImGui::DragFloat("interval", &group.emit.interval, 0.01f);
		ImGui::DragFloat("duration", &group.emit.duration, 0.01f);
	}
	if (ImGui::CollapsingHeader("StopSetting",
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf)) {

		EnumAdapter<StopCondition>::Combo("StopCondition", &group.stop.condition);
		ImGui::DragInt("systemIndex", &group.stop.dependency.systemIndex);
		ImGui::DragInt("groupIndex", &group.stop.dependency.groupIndex);
	}
	if (ImGui::CollapsingHeader("ModuleSetting",
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf)) {

		EnumAdapter<ParticleLifeEndMode>::Combo("LifeEndMode", &group.module.lifeEndMode);
		ImGui::DragFloat3("spawnPos", &group.module.spawnPos.x, 0.01f);
		ImGui::DragFloat3("spawnRotate", &group.module.spawnRotate.x, 0.01f);
	}
}

void GameEffectGroup::DisplayInformation() {

	ImGui::Text("name: %s", tag_->name.c_str());
	ImGui::Text("objectId: %u", objectId_);

	ImGui::Separator();
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

void GameEffectGroup::EditLayout() {

	ImGui::Begin("GameEffectGroupLayout");

	if (ImGui::Button("Save")) {

		SaveLayout();
	}

	ImGui::DragFloat("leftChildWidth", &leftChildWidth_, 0.1f);
	ImGui::DragInt("displaySystemCount_", &displaySystemCount_);

	ImGui::End();
}

void GameEffectGroup::SaveJson(const std::string& filePath) {

	Json data;

	JsonAdapter::Save(filePath, data);
}

void GameEffectGroup::ApplyLayout() {

	Json data;
	if (!JsonAdapter::LoadCheck(baseJsonPath_ + "Layout/editorLayout.json", data)) {
		return;
	}

	leftChildWidth_ = data.value("leftChildWidth_", 192.0f);
	displaySystemCount_ = data.value("displaySystemCount_", 16);
}

void GameEffectGroup::SaveLayout() {

	Json data;

	data["leftChildWidth_"] = leftChildWidth_;
	data["displaySystemCount_"] = displaySystemCount_;

	JsonAdapter::Save(baseJsonPath_ + "Layout/editorLayout.json", data);
}