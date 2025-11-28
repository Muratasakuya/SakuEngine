#include "SubPlayerPunchAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	SubPlayerPunchAttackState classMethods
//============================================================================

SubPlayerPunchAttackState::SubPlayerPunchAttackState() {

	// 各パーツのキーフレーム移動の生成
	// 体
	bodyApproachKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	bodyApproachKeyframeObject_->Init("subPlayerPunchBodyApproachKey", "subPlayerBody");
	bodyLeaveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	bodyLeaveKeyframeObject_->Init("subPlayerPunchBodyLeaveKey", "subPlayerBody");
	// 右手
	rightHandApproachKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	rightHandApproachKeyframeObject_->Init("subPlayerPunchRightHandApproachKey", "subPlayerHand");
	rightHandLeaveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	rightHandLeaveKeyframeObject_->Init("subPlayerPunchRightHandLeaveKey", "subPlayerHand");
	// 左手
	leftHandApproachKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	leftHandApproachKeyframeObject_->Init("subPlayerPunchLeftHandApproachKey", "subPlayerHand");
	leftHandLeaveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	leftHandLeaveKeyframeObject_->Init("subPlayerPunchLeftHandLeaveKey", "subPlayerHand");

	// 追加のキー情報を追加
	bodyApproachKeyframeObject_->AddKeyValue(AnyMold::Color, addKeyColor);
	bodyLeaveKeyframeObject_->AddKeyValue(AnyMold::Color, addKeyColor);
	rightHandApproachKeyframeObject_->AddKeyValue(AnyMold::Color, addKeyColor);
	rightHandLeaveKeyframeObject_->AddKeyValue(AnyMold::Color, addKeyColor);
	leftHandApproachKeyframeObject_->AddKeyValue(AnyMold::Color, addKeyColor);
	leftHandLeaveKeyframeObject_->AddKeyValue(AnyMold::Color, addKeyColor);
}

void SubPlayerPunchAttackState::Enter() {

	// 攻撃処理開始
	canExit_ = false;
	currentState_ = State::Approach;

	// 最初の補間処理を開始
	bodyApproachKeyframeObject_->StartLerp();
	rightHandApproachKeyframeObject_->StartLerp();
	leftHandApproachKeyframeObject_->StartLerp();

	// 親子付け解除
	rightHand_->SetParent(Transform3D(), true);
	leftHand_->SetParent(Transform3D(), true);
}

void SubPlayerPunchAttackState::Update() {

	// 状態に応じて更新
	switch (currentState_) {
	case SubPlayerPunchAttackState::State::Approach:

		UpdateApproach();
		break;
	case SubPlayerPunchAttackState::State::Attack:

		UpdateAttack();
		break;
	case SubPlayerPunchAttackState::State::Leave:

		UpdateLeave();
		break;
	}
}

void SubPlayerPunchAttackState::UpdateApproach() {

	// 補間の更新を行って適応
	UpdateKeyAndApply(*bodyApproachKeyframeObject_,
		*rightHandApproachKeyframeObject_, *leftHandApproachKeyframeObject_);

	// 全ての補間が終了したら次の状態へ
	if (IsAllKeyframeEnd(*bodyApproachKeyframeObject_,
		*rightHandApproachKeyframeObject_, *leftHandApproachKeyframeObject_)) {

		// 状態終了可能にする
		canExit_ = true;

		//// 次の状態へ
		//currentState_ = State::Leave;
		//// 攻撃状態の補間開始
		//bodyLeaveKeyframeObject_->StartLerp();
		//rightHandLeaveKeyframeObject_->StartLerp();
		//leftHandLeaveKeyframeObject_->StartLerp();
	}
}

void SubPlayerPunchAttackState::UpdateAttack() {
}

void SubPlayerPunchAttackState::UpdateLeave() {

	// 補間の更新を行って適応
	UpdateKeyAndApply(*bodyLeaveKeyframeObject_,
		*rightHandLeaveKeyframeObject_, *leftHandLeaveKeyframeObject_);

	// 全ての補間が終了したら状態終了にする
	if (IsAllKeyframeEnd(*bodyLeaveKeyframeObject_,
		*rightHandLeaveKeyframeObject_, *leftHandLeaveKeyframeObject_)) {

		// 状態終了可能にする
		canExit_ = true;
	}
}

void SubPlayerPunchAttackState::UpdateAlways() {

	// 常にキーフレームオブジェクトを更新
	UpdateKeyframeObjects();
}

void SubPlayerPunchAttackState::Exit() {

	// リセットして終了
	canExit_ = false;
	ResetKeyframeObjects();

	// 親子付けを元に戻す
	rightHand_->SetParent(body_->GetTransform());
	leftHand_->SetParent(body_->GetTransform());
}

void SubPlayerPunchAttackState::UpdateKeyframeObjects() {

	// 常にキーフレームオブジェクトを更新
	// 体
	bodyApproachKeyframeObject_->UpdateKey();
	bodyLeaveKeyframeObject_->UpdateKey();
	// 右手
	rightHandApproachKeyframeObject_->UpdateKey();
	rightHandLeaveKeyframeObject_->UpdateKey();
	// 左手
	leftHandApproachKeyframeObject_->UpdateKey();
	leftHandLeaveKeyframeObject_->UpdateKey();
}

void SubPlayerPunchAttackState::ResetKeyframeObjects() {

	// キーオブジェクトのリセット
	// 体
	bodyApproachKeyframeObject_->Reset();
	bodyLeaveKeyframeObject_->Reset();
	// 右手
	rightHandApproachKeyframeObject_->Reset();
	rightHandLeaveKeyframeObject_->Reset();
	// 左手
	leftHandApproachKeyframeObject_->Reset();
	leftHandLeaveKeyframeObject_->Reset();
}

void SubPlayerPunchAttackState::UpdateKeyAndApply(KeyframeObject3D& bodyKeyframe,
	KeyframeObject3D& rightHandKeyframe, KeyframeObject3D& leftHandKeyframe) {

	// 補間の更新
	bodyKeyframe.SelfUpdate();
	rightHandKeyframe.SelfUpdate();
	leftHandKeyframe.SelfUpdate();

	// 補間した値を設定
	body_->SetSRT(bodyKeyframe.GetCurrentTransform());
	rightHand_->SetSRT(rightHandKeyframe.GetCurrentTransform());
	leftHand_->SetSRT(leftHandKeyframe.GetCurrentTransform());

	// 色
	body_->SetColor(std::get<Color>(bodyKeyframe.GetCurrentAnyValue(addKeyColor)));
	rightHand_->SetColor(std::get<Color>(rightHandKeyframe.GetCurrentAnyValue(addKeyColor)));
	leftHand_->SetColor(std::get<Color>(leftHandKeyframe.GetCurrentAnyValue(addKeyColor)));
}

bool SubPlayerPunchAttackState::IsAllKeyframeEnd(KeyframeObject3D& bodyKeyframe,
	KeyframeObject3D& rightHandKeyframe, KeyframeObject3D& leftHandKeyframe) {

	// 全ての補間処理が終了したかのチェック
	if (!bodyKeyframe.IsUpdating() &&
		!rightHandKeyframe.IsUpdating() &&
		!leftHandKeyframe.IsUpdating()) {
		return true;
	}
	return false;
}

void SubPlayerPunchAttackState::ImGui() {

	if (ImGui::BeginTabBar("SubPlayerPunchAttackState")) {
		if (ImGui::BeginTabItem("Approach")) {

			// 体
			if (ImGui::CollapsingHeader("Body")) {

				bodyApproachKeyframeObject_->ImGui();
			}
			ImGui::Spacing();
			// 右手
			if (ImGui::CollapsingHeader("RightHand")) {

				rightHandApproachKeyframeObject_->ImGui();
			}
			ImGui::Spacing();
			// 左手
			if (ImGui::CollapsingHeader("LeftHand")) {

				leftHandApproachKeyframeObject_->ImGui();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Attack")) {

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Leave")) {

			// 体
			if (ImGui::CollapsingHeader("Body")) {

				bodyLeaveKeyframeObject_->ImGui();
			}
			ImGui::Spacing();
			// 右手
			if (ImGui::CollapsingHeader("RightHand")) {

				rightHandLeaveKeyframeObject_->ImGui();
			}
			ImGui::Spacing();
			// 左手
			if (ImGui::CollapsingHeader("LeftHand")) {

				leftHandLeaveKeyframeObject_->ImGui();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void SubPlayerPunchAttackState::ApplyJson([[maybe_unused]] const Json& data) {

	// 近づく
	{
		bodyApproachKeyframeObject_->FromJson(data.value("BodyApproachKeyframeObject", Json()));
		rightHandApproachKeyframeObject_->FromJson(data.value("RightHandApproachKeyframeObject", Json()));
		leftHandApproachKeyframeObject_->FromJson(data.value("LeftHandApproachKeyframeObject", Json()));
	}
	// 攻撃
	{

	}
	// 離れる
	{
		bodyLeaveKeyframeObject_->FromJson(data.value("BodyLeaveKeyframeObject", Json()));
		rightHandLeaveKeyframeObject_->FromJson(data.value("RightHandLeaveKeyframeObject", Json()));
		leftHandLeaveKeyframeObject_->FromJson(data.value("LeftHandLeaveKeyframeObject", Json()));
	}
}

void SubPlayerPunchAttackState::SaveJson([[maybe_unused]] Json& data) {

	// 近づく
	{
		bodyApproachKeyframeObject_->ToJson(data["BodyApproachKeyframeObject"]);
		rightHandApproachKeyframeObject_->ToJson(data["RightHandApproachKeyframeObject"]);
		leftHandApproachKeyframeObject_->ToJson(data["LeftHandApproachKeyframeObject"]);
	}
	// 攻撃
	{

	}
	// 離れる
	{
		bodyLeaveKeyframeObject_->ToJson(data["BodyLeaveKeyframeObject"]);
		rightHandLeaveKeyframeObject_->ToJson(data["RightHandLeaveKeyframeObject"]);
		leftHandLeaveKeyframeObject_->ToJson(data["LeftHandLeaveKeyframeObject"]);
	}
}