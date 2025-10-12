#include "ParticleSystem.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Window/WinApp.h>
#include <Engine/Effect/Particle/ParticleConfig.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

// imgui
#include <imgui.h>

//============================================================================
//	ParticleSystem classMethods
//============================================================================

void ParticleSystem::Init(ID3D12Device* device,
	Asset* asset, const std::string& name) {

	device_ = nullptr;
	device_ = device;

	asset_ = nullptr;
	asset_ = asset;

	name_ = name;

	useGame_ = false;
	allEmitEnable_ = false;
	allEmitTime_ = 1.0f;
}

void ParticleSystem::SetParent(const BaseTransform& parent) {

	for (auto& group : gpuGroups_) {

		group.group.SetParent(true, parent);
	}
	for (auto& group : cpuGroups_) {

		group.group.SetParent(true, parent);
	}
}

void ParticleSystem::Update() {

	// 全ての同時発生
	UpdateAllEmit();

	// 所持しているパーティクルの更新
	// GPU、発生処理しか行わない
	for (auto& group : gpuGroups_) {

		// ゲーム側で使用しない場合は常に
		// 一定間隔で発生させる
		if (!useGame_ && !allEmitEnable_) {

			group.group.FrequencyEmit();
		}
		group.group.Update();
	}
	// CPU
	for (auto& group : cpuGroups_) {

		// ゲーム側で使用しない場合は常に
		// 一定間隔で発生させる
		if (!useGame_ && !allEmitEnable_) {

			group.group.FrequencyEmit();
		}
		group.group.Update();
	}
}

void ParticleSystem::FrequencyEmit() {

	// 全てグループを一定間隔で発生させる
	// GPU
	for (auto& group : gpuGroups_) {

		group.group.FrequencyEmit();
	}
	// CPU
	for (auto& group : cpuGroups_) {

		group.group.FrequencyEmit();
	}
}

void ParticleSystem::Emit() {

	// 全てのグループを発生させる
	// GPU
	for (auto& group : gpuGroups_) {

		group.group.Emit();
	}
	// CPU
	for (auto& group : cpuGroups_) {

		group.group.Emit();
	}
}

void ParticleSystem::UpdateAllEmit() {

	// フラグがtrueじゃないときは処理しない
	if (!allEmitEnable_) {
		return;
	}

	// 時間経過で全て発生させる
	allEmitTimer_ += GameTimer::GetDeltaTime();
	if (allEmitTime_ < allEmitTimer_) {

		// 発生
		Emit();
		// リセット
		allEmitTimer_ = 0.0f;
	}
}

void ParticleSystem::ApplyCommand(const ParticleCommand& command) {

	// GPU
	for (auto& group : gpuGroups_) {

		group.group.ApplyCommand(command);
	}
	// CPU
	for (auto& group : cpuGroups_) {

		group.group.ApplyCommand(command);
	}
}

void ParticleSystem::AddGroup() {

	// タイプに応じて作成
	if (particleType_ == ParticleType::CPU) {

		// 追加
		auto& group = cpuGroups_.emplace_back();
		// 名前の設定
		group.name = "particle" + std::to_string(++nextGroupId_);
		// 作成
		group.group.Create(device_, asset_, primitiveType_);
	} else if (particleType_ == ParticleType::GPU) {

		// 追加
		auto& group = gpuGroups_.emplace_back();
		// 名前の設定
		group.name = "particle" + std::to_string(++nextGroupId_);
		// 作成
		group.group.Create(device_, asset_, primitiveType_);
	}
}

void ParticleSystem::RemoveGroup() {

	// 選択中のグループを削除
	if (selected_.index < 0) {
		return;
	}

	if (selected_.type == ParticleType::GPU) {

		gpuGroups_.erase(gpuGroups_.begin() + selected_.index);
	} else if (selected_.type == ParticleType::CPU) {

		cpuGroups_.erase(cpuGroups_.begin() + selected_.index);
	}
	// 未選択状態にする
	selected_.index = -1;
	renaming_.index = -1;
}

void ParticleSystem::HandleCopyPaste() {

	ImGuiIO& io = ImGui::GetIO();
	const bool ctrl = io.KeyCtrl;

	// Ctrl + Cで選択したGroupのコピー
	if (ctrl && ImGui::IsKeyPressed(ImGuiKey_C) && 0 <= selected_.index && !ImGui::IsAnyItemActive()) {

		copyGroup_.type = selected_.type;
		if (selected_.type == ParticleType::GPU) {

			copyGroup_.data = gpuGroups_[selected_.index].group.ToJson();
		} else if (selected_.type == ParticleType::CPU) {

			copyGroup_.data = cpuGroups_[selected_.index].group.ToJson();
		}
		copyGroup_.hasData = true;
	}

	// Ctrl + Vで選択したGroupのコピーを追加
	if (ctrl && ImGui::IsKeyPressed(ImGuiKey_V) && copyGroup_.hasData && !ImGui::IsAnyItemActive()) {

		// indexを進める
		++nextGroupId_;
		if (copyGroup_.type == ParticleType::GPU) {

			auto& group = gpuGroups_.emplace_back();
			group.name = "particle" + std::to_string(nextGroupId_);
			group.group.CreateFromJson(device_, asset_, copyGroup_.data);
		} else if (copyGroup_.type == ParticleType::CPU) {

			auto& group = cpuGroups_.emplace_back();
			group.name = "particle" + std::to_string(nextGroupId_);
			group.group.Create(device_, asset_, primitiveType_);
			group.group.FromJson(copyGroup_.data, asset_);
		}
		copyGroup_.hasData = false;
	}

	// 他の場所をクリックしたらコピー状態解除
	if (copyGroup_.hasData && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {

		copyGroup_.hasData = false;
	}
}

void ParticleSystem::ImGuiGroupAdd() {

	EditLayout();

	ImGui::PushItemWidth(comboWidth_);

	EnumAdapter<ParticleType>::Combo("Type", &particleType_);
	EnumAdapter<ParticlePrimitiveType>::Combo("Primitive", &primitiveType_);

	ImGui::PopItemWidth();

	// 追加と削除
	if (ImGui::Button("+Group", buttonSize_)) {

		AddGroup();
	}
	ImGui::SameLine();
	if (ImGui::Button("-Group", buttonSize_)) {

		RemoveGroup();
	}
}

void ParticleSystem::ImGuiGroupSelect() {

	// コピー&ペースト処理
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_None)) {

		HandleCopyPaste();
	}

	int id = 0;
	auto drawItem = [&](auto& vec, ParticleType type) {
		for (int i = 0; i < static_cast<int>(vec.size()); ++i, ++id) {

			const bool isSelected = (selected_.type == type && selected_.index == i);

			// 表示名
			std::string label = std::format("[{}] {}", EnumAdapter<ParticleType>::ToString(type), vec[i].name);
			ImGui::PushID(id);
			if (ImGui::Selectable(label.c_str(), isSelected,
				ImGuiSelectableFlags_AllowDoubleClick)) {

				selected_ = { type, i };
			}

			// ダブルクリックで改名開始
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {

				renaming_ = { type, i };
				strcpy_s(renameBuffer_, vec[i].name.c_str());
			}

			// 改名中
			if (renaming_.type == type && renaming_.index == i) {

				ImGui::SameLine();
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (ImGui::InputText("##rename", renameBuffer_, sizeof(renameBuffer_),
					ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue) ||
					(!ImGui::IsItemActive() && ImGui::IsItemDeactivated())) {

					vec[i].name = renameBuffer_;
					renaming_ = { ParticleType::GPU, -1 };
				}
			}
			ImGui::PopID();
		}
		};

	drawItem(gpuGroups_, ParticleType::GPU);
	drawItem(cpuGroups_, ParticleType::CPU);
}

void ParticleSystem::ImGuiSystemParameter() {

	// 保存処理
	if (ImGui::Button("Save##ps")) {
		showSavePopup_ = true;
	}

	if (showSavePopup_) {
		ImGui::OpenPopup("Save Particle");
	}

	if (ImGui::BeginPopupModal("Save Particle", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Json/Particle/%s", jsonSaveInput_);
		ImGui::InputText("##JsonFilename", jsonSaveInput_, kSaveNameSize);

		if (ImGui::Button("Save")) {
			std::string fileName = "Particle/" + std::string(jsonSaveInput_);
			if (!fileName.empty()) {
				strncpy_s(fileBuffer_, sizeof(fileBuffer_), fileName.c_str(), _TRUNCATE);
				SaveJson();
				showSavePopup_ = false;
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			showSavePopup_ = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	ImGui::SameLine();
	// 読み込み処理
	if (ImGui::Button("Load##ps")) {

		std::string relPath;
		if (ShowOpenJsonDialog(relPath)) {

			strncpy_s(fileBuffer_, sizeof(fileBuffer_), relPath.c_str(), _TRUNCATE);
			LoadJson();
		}
	}

	if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf)) {

		ImGui::Checkbox("allEmitEnable", &allEmitEnable_);

		ImGui::Text("%.3f / %.3f", allEmitTimer_, allEmitTime_);
		ImGui::DragFloat("emit", &allEmitTime_, 0.01f);
	}

	ImGui::SeparatorText("Config");

	if (!loadFileName_.empty()) {
		if (ImGui::Selectable(loadFileName_.c_str())) {

			ImGui::SetClipboardText(loadFileName_.c_str());
		}
	}
}

void ParticleSystem::ImGuiSelectedGroupEditor() {

	ImGui::PushItemWidth(itemWidth_);

	if (0 <= selected_.index) {
		if (selected_.type == ParticleType::GPU) {

			gpuGroups_[selected_.index].group.ImGui(device_);
		} else if (selected_.type == ParticleType::CPU) {

			cpuGroups_[selected_.index].group.ImGui();
		}
	}

	ImGui::PopItemWidth();
}

void ParticleSystem::EditLayout() {

	ImGui::Begin("ParticleSystem");

	ImGui::DragFloat("comboWidth_", &comboWidth_, 0.1f);
	ImGui::DragFloat("itemWidth_", &itemWidth_, 0.1f);
	ImGui::DragFloat2("buttonSize_", &buttonSize_.x, 0.1f);

	ImGui::End();
}

bool ParticleSystem::ShowOpenJsonDialog(std::string& outRelPath) {

	char szFile[MAX_PATH] = {};
	OPENFILENAMEA ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = WinApp::GetHwnd();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "JSON (*.json)\0*.json\0All\0*.*\0";

	static const std::string kInitDir =
		std::filesystem::absolute(JsonAdapter::baseDirectoryFilePath_).string();
	ofn.lpstrInitialDir = kInitDir.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn)) {

		namespace fs = std::filesystem;
		fs::path base = fs::weakly_canonical(JsonAdapter::baseDirectoryFilePath_);
		fs::path full = fs::weakly_canonical(ofn.lpstrFile);
		try {

			outRelPath = fs::relative(full, base).generic_string();
		}
		catch (...) {

			// base配下でなければファイル名のみ
			outRelPath = full.filename().generic_string();
		}
		return true;
	}
	return false;
}

void ParticleSystem::SaveJson() {

	Json data;

	//============================================================================
	//	SystemParameters
	//============================================================================

	data["primitiveType"] = EnumAdapter<ParticlePrimitiveType>::ToString(primitiveType_);
	data["name"] = name_;

	//============================================================================
	//	GroupsParameters
	//============================================================================

	for (auto& group : gpuGroups_) {

		Json groupData = group.group.ToJson();
		groupData["name"] = group.name;
		data["GPUGroups"].push_back(std::move(groupData));
	}
	for (auto& group : cpuGroups_) {

		Json groupData = group.group.ToJson();
		groupData["name"] = group.name;
		data["CPUGroups"].push_back(std::move(groupData));
	}

	std::string fileName = static_cast<std::string>(fileBuffer_);
	JsonAdapter::Save(fileName, data);
}

void ParticleSystem::LoadJson(const std::optional<std::string>& filePath, bool useGame) {

	Json data;
	loadFileName_ = static_cast<std::string>(fileBuffer_);
	if (filePath.has_value()) {

		loadFileName_ = filePath.value();
	}
	if (!JsonAdapter::LoadCheck(loadFileName_, data)) {
		return;
	}
	// リセット
	fileBuffer_[0] = '\0';
	// Particle/を削除
	loadFileName_ = Algorithm::RemoveSubstring(loadFileName_, "Particle/");

	// 設定
	useGame_ = useGame;

	//============================================================================
	//	SystemParameters
	//============================================================================

	const auto& primitiveType = EnumAdapter<ParticlePrimitiveType>::FromString(data["primitiveType"]);
	primitiveType_ = primitiveType.value();
	name_ = data.value("name", "particleSystem");

	//============================================================================
	//	GroupsParameters
	//============================================================================

	gpuGroups_.clear();
	cpuGroups_.clear();

	// GPU
	for (auto& groupData : data["GPUGroups"]) {

		auto& group = gpuGroups_.emplace_back();
		group.name = groupData.value("name", "");
		group.group.CreateFromJson(device_, asset_, groupData);
	}
	// CPU
	for (auto& groupData : data["CPUGroups"]) {

		auto& group = cpuGroups_.emplace_back();
		group.name = groupData.value("name", "");
		group.group.CreateFromJson(device_, asset_, groupData, useGame_);
	}
}