#include "BossEnemyAnimationEffect.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Effect/Game/Helper/GameEffectCommandHelper.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyAnimationEffect classMethods
//============================================================================

void BossEnemyAnimationEffect::Init(const BossEnemy& bossEnemy) {

	// エフェクト追加
	// 弱攻撃
	lightSlash_.effect = std::make_unique<GameEffect>();
	lightSlash_.effect->CreateParticleSystem("Particle/bossEnemyLightSlash.json");
	// 親を設定
	lightSlash_.effect->SetParent(bossEnemy.GetTransform());

	// 強攻撃
	strongSlash_.effect = std::make_unique<GameEffect>();
	strongSlash_.effect->CreateParticleSystem("Particle/bossEnemyStrongSlash.json");
	// 親を設定
	strongSlash_.effect->SetParent(bossEnemy.GetTransform());

	// チャージ
	// 星
	chargeStar_.effect = std::make_unique<GameEffect>();
	chargeStar_.effect->CreateParticleSystem("Particle/bossEnemyChargeStar.json");
	// 親を設定
	chargeStar_.effect->SetParent(bossEnemy.GetTransform());
	// 集まってくるエフェクト
	chargeCircle_.effect = std::make_unique<GameEffect>();
	chargeCircle_.effect->CreateParticleSystem("Particle/bossEnemyChargeCircle.json");
	// 親を設定
	chargeCircle_.effect->SetParent(bossEnemy.GetTransform());
	// 攻撃発生
	chargeEmit_.effect = std::make_unique<GameEffect>();
	chargeEmit_.effect->CreateParticleSystem("Particle/bossEnemyChargeEmit.json");
	// 親を設定
	chargeEmit_.effect->SetParent(bossEnemy.GetTransform());

	// 移動時の巻き風
	moveWind_.effect = std::make_unique<GameEffect>();
	moveWind_.effect->CreateParticleSystem("Particle/bossEnemyMoveWind.json");
	// 親を設定
	moveWind_.effect->SetParent(bossEnemy.GetTransform());

	// json適応
	ApplyJson();

	// 初期化値
	currentAnimationKey_ = AnimationKey::None;
	editAnimationKey_ = AnimationKey::None;
}

void BossEnemyAnimationEffect::SetFollowCamera(const FollowCamera* followCamera) {

	followCamera_ = nullptr;
	followCamera_ = followCamera;
}

void BossEnemyAnimationEffect::Update(BossEnemy& bossEnemy) {

	// 再生されているアニメーションを取得
	UpdateAnimationKey(bossEnemy);

	// 現在のアニメーションに応じてエフェクトを発生
	UpdateEmit(bossEnemy);

	// 常に更新するエフェクト
	UpdateAlways();
}

void BossEnemyAnimationEffect::UpdateAnimationKey(BossEnemy& bossEnemy) {

	const auto& name = bossEnemy.GetCurrentAnimationName();

	currentAnimationKey_ = AnimationKey::None;
	if (name == "bossEnemy_lightAttackParrySign" ||
		name == "bossEnemy_strongAttackParrySign") {

		currentAnimationKey_ = AnimationKey::Move;
	} else if (name == "bossEnemy_lightAttack") {

		currentAnimationKey_ = AnimationKey::LightAttack;
	} else if (name == "bossEnemy_strongAttack") {

		currentAnimationKey_ = AnimationKey::StrongAttack;
	} else if (name == "bossEnemy_chargeAttack") {

		currentAnimationKey_ = AnimationKey::ChargeAttack;
	} else if (name == "bossEnemy_continuousAttack") {

		currentAnimationKey_ = AnimationKey::ContinuousAttack;
	}
}

void BossEnemyAnimationEffect::UpdateEmit(BossEnemy& bossEnemy) {

	// 現在のアニメーションに応じてエフェクトを発生させる
	switch (currentAnimationKey_) {
	case BossEnemyAnimationEffect::AnimationKey::None: {

		// エフェクトの発生をリセット
		moveWind_.emitEnble = true;
		continuousEventIndex_ = 0;
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::Move: {

		if (!moveWind_.emitEnble) {
			return;
		}

		// この状態の間は一定間隔で発生させる
		GameEffectCommandHelper::ApplyAndSend(*moveWind_.effect, bossEnemy.GetRotation(),
			moveWind_.translation);
		moveWind_.effect->FrequencyEmit();
		// 指定キーイベントで発生を止める
		if (bossEnemy.IsEventKey("Effect", 0)) {

			moveWind_.emitEnble = false;
		}
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::LightAttack: {

		if (bossEnemy.IsEventKey("Effect", 0)) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*lightSlash_.effect, 1.0f);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*lightSlash_.effect, bossEnemy.GetRotation(),
				lightSlash_.translation, lightSlash_.rotation);
			lightSlash_.effect->Emit();
		}
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::StrongAttack: {

		if (bossEnemy.IsEventKey("Effect", 0)) {

			// スケーリング
			GameEffectCommandHelper::SendScaling(*strongSlash_.effect, 1.0f);

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*strongSlash_.effect, bossEnemy.GetRotation(),
				strongSlash_.translation, strongSlash_.rotation);
			strongSlash_.effect->Emit();
		}
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::ChargeAttack: {

		if (bossEnemy.IsEventKey("Effect", 0)) {

			// 発生させる
			EmitChargeEffect(bossEnemy);
		}
		if (bossEnemy.IsEventKey("Effect", 1)) {

			// 発生させる
			GameEffectCommandHelper::ApplyAndSend(*chargeEmit_.effect, bossEnemy.GetRotation(),
				chargeEmit_.translation);
			chargeEmit_.effect->Emit();
		}
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::ContinuousAttack: {

		// イベントが発生するごとにインデックスを進める
		if (bossEnemy.IsEventKey("Effect", continuousEventIndex_)) {

			// 発生させる
			EmitContinuousEffect(bossEnemy, continuousSlashParams_[continuousEventIndex_]);
			// インデックスを進める
			++continuousEventIndex_;
		}
		break;
	}
	}
}

void BossEnemyAnimationEffect::UpdateAlways() {

	// 集まってくるエフェクト
	GameEffectCommandHelper::SendSpawnerBillboard(*chargeCircle_.effect,
		static_cast<const BaseCamera&>(*followCamera_));
	chargeCircle_.effect->Emit();
}

void BossEnemyAnimationEffect::EmitChargeEffect(const BossEnemy& bossEnemy) {

	// 座標、コマンドを設定
	// 星
	GameEffectCommandHelper::ApplyAndSend(*chargeStar_.effect, bossEnemy.GetRotation(),
		chargeStar_.translation);
	chargeStar_.effect->Emit();

	// 集まってくるエフェクト
	GameEffectCommandHelper::ApplyAndSend(*chargeCircle_.effect, bossEnemy.GetRotation(),
		chargeCircle_.translation);
	// フラグで発生
	GameEffectCommandHelper::SendSpawnerEmit(*chargeCircle_.effect, true);
}

void BossEnemyAnimationEffect::EmitContinuousEffect(const BossEnemy& bossEnemy, const ContinuousSlash& param) {

	switch (param.slashType) {
	case SlashType::Light: {

		// スケーリング
		GameEffectCommandHelper::SendScaling(*strongSlash_.effect, param.scaling);

		// 座標回転、コマンドをセット
		GameEffectCommandHelper::ApplyAndSend(*strongSlash_.effect, bossEnemy.GetRotation(),
			param.translation, param.rotation);
		strongSlash_.effect->Emit();
		break;
	}
	case SlashType::Strong: {

		// スケーリング
		GameEffectCommandHelper::SendScaling(*lightSlash_.effect, param.scaling);

		// 座標回転、コマンドをセット
		GameEffectCommandHelper::ApplyAndSend(*lightSlash_.effect, bossEnemy.GetRotation(),
			param.translation, param.rotation);
		lightSlash_.effect->Emit();
		break;
	}
	}
}

void BossEnemyAnimationEffect::ImGui(const BossEnemy& bossEnemy) {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::Text("currentKey: %s", EnumAdapter<AnimationKey>::ToString(currentAnimationKey_));
	EnumAdapter<AnimationKey>::Combo("AnimationKey", &editAnimationKey_);

	switch (editAnimationKey_) {
	case BossEnemyAnimationEffect::AnimationKey::Move: {

		ImGui::DragFloat3("windTranslation", &moveWind_.translation.x, 0.01f);
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::LightAttack: {

		if (ImGui::Button("Emit")) {

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*lightSlash_.effect, bossEnemy.GetRotation(),
				lightSlash_.translation, lightSlash_.rotation);
			lightSlash_.effect->Emit();
		}

		ImGui::DragFloat3("rotation", &lightSlash_.rotation.x, 0.01f);
		ImGui::DragFloat3("translation", &lightSlash_.translation.x, 0.01f);
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::StrongAttack: {

		if (ImGui::Button("Emit")) {

			// 座標回転、コマンドをセット
			GameEffectCommandHelper::ApplyAndSend(*strongSlash_.effect, bossEnemy.GetRotation(),
				strongSlash_.translation, strongSlash_.rotation);
			strongSlash_.effect->Emit();
		}

		ImGui::DragFloat3("rotation", &strongSlash_.rotation.x, 0.01f);
		ImGui::DragFloat3("translation", &strongSlash_.translation.x, 0.01f);
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::ChargeAttack: {

		if (ImGui::Button("Emit")) {

			EmitChargeEffect(bossEnemy);
		}

		ImGui::DragFloat3("starTranslation", &chargeStar_.translation.x, 0.01f);
		ImGui::DragFloat3("circleTranslation", &chargeCircle_.translation.x, 0.01f);
		ImGui::DragFloat3("emitTranslation", &chargeEmit_.translation.x, 0.01f);
		break;
	}
	case BossEnemyAnimationEffect::AnimationKey::ContinuousAttack: {

		for (uint32_t index = 0; index < continuousCount_; ++index) {

			ImGui::PushID(index);

			ImGui::SeparatorText(("animIndex: " + std::to_string(index)).c_str());

			auto& param = continuousSlashParams_[index];
			if (ImGui::Button("Emit")) {

				EmitContinuousEffect(bossEnemy, param);
			}

			EnumAdapter<SlashType>::Combo("SlashType", &param.slashType);
			ImGui::DragFloat("scaling", &param.scaling, 0.01f);
			ImGui::DragFloat3("rotation", &param.rotation.x, 0.01f);
			ImGui::DragFloat3("translation", &param.translation.x, 0.01f);

			ImGui::PopID();
		}
		break;
	}
	}
}

void BossEnemyAnimationEffect::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Enemy/Boss/animationEffectEmit.json", data)) {
		return;
	}

	auto key = EnumAdapter<AnimationKey>::ToString(AnimationKey::LightAttack);
	lightSlash_.translation = Vector3::FromJson(data[key].value("translation", Json()));
	lightSlash_.rotation = Vector3::FromJson(data[key].value("rotation", Json()));

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::StrongAttack);
	if (data.contains(key)) {

		strongSlash_.translation = Vector3::FromJson(data[key].value("translation", Json()));
		strongSlash_.rotation = Vector3::FromJson(data[key].value("rotation", Json()));
	}

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::ChargeAttack);
	if (data.contains(key)) {

		chargeStar_.translation = Vector3::FromJson(data[key].value("starTranslation", Json()));
		chargeCircle_.translation = Vector3::FromJson(data[key].value("circleTranslation", Json()));
		chargeEmit_.translation = Vector3::FromJson(data[key].value("emitTranslation", Json()));
	}

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Move);
	if (data.contains(key)) {

		moveWind_.translation = Vector3::FromJson(data[key].value("translation", Json()));
	}

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::ContinuousAttack);
	if (data.contains(key)) {

		for (uint32_t index = 0; index < continuousCount_; ++index) {

			auto& param = continuousSlashParams_[index];
			std::string keyIndex = std::to_string(index);

			const auto& slash = EnumAdapter<SlashType>::FromString(data[key][keyIndex]["slashType"]);
			param.slashType = slash.value();
			param.scaling = data[key][keyIndex].value("scaling", 1.0f);
			param.translation = Vector3::FromJson(data[key][keyIndex].value("translation", Json()));
			param.rotation = Vector3::FromJson(data[key][keyIndex].value("rotation", Json()));
		}
	}
}

void BossEnemyAnimationEffect::SaveJson() {

	Json data;

	auto key = EnumAdapter<AnimationKey>::ToString(AnimationKey::LightAttack);
	data[key]["translation"] = lightSlash_.translation.ToJson();
	data[key]["rotation"] = lightSlash_.rotation.ToJson();

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::StrongAttack);
	data[key]["translation"] = strongSlash_.translation.ToJson();
	data[key]["rotation"] = strongSlash_.rotation.ToJson();

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::ChargeAttack);
	data[key]["starTranslation"] = chargeStar_.translation.ToJson();
	data[key]["circleTranslation"] = chargeCircle_.translation.ToJson();
	data[key]["emitTranslation"] = chargeEmit_.translation.ToJson();

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::Move);
	data[key]["translation"] = moveWind_.translation.ToJson();

	key = EnumAdapter<AnimationKey>::ToString(AnimationKey::ContinuousAttack);
	for (uint32_t index = 0; index < continuousCount_; ++index) {

		auto& param = continuousSlashParams_[index];
		std::string keyIndex = std::to_string(index);

		data[key][keyIndex]["slashType"] = EnumAdapter<SlashType>::ToString(param.slashType);
		data[key][keyIndex]["scaling"] = param.scaling;
		data[key][keyIndex]["translation"] = param.translation.ToJson();
		data[key][keyIndex]["rotation"] = param.rotation.ToJson();
	}

	JsonAdapter::Save("Enemy/Boss/animationEffectEmit.json", data);
}