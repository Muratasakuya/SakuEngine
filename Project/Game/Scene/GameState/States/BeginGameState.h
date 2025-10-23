#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/Interface/IGameSceneState.h>

//============================================================================
//	BeginGameState class
//	ゲーム開始時の状態
//============================================================================
class BeginGameState :
	public IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BeginGameState(GameContext* context) :IGameSceneState(context){}
	~BeginGameState() = default;

	void Init(SceneView* sceneView) override;

	void Update(SceneManager* sceneManager) override;
	void NonActiveUpdate(SceneManager* sceneManager) override;

	void ImGui() override;

	// 遷移終了時
	void Exit() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 遷移後のプレイヤーの座標
	Vector3 startPlayerPos_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};