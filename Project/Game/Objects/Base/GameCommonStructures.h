#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector2.h>

// c++
#include <string>
// front
class GameObject2D;

//============================================================================
//	GameCommonStructures class
//============================================================================

namespace GameCommon {

	// HUDの初期化値
	struct HUDInitParameter {

		Vector2 translation; // 座標

		// imgui
		bool ImGui(const std::string& label);

		// json
		void ApplyJson(const Json& data);
		void SaveJson(Json& data);
	};

	void SetInitParameter(GameObject2D& sprite, const  GameCommon::HUDInitParameter& parameter);
}