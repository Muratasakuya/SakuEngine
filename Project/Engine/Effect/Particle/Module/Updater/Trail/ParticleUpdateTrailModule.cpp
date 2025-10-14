#include "ParticleUpdateTrailModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/SceneView.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateTrailModule classMethods
//============================================================================

void ParticleUpdateTrailModule::Init() {

	// 初期化値
	enable_ = false;  // 最初はfalse
	lifeTime_ = 0.8f;
	width_.start = 0.5f;
	width_.target = 0.5f;

	// 最初は距離でノードを作成するようにする
	minDistance_ = 0.05f;
	minTime_ = 0.0f;

	maxPoints_ = 64;
	subdivPerSegment_ = 0;
	faceCamera_ = true;
	uvTileLength_ = 0.32f;
}

void ParticleUpdateTrailModule::Execute(
	CPUParticle::ParticleData& particle, float deltaTime) {

	ParticleCommon::TrailRuntime& trail = particle.trailRuntime;

	// ノードの寿命を更新
	for (auto& node : trail.nodes) {

		node.age += deltaTime;
		// 寿命が尽きていたら削除
		while (!trail.nodes.empty() &&
			lifeTime_ < trail.nodes.front().age) {

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
	bool passTime = (0.0f < minTime_ && minTime_ <= trail.time);
	// minTimeが0.0f以上なら時間経過置きに発生させる
	// それ以外は距離で発生
	if (minDistance_ <= distance || passTime) {

		// 等間隔で分割して複数ノード押し込み
		int steps = (std::max)(1, static_cast<int>(std::floor(distance / (std::max)(1e-6f, minDistance_))));
		Vector3 pointA = trail.prePos;
		Vector3 pointB = currentPos;
		for (int i = 1; i <= steps; i++) {

			float t = static_cast<float>(i) / steps;
			ParticleCommon::TrailPoint point{};
			point.age = 0.0f;
			point.pos = pointA * (1.0f - t) + pointB * t;
			// 最大数を超えたら削除
			if (maxPoints_ <= static_cast<int>(trail.nodes.size())) {

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

	// トレイルしないなら頂点数は0にしてセット
	if (!enable_ || nodes.size() < 2) {
		transferTrailHeaders[particleIndex] = { start, 0 };
		return;
	}

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
		if (faceCamera_) {

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

		// 各ノードの進捗率
		float progress = std::clamp(nodes[i].age / lifeTime_, 0.0f, 1.0f);
		// 色を取得し、αのみ寿命でフェードさせる
		Color color = particle.material.color;
		color.a = 1.0f - progress;

		// 幅
		float lerpedWidth = std::lerp(width_.start,
			width_.target, EasedValue(widthEasing_, progress));
		// 半分にする
		float halfWidth = lerpedWidth * 0.5f;

		// 左右頂点
		const Vector3 center = nodes[i].pos;
		const Vector3 leftWorldPos = center - side * halfWidth;
		const Vector3 rightWorldPos = center + side * halfWidth;

		// 距離ベースでタイリングする
		float u = (1e-6f < uvTileLength_) ? (uAccum / uvTileLength_) : 0.0f;

		// 左と右の頂点情報をセットして追加
		ParticleCommon::TrailVertexForGPU leftVertex;
		ParticleCommon::TrailVertexForGPU rightVertex;
		// 左
		leftVertex.worldPos = leftWorldPos;
		leftVertex.uv = Vector2(u, 0.0f);
		leftVertex.color = color;
		// 右
		rightVertex.worldPos = rightWorldPos;
		rightVertex.uv = Vector2(u, 1.0f);
		rightVertex.color = color;

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

	ImGui::Checkbox("enable", &enable_);
	ImGui::Checkbox("isDrawOrigin", &isDrawOrigin_);
	ImGui::Checkbox("faceCamera", &faceCamera_);

	ImGui::DragFloat("lifeTime", &lifeTime_, 0.01f);

	ImGui::DragFloat("startWidth", &width_.start, 0.01f);
	ImGui::DragFloat("targetWidth", &width_.target, 0.01f);
	EnumAdapter<EasingType>::Combo("widthEasing", &widthEasing_);

	// minDistanceが0.0f以下になるとDevice君がRemoveする
	ImGui::DragFloat("minDistance", &minDistance_, 0.01f, 0.01f);
	ImGui::DragFloat("minTime", &minTime_, 0.01f);

	ImGui::DragInt("maxPoints", &maxPoints_, 1, 0, 64);
	ImGui::DragInt("subdivPerSegment", &subdivPerSegment_, 1, 1, 64);
	ImGui::DragFloat("uvTileLength", &uvTileLength_, 0.01f);
}

Json ParticleUpdateTrailModule::ToJson() {

	Json data;

	data["enable"] = enable_;
	data["isDrawOrigin"] = isDrawOrigin_;
	data["faceCamera"] = faceCamera_;

	data["lifeTime"] = lifeTime_;
	data["startWidth"] = width_.start;
	data["targetWidth"] = width_.target;
	data["widthEasing_"] = EnumAdapter<EasingType>::ToString(widthEasing_);
	data["minDistance"] = minDistance_;
	data["minTime"] = minTime_;
	data["uvTileLength"] = uvTileLength_;

	data["maxPoints"] = maxPoints_;
	data["subdivPerSegment"] = subdivPerSegment_;

	return data;
}

void ParticleUpdateTrailModule::FromJson(const Json& data) {

	enable_ = data.value("enable", false);
	isDrawOrigin_ = data.value("isDrawOrigin", true);
	faceCamera_ = data.value("faceCamera", false);

	lifeTime_ = data.value("lifeTime", 0.0f);
	width_.start = data.value("startWidth", 0.5f);
	width_.target = data.value("targetWidth", 0.5f);
	widthEasing_ = EnumAdapter<EasingType>::FromString(
		data.value("widthEasing_", "EaseInSine")).value();

	minDistance_ = data.value("minDistance", 0.0f);
	minTime_ = data.value("minTime", 0.0f);
	uvTileLength_ = data.value("uvTileLength", 0.0f);

	maxPoints_ = data.value("maxPoints", 0);
	subdivPerSegment_ = data.value("subdivPerSegment", 0);
}