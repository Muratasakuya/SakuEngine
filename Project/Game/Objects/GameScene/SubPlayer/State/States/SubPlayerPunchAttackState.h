#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/KeyframeObject3D.h>
#include <Game/Objects/GameScene/SubPlayer/State/Interface/SubPlayerIState.h>

//============================================================================
//	SubPlayerPunchAttackState class
//============================================================================
class SubPlayerPunchAttackState :
	public SubPlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SubPlayerPunchAttackState();
	~SubPlayerPunchAttackState() = default;

	// 状態遷移時
	void Enter() override;

	// 更新処理
	void Update() override;
	void UpdateAlways() override;

	// 状態終了時
	void Exit() override;

	// エディター
	void ImGui() override;

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

		Approach, // 近づく
		Attack,   // 攻撃
		Leave,    // 離れる
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 各パーツのキーフレーム移動
	// 体
	std::unique_ptr<KeyframeObject3D> bodyApproachKeyframeObject_;
	std::unique_ptr<KeyframeObject3D> bodyLeaveKeyframeObject_;
	// 右手
	std::unique_ptr<KeyframeObject3D> rightHandApproachKeyframeObject_;
	std::unique_ptr<KeyframeObject3D> rightHandLeaveKeyframeObject_;
	// 左手
	std::unique_ptr<KeyframeObject3D> leftHandApproachKeyframeObject_;
	std::unique_ptr<KeyframeObject3D> leftHandLeaveKeyframeObject_;

	//--------- functions ----------------------------------------------------

	// 状態別更新処理
	void UpdateApproach();
	void UpdateAttack();
	void UpdateLeave();

	// 常にキーフレームオブジェクトを更新
	void UpdateKeyframeObjects();
	// キーオブジェクトのリセット
	void ResetKeyframeObjects();

	// 補間の更新とトランスフォームの更新
	void UpdateKeyAndApply(KeyframeObject3D& bodyKeyframe,
		KeyframeObject3D& rightHandKeyframe, KeyframeObject3D& leftHandKeyframe);
	// 全ての補間処理が終了したかのチェック
	bool IsAllKeyframeEnd(KeyframeObject3D& bodyKeyframe,
		KeyframeObject3D& rightHandKeyframe, KeyframeObject3D& leftHandKeyframe);
};