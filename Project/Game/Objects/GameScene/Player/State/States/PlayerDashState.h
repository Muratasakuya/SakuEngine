#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>

//============================================================================
//	PlayerDashState class
//============================================================================
class PlayerDashState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerDashState() = default;
	~PlayerDashState() = default;

	void Enter(Player& player) override;

	void Update(Player& player) override;

	void Exit(Player& player) override;

	// imgui
	void ImGui(const Player& player) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	bool GetCanExit() const override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	enum class State {

		Accel,   // 加速
		Sustain, // 待ち
		Decel    // 減速
	};

	//--------- variables ----------------------------------------------------

	State currentState_;

	Vector3 move_;    // 移動量
	float moveSpeed_; // 移動速度
	float sustainTimer_; // 最高速を維持する時間経過
	float sustainTime_;  // 最高速を維持する時間

	// ダッシュの速度補間
	std::unique_ptr<SimpleAnimation<float>> accelLerp_;
	std::unique_ptr<SimpleAnimation<float>> decelLerp_;

	//--------- functions ----------------------------------------------------

	void UpdateState();
	void UpdateDash(Player& player);
};