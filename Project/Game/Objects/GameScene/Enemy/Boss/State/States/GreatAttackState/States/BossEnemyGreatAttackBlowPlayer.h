#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/Interface/BossEnemyGreatAttackIState.h>

//============================================================================
//	BossEnemyGreatAttackBlowPlayer class
//	プレイヤーを遠くに吹っ飛ばす状態
//============================================================================
class BossEnemyGreatAttackBlowPlayer :
	public BossEnemyGreatAttackIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackBlowPlayer();
	~BossEnemyGreatAttackBlowPlayer() = default;

	// 状態遷移時
	void Enter() override;

	// 更新処理
	void Update() override;

	// 状態終了時
	void Exit() override;

	// imgui
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 座標の補間
	std::unique_ptr<SimpleAnimation<Vector3>> animTranslation_;

	//--------- functions ----------------------------------------------------

};