#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	PlayerAvoidSatate class
//============================================================================
class PlayerAvoidSatate :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAvoidSatate() = default;
	~PlayerAvoidSatate() = default;

	void Enter(Player& player) override;

	void Update(Player& player) override;

	void Exit(Player& player) override;

	// imgui
	void ImGui(const Player& player) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// parameters
	float lerpTimer_; // 補間時間
	float lerpTime_;  // 補間にかける時間
	EasingType easingType_;
	float moveDistance_; // 移動距離

	Vector3 startPos_;  // 開始座標
	Vector3 targetPos_; // 目標座標
};