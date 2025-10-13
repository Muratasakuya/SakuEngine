#include "ParticleUpdateRotationModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateRotationModule classMethods
//============================================================================

static Quaternion MakeTwist(const Quaternion& rotation, const Vector3& axisN) {

	// rotationのベクトル部を軸へ射影して、軸回りのツイストを取り出す
	Vector3 a = axisN.Normalize();
	Vector3 v = { rotation.x, rotation.y, rotation.z };
	Vector3 projection = a * Vector3::Dot(v, a);
	Quaternion twist{ projection.x, projection.y, projection.z, rotation.w };
	return Quaternion::Normalize(twist);
}

bool ParticleUpdateRotationModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetRotation: {
		if (const auto& rotation = std::get_if<Quaternion>(&command.value)) {

			setRotation_ = *rotation;
		}
		return false;
	}
	}
	return false;
}

void ParticleUpdateRotationModule::ToAxisAngle(const Quaternion& rotation, Vector3& axis, float& angle) {

	Quaternion normalizedRotate = Quaternion::Normalize(rotation);
	angle = 2.0f * std::acos(std::clamp(normalizedRotate.w, -1.0f, 1.0f)); // 0, 2π
	const float sin = std::sqrt((std::max)(
		0.0f, 1.0f - normalizedRotate.w * normalizedRotate.w)); // sin(angle/2)
	// 角度ほぼ0.0f
	if (sin < 1e-6f) {

		axis = Vector3(1.0f, 0.0f, 0.0f);
	} else {

		axis = Vector3(normalizedRotate.x / sin,
			normalizedRotate.y / sin, normalizedRotate.z / sin);
	}
}

Quaternion ParticleUpdateRotationModule::UpdateRotation(
	CPUParticle::ParticleData& particle, float deltaTime) const {

	switch (updateType_) {
	case UpdateType::Slerp: {

		const Quaternion start = Quaternion::Normalize(lerpRotation_.start);
		const Quaternion target = Quaternion::Normalize(lerpRotation_.target);

		// 回転差分
		const Quaternion delta = Quaternion::Multiply(Quaternion::Inverse(start), target);

		// 軸角へ
		Vector3 axis; float angle = 0.0f;
		ToAxisAngle(delta, axis, angle);

		// 長い方の弧を処理するときは軸を反転する
		if (slerpPreferLongArc_ && angle > 1e-6f && angle < pi) {

			angle = 2.0f * pi - angle;
			axis = -axis;
		}
		// 追加回転数を角度にかける
		angle += 2.0f * pi * static_cast<float>(slerpExtraTurns_);

		const float t = EasedValue(easing_, particle.progress);
		Quaternion step = Quaternion::MakeAxisAngle(axis, angle * t);
		return Quaternion::Normalize(Quaternion::Multiply(start, step));
	}
	case UpdateType::AngularVelocity: {

		// 前フレームの姿勢に回転を掛ける
		Quaternion rotation = particle.rotation;
		Vector3 axis = angleAxis_.Normalize();
		// 軸回転させた後の回転を計算
		Quaternion angleRotation = Quaternion::Normalize(
			Quaternion::MakeAxisAngle(axis, angleSpeedRadian_ * deltaTime));
		return Quaternion::Normalize(rotation * angleRotation);
	}
	case UpdateType::LookToVelocity: {

		Vector3 direction = particle.velocity.Normalize();
		// 止まっている間はそのまま
		if (direction.Length() < std::numeric_limits<float>::epsilon()) {

			return particle.rotation;
		}
		return Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
	}
	}
	return particle.rotation;
}

Quaternion ParticleUpdateRotationModule::LockAxis(const Quaternion& rotation) const {

	// そのまま返す
	if (lockAxisType_ == LockAxisType::None) {

		return rotation;
	}

	// 軸の方向に応じて回転軸を取得
	Vector3 axis = (lockAxisType_ == LockAxisType::AxisX) ? Vector3(1.0f, 0.0f, 0.0f) :
		(lockAxisType_ == LockAxisType::AxisY) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(0.0f, 0.0f, 1.0f);
	// 元の姿勢をSwing * Twistに分解してTwistの角度を置き換える
	Quaternion twist = MakeTwist(rotation, axis);
	Quaternion swing = Quaternion::Multiply(rotation, Quaternion::Inverse(twist));
	Quaternion fixedTwist = Quaternion::MakeAxisAngle(axis, lockAxisAngle_);
	return Quaternion::Normalize(Quaternion::Multiply(swing, fixedTwist));
}

void ParticleUpdateRotationModule::UpdateMatrix(
	CPUParticle::ParticleData& particle, const Quaternion& rotation) {

	particle.rotation = Quaternion::Normalize(rotation);
	// 全軸ビルボード処理
	if (billboardType_ == ParticleBillboardType::All) {

		particle.transform.rotationMatrix = Matrix4x4::MakeIdentity4x4();
	}
	// それ以外は回転を計算
	else {

		particle.transform.rotationMatrix = Quaternion::MakeRotateMatrix(particle.rotation);
	}
}

void ParticleUpdateRotationModule::Execute(
	CPUParticle::ParticleData& particle, float deltaTime) {

	particle.transform.billboardMode = static_cast<uint32_t>(billboardType_);

	// 外部で姿勢を直接セット
	if (setRotation_.has_value() && billboardType_ == ParticleBillboardType::None) {

		// 行列を計算
		UpdateMatrix(particle, LockAxis(*setRotation_));
		return;
	}

	// 回転を更新
	Quaternion rotation = UpdateRotation(particle, deltaTime);
	// 軸固定処理
	rotation = LockAxis(rotation);
	// 行列を計算
	UpdateMatrix(particle, rotation);
}

void ParticleUpdateRotationModule::ImGui() {

	EnumAdapter<ParticleBillboardType>::Combo("billboardType", &billboardType_);
	EnumAdapter<UpdateType>::Combo("updateType", &updateType_);
	EnumAdapter<LockAxisType>::Combo("lockAxisType", &lockAxisType_);

	switch (updateType_) {
	case UpdateType::Slerp:

		if (ImGui::DragFloat4("startRotation", &lerpRotation_.start.x, 0.01f)) {

			lerpRotation_.start = Quaternion::Normalize(lerpRotation_.start);
		}
		if (ImGui::DragFloat4("targetRotation", &lerpRotation_.target.x, 0.01f)) {

			lerpRotation_.target = Quaternion::Normalize(lerpRotation_.target);
		}
		ImGui::DragInt("extraTurns", &slerpExtraTurns_, 1, 0, 20);
		ImGui::Checkbox("preferLongArc", &slerpPreferLongArc_);
		Easing::SelectEasingType(easing_, GetName());
		break;
	case UpdateType::AngularVelocity:

		ImGui::DragFloat3("angleAxis", &angleAxis_.x, 0.01f);
		ImGui::DragFloat("angleSpeedRadian", &angleSpeedRadian_, 0.01f);
		break;
	case UpdateType::LookToVelocity:

		ImGui::TextUnformatted("Faces velocity");
		break;
	}
	if (lockAxisType_ != LockAxisType::None) {

		ImGui::DragFloat("lockAxisAngle", &lockAxisAngle_, 0.01f);
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
	data["angleAxis_"] = angleAxis_.ToJson();
	data["angleSpeedRadian_"] = angleSpeedRadian_;
	data["slerpExtraTurns_"] = slerpExtraTurns_;
	data["slerpPreferLongArc_"] = slerpPreferLongArc_;

	return data;
}

void ParticleUpdateRotationModule::FromJson(const Json& data) {

	const auto& easingType = EnumAdapter<EasingType>::FromString(data.value("easingType", "EaseInSine"));
	easing_ = easingType.value();

	const auto& billboardType = EnumAdapter<ParticleBillboardType>::FromString(data.value("billboardType", "All"));
	billboardType_ = billboardType.value();

	const auto& updateType = EnumAdapter<UpdateType>::FromString(data.value("updateType", "Slerp"));
	updateType_ = updateType.value();

	const auto& lockAxisType = EnumAdapter<LockAxisType>::FromString(data.value("lockAxisType", "None"));
	lockAxisType_ = lockAxisType.value();

	// 回転
	const auto& lerpData = data["lerpRotation"];
	lerpRotation_.start = Quaternion::FromJson(lerpData["start"]);
	lerpRotation_.target = Quaternion::FromJson(lerpData["target"]);
	angleAxis_ = Vector3::FromJson(data["angleAxis_"]);
	angleSpeedRadian_ = data.value("angleSpeedRadian_", 0.0f);
	slerpExtraTurns_ = data.value("slerpExtraTurns_", 0);
	slerpPreferLongArc_ = data.value("slerpPreferLongArc_", false);
}