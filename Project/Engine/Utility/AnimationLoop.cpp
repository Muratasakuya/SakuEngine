#include "AnimationLoop.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/EnumAdapter.h>

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

	ImGui::DragInt("loopCount", &loopCount_, 1, 1, 64);
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