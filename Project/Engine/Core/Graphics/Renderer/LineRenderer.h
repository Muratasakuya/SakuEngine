#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/GPUObject/VertexBuffer.h>
#include <Engine/Collision/CollisionGeometry.h>
#include <Engine/MathLib/MathUtils.h>

// c++
#include <memory>
// front
class SceneView;
class SRVDescriptor;
class DxShaderCompiler;

//============================================================================
//	enum class
//============================================================================

// 線の種類
enum class LineType {

	None,        // 通常描画
	DepthIgnore, // 深度値無視
};

//============================================================================
//	LineRenderer class
//============================================================================
class LineRenderer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void Init(ID3D12Device8* device, ID3D12GraphicsCommandList* commandList,
		SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler,
		SceneView* sceneView);

	void DrawLine3D(const Vector3& pointA, const Vector3& pointB, const Color& color,
		LineType type = LineType::None);

	void ExecuteLine(bool debugEnable, LineType type);

	void ResetLine();

	//--------- shapes ------------------------------------------------------

	void DrawGrid(int division, float gridSize, const Color& color, LineType type = LineType::None);

	void DrawSegment(int division, float radius, const Vector3& pointA,
		const Vector3& pointB, const Color& color, LineType type = LineType::None);

	// polygonCount <= 3
	template <typename T>
	void DrawPolygon(int polygonCount, const Vector3& centerPos, float scale,
		const T& rotation, const Color& color, LineType type = LineType::None);

	void DrawSphere(int division, float radius, const Vector3& centerPos,
		const Color& color, LineType type = LineType::None);

	void DrawAABB(const Vector3& min, const Vector3& max,
		const Color& color, LineType type = LineType::None);

	void DrawCircle(int division, float radius, const Vector3& center,
		const Color& color, LineType type = LineType::None);

	void DrawArc(int division, float radius, float halfAngle, const Vector3& center,
		const Vector3& direction, const Color& color, LineType type = LineType::None);

	void DrawSquare(float length, const Vector3& center,
		const Color& color, LineType type = LineType::None);

	template <typename T>
	void DrawHemisphere(int division, float radius, const Vector3& centerPos,
		const T& rotation, const Color& color, LineType type = LineType::None);

	template <typename T>
	void DrawOBB(const Vector3& centerPos, const Vector3& size,
		const T& rotation, const Color& color, LineType type = LineType::None);

	template <typename T>
	void DrawCone(int division, float baseRadius, float topRadius, float height,
		const Vector3& centerPos, const T& rotation, const Color& color, LineType type = LineType::None);

	//--------- accessor -----------------------------------------------------

	// singleton
	static LineRenderer* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 頂点情報
	struct LineVertex {

		Vector4 pos;
		Color color;
	};

	// 描画に使うデータ
	struct RenderStructure {

		std::unique_ptr<PipelineState> pipeline;

		std::vector<LineVertex> lineVertices;
		VertexBuffer<LineVertex> vertexBuffer;

		void Init(const std::string& pipelineFile, ID3D12Device8* device,
			SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);
	};

	//--------- variables ----------------------------------------------------

	static LineRenderer* instance_;

	// 線分の最大数
	static constexpr const uint32_t kMaxLineCount_ = 8196;
	// 線分の頂点数
	static constexpr const uint32_t kVertexCountLine_ = 2;

	ID3D12GraphicsCommandList* commandList_;
	SceneView* sceneView_;

	// カメラ視点
	DxConstBuffer<Matrix4x4> viewProjectionBuffer_;
	DxConstBuffer<Matrix4x4> debugSceneViewProjectionBuffer_;

	// 描画情報(buffer)
	std::unordered_map<LineType, RenderStructure> renderData_;

	//--------- functions ----------------------------------------------------

	LineRenderer() = default;
	~LineRenderer() = default;
	LineRenderer(const LineRenderer&) = delete;
	LineRenderer& operator=(const LineRenderer&) = delete;
};

//============================================================================
//	LineRenderer templateMethods
//============================================================================

template<typename T>
inline void LineRenderer::DrawPolygon(int polygonCount, const Vector3& centerPos,
	float scale, const T& rotation, const Color& color, LineType type) {

	if (polygonCount < 3 || polygonCount > 12) {
		return;
	}

	std::vector<Vector3> vertices;
	vertices.reserve(polygonCount);

	// 回転
	Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
	if constexpr (std::is_same_v<T, Vector3>) {

		rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
	} else if constexpr (std::is_same_v<T, Quaternion>) {

		rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
	} else if constexpr (std::is_same_v<T, Matrix4x4>) {

		rotationMatrix = rotation;
	}

	// 各頂点を計算
	for (int i = 0; i < polygonCount; ++i) {
		float angle = 2.0f * pi * static_cast<float>(i) / static_cast<float>(polygonCount);
		Vector3 localPos = { std::cos(angle) * scale, 0.0f, std::sin(angle) * scale };
		Vector3 worldPos = rotationMatrix.TransformPoint(localPos) + centerPos;
		vertices.push_back(worldPos);
	}

	// 線を描画
	for (int i = 0; i < polygonCount; ++i) {

		const Vector3& start = vertices[i];
		const Vector3& end = vertices[(i + 1) % polygonCount];
		DrawLine3D(start, end, color, type);
	}
}

template<typename T>
inline void LineRenderer::DrawHemisphere(int division, float radius, const Vector3& centerPos,
	const T& rotation, const Color& color, LineType type) {

	const float kLatEvery = (pi / 2.0f) / division; // 緯度
	const float kLonEvery = 2.0f * pi / division;   // 経度

	auto calculatePoint = [&](float lat, float lon) -> Vector3 {
		return {
			radius * std::cos(lat) * std::cos(lon),
			radius * std::sin(lat),
			radius * std::cos(lat) * std::sin(lon)
		};
		};

	Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
	if constexpr (std::is_same_v<T, Vector3>) {

		rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
	} else if constexpr (std::is_same_v<T, Quaternion>) {

		rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
	} else if constexpr (std::is_same_v<T, Matrix4x4>) {

		rotationMatrix = rotation;
	}

	for (int latIndex = 0; latIndex < division; ++latIndex) {
		float lat = kLatEvery * latIndex;
		for (int lonIndex = 0; lonIndex < division; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			Vector3 pointA = calculatePoint(lat, lon);
			Vector3 pointB = calculatePoint(lat + kLatEvery, lon);
			Vector3 pointC = calculatePoint(lat, lon + kLonEvery);

			pointA = rotationMatrix.TransformPoint(pointA) + centerPos;
			pointB = rotationMatrix.TransformPoint(pointB) + centerPos;
			pointC = rotationMatrix.TransformPoint(pointC) + centerPos;

			DrawLine3D(pointA, pointB, color, type);
			DrawLine3D(pointA, pointC, color, type);
		}
	}
}

template<typename T>
inline void LineRenderer::DrawOBB(const Vector3& centerPos, const Vector3& size,
	const T& rotation, const Color& color, LineType type) {

	const uint32_t vertexNum = 8;

	Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
	if constexpr (std::is_same_v<T, Vector3>) {

		rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
	} else if constexpr (std::is_same_v<T, Quaternion>) {

		rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
	} else if constexpr (std::is_same_v<T, Matrix4x4>) {

		rotationMatrix = rotation;
	}

	Vector3 vertices[vertexNum];
	Vector3 halfSizeX = Vector3::Transform(Vector3(1.0f, 0.0f, 0.0f), rotationMatrix) * size.x;
	Vector3 halfSizeY = Vector3::Transform(Vector3(0.0f, 1.0f, 0.0f), rotationMatrix) * size.y;
	Vector3 halfSizeZ = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), rotationMatrix) * size.z;

	Vector3 offsets[vertexNum] = {
		{-1, -1, -1}, {-1,  1, -1}, {1, -1, -1}, {1,  1, -1},
		{-1, -1,  1}, {-1,  1,  1}, {1, -1,  1}, {1,  1,  1}
	};

	for (int i = 0; i < vertexNum; ++i) {

		Vector3 localVertex = offsets[i].x * halfSizeX +
			offsets[i].y * halfSizeY +
			offsets[i].z * halfSizeZ;
		vertices[i] = centerPos + localVertex;
	}

	int edges[12][2] = {
		{0, 1}, {1, 3}, {3, 2}, {2, 0},
		{4, 5}, {5, 7}, {7, 6}, {6, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	};

	for (int i = 0; i < 12; ++i) {

		int start = edges[i][0];
		int end = edges[i][1];

		DrawLine3D(vertices[start], vertices[end], color, type);
	}
}

template<typename T>
inline void LineRenderer::DrawCone(int division, float baseRadius, float topRadius,
	float height, const Vector3& centerPos, const T& rotation, const Color& color, LineType type) {

	const float kAngleStep = 2.0f * pi / division;

	std::vector<Vector3> baseCircle;
	std::vector<Vector3> topCircle;

	// 基底円と上面円の計算
	for (int i = 0; i <= division; ++i) {

		float angle = i * kAngleStep;
		baseCircle.emplace_back(baseRadius * std::cos(angle), 0.0f, baseRadius * std::sin(angle));
		topCircle.emplace_back(topRadius * std::cos(angle), height, topRadius * std::sin(angle));
	}

	Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
	if constexpr (std::is_same_v<T, Vector3>) {

		rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
	} else if constexpr (std::is_same_v<T, Quaternion>) {

		rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
	} else if constexpr (std::is_same_v<T, Matrix4x4>) {

		rotationMatrix = rotation;
	}

	for (int i = 0; i < division; ++i) {

		// 円周上の点を回転＆平行移動
		Vector3 baseA = rotationMatrix.TransformPoint(baseCircle[i]) + centerPos;
		Vector3 baseB = rotationMatrix.TransformPoint(baseCircle[i + 1]) + centerPos;
		Vector3 topA = rotationMatrix.TransformPoint(topCircle[i]) + centerPos;
		Vector3 topB = rotationMatrix.TransformPoint(topCircle[i + 1]) + centerPos;

		// 円の描画
		DrawLine3D(baseA, baseB, color, type);
		DrawLine3D(topA, topB, color, type);

		// 側面の描画
		DrawLine3D(baseA, topA, color, type);
	}
}