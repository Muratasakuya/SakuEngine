#include "ParticleUpdateGravityModule.h"

//============================================================================
//	ParticleUpdateGravityModule classMethods
//============================================================================

void ParticleUpdateGravityModule::Init() {

	// 初期値の設定
	reflectGround_ = true;

	reflectGroundY_ = 0.0f;
	restitution_ = 0.4f;

	// 真下方向
	strength_ = 9.8f;
	direction_ = Vector3(0.0f, -1.0f, 0.0f);
}

void ParticleUpdateGravityModule::Execute(
	CPUParticle::ParticleData& particle, float deltaTime) {

	// 地面に反射する場合の処理
	if (reflectGround_) {

		// 速度を加算後の座標
		Vector3 newPos = particle.transform.translation + particle.velocity * deltaTime;
		// 地面との衝突判定
		if (newPos.y <= reflectGroundY_ && particle.velocity.y < 0.0f) {

			// 反射処理
			particle.velocity = Vector3::Reflect(particle.velocity, Vector3(0.0f, 1.0f, 0.0f)) * restitution_;
			// 反射位置よりも進めないように調整
			newPos.y = reflectGroundY_;
		}

		// 更新された位置に適用
		particle.transform.translation = newPos;
	}

	// 重力の適応
	Vector3 gravity = direction_ * strength_ * deltaTime;
	particle.velocity += gravity;
}

void ParticleUpdateGravityModule::ImGui() {

	ImGui::Checkbox("reflectGround", &reflectGround_);

	ImGui::DragFloat("reflectGroundY", &reflectGroundY_, 0.01f);
	ImGui::DragFloat("restitution", &restitution_, 0.01f);

	ImGui::DragFloat("strength", &strength_, 0.01f);
	if (ImGui::DragFloat3("direction", &direction_.x, 0.01f)) {

		direction_ = direction_.Normalize();
	}
}

Json ParticleUpdateGravityModule::ToJson() {

	Json data;

	data["reflectGround"] = reflectGround_;
	data["reflectGroundY"] = reflectGroundY_;
	data["restitution"] = restitution_;

	data["strength"] = strength_;
	data["direction"] = direction_.ToJson();

	return data;
}

void ParticleUpdateGravityModule::FromJson(const Json& data) {

	reflectGround_ = data.value("reflectGround", true);
	reflectGroundY_ = data.value("reflectGroundY", 0.0f);
	restitution_ = data.value("restitution", 0.4f);

	strength_ = data.value("strength", 9.8f);
	direction_ = direction_.FromJson(data["direction"]).Normalize();
}