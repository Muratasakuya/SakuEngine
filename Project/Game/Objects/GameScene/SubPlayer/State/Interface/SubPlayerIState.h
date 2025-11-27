#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

// front
class SubPlayer;

//============================================================================
//	SubPlayerIState class
//	サブプレイヤーの状態インターフェース
//============================================================================
class SubPlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SubPlayerIState() = default;
	virtual ~SubPlayerIState() = default;

	// 状態遷移時
	virtual void Enter() = 0;

	// 更新処理
	virtual void Update() = 0;
	virtual void UpdateAlways() {}

	// 状態終了時
	virtual void Exit() = 0;

	// エディター
	virtual void ImGui() = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	// 各パーツ
	void SetBody(GameObject3D* body) { body_ = body; }
	void SetRightHand(GameObject3D* rightHand) { rightHand_ = rightHand; }
	void SetLeftHand(GameObject3D* leftHand) { leftHand_ = leftHand; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 状態遷移先相手の各パーツ
	GameObject3D* body_;      // 体
	GameObject3D* rightHand_; // 右手
	GameObject3D* leftHand_;  // 左手

	//--------- functions ----------------------------------------------------

};