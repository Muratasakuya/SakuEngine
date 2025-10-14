#include "ParticleUpdateTrailModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/SceneView.h>

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

void ParticleUpdateTrailModule::BuildTransferData(uint32_t particleIndex,
	const CPUParticle::ParticleData& particle,
	std::vector<ParticleCommon::TrailHeaderForGPU>& transferTrailHeaders,
	std::vector<ParticleCommon::TrailVertexForGPU>& transferTrailVertices,
	const SceneView* sceneView) {

	// パーティクルのトレイルノード
	const auto& nodes = particle.trailRuntime.nodes;

	// デフォルトは空
	uint32_t start = static_cast<uint32_t>(transferTrailVertices.size());
	uint32_t vertexCount = 0;

	// CPUParticleGroup 側のコード互換のため同名変数を用意
	const ParticleUpdateTrailModule* trailModule = this;

	// トレイルしないなら頂点数は0にしてセット
	if (!trailModule || !trailModule->GetParam().enable || nodes.size() < 2) {
		transferTrailHeaders[particleIndex] = { start, 0 };
		return;
	}

	// 半分のサイズ
	const float halfWidth = 0.5f * param_.width;
	// U(横)方向の距離累積
	float uAccum = 0.0f;
	Vector3 prevSide = Vector3(1.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < nodes.size(); ++i) {

		// 方向ベクトル
		Vector3 direction;
		// 現在の座標から前の座標を引いた方向
		if (i == 0) {
			direction = nodes[1].pos - nodes[0].pos;
		} else if (i == nodes.size() - 1) {
			direction = nodes[i].pos - nodes[i - 1].pos;
		} else {
			direction = nodes[i + 1].pos - nodes[i - 1].pos;
		}

		float directionLength = direction.Length();
		if (directionLength > 1e-6f) {
			direction /= directionLength;
		} else {
			direction = Vector3(0.0f, 0.0f, 1.0f);
		}

		// 帯の左右方向
		Vector3 side;
		if (param_.faceCamera) {

			// カメラ面に帯が立つようにする
			side = Vector3::Cross(direction, sceneView->GetCamera()->GetTransform().GetForward());
		} else {

			const Vector3 up(0.0f, 1.0f, 0.0f);
			side = Vector3::Cross(direction, up);
		}

		// 反転しないようにする
		if (Vector3::Dot(prevSide, side) < 0.0f) {
			side = -side;
		}

		if (side.Length() < 1e-8f) {
			// 直前を利用
			side = prevSide;
		}
		prevSide = side = side.Normalize();

		// 左右頂点
		const Vector3 center = nodes[i].pos;
		const Vector3 leftWorldPos = center - side * halfWidth;
		const Vector3 rightWorldPos = center + side * halfWidth;

		// 頂点カラー、αフェードさせる
		float fade = 1.0f;
		if (1e-6f < param_.lifeTime) {
			fade = std::clamp(1.0f - nodes[i].age / param_.lifeTime, 0.0f, 1.0f);
		}

		// 距離ベースでタイリングする
		float u = (1e-6f < param_.uvTileLength) ? (uAccum / param_.uvTileLength) : 0.0f;

		// 左と右の頂点情報をセットして追加
		ParticleCommon::TrailVertexForGPU leftVertex;
		ParticleCommon::TrailVertexForGPU rightVertex;
		// 左
		leftVertex.worldPos = leftWorldPos;
		leftVertex.uv = Vector2(u, 0.0f);
		leftVertex.color = particle.material.color;
		// 右
		rightVertex.worldPos = rightWorldPos;
		rightVertex.uv = Vector2(u, 1.0f);
		rightVertex.color = particle.material.color;

		transferTrailVertices.push_back(leftVertex);
		transferTrailVertices.push_back(rightVertex);

		// 次のノードがあれば距離を加算する
		if (i + 1 < nodes.size()) {
			uAccum += (nodes[i + 1].pos - nodes[i].pos).Length();
		}
	}

	// 偶数個に制限、奇数にすると頂点数とポリゴンの数が合わなくなるため
	uint32_t end = static_cast<uint32_t>(transferTrailVertices.size());
	vertexCount = end - start;
	if (vertexCount & 1u) {
		transferTrailVertices.pop_back();
		--vertexCount;
	}

	// 4未満ならすべて削除
	if (vertexCount < 4) {

		// 使わないぶんは戻す
		transferTrailVertices.resize(start);
		vertexCount = 0;
	}
	transferTrailHeaders[particleIndex] = { start, vertexCount };
}

void ParticleUpdateTrailModule::ImGui() {

	ImGui::Checkbox("enable", &param_.enable);
	ImGui::Checkbox("isDrawOrigin", &param_.isDrawOrigin);
	ImGui::Checkbox("faceCamera", &param_.faceCamera);

	ImGui::DragFloat("lifeTime", &param_.lifeTime, 0.01f);
	ImGui::DragFloat("width", &param_.width, 0.01f);

	// minDistanceが0.0f以下になるとDevice君がRemoveする
	ImGui::DragFloat("minDistance", &param_.minDistance, 0.01f, 0.01f);
	ImGui::DragFloat("minTime", &param_.minTime, 0.01f);

	ImGui::DragInt("maxPoints", &param_.maxPoints, 1, 0, 64);
	ImGui::DragInt("subdivPerSegment", &param_.subdivPerSegment, 1, 1, 64);
	ImGui::DragFloat("uvTileLength", &param_.uvTileLength, 0.01f);
}

Json ParticleUpdateTrailModule::ToJson() {

	Json data;

	data["enable"] = param_.enable;
	data["isDrawOrigin"] = param_.isDrawOrigin;
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
	param_.isDrawOrigin = data.value("isDrawOrigin", true);
	param_.faceCamera = data.value("faceCamera", false);

	param_.lifeTime = data.value("lifeTime", 0.0f);
	param_.width = data.value("width", 0.0f);
	param_.minDistance = data.value("minDistance", 0.0f);
	param_.minTime = data.value("minTime", 0.0f);
	param_.uvTileLength = data.value("uvTileLength", 0.0f);

	param_.maxPoints = data.value("maxPoints", 0);
	param_.subdivPerSegment = data.value("subdivPerSegment", 0);
}