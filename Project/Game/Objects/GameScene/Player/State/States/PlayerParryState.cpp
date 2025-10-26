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

	// エフェクト作成
	parryEffect_ = std::make_unique<EffectGroup>();
	parryEffect_->Init("parryHitEffect", "PlayerEffect");
	parryEffect_->LoadJson("GameEffectGroup/Player/playerParryHitEffect.json");
}

void PlayerParryState::Enter(Player& player) {

	player.SetNextAnimation("player_parry", false, nextAnimDuration_);

	// 座標、向きを計算
	Vector3 direction = SetLerpValue(startPos_, targetPos_,
		player, parryLerp_.moveDistance, true);
	direction.y = 0.0f;
	direction = direction.Normalize();

	// 敵の方向を向かせる
	player.SetRotation(Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f)));
	// 左手の武器を反転
	player.SetReverseWeapon(true, PlayerWeaponType::Left);

	// deltaTimeをスケーリングしても元の値に戻らないようにする
	GameTimer::SetReturnScaleEnable(false);
	GameTimer::SetTimeScale(0.0f);
	GameTimer::SetLerpSpeed(deltaLerpSpeed_);

	// パリィ用のカメラアニメーションを設定
	followCamera_->StartPlayerActionAnim(PlayerState::Parry);

	canExit_ = false;
	isEmitedBlur_ = false;
	request_ = std::nullopt;
	parryLerp_.isFinised = false;
	attackLerp_.isFinised = false;
	deltaWaitTimer_ = 0.0f;
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

void PlayerParryState::UpdateAlways(Player& player) {

	// エフェクトの更新
	parryEffect_->Update();
}

void PlayerParryState::UpdateDeltaWaitTime(const Player& player) {

	// 時間経過を進める
	deltaWaitTimer_ += GameTimer::GetDeltaTime();
	// 時間経過が過ぎたら元に戻させる
	if (deltaWaitTime_ < deltaWaitTimer_) {

		GameTimer::SetReturnScaleEnable(true);

		if (!isEmitedBlur_) {

			// 左手にエフェクトを発生させる
			// 発生座標
			Vector3 effectPos = player.GetJointWorldPos("leftHand");
			effectPos.y = parryEffectPosY_;
			parryEffect_->Emit(effectPos);

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

	// 補間終了後アングル固定を解除
	if (parryLerp_.isFinised) {

		// 攻撃入力がされていなければ滑らかに元のカメラに戻るようにする
		if (!request_.has_value()) {

			followCamera_->WarmStart();
		}
	}
}

void PlayerParryState::CheckInput() {

	// 座標補間が終了したら入力を受け付けない
	if (parryLerp_.isFinised) {
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
		return;
	}

	// 攻撃ボタンが押されていなければ状態を終了する
	if (!request_.has_value()) {

		canExit_ = true;
		// 元に戻す
		player.SetReverseWeapon(false, PlayerWeaponType::Left);
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

		// カメラアニメーションを終了させる
		followCamera_->EndPlayerActionAnim(PlayerState::Parry, true);
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
			followCamera_->SetOverlayState(FollowCameraOverlayState::Shake, true);
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

	Vector3 playerPos = player.GetTranslation();
	playerPos.y = 0.0f;
	Vector3 enemyPos = bossEnemy_->GetTranslation();
	enemyPos.y = 0.0f;
	// 向き
	Vector3 direction = enemyPos - playerPos;
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

	// カメラアニメーションを終了させる
	followCamera_->EndPlayerActionAnim(PlayerState::Parry, false);

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
}

void PlayerParryState::ImGui(const Player& player) {

	ImGui::Text(std::format("allowAttack: {}", allowAttack_).c_str());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("deltaWaitTime", &deltaWaitTime_, 0.01f);
	ImGui::DragFloat("deltaLerpSpeed_", &deltaLerpSpeed_, 0.01f);
	ImGui::DragFloat("cameraLookRate", &cameraLookRate_, 0.01f);
	ImGui::DragFloat("parryEffectPosY", &parryEffectPosY_, 0.01f);

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
	deltaLerpSpeed_ = data.value("deltaLerpSpeed_", 8.0f);
	cameraLookRate_ = data.value("cameraLookRate_", 1.0f);
	parryEffectPosY_ = data.value("parryEffectPosY_", 4.0f);

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
	data["deltaLerpSpeed_"] = deltaLerpSpeed_;
	data["cameraLookRate_"] = cameraLookRate_;
	data["parryEffectPosY_"] = parryEffectPosY_;

	data["parryLerp_.time"] = parryLerp_.time;
	data["parryLerp_.moveDistance"] = parryLerp_.moveDistance;
	data["parryLerp_.easingType"] = parryLerp_.easingType;

	data["attackLerp_.time"] = attackLerp_.time;
	data["attackLerp_.moveDistance"] = attackLerp_.moveDistance;
	data["attackLerp_.easingType"] = static_cast<int>(attackLerp_.easingType);
}