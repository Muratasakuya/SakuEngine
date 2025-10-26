#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Engine/Effect/User/EffectGroup.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	PlayerParryState class
//	パリィ状態、入力に応じてカウンター攻撃を行う
//============================================================================
class PlayerParryState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerParryState();
	~PlayerParryState() = default;

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

	void SetAllowAttack(bool allow) { allowAttack_ = allow; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class RequestState {

		PlayAnimation,  // animationの再生を行う
		AttackAnimation // animationの再生中の処理
	};

	struct LerpParameter {

		float timer; // 補間時間
		float time;  // 補間にかける時間
		EasingType easingType;
		float moveDistance; // 移動距離
		bool isFinised;     // 座標補間が終了したか
	};

	//--------- variables ----------------------------------------------------

	// 攻撃制御
	bool allowAttack_;
	bool isEmitedBlur_;

	// parameters
	LerpParameter parryLerp_;
	LerpParameter attackLerp_;

	Vector3 startPos_;  // 開始座標
	Vector3 targetPos_; // 目標座標

	float deltaWaitTimer_; // deltaTimeが元に戻るまでの時間経過
	float deltaWaitTime_;  // deltaTimeが元に戻るまでの時間
	float deltaLerpSpeed_; // 補間速度
	float cameraLookRate_; // カメラ補間速度

	std::optional<RequestState> request_;

	// パリィエフェクト
	std::unique_ptr<EffectGroup> parryEffect_;
	float parryEffectPosY_; // エフェクトのY座標

	//--------- functions ----------------------------------------------------

	void UpdateDeltaWaitTime(const Player& player);
	void UpdateLerpTranslation(Player& player);
	void CheckInput();
	void UpdateAnimation(Player& player);

	// helper
	Vector3 GetLerpTranslation(LerpParameter& lerp);
	Vector3 SetLerpValue(Vector3& start, Vector3& target,
		const Player& player, float moveDistance, bool isPlayerBase);
};