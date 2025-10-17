#include "PlayerParryState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/PostEffect/RadialBlurUpdater.h>

//============================================================================
//	PlayerParryState classMethods
//============================================================================

PlayerParryState::PlayerParryState() {

	isEmitedBlur_ = false;

	// effect作成
	// エフェクト、エンジン機能変更中...
	/*parryEffect_ = std::make_unique<GameEffect>();
	parryEffect_->CreateParticleSystem("Particle/parryParticle.json");*/
}

void PlayerParryState::Enter(Player& player) {

	player.SetNextAnimation("player_parry", false, nextAnimDuration_);

	// 座標、向きを計算
	Vector3 direction = SetLerpValue(startPos_, targetPos_,
		player, parryLerp_.moveDistance, true);

	// 敵の方向を向かせる
	player.SetRotation(Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f)));
	// 左手の武器を反転
	player.SetReverseWeapon(true, PlayerWeaponType::Left);

	// deltaTimeをスケーリングしても元の値に戻らないようにする
	GameTimer::SetReturnScaleEnable(false);
	GameTimer::SetTimeScale(0.0f);

	// パリィ用のカメラアニメーションを設定
	followCamera_->StartPlayerActionAnim(PlayerState::Parry);

	canExit_ = false;
	isEmitedBlur_ = false;
	request_ = std::nullopt;
	parryLerp_.isFinised = false;
	attackLerp_.isFinised = false;
}

void PlayerParryState::Update(Player& player) {

	// deltaTimeの管理時間を更新
	UpdateDeltaWaitTime(player);

	// 入力状態を確認
	CheckInput();

	// 座標補間を更新
	UpdateLerpTranslation(player);

	// animationの更新
	UpdateAnimation(player);
}

void PlayerParryState::UpdateDeltaWaitTime(const Player& player) {

	// 時間経過を進める
	deltaWaitTimer_ += GameTimer::GetDeltaTime();
	// 時間経過が過ぎたら元に戻させる
	if (deltaWaitTime_ < deltaWaitTimer_) {

		GameTimer::SetReturnScaleEnable(true);

		// コマンドに設定
		// エフェクト、エンジン機能変更中...
		//ParticleCommand command{};
		//// 座標設定
		//command.target = ParticleCommandTarget::Spawner;
		//command.id = ParticleCommandID::SetTranslation;
		//command.value = player.GetJointWorldPos("leftHand");
		//parryEffect_->SendCommand(command);

		if (!isEmitedBlur_) {

			// 発生させる
			// エフェクト、エンジン機能変更中...
			//parryEffect_->Emit(true);

			// ブラー手の位置に発生させる
			postProcess_->Start(PostProcessType::RadialBlur);
			RadialBlurUpdater* blur = postProcess_->GetUpdater<RadialBlurUpdater>(
				PostProcessType::RadialBlur);
			// 自動で元の値に戻すように設定
			blur->StartState();
			blur->SetBlurType(RadialBlurType::Parry);
			blur->SetIsAutoReturn(true);

			// 腕の入りをスクリーン座標に変換して0.0f~1.0fの正規化する
			Vector2 screenPos = Math::ProjectToScreen(
				player.GetWeapon(PlayerWeaponType::Left)->GetTransform().GetWorldPos(), *followCamera_).Normalize();
			blur->SetBlurCenter(screenPos);

			// 発生済み
			isEmitedBlur_ = true;
		}
	}
}

void PlayerParryState::UpdateLerpTranslation(Player& player) {

	if (parryLerp_.isFinised) {
		return;
	}

	// 座標を補間
	Vector3 translation = GetLerpTranslation(parryLerp_);

	// 座標を設定
	player.SetTranslation(translation);
}

void PlayerParryState::CheckInput() {

	if (request_.has_value()) {
		return;
	}

	// deltaTimeが元に戻った後どうするかを入力確認
	// 攻撃を押していたらanimationが終了した後攻撃に移る
	if (allowAttack_ && inputMapper_->IsTriggered(PlayerInputAction::Attack)) {

		// 攻撃可能なら攻撃をリクエストする
		request_ = RequestState::PlayAnimation;
	}
}

void PlayerParryState::UpdateAnimation(Player& player) {

	// 座標補間が終了するまでなにもしない
	if (!parryLerp_.isFinised) {

		const bool hasAttackRequest = request_.has_value() && request_.value() == RequestState::PlayAnimation;
		const bool reachedHalf = (parryLerp_.time > 0.0f) && (parryLerp_.timer >= parryLerp_.time * 0.5f);
		// 半分以上時間経過していれば攻撃アニメーションを行えるようにする
		if (hasAttackRequest && reachedHalf) {

			parryLerp_.isFinised = true;
		} else {
			return;
		}
	}

	// 攻撃ボタンが押されていなければ状態を終了する
	if (!request_.has_value()) {

		canExit_ = true;
		// 元に戻す
		player.SetReverseWeapon(false, PlayerWeaponType::Left);

		// カメラの回転を戻す
		followCamera_->StartLookToTarget(FollowCameraTargetType::Player,
			FollowCameraTargetType::BossEnemy, true, true, std::nullopt, 0.88f);
		return;
	}
	switch (request_.value()) {
	case RequestState::PlayAnimation: {

		// 4段目の攻撃を再生させる
		player.SetNextAnimation("player_attack_4th", false, nextAnimDuration_);

		// 補間先の座標を再設定する
		SetLerpValue(startPos_, targetPos_,
			player, attackLerp_.moveDistance, false);

		request_ = RequestState::AttackAnimation;

		// カメラをパリィ攻撃用アニメーションにする
		followCamera_->EndPlayerActionAnim(PlayerState::Parry);
		// パリィ攻撃は4段目の攻撃と同じ
		followCamera_->StartPlayerActionAnim(PlayerState::Attack_4th);

		// 元に戻す
		player.SetReverseWeapon(false, PlayerWeaponType::Left);
		break;
	}
	case RequestState::AttackAnimation: {

		// 座標補間を行う
		Vector3 translation = GetLerpTranslation(attackLerp_);

		// 座標を設定
		player.SetTranslation(translation);

		// animationが再生し終わったら状態を終了させる
		if (player.IsAnimationFinished()) {

			request_ = std::nullopt;

			// 画面シェイクを行わせる
			followCamera_->SetScreenShake(true);
		}
		break;
	}
	}
}

Vector3 PlayerParryState::GetLerpTranslation(LerpParameter& lerp) {

	// 時間を進める
	lerp.timer += GameTimer::GetScaledDeltaTime();
	float lerpT = std::clamp(lerp.timer / lerp.time, 0.0f, 1.0f);
	lerpT = EasedValue(lerp.easingType, lerpT);

	// 座標を補間
	Vector3 translation = Vector3::Lerp(startPos_, targetPos_, lerpT);

	if (lerp.time < lerp.timer) {

		lerp.isFinised = true;
	}
	return translation;
}

Vector3 PlayerParryState::SetLerpValue(Vector3& start, Vector3& target,
	const Player& player, float moveDistance, bool isPlayerBase) {

	const Vector3 playerPos = player.GetTranslation();
	const Vector3 enemyPos = bossEnemy_->GetTranslation();
	// 向き
	Vector3 direction = enemyPos - playerPos;
	direction.y = 0.0f;
	direction = direction.Normalize();

	// 補間座標を設定する
	start = playerPos;
	if (isPlayerBase) {

		target = playerPos + direction * moveDistance;
	} else {

		target = enemyPos + direction * moveDistance;
	}
	return direction;
}

void PlayerParryState::Exit([[maybe_unused]] Player& player) {

	GameTimer::SetReturnScaleEnable(true);

	// リセット
	request_ = std::nullopt;
	deltaWaitTimer_ = 0.0f;
	parryLerp_.timer = 0.0f;
	attackLerp_.timer = 0.0f;
	parryLerp_.isFinised = false;
	attackLerp_.isFinised = false;
	canExit_ = false;
	allowAttack_ = false;
	isEmitedBlur_ = false;

	// エフェクト、エンジン機能変更中...
	//parryEffect_->ResetEmitFlag();
}

void PlayerParryState::ImGui(const Player& player) {

	ImGui::Text(std::format("allowAttack: {}", allowAttack_).c_str());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("deltaWaitTime", &deltaWaitTime_, 0.01f);

	ImGuiHelper::ValueText<Vector3>("stratPos", startPos_);
	ImGuiHelper::ValueText<Vector3>("targetPos", targetPos_);

	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	ImGui::SeparatorText("Parry: RED");

	ImGui::DragFloat("time##Parry", &parryLerp_.time, 0.01f);
	ImGui::DragFloat("moveDistance##Parry", &parryLerp_.moveDistance, 0.1f);
	Easing::SelectEasingType(parryLerp_.easingType, "Parry");

	{
		Vector3 start{};
		Vector3 target{};
		Vector3 translation = SetLerpValue(start, target,
			player, parryLerp_.moveDistance, true);
		start.y = 2.0f;
		target.y = 2.0f;

		lineRenderer->DrawLine3D(
			start, target, Color::Red());
	}

	ImGui::SeparatorText("Attack: YELLOW");

	ImGui::DragFloat("time##Attack", &attackLerp_.time, 0.01f);
	ImGui::DragFloat("moveDistance##Attack", &attackLerp_.moveDistance, 0.1f);
	Easing::SelectEasingType(attackLerp_.easingType, "Attack");

	{
		Vector3 start{};
		Vector3 target{};
		Vector3 translation = SetLerpValue(start, target,
			player, attackLerp_.moveDistance, false);
		start.y = 2.0f;
		target.y = 2.0f;

		lineRenderer->DrawLine3D(
			start, target, Color(0.0f, 1.0f, 1.0f, 1.0f));
	}
}

void PlayerParryState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	deltaWaitTime_ = JsonAdapter::GetValue<float>(data, "deltaWaitTime_");

	parryLerp_.time = JsonAdapter::GetValue<float>(data, "parryLerp_.time");
	parryLerp_.moveDistance = JsonAdapter::GetValue<float>(data, "parryLerp_.moveDistance");
	parryLerp_.easingType = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "parryLerp_.easingType"));

	attackLerp_.time = JsonAdapter::GetValue<float>(data, "attackLerp_.time");
	attackLerp_.moveDistance = JsonAdapter::GetValue<float>(data, "attackLerp_.moveDistance");
	attackLerp_.easingType = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "attackLerp_.easingType"));
}

void PlayerParryState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["deltaWaitTime_"] = deltaWaitTime_;

	data["parryLerp_.time"] = parryLerp_.time;
	data["parryLerp_.moveDistance"] = parryLerp_.moveDistance;
	data["parryLerp_.easingType"] = parryLerp_.easingType;

	data["attackLerp_.time"] = attackLerp_.time;
	data["attackLerp_.moveDistance"] = attackLerp_.moveDistance;
	data["attackLerp_.easingType"] = static_cast<int>(attackLerp_.easingType);
}