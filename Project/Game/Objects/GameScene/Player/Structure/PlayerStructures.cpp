#include "PlayerStructures.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	PlayerStructures Methods
//============================================================================

void PlayerStateCondition::FromJson(const Json& data) {

	coolTime = JsonAdapter::GetValue<float>(data, "coolTime");
	chainInputTime = JsonAdapter::GetValue<float>(data, "chainInputTime");
	isArmor = data.value("isArmor", false);
	enableInARowForceState = data.value("enableInARowForceState", false);

	auto intsToStates = [](const Json& array) {
		std::vector<PlayerState> out;
		for (int vector : JsonAdapter::ToVector<int>(array)) {
			out.push_back(static_cast<PlayerState>(vector));
		}
		return out;
		};
	allowedPreState = intsToStates(data["allowedPreState"]);
	interruptableBy = intsToStates(data["interruptableBy"]);
}

void PlayerStateCondition::ToJson(Json& data) {

	data["coolTime"] = coolTime;
	data["chainInputTime"] = chainInputTime;
	data["isArmor"] = isArmor;
	data["enableInARowForceState"] = enableInARowForceState;

	auto statesToInts = [](const std::vector<PlayerState>& vector) {
		std::vector<int> output;
		for (auto state : vector) {
			output.push_back(static_cast<int>(state));
		}return output;
		};
	data["allowedPreState"] = JsonAdapter::FromVector<int>(statesToInts(allowedPreState));
	data["interruptableBy"] = JsonAdapter::FromVector<int>(statesToInts(interruptableBy));
}

bool PlayerStateCondition::CheckInterruptableByState(PlayerState current) {

	// 含まれているかチェック
	return !interruptableBy.empty() && std::ranges::find(interruptableBy, current) != interruptableBy.end();
}