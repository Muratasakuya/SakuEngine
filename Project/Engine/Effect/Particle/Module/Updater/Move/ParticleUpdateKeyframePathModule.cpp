#include "ParticleUpdateKeyframePathModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	ParticleUpdateKeyframePathModule classMethods
//============================================================================

void ParticleUpdateKeyframePathModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			// 親として動かす座標
			parentTranslation_ = *translation;
		}
		break;
	}
	case ParticleCommandID::SetRotation: {
		if (const auto& rotation = std::get_if<Quaternion>(&command.value)) {

			// キーの位置を回転させる
			parentRotation_ = *rotation;
		}
		break;
	}
	}
}

void ParticleUpdateKeyframePathModule::Init() {

	// 初期化値
	parentTranslation_ = Vector3::AnyInit(0.0f);
	parentRotation_ = Quaternion::IdentityQuaternion();

	isDrawKeyframe_ = true;
	swirlRadius_.start = 0.0f;
	swirlRadius_.target = 0.0f;
	swirlRadiusEasing_ = EasingType::Linear;
	swirlTurns_ = 1.0f;
	swirlPhase_ = 0.0f;

	spawnAngleEnable_ = true;
	spawnAngleStart_ = 0.0f;
	spawnAngleStride_ = (pi * 2.0f) / 8.0f;
	spawnAngleJitter_ = 0.0f;
	spawnAngleWrap_ = true;

	spawnAngleSerial_ = 0;
}

void ParticleUpdateKeyframePathModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// キーフレームがないときは処理しない
	if (keys_.empty()) {
		return;
	}

	// 進捗で渦巻の半径を補間
	const float eased = EasedValue(swirlRadiusEasing_, particle.progress);
	const float currentRadius =
		swirlRadius_.start + (swirlRadius_.target - swirlRadius_.start) * eased;

	// 補間開始時の処理
	if (!particle.hasKeyPathStart) {

		// 発生位置
		const Vector3 spawnTranlation = particle.spawnTranlation;

		const int kSearchDivisions = 128;
		float paramT = 0.0f;
		float paramDistance = FLT_MAX;

		// 補間0.0f時の位置
		Vector3 prevTranlation = LerpKeyframe::GetValue<Vector3>(keys_, 0.0f, type_);
		for (int i = 1; i <= kSearchDivisions; ++i) {

			float t1 = float(i) / static_cast<float>(kSearchDivisions);
			Vector3 currentTranlation = LerpKeyframe::GetValue<Vector3>(keys_, t1, type_);

			// 現在の座標から前の座標への射影
			Vector3 segment = currentTranlation - prevTranlation;
			float segmentLength = Vector3::Dot(segment, segment);
			float projection = Vector3::Dot(spawnTranlation - prevTranlation, segment) / segmentLength;
			projection = std::clamp(projection, 0.0f, 1.0f);
			Vector3 closestPoint = prevTranlation + segment * projection;

			float distance = (spawnTranlation - closestPoint).Length();
			if (distance < paramDistance) {

				// 区間内補間も反映
				paramDistance = distance;
				paramT = (static_cast<float>((i - 1)) + projection) / static_cast<float>(kSearchDivisions);
			}

			// 位置を更新
			prevTranlation = currentTranlation;
		}
		particle.keyPathStartT = paramT;
		particle.hasKeyPathStart = true;
	}

	// 発生した瞬間の位置を角度オフセットで設定する
	// 
	if (currentRadius > 0.0f && spawnAngleEnable_ && !particle.hasKeyPathSpawnAngle) {

		float angle = spawnAngleStart_ + spawnAngleStride_ * static_cast<float>(spawnAngleSerial_++);
		if (0.0f < spawnAngleJitter_) {

			// 乱数でアングルを設定
			angle += RandomGenerator::Generate(-0.5f, 0.5f) * spawnAngleJitter_;
		}
		if (spawnAngleWrap_) {

			angle = std::fmodf(angle, 2.0f * pi);
			if (angle < 0.0f) {
				angle += 2.0f * pi;
			}
		}
		particle.keyPathSpawnAngle = angle;
		particle.hasKeyPathSpawnAngle = true;
	}

	// 補間位置
	float t = particle.keyPathStartT + std::clamp(particle.progress, 0.0f, 1.0f);
	t = std::clamp(t, 0.0f, 1.0f);

	Vector3 onPath = LerpKeyframe::GetValue<Vector3>(keys_, t, type_);
	Vector3 translation = onPath;

	// 渦巻移動のオフセット計算
	// 半径の値が入っているときのみ処理
	if (0.0f < currentRadius) {

		Vector3 tangent = GetTangent(t);
		const float dotY = Vector3::Dot(tangent, Vector3(0.0f, 1.0f, 0.0f));
		Vector3 upRef = (std::fabs(dotY) > 0.99f) ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
		Vector3 right = Vector3::Cross(upRef, tangent).Normalize();
		Vector3 normal = Vector3::Cross(tangent, right).Normalize();

		// パーティクルごとに開始位置の回転を加算する
		float spawnAngle = particle.hasKeyPathSpawnAngle ? particle.keyPathSpawnAngle : 0.0f;
		float theta = spawnAngle + swirlPhase_ + 2.0f * pi * swirlTurns_ * t;

		// 座標を設定
		translation = onPath + right * (std::cos(theta) * currentRadius) +
			normal * (std::sin(theta) * currentRadius);
	}

	// 親の回転をかける
	Vector3 worldTranslation = parentRotation_ * translation;
	// 親の座標を乗算
	worldTranslation += parentTranslation_;
	particle.transform.translation = worldTranslation;
}

Vector3 ParticleUpdateKeyframePathModule::GetTangent(float t) const {

	const float eps = 1e-3f;
	const float t0 = std::clamp(t - eps, 0.0f, 1.0f);
	const float t1 = std::clamp(t + eps, 0.0f, 1.0f);
	Vector3 p0 = LerpKeyframe::GetValue<Vector3>(keys_, t0, type_);
	Vector3 p1 = LerpKeyframe::GetValue<Vector3>(keys_, t1, type_);
	Vector3 v = (p1 - p0);
	float len = v.Length();
	return (len > 1e-6f) ? (v / len) : Vector3(0.0f, 0.0f, 1.0f);
}

void ParticleUpdateKeyframePathModule::ImGui() {

	// 補間タイプ
	EnumAdapter<LerpKeyframe::Type>::Combo("Type", &type_);
	ImGui::Checkbox("isDrawKeyframe", &isDrawKeyframe_);

	{
		// 渦巻き
		ImGui::SeparatorText("Swirl");

		ImGui::DragFloat("startSwirlRadius", &swirlRadius_.start, 0.01f);
		ImGui::DragFloat("targetSwirlRadius", &swirlRadius_.target, 0.01f);
		Easing::SelectEasingType(swirlRadiusEasing_, "swirlRadius");

		ImGui::DragFloat("swirlTurns", &swirlTurns_, 0.01f);
		ImGui::DragFloat("swirlPhase", &swirlPhase_, 0.01f);
	}
	{
		// パーティクル発生位置の角度オフセット
		ImGui::SeparatorText("AngleOffset");

		ImGui::Checkbox("spawnAngleEnable", &spawnAngleEnable_);
		ImGui::DragFloat("spawnAngleStart_", &spawnAngleStart_, 0.01f);
		ImGui::DragFloat("spawnAngleStride", &spawnAngleStride_, 0.01f);
		ImGui::DragFloat("spawnAngleJitter", &spawnAngleJitter_, 0.01f, 0.0f, pi);
		ImGui::Checkbox("spawnAngleWrap", &spawnAngleWrap_);
		if (ImGui::Button("Reset")) {
			spawnAngleSerial_ = 0;
		}
	}

	// キー編集
	if (ImGui::Button("Add Key")) {
		keys_.push_back(Vector3(0.0f, 2.0f, 0.0f));
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		keys_.clear();
	}

	// 並び替え
	const int keyCount = static_cast<int>(keys_.size());
	for (int i = 0; i < keyCount; ++i) {

		ImGui::PushID(i);
		ImGui::Separator();

		ImGui::Text("Key %d", i);
		ImGui::DragFloat3("pos", &keys_[i].x, 0.01f);

		ImGui::PopID();
	}

	// デバッグ表示
	if (isDrawKeyframe_ && keyCount >= 2) {

		const int division = 64;

		Vector3 prevTranslation = LerpKeyframe::GetValue<Vector3>(keys_, 0.0f, type_);
		for (int i = 1; i <= division; ++i) {

			float t = static_cast<float>(i) / static_cast<float>(division);
			Vector3 currentTranslation = LerpKeyframe::GetValue<Vector3>(keys_, t, type_);
			LineRenderer::GetInstance()->DrawLine3D(
				prevTranslation, currentTranslation, Color::Cyan());
			prevTranslation = currentTranslation;
		}

		for (const auto& key : keys_) {

			// キー位置に球を描画
			LineRenderer::GetInstance()->DrawSphere(6, 0.8f, key,
				Color::Cyan());
		}
	}
}

Json ParticleUpdateKeyframePathModule::ToJson() {

	Json data;

	data["type_"] = EnumAdapter<LerpKeyframe::Type>::ToString(type_);
	data["isDrawKeyframe_"] = isDrawKeyframe_;

	data["startSwirlRadius"] = swirlRadius_.start;
	data["targetSwirlRadius"] = swirlRadius_.target;
	data["swirlRadiusEasing_"] = EnumAdapter<EasingType>::ToString(swirlRadiusEasing_);
	data["swirlTurns_"] = swirlTurns_;
	data["swirlPhase_"] = swirlPhase_;

	data["spawnAngleEnable_"] = spawnAngleEnable_;
	data["spawnAngleStart_"] = spawnAngleStart_;
	data["spawnAngleStride_"] = spawnAngleStride_;
	data["spawnAngleJitter_"] = spawnAngleJitter_;
	data["spawnAngleWrap_"] = spawnAngleWrap_;

	// keys
	Json keyJson = Json::array();
	for (const auto& key : keys_) {

		keyJson.push_back(key.ToJson());
	}
	data["keys"] = keyJson;
	return data;
}

void ParticleUpdateKeyframePathModule::FromJson(const Json& data) {

	type_ = EnumAdapter<LerpKeyframe::Type>::FromString(data.value("type_", "Linear")).value();

	Init();

	isDrawKeyframe_ = data.value("isDrawKeyframe_", isDrawKeyframe_);

	swirlRadius_.start = data.value("startSwirlRadius", swirlRadius_.start);
	swirlRadius_.target = data.value("targetSwirlRadius", swirlRadius_.target);
	swirlRadiusEasing_ = EnumAdapter<EasingType>::FromString(data.value("swirlRadiusEasing_", "Linear")).value();
	swirlTurns_ = data.value("swirlTurns_", swirlTurns_);
	swirlPhase_ = data.value("swirlPhase_", swirlPhase_);

	spawnAngleEnable_ = data.value("spawnAngleEnable_", spawnAngleEnable_);
	spawnAngleStart_ = data.value("spawnAngleStart_", spawnAngleStart_);
	spawnAngleStride_ = data.value("spawnAngleStride_", spawnAngleStride_);
	spawnAngleJitter_ = data.value("spawnAngleJitter_", spawnAngleJitter_);
	spawnAngleWrap_ = data.value("spawnAngleWrap_", spawnAngleWrap_);

	keys_.clear();
	if (data.contains("keys") && data["keys"].is_array()) {
		for (const auto& keyJson : data["keys"]) {

			keys_.push_back(Vector3::FromJson(keyJson));
		}
	}
}