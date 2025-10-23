#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>

// scene
#include <Game/Scene/GameState/GameSceneState.h>
// cameras
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Camera/BeginGame/BeginGameCamera.h>
#include <Game/Camera/EndGame/EndGameCamera.h>
#include <Game/Camera/ResultCamera/ClearResultCamera.h>
// front
class Player;
class BossEnemy;
class SceneView;

//============================================================================
//	CameraManager class
//	全てのカメラを管理する
//============================================================================
class CameraManager :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CameraManager() :IGameEditor("CameraManager") {};
	~CameraManager() = default;

	// 全てのカメラを初期化
	void Init(SceneView* sceneView);

	// シーンの状態に応じてカメラを更新
	void Update(GameSceneState sceneState);

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 追従先の設定
	void SetTarget(const Player* player, const BossEnemy* bossEnemy);

	FollowCamera* GetFollowCamera() const { return followCamera_.get(); }
	BeginGameCamera* GetBeginGameCamera() const { return beginGameCamera_.get(); }
	EndGameCamera* GetEndGameCamera() const { return endGameCamera_.get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	SceneView* sceneView_;
	const Player* player_;
	GameSceneState preSceneState_;

	// 追従カメラ
	std::unique_ptr<FollowCamera> followCamera_;
	// ゲーム開始時のカメラ
	std::unique_ptr<BeginGameCamera> beginGameCamera_;
	// ゲーム終了時のカメラ
	std::unique_ptr<EndGameCamera> endGameCamera_;
	// リザルト画面のカメラ
	std::unique_ptr<ClearResultCamera> resultCamera_;

	//--------- functions ----------------------------------------------------

	// 現在の状態をチェックしてカメラを切り替える
	void CheckSceneState(GameSceneState sceneState);
};