#include "ParticleSpawnPolygonVertexModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>

//============================================================================
//	ParticleSpawnPolygonVertexModule classMethods
//============================================================================

bool ParticleSpawnPolygonVertexModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			translation_ = *translation;
			return true;
		}
		return false;
	}
	case ParticleCommandID::SetRotation: {
		if (const auto& rotation = std::get_if<Vector3>(&command.value)) {

			emitterRotation_ = *rotation;
			return true;
		}
		return false;
	}
	case ParticleCommandID::SetBillboardRotation: {
		if (const auto* matrix = std::get_if<Matrix4x4>(&command.value)) {

			// ビルボード回転を取得
			billboardRotation_ = *matrix;
			useBillboardRotation_ = true;
			return true;
		}
		return false;
	}
	case ParticleCommandID::SetEmitFlag: {
		if (const auto& flag = std::get_if<bool>(&command.value)) {

			bool emit = flag;
			// trueならリセットして発生させる
			if (emit) {
				
				spawnTimer_.Reset();
				updater_.Reset();
				spawned_ = 0;
				instances_.clear();
				multiEmit_ = true;
				updateEnable_ = true;
			} else {

				multiEmit_ = false;
				updateEnable_ = false;
			}
			return true;
		}
		return false;
	}
	}
	return false;
}

void ParticleSpawnPolygonVertexModule::Init() {

	// 初期化値
	ICPUParticleSpawnModule::InitCommonData();

	isInterpolate_ = false;
	notMoveEmit_ = false;

	// xを90度回転
	emitterRotation_.x = pi / 2.0f;

	vertexCount_ = 3;
	scale_ = 1.0f;

	emitPerVertex_ = ParticleValue<uint32_t>::SetValue(4);
	interpolateSpacing_ = ParticleValue<float>::SetValue(0.08f);
	prevVertices_ = CalcVertices();
}

std::vector<Vector3> ParticleSpawnPolygonVertexModule::CalcVertices() const {

	return CalcVertices(scale_, emitterRotation_);
}

std::vector<Vector3> ParticleSpawnPolygonVertexModule::CalcVertices(
	float scale, const Vector3& rotation) const {

	std::vector<Vector3> vertices; vertices.reserve(vertexCount_);

	// ビルボード回転か既存の回転を使用するか分岐
	Matrix4x4 rotateMatrix = Matrix4x4::MakeIdentity4x4();
	if (useBillboardRotation_) {

		Matrix4x4 rollZ = Matrix4x4::MakeRotateMatrix(Vector3(0.0f, 0.0f, rotation.y));
		rotateMatrix = Matrix4x4::Multiply(billboardRotation_, rollZ);
	} else {

		rotateMatrix = Matrix4x4::MakeRotateMatrix(rotation);
	}
	for (int i = 0; i < vertexCount_; ++i) {

		float ang = 2.0f * pi * i / vertexCount_;
		Vector3 local(std::cos(ang) * scale, 0.0f, std::sin(ang) * scale);

		// 回転適応後の頂点座標
		vertices.push_back(rotateMatrix.TransformPoint(local) + translation_);
	}
	return vertices;
}

void ParticleSpawnPolygonVertexModule::SpawnInstance() {

	if (maxConcurrent_ && maxConcurrent_ <= instances_.size()) {
		return;
	}

	// 新しいインスタンスの設定
	PolygonInstance instance{};
	instance.updater = updater_;
	// オフセット回転をかける
	if (offsetRotation_.Length() != 0.0f) {

		Vector3 step = offsetRotation_ * static_cast<float>(spawned_);
		instance.updater.SetOffsetRotation(step);
	}
	instance.updater.Reset();

	// 始姿勢で固めて prev を作る
	instance.scale = instance.updater.GetStartScale();
	instance.rotation = instance.updater.GetStartRotation();
	instance.prevVertices = CalcVertices(instance.scale, instance.rotation);
	instances_.push_back(std::move(instance));
}

void ParticleSpawnPolygonVertexModule::EmitForInstance(PolygonInstance& instance,
	std::list<CPUParticle::ParticleData>& particles) {

	const auto current = CalcVertices(instance.scale, instance.rotation);

	// 動いていないときは発生させない
	if (notMoveEmit_) {
		bool moved = false;
		for (size_t i = 0, n = (std::min)(current.size(), instance.prevVertices.size()); i < n; ++i) {
			if (std::numeric_limits<float>::epsilon() < (current[i] - instance.prevVertices[i]).Length()) {

				moved = true;
				break;
			}
		}
		if (!moved) {
			return;
		}
	}

	if (isInterpolate_) {

		const float spacing = interpolateSpacing_.GetValue();
		const uint32_t emitPerVertex = emitPerVertex_.GetValue();
		if (instance.prevVertices.size() != current.size()) {

			instance.prevVertices = current;
		}

		for (uint32_t v = 0; v < current.size(); ++v) {

			const Vector3 diff = current[v] - instance.prevVertices[v];
			const float   len = diff.Length();
			if (len < spacing || len < std::numeric_limits<float>::epsilon()) {

				Vector3 velocity = Vector3::Normalize(diff) * moveSpeed_.GetValue();
				for (uint32_t n = 0; n < emitPerVertex; ++n) {

					CPUParticle::ParticleData particle{};
					ICPUParticleSpawnModule::SetCommonData(particle);
					particle.velocity = velocity;
					particle.transform.translation = current[v];
					// 発生した瞬間の座標を記録
					particle.spawnTranlation = particle.transform.translation;
					particles.push_back(particle);
				}
				continue;
			}

			const uint32_t interpCount = static_cast<uint32_t>(len / spacing);
			const Vector3 direction = diff / len;
			const float step = spacing;
			const Vector3 velocity = direction * moveSpeed_.GetValue();

			for (uint32_t i = 1; i <= interpCount; ++i) {

				const Vector3 pos = instance.prevVertices[v] + direction * step * static_cast<float>(i);
				for (uint32_t n = 0; n < emitPerVertex; ++n) {

					CPUParticle::ParticleData particle{};
					ICPUParticleSpawnModule::SetCommonData(particle);
					particle.velocity = velocity;
					particle.transform.translation = pos;
					// 発生した瞬間の座標を記録
					particle.spawnTranlation = particle.transform.translation;
					particles.push_back(particle);
				}
			}
			for (uint32_t n = 0; n < emitPerVertex; ++n) {

				CPUParticle::ParticleData particle{};
				ICPUParticleSpawnModule::SetCommonData(particle);
				particle.velocity = velocity;
				particle.transform.translation = current[v];
				// 発生した瞬間の座標を記録
				particle.spawnTranlation = particle.transform.translation;
				particles.push_back(particle);
			}
		}
	} else {

		const uint32_t emitPerVertex = emitPerVertex_.GetValue();
		if (instance.prevVertices.size() != current.size()) {
			instance.prevVertices = current;
		}

		for (uint32_t i = 0; i < current.size(); ++i) {

			Vector3 direction = Vector3::Normalize(current[i] - instance.prevVertices[i]);
			bool moving = direction.Length() > std::numeric_limits<float>::epsilon();
			Vector3 vellocity = moving ? direction * moveSpeed_.GetValue() : Vector3(0.0f, 0.0f, moveSpeed_.GetValue());

			for (uint32_t n = 0; n < emitPerVertex; ++n) {

				CPUParticle::ParticleData particle{};
				ICPUParticleSpawnModule::SetCommonData(particle);
				particle.velocity = vellocity;
				particle.transform.translation = current[i];
				// 発生した瞬間の座標を記録
				particle.spawnTranlation = particle.transform.translation;
				particles.push_back(particle);
			}
		}
	}

	// 次フレーム用の値を保持
	instance.prevVertices = current;
}

void ParticleSpawnPolygonVertexModule::UpdateEmitter() {

	// 前フレーム頂点の保存
	prevVertices_ = CalcVertices();
}

void ParticleSpawnPolygonVertexModule::Execute(std::list<CPUParticle::ParticleData>& particles) {

	if (!updateEnable_) {
		return;
	}

	if (useMulti_) {
		if (!multiEmit_) {
			return;
		}

		// 複数発生時間更新
		spawnTimer_.Update();
		// 初回即時起動したい場合
		if (spawned_ == 0 && (spawnBurstCount_ == 0 || spawned_ < spawnBurstCount_)) {

			SpawnInstance();
			++spawned_;
		}
		// 最大数にインスタンス数が行くまで発生
		if (spawnTimer_.IsReached() && (spawnBurstCount_ == 0 || spawned_ < spawnBurstCount_)) {

			SpawnInstance();
			++spawned_;
			spawnTimer_.Reset();
		}

		// 各インスタンスを更新、発生
		for (size_t i = 0; i < instances_.size();) {

			auto& inst = instances_[i];
			inst.updater.Update(inst.scale, inst.rotation);
			if (!inst.updater.CanEmit()) {
				++i;
				continue;
			}

			// 各インスタンスを発生させる
			EmitForInstance(inst, particles);

			// アニメ終了で破棄
			if (inst.updater.IsFinished()) {

				instances_.erase(instances_.begin() + i);
			} else {

				++i;
			}
		}
		return;
	}

	if (isSelfUpdate_) {

		const bool startThisFrame = !updater_.CanEmit();
		if (startThisFrame) {

			const float savedScale = scale_;
			const Vector3 savedRot = emitterRotation_;
			scale_ = updater_.GetStartScale();
			emitterRotation_ = updater_.GetStartRotation();
			prevVertices_ = CalcVertices();
			scale_ = savedScale;
			emitterRotation_ = savedRot;
		}
		updater_.Update(scale_, emitterRotation_);
	}

	if (!EnableEmit()) {

		return;
	}
	if (isInterpolate_) {

		InterpolateEmit(particles);
	} else {

		NoneEmit(particles);
	}
}

bool ParticleSpawnPolygonVertexModule::EnableEmit() {

	if (!notMoveEmit_) {
		return true;
	}

	bool moved = false;
	const std::vector<Vector3> currentVertices = CalcVertices();
	const size_t vertexCount = (std::min)(currentVertices.size(), prevVertices_.size());
	// 前フレームの頂点位置と比較する
	for (size_t i = 0; i < vertexCount; ++i) {
		if (std::numeric_limits<float>::epsilon() < (currentVertices[i] - prevVertices_[i]).Length()) {

			moved = true;
			break;
		}
	}
	return moved;
}

void ParticleSpawnPolygonVertexModule::InterpolateEmit(std::list<CPUParticle::ParticleData>& particles) {

	const std::vector<Vector3> currentVertices = CalcVertices();
	// 頂点数が変わった時は速度を0.0fにする
	if (prevVertices_.size() != currentVertices.size()) {
		prevVertices_ = currentVertices;
	}

	const float spacing = interpolateSpacing_.GetValue();
	const uint32_t emitPerVertex = emitPerVertex_.GetValue();
	for (uint32_t v = 0; v < static_cast<uint32_t>(currentVertices.size()); ++v) {

		const Vector3 diff = currentVertices[v] - prevVertices_[v];
		const float length = diff.Length();

		// 頂点が静止している場合は通常発生のみ
		if (length < spacing || length < std::numeric_limits<float>::epsilon()) {

			// 速度設定
			Vector3 velocity = Vector3::Normalize(diff) * moveSpeed_.GetValue();
			for (uint32_t n = 0; n < emitPerVertex; ++n) {

				CPUParticle::ParticleData particle{};

				// 共通設定
				ICPUParticleSpawnModule::SetCommonData(particle);

				// 速度、発生位置
				particle.velocity = velocity;
				particle.transform.translation = currentVertices[v];
				// 発生した瞬間の座標を記録
				particle.spawnTranlation = particle.transform.translation;

				// 追加
				particles.push_back(particle);
			}
			continue;
		}

		// 補間個数と方向
		const uint32_t interpCount = static_cast<uint32_t>(length / spacing);
		const Vector3 direction = diff / length; // 正規化
		const float stepLen = spacing;           // 等間隔
		const Vector3 velocity = direction * moveSpeed_.GetValue();

		// パーティクル間の補間
		for (uint32_t i = 1; i <= interpCount; ++i) {

			const Vector3 pos = prevVertices_[v] + direction * stepLen * static_cast<float>(i);
			for (uint32_t n = 0; n < emitPerVertex; ++n) {

				CPUParticle::ParticleData particle{};

				// 共通設定
				ICPUParticleSpawnModule::SetCommonData(particle);

				// 速度、発生位置
				particle.velocity = velocity;
				particle.transform.translation = pos;
				// 発生した瞬間の座標を記録
				particle.spawnTranlation = particle.transform.translation;

				// 追加
				particles.push_back(particle);
			}

		}

		// 現在のフレームの頂点位置に発生
		for (uint32_t n = 0; n < emitPerVertex; ++n) {

			CPUParticle::ParticleData particle{};

			// 共通設定
			ICPUParticleSpawnModule::SetCommonData(particle);

			// 速度、発生位置
			particle.velocity = velocity;
			particle.transform.translation = currentVertices[v];
			// 発生した瞬間の座標を記録
			particle.spawnTranlation = particle.transform.translation;

			// 追加
			particles.push_back(particle);
		}

	}
}

void ParticleSpawnPolygonVertexModule::NoneEmit(std::list<CPUParticle::ParticleData>& particles) {

	const std::vector<Vector3> currentVertices = CalcVertices();
	// 頂点数が変わった時は速度を0.0fにする
	if (prevVertices_.size() != currentVertices.size()) {
		prevVertices_ = currentVertices;
	}

	const uint32_t emitPerVertex = emitPerVertex_.GetValue();
	for (uint32_t cIndex = 0; cIndex < static_cast<uint32_t>(currentVertices.size()); ++cIndex) {

		// 進行方向ベクトル
		Vector3 direction = Vector3::Normalize(currentVertices[cIndex] - prevVertices_[cIndex]);
		bool isMoving = direction.Length() > std::numeric_limits<float>::epsilon();
		// 速度の設定
		Vector3 velocity = isMoving ? direction * moveSpeed_.GetValue() :
			Vector3(0.0f, 0.0f, moveSpeed_.GetValue());

		for (uint32_t pIndex = 0; pIndex < emitPerVertex; ++pIndex) {

			CPUParticle::ParticleData particle{};

			// 共通設定
			ICPUParticleSpawnModule::SetCommonData(particle);

			// 速度、発生位置
			particle.velocity = velocity;
			particle.transform.translation = currentVertices[cIndex];
			// 発生した瞬間の座標を記録
			particle.spawnTranlation = particle.transform.translation;

			// 追加
			particles.push_back(particle);
		}
	}
}

void ParticleSpawnPolygonVertexModule::ImGui() {

	ImGui::Checkbox("updateEnable", &updateEnable_);
	ImGui::Checkbox("notMoveEmit", &notMoveEmit_);
	ImGui::Checkbox("isInterpolate", &isInterpolate_);

	ImGui::DragFloat3("rotation", &emitterRotation_.x, 0.01f);
	ImGui::DragFloat("scale", &scale_, 0.05f);
	ImGui::DragFloat3("translation", &translation_.x, 0.05f);

	ImGui::DragInt("vertexCount", &vertexCount_, 1, 1, 12);
	emitPerVertex_.EditDragValue("emitPerVertex");

	if (!isInterpolate_) {
		return;
	}

	interpolateSpacing_.EditDragValue("spacing");

	ImGui::Checkbox("isSelfUpdate", &isSelfUpdate_);
	if (isSelfUpdate_) {

		updater_.ImGui();
	}

	ImGui::SeparatorText("Multi Emit");

	ImGui::Checkbox("useMulti", &useMulti_);
	if (!useMulti_) {
		return;
	}

	ImGui::Checkbox("multiEmit_", &multiEmit_);
	if (ImGui::Button("Reset##MultiSpawn")) {

		spawnTimer_.Reset();
		spawned_ = 0;
		instances_.clear();
	}

	ImGui::Text("spawned: %d", spawned_);
	ImGui::DragInt("spawnBurstCount", &spawnBurstCount_, 1, 1, 12);
	ImGui::DragInt("maxConcurrent", &maxConcurrent_, 1, 1, 12);
	ImGui::DragFloat3("offsetRotation", &offsetRotation_.x, 0.01f);

	spawnTimer_.ImGui("MultiSpawn");
}

void ParticleSpawnPolygonVertexModule::DrawEmitter() {

	LineRenderer* lineRenderer = LineRenderer::GetInstance();
	Vector3 parentTranslation{};
	// 親の座標
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}

	if (3 <= vertexCount_) {

		// 多角形の場合
		lineRenderer->DrawPolygon(vertexCount_,
			parentTranslation + translation_, scale_, emitterRotation_, emitterLineColor_);
	} else if (vertexCount_ == 2) {

		// 2頂点の場合
		const auto vertices = CalcVertices();
		lineRenderer->DrawLine3D(
			parentTranslation + vertices[0],
			parentTranslation + vertices[1], emitterLineColor_);
	} else {

		// 1頂点の場合
		const auto vertices = CalcVertices();
		lineRenderer->DrawSphere(4, 0.08f * scale_,
			parentTranslation + vertices[0], emitterLineColor_);
	}
}

Json ParticleSpawnPolygonVertexModule::ToJson() {

	Json data;

	// 共通設定
	ICPUParticleSpawnModule::ToCommonJson(data);

	data["isInterpolate"] = isInterpolate_;
	data["notMoveEmit"] = notMoveEmit_;
	data["isSelfUpdate"] = isSelfUpdate_;
	data["vertexCount"] = vertexCount_;
	data["scale"] = scale_;
	data["useMulti"] = useMulti_;
	data["spawnBurstCount"] = spawnBurstCount_;
	data["maxConcurrent"] = maxConcurrent_;
	data["emitterRotation"] = emitterRotation_.ToJson();
	data["translation"] = translation_.ToJson();
	data["offsetRotation"] = offsetRotation_.ToJson();

	emitPerVertex_.SaveJson(data, "emitPerVertex");
	interpolateSpacing_.SaveJson(data, "interpolateSpacing");
	updater_.ToJson(data["SelfUpdater"]);
	spawnTimer_.ToJson(data["MultiSpawn"]);

	return data;
}

void ParticleSpawnPolygonVertexModule::FromJson(const Json& data) {

	// 共通設定
	ICPUParticleSpawnModule::FromCommonJson(data);

	isInterpolate_ = data.value("isInterpolate", false);
	notMoveEmit_ = data.value("notMoveEmit", false);
	isSelfUpdate_ = data.value("isSelfUpdate", false);
	vertexCount_ = data.value("vertexCount", 3);
	scale_ = data.value("scale", 1.0f);

	emitterRotation_.FromJson(data["emitterRotation"]);
	translation_.FromJson(data["translation"]);

	emitPerVertex_.ApplyJson(data, "emitPerVertex");
	interpolateSpacing_.ApplyJson(data, "interpolateSpacing");

	if (data.contains("SelfUpdater")) {

		updater_.FromJson(data["SelfUpdater"]);
	}

	useMulti_ = data.value("useMulti", false);
	spawnBurstCount_ = data.value("spawnBurstCount", 1);
	maxConcurrent_ = data.value("maxConcurrent", 1);
	offsetRotation_ = Vector3::FromJson(data.value("offsetRotation", Json()));
	if (data.contains("MultiSpawn")) {

		spawnTimer_.FromJson(data["MultiSpawn"]);
	}
}