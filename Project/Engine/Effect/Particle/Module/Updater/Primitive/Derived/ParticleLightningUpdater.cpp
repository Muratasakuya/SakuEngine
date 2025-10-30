#include "ParticleLightningUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

//============================================================================
//	ParticleLightningUpdater classMethods
//============================================================================

void ParticleLightningUpdater::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			// 親として動かす座標
			parentTranslation_ = *translation;
		}
		break;
	}
	case ParticleCommandID::SetLightningSegment: {
		if (const auto& segment = std::get_if<std::vector<Vector3>>(&command.value)) {

			// 2点なければ処理しない
			if (segment->size() != 2) {
				return;
			}
			// 開始地点
			start_.start = segment->front();
			target_.start = segment->front();
			// 最終地点
			start_.end = segment->back();
			target_.end = segment->back();
		}
		break;
	}
	}
}

void ParticleLightningUpdater::Init() {

	// 初期化値
	start_.Init();
	target_.Init();

	isLookAtEnd_ = false;
	isDrawDebugPoint_ = false;
	isRefSpawnPos_ = false;
	parentTranslation_ = Vector3::AnyInit(0.0f);
}

void ParticleLightningUpdater::Update(CPUParticle::ParticleData& particle, EasingType easingType) {

	LightningForGPU& lightning = particle.primitive.lightning;
	const float lifeProgress = particle.progress;

	// 親座標を考慮した位置を取得する
	auto GetParentedPos = [&](const Vector3& pos) {
		// 発生地点を参照する場合
		if (isRefSpawnPos_) {

			return pos + parentTranslation_ + particle.spawnTranlation;
		}
		return pos + parentTranslation_; };

	// 値の補間
	// 開始地点
	lightning.start = Vector3::Lerp(GetParentedPos(start_.start),
		GetParentedPos(target_.start), EasedValue(easingType, lifeProgress));
	// 終了地点
	lightning.end = Vector3::Lerp(GetParentedPos(start_.end),
		GetParentedPos(target_.end), EasedValue(easingType, lifeProgress));
	// 進行方向に雷の終了地点を回転させる
	if (isLookAtEnd_) {

		Vector3 base = lightning.end - lightning.start; // 現在の向き
		Vector3 velocity = particle.velocity;           // 目標の向き

		float baseLength = base.Length();
		float velocityLength = velocity.Length();
		// 両方とも長さが十分にある場合のみ回転を行う
		if (baseLength > std::numeric_limits<float>::epsilon() &&
			velocityLength > std::numeric_limits<float>::epsilon()) {

			// 開始地点から終了地点へのベクトルを、速度ベクトルの方向に回転させる
			Quaternion rotation = Quaternion::FromToRotation(base, velocity);
			Vector3 rotated = rotation * base;

			// 終了座標を回転後の位置に更新
			lightning.end = lightning.start + rotated;
		}
	}

	// 幅
	lightning.width = std::lerp(start_.width, target_.width, EasedValue(easingType, lifeProgress));

	// ノード数の設定
	lightning.nodeCount = (start_.nodeCount + target_.nodeCount) / 2;

	lightning.amplitudeRatio = std::lerp(start_.amplitudeRatio, target_.amplitudeRatio, EasedValue(easingType, lifeProgress));
	lightning.frequency = std::lerp(start_.frequency, target_.frequency, EasedValue(easingType, lifeProgress));
	lightning.smoothness = std::lerp(start_.smoothness, target_.smoothness, EasedValue(easingType, lifeProgress));
	lightning.seed = std::lerp(start_.seed, target_.seed, EasedValue(easingType, lifeProgress));
	lightning.angle = std::lerp(start_.angle, target_.angle, EasedValue(easingType, lifeProgress));

	// 経過時間の更新
	lightning.time += GameTimer::GetScaledDeltaTime();
}

void ParticleLightningUpdater::ImGui() {

	ImGui::Checkbox("isDrawDebugPoint", &isDrawDebugPoint_);

	ImGui::Separator();

	ImGui::Checkbox("isLookAtEnd", &isLookAtEnd_);
	ImGui::Checkbox("isRefSpawnPos", &isRefSpawnPos_);

	ImGui::DragFloat3("startStart:  Cyan", &start_.start.x, 0.01f);
	ImGui::DragFloat3("targetStart: Cyan", &target_.start.x, 0.01f);

	ImGui::DragFloat3("startEnd:  Red", &start_.end.x, 0.01f);
	ImGui::DragFloat3("targetEnd: Red", &target_.end.x, 0.01f);

	ImGui::DragFloat("startWidth", &start_.width, 0.001f, 0.0f, 10.0f);
	ImGui::DragFloat("targetWidth", &target_.width, 0.001f, 0.0f, 10.0f);

	ImGui::DragFloat("startAmplitude", &start_.amplitudeRatio, 0.01f, 0.0f);
	ImGui::DragFloat("targetAmplitude", &target_.amplitudeRatio, 0.01f, 0.0f);

	ImGui::DragFloat("startFrequency", &start_.frequency, 0.01f, 0.0f);
	ImGui::DragFloat("targetFrequency", &target_.frequency, 0.01f, 0.0f);

	ImGui::DragFloat("startSmoothness", &start_.smoothness, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("targetSmoothness", &target_.smoothness, 0.01f, 0.0f, 1.0f);

	ImGui::DragFloat("startAngle", &start_.angle, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("targetAngle", &target_.angle, 1.0f, -180.0f, 180.0f);

	ImGuiHelper::DragUint32("startNodeCount", start_.nodeCount, 64);
	ImGuiHelper::DragUint32("targetNodeCount", target_.nodeCount, 64);

	ImGui::DragFloat("startSeed", &start_.seed, 1.0f, 0.0f, 6000.0f);
	ImGui::DragFloat("targetSeed", &target_.seed, 1.0f, 0.0f, 6000.0f);

	if (isDrawDebugPoint_) {

		LineRenderer::GetInstance()->DrawSphere(6, 0.8f, start_.start, Color::Cyan());
		LineRenderer::GetInstance()->DrawSphere(6, 0.8f, target_.start, Color::Cyan());

		LineRenderer::GetInstance()->DrawSphere(6, 0.8f, start_.end, Color::Red());
		LineRenderer::GetInstance()->DrawSphere(6, 0.8f, target_.end, Color::Red());
	}
}

void ParticleLightningUpdater::FromJson(const Json& data) {

	if (!data.contains("lightning")) {
		return;
	}

	Init();

	const auto& lightningData = data["lightning"];

	isLookAtEnd_ = lightningData.value("isLookAtEnd", isLookAtEnd_);
	isRefSpawnPos_ = lightningData.value("isLookAtEnd", isRefSpawnPos_);

	start_.start = Vector3::FromJson(lightningData["startStart"]);
	target_.start = Vector3::FromJson(lightningData["targetStart"]);

	start_.end = Vector3::FromJson(lightningData["startEnd"]);
	target_.end = Vector3::FromJson(lightningData["targetEnd"]);

	start_.width = lightningData.value("startWidth", start_.width);
	target_.width = lightningData.value("targetWidth", target_.width);

	start_.amplitudeRatio = lightningData.value("startAmplitude", start_.amplitudeRatio);
	target_.amplitudeRatio = lightningData.value("targetAmplitude", target_.amplitudeRatio);

	start_.frequency = lightningData.value("startFrequency", start_.frequency);
	target_.frequency = lightningData.value("targetFrequency", target_.frequency);

	start_.smoothness = lightningData.value("startSmoothness", start_.smoothness);
	target_.smoothness = lightningData.value("targetSmoothness", target_.smoothness);

	start_.angle = lightningData.value("startAngle", start_.angle);
	target_.angle = lightningData.value("targetAngle", target_.angle);

	start_.nodeCount = lightningData.value("startNodeCount", start_.nodeCount);
	target_.nodeCount = lightningData.value("targetNodeCount", target_.nodeCount);

	start_.seed = lightningData.value("startSeed", start_.seed);
	target_.seed = lightningData.value("targetSeed", target_.seed);
}

void ParticleLightningUpdater::ToJson(Json& data) const {

	Json lightningData;

	lightningData["isLookAtEnd"] = isLookAtEnd_;
	lightningData["isRefSpawnPos_"] = isRefSpawnPos_;

	lightningData["startStart"] = start_.start.ToJson();
	lightningData["targetStart"] = target_.start.ToJson();

	lightningData["startEnd"] = start_.end.ToJson();
	lightningData["targetEnd"] = target_.end.ToJson();

	lightningData["startWidth"] = start_.width;
	lightningData["targetWidth"] = target_.width;

	lightningData["startAmplitude"] = start_.amplitudeRatio;
	lightningData["targetAmplitude"] = target_.amplitudeRatio;

	lightningData["startFrequency"] = start_.frequency;
	lightningData["targetFrequency"] = target_.frequency;

	lightningData["startSmoothness"] = start_.smoothness;
	lightningData["targetSmoothness"] = target_.smoothness;

	lightningData["startAngle"] = start_.angle;
	lightningData["targetAngle"] = target_.angle;

	lightningData["startNodeCount"] = start_.nodeCount;
	lightningData["targetNodeCount"] = target_.nodeCount;

	lightningData["startSeed"] = start_.seed;
	lightningData["targetSeed"] = target_.seed;

	data["lightning"] = lightningData;
}
