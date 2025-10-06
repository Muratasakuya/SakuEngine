#include "FollowCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Editor/Camera/3D/Camera3DEditor.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/MathLib/MathUtils.h>

//============================================================================
//	FollowCamera classMethods
//============================================================================

void FollowCamera::LoadAnim() {

	if (isLoadedAnim_) {
		return;
	}

	Camera3DEditor* editor = Camera3DEditor::GetInstance();
	// プレイヤーの攻撃
	editor->LoadAnimFile("2ndAttackCamera.json");
	editor->LoadAnimFile("3rdAttackCamera.json");
	editor->LoadAnimFile("4thAttackCamera.json");
	editor->LoadAnimFile("skillAttackCamera.json");

	// 読み込み済み
	isLoadedAnim_ = true;
}

void FollowCamera::StartPlayerActionAnim(PlayerState state) {

	Camera3DEditor* editor = Camera3DEditor::GetInstance();
	switch (state) {
	case PlayerState::Attack_2nd:

		editor->StartAnim("AttackProgress_2nd", true);
		break;
	case PlayerState::Attack_3rd:

		editor->StartAnim("AttackProgress_3rd", true);
		break;
	case PlayerState::Attack_4th:

		editor->StartAnim("AttackProgress_4th", true);
		break;
	case PlayerState::SkilAttack:

		editor->StartAnim("SkillProgress", true);
		break;
	}
}

void FollowCamera::EndPlayerActionAnim(PlayerState state) {

	Camera3DEditor* editor = Camera3DEditor::GetInstance();
	switch (state) {
	case PlayerState::Attack_2nd:

		editor->EndAnim("AttackProgress_2nd");
		break;
	case PlayerState::Attack_3rd:

		editor->EndAnim("AttackProgress_3rd");
		break;
	case PlayerState::Attack_4th:

		editor->EndAnim("AttackProgress_4th");
		break;
	case PlayerState::SkilAttack:

		editor->EndAnim("SkillProgress");
		break;
	}
}

void FollowCamera::StartLookToTarget(FollowCameraTargetType from,
	FollowCameraTargetType to, bool isReset, bool isLockTarget, float lookTimerRate) {

	// 処理中なら受け付けない
	if (lookStart_ && !isReset) {
		return;
	}

	// 補間開始
	lookStart_ = true;
	lookTimer_.Reset();
	lookTimerRate_ = lookTimerRate;
	lookPair_.first = from;
	lookPair_.second = to;

	// 開始回転を設定
	lookToStart_ = Quaternion::Normalize(transform_.rotation);
	lookToTarget_ = std::nullopt;

	// 目標回転を固定するなら
	if (isLockTarget) {

		// 目標回転を設定
		lookToTarget_ = GetTargetRotation();
	}
}

void FollowCamera::Init() {

	displayFrustum_ = false;
	isLoadedAnim_ = false;

	// json適応
	ApplyJson();

	stateController_ = std::make_unique<FollowCameraStateController>();
	stateController_->Init(*this);

	// 行列更新
	BaseCamera::UpdateView();
}

void FollowCamera::SetScreenShake(bool isShake) {

	if (isShake) {

		// 状態を設定する
		stateController_->SetOverlayState(*this, FollowCameraOverlayState::Shake);
	} else {

		// shakeを止める
		stateController_->ExitOverlayState(FollowCameraOverlayState::Shake);
	}
}

void FollowCamera::SetParry(bool isParry) {

	if (isParry) {

		// 状態を設定する
		stateController_->SetOverlayState(*this, FollowCameraOverlayState::Parry);
	} else {

		// parryを止める
		stateController_->ExitOverlayState(FollowCameraOverlayState::Parry);
	}
}

void FollowCamera::SetParryAttack(bool isParry) {

	if (isParry) {

		// 状態を設定する
		stateController_->SetOverlayState(*this, FollowCameraOverlayState::ParryAttack);
	} else {

		// parryを止める
		stateController_->ExitOverlayState(FollowCameraOverlayState::ParryAttack);
	}
}

void FollowCamera::SetTarget(FollowCameraTargetType type, const Transform3D& target) {

	stateController_->SetTarget(type, target);
	targets_[type] = &target;
}

void FollowCamera::SetState(FollowCameraState state) {

	stateController_->SetState(state);
}

void FollowCamera::Update() {

	// 状態の更新
	stateController_->Update(*this);

	// エディターで更新しているときは処理しない
	if (isUpdateEditor_) {
		return;
	}

	// 視点を注視点に向ける処理
	UpdateLookToTarget();

	// 行列更新
	BaseCamera::UpdateView(UpdateMode::Quaternion);
}

void FollowCamera::UpdateLookToTarget() {

	if (!lookStart_) {
		return;
	}

	// 目標回転
	Quaternion targetRotation = lookToTarget_.has_value() ?
		lookToTarget_.value() : GetTargetRotation();

	// 時間の更新
	lookTimer_.Update(lookTimer_.target_ * lookTimerRate_);

	// 回転補間
	Quaternion rotation = Quaternion::Slerp(lookToStart_,
		targetRotation, lookTimer_.easedT_);
	transform_.rotation = Quaternion::Normalize(rotation);

	// 時間経過で終了
	if (lookTimer_.IsReached()) {

		// 回転を固定する
		transform_.rotation = targetRotation;
		lookStart_ = false;
		lookTimer_.Reset();
		lookToTarget_ = std::nullopt;
	}
}

Quaternion FollowCamera::GetTargetRotation() const {

	// 注視点に向ける
	// 視点と注視点
	const Transform3D fromTransform = *targets_.at(lookPair_.first);
	const Transform3D toTransform = *targets_.at(lookPair_.second);

	// ワールド座標
	const Vector3 fromPos = fromTransform.GetWorldPos();
	const Vector3 toPos = toTransform.GetWorldPos();

	// 水平のみの前方ベクトル方向を取得
	Vector3 forward = toPos - fromPos;
	forward.y = 0.0f;
	forward = forward.Normalize();

	// Y軸の回転
	Quaternion yawRotation = Quaternion::LookRotation(
		forward, Direction::Get(Direction3D::Up));
	// X軸回転
	Vector3 rightAxis = (yawRotation * Direction::Get(Direction3D::Right)).Normalize();
	Quaternion pitchRotation = Quaternion::Normalize(Quaternion::MakeAxisAngle(
		rightAxis, targetXRotation_));
	// 目標回転
	Quaternion targetRotation = Quaternion::Normalize(pitchRotation * yawRotation);
	return targetRotation;
}

void FollowCamera::ImGui() {

	ImGui::SetWindowFontScale(0.8f);
	ImGui::PushItemWidth(itemWidth_);

	if (ImGui::BeginTabBar("FollowCameraTabs")) {
		if (ImGui::BeginTabItem("Init")) {
			if (ImGui::Button("Save Json", ImVec2(itemWidth_, 32.0f))) {

				SaveJson();
			}

			ImGui::DragFloat3("translation", &transform_.translation.x, 0.01f);
			ImGui::DragFloat3("rotation", &transform_.eulerRotate.x, 0.01f);

			ImGui::DragFloat("fovY", &fovY_, 0.01f);
			ImGui::DragFloat("nearClip", &nearClip_, 0.001f);
			ImGui::DragFloat("farClip", &farClip_, 1.0f);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("State")) {

			stateController_->ImGui(*this);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("LookTarget")) {

			if (ImGui::Button("Start")) {

				StartLookToTarget(FollowCameraTargetType::Player,
					FollowCameraTargetType::BossEnemy, true);
			}

			ImGui::DragFloat("targetXRotation", &targetXRotation_, 0.01f);
			lookTimer_.ImGui("Timer", true);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::SetWindowFontScale(1.0f);
	ImGui::PopItemWidth();
}

void FollowCamera::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Follow/initParameter.json", data)) {
		return;
	}

	fovY_ = JsonAdapter::GetValue<float>(data, "fovY_");
	nearClip_ = JsonAdapter::GetValue<float>(data, "nearClip_");
	farClip_ = JsonAdapter::GetValue<float>(data, "farClip_");

	targetXRotation_ = data.value("targetXRotation_", 0.4f);
	lookTimer_.FromJson(data.value("LookTimer", Json()));
}

void FollowCamera::SaveJson() {

	Json data;

	data["fovY_"] = fovY_;
	data["nearClip_"] = nearClip_;
	data["farClip_"] = farClip_;

	data["targetXRotation_"] = targetXRotation_;
	lookTimer_.ToJson(data["LookTimer"]);

	JsonAdapter::Save("Camera/Follow/initParameter.json", data);
}