#include "LerpKeyframe.h"

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