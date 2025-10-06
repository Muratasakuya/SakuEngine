#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/Easing.h>
#include <Engine/MathLib/Vector2.h>

// c++
#include <string>
#include <optional>

//============================================================================
//	StateTimer class
//============================================================================
class StateTimer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	StateTimer() = default;
	~StateTimer() = default;

	void Update(const std::optional<float>& target = std::nullopt);

	void Reset();

	bool IsReached() const;

	// editor
	void ImGui(const std::string& name, bool isSeparate = true);

	// json
	void FromJson(const Json& data);
	void ToJson(Json& data);

	//--------- variables ----------------------------------------------------

	float current_; // 現在の時間
	float target_;  // 目標時間
	float t_;       // イージングされていない補間T
	float easedT_;  // イージングした補間T

	EasingType easeingType_;
};