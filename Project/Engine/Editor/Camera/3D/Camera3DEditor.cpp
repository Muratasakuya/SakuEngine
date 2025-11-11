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

		const bool isPlaying = (runtime_ && runtime_->playing &&
			param.overallName == runtime_->action);
		// ゲームで開始したときの処理
		if (isPlaying || param.isUseGame) {

			gizmoSynch_->ApplyLocalToWorldByTarget(param);
		}
		// エディターで動かしているときの処理
		else {

			gizmoSynch_->UpdateFollowTarget(param);
		}

		// 追従先オフセットを更新
		gizmoSynch_->UpdateFollowTarget(param);
	}

	// ランタイムのゲームカメラを更新
	UpdateGameAnimation();

	// ゲーム画面のカメラを更新する
	if (!selectedParamKey_.empty()) {

		auto& param = params_[selectedParamKey_];
		controller_->Update(playbackCamera_, param);
	}

	// アクションとカメラの同期
	if (!selectedParamKey_.empty() && playbackCamera_.isSynch) {

		auto& data = params_[selectedParamKey_];
		const float synchT = ComputeEffectiveCameraT(playbackCamera_, data);
		ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();
		for (auto& [name, bind] : actionBinds_) {

			// オブジェクト未設定ならスキップ
			if (bind.objectName.empty() || bind.spanName.empty()) {
				continue;
			}

			const int objectId = monitor->FindObjectID(bind.objectName);

			// 同期するか進捗に通知
			const bool external = playbackCamera_.isSynch && bind.driveStateFromCamera;
			monitor->NotifySynchState(objectId, external);
			if (bind.driveStateFromCamera) {

				// オブジェクトが所持しているローカルの進捗を更新
				const auto spanNames = monitor->GetSpanNames(objectId);
				for (const auto& span : spanNames) {

					monitor->DriveSpanByGlobalT(objectId, span, synchT);
				}
				monitor->NotifyOverallDrive(objectId, synchT);
			} else {

				// アクション全体進捗でカメラを更新する
				float overallT = 0.0f;
				if (monitor->GetOverallValue(objectId, bind.spanName, &overallT)) {

					// マニュアルにして自動更新されるようにする
					playbackCamera_.mode = CameraPathController::PreviewMode::Manual;
					playbackCamera_.time = std::clamp(overallT, 0.0f, 1.0f);
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

void Camera3DEditor::LoadAnimFile(const std::string& fileName) {

	// 読み込めなければ作成しない
	Json data;
	const std::string filePath = CameraPathData::cameraParamJsonPath + fileName;
	if (!JsonAdapter::LoadCheck(filePath, data)) {
		return;
	}

	CameraPathData param{};
	// 名前を取得
	param.objectName = data["objectName"];
	param.overallName = data["overallName"];

	// 追加済みの場合処理しない
	if (Algorithm::Find(params_, param.overallName)) {
		return;
	}

	// データから値を設定
	param.ApplyJson(filePath, true);
	// 追加
	params_.emplace(param.overallName, std::move(param));
}

void Camera3DEditor::StartAnim(const std::string& actionName, bool canCutIn, bool isAddFirstKey) {

	// 無ければ処理できない
	if (!Algorithm::Find(params_, actionName)) {
		return;
	}
	// 再生中
	if (runtime_ && runtime_->playing) {
		// 割り込み不可なら処理しない
		if (!canCutIn) {
			return;
		}
		// リセットして再スタート
		EndAnim(runtime_->action);
	}
	// キーフレームの最初に現在位置のカメラを追加
	CameraPathData& param = params_[actionName];
	CameraPathData::KeyframeParam keyframe{};
	BaseCamera* camera = sceneView_->GetCamera();

	// 追従先がいるかどうか
	const bool hasTarget = (param.followTarget && param.target);
	// 追従情報
	const Vector3 targetTranslation = hasTarget ? param.target->GetWorldPos() : Vector3::AnyInit(0.0f);
	const Quaternion targetRotation = hasTarget ?
		Quaternion::Normalize(param.target->rotation) :
		Quaternion::IdentityQuaternion();
	const Quaternion inverseTargetRotation = Quaternion::Conjugate(targetRotation);
	// カメラ位置、回転
	const Vector3 cameraTranslation = camera->GetTransform().translation;
	const Quaternion cameraRotation = camera->GetTransform().rotation;
	// ローカルの座標と回転を求める
	Vector3 localTranslation = hasTarget ? inverseTargetRotation *
		(cameraTranslation - targetTranslation) : cameraTranslation;
	Quaternion localRataion = param.followRotation ?
		Quaternion::Normalize(inverseTargetRotation * cameraRotation) : cameraRotation;
	// キーフレームを初期位置に設定して初期化
	keyframe.Init(true);
	keyframe.demoObject->SetOffsetTranslation(targetTranslation);
	keyframe.demoObject->SetTranslation(targetRotation * localTranslation);
	keyframe.demoObject->SetRotation(param.followRotation ?
		Quaternion::Normalize(targetRotation * localRataion) : localRataion);
	// 行列を更新
	keyframe.demoObject->UpdateMatrix();
	// ローカルを設定
	keyframe.translation = localTranslation;
	keyframe.rotation = localRataion;
	keyframe.fovY = camera->GetFovY();

	// 追加
	const uint32_t injectedId = keyframe.demoObject->GetObjectID();
	// フラグが立っていれば最初に追加
	if (isAddFirstKey) {

		param.keyframes.insert(param.keyframes.begin(), std::move(keyframe));
	}

	// キーフレームの平均の再取得
	if (param.useAveraging) {

		auto points = param.CollectTranslationPoints();
		param.averagedT = LerpKeyframe::AveragingPoints<Vector3>(
			points, param.divisionCount, param.lerpType);
	}

	// リセットして開始させる
	param.timer.Reset();
	runtime_ = RuntimePlayState{ actionName, true, canCutIn, injectedId };
}

void Camera3DEditor::EndAnim(const std::string& actionName) {

	// 無ければ処理できない
	if (!Algorithm::Find(params_, actionName)) {
		return;
	}
	// 開始時に追加した最初のキーフレームを削除
	CameraPathData& param = params_[actionName];
	if (runtime_ && runtime_->playing && runtime_->action == actionName) {

		// IDが一致したオブジェクト
		const auto headId = param.keyframes.front().demoObject->GetObjectID();
		// 追加したオブジェクトなら削除
		if (headId == runtime_->injectedHeadId) {

			param.keyframes.erase(param.keyframes.begin());
		}
		// キーフレームの平均の再取得
		if (param.useAveraging) {

			auto points = param.CollectTranslationPoints();
			param.averagedT = LerpKeyframe::AveragingPoints<Vector3>(
				points, param.divisionCount, param.lerpType);
		}
		runtime_ = std::nullopt;
	}
	// 更新状態を元に戻す
	sceneView_->GetCamera()->SetIsUpdateEditor(false);
}

void Camera3DEditor::UpdateGameAnimation() {

	// 再生中のみ処理
	if (!runtime_.has_value()) {
		return;
	}

	auto& param = params_[runtime_->action];

	// 時間を進める
	float easedT = param.UpdateAndGetEffectiveEasedT();
	float t = param.useAveraging ? LerpKeyframe::GetReparameterizedT(easedT, param.averagedT) : easedT;
	// それぞれの値の補間
	Vector3 translation;
	Quaternion rotation;
	float fovY;
	controller_->Evaluate(param, t, translation, rotation, fovY);

	// カメラへ適応
	BaseCamera* camera = sceneView_->GetCamera();
	camera->SetIsUpdateEditor(true);
	controller_->ApplyToCamera(*camera, translation, rotation, fovY, !param.isDrawLine3D);

	// 補間が最後まで行けば終了
	if (!param.staying && param.timer.IsReached()) {

		camera->SetRotation(param.keyframes.back().demoObject->GetRotation());
		EndAnim(runtime_->action);
	}
}

bool Camera3DEditor::IsAnimationFinished(const std::string& actionName) const {

	// 存在しないキーの名前ならfalse
	auto it = params_.find(actionName);
	if (it == params_.end()) {

		return false;
	}

	// タイマーが最後まで到達しているか
	return !it->second.staying && it->second.timer.IsReached();
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

		if (data.useAveraging && !data.averagedT.empty()) {

			return LerpKeyframe::GetReparameterizedT(state.time, data.averagedT);
		}
		return state.time;
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