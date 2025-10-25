#include "BossEnemyBladeEffect.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Helper/GameEffectCommandHelper.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	BossEnemyBladeEffect classMethods
//============================================================================

void BossEnemySingleBladeEffect::Init(const BaseTransform& transform, const std::string& typeName) {

	// ファイルの名前を設定
	fileName_ = "Enemy/Boss/singleBladeEffect" + typeName + ".json";

	// エフェクト追加
	// 真ん中の刃
	slash_.effect = std::make_unique<GameEffect>();
	slash_.effect->CreateParticleSystem("Particle/bossEnemyChargeSlash.json");
	// 親を設定
	slash_.effect->SetParent(transform);

	// 光る部分
	plane_.effect = std::make_unique<GameEffect>();
	plane_.effect->CreateParticleSystem("Particle/bossEnemyChargeSlashCenterPlane.json");
	// 親を設定
	plane_.effect->SetParent(transform);

	// 後ろのパーティクル
	particle_.effect = std::make_unique<GameEffect>();
	particle_.effect->CreateParticleSystem("Particle/bossEnemyChargeSlashParticle.json");
	// 親を設定
	particle_.effect->SetParent(transform);

	// json適応
	ApplyJson();

	// 初期化値
	currentState_ = State::None;
	scalingValue_ = 1.0f;
}

void BossEnemySingleBladeEffect::EmitEffect(
	const BaseTransform& transform, float scalingValue) {

	// 座標、回転を設定して発生させる
	// オイラー角を取得
	Vector3 localRotation = Vector3::AnyInit(0.0f);
	// X軸を固定
	localRotation.x = pi / 2.0f;

	// 全てのエフェクトにスケーリングをかける
	GameEffectCommandHelper::SendScaling(*slash_.effect, scalingValue);
	GameEffectCommandHelper::SendScaling(*plane_.effect, scalingValue);
	GameEffectCommandHelper::SendScaling(*particle_.effect, scalingValue);

	// 刃
	GameEffectCommandHelper::ApplyAndSend(*slash_.effect, transform.rotation,
		slash_.translation, localRotation);
	slash_.effect->Emit();
	// 光る部分
	GameEffectCommandHelper::ApplyAndSend(*plane_.effect, transform.rotation,
		plane_.translation, localRotation);
	plane_.effect->Emit();
	// 後ろのパーティクル
	Quaternion inverseYRotation = Quaternion::MakeAxisAngle(Vector3(0.0f, 1.0f, 0.0f), -pi);
	GameEffectCommandHelper::SendSpawnerRotation(*particle_.effect,
		Quaternion::MakeRotateMatrix(Quaternion::Multiply(transform.rotation, inverseYRotation)));
	GameEffectCommandHelper::ApplyAndSend(*particle_.effect, transform.rotation,
		particle_.translation);

	// 状態を発生中に遷移
	emitTimer_.Reset();
	currentState_ = State::Emited;
}

void BossEnemySingleBladeEffect::Update() {

	switch (currentState_) {
	case BossEnemySingleBladeEffect::State::None: {
		break;
	}
	case BossEnemySingleBladeEffect::State::Emited: {

		// 一定間隔で発生
		particle_.effect->FrequencyEmit();

		// 発生時間を進める
		emitTimer_.Update();
		if (emitTimer_.IsReached()) {

			// 処理終了
			emitTimer_.Reset();
			currentState_ = State::None;
		}
		break;
	}
	}
}

void BossEnemySingleBladeEffect::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	if (ImGui::Button("Emit")) {
	}

	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));

	ImGui::DragFloat3("slashTranslation", &slash_.translation.x, 0.01f);
	ImGui::DragFloat3("planeTranslation", &plane_.translation.x, 0.01f);
	ImGui::DragFloat3("particleTranslation", &particle_.translation.x, 0.01f);

	emitTimer_.ImGui("EmitTimer");
}

void BossEnemySingleBladeEffect::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck(fileName_, data)) {
		return;
	}

	slash_.translation = Vector3::FromJson(data.value("slashTranslation", Json()));
	plane_.translation = Vector3::FromJson(data.value("planeTranslation", Json()));
	particle_.translation = Vector3::FromJson(data.value("particleTranslation", Json()));

	emitTimer_.FromJson(data["EmitTimer"]);
}

void BossEnemySingleBladeEffect::SaveJson() {

	Json data;

	data["slashTranslation"] = slash_.translation.ToJson();
	data["planeTranslation"] = plane_.translation.ToJson();
	data["particleTranslation"] = particle_.translation.ToJson();

	emitTimer_.ToJson(data["EmitTimer"]);

	JsonAdapter::Save(fileName_, data);
}