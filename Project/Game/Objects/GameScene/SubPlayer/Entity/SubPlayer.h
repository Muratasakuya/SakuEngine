#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Game/Objects/GameScene/SubPlayer/State/SubPlayerStateController.h>

//============================================================================
//	SubPlayer class
//	サブプレイヤー
//============================================================================
class SubPlayer :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SubPlayer() :IGameEditor("SubPlayer") {}
	~SubPlayer() = default;

	void Init();

	void Update();

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// パーツ
	std::unique_ptr<GameObject3D> body_; // 体、親となる
	std::unique_ptr<GameObject3D> rightHand_; // 右手
	std::unique_ptr<GameObject3D> leftHand_;  // 左手

	// 状態管理
	std::unique_ptr<SubPlayerStateController> stateController_;

	// パーツ別の初期トランスフォーム
	Transform3D initBodyTransform_;
	Transform3D initRightHandTransform_;
	Transform3D initLeftHandTransform_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitParts();
	void InitState();

	// パーツにトランスフォームをセットする
	void SetPartsTransform(GameObject3D* parts, const Transform3D& transform);
};