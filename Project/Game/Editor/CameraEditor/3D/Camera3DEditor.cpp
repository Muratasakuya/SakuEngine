#include "Camera3DEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Editor/ImGuiObjectEditor.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/InstancedMeshSystem.h>
#include <Engine/Object/System/Systems/TagSystem.h>
#include <Engine/Utility/ImGuiHelper.h>
#include <Engine/Utility/EnumAdapter.h>
#include <Lib/MathUtils/Algorithm.h>

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

	// リセット
	selectedSkinnedKey_.clear();
	selectedAnimName_.clear();
	selectedParamKey_.clear();
}

void Camera3DEditor::Update() {

	// 追従先のオフセットを更新
	UpdateFollowTarget();
}

void Camera3DEditor::UpdateFollowTarget() {

	if (params_.empty()) {
		return;
	}

	// 追従先があるならオフセットを更新する
	for (const auto& param : std::views::values(params_)) {
		for (const auto& keyframe : param.keyframes) {

			// trueなら更新
			if (param.followTarget && param.target != nullptr) {

				keyframe.demoObject->SetOffsetTranslation(param.target->GetWorldPos());

				// 回転を考慮した位置
				const Vector3 rotatedTranslation = param.target->rotation * keyframe.translation;
				keyframe.demoObject->SetTranslation(rotatedTranslation);
			} else {

				keyframe.demoObject->SetOffsetTranslation(Vector3::AnyInit(0.0f));
			}
		}

		// デバッグ表示の線
		DrawKeyframeLine(param);
	}
}

void Camera3DEditor::KeyframeParam::Init() {

	// キーフレーム初期値
	fovY = 0.54f;
	translation = Vector3::AnyInit(0.0f);
	rotation = Quaternion::IdentityQuaternion();
	eulerRotation = Vector3::AnyInit(0.0f);

	// デモ用オブジェクトを作成
	demoObject = std::make_unique<GameObject3D>();
	demoObject->Init("demoCamera", "demoCamera", "Editor");

	Json data;
	if (!JsonAdapter::LoadCheck(demoCameraJsonPath_, data)) {
		return;
	}
	// 見た目を設定
	demoObject->ApplyTransform(data);
	demoObject->ApplyMaterial(data);
}

void Camera3DEditor::AddAnimation(const std::string& name, const SkinnedAnimation* animation) {

	// 同じアニメーションは追加できないようにする
	if (Algorithm::Find(skinnedAnimations_, name)) {
		return;
	}
	// アニメーションを追加
	skinnedAnimations_.emplace(name, animation);
}

void Camera3DEditor::ImGui() {

	float avail = ImGui::GetContentRegionAvail().x;
	float leftChild = avail * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f;
	float rightChild = avail * 0.5f;

	ImGui::SetWindowFontScale(0.8f);
	if (ImGuiHelper::BeginFramedChild("##SelectAdd", nullptr, ImVec2(leftChild, 128.0f))) {

		ImGui::PushItemWidth(itemWidth_);

		// 作成対象のアニメーションの種類を選択
		SelectAnimationSubject();
		ImGui::Spacing();
		// カメラ調整項目の追加
		AddCameraParam();

		ImGui::PopItemWidth();
	}
	ImGuiHelper::EndFramedChild();
	ImGui::SameLine();
	if (ImGuiHelper::BeginFramedChild("##SelectParam", nullptr, ImVec2(rightChild, 128.0f))) {

		// どの調整項目の値を操作するか
		SelectCameraParam();
	}
	ImGuiHelper::EndFramedChild();
	ImGui::SetWindowFontScale(1.0f);

	if (ImGuiHelper::BeginFramedChild("##EditParam", nullptr,
		ImVec2(leftChild + rightChild, ImGui::GetContentRegionAvail().y))) {

		// 調整項目の値操作
		EditCameraParam();
	}
	ImGuiHelper::EndFramedChild();
}

void Camera3DEditor::SelectAnimationSubject() {

	ImGui::PushItemWidth(itemWidth_);

	// アニメーションの中からどれを対象にするか選択
	int currentSkinnedIndex = -1;
	int index = 0;
	for (const auto& animation : skinnedAnimations_) {
		if (animation.first == selectedSkinnedKey_) {

			currentSkinnedIndex = index;
			break;
		}
		++index;
	}
	if (ImGuiHelper::ComboFromKeys("Object",
		&currentSkinnedIndex, skinnedAnimations_, &selectedSkinnedKey_)) {

		// 新しく選択したらリセット
		selectedAnimName_.clear();
	}

	// 対象アニメーションの中から選択
	if (!selectedSkinnedKey_.empty()) {

		const SkinnedAnimation* skinned = skinnedAnimations_.at(selectedSkinnedKey_);
		std::vector<std::string> animNames = skinned->GetAnimationNames();
		int currentAnimIndex = -1;
		int animationCount = static_cast<int>(animNames.size());
		for (int i = 0; i < animationCount; ++i) {
			if (animNames[i] == selectedAnimName_) {

				currentAnimIndex = i;
				break;
			}
		}
		if (ImGuiHelper::ComboFromStrings("In Subject", &currentAnimIndex, animNames)) {
			if (currentAnimIndex >= 0 && currentAnimIndex < animationCount) {

				selectedAnimName_ = animNames[currentAnimIndex];
			}
		}
	}
	ImGui::PopItemWidth();
}

void Camera3DEditor::AddCameraParam() {

	// 何も設定されていなければ追加できない
	if (selectedAnimName_.empty()) {
		return;
	}

	if (ImGui::Button("Add EditData", ImVec2(itemWidth_, 24.0f))) {

		// 同じアニメーションは追加できないようにする
		if (Algorithm::Find(params_, selectedAnimName_)) {
			return;
		}

		CameraParam param{};
		KeyframeParam keyframe{};
		keyframe.Init();
		// キーフレームをデフォルトで1個追加
		param.keyframes.emplace_back(std::move(keyframe));

		// 調整項目追加
		params_.emplace(selectedAnimName_, std::move(param));
	}
}

void Camera3DEditor::SelectCameraParam() {

	// paramsのキー一覧
	std::vector<std::string> keys;
	keys.reserve(params_.size());
	for (const auto& param : params_) {

		keys.push_back(param.first);
	}

	// 現在のindexをkeysから逆引き
	int currentIndex = -1;
	if (!selectedParamKey_.empty()) {
		for (int i = 0; i < static_cast<int>(keys.size()); ++i) {
			if (keys[i] == selectedParamKey_) {

				currentIndex = i;
				break;
			}
		}
	}

	// 何もない場合は文字を表示する
	if (keys.empty()) {
		ImGui::TextDisabled("CameraParam is Empty");
		return;
	}

	// リスト選択
	if (ImGuiHelper::SelectableListFromStrings("CameraParam List", &currentIndex, keys, 8)) {
		if (currentIndex >= 0 && currentIndex < static_cast<int>(keys.size())) {

			selectedParamKey_ = keys[currentIndex];
		}
	}
}

void Camera3DEditor::EditCameraParam() {

	if (selectedParamKey_.empty()) {

		ImGui::TextDisabled("not selected param");
		return;
	}

	if (ImGui::Button("Save DemoCamera")) {

		SaveDemoCamera();
	}
	ImGui::Separator();

	CameraParam& param = params_[selectedParamKey_];
	// 選択するキーフレームを更新
	SelectKeyframe(param);
	KeyframeParam& keyframeParam = param.keyframes[selectedKeyIndex_];

	// 選択されたものの操作
	if (ImGui::BeginTabBar("EditCameraParam")) {
		if (ImGui::BeginTabItem("Lerp")) {

			EditLerp(param);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Keyframe")) {

			EditKeyframe(param);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Main")) {

			EditMainParam(keyframeParam);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Target")) {

			SelectTarget(param);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	if (ImGuiObjectEditor::GetInstance()->IsUsingGuizmo()) {

		// 値の同期
		SynchMainParam(keyframeParam);
	}
}

void Camera3DEditor::EditLerp(CameraParam& param) {

	EnumAdapter<LerpKeyframe::Type>::Combo("LerpType", &param.lerpType);

	bool preUseAveraging = param.useAveraging;
	ImGui::Checkbox("averagingPoints", &param.useAveraging);
	ImGui::DragInt("divisionCount", &param.divisionCount, 1, 4, 512);
	if (preUseAveraging != param.useAveraging) {
		if (param.useAveraging) {

			param.averagedT = LerpKeyframe::AveragingPoints<Vector3>(
				CollectTranslationPoints(param), param.divisionCount, param.lerpType);
		} else {

			param.averagedT.clear();
		}
	}
	param.timer.ImGui("Timer");

	ImGui::SeparatorText("Debug");

	ImGui::Checkbox("isDrawLine3D", &param.isDrawLine3D);
}

void Camera3DEditor::EditKeyframe(CameraParam& param) {

	auto& keyframes = param.keyframes;
	int keyframeSize = static_cast<int>(keyframes.size());
	if (keyframes.empty()) {
		return;
	}
	// 選択中のキーフレーム
	ImGui::Text("selected Keyframe: %d / %d", selectedKeyIndex_ + 1, keyframeSize);

	// キーフレームの追加
	if (ImGui::Button("Add Keyframe")) {

		// 選択中のキーフレームの情報をコピー
		const KeyframeParam& sourceKeyframe = keyframes[selectedKeyIndex_];
		KeyframeParam dstKeyframe{};
		dstKeyframe.fovY = sourceKeyframe.fovY;
		dstKeyframe.translation = sourceKeyframe.translation;
		dstKeyframe.rotation = sourceKeyframe.rotation;
		dstKeyframe.eulerRotation = sourceKeyframe.eulerRotation;
		// デモ用オブジェクトを作成
		dstKeyframe.demoObject = std::make_unique<GameObject3D>();
		dstKeyframe.demoObject->Init("demoCamera", "demoCamera", "Editor");

		Json data;
		if (!JsonAdapter::LoadCheck(demoCameraJsonPath_, data)) {
			return;
		}
		// 見た目を設定
		dstKeyframe.demoObject->ApplyTransform(data);
		dstKeyframe.demoObject->ApplyMaterial(data);

		// 追加
		keyframes.insert(keyframes.begin() + (selectedKeyIndex_ + 1), std::move(dstKeyframe));
		selectedKeyIndex_ += 1;
	}
	ImGui::SameLine();

	// キーフレームの削除
	if (ImGui::Button("Remove Keyframe")) {

		keyframes.erase(keyframes.begin() + selectedKeyIndex_);
		if (keyframeSize <= selectedKeyIndex_) {
			selectedKeyIndex_ = keyframeSize - 1;
		}
	}
	// キーフレームの順番入れ替え
	ImGui::Separator();
	ImGui::TextUnformatted("Reorder");
	bool canUp = (selectedKeyIndex_ > 0);
	bool canDown = (selectedKeyIndex_ + 1 < keyframeSize);

	if (!canUp) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Move Up")) {

		std::swap(keyframes[selectedKeyIndex_ - 1], keyframes[selectedKeyIndex_]);
		selectedKeyIndex_ -= 1;
	}
	if (!canUp) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();

	if (!canDown) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Move Down")) {

		std::swap(keyframes[selectedKeyIndex_], keyframes[selectedKeyIndex_ + 1]);
		selectedKeyIndex_ += 1;
	}
	if (!canDown) {
		ImGui::EndDisabled();
	}
}

void Camera3DEditor::EditMainParam(KeyframeParam& keyframeParam) {

	ImGui::PushItemWidth(itemWidth_);

	bool isEdit = false;
	isEdit |= ImGui::DragFloat3("translation", &keyframeParam.translation.x, 0.1f);
	isEdit |= ImGui::DragFloat3("eulerRotation", &keyframeParam.eulerRotation.x, 0.01f);
	ImGui::DragFloat("fovY", &keyframeParam.fovY, 0.01f);

	// 値を反映
	if (isEdit) {

		keyframeParam.demoObject->SetTranslation(keyframeParam.translation);
		keyframeParam.rotation = Quaternion::Normalize(
			Quaternion::EulerToQuaternion(keyframeParam.eulerRotation));
		keyframeParam.demoObject->SetRotation(keyframeParam.rotation);
	}
	ImGui::PopItemWidth();
}

void Camera3DEditor::SelectTarget(CameraParam& param) {

	ImGui::Text("currentTarget: %s", param.targetName.c_str());
	ImGui::Checkbox("followTarget", &param.followTarget);
	if (ImGui::Button("Remove Target")) {

		param.target = nullptr;
		param.targetName = "";
	}
	ImGui::Separator();

	uint32_t currentId = 0;
	ObjectManager* objectManager = ObjectManager::GetInstance();
	TagSystem* tagSystem = objectManager->GetSystem<TagSystem>();
	// 現在選択されているオブジェクトIDを設定
	for (const auto& [id, tagPtr] : tagSystem->Tags()) {
		if (objectManager->GetData<Transform3D>(id) == param.target) {

			currentId = id;
			break;
		}
	}

	std::string selectedName = param.targetName;
	if (ImGuiHelper::SelectTagTarget("Select Follow Target", &currentId, &selectedName)) {

		// Transformと名前を更新
		param.target = objectManager->GetData<Transform3D>(currentId);
		param.targetName = selectedName;
	}
}

void Camera3DEditor::SynchMainParam(KeyframeParam& keyframeParam) {

	// ギズモで動かした値の同期
	keyframeParam.translation = keyframeParam.demoObject->GetTranslation();
	keyframeParam.rotation = keyframeParam.demoObject->GetRotation();
	keyframeParam.eulerRotation = keyframeParam.demoObject->GetTransform().eulerRotate;
}

void Camera3DEditor::SelectKeyframe(const CameraParam& param) {

	// 範囲内に制限
	if (selectedKeyIndex_ < 0 || static_cast<int>(param.keyframes.size()) <= selectedKeyIndex_) {
		selectedKeyIndex_ = 0;
	}
	// 選択するキーフレームを更新
	if (selectObjectID_ != 0) {
		for (int i = 0; i < static_cast<int>(param.keyframes.size()); ++i) {

			const auto& keyframe = param.keyframes[i];
			if (keyframe.demoObject && keyframe.demoObject->GetObjectID() == selectObjectID_) {

				selectedKeyIndex_ = i;
				break;
			}
		}
	}
}

std::vector<Vector3> Camera3DEditor::CollectTranslationPoints(const CameraParam& param) const {

	// 補間座標の取得
	std::vector<Vector3> points;
	points.reserve(param.keyframes.size());
	for (auto& keyframe : param.keyframes) {

		points.push_back(keyframe.demoObject->GetTransform().GetWorldPos());
	}
	return points;
}

void Camera3DEditor::DrawKeyframeLine(const CameraParam& param) {

	if (!param.isDrawLine3D) {
		return;
	}
	const auto points = CollectTranslationPoints(param);
	Vector3 prev{};
	bool hasPrev = false;
	for (int i = 0; i <= param.divisionCount; ++i) {

		float t = static_cast<float>(i) / static_cast<float>(param.divisionCount);
		Vector3 point = LerpKeyframe::GetValue<Vector3>(points, t, param.lerpType);
		if (hasPrev) {

			LineRenderer::GetInstance()->DrawLine3D(prev, point, Color::Green());
		}
		prev = point;
		hasPrev = true;
	}
}

void Camera3DEditor::SaveDemoCamera() {

	if (params_.empty()) {
		return;
	}

	Json data;
	const auto& firstParam = params_.begin()->second;
	const auto& keyframe = firstParam.keyframes.front();
	keyframe.demoObject->SaveTransform(data);
	keyframe.demoObject->SaveMaterial(data);
	JsonAdapter::Save(demoCameraJsonPath_, data);
}