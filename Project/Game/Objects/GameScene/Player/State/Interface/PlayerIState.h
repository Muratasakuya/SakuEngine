#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/InputMapper.h>
#include <Game/Objects/GameScene/Player/Structures/PlayerStructures.h>
#include <Game/Objects/GameScene/Player/Input/PlayerInputAction.h>
#include <Engine/MathLib/MathUtils.h>

// front
class Player;
class BossEnemy;
class FollowCamera;
class PostProcessSystem;

//============================================================================
//	PlayerIState class
//	プレイヤー状態のインターフェース
//============================================================================
class PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerIState();
	virtual ~PlayerIState() = default;

	// 状態遷移時
	virtual void Enter(Player& player) = 0;

	// 更新処理
	virtual void Update(Player& player) = 0;
	virtual void UpdateAlways([[maybe_unused]] Player& player) {}

	// 状態終了時
	virtual void Exit(Player& player) = 0;

	// imgui
	virtual void ImGui(const Player& player) = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetInputMapper(const InputMapper<PlayerInputAction>* inputMapper) { inputMapper_ = inputMapper; }
	void SetBossEnemy(const BossEnemy* bossEnemy) { bossEnemy_ = bossEnemy; }
	void SetFollowCamera(FollowCamera* followCamera) { followCamera_ = followCamera; }

	void SetCanExit(bool canExit) { canExit_ = canExit; }
	void SetPreState(PlayerState preState) { preState_ = preState; }

	virtual bool GetCanExit() const { return canExit_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const InputMapper<PlayerInputAction>* inputMapper_;
	const BossEnemy* bossEnemy_;
	FollowCamera* followCamera_;
	Player* player_;
	PostProcessSystem* postProcess_;

	// 遷移前の状態
	PlayerState preState_;

	// 共通parameters
	const float epsilon_ = std::numeric_limits<float>::epsilon();

	float nextAnimDuration_; // 次のアニメーション遷移にかかる時間
	bool canExit_ = true;    // 遷移可能かどうか
	float rotationLerpRate_; // 回転補間割合

	float targetCameraRotateX_; // 目標カメラX軸回転

	//--------- functions ----------------------------------------------------

	// helper
	void SetRotateToDirection(Player& player, const Vector3& move);
};