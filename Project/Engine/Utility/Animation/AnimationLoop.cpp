#include "AnimationLoop.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	AnimationLoop classMethods
//============================================================================

float AnimationLoop::LoopedT(float rawT) {

	if (loopCount_ <= 1) {

		return std::clamp(rawT, 0.0f, 1.0f);
	}

	float t = rawT * static_cast<float>(loopCount_);
	if (loopType_ == AnimationLoopType::Repeat) {

		t = std::fmod(t, 1.0f);
	} else if (loopType_ == AnimationLoopType::PingPong) {

		t = std::fmod(t, 2.0f);
		if (t > 1.0f) {

			t = 2.0f - t;
		}
	}
	return t;
}

void AnimationLoop::ImGuiLoopParam(bool isSeparate) {

	if (isSeparate) {

		ImGui::SeparatorText("Loop");
	}

	int32_t loopCount = static_cast<int32_t>(loopCount_);
	if (ImGui::DragInt("loopCount", &loopCount, 1, 1)) {

		loopCount_ = static_cast<uint32_t>(loopCount);
	}
	EnumAdapter<AnimationLoopType>::Combo("loopType", &loopType_);
}

void AnimationLoop::ToLoopJson(Json& data) {

	const std::string key = "loop";

	data[key]["loopCount"] = loopCount_;
	data[key]["type"] = EnumAdapter<AnimationLoopType>::ToString(loopType_);
}

void AnimationLoop::FromLoopJson(const Json& data) {

	const std::string key = "loop";
	if (data.contains(key)) {

		loopCount_ = data[key].value("loopCount", 1);

		const auto& type = EnumAdapter<AnimationLoopType>::FromString(data[key]["type"]);
		loopType_ = type.value();
	} else {

		loopCount_ = 1;
		loopType_ = AnimationLoopType::Repeat;
	}
}

bool AnimationLoop::IsReachedEnd(float prevRawT, float currentRawT, float start, float end) const {

	auto normalize = [start, end](float t) -> float {
		float len = end - start;
		if (len <= 0.0f) { return 0.0f; }
		return std::clamp((t - start) / len, 0.0f, 1.0f);
		};

	float prevNorm = normalize(prevRawT);
	float curNorm = normalize(currentRawT);

	if (loopCount_ <= 1) {
		return (prevNorm < 1.0f && curNorm >= 1.0f);
	}

	// 0~loopCount_の周回フェーズを計算
	float prevPhase = prevNorm * static_cast<float>(loopCount_);
	float curPhase = curNorm * static_cast<float>(loopCount_);

	uint32_t prevIndex = static_cast<uint32_t>(prevPhase); // 前フレームの何周目か
	uint32_t curIndex = static_cast<uint32_t>(curPhase);   // 今フレームの何周目か

	// 周インデックスが増えた瞬間＝前のループのendを通過した瞬間
	return prevIndex < curIndex;

}