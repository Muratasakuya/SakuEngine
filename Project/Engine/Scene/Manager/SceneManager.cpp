#include "SceneManager.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Editor/Camera/3D/Camera3DEditor.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/InstancedMeshSystem.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	SceneManager classMethods
//============================================================================

SceneManager::SceneManager(Scene scene, Asset* asset, SceneView* sceneView) :IGameEditor("SceneManager") {

	sceneView_ = nullptr;
	sceneView_ = sceneView;

	asset_ = nullptr;
	asset_ = asset;

	factory_ = std::make_unique<SceneFactory>();

	sceneTransition_ = std::make_unique<SceneTransition>();
	sceneTransition_->Init();

	// 最初のシーンファイルを読みこみ
	asset->LoadSceneAsync(scene, AssetLoadType::Synch);
	// メッシュの構築
	const auto& system = ObjectManager::GetInstance()->GetSystem<InstancedMeshSystem>();
	system->BuildForSceneSynch(scene);
	// 最初のシーン以外を非同期で読み込む
	for (uint32_t index = 0; index < EnumAdapter<Scene>::GetEnumCount(); ++index) {

		// 同じシーンは処理しない
		if (index == static_cast<uint32_t>(scene)) {
			continue;
		}
		asset->LoadSceneAsync(EnumAdapter<Scene>::GetValue(index), AssetLoadType::Async);
	}

	// 最初のシーンを読み込んで初期化
	LoadScene(scene);
	currentScene_->Init();
}

void SceneManager::Update() {

	currentScene_->Update();
	sceneTransition_->Update();

	// cameraEditor更新
	Camera3DEditor::GetInstance()->Update();
}

void SceneManager::BeginFrame() {

	currentScene_->BeginFrame();
}

void SceneManager::EndFrame() {

	currentScene_->EndFrame();
}

void SceneManager::SwitchScene() {

	if (sceneTransition_->IsBeginTransitionFinished()) {

		isSceneSwitching_ = true;
		sceneTransition_->SetResetBeginTransition();
		queuedMeshBuild_ = false;
		allowMeshRendering_ = false;
	}

	if (isSceneSwitching_) {
		// アセットファイルの読み込みが終了したかどうか
		if (asset_->IsScenePreloadFinished(nextSceneType_)) {

			const auto& system = ObjectManager::GetInstance()->GetSystem<InstancedMeshSystem>();
			// シーンに必要なメッシュ生成を依頼する
			if (!queuedMeshBuild_) {

				system->RequestBuildForScene(nextSceneType_);
				queuedMeshBuild_ = true;
			}
			// 終了したら遷移を終了させる
			if (1.0f <= system->GetBuildProgressForScene(nextSceneType_)) {

				sceneTransition_->NotifyAssetsFinished();
			}
		}
		// 遷移終了後
		if (!needInitNextScene_ && sceneTransition_->ConsumeLoadEndFinished()) {

			LoadScene(nextSceneType_);
			needInitNextScene_ = true;
			allowMeshRendering_ = true;
		}
	}
}

void SceneManager::InitNextScene() {

	currentScene_->Init();
	isSceneSwitching_ = false;
}

void SceneManager::SetNextScene(Scene scene, std::unique_ptr<ITransition> transition) {

	nextSceneType_ = scene;
	sceneTransition_->SetTransition(std::move(transition));
}

void SceneManager::ImGui() {

	ImGui::Text("CurrentScene : %s", EnumAdapter<Scene>::ToString(currentSceneType_));

	// 選択
	static Scene selected = currentSceneType_;
	if (EnumAdapter<Scene>::Combo("Select Scene", &selected)) {
	}

	if (ImGui::Button("Apply") && selected != currentSceneType_ && !isSceneSwitching_) {

		isSceneSwitching_ = true;
		nextSceneType_ = selected;
	}

	ImGui::SeparatorText("Scene Transition");

	sceneTransition_->ImGui();
}

bool SceneManager::ConsumeNeedInitNextScene() {

	bool need = needInitNextScene_;
	needInitNextScene_ = false;
	return need;
}

void SceneManager::LoadScene(Scene scene) {

	currentScene_.reset();
	// 次のSceneを作成
	currentSceneType_ = scene;
	currentScene_ = factory_->Create(scene);
	currentScene_->SetPtr(sceneView_, this);

	// imgui選択をリセット
	ImGuiObjectEditor::GetInstance()->Reset();
	// すべてのオブジェクトを破棄
	ObjectManager::GetInstance()->DestroyAll();
}