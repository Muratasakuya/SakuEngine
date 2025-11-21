#include "FollowCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Editor/Camera/CameraEditor.h>
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

	CameraEditor* editor = CameraEditor::GetInstance();
	// プレイヤーの攻撃
	editor->LoadJson("Player/player3rdAttack.json");
	editor->LoadJson("Player/player4thAttack.json");
	editor->LoadJson("Player/playerParryRight.json");
	editor->LoadJson("Player/playerParryLeft.json");
	editor->LoadJson("Player/playerSkilMove.json");
	editor->LoadJson("Player/playerSkilJump.json");

	// 読み込み済み
	isLoadedAnim_ = true;
}

void FollowCamera::StartPlayerActionAnim(PlayerState state) {

	CameraEditor* editor = CameraEditor::GetInstance();

	// 状態毎に名前を取得
	std::string name{};
	switch (state) {
	case PlayerState::Attack_3rd: name = "player3rdAttack"; break;
	case PlayerState::Attack_4th: name = "player4thAttack"; break;
	case PlayerState::Parry:

		// 目標回転
		const Quaternion baseTarget = targets_[FollowCameraTargetType::BossEnemy]->rotation;
		// y軸最短補間方向
		int direction = Math::YawShortestDirection(transform_.rotation, baseTarget);

		// 0か-1なら左、+1は右からの視点のパリィアニメーションを行わせる
		name = (0 <= direction) ? "playerParryRight" : "playerParryLeft";
		break;
	}

	// 名前が設定されていればアニメーションを再生
	if (!name.empty()) {

		editor->StartAnim(name, true);
	}
}

void FollowCamera::StartPlayerActionAnim(const std::string& animName) {

	CameraEditor* editor = CameraEditor::GetInstance();

	// 名前が設定されていればアニメーションを再生
	if (!animName.empty()) {

		editor->StartAnim(animName, true);
	}
}

void FollowCamera::EndPlayerActionAnim(bool isWarmStart) {

	CameraEditor* editor = CameraEditor::GetInstance();

	// アニメーションを終了させる
	editor->EndAnim();
	if (isWarmStart) {

		stateController_->WarmStartFollow(*this);
	}
}

void FollowCamera::StartLookToTarget(FollowCameraTargetType from,
	FollowCameraTargetType to, bool isReset, bool isLockTarget,
	std::optional<float> targetXRotation, float lookTimerRate) {

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
	startFovY_ = fovY_;

	// 開始回転を設定
	lookToStart_ = Quaternion::Normalize(transform_.rotation);
	lookToTarget_ = std::nullopt;
	anyTargetXRotation_ = std::nullopt;

	// 任意のX軸回転が設定されていれば
	if (targetXRotation.has_value()) {

		anyTargetXRotation_ = targetXRotation.value();
	}

	// 目標回転を固定するなら
	if (isLockTarget) {

		// 目標回転を設定
		lookToTarget_ = GetTargetRotation();
	}

	// 目標回転
	const Quaternion baseTarget = lookToTarget_.has_value()
		? lookToTarget_.value() : GetTargetRotation();

	// Y軸回転の差
	const float yawDelta = Math::YawSignedDelta(lookToStart_, baseTarget);
	const float kDeadZone = 0.008f;
	if (std::abs(yawDelta) <= kDeadZone) {

		lookYawDirection_ = (preLookYawDirection_ != 0) ? preLookYawDirection_ : +1;
	} else {

		lookYawDirection_ = (0.0f < yawDelta) ? +1 : -1;
	}
	preLookYawDirection_ = lookYawDirection_;
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

void FollowCamera::SetOverlayState(FollowCameraOverlayState state, bool isStart) {

	if (isStart) {

		// 状態を設定する
		stateController_->SetOverlayState(*this, state);
	} else {

		// 状態を止める
		stateController_->ExitOverlayState(state);
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
	UpdateLookAlwaysTarget();

	// 行列更新
	BaseCamera::UpdateView(UpdateMode::Quaternion);
}

void FollowCamera::UpdateLookToTarget() {

	if (!lookStart_) {
		return;
	}

	// 目標回転
	Quaternion baseTarget = lookToTarget_.has_value() ?
		lookToTarget_.value() : GetTargetRotation();

	// 最短のY軸補間方向を取得
	// 0オフセットなし、1左周り、2右回り
	float signedOffset = -static_cast<float>(lookYawDirection_) * lookYawOffset_;

	// Y軸にオフセットをかけて目標回転を算出
	Quaternion yawOffset = Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Up), signedOffset);
	Quaternion targetRotation = Quaternion::Normalize(yawOffset * baseTarget);

	// 時間の更新
	lookTimer_.Update(lookTimer_.target_ * lookTimerRate_);

	// 回転補間
	Quaternion rotation = Quaternion::Slerp(lookToStart_,
		targetRotation, lookTimer_.easedT_);
	transform_.rotation = Quaternion::Normalize(rotation);

	fovY_ = std::lerp(startFovY_, initFovY_, lookTimer_.easedT_);

	// 時間経過で終了
	if (lookTimer_.IsReached()) {

		// 回転を固定する
		transform_.rotation = targetRotation;
		lookStart_ = false;
		lookToTarget_ = std::nullopt;
	}
}

void FollowCamera::UpdateLookAlwaysTarget() {

	if (!lookAlwaysTarget_) {
		return;
	}

	// 視点と注視点は固定
	lookPair_.first = FollowCameraTargetType::Player;
	lookPair_.second = FollowCameraTargetType::BossEnemy;

	// 目標回転
	Quaternion targetRotation = GetTargetRotation();

	// 回転をフレーム補間
	Quaternion rotation = Quaternion::Slerp(
		transform_.rotation, targetRotation, lookTargetLerpRate_);
	transform_.rotation = Quaternion::Normalize(rotation);
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

	float targetXRotation = anyTargetXRotation_.has_value() ? anyTargetXRotation_.value() : targetXRotation_;
	// 0.0f以下にはならないようにする
	targetXRotation = (std::max)(0.0f, targetXRotation);

	// Y軸の回転
	Quaternion yawRotation = Quaternion::LookRotation(
		forward, Direction::Get(Direction3D::Up));
	// X軸回転
	Vector3 rightAxis = (yawRotation * Direction::Get(Direction3D::Right)).Normalize();
	Quaternion pitchRotation = Quaternion::Normalize(Quaternion::MakeAxisAngle(
		rightAxis, targetXRotation));
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
			ImGui::Text("rotation: %.2f, %.2f, %.2f, %.2f", transform_.rotation.x,
				transform_.rotation.y, transform_.rotation.z, transform_.rotation.w);

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

			ImGui::Text(std::format("lookTimer_.IsReached(): {}", lookTimer_.IsReached()).c_str());

			ImGui::Checkbox("lookAlwaysTarget", &lookAlwaysTarget_);

			ImGui::DragFloat("targetXRotation", &targetXRotation_, 0.01f);
			ImGui::DragFloat("lookTargetLerpRate", &lookTargetLerpRate_, 0.01f);
			ImGui::DragFloat("lookYawOffset", &lookYawOffset_, 0.01f);
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
	initFovY_ = fovY_;
	nearClip_ = JsonAdapter::GetValue<float>(data, "nearClip_");
	farClip_ = JsonAdapter::GetValue<float>(data, "farClip_");

	targetXRotation_ = data.value("targetXRotation_", 0.4f);
	lookTargetLerpRate_ = data.value("lookTargetLerpRate_", 0.4f);
	lookYawOffset_ = data.value("lookYawOffset_", 0.01f);
	lookTimer_.FromJson(data.value("LookTimer", Json()));
}

void FollowCamera::SaveJson() {

	Json data;

	data["fovY_"] = fovY_;
	data["nearClip_"] = nearClip_;
	data["farClip_"] = farClip_;

	data["targetXRotation_"] = targetXRotation_;
	data["lookTargetLerpRate_"] = lookTargetLerpRate_;
	data["lookYawOffset_"] = lookYawOffset_;
	lookTimer_.ToJson(data["LookTimer"]);

	JsonAdapter::Save("Camera/Follow/initParameter.json", data);
}