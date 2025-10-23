#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/Interface/IGameSceneState.h>

//============================================================================
//	ResultGameState class
//	リザルト画面の状態
//============================================================================
class ResultGameState :
	public IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ResultGameState(GameContext* context) :IGameSceneState(context) {}
	~ResultGameState() = default;

	void Init(SceneView* sceneView) override;

	void Update(SceneManager* sceneManager) override;
	void NonActiveUpdate(SceneManager* sceneManager) override;

	// 遷移開始時
	void Enter() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------



	//--------- functions ----------------------------------------------------

};