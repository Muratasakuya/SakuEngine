#include "PlayerAnimationEffect.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Helper/GameEffectCommandHelper.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerAnimationEffect classMethods
//============================================================================

void PlayerAnimationEffect::Init(const Player& player) {

	// エフェクト追加
	// 斬撃
	slashEffect_ = std::make_unique<GameEffect>();
	slashEffect_->CreateParticleSystem("Particle/playerSlash.json");
	// 親を設定
	slashEffect_->SetParent(player.GetTransform());
	// 斬撃
	rotateSlashEffect_ = std::make_unique<GameEffect>();
	rotateSlashEffect_->CreateParticleSystem("Particle/playerRotateSlash.json");
	// 親を設定
	rotateSlashEffect_->SetParent(player.GetTransform());
	// 回転する剣の周り
	// 左
	leftSword_.slashEffect = std::make_unique<GameEffect>();
	leftSword_.slashEffect->CreateParticleSystem("Particle/playerLeftSwordSlash.json");
	leftSword_.sparkEffect = std::make_unique<GameEffect>();
	leftSword_.sparkEffect->CreateParticleSystem("Particle/rotateDispersionParticle_0.json");
	// 右
	rightSword_.slashEffect = std::make_unique<GameEffect>();
	rightSword_.slashEffect->CreateParticleSystem("Particle/playerRightSwordSlash.json");
	rightSword_.sparkEffect = std::make_unique<GameEffect>();
	rightSword_.sparkEffect->CreateParticleSystem("Particle/rotateDispersionParticle_1.json");
	// 親を設定
	leftSword_.slashEffect->SetParent(
		player.GetWeapon(PlayerWeaponType::Left)->GetTransform());
	leftSword_.sparkEffect->SetParent(
		player.GetWeapon(PlayerWeaponType::Left)->GetTransform());
	rightSword_.slashEffect->SetParent(
		player.GetWeapon(PlayerWeaponType::Right)->GetTransform());
	rightSword_.sparkEffect->SetParent(
		player.GetWeapon(PlayerWeaponType::Right)->GetTransform());

	// 地割れ
	groundEffect_ = std::make_unique<GameEffect>();
	groundEffect_->CreateParticleSystem("Particle/playerAttackGround.json");
	// 親を設定
	groundEffect_->SetParent(player.GetTransform());

	// json適応
	ApplyJson();

	// 初期化値
	currentAnimationKey_ = AnimationKey::None;
	editAnimationKey_ = AnimationKey::None;
}

void PlayerAnimationEffect::Update(Player& player) {

	// 再生されているアニメーションを取得
	UpdateAnimationKey(player);

	// 現在のアニメーションに応じてエフェクトを発生
	UpdateEmit(player);

	// 常に更新するエフェクト
	UpdateAlways();
}

void PlayerAnimationEffect::UpdateAnimationKey(Player& player) {

	const auto& name = player.GetCurrentAnimationName();
	currentAnimationKey_ = AnimationKey::None;
	if (name == "player_attack_1st") {

		currentAnimationKey_ = AnimationKey::Attack_1st;
		// エフェクトの発生をリセット
		secondAttackEventIndex_ = 0;
	} else if (name == "player_attack_2nd") {

		currentAnimationKey_ = AnimationKey::Attack_2nd;
	} else if (name == "player_attack_3rd") {

		currentAnimationKey_ = AnimationKey::Attack_3rd;
	} else if (name == "player_attack_4th") {

		currentAnimationKey_ = AnimationKey::Attack_4th;
	} else if (name == "player_skilAttack") {

		currentAnimationKey_ = AnimationKey::Skil;
	}
}

void PlayerAnimationEffect::UpdateEmit(Player& player) {

	// 現在のアニメーションに応じてエフェクトを発生させる
	switch (currentAnimationKey_) {
	case PlayerAnimationEffect::AnimationKey::None: {

		// エフェクトの発生をリセット
		secondAttackEventIndex_ = 0;
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Attack_1st: {

		if (player.IsEventKey("Effect", 0)) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*slashEffect_, firstSlashParam_.scaling);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*slashEffect_, player.GetRotation(),
				firstSlashParam_.translation, firstSlashParam_.rotation);
			slashEffect_->Emit();
		}
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Attack_2nd: {

		if (player.IsEventKey("Effect", secondAttackEventIndex_)) {

			auto& param = secondSlashParams_[secondAttackEventIndex_];

			// スケーリング
			GameEffectCommandHelper::SendScaling(*slashEffect_, param.scaling);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*slashEffect_, player.GetRotation(),
				param.translation, param.rotation);
			slashEffect_->Emit();
			// インデックスを進める
			++secondAttackEventIndex_;
		}
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Attack_3rd: {

		// 左手
		if (player.IsEventKey("Effect", 0)) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*leftSword_.slashEffect, thirdSlashParam_.scaling);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*leftSword_.slashEffect, player.GetRotation(),
				thirdSlashParam_.translation, thirdSlashParam_.rotation);
			leftSword_.slashEffect->Emit();

			// 火花
			GameEffectCommandHelper::ApplyAndSend(*leftSword_.sparkEffect, player.GetRotation(),
				thirdParticleTranslation_);
			// フラグで発生
			GameEffectCommandHelper::SendSpawnerEmit(*leftSword_.sparkEffect, true);
		}
		// 右手
		if (player.IsEventKey("Effect", 1)) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*rightSword_.slashEffect, thirdSlashParam_.scaling);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*rightSword_.slashEffect, player.GetRotation(),
				thirdSlashParam_.translation, thirdSlashParam_.rotation);
			rightSword_.slashEffect->Emit();

			// 火花
			GameEffectCommandHelper::ApplyAndSend(*rightSword_.sparkEffect, player.GetRotation(),
				thirdParticleTranslation_);
			// フラグで発生
			GameEffectCommandHelper::SendSpawnerEmit(*rightSword_.sparkEffect, true);
		}

		// 集まってくるエフェクト
		leftSword_.sparkEffect->Emit();
		rightSword_.sparkEffect->Emit();
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Attack_4th: {

		// 状態が切り替わった瞬間のみ
		if (currentAnimationKey_ != preAnimationKey_) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*rotateSlashEffect_, fourthSlashParam_.scaling);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*rotateSlashEffect_, player.GetRotation(),
				fourthSlashParam_.translation, fourthSlashParam_.rotation);
			rotateSlashEffect_->Emit();
		}

		// 地割れ
		if (player.IsEventKey("Effect", 1)) {

			// 座標を設定
			GameEffectCommandHelper::ApplyAndSend(*groundEffect_, player.GetRotation(),
				fourthGroundTranslation_);
			groundEffect_->Emit();
		}
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Skil: {
		break;
	}
	}
}

void PlayerAnimationEffect::UpdateAlways() {

	// 前回のフレームの値を保持
	preAnimationKey_ = currentAnimationKey_;
}

void PlayerAnimationEffect::ImGui(const Player& player) {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::Text("currentKey: %s", EnumAdapter<AnimationKey>::ToString(currentAnimationKey_));
	EnumAdapter<AnimationKey>::Combo("AnimationKey", &editAnimationKey_);

	switch (editAnimationKey_) {
	case PlayerAnimationEffect::AnimationKey::Attack_1st: {

		if (ImGui::Button("Emit")) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*slashEffect_, firstSlashParam_.scaling);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*slashEffect_, player.GetRotation(),
				firstSlashParam_.translation, firstSlashParam_.rotation);
			slashEffect_->Emit();
		}

		ImGui::DragFloat("scaling", &firstSlashParam_.scaling, 0.01f);
		ImGui::DragFloat3("rotation", &firstSlashParam_.rotation.x, 0.01f);
		ImGui::DragFloat3("translation", &firstSlashParam_.translation.x, 0.01f);
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Attack_2nd: {

		for (uint32_t index = 0; index < secondSlashCount_; ++index) {

			ImGui::PushID(index);

			ImGui::SeparatorText(("animIndex: " + std::to_string(index)).c_str());

			auto& param = secondSlashParams_[index];
			if (ImGui::Button("Emit")) {

				// スケーリング
				GameEffectCommandHelper::SendScaling(*slashEffect_, param.scaling);

				// 座標回転、コマンドをセット
				GameEffectCommandHelper::ApplyAndSend(*slashEffect_, player.GetRotation(),
					param.translation, param.rotation);
				slashEffect_->Emit();
			}

			ImGui::DragFloat("scaling", &param.scaling, 0.01f);
			ImGui::DragFloat3("rotation", &param.rotation.x, 0.01f);
			ImGui::DragFloat3("translation", &param.translation.x, 0.01f);

			ImGui::PopID();
		}
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Attack_3rd: {

		if (ImGui::Button("Emit")) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*leftSword_.slashEffect, thirdSlashParam_.scaling);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*leftSword_.slashEffect, player.GetRotation(),
				thirdSlashParam_.translation, thirdSlashParam_.rotation);
			leftSword_.slashEffect->Emit();

			// 火花
			GameEffectCommandHelper::ApplyAndSend(*leftSword_.sparkEffect, player.GetRotation(),
				thirdParticleTranslation_);
			// フラグで発生
			GameEffectCommandHelper::SendSpawnerEmit(*leftSword_.sparkEffect, true);
		}

		ImGui::DragFloat("scaling", &thirdSlashParam_.scaling, 0.01f);
		ImGui::DragFloat3("rotation", &thirdSlashParam_.rotation.x, 0.01f);
		ImGui::DragFloat3("translation", &thirdSlashParam_.translation.x, 0.01f);
		ImGui::DragFloat3("particleTranslation", &thirdParticleTranslation_.x, 0.01f);
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Attack_4th: {

		if (ImGui::Button("Emit")) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*rotateSlashEffect_, fourthSlashParam_.scaling);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*rotateSlashEffect_, player.GetRotation(),
				fourthSlashParam_.translation, fourthSlashParam_.rotation);
			rotateSlashEffect_->Emit();
		}

		ImGui::DragFloat("scaling", &fourthSlashParam_.scaling, 0.01f);
		ImGui::DragFloat3("rotation", &fourthSlashParam_.rotation.x, 0.01f);
		ImGui::DragFloat3("translation", &fourthSlashParam_.translation.x, 0.01f);
		ImGui::DragFloat3("groundTranslation", &fourthGroundTranslation_.x, 0.01f);
		break;
	}
	case PlayerAnimationEffect::AnimationKey::Skil: {
		break;
	}
	}
}

void PlayerAnimationEffect::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Player/animationEffectEmit.json", data)) {
		return;
	}

	auto key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Attack_1st);
	firstSlashParam_.scaling = data[key].value("scaling", 1.0f);
	firstSlashParam_.translation = Vector3::FromJson(data[key].value("translation", Json()));
	firstSlashParam_.rotation = Vector3::FromJson(data[key].value("rotation", Json()));

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Attack_2nd);
	for (uint32_t index = 0; index < secondSlashCount_; ++index) {

		auto& param = secondSlashParams_[index];
		std::string keyIndex = std::to_string(index);

		param.scaling = data[key][keyIndex].value("scaling", 1.0f);
		param.translation = Vector3::FromJson(data[key][keyIndex].value("translation", Json()));
		param.rotation = Vector3::FromJson(data[key][keyIndex].value("rotation", Json()));
	}

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Attack_3rd);
	if (data.contains(key)) {

		thirdSlashParam_.scaling = data[key].value("scaling", 1.0f);
		thirdSlashParam_.translation = Vector3::FromJson(data[key].value("translation", Json()));
		thirdSlashParam_.rotation = Vector3::FromJson(data[key].value("rotation", Json()));
		thirdParticleTranslation_ = Vector3::FromJson(data[key].value("thirdParticleTranslation_", Json()));
	}

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Attack_4th);
	if (data.contains(key)) {

		fourthSlashParam_.scaling = data[key].value("scaling", 1.0f);
		fourthSlashParam_.translation = Vector3::FromJson(data[key].value("translation", Json()));
		fourthSlashParam_.rotation = Vector3::FromJson(data[key].value("rotation", Json()));
		fourthGroundTranslation_ = Vector3::FromJson(data[key].value("fourthGroundTranslation_", Json()));
	}
}

void PlayerAnimationEffect::SaveJson() {

	Json data;

	auto key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Attack_1st);
	data[key]["scaling"] = firstSlashParam_.scaling;
	data[key]["translation"] = firstSlashParam_.translation.ToJson();
	data[key]["rotation"] = firstSlashParam_.rotation.ToJson();

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Attack_2nd);
	for (uint32_t index = 0; index < secondSlashCount_; ++index) {

		auto& param = secondSlashParams_[index];
		std::string keyIndex = std::to_string(index);

		data[key][keyIndex]["scaling"] = param.scaling;
		data[key][keyIndex]["translation"] = param.translation.ToJson();
		data[key][keyIndex]["rotation"] = param.rotation.ToJson();
	}

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Attack_3rd);
	data[key]["scaling"] = thirdSlashParam_.scaling;
	data[key]["translation"] = thirdSlashParam_.translation.ToJson();
	data[key]["rotation"] = thirdSlashParam_.rotation.ToJson();
	data[key]["thirdParticleTranslation_"] = thirdParticleTranslation_.ToJson();

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Attack_4th);
	data[key]["scaling"] = fourthSlashParam_.scaling;
	data[key]["translation"] = fourthSlashParam_.translation.ToJson();
	data[key]["rotation"] = fourthSlashParam_.rotation.ToJson();
	data[key]["fourthGroundTranslation_"] = fourthGroundTranslation_.ToJson();

	JsonAdapter::Save("Player/animationEffectEmit.json", data);
}