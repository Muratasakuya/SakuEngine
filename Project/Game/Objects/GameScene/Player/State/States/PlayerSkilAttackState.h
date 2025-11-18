#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/KeyframeObject3D.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerBaseAttackState.h>

//============================================================================
//	PlayerSkilAttackState class
//	スキル攻撃
//============================================================================
class PlayerSkilAttackState :
	public PlayerBaseAttackState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerSkilAttackState(Player* player);
	~PlayerSkilAttackState() = default;

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

	// 次の状態に遷移可能かどうか
	bool GetCanExit() const override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 座標移動のキーフレーム
	std::unique_ptr<KeyframeObject3D> moveKeyframeObject_;
	// 空の親トランスフォーム、敵が範囲内にいないときに参照する親
	Transform3D* moveFrontTransform_;
	// タグ
	ObjectTag* moveFrontTag_;

	// 移動の前座標
	Vector3 preMovePos_;

	//--------- functions ----------------------------------------------------

};