#include "ParticleUpdateTrailModule.h"

//============================================================================
//	ParticleUpdateTrailModule classMethods
//============================================================================

void ParticleUpdateTrailModule::Init() {

	// 初期化値
	param_.enable = false;  // 最初はfalse
	param_.lifeTime = 0.8f;
	param_.width = 0.5f;

	// 最初は距離でノードを作成するようにする
	param_.minDistance = 0.05f;
	param_.minTime = 0.0f;

	param_.maxPoints = 64;
	param_.subdivPerSegment = 0;
	param_.faceCamera = true;
	param_.uvTileLength = 0.32f;
}

void ParticleUpdateTrailModule::Execute(
	CPUParticle::ParticleData& particle, float deltaTime) {

	ParticleCommon::TrailRuntime& trail = particle.trailRuntime;

	// ノードの寿命を更新
	for (auto& node : trail.nodes) {

		node.age += deltaTime;
		// 寿命が尽きていたら削除
		while (!trail.nodes.empty() &&
			param_.lifeTime < trail.nodes.front().age) {

			trail.nodes.pop_front();
		}
	}

	// 現在の座標
	const Vector3 currentPos = particle.transform.translation;

	// 未初期化なら初期化する
	if (!trail.isInitialized) {

		trail.isInitialized = true;
		trail.prePos = currentPos;

		// 初期化してノード追加
		ParticleCommon::TrailPoint point{};
		point.pos = particle.transform.translation;
		point.age = 0.0f;
		trail.nodes.emplace_back(point);
		return;
	}

	// サンプル条件
	trail.time += deltaTime;
	float distance = Vector3(currentPos - trail.prePos).Length();
	bool passTime = (0.0f < param_.minTime && param_.minTime <= trail.time);
	// minTimeが0.0f以上なら時間経過置きに発生させる
	// それ以外は距離で発生
	if (param_.minDistance <= distance || passTime) {

		// 等間隔で分割して複数ノード押し込み
		int steps = (std::max)(1, static_cast<int>(std::floor(distance / (std::max)(1e-6f, param_.minDistance))));
		Vector3 pointA = trail.prePos;
		Vector3 pointB = currentPos;
		for (int i = 1; i <= steps; i++) {

			float t = static_cast<float>(i) / steps;
			ParticleCommon::TrailPoint point{};
			point.age = 0.0f;
			point.pos = pointA * (1.0f - t) + pointB * t;
			// 最大数を超えたら削除
			if (param_.maxPoints <= static_cast<int>(trail.nodes.size())) {

				trail.nodes.pop_front();
			}
			trail.nodes.emplace_back(point);
		}
		// 前座標を記録
		trail.prePos = currentPos;
		trail.time = 0.0f;
	}
}

void ParticleUpdateTrailModule::ImGui() {

	ImGui::Checkbox("enable", &param_.enable);
	ImGui::Checkbox("faceCamera", &param_.faceCamera);

	ImGui::DragFloat("lifeTime", &param_.lifeTime, 0.01f);
	ImGui::DragFloat("width", &param_.width, 0.01f);
	ImGui::DragFloat("minDistance", &param_.minDistance, 0.01f);
	ImGui::DragFloat("minTime", &param_.minTime, 0.01f);

	ImGui::DragInt("maxPoints", &param_.maxPoints, 1, 0, 64);
	ImGui::DragInt("subdivPerSegment", &param_.subdivPerSegment, 1, 1, 64);
	ImGui::DragFloat("uvTileLength", &param_.uvTileLength, 0.01f);
}

Json ParticleUpdateTrailModule::ToJson() {

	Json data;

	data["enable"] = param_.enable;
	data["faceCamera"] = param_.faceCamera;

	data["lifeTime"] = param_.lifeTime;
	data["width"] = param_.width;
	data["minDistance"] = param_.minDistance;
	data["minTime"] = param_.minTime;
	data["uvTileLength"] = param_.uvTileLength;

	data["maxPoints"] = param_.maxPoints;
	data["subdivPerSegment"] = param_.subdivPerSegment;

	return data;
}

void ParticleUpdateTrailModule::FromJson(const Json& data) {

	param_.enable = data.value("enable", false);
	param_.faceCamera = data.value("faceCamera", false);

	param_.lifeTime = data.value("lifeTime", 0.0f);
	param_.width = data.value("width", 0.0f);
	param_.minDistance = data.value("minDistance", 0.0f);
	param_.minTime = data.value("minTime", 0.0f);
	param_.uvTileLength = data.value("uvTileLength", 0.0f);

	param_.maxPoints = data.value("maxPoints", 0);
	param_.subdivPerSegment = data.value("subdivPerSegment", 0);
}