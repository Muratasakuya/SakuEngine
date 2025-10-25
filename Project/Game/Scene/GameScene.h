#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Methods/IScene.h>

// scene
#include <Game/Scene/GameState/Interface/IGameSceneState.h>
#include <Game/Objects/SceneTransition/FadeTransition.h>

// effect
#include <Engine/Effect/User/GameEffect.h>

//============================================================================
//	GameScene class
//	ゲームシーン
//============================================================================
class GameScene :
	public IScene,
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameScene() :IGameEditor("GameScene") {};
	~GameScene() = default;

	void Init() override;

	void Update() override;
	void EndFrame() override;

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// sceneState
	GameSceneState currentState_;
	GameContext context_;
	std::array<std::unique_ptr<IGameSceneState>, static_cast<uint32_t>(GameSceneState::Count)> states_;

	// scene
	std::unique_ptr<CameraManager> cameraManager_;
	std::unique_ptr<PunctualLight> gameLight_;
	std::unique_ptr<FadeTransition> fadeTransition_;
	// collision
	std::unique_ptr<FieldBoundary> fieldBoundary_;

	// objects
	std::unique_ptr<Player> player_;
	std::unique_ptr<BossEnemy> bossEnemy_;
	std::unique_ptr<GameResultDisplay> result_;

	// sprites
	std::unique_ptr<FadeSprite> fadeSprite_;

	// editor
	std::unique_ptr<LevelEditor> levelEditor_;

	//--------- functions ----------------------------------------------------

	// init
	void InitStates();

	// update
	void UpdateAlways();

	// helper
	void RequestNextState(GameSceneState next);
};