#include "PlayerBaseAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/3D/Camera3DEditor.h>
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	PlayerBaseAttackState classMethods
//============================================================================

void PlayerBaseAttackState::UpdateTimer(StateTimer& timer) {

	// 外部による更新がないときのみ
	if (!externalActive_) {

		timer.Update();
	}
}

void PlayerBaseAttackState::AttackAssist(Player& player, bool onceTarget) {

	// 時間経過
	attackPosLerpTimer_ += GameTimer::GetScaledDeltaTime();
	float lerpT = std::clamp(attackPosLerpTimer_ / attackPosLerpTime_, 0.0f, 1.0f);
	lerpT = EasedValue(attackPosEaseType_, lerpT);

	// 座標、距離を取得
	const Vector3 playerPos = player.GetTranslation();
	const Vector3 enemyPos = bossEnemy_->GetTranslation();
	const float distance = (enemyPos - playerPos).Length();

	// 指定円の中に敵がいれば座標を補完する
	Vector3 direction = (enemyPos - playerPos).Normalize();

	// 補間先を設定
	if (onceTarget) {
		if (!targetTranslation_.has_value() &&
			!targetRotation_.has_value()) {

			targetTranslation_ = enemyPos - direction * attackOffsetTranslation_;
			targetRotation_ = Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
		}
	} else {

		targetTranslation_ = enemyPos - direction * attackOffsetTranslation_;
		targetRotation_ = Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
	}

	// 指定円の中に敵がいれば敵の座標まで補間する
	if (CheckInRange(attackPosLerpCircleRange_, distance)) {

		// 補間先
		Vector3 translation = Vector3::Lerp(playerPos, *targetTranslation_, std::clamp(lerpT, 0.0f, 1.0f));
		player.SetTranslation(translation);
	}

	// 指定円の中に敵がいれば敵の方向に向かせる
	if (CheckInRange(attackLookAtCircleRange_, distance)) {

		Quaternion currentRotation = player.GetRotation();
		player.SetRotation(Quaternion::Slerp(currentRotation, *targetRotation_, std::clamp(rotationLerpRate_, 0.0f, 1.0f)));
	}
}

bool PlayerBaseAttackState::CheckInRange(float range, float distance) {

	bool result = range > epsilon_ && distance <= range;
	return result;
}

Vector3 PlayerBaseAttackState::GetPlayerOffsetPos(
	const Player& player, const Vector3& offsetTranslation) const {

	Vector3 offset = Vector3::Transform(offsetTranslation,
		Quaternion::MakeRotateMatrix(player.GetRotation()));

	return player.GetTranslation() + offset;
}

Matrix4x4 PlayerBaseAttackState::GetPlayerOffsetRotation(
	const Player& player, const Vector3& offsetRotation) const {

	// playerの回転
	Matrix4x4 playerRotation = Quaternion::MakeRotateMatrix(player.GetRotation());
	// オフセット回転
	Matrix4x4 offsetMatrix = Matrix4x4::MakeRotateMatrix(offsetRotation);

	return playerRotation * offsetMatrix;
}

void PlayerBaseAttackState::SetTimerByOverall(StateTimer& timer, float overall,
	float start, float end, EasingType easing) {

	timer.t_ = Algorithm::MapOverallToLocal(overall, start, end);
	timer.easedT_ = EasedValue(easing, timer.t_);
}

void PlayerBaseAttackState::DrawAttackOffset(const Player& player) {

	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	// 座標、距離を取得
	Vector3 playerPos = player.GetTranslation();
	playerPos.y = 2.0f;
	Vector3 enemyPos = bossEnemy_->GetTranslation();
	enemyPos.y = 2.0f;
	Vector3 direction = (enemyPos - playerPos).Normalize();
	Vector3 target = enemyPos - direction * attackOffsetTranslation_;

	lineRenderer->DrawLine3D(playerPos, target, Color::Red());
	lineRenderer->DrawSphere(8, 2.0f, target, Color::Red());
}

void PlayerBaseAttackState::ResetTarget() {

	// 補間目標をリセット
	attackPosLerpTimer_ = 0.0f;
	targetTranslation_ = std::nullopt;
	targetRotation_ = std::nullopt;
}

void PlayerBaseAttackState::ImGui(const Player& player) {

	ImGui::DragFloat("attackPosLerpCircleRange", &attackPosLerpCircleRange_, 0.1f);
	ImGui::DragFloat("attackLookAtCircleRange", &attackLookAtCircleRange_, 0.1f);
	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("attackPosLerpTime", &attackPosLerpTime_, 0.01f);
	Easing::SelectEasingType(attackPosEaseType_);

	DrawAttackOffset(player);

	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	Vector3 center = player.GetTranslation();
	center.y = 4.0f;
	lineRenderer->DrawCircle(8, attackPosLerpCircleRange_, center, Color::Red());
	lineRenderer->DrawCircle(8, attackLookAtCircleRange_, center, Color::Blue());
}

void PlayerBaseAttackState::ApplyJson(const Json& data) {

	attackPosLerpCircleRange_ = JsonAdapter::GetValue<float>(data, "attackPosLerpCircleRange_");
	attackLookAtCircleRange_ = JsonAdapter::GetValue<float>(data, "attackLookAtCircleRange_");
	attackOffsetTranslation_ = JsonAdapter::GetValue<float>(data, "attackOffsetTranslation_");
	attackPosLerpTime_ = JsonAdapter::GetValue<float>(data, "attackPosLerpTime_");
	attackPosEaseType_ = static_cast<EasingType>(JsonAdapter::GetValue<int>(data, "attackPosEaseType_"));
}

void PlayerBaseAttackState::SaveJson(Json& data) {

	data["attackPosLerpCircleRange_"] = attackPosLerpCircleRange_;
	data["attackLookAtCircleRange_"] = attackLookAtCircleRange_;
	data["attackOffsetTranslation_"] = attackOffsetTranslation_;
	data["attackPosLerpTime_"] = attackPosLerpTime_;
	data["attackPosEaseType_"] = static_cast<int>(attackPosEaseType_);
}

int PlayerBaseAttackState::AddActionObject(const std::string& name) {

	Camera3DEditor::GetInstance()->AddActionObject(name);
	return ActionProgressMonitor::GetInstance()->AddObject(name);
}

void PlayerBaseAttackState::SetSynchObject(int objectID) {

	// IDを設定
	editObjectID_ = objectID;

	// エディターの同期設定の共有
	ActionProgressMonitor::GetInstance()->SetSynchToggleHandler(
		objectID, [this](bool external) {

			externalActive_ = external;
			if (external) {
				player_->SetUpdateMode(ObjectUpdateMode::External);
			} else {
				player_->SetUpdateMode(ObjectUpdateMode::None);
			}
		});
}