#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBufferSize.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	PlayerIdleState class
//============================================================================
class PlayerIdleState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerIdleState();
	~PlayerIdleState() = default;

	void Enter(Player& player) override;

	void Update(Player& player) override;
	void UpdateAlways(Player& player) override;

	void Exit(Player& player) override;

	// imgui
	void ImGui(const Player& player) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetBlurParam(RadialBlurForGPU blur) { startRadialBlur_ = blur; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// postProcess
	float blurTimer_; // ブラーの経過時間
	float blurTime_;  // ブラーの時間
	EasingType blurEasingType_;
	RadialBlurForGPU radialBlur_;       // systemに渡す値
	RadialBlurForGPU startRadialBlur_;  // 開始値
	RadialBlurForGPU targetRadialBlur_; // 初期化値
};