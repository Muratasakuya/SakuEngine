#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/KeyframeObject3D.h>
#include <Engine/Utility/Timer/DelayedHitstop.h>
#include <Game/Objects/GameScene/Player/Effect/PlayerAfterImageEffect.h>
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
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		MoveAttack, // 移動攻撃
		JumpAttack, // ジャンプ攻撃
	};

	// ヒットストップ
	struct HitStop {

		bool isStarted = false;
		float startProgress;    // 発生させる攻撃進捗
		DelayedHitstop hitStop; // ヒットストップ
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 座標移動のキーフレーム
	std::unique_ptr<KeyframeObject3D> moveKeyframeObject_;
	// ジャンプ移動のキーフレーム
	std::unique_ptr<KeyframeObject3D> jumpKeyframeObject_;
	// 空の親トランスフォーム、敵が範囲内にいないときに参照する親
	Transform3D* moveFrontTransform_;
	// タグ
	ObjectTag* moveFrontTag_;
	// 敵のトランスフォームの値を補正する用のトランスフォーム
	Transform3D* fixedEnemyTransform_;
	// タグ
	ObjectTag* fixedEnemyTag_;

	// 回転の軸
	Vector3 rotationAxis_;
	// 移動の前座標
	Vector3 preMovePos_;

	// 目標への回転
	Quaternion targetRotation_;

	// ジャンプ攻撃アニメーションへの遷移時間
	float nextJumpAnimDuration_;

	// 範囲内にいるかどうか
	bool isInRange_;

	// 攻撃ヒットストップ
	HitStop moveAttackHitstop_;
	HitStop jumpAttackHitstop_;

	// 残像表現エフェクト
	std::unique_ptr<PlayerAfterImageEffect> afterImageEffect_;

	//--------- functions ----------------------------------------------------

	// 状態毎の更新
	void UpdateMoveAttack(Player& player);
	void UpdateJumpAttack(Player& player);

	// 範囲内チェックを行って補間目標を設定する
	void SetTargetByRange(KeyframeObject3D& keyObject, const std::string& cameraKeyName);
};