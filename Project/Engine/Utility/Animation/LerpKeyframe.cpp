#include "LerpKeyframe.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>

//============================================================================
//	LerpKeyframe classMethods
//============================================================================

float LerpKeyframe::GetReparameterizedT(float t, const std::vector<float>& averagedT) {

	// なにも値が入ってない
	assert(!averagedT.empty() && "averagedT is empty");
	for (uint32_t i = 1; i < averagedT.size(); ++i) {
		if (t < averagedT[i]) {

			// 2つの間で補間する
			float segmentT = (t - averagedT[i - 1]) / (averagedT[i] - averagedT[i - 1]);
			float result = (static_cast<float>(i - 1) + segmentT) / static_cast<float>(averagedT.size() - 1);
			return result;
		}
	}
	return t;
}

void LerpKeyframe::DrawKeyframeLine(const std::vector<Vector3>& points,
	LerpKeyframe::Type type, bool isConnectEnds) {

	for (size_t i = 0; i + 1 < points.size(); ++i) {

		// 線を分割する数
		constexpr int division = 16;
		Vector3 previousPoint = points[i];
		for (int j = 1; j <= division; ++j) {

			float t = static_cast<float>(j) / static_cast<float>(division);
			Vector3 currentPoint = LerpKeyframe::GetValue<Vector3>(points,
				isConnectEnds ? (t + static_cast<float>(i)) / points.size() :
				(t + static_cast<float>(i)) / (points.size() - 1), type);

			// 線を描画
			LineRenderer::GetInstance()->DrawLine3D(
				previousPoint, currentPoint, Color::Cyan());

			// 現在の点を前の点に更新
			previousPoint = currentPoint;
		}
	}
}