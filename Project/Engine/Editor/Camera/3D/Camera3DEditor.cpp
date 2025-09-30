#include "Camera3DEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/TagSystem.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	Camera3DEditor classMethods
//============================================================================

Camera3DEditor* Camera3DEditor::instance_ = nullptr;
Camera3DEditor* Camera3DEditor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new Camera3DEditor();
	}
	return instance_;
}

void Camera3DEditor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void Camera3DEditor::Init(SceneView* sceneView) {

	sceneView_ = nullptr;
	sceneView_ = sceneView;

	controller_ = std::make_unique<CameraPathController>(sceneView_);
	panel_ = std::make_unique<Camera3DEditorPanel>();
	gizmoSynch_ = std::make_unique<CameraPathGizmoSynch>();
	renderer_ = std::make_unique<CameraPathRenderer>();

	// リセット
	selectedSkinnedKey_.clear();
	selectedAnimName_.clear();
	selectedParamKey_.clear();
}

void Camera3DEditor::AddAnimation(const std::string& name, const SkinnedAnimation* animation) {

	// 同じアニメーションは追加できないようにする
	if (Algorithm::Find(skinnedAnimations_, name)) {
		return;
	}
	// アニメーションを追加
	skinnedAnimations_[name] = animation;
}

void Camera3DEditor::Update() {

	// 選択されているキーフレームの更新
	if (!selectedParamKey_.empty()) {

		auto& path = params_[selectedParamKey_];
		selectedKeyIndex_ = gizmoSynch_->SynchSelectedKeyIndex(path, selectedKeyIndex_);
	}

	for (auto& param : std::views::values(params_)) {

		// 追従先オフセットを更新
		gizmoSynch_->UpdateFollowTarget(param);

		// デバッグ線表示
		if (param.isDrawLine3D) {

			renderer_->DrawLine3D(param);
		}
	}

	// ゲーム画面のカメラを更新する
	if (!selectedParamKey_.empty()) {

		auto& param = params_[selectedParamKey_];
		controller_->Update(playbackCamera_, param);
	}
}

void Camera3DEditor::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	// エディター、UIの更新
	panel_->Edit(params_, skinnedAnimations_,
		selectedSkinnedKey_, selectedAnimName_, selectedParamKey_,
		selectedKeyIndex_, paramSaveState_, lastLoaded_, playbackCamera_);

	ImGui::PopItemWidth();
}