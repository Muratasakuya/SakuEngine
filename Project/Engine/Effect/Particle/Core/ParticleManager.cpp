#include "ParticleManager.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Effect/Particle/ParticleConfig.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Utility/Timer/GameTimer.h>

// modules
// Spawner
#include <Engine/Effect/Particle/Module/ParticleModuleRegistry.h>
#include <Engine/Effect/Particle/Module/Spawner/ParticleSpawnSphereModule.h>
#include <Engine/Effect/Particle/Module/Spawner/ParticleSpawnHemisphereModule.h>
#include <Engine/Effect/Particle/Module/Spawner/ParticleSpawnBoxModule.h>
#include <Engine/Effect/Particle/Module/Spawner/ParticleSpawnConeModule.h>
#include <Engine/Effect/Particle/Module/Spawner/ParticleSpawnPolygonVertexModule.h>
// Updater
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdateColorModule.h>
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdateVelocityModule.h>
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdateRotationModule.h>
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdateScaleModule.h>
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdateUVModule.h>
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdateGravityModule.h>
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdateEmissiveModule.h>
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdateAlphaReferenceModule.h>
#include <Engine/Effect/Particle/Module/Updater/ParticleUpdatePrimitiveModule.h>

//============================================================================
//	ParticleManager classMethods
//============================================================================

ParticleManager* ParticleManager::instance_ = nullptr;

ParticleManager* ParticleManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ParticleManager();
	}
	return instance_;
}

void ParticleManager::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

ParticleSystem* ParticleManager::CreateParticleSystem(const std::string& filePath, bool useGame) {

	// ファイル読み込みチェック
	if (!JsonAdapter::LoadAssert(filePath)) {

		LOG_WARN("particleFile not found → {}", filePath);
		ASSERT(FALSE, "particleFile not found:" + filePath);
		return nullptr;
	}

	// システム作成
	std::unique_ptr<ParticleSystem> system = std::make_unique<ParticleSystem>();
	system->Init(device_, asset_, filePath);
	// jsonファイルから作成
	system->LoadJson(filePath, useGame);
	// 配列に追加
	systems_.emplace_back(std::move(system));

	// 生ptrを返す
	return systems_.back().get();
}

//============================================================================
//	Registry using
//============================================================================

using SpawnModuleRegistry = ParticleModuleRegistry<ICPUParticleSpawnModule, ParticleSpawnModuleID>;
using UpdateModuleRegistry = ParticleModuleRegistry<ICPUParticleUpdateModule, ParticleUpdateModuleID>;

void ParticleManager::RegisterModules() {

	//============================================================================
	//	Spawners
	//============================================================================

	auto& sRegistry = SpawnModuleRegistry::GetInstance();

	sRegistry.Register<ParticleSpawnSphereModule>();
	sRegistry.Register<ParticleSpawnHemisphereModule>();
	sRegistry.Register<ParticleSpawnBoxModule>();
	sRegistry.Register<ParticleSpawnConeModule>();
	sRegistry.Register<ParticleSpawnPolygonVertexModule>();

	//============================================================================
	//	Updaters
	//============================================================================

	auto& uRegistry = UpdateModuleRegistry::GetInstance();

	uRegistry.Register<ParticleUpdateColorModule>();
	uRegistry.Register<ParticleUpdateVelocityModule>();
	uRegistry.Register<ParticleUpdateRotationModule>();
	uRegistry.Register<ParticleUpdateScaleModule>();
	uRegistry.Register<ParticleUpdateUVModule>();
	uRegistry.Register<ParticleUpdateGravityModule>();
	uRegistry.Register<ParticleUpdateEmissiveModule>();
	uRegistry.Register<ParticleUpdateAlphaReferenceModule>();
	uRegistry.Register<ParticleUpdatePrimitiveModule>();
}

void ParticleManager::Init(Asset* asset, ID3D12Device8* device,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler) {

	asset_ = nullptr;
	asset_ = asset;

	device_ = nullptr;
	device_ = device;

	// 各メインモジュール初期化
	gpuUpdater_ = std::make_unique<GPUParticleUpdater>();
	gpuUpdater_->Init(device, asset, srvDescriptor, shaderCompiler);

	renderer_ = std::make_unique<ParticleRenderer>();
	renderer_->Init(device, asset, srvDescriptor, shaderCompiler);

	// 発生、更新モジュールの登録
	RegisterModules();
}

void ParticleManager::Update(DxCommand* dxCommand) {

	if (systems_.empty()) {
		return;
	}

	// すべてのシステムを更新
	for (auto& system : systems_) {

		// パーティクルを更新
		system->Update();

		// GPU
		for (auto& group : system->GetGPUGroup()) {

			// GPU更新
			gpuUpdater_->Update(group.group, dxCommand);
		}
	}
}

void ParticleManager::Rendering(bool debugEnable,
	SceneConstBuffer* sceneBuffer, DxCommand* dxCommand) {

	if (systems_.empty()) {
		return;
	}

	// すべてのシステムを更新
	for (const auto& system : systems_) {
		for (const auto& group : system->GetGPUGroup()) {

			// 描画処理
			renderer_->Rendering(debugEnable, group.group, sceneBuffer, dxCommand);
		}
		for (const auto& group : system->GetCPUGroup()) {

			// 描画処理
			renderer_->Rendering(debugEnable, group.group, sceneBuffer, dxCommand);
		}
	}
}

void ParticleManager::AddSystem() {

	// 名前の設定
	const std::string name = "particleSystem" + std::to_string(++nextSystemId_);
	// 初期化して追加
	std::unique_ptr<ParticleSystem> system = std::make_unique<ParticleSystem>();
	system->Init(device_, asset_, name);
	systems_.emplace_back(std::move(system));
}

void ParticleManager::RemoveSystem() {

	// 選択中のシステムを削除
	if (0 <= selectedSystem_ && selectedSystem_ < static_cast<int>(systems_.size())) {

		systems_.erase(systems_.begin() + selectedSystem_);

		// 未選択状態にする
		selectedSystem_ = -1;
		renamingSystem_ = -1;
	}
}

void ParticleManager::ImGui() {

	EditLayout();

	// 文字のサイズ設定
	ImGui::SetWindowFontScale(0.72f);

	// 左側の表示
	DrawLeftChild();
	// 右側の表示
	DrawRightChild();

	ImGui::SetWindowFontScale(1.0f);
}

void ParticleManager::DrawLeftChild() {

	//============================================================================
	// 左側の表示
	ImGui::BeginChild("##LeftColumn",
		ImVec2(leftColumnWidth_, 0.0f), ImGuiChildFlags_None);

	//============================================================================
	// 左上
	{
		ImGui::BeginChild("SystemAdd", leftUpChildSize_,
			ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		ImGui::SeparatorText("Add System");
		DrawSystemAdd();

		ImGui::EndChild();
	}
	//============================================================================
	// 左の真ん中
	{
		ImGui::BeginChild("GroupAdd", leftCenterChildSize_,
			ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		ImGui::SeparatorText("Add Group");

		// 作成されているかいないか
		if (IsSystemSelected()) {

			systems_[selectedSystem_]->ImGuiGroupAdd();
		} else {

			ImGui::TextDisabled("not selected system");
		}
		ImGui::EndChild();
	}
	//============================================================================
	// 左下
	{
		ImGui::BeginChild("SystemParam", ImVec2(0.0f, 0.0f),
			ImGuiChildFlags_Border);
		ImGui::SeparatorText("System Patameter");

		// 作成されているかいないか
		if (IsSystemSelected()) {

			systems_[selectedSystem_]->ImGuiSystemParameter();
		} else {

			ImGui::TextDisabled("not selected system");
		}
		ImGui::EndChild();

		ImGui::EndChild();
	}
}

void ParticleManager::DrawRightChild() {

	//============================================================================
	// 右側の表示
	ImGui::SameLine();
	ImGui::BeginChild("##RightColumn", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None);

	//============================================================================
	// 右上
	{
		ImGui::BeginChild("SystemSelect", ImVec2(0.0f, rightUpChildSizeY_),
			ImGuiChildFlags_Border);
		ImGui::SeparatorText("Select System");

		DrawSystemSelect();
		ImGui::EndChild();
	}
	//============================================================================
	// 右の真ん中
	{
		ImGui::BeginChild("GroupSelect", ImVec2(0.0f, rightCenterChildSizeY_),
			ImGuiChildFlags_Border);
		ImGui::SeparatorText("Select Group");

		// 作成されているかいないか
		if (IsSystemSelected()) {

			systems_[selectedSystem_]->ImGuiGroupSelect();
		} else {

			ImGui::TextDisabled("not selected system");
		}
		ImGui::EndChild();
	}
	//============================================================================
	// 右下
	{
		ImGui::BeginChild("GroupParam", ImVec2(0.0f, 0.0f),
			ImGuiChildFlags_Border);
		ImGui::SeparatorText("Group Parameter");

		// 作成されているかいないか
		if (IsSystemSelected()) {

			systems_[selectedSystem_]->ImGuiSelectedGroupEditor();
		} else {

			ImGui::TextDisabled("not selected system");
		}
		ImGui::EndChild();

		ImGui::EndChild();
	}
}

void ParticleManager::DrawSystemAdd() {

	// 追加・削除ボタン
	if (ImGui::Button("+System")) {

		AddSystem();
	}
	ImGui::SameLine();
	if (ImGui::Button("-System")) {

		RemoveSystem();
	}
}

void ParticleManager::DrawSystemSelect() {

	// ツリー表示
	for (int systemI = 0; systemI < systems_.size(); ++systemI) {

		ParticleSystem& system = *systems_[systemI];

		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_Leaf
			| ImGuiTreeNodeFlags_NoTreePushOnOpen
			| ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth;
		if (systemI == selectedSystem_) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}
		ImGui::TreeNodeEx((void*)(intptr_t)systemI,
			flags, "%s", system.GetName().c_str());

		// 選択後、ダブルクリックで改名
		if (ImGui::IsItemClicked()) {

			selectedSystem_ = systemI;
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {

			renamingSystem_ = systemI;
			strncpy_s(renameBuffer_, sizeof(renameBuffer_), system.GetName().c_str(), _TRUNCATE);
		}

		// 改名中ならInputTextを重ね描画
		if (renamingSystem_ == systemI) {

			ImGui::SameLine();

			ImGui::PushID(systemI);
			ImGui::SetNextItemWidth(-FLT_MIN);

			ImGuiInputTextFlags itFlags =
				ImGuiInputTextFlags_AutoSelectAll |
				ImGuiInputTextFlags_EnterReturnsTrue;
			bool done = ImGui::InputText("##rename", renameBuffer_,
				sizeof(renameBuffer_), itFlags);
			if (ImGui::IsItemActivated()) {

				ImGui::SetKeyboardFocusHere(-1);
			}

			if (done || (!ImGui::IsItemActive() && ImGui::IsItemDeactivated())) {

				system.SetName(renameBuffer_);
				renamingSystem_ = -1;
			}
			ImGui::PopID();
		}
	}
}

void ParticleManager::EditLayout() {

	ImGui::Begin("ParticleManager EditLayout");

	ImGui::DragFloat("leftColumnWidth_", &leftColumnWidth_, 0.1f);
	ImGui::DragFloat("leftUpChildSize_Y", &leftUpChildSize_.y, 0.1f);
	ImGui::DragFloat("leftCenterChildSize_Y", &leftCenterChildSize_.y, 0.1f);
	rightUpChildSizeY_ = leftUpChildSize_.y;
	rightCenterChildSizeY_ = leftCenterChildSize_.y;

	ImGui::End();
}

bool ParticleManager::IsSystemSelected() const {

	return 0 <= selectedSystem_ && selectedSystem_ < static_cast<int>(systems_.size());
}