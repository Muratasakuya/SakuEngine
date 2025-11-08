#include "StartGameState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Scene/SceneView.h>

//============================================================================
//	StartGameState classMethods
//============================================================================

void StartGameState::Init(SceneView* sceneView) {

	//========================================================================
	//	postProcess
	//========================================================================

	PostProcessSystem* postProcessSystem = PostProcessSystem::GetInstance();
	postProcessSystem->AddProcess(PostProcessType::RadialBlur);
	postProcessSystem->AddProcess(PostProcessType::PlayerAfterImage);
	postProcessSystem->AddProcess(PostProcessType::DepthBasedOutline);
	postProcessSystem->AddProcess(PostProcessType::Grayscale);
	postProcessSystem->AddProcess(PostProcessType::Bloom);

	//========================================================================
	//	sceneObject
	//===================================================,.=====================

	// カメラ
	context_->camera->Init(sceneView);

	// ライトの初期設定
	context_->light->Init();
	context_->light->directional.direction.x = 0.558f;
	context_->light->directional.direction.y = -0.476f;
	context_->light->directional.direction.z = -0.68f;
	context_->light->directional.color = Color::White();
	sceneView->SetLight(context_->light);

	// 衝突
	context_->fieldBoundary->Init();

	//========================================================================
	//	frontObjects
	//========================================================================

	// プレイヤー
	context_->player->Init("player", "player", "Player", "player_idle");

	// ボス
	context_->boss->Init("bossEnemy", "bossEnemy", "Enemy", "bossEnemy_idle");

	// プレイヤー、カメラをセット
	context_->boss->SetPlayer(context_->player);
	context_->boss->SetFollowCamera(context_->camera->GetFollowCamera());
	// ボス、カメラをセット
	context_->player->SetBossEnemy(context_->boss);
	context_->player->SetFollowCamera(context_->camera->GetFollowCamera());

	// 衝突応答にプレイヤー、ボスをセット
	context_->fieldBoundary->SetPushBackTarget(context_->player, context_->boss);

	// リザルト画面
	context_->result->Init();

	// 追従先を設定する
	context_->camera->SetTarget(context_->player, context_->boss);

	//========================================================================
	//	sprites
	//========================================================================

	// プレイヤーの移動する方向
	playerToBossText_ = std::make_unique<GameObject2D>();
	playerToBossText_->Init("playerToBoss", "playerToBoss", "Scene");

	context_->fadeSprite->Init("white", "fadeSprite", "Scene");

	//========================================================================
	//	sceneEvent
	//========================================================================

	nextStateEvent_ = std::make_unique<Collider>();
	// 衝突タイプ設定
	CollisionBody* body = nextStateEvent_->AddCollider(CollisionShape::AABB().Default(), true);
	// タイプ設定
	body->SetType(ColliderType::Type_Event);
	body->SetTargetType(ColliderType::Type_Player);

	// json適応
	ApplyJson();
}

void StartGameState::Update([[maybe_unused]] SceneManager* sceneManager) {

	const GameSceneState currentState = GameSceneState::Start;

	//========================================================================
	//	object
	//========================================================================

	context_->player->Update();
	context_->boss->Update(currentState);

	//========================================================================
	//	scene
	//========================================================================

	context_->camera->Update(currentState);

	//========================================================================
	//	sceneEvent
	//========================================================================

	Transform3D transform{};
	transform.scale = Vector3::AnyInit(1.0f);
	nextStateEvent_->UpdateAllBodies(transform);

	// イベント範囲内に入ったら次の状態に遷移させる
	if (nextStateEvent_->IsHitTrigger()) {

		requestNext_ = true;
	}
}

void StartGameState::NonActiveUpdate([[maybe_unused]] SceneManager* sceneManager) {
}

void StartGameState::Exit() {

	// 方向指示を消す
	playerToBossText_.reset();
}

void StartGameState::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	nextStateEvent_->ImGui(192.0f);
}

void StartGameState::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Scene/State/startGameState.json", data)) {
		return;
	}

	nextStateEvent_->ApplyBodyOffset(data["NextStateEvent"]);

	playerToBossText_->SetSize(Vector2::FromJson(data.value("playerToBossText_Size", Json())));
	playerToBossText_->SetTranslation(Vector2::FromJson(data.value("playerToBossText_Pos", Json())));
}

void StartGameState::SaveJson() {

	Json data;

	nextStateEvent_->SaveBodyOffset(data["NextStateEvent"]);

	data["playerToBossText_Size"] = playerToBossText_->GetSize().ToJson();
	data["playerToBossText_Pos"] = playerToBossText_->GetTranslation().ToJson();

	JsonAdapter::Save("Scene/State/startGameState.json", data);
}