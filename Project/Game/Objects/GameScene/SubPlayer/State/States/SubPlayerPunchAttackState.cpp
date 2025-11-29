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
	bodyApproachKeyframeObject_->Init("subPlayerPunchBodyApproachKey");
	bodyLeaveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	bodyLeaveKeyframeObject_->Init("subPlayerPunchBodyLeaveKey");
	// 右手
	rightHandApproachKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	rightHandApproachKeyframeObject_->Init("subPlayerPunchRightHandApproachKey");
	rightHandLeaveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	rightHandLeaveKeyframeObject_->Init("subPlayerPunchRightHandLeaveKey");
	// 左手
	leftHandApproachKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	leftHandApproachKeyframeObject_->Init("subPlayerPunchLeftHandApproachKey");
	leftHandLeaveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	leftHandLeaveKeyframeObject_->Init("subPlayerPunchLeftHandLeaveKey");

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

		// 次の状態へ
		currentState_ = State::Attack;

		// 攻撃状態の補間処理を開始
		// 体
		Quaternion plusRotation = Quaternion::MakeAxisAngle(Vector3(0.0f, 1.0f, 0.0f), bodyOffsetAngleY_);
		Quaternion minusRotation = Quaternion::MakeAxisAngle(Vector3(0.0f, 1.0f, 0.0f), -bodyOffsetAngleY_);
		enterBodyRotation_ = body_->GetRotation();
		bodyStartRotation_ = Quaternion::Normalize(Quaternion::Multiply(plusRotation, enterBodyRotation_));
		bodyTargetRotation_ = Quaternion::Normalize(Quaternion::Multiply(minusRotation, enterBodyRotation_));
		// 左手
		SetupAttackInfo(leftHandAttackInfo_, *leftHand_, false);
		// 右手
		SetupAttackInfo(rightHandAttackInfo_, *rightHand_, true);
		rightHandAttackInfo_.isActive = true;
		leftHandAttackDelayTimer_.Reset();
		chargeTimer_.Reset();
		chargeAttackTimer_.Reset();
	}
}

void SubPlayerPunchAttackState::UpdateAttack() {

	// 攻撃遅延時間更新
	leftHandAttackDelayTimer_.Update();
	// 時間経過で左手の補間を開始
	if (!leftHandAttackInfo_.isActive &&
		leftHandAttackDelayTimer_.IsReached()) {

		leftHandAttackInfo_.isActive = true;
	}

	// アクティブな時のみタイマーで補間
	LerpAttackHand(rightHandAttackInfo_, *rightHand_);
	LerpAttackHand(leftHandAttackInfo_, *leftHand_);

	// 左手の攻撃が完了していなければ
	if (!leftHandAttackInfo_.timer.IsReached()) {

		// 体の回転を補間
		float currentT = leftHandAttackInfo_.loop.LoopedT(leftHandAttackInfo_.timer.t_);
		Quaternion rotation = Quaternion::Slerp(bodyStartRotation_,
			bodyTargetRotation_, EasedValue(leftHandAttackInfo_.timer.easeingType_, currentT));

		// 回転を適応
		body_->SetRotation(Quaternion::Normalize(rotation));
		return;
	}
	// 補間終了
	rightHandAttackInfo_.isActive = false;
	leftHandAttackInfo_.isActive = false;
	body_->SetRotation(enterBodyRotation_);

	// 溜め時間更新
	chargeTimer_.Update();

	// 溜め補間
	LerpChargeHand(rightHandAttackInfo_, *rightHand_);
	LerpChargeHand(leftHandAttackInfo_, *leftHand_);

	// 溜めが終了後殴る
	if (!chargeTimer_.IsReached()) {
		return;
	}

	// 溜め後の攻撃時間更新
	chargeAttackTimer_.Update();

	// 溜め後の攻撃補間、逆向きに補間する
	// 右手
	{
		Vector3 lerpPos = Vector3::Lerp(rightHandAttackInfo_.chargeTargetPos,
			rightHandAttackInfo_.chargeStartPos, chargeAttackTimer_.easedT_);
		rightHand_->SetTranslation(lerpPos);
	}
	// 左手
	{
		Vector3 lerpPos = Vector3::Lerp(leftHandAttackInfo_.chargeTargetPos,
			leftHandAttackInfo_.chargeStartPos, chargeAttackTimer_.easedT_);
		leftHand_->SetTranslation(lerpPos);
	}

	// 溜め後の攻撃が終了したら次の状態へ
	if (chargeAttackTimer_.IsReached()) {

		// 次の状態へ
		currentState_ = State::Leave;
		// 離脱の補間処理を開始
		bodyLeaveKeyframeObject_->StartLerp();
		rightHandLeaveKeyframeObject_->StartLerp();
		leftHandLeaveKeyframeObject_->StartLerp();
	}
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

void SubPlayerPunchAttackState::LerpAttackHand(AttackInfo& attackInfo, GameObject3D& hand) {

	// アクティブな時のみ補間
	if (!attackInfo.isActive) {
		return;
	}

	// 時間を更新
	attackInfo.timer.Update(attackDuration_);
	float currentT = attackInfo.loop.LoopedT(attackInfo.timer.t_);

	// 座標を補間
	Vector3 lerpPos = Vector3::Lerp(attackInfo.startPos,
		attackInfo.targetPos, EasedValue(attackInfo.timer.easeingType_, currentT));

	// トランスフォームに適応
	hand.SetTranslation(lerpPos);
}

void SubPlayerPunchAttackState::LerpChargeHand(AttackInfo& attackInfo, GameObject3D& hand) {

	// 溜め時間が終了していれば補間しない
	if (chargeTimer_.IsReached()) {
		return;
	}

	// 座標を補間
	Vector3 lerpPos = Vector3::Lerp(attackInfo.chargeStartPos,
		attackInfo.chargeTargetPos, chargeTimer_.easedT_);

	// トランスフォームに適応
	hand.SetTranslation(lerpPos);
}

void SubPlayerPunchAttackState::SetupAttackInfo(AttackInfo& attackInfo,
	const GameObject3D& hand, bool isAttackHand) {

	// 敵への向き
	Vector3 handPos = hand.GetTransform().GetWorldPos();
	Vector3 bossEnemyPos = bossEnemy_->GetTranslation();
	bossEnemyPos.y = bossEnemyOffsetY_;
	Vector3 direction = Vector3(bossEnemyPos - handPos).Normalize();

	// 補間座標を設定する
	// 開始位置
	attackInfo.startPos = handPos;
	// 目標位置
	attackInfo.targetPos = handPos + direction * attackMoveDistance_;

	// 攻撃する方の手かどうかで分ける
	if (isAttackHand) {

		// 溜め開始位置、少しだけ手前に下げる
		attackInfo.chargeStartPos = attackInfo.startPos;
		// 溜め目標位置
		attackInfo.chargeTargetPos = attackInfo.targetPos;
	} else {

		// 目標位置と開始位置を逆にして補間させる
		attackInfo.chargeStartPos = attackInfo.targetPos;
		attackInfo.chargeTargetPos = attackInfo.startPos;
	}

	// ループ回数のセットとタイマーの初期化
	attackInfo.loop.SetLoopCount(attackCount_);
	attackInfo.timer.Reset();
	attackInfo.isActive = false;
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

			// 攻撃に関する設定
			int32_t attackCount = static_cast<int32_t>(attackCount_);
			if (ImGui::DragInt("attackCount", &attackCount, 1, 0)) {

				attackCount_ = static_cast<uint32_t>(attackCount);
			}
			ImGui::DragFloat("attackDuration", &attackDuration_, 0.01f);
			ImGui::DragFloat("attackMoveDistance", &attackMoveDistance_, 0.1f);
			ImGui::DragFloat("bossEnemyOffsetY", &bossEnemyOffsetY_, 0.1f);
			ImGui::DragFloat("bodyOffsetAngleY", &bodyOffsetAngleY_, 0.01f);
			leftHandAttackDelayTimer_.ImGui("leftHandAttackDelay");
			chargeTimer_.ImGui("chargeTimer");
			chargeAttackTimer_.ImGui("chargeAttackTimer");

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
		attackCount_ = data.value("AttackCount", 3u);
		attackDuration_ = data.value("AttackDuration", 0.5f);
		attackMoveDistance_ = data.value("AttackMoveDistance", 2.0f);
		bossEnemyOffsetY_ = data.value("bossEnemyOffsetY_", 0.5f);
		bodyOffsetAngleY_ = data.value("bodyOffsetAngleY_", 0.5f);
		leftHandAttackDelayTimer_.FromJson(data.value("LeftHandAttackDelayTimer", Json()));
		chargeTimer_.FromJson(data.value("ChargeTimer", Json()));
		chargeAttackTimer_.FromJson(data.value("ChargeAttackTimer", Json()));
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
		data["AttackCount"] = attackCount_;
		data["AttackDuration"] = attackDuration_;
		data["AttackMoveDistance"] = attackMoveDistance_;
		data["bossEnemyOffsetY_"] = bossEnemyOffsetY_;
		data["bodyOffsetAngleY_"] = bodyOffsetAngleY_;
		leftHandAttackDelayTimer_.ToJson(data["LeftHandAttackDelayTimer"]);
		chargeTimer_.ToJson(data["ChargeTimer"]);
		chargeAttackTimer_.ToJson(data["ChargeAttackTimer"]);
	}
	// 離れる
	{
		bodyLeaveKeyframeObject_->ToJson(data["BodyLeaveKeyframeObject"]);
		rightHandLeaveKeyframeObject_->ToJson(data["RightHandLeaveKeyframeObject"]);
		leftHandLeaveKeyframeObject_->ToJson(data["LeftHandLeaveKeyframeObject"]);
	}
}