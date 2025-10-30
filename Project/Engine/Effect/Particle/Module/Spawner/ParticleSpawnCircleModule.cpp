#include "ParticleSpawnCircleModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/MathLib/MathUtils.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleSpawnCircleModule classMethods
//============================================================================

void ParticleSpawnCircleModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			translation_ = *translation;
		}
		break;
	}
	case ParticleCommandID::SetRotation: {
		if (const auto& ERotation = std::get_if<Vector3>(&command.value)) {

			rotation_ = Quaternion::EulerToQuaternion(*ERotation);
		} else if (const auto& QRotation = std::get_if<Quaternion>(&command.value)) {

			rotation_ = *QRotation;
		}
		break;
	}
	}
}

void ParticleSpawnCircleModule::Init() {

	// 値の初期値
	ICPUParticleSpawnModule::InitCommonData();

	radius_ = 2.0f;
	translation_ = Vector3(0.0f, 6.0f, 0.0f);
	rotation_ = Quaternion::IdentityQuaternion();

	// デフォでランダム
	mode_ = SpawnMode::Random;

	angleMin_ = 0.0f;
	angleMax_ = 360.0f;
	clockwise_ = false;

	stepAngle_ = 15.0f;
	currentAngle_ = 0.0f;
	circleDivision_ = 64;
}

void ParticleSpawnCircleModule::Execute(std::list<CPUParticle::ParticleData>& particles) {

	// 発生数
	uint32_t emitCount = emitCount_.GetValue();
	if (emitCount == 0) {
		return;
	}

	const bool fullCircleAngle = (360.0f - std::numeric_limits<float>::epsilon()) <= std::fabs(angleMax_ - angleMin_);
	float amin = Math::WrapDegree(angleMin_);
	float amax = Math::WrapDegree(angleMax_);
	float span = fullCircleAngle ? 360.0f : Math::WrapDegree(amax - amin);

	// 発生インデックスから角度を取得
	auto GetAngle = [&](uint32_t i)->float {
		switch (mode_) {
		case SpawnMode::Random: {
			//============================================================================
			//	円周からランダム発生
			//============================================================================

			// 範囲内でランダムに角度を決定
			float degree = (span <= 0.0f) ? 0.0f : RandomGenerator::Generate(0.0f, span);
			// 反転させた方向に進ませるかどうかで最終的な角度を決定する
			float base = clockwise_ ? amax : amin;
			return Math::WrapDegree(base + (clockwise_ ? -degree : degree));
		}
		case SpawnMode::EvenPerFrame: {
			//============================================================================
			//	発生個数で等間隔に発生
			//============================================================================

			// 発生個数で等間隔
			float delta = (span <= 0.0f) ? 0.0f : (span / static_cast<float>(emitCount));
			// 反転させた方向に進ませるかどうかで最終的な角度を決定する
			float base = clockwise_ ? amax : amin;
			return Math::WrapDegree(base + (i + 0.5f) * delta * (clockwise_ ? -1.0f : 1.0f));
		}
		case SpawnMode::Progressive: {
			//============================================================================
			//	発生させるごとに角度を進めて発生
			//============================================================================

			// アークの基準
			const float base = clockwise_ ? amax : amin;
			const float angle = Math::WrapDegree(currentAngle_);
			const float stepMag = std::fabs(stepAngle_);

			// minとmaxが同じならその角度で返す
			if (span <= 0.0f) {
				return base;
			}

			// 進行方向に沿った基準からの距離を取得
			auto distAlong = [&](float from, float to)->float {
				return clockwise_ ? Math::WrapDegree(from - to) :
					Math::WrapDegree(to - from); };
			// 現在角をアークに射影
			float pos = distAlong(base, angle);
			// ステップ分進めてアーク内に収める
			float adv = std::fmod(pos + stepMag * static_cast<float>(i), span);
			float result = Math::WrapDegree(base + (clockwise_ ? -adv : adv));
			return result;
		}
		}
		return amin;
		};

	// 親の座標
	Vector3 parentTranslation{};
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}
	for (uint32_t index = 0; index < emitCount; ++index) {

		CPUParticle::ParticleData particle{};

		// 共通設定
		ICPUParticleSpawnModule::SetCommonData(particle);

		// 角度取得
		float rad = GetAngle(index) * radian;

		// ローカルの半径方向
		Vector3 localDirection = Vector3(std::cos(rad), 0.0f, std::sin(rad));
		Vector3 localPos = localDirection * radius_;

		// ローカルの向きに角度回転と座標移動を反映
		Vector3 worldDirection = rotation_ * localDirection;
		Vector3 worldPos = rotation_ * localPos + translation_ + parentTranslation;

		// 速度、発生位置
		particle.velocity = GetVelocity(index, emitCount, worldDirection);
		particle.transform.translation = worldPos;

		// 発生した瞬間の座標を記録
		particle.spawnTranlation = worldPos;

		// 追加
		particles.push_back(particle);
	}

	// Progressiveのときに発生間隔を進める
	UpdateAdvanceProgressive(emitCount);
}

void ParticleSpawnCircleModule::UpdateAdvanceProgressive(uint32_t emitCount) {

	// Progressiveの時のみ
	if (mode_ == SpawnMode::Progressive) {

		// 全周かどうか
		const bool full = 360.0f - std::numeric_limits<float>::epsilon() <= std::fabs(angleMax_ - angleMin_);
		// 弧の長さ
		const float span = full ? 360.0f : Math::WrapDegree(angleMax_ - angleMin_);
		// 進行の基準端点
		const float base = clockwise_ ? angleMax_ : angleMin_;
		// 1発生あたりの前進量
		const float stepMag = std::fabs(stepAngle_) * static_cast<float>(emitCount);

		// 指定方向に沿ったfromからtoへの距離
		auto DistanceAlong = [&](float from, float to)->float {
			return clockwise_ ? Math::WrapDegree(from - to) : Math::WrapDegree(to - from); };

		// 現在角を弧の座標系に射影
		float pos = DistanceAlong(base, Math::WrapDegree(currentAngle_));
		// emitCount回分進めて弧長で折り返す
		pos = std::fmod(pos + stepMag, span);
		currentAngle_ = Math::WrapDegree(base + (clockwise_ ? -pos : +pos));
	}
}

Vector3 ParticleSpawnCircleModule::GetVelocity(
	uint32_t index, uint32_t emitCount, const Vector3& direction) const {

	// 速度を取得
	const float speed = moveSpeed_.GetValue();
	// Normalはそのまま法線方向を返す
	if (velocityMode_ == VelocityMode::Normal) {

		return direction.Normalize() * speed;
	}

	// 角度レンジの準備（ゼロ度跨ぎ対応）
	const bool fullCircleAngle = (360.0f - std::numeric_limits<float>::epsilon()) <= std::fabs(angleMax_ - angleMin_);
	const float amin = Math::WrapDegree(angleMin_);
	const float amax = Math::WrapDegree(angleMax_);
	const float span = fullCircleAngle ? 360.0f : Math::WrapDegree(amax - amin);

	// 発生インデックスから角度を取得
	auto GetAngle = [&](uint32_t i)->float {
		switch (mode_) {
		case SpawnMode::Random: {
			//============================================================================
			//	円周からランダム発生
			//============================================================================

			// 範囲内でランダムに角度を決定
			float degree = (span <= 0.0f) ? 0.0f : RandomGenerator::Generate(0.0f, span);
			// 反転させた方向に進ませるかどうかで最終的な角度を決定する
			float base = clockwise_ ? amax : amin;
			return Math::WrapDegree(base + (clockwise_ ? -degree : degree));
		}
		case SpawnMode::EvenPerFrame: {
			//============================================================================
			//	発生個数で等間隔に発生
			//============================================================================

			// 発生個数で等間隔
			float delta = (span <= 0.0f) ? 0.0f : (span / static_cast<float>(emitCount));
			// 反転させた方向に進ませるかどうかで最終的な角度を決定する
			float base = clockwise_ ? amax : amin;
			return Math::WrapDegree(base + (i + 0.5f) * delta * (clockwise_ ? -1.0f : 1.0f));
		}
		case SpawnMode::Progressive: {
			//============================================================================
			//	発生させるごとに角度を進めて発生
			//============================================================================

			// アークの基準
			const float base = clockwise_ ? amax : amin;
			const float angle = Math::WrapDegree(currentAngle_);
			const float stepMag = std::fabs(stepAngle_);

			// minとmaxが同じならその角度で返す
			if (span <= 0.0f) {
				return base;
			}

			// 進行方向に沿った基準からの距離を取得
			auto distAlong = [&](float from, float to)->float {
				return clockwise_ ? Math::WrapDegree(from - to) :
					Math::WrapDegree(to - from); };
			// 現在角をアークに射影
			float pos = distAlong(base, angle);
			// ステップ分進めてアーク内に収める
			float adv = std::fmod(pos + stepMag * static_cast<float>(i), span);
			float result = Math::WrapDegree(base + (clockwise_ ? -adv : adv));
			return result;
		}
		}
		return amin;
		};

	// 親の座標
	Vector3 parentTranslation{};
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}

	// 角度取得
	float rad = GetAngle(index) * radian;

	// ローカルの半径方向
	Vector3 localDirection = Vector3(std::cos(rad), 0.0f, std::sin(rad));
	Vector3 localPos = localDirection * radius_;
	Vector3 worldPos = rotation_ * localPos + translation_ + parentTranslation;

	// モード別に隣接している点のインデックスを取得
	auto NeighborIndex = [&](bool next)->uint32_t {
		if (mode_ == SpawnMode::EvenPerFrame) {
			if (fullCircleAngle) {

				// 360度の円なら発生位置をループさせる
				return next ? ((index + 1) % emitCount) : ((index + emitCount - 1) % emitCount);
			} else {
				// 次の発生位置を取得するか
				if (next) {

					return (index + 1 < emitCount) ? (index + 1) : index;
				}
				// 前の発生位置を取得するか
				else {

					return (0 < index) ? (index - 1) : index;
				}
			}
		}
		//  発生させるごとに発生角度を進めるモード
		else if (mode_ == SpawnMode::Progressive) {

			// 次の発生位置か前の発生位置のindexを+-して返す
			return next ? (index + 1) : (index + emitCount - 1);
		} else {

			// Randomは次も何もないのでそのまま返す
			return index;
		}
		};

	// 次の点の位置に向かって動かすか
	// true:  次の点に向かう
	// false: 前の点に向かう
	bool wantNext = (velocityMode_ == VelocityMode::NextPoint);
	// 隣接点のインデックスを取得
	uint32_t j = NeighborIndex(wantNext);

	// 隣接点の角度、位置を取得する
	float nextRad = GetAngle(j) * radian;
	Vector3 nextLocalDir(std::cos(nextRad), 0.0f, std::sin(nextRad));
	Vector3 nextLocalPos = nextLocalDir * radius_;
	Vector3 nextWorldPos = rotation_ * nextLocalPos + translation_ + parentTranslation;

	// 次の点と発生予定の座標の差分ベクトルを取得
	Vector3 resultDirection = nextWorldPos - worldPos;
	// ほぼ同じ位置の場合は接線方向を取得する
	if (resultDirection.Length() <= std::numeric_limits<float>::epsilon()) {

		float sign = 1.0f;
		if (clockwise_) {
			sign *= -1.0f;
		}
		if (!wantNext) {
			sign *= -1.0f;
		}
		Vector3 tanLocal(-std::sin(rad) * sign, 0.0f, std::cos(rad) * sign);
		Vector3 tanWorld = rotation_ * tanLocal;
		resultDirection = tanWorld;
	}
	return resultDirection.Normalize() * speed;
}

void ParticleSpawnCircleModule::DrawEmitter() {

	Vector3 parentTranslation{};
	// 親の座標
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}

	// 円を描画
	LineRenderer* renderer = LineRenderer::GetInstance();
	Vector3 center = parentTranslation + translation_;
	float step = 2.0f * pi / static_cast<float>((std::max)(3, circleDivision_));
	for (int i = 0; i < circleDivision_; ++i) {

		float a0 = step * static_cast<float>(i);
		float a1 = step * static_cast<float>((i + 1) % circleDivision_);
		Vector3 p0 = rotation_ * Vector3(std::cos(a0) * radius_, 0.0f, std::sin(a0) * radius_);
		Vector3 p1 = rotation_ * Vector3(std::cos(a1) * radius_, 0.0f, std::sin(a1) * radius_);
		renderer->DrawLine3D(center + p0, center + p1, emitterLineColor_);
	}

	// 角度レンジの線を描画
	auto DrawRayAtDegree = [&](float degree, float length, const Color& color) {

		float rad = degree * radian;
		Vector3 direction = rotation_ * Vector3(std::cos(rad), 0.0f, std::sin(rad));
		renderer->DrawLine3D(center, center + direction * length, color); };
	// 最小角度
	DrawRayAtDegree(angleMin_, radius_ * 1.1f, Color::Cyan());
	// 最大角度
	DrawRayAtDegree(angleMax_, radius_ * 1.1f, Color::Cyan());
}

void ParticleSpawnCircleModule::ImGui() {

	ImGui::DragFloat("radius", &radius_, 0.01f, 0.0f);
	ImGui::DragFloat3("translation", &translation_.x, 0.01f);
	if (ImGui::DragFloat4("rotation", &rotation_.x, 0.01f)) {

		rotation_ = Quaternion::Normalize(rotation_);
	}

	ImGui::SeparatorText("Angle");

	EnumAdapter<SpawnMode>::Combo("SpawnMode", &mode_);
	EnumAdapter<VelocityMode>::Combo("VelocityMode", &velocityMode_);

	ImGui::DragFloat("angleMinDeg", &angleMin_, 0.1f, 0.0f, 360.0f);
	ImGui::DragFloat("angleMaxDeg", &angleMax_, 0.1f, 0.0f, 360.0f);
	ImGui::Checkbox("clockwise", &clockwise_);

	if (mode_ == SpawnMode::Progressive) {

		ImGui::Text("currentAngle: %.3f", currentAngle_);
		ImGui::DragFloat("stepAngle", &stepAngle_, 0.1f, 0.0f, 360.0f);
	}
}

Json ParticleSpawnCircleModule::ToJson() {

	Json data;

	// 共通設定
	ICPUParticleSpawnModule::ToCommonJson(data);

	data["radius"] = radius_;
	data["translation"] = translation_.ToJson();
	data["rotation"] = rotation_.ToJson();

	data["mode"] = EnumAdapter<SpawnMode>::ToString(mode_);
	data["velocityMode_"] = EnumAdapter<VelocityMode>::ToString(velocityMode_);
	data["angleMin_"] = angleMin_;
	data["angleMax_"] = angleMax_;
	data["clockwise_"] = clockwise_;
	data["stepAngle_"] = stepAngle_;

	return data;
}

void ParticleSpawnCircleModule::FromJson(const Json& data) {

	// 共通設定
	ICPUParticleSpawnModule::FromCommonJson(data);

	radius_ = data.value("radius", 2.0f);
	translation_ = Vector3::FromJson(data.value("translation", Json()));
	rotation_ = Quaternion::FromJson(data.value("rotation", Json()));

	mode_ = EnumAdapter<SpawnMode>::FromString(data.value("mode", "Random")).value();
	velocityMode_ = EnumAdapter<VelocityMode>::FromString(data.value("velocityMode_", "Normal")).value();
	angleMin_ = data.value("angleMin_", 0.0f);
	angleMax_ = data.value("angleMax_", 360.0f);
	clockwise_ = data.value("clockwise_", false);
	stepAngle_ = data.value("stepAngle_", 15.0f);
}