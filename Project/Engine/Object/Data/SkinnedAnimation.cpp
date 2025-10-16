#include "SkinnedAnimation.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

// imgui
#include <imgui.h>

//============================================================================
//	SkinnedAnimation classMethods
//============================================================================

void SkinnedAnimation::Init(const std::string& animationName, Asset* asset) {

	asset_ = nullptr;
	asset_ = asset;

	isDisplayBone_ = false;

	// 初期値
	playbackSpeed_ = 1.0f;
	transitionDuration_ = 0.4f;
	currentAnimationName_ = animationName;

	// 骨の情報とクラスターを渡す
	skeleton_ = asset_->GetSkeletonData(Algorithm::RemoveAfterUnderscore(currentAnimationName_));
	skinCluster_ = asset_->GetSkinClusterData(Algorithm::RemoveAfterUnderscore(currentAnimationName_));
	animationData_[currentAnimationName_] = asset_->GetAnimationData(currentAnimationName_);

	// 使用するjointを記録
	std::vector<const NodeAnimation*>& tracks = jointAnimationTracks_[currentAnimationName_];
	tracks.assign(skeleton_.joints.size(), nullptr);
	for (const Joint& j : skeleton_.joints) {
		// jointIndexで値を設定
		auto it = animationData_[currentAnimationName_].nodeAnimations.find(j.name);
		if (it != animationData_[currentAnimationName_].nodeAnimations.end()) {

			tracks[j.index] = &it->second;
		}
	}

	// 子の値を設定
	children_.assign(skeleton_.joints.size(), {});
	for (size_t i = 0; i < skeleton_.joints.size(); ++i) {
		if (skeleton_.joints[i].parent) {

			children_[*skeleton_.joints[i].parent].push_back(static_cast<int>(i));
		}
	}

	// ループ再生状態にする
	SetPlayAnimation(currentAnimationName_, true);
}

void SkinnedAnimation::Update(const Matrix4x4& worldMatrix) {

	// animationがなにも設定されていなければ何もしない
	if (animationData_.empty()) {
		return;
	}

	animationFinish_ = false;
	// 更新方法のインデックスを更新
	updateModeIndex_ = static_cast<uint32_t>(updateMode_);

	// Notなら処理しない
	if (updateMode_ == ObjectUpdateMode::Not) {
		return;
	}

	//========================================================================
	// 通常のAnimation再生
	//========================================================================
	float deltaTime = GameTimer::GetScaledDeltaTime() * playbackSpeed_;
	auto& currentTimer = currentAnimationTimers_[updateModeIndex_];
	if (!inTransition_) {
		if (updateMode_ == ObjectUpdateMode::None) {
			// ループ再生かしないか
			if (roopAnimation_) {

				float duration = animationData_[currentAnimationName_].duration;
				if (currentTimer + deltaTime >= duration) {

					// 再生カウントをインクリメント
					++repeatCount_;
					prevFrameIndexPerKey_.clear();
				}

				currentTimer = std::fmod(currentTimer + deltaTime, duration);

				// 進行度を計算
				animationProgress_ = currentTimer / animationData_[currentAnimationName_].duration;
			} else {
				// 経過時間が最大にいくまで時間を進める
				if (animationData_[currentAnimationName_].duration > currentTimer) {

					currentTimer += deltaTime;
				}

				// 経過時間に達したら終了させる
				if (currentTimer >= animationData_[currentAnimationName_].duration) {

					currentTimer = animationData_[currentAnimationName_].duration;
					animationFinish_ = true;
				}

				animationProgress_ = currentTimer / animationData_[currentAnimationName_].duration;
			}
		}

		// jointの値を更新する
		ApplyAnimation(currentTimer);
		UpdateSkeleton(worldMatrix);

		// bufferに渡す値を更新する
		UpdateSkinCluster();
	}
	//========================================================================
	// 遷移中のAnimation再生
	//========================================================================
	else {

		// 遷移時間を進める
		transitionTimer_ += GameTimer::GetScaledDeltaTime();
		float alpha = transitionTimer_ / transitionDuration_;
		if (alpha > 1.0f) {

			alpha = 1.0f;
		}

		// animationをblendしてjointの値を更新する
		BlendAnimation(skeleton_,
			animationData_[oldAnimationName_], oldAnimationTimer_,
			animationData_[nextAnimationName_], nextAnimationTimer_, alpha);
		UpdateSkeleton(worldMatrix);

		// bufferに渡す値を更新する
		UpdateSkinCluster();

		// 遷移終了
		if (alpha >= 1.0f) {

			inTransition_ = false;
			currentAnimationName_ = nextAnimationName_;
			currentTimer = nextAnimationTimer_;
		}
	}

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// 骨の描画
	DebugDrawBone(worldMatrix);
#endif
}

void SkinnedAnimation::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	// ループ再生・リスタート
	ImGui::Text("currentAnim: %s", currentAnimationName_.c_str());
	if (ImGui::CollapsingHeader("Playback", ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::Checkbox("isDisplayBone", &isDisplayBone_);
		ImGui::Checkbox("Loop", &roopAnimation_);
		ImGui::SameLine();
		if (ImGui::Button("Restart")) {

			for (auto& timer : currentAnimationTimers_) {

				timer = 0.0f;
			}
			repeatCount_ = 0;
		}
		ImGui::Text("RepeatCount: %d", repeatCount_);
		ImGui::DragFloat("playbackSpeed", &playbackSpeed_, 0.01f);
		EnumAdapter<ObjectUpdateMode>::Combo("UpdateMode", &updateMode_);
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen)) {

		const float timer = currentAnimationTimers_[updateModeIndex_];
		const float duration = animationData_[currentAnimationName_].duration;
		const float prog = timer / duration;
		const float transProg = transitionDuration_ > 0.0f ? transitionTimer_ / transitionDuration_ : 0.0f;

		ImGui::Text("Time       : %5.3f / %5.3f", timer, duration);
		ImGui::ProgressBar(prog, ImVec2(-FLT_MIN, 4));
		ImGui::Text("Transition : %5.2f %%", transProg * 100.0f);
		ImGui::ProgressBar(transProg, ImVec2(-FLT_MIN, 4));
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Animation Select", ImGuiTreeNodeFlags_DefaultOpen)) {

		static std::vector<const char*> animNames;
		animNames.clear();
		for (const auto& [name, _] : animationData_) {

			animNames.push_back(name.c_str());
		}
		int currentIndex = 0;
		for (size_t i = 0; i < animNames.size(); ++i) {
			if (currentAnimationName_ == animNames[i]) {

				currentIndex = static_cast<int>(i);
				break;
			}
		}

		if (ImGui::Combo("##AnimCombo", &currentIndex, animNames.data(),
			static_cast<int>(animNames.size()))) {

			SwitchAnimation(animNames[currentIndex], true, transitionDuration_);
		}
	}

	ImGui::Separator();

	if (auto itAnim = eventKeyTables_.find(currentAnimationName_);
		itAnim != eventKeyTables_.end() && !itAnim->second.empty()) {

		const auto& kindMap = itAnim->second;
		const std::vector<int>* framesPtr = nullptr;
		auto itEffect = kindMap.find("Effect");
		if (itEffect != kindMap.end()) {

			framesPtr = &itEffect->second;
		} else {

			framesPtr = &kindMap.begin()->second;
		}

		int currentFrame = CurrentFrameIndex();
		int totalFrames = static_cast<int>(animationData_[currentAnimationName_].duration * 30.0f);

		ImGui::Text("Key Timeline");
		DrawEventTimeline(*framesPtr, currentFrame, totalFrames, itemSize * 2.0f, 10.0f);
	}

	ImGui::PopItemWidth();
}

void SkinnedAnimation::ApplyAnimation(float timer) {

	// index配列を用意
	std::vector<size_t> indices(skeleton_.joints.size());
	// 0からsize分までの連続した値を作成する
	std::iota(indices.begin(), indices.end(), 0);
	const auto& tracks = jointAnimationTracks_[currentAnimationName_];

	// par_unseqで並列処理
	std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](size_t i) {

		const NodeAnimation* track = tracks[i];
		if (!track) {
			return;
		}

		Joint& joint = skeleton_.joints[i];
		joint.transform.translation = Vector3::CalculateValue(track->translate.keyframes, timer);
		joint.transform.rotation = Quaternion::CalculateValue(track->rotate.keyframes, timer);
		joint.transform.scale = Vector3::CalculateValue(track->scale.keyframes, timer);
		});
}

void SkinnedAnimation::UpdateSkeleton(const Matrix4x4& worldMatrix) {

	// 全てのJointを更新、親が若いので通常ループで処理可能
	for (auto& joint : skeleton_.joints) {

		joint.localMatrix =
			Matrix4x4::MakeAxisAffineMatrix(joint.transform.scale, joint.transform.rotation, joint.transform.translation);
		// 親がいれば親の行列を掛ける
		if (joint.parent) {

			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton_.joints[*joint.parent].skeletonSpaceMatrix;
		}
		// 親がいないのでlocalMatrixとSkeletonSpaceMatrixは一致する
		else {

			joint.skeletonSpaceMatrix = joint.localMatrix;
		}

		// 親なら行列を更新する
		if (joint.isParentTransform) {

			joint.transform.SetIsDirty(true);
			joint.transform.matrix.world = joint.skeletonSpaceMatrix * worldMatrix;
		}
	}
}

void SkinnedAnimation::UpdateSkinCluster() {

	for (size_t jointIndex = 0; jointIndex < skeleton_.joints.size(); ++jointIndex) {

		assert(jointIndex < skinCluster_.inverseBindPoseMatrices.size());

		skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix =
			skinCluster_.inverseBindPoseMatrices[jointIndex] *
			skeleton_.joints[jointIndex].skeletonSpaceMatrix;
		skinCluster_.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Matrix4x4::Transpose(Matrix4x4::Inverse(skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
}

void SkinnedAnimation::BlendAnimation(Skeleton& skeleton, const AnimationData& oldAnimationData,
	float oldAnimTime, const AnimationData& nextAnimationData, float nextAnimTime, float alpha) {

	// すべてのJointを対象
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {

		auto& jointOld = skeleton.joints[jointIndex];

		const std::string& nodeName = jointOld.name;

		// old の transform
		Vector3 posOld = Vector3(0.0f, 0.0f, 0.0f);
		Quaternion rotOld = Quaternion::IdentityQuaternion();
		Vector3 sclOld = Vector3(1.0f, 1.0f, 1.0f);
		if (auto itOld = oldAnimationData.nodeAnimations.find(nodeName); itOld != oldAnimationData.nodeAnimations.end()) {

			const auto& rootNodeAnimation = itOld->second;
			if (!rootNodeAnimation.translate.keyframes.empty()) {

				posOld = Vector3::CalculateValue(rootNodeAnimation.translate.keyframes, oldAnimTime);
			}
			if (!rootNodeAnimation.rotate.keyframes.empty()) {

				rotOld = Quaternion::CalculateValue(rootNodeAnimation.rotate.keyframes, oldAnimTime);
			}
			if (!rootNodeAnimation.scale.keyframes.empty()) {

				sclOld = Vector3::CalculateValue(rootNodeAnimation.scale.keyframes, oldAnimTime);
			}
		}

		// next の transform
		Vector3 posNext = Vector3(0.0f, 0.0f, 0.0f);
		Quaternion rotNext = Quaternion::IdentityQuaternion();
		Vector3 sclNext = Vector3(1.0f, 1.0f, 1.0f);
		if (auto itNext = nextAnimationData.nodeAnimations.find(nodeName); itNext != nextAnimationData.nodeAnimations.end()) {
			const auto& rootNodeAnimation = itNext->second;

			if (!rootNodeAnimation.translate.keyframes.empty()) {

				posNext = Vector3::CalculateValue(rootNodeAnimation.translate.keyframes, nextAnimTime);
			}
			if (!rootNodeAnimation.rotate.keyframes.empty()) {

				rotNext = Quaternion::CalculateValue(rootNodeAnimation.rotate.keyframes, nextAnimTime);
			}
			if (!rootNodeAnimation.scale.keyframes.empty()) {

				sclNext = Vector3::CalculateValue(rootNodeAnimation.scale.keyframes, nextAnimTime);
			}
		}

		// αブレンド
		Vector3 posBlend = Vector3::Lerp(posOld, posNext, alpha);
		Quaternion rotBlend = Quaternion::Slerp(rotOld, rotNext, alpha);
		Vector3 sclBlend = Vector3::Lerp(sclOld, sclNext, alpha);

		jointOld.transform.translation = posBlend;
		jointOld.transform.rotation = rotBlend;
		jointOld.transform.scale = sclBlend;
	}
}

int SkinnedAnimation::CurrentFrameIndex() const {

	// blenderの再生FPS
	constexpr float kFps = 30.0f;
	const float timer = currentAnimationTimers_[updateModeIndex_];
	return static_cast<int>((std::max)(0.0f, timer * kFps + 0.5f));
}

void SkinnedAnimation::DrawEventTimeline(const std::vector<int>& frames,
	int currentFrame, int totalFrames, float barWidth, float barHeight) {

	if (frames.empty() || totalFrames <= 0) {
		return;
	}

	// レイアウト領域を確保
	ImGui::Dummy(ImVec2(barWidth, barHeight * 2.0f));
	ImVec2 p0 = ImGui::GetItemRectMin();
	ImVec2 p1 = ImGui::GetItemRectMax();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	// 背景バー
	dl->AddRectFilled(p0, p1, IM_COL32(80, 80, 80, 255), barHeight * 0.5f);

	// 経過バー
	float progT = static_cast<float>(currentFrame) / static_cast<float>(totalFrames);
	progT = std::clamp(progT, 0.0f, 1.0f);
	ImVec2 pProg = ImVec2(std::lerp(p0.x, p1.x, progT), p1.y);
	dl->AddRectFilled(p0, pProg, IM_COL32(240, 200, 0, 255), barHeight * 0.5f);

	// キー・マーカー
	const float yCenter = (p0.y + p1.y) * 0.5f;
	const float radius = barHeight * 0.64f;

	for (size_t i = 0; i < frames.size(); ++i) {

		float t = static_cast<float>(frames[i]) / static_cast<float>(totalFrames);
		float x = std::lerp(p0.x, p1.x, t);

		// 通過済み => 緑 / 未来 => 灰
		ImU32 col = (currentFrame >= frames[i])
			? IM_COL32(50, 220, 50, 255)
			: IM_COL32(180, 180, 180, 255);

		dl->AddCircleFilled(ImVec2(x, yCenter), radius, col);
	}
}

void SkinnedAnimation::DebugDrawBone(const Matrix4x4& worldMatrix) {

	if (!isDisplayBone_) {
		return;
	}

	constexpr int kDivision = 4;
	constexpr float kRatioTop = 0.02f;
	constexpr float kRatioBase = 0.088f;

	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	// jointの描画
	std::vector<Vector3> worldPos(skeleton_.joints.size());
	for (size_t i = 0; i < skeleton_.joints.size(); ++i) {

		worldPos[i] = Vector3::Transform(Vector3::AnyInit(0.0f),
			skeleton_.joints[i].skeletonSpaceMatrix * worldMatrix);
		lineRenderer->DrawSphere(kDivision, 0.32f, worldPos[i], Color::Green(), LineType::DepthIgnore);
	}

	// 親から子に向けて描画
	for (size_t parent = 0; parent < skeleton_.joints.size(); ++parent) {
		for (int children : children_[parent]) {

			Vector3 base = worldPos[parent];  // 親
			Vector3 tip = worldPos[children]; // 子
			Vector3 direction = tip - base;
			float length = direction.Length();
			if (length < 1e-4f) {
				continue;
			}

			direction.x /= length;
			direction.y /= length;
			direction.z /= length;
			Quaternion rotation = Quaternion::FromToY(direction);
			rotation = rotation.Inverse(rotation);

			float top = length * kRatioTop;
			float bottom = length * kRatioBase;

			lineRenderer->DrawCone(kDivision, bottom, top,
				length, base, rotation, Color::Convert(0xffff00ff), LineType::DepthIgnore);
		}
	}
}

void SkinnedAnimation::SetAnimationData(const std::string& animationName) {

	// 登録済みの場合は処理しない
	if (Algorithm::Find(animationData_, animationName)) {
		return;
	}

	animationData_[animationName] = asset_->GetAnimationData(animationName);

	// 使用するjointを記録
	std::vector<const NodeAnimation*>& tracks = jointAnimationTracks_[animationName];
	tracks.assign(skeleton_.joints.size(), nullptr);
	for (const Joint& j : skeleton_.joints) {
		// jointIndexで値を設定
		auto it = animationData_[animationName].nodeAnimations.find(j.name);
		if (it != animationData_[animationName].nodeAnimations.end()) {

			tracks[j.index] = &it->second;
		}
	}
}

void SkinnedAnimation::SetKeyframeEvent(const std::string& fileName) {

	Json data;
	if (!JsonAdapter::LoadCheck(fileName, data)) {
		return;
	}

	// リセット
	eventKeyTables_.clear();
	prevFrameIndexPerKey_.clear();

	// animationの名前の前のmodelの名前を取得
	std::string prefix = Algorithm::RemoveAfterUnderscore(currentAnimationName_);
	prefix += "_";

	for (const auto& animJson : data["animations"]) {

		if (animJson.contains("events")) {

			// アニメーションの名前をキーにする
			const std::string& action = animJson["action"].get<std::string>();
			std::string fullName = prefix + action;

			auto& kindMap = eventKeyTables_[fullName];
			kindMap.clear();
			const auto& ev = animJson["events"];
			for (auto it = ev.begin(); it != ev.end(); ++it) {

				if (!it->is_array()) {
					continue;
				}
				std::vector<int> frames = it->get<std::vector<int>>();
				std::sort(frames.begin(), frames.end());
				frames.erase(std::unique(frames.begin(), frames.end()), frames.end());
				kindMap[it.key()] = std::move(frames);
			}
		}
	}
}

void SkinnedAnimation::SetPlayAnimation(const std::string& animationName, bool roopAnimation) {

	// Animationの再生設定
	currentAnimationTimers_[updateModeIndex_] = 0.0f;
	currentAnimationName_ = animationName;
	roopAnimation_ = roopAnimation;

	animationFinish_ = false;
	prevFrameIndexPerKey_.clear();
}

void SkinnedAnimation::ResetAnimation() {

	// animationリセット
	repeatCount_ = 0;
	animationFinish_ = false;
	prevFrameIndexPerKey_.clear();
}

void SkinnedAnimation::SwitchAnimation(const std::string& nextAnimName,
	bool loopAnimation, float transitionDuration) {

	prevFrameIndexPerKey_.clear();

	// 現在のAnimationを設定
	oldAnimationName_ = currentAnimationName_;
	oldAnimationTimer_ = currentAnimationTimers_[updateModeIndex_];

	// 次のAnimationを設定
	nextAnimationName_ = nextAnimName;
	nextAnimationTimer_ = 0.0f;

	// 遷移開始
	inTransition_ = true;
	transitionTimer_ = 0.0f;
	transitionDuration_ = transitionDuration;

	roopAnimation_ = loopAnimation;
	animationFinish_ = false;
}

void SkinnedAnimation::SetParentJoint(const std::string& jointName) {

	for (auto& joint : skeleton_.joints) {
		if (joint.name == jointName) {

			// 親として更新させる
			joint.isParentTransform = true;
			return;
		}
	}
}

bool SkinnedAnimation::IsEventKey(const std::string& keyEvent, uint32_t frameIndex) {

	// 対象アニメーションのイベント表
	auto animIt = eventKeyTables_.find(currentAnimationName_);
	if (animIt == eventKeyTables_.end()) {
		return false;
	}

	// 種類ごとの配列
	const auto& kindMap = animIt->second;
	auto kindIt = kindMap.find(keyEvent);
	if (kindIt == kindMap.end()) {
		return false;
	}

	const auto& frames = kindIt->second;
	if (frameIndex >= frames.size()) {
		return false;
	}

	// 現在フレームと照合
	const int current = CurrentFrameIndex();
	const int target = frames[frameIndex];

	const std::string prevKey = currentAnimationName_ + "|" + keyEvent;
	int& prev = prevFrameIndexPerKey_[prevKey];

	// 連続で発生させないようにする
	const bool result = (current == target && prev != target);
	if (result) {
		prev = current;
	}
	return result;
}

float SkinnedAnimation::GetAnimationDuration(const std::string& animationName) const {

	// animationの再生時間を取得
	float duration = animationData_.at(animationName).duration;
	return duration;
}

float SkinnedAnimation::GetEventTime(const std::string& animName,
	const std::string& keyEvent, uint32_t frameIndex) const {

	// 対象アニメーションのイベント表
	auto animIt = eventKeyTables_.find(animName);
	if (animIt == eventKeyTables_.end()) {
		return 0.0f;
	}

	// 種類ごとの配列
	const auto& kindMap = animIt->second;
	auto kindIt = kindMap.find(keyEvent);
	if (kindIt == kindMap.end()) {
		return 0.0f;
	}

	const auto& frames = kindIt->second;
	constexpr float kFps = 30.0f;
	float time = static_cast<float>(frames[frameIndex]) / kFps;
	return time;
}

void SkinnedAnimation::SetCurrentAnimTime(float time) {

	// 時間を設定
	updateModeIndex_ = static_cast<uint32_t>(updateMode_);
	currentAnimationTimers_[updateModeIndex_] = time;

	// 進行度を計算
	animationProgress_ = currentAnimationTimers_[updateModeIndex_] /
		animationData_[currentAnimationName_].duration;
}

std::vector<std::string> SkinnedAnimation::GetAnimationNames() const {

	std::vector<std::string> names;
	names.reserve(animationData_.size());
	for (const auto& data : animationData_) {

		names.push_back(data.first);
	}
	std::sort(names.begin(), names.end());
	return names;
}

const Transform3D* SkinnedAnimation::FindJointTransform(const std::string& name) const {

	auto it = skeleton_.jointMap.find(name);
	return (it == skeleton_.jointMap.end()) ? nullptr : &skeleton_.joints[it->second].transform;
}