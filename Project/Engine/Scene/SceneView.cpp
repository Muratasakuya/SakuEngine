#include "SceneView.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	SceneView classMethods
//============================================================================

void SceneView::InitCamera() {

	// 2D
	camera2D_ = std::make_unique<Camera2D>();
	camera2D_->Init();

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Init();

	// カメラを追加
	AddSceneCamera("DebugCamera", debugCamera_.get());

	// シーン視点はデフォルトでデバッグカメラを使用
	activeSceneCamera_ = std::nullopt;
	activeSceneCamera_ = debugCamera_.get();
#endif
}

void SceneView::Init() {

	// scene情報初期化
	InitCamera();

	dither_.Init();
}

void SceneView::Update() {

	// scene更新
	UpdateCamera();
	UpdateLight();
}

void SceneView::UpdateCamera() {

	// 3Dカメラの更新は各sceneクラスで行う
	if (activeGameCamera3D_.has_value()) {

		activeGameCamera3D_.value()->RenderFrustum();
	}

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	debugCamera_->Update();

	if (activeSceneCamera_.has_value()) {
		if (activeSceneCamera_.value()->IsUpdateDebugView()) {

			activeSceneCamera_.value()->Update();
		}
	}
#endif
}

void SceneView::UpdateLight() {

	// ライトの更新はエンジン側で行う
	punctualLight_.value()->Update();

	// pointLight、spotLightのデバッグ表示
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	// point
	//DisplayPointLight();
	// spot
	//DisplaySpotLight();
#endif
}

void SceneView::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	if (ImGui::BeginTabBar("SceneTab")) {

		if (ImGui::BeginTabItem("Camera")) {

			EditCamera();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Light")) {

			EditLight();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Dither")) {

			dither_.ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
}

void SceneView::EditCamera() {

	if (ImGui::CollapsingHeader("SceneCamera")) {

		// カメラの選択・表示
		std::vector<BaseCamera*>  cameraPtrs;
		std::vector<std::string>  cameraLabels;
		cameraPtrs.reserve(sceneCameras_.size());
		cameraLabels.reserve(sceneCameras_.size());
		for (auto& [groupName, cameras] : sceneCameras_) {
			for (size_t i = 0; i < cameras.size(); ++i) {

				cameraPtrs.push_back(cameras[i]);
				std::string label = groupName;
				if (cameras.size() > 1) {
					label += " #" + std::to_string(i);
				}
				cameraLabels.push_back(label);
			}
		}

		// カメラが設定されていなければ表示しない
		if (cameraPtrs.empty()) {

			ImGui::TextDisabled("scene cameras have been registered");
		} else {

			// 選択されていないカメラを追従させる
			if (!activeSceneCamera_.has_value()) {

				activeSceneCameraIndex_ = 0;
				activeSceneCamera_ = cameraPtrs[0];
			} else {
				for (int i = 0; i < (int)cameraPtrs.size(); ++i) {
					if (cameraPtrs[i] == activeSceneCamera_.value()) {

						activeSceneCameraIndex_ = i;
						break;
					}
				}
			}

			// カメラの選択
			std::vector<const char*> itemsCamera{};
			itemsCamera.reserve(cameraLabels.size());
			for (auto& s : cameraLabels) { itemsCamera.push_back(s.c_str()); }

			if (ImGui::Combo("ActiveSceneCamera", &activeSceneCameraIndex_,
				itemsCamera.data(), static_cast<int>(itemsCamera.size()))) {

				activeSceneCamera_ = cameraPtrs[activeSceneCameraIndex_];
			}
			ImGui::Separator();
			if (activeSceneCamera_) {

				activeSceneCamera_.value()->ImGui();
			}
		}
	}

	if (activeGameCamera3D_ && ImGui::CollapsingHeader("GameCamera")) {

		activeGameCamera3D_.value()->EditFrustum();
		activeGameCamera3D_.value()->ImGui();
	}
	if (ImGui::CollapsingHeader("DebugCamera")) {

		debugCamera_->ImGui();
	}
}

void SceneView::EditLight() {

	punctualLight_.value()->ImGui();
}

void SceneView::SetGameCamera(BaseCamera* gameCamera) {

	// カメラのセット
	activeGameCamera3D_ = std::nullopt;
	activeGameCamera3D_ = gameCamera;
}

void SceneView::AddSceneCamera(const std::string& name, BaseCamera* sceneCamera) {

	// すでに追加済みの場合追加できない
	if (Algorithm::Find(sceneCameras_, name)) {
		return;
	}

	sceneCameras_[name].emplace_back(sceneCamera);
}

void SceneView::SetLight(PunctualLight* gameLight) {

	// ライトのセット
	punctualLight_ = std::nullopt;
	punctualLight_ = gameLight;
}

void SceneView::DisplayPointLight() {

	LineRenderer* lineRenderer = LineRenderer::GetInstance();
	const auto& light = punctualLight_.value();

	// 規定値
	const int sphereDivision = 8;
	const float sphereRadius = 0.12f;
	const Color sphereColor = Color(
		light->point.color.r,
		light->point.color.g,
		light->point.color.b,
		0.8f); // 薄く表示する

	// 球で描画
	lineRenderer->DrawSphere(
		sphereDivision,
		sphereRadius,
		light->point.pos,
		sphereColor, LineType::DepthIgnore);
}

void SceneView::DisplaySpotLight() {

	LineRenderer* lineRenderer = LineRenderer::GetInstance();
	const auto& light = punctualLight_.value();

	const float coneLength = 2.0f;
	const Color coneColor = Color(
		light->spot.color.r,
		light->spot.color.g,
		light->spot.color.b,
		0.8f);

	const Vector3 pos = light->spot.pos;
	const Vector3 dir = Vector3::Normalize(light->spot.direction);

	const int coneDivision = 4;
	const float radius = coneLength * std::tanf(light->spot.cosAngle * 0.5f);
	Vector3 baseCenter = pos + dir * coneLength;

	for (uint32_t index = 0; index < coneDivision; ++index) {

		float theta1 = static_cast<float>(index) / coneDivision * std::numbers::pi_v<float>*2.0f;
		float theta2 = static_cast<float>(index + 1) / coneDivision * std::numbers::pi_v<float>*2.0f;

		Vector3 p1 = baseCenter + Vector3(cosf(theta1), 0, sinf(theta1)) * radius;
		Vector3 p2 = baseCenter + Vector3(cosf(theta2), 0, sinf(theta2)) * radius;

		// coneの形状で描画
		lineRenderer->DrawLine3D(p1, p2, coneColor, LineType::DepthIgnore);
		lineRenderer->DrawLine3D(pos, p1, coneColor, LineType::DepthIgnore);
	}
}