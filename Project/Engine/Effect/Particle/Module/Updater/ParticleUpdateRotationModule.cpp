#include "ParticleUpdateRotationModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateRotationModule classMethods
//============================================================================

bool ParticleUpdateRotationModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetRotation: {
		if (const auto& rotation = std::get_if<Vector3>(&command.value)) {

			setRotation_ = *rotation;
		} else if (const auto& matrix = std::get_if<Matrix4x4>(&command.value)){

			setRotationMatrix_ = *matrix;
		}
		return false;
	}
	}
	return false;
}

Vector3 ParticleUpdateRotationModule::UpdateRotation(CPUParticle::ParticleData& particle) const {

	Vector3 rotation{};

	// 更新処理分岐
	switch (updateType_) {
	case ParticleUpdateRotationModule::UpdateType::Lerp:

		// 回転補間
		rotation = Vector3::Lerp(lerpRotation_.start,
			lerpRotation_.target, EasedValue(easing_, particle.progress));
		break;
	case ParticleUpdateRotationModule::UpdateType::ConstantAdd:

		// 回転加算
		rotation = particle.transform.rotationMatrix.GetRotationValue();
		rotation += addRotation_;
		break;
	case ParticleUpdateRotationModule::UpdateType::MoveToDirection:

		// 進行方向に向かせる
		// 進行方向を取得
		Vector3 direction = particle.velocity.Normalize();
		rotation.y = std::atan2(direction.x, direction.z);
		// 横軸方向の長さを求める
		float horizontalLength = std::sqrtf(direction.x * direction.x + direction.z * direction.z);
		// X軸周りの角度(θx)
		rotation.x = std::atan2(-direction.y, horizontalLength);
		break;
	}
	return rotation;
}

Vector3 ParticleUpdateRotationModule::LockAxis(const Vector3& rotation) {

	// そのまま返す
	if (lockAxisType_ == LockAxisType::None) {
		return rotation;
	}

	// 回転軸の固定
	Vector3 lockedRotation = rotation;
	switch (lockAxisType_) {
	case ParticleUpdateRotationModule::LockAxisType::AxisX:

		lockedRotation.x = lockAxis_.x;
		break;
	case ParticleUpdateRotationModule::LockAxisType::AxisY:

		lockedRotation.y = lockAxis_.y;
		break;
	case ParticleUpdateRotationModule::LockAxisType::AxisZ:

		lockedRotation.z = lockAxis_.z;
		break;
	}
	return lockedRotation;
}

void ParticleUpdateRotationModule::UpdateMatrix(
	CPUParticle::ParticleData& particle, const Vector3& rotation) {

	switch (billboardType_) {
	case ParticleBillboardType::None:
	case ParticleBillboardType::YAxis:

		if (setRotationMatrix_.has_value()) {

			particle.transform.rotationMatrix = *setRotationMatrix_;
		} else {

			// 回転行列の更新
			particle.transform.rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
		}
		break;
	case ParticleBillboardType::All:

		particle.transform.rotationMatrix = Matrix4x4::MakeIdentity4x4();
		break;
	}
}

void ParticleUpdateRotationModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	particle.transform.billboardMode = static_cast<uint32_t>(billboardType_);

	if (setRotation_.has_value() && billboardType_ == ParticleBillboardType::None) {

		// 回転軸の固定
		Vector3 rotation = LockAxis(setRotation_.value());

		// 行列の更新
		UpdateMatrix(particle, rotation);
		return;
	} else {

		// 回転の更新
		Vector3 rotation = UpdateRotation(particle);

		// 回転軸の固定
		rotation = LockAxis(rotation);

		// 行列の更新
		UpdateMatrix(particle, rotation);
	}
}

void ParticleUpdateRotationModule::ImGui() {

	EnumAdapter<ParticleBillboardType>::Combo("billboardType", &billboardType_);
	EnumAdapter<UpdateType>::Combo("updateType", &updateType_);
	EnumAdapter<LockAxisType>::Combo("lockAxisType", &lockAxisType_);

	switch (updateType_) {
	case ParticleUpdateRotationModule::UpdateType::Lerp:

		ImGui::DragFloat3("startRotation", &lerpRotation_.start.x, 0.01f);
		ImGui::DragFloat3("targetRotation", &lerpRotation_.target.x, 0.01f);

		Easing::SelectEasingType(easing_, GetName());
		break;
	case ParticleUpdateRotationModule::UpdateType::ConstantAdd:

		ImGui::DragFloat3("addRotation", &addRotation_.x, 0.01f);
		break;
	}

	switch (lockAxisType_) {
	case ParticleUpdateRotationModule::LockAxisType::AxisX:

		ImGui::DragFloat("lockAxisX", &lockAxis_.x, 0.01f);
		break;
	case ParticleUpdateRotationModule::LockAxisType::AxisY:

		ImGui::DragFloat("lockAxisY", &lockAxis_.y, 0.01f);
		break;
	case ParticleUpdateRotationModule::LockAxisType::AxisZ:

		ImGui::DragFloat("lockAxisZ", &lockAxis_.z, 0.01f);
		break;
	}
}

Json ParticleUpdateRotationModule::ToJson() {

	Json data;

	data["easingType"] = EnumAdapter<EasingType>::ToString(easing_);
	data["billboardType"] = EnumAdapter<ParticleBillboardType>::ToString(billboardType_);
	data["updateType"] = EnumAdapter<UpdateType>::ToString(updateType_);
	data["lockAxisType"] = EnumAdapter<LockAxisType>::ToString(lockAxisType_);

	// 回転
	data["lerpRotation"]["start"] = lerpRotation_.start.ToJson();
	data["lerpRotation"]["target"] = lerpRotation_.target.ToJson();
	data["addRotation"] = addRotation_.ToJson();
	data["lockAxis"] = lockAxis_.ToJson();

	return data;
}

void ParticleUpdateRotationModule::FromJson(const Json& data) {

	const auto& easingType = EnumAdapter<EasingType>::FromString(data.value("easingType", ""));
	easing_ = easingType.value();

	const auto& billboardType = EnumAdapter<ParticleBillboardType>::FromString(data.value("billboardType", ""));
	billboardType_ = billboardType.value();

	const auto& updateType = EnumAdapter<UpdateType>::FromString(data.value("updateType", ""));
	updateType_ = updateType.value();

	const auto& lockAxisType = EnumAdapter<LockAxisType>::FromString(data.value("lockAxisType", ""));
	lockAxisType_ = lockAxisType.value();

	// 回転
	const auto& lerpData = data["lerpRotation"];
	lerpRotation_.start = lerpRotation_.start.FromJson(lerpData["start"]);
	lerpRotation_.target = lerpRotation_.target.FromJson(lerpData["target"]);
	addRotation_ = addRotation_.FromJson(data["addRotation"]);
	lockAxis_ = lockAxis_.FromJson(data["lockAxis"]);
}