#include "ParticleLoopableModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleLoopableModule classMethods
//============================================================================

void ParticleLoopableModule::ImGuiLoopParam() {

	ImGui::SeparatorText("Loop");

	ImGui::DragInt("loopCount", &loopCount_, 1, 1, 64);
	EnumAdapter<ParticleLoop::Type>::Combo("loopType", &loopType_);
}

void ParticleLoopableModule::ToLoopJson(Json& data) {

	const std::string key = "loop";

	data[key]["loopCount"] = loopCount_;
	data[key]["type"] = EnumAdapter<ParticleLoop::Type>::ToString(loopType_);
}

void ParticleLoopableModule::FromLoopJson(const Json& data) {

	const std::string key = "loop";
	if (data.contains(key)) {

		loopCount_ = data[key].value("loopCount", 1);

		const auto& type = EnumAdapter<ParticleLoop::Type>::FromString(data[key]["type"]);
		loopType_ = type.value();
	} else {

		loopCount_ = 1;
		loopType_ = ParticleLoop::Type::Repeat;
	}
}