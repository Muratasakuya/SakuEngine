#include "GameScene.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Scene/Manager/SceneManager.h>

// scene
#include <Game/Scene/GameState/States/StartGameState.h>
#include <Game/Scene/GameState/States/BeginGameState.h>
#include <Game/Scene/GameState/States/PlayGameState.h>
#include <Game/Scene/GameState/States/EndGameState.h>
#include <Game/Scene/GameState/States/ResultGameState.h>
#include <Game/Scene/GameState/States/PauseState.h>

//============================================================================
//	GameScene classMethods
//============================================================================

void GameScene::InitStates() {

	// scene
	context_.camera = cameraManager_.get();
	context_.light = gameLight_.get();
	context_.fieldBoundary = fieldBoundary_.get();
	// object
	context_.player = player_.get();
	context_.boss = bossEnemy_.get();
	context_.result = result_.get();
	// sprite
	context_.fadeSprite = fadeSprite_.get();
	// editor
	context_.level = levelEditor_.get();

	// シーン状態クラスの初期化
	states_[static_cast<uint32_t>(GameSceneState::Start)] = std::make_unique<StartGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::Start)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::BeginGame)] = std::make_unique<BeginGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::BeginGame)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::PlayGame)] = std::make_unique<PlayGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::PlayGame)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::EndGame)] = std::make_unique<EndGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::EndGame)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::Result)] = std::make_unique<ResultGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::Result)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::Pause)] = std::make_unique<PauseState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::Pause)]->Init(sceneView_);

	// 最初の状態を設定
	currentState_ = GameSceneState::Start;
}

void GameScene::Init() {

	//========================================================================
	//	sceneObjects
	//========================================================================

	cameraManager_ = std::make_unique<CameraManager>();
	gameLight_ = std::make_unique<PunctualLight>();
	fieldBoundary_ = std::make_unique<FieldBoundary>();

	//========================================================================
	//	editor
	//========================================================================

	levelEditor_ = std::make_unique<LevelEditor>();

	//========================================================================
	//	frontObjects
	//========================================================================

	player_ = std::make_unique<Player>();
	bossEnemy_ = std::make_unique<BossEnemy>();
	result_ = std::make_unique<GameResultDisplay>();

	fadeSprite_ = std::make_unique<FadeSprite>();

	//========================================================================
	//	state
	//========================================================================

	// シーン状態の初期化
	InitStates();

	// 遷移の設定
	fadeTransition_ = std::make_unique<FadeTransition>();
	fadeTransition_->Init();
}

void GameScene::Update() {

	// 状態に応じて更新
	uint32_t stateIndex = static_cast<uint32_t>(currentState_);
	switch (currentState_) {
		//========================================================================
		//	ゲーム開始時の処理
		//========================================================================
	case GameSceneState::Start: {

		states_[stateIndex]->Update(nullptr);

		// ゲーム開始演出状態にする
		if (states_[stateIndex]->IsRequestNext()) {

			RequestNextState(GameSceneState::BeginGame);
		}
		break;
	}
		//========================================================================
		//	ゲーム開始演出の処理
		//========================================================================
	case GameSceneState::BeginGame: {

		states_[stateIndex]->Update(nullptr);

		// ゲーム開始
		if (states_[stateIndex]->IsRequestNext()) {

			RequestNextState(GameSceneState::PlayGame);
		}
		break;
	}
		//========================================================================
		//	ゲームプレイ中の処理
		//========================================================================
	case GameSceneState::PlayGame: {

		states_[stateIndex]->Update(nullptr);

		// ゲーム終了
		if (states_[stateIndex]->IsRequestNext()) {

			RequestNextState(GameSceneState::EndGame);
		}
		break;
	}
		//========================================================================
		//	ゲーム終了時の処理
		//========================================================================
	case GameSceneState::EndGame: {

		states_[stateIndex]->Update(nullptr);

		// 終了演出後リザルト画面に遷移させる
		if (states_[stateIndex]->IsRequestNext()) {

			RequestNextState(GameSceneState::Result);
			// プレイヤーと敵を消す
			player_.reset();
			bossEnemy_.reset();
		}
		break;
	}
		//========================================================================
		//	リザルト画面の処理
		//========================================================================
	case GameSceneState::Result: {

		states_[stateIndex]->Update(nullptr);

		// 入力に応じて遷移先を決定する
		if (states_[stateIndex]->IsRequestNext() && fadeTransition_) {

			ResultGameState* state = static_cast<ResultGameState*>(states_[stateIndex].get());
			if (state->GetResultSelect() == ResultSelect::Retry) {

				sceneManager_->SetNextScene(Scene::Game, std::move(fadeTransition_));
			} else if (state->GetResultSelect() == ResultSelect::Title) {

				sceneManager_->SetNextScene(Scene::Title, std::move(fadeTransition_));
			}
		}
		break;
	}
		//========================================================================
		//	ポーズ中の処理
		//========================================================================
	case GameSceneState::Pause: {

		states_[stateIndex]->Update(nullptr);
		break;
	}
	}
}

void GameScene::RequestNextState(GameSceneState next) {

	// 現在の状態を終了させる
	uint32_t stateIndex = static_cast<uint32_t>(currentState_);
	states_[stateIndex]->Exit();

	// 遷移
	currentState_ = next;
	uint32_t nextIndex = static_cast<uint32_t>(next);

	// 次の状態を設定
	states_[nextIndex]->Enter();
	states_[stateIndex]->ClearRequestNext();
}

void GameScene::ImGui() {

	ImGui::SeparatorText(EnumAdapter<GameSceneState>::ToString(currentState_));

	uint32_t stateIndex = static_cast<uint32_t>(currentState_);
	states_[stateIndex]->ImGui();
}