#include "Camera3DEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Utility/ImGuiHelper.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/InstancedMeshSystem.h>
#include <Engine/Object/System/Systems/TagSystem.h>
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
}

void Camera3DEditor::KeyframeParam::Init() {

	// キーフレーム初期値
	fovY = 0.54f;
	translation = Vector3::AnyInit(0.0f);
	rotation = Vector3::AnyInit(0.0f);
	timer.target_ = 0.4f;
	timer.Reset();

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
		if (ImGui::BeginTabItem("Target")) {

			SelectTarget(keyframeParam);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
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

void Camera3DEditor::SelectTarget(KeyframeParam& keyframeParam) {

	ImGui::Text("currentTarget: %s", keyframeParam.targetName.c_str());
	if (ImGui::Button("Remove Target")) {

		keyframeParam.target = nullptr;
		keyframeParam.targetName = "";
	}
	ImGui::Separator();

	uint32_t currentId = 0;
	ObjectManager* objectManager = ObjectManager::GetInstance();
	TagSystem* tagSystem = objectManager->GetSystem<TagSystem>();
	// 現在選択されているオブジェクトIDを設定
	for (const auto& [id, tagPtr] : tagSystem->Tags()) {
		if (objectManager->GetData<Transform3D>(id) == keyframeParam.target) {

			currentId = id;
			break;
		}
	}

	std::string selectedName = keyframeParam.targetName;
	if (ImGuiHelper::SelectTagTarget("Select Follow Target", &currentId, &selectedName)) {

		// Transformと名前を更新
		keyframeParam.target = objectManager->GetData<Transform3D>(currentId);
		keyframeParam.targetName = selectedName;
	}
}

void Camera3DEditor::SaveDemoCamera() {

	Json data;

	for (const auto& keyframes : std::views::values(params_)) {
		for (const auto& keyframe : keyframes.keyframes) {

			keyframe.demoObject->SaveTransform(data);
			keyframe.demoObject->SaveMaterial(data);
			break;
		}
		break;
	}
	JsonAdapter::Save(demoCameraJsonPath_, data);
}