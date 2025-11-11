#include "CameraPathRenderer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>

//============================================================================
//	CameraPathRenderer classMethods
//============================================================================

void CameraPathRenderer::DrawLine3D(const CameraPathData& data) {

	if (data.keyframes.size() < 2) {
		return;
	}

	const auto points = data.CollectTranslationPoints();
	Vector3 prev{};
	bool hasPrev = false;
	for (int i = 0; i <= data.divisionCount; ++i) {

		float t = static_cast<float>(i) / static_cast<float>(data.divisionCount);
		Vector3 p = LerpKeyframe::GetValue<Vector3>(points, t, data.lerpType);
		if (hasPrev) {

			LineRenderer::GetInstance()->DrawLine3D(prev, p, Color::Yellow());
		}
		prev = p;
		hasPrev = true;
	}
}