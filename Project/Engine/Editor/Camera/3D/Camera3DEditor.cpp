#include "Camera3DEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
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
	selectedObjectKey_.clear();
	selectedActionName_.clear();
	selectedParamKey_.clear();
}

void Camera3DEditor::AddActionObject(const std::string& name) {

	// 追加済みの場合は処理しない
	if (Algorithm::Find(actionBinds_, name)) {
		return;
	}
	// 初期値で追加
	CameraPathController::ActionSynchBind bind{};
	bind.objectName = name;
	actionBinds_.emplace(name, std::move(bind));
}

void Camera3DEditor::SetActionBinding(const std::string& objectName,
	const std::string& spanName, bool driveStateFromCamera) {

	// 追加されていなければ追加して値を設定
	if (!Algorithm::Find(actionBinds_, objectName)) {

		AddActionObject(objectName);
	}
	auto& bind = actionBinds_[objectName];
	bind.objectName = objectName;
	bind.spanName = spanName;
	bind.driveStateFromCamera = driveStateFromCamera;
}

void Camera3DEditor::Update() {

	// 選択されているキーフレームの更新
	if (!selectedParamKey_.empty()) {

		auto& path = params_[selectedParamKey_];
		selectedKeyIndex_ = gizmoSynch_->SynchSelectedKeyIndex(path, selectedKeyIndex_);
		playbackCamera_.selectedKeyIndex = selectedKeyIndex_;
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

	if (!selectedParamKey_.empty()) {

		auto& data = params_[selectedParamKey_];
		// 同期を行う補間値を取得
		const float synchT = ComputeEffectiveCameraT(playbackCamera_, data);

		ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();
		for (auto& [name, bind] : actionBinds_) {

			// 設定されていないアクションは処理しない
			if (bind.spanName.empty()) {
				continue;
			}

			const int objectId = monitor->FindObjectID(bind.objectName);
			if (bind.driveStateFromCamera) {

				// 補間値をバインドした状態にセット
				monitor->DriveSpanByGlobalT(objectId, bind.spanName, synchT);
			} else {

				// バインドされている状態の補間値
				float start = 0.0f;  // 開始
				float end = 1.0f;    // 終了
				float localT = 0.0f; // ローカル
				if (monitor->GetSpanStart(objectId, bind.spanName, &start) &&
					monitor->GetSpanEnd(objectId, bind.spanName, &end) &&
					monitor->GetSpanLocal(objectId, bind.spanName, &localT)) {

					const float world = (std::max)(end - start, 1e-6f);
					const float t = std::clamp(start + world * localT, 0.0f, 1.0f);

					// マニュアルにして現在値を確認できるようにする
					playbackCamera_.mode = CameraPathController::PreviewMode::Manual;
					// 値を反映
					playbackCamera_.time = t;
				}
			}
		}
	}
}

void Camera3DEditor::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	// エディター、UIの更新
	panel_->Edit(params_, actionBinds_,
		selectedObjectKey_, selectedActionName_, selectedParamKey_,
		selectedKeyIndex_, paramSaveState_, lastLoaded_, playbackCamera_);

	ImGui::PopItemWidth();
}

float Camera3DEditor::ComputeEffectiveCameraT(
	const CameraPathController::PlaybackState& state,
	const CameraPathData& data) const {

	switch (state.mode) {
	case CameraPathController::PreviewMode::Keyframe: {

		// キーフレームがない場合は処理しない
		if (data.keyframes.empty()) {
			return 0.0f;
		}

		const int keyCount = static_cast<int>(data.keyframes.size());
		int index = std::clamp(state.selectedKeyIndex, 0, keyCount - 1);

		// 現在のキーフレームインデックス位置
		const float division = 1.0f / (keyCount - 1);
		// 前後のキーの中間値を返す
		return division * index + division * 0.5f;
	}
	case CameraPathController::PreviewMode::Manual: {

		float rawT = std::clamp(state.time, 0.0f, 1.0f);
		float easedT = EasedValue(data.timer.easeingType_, rawT);
		if (data.useAveraging && !data.averagedT.empty()) {

			return LerpKeyframe::GetReparameterizedT(easedT, data.averagedT);
		}
		return easedT;
	}
	case CameraPathController::PreviewMode::Play: {

		if (data.useAveraging && !data.averagedT.empty()) {

			return LerpKeyframe::GetReparameterizedT(data.timer.easedT_, data.averagedT);
		}
		return data.timer.easedT_;
	}
	}
	return 0.0f;
}