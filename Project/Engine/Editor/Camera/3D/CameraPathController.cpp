#include "CameraPathController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/SceneView.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>

//============================================================================
//	CameraPathController classMethods
//============================================================================

CameraPathController::CameraPathController(SceneView* sceneView) {

	sceneView_ = nullptr;
	sceneView_ = sceneView;
}

void CameraPathController::Update(const PlaybackState& state, CameraPathData& data) {

	// 更新対象のカメラ
	BaseCamera* camera = sceneView_->GetCamera();
	if (!state.isActive || data.keyframes.empty()) {
		camera->SetIsUpdateEditor(false);
		return;
	}

	// エディターで更新中
	camera->SetIsUpdateEditor(true);
	// 補間値
	Vector3 translation{};
	Quaternion rotation{};
	float fovY{};

	switch (state.mode) {
	case PreviewMode::Keyframe: {

		int index = std::clamp(state.selectedKeyIndex, 0, static_cast<int>(data.keyframes.size()) - 1);
		EvaluateAtKey(data, index, translation, rotation, fovY);
		break;
	}
	case PreviewMode::Manual: {

		float rawT = std::clamp(state.time, 0.0f, 1.0f);
		float easedT = EasedValue(data.timer.easeingType_, rawT);

		// 平均化された値を使用するか
		float t = 0.0f;
		if (data.useAveraging && !data.averagedT.empty()) {

			t = LerpKeyframe::GetReparameterizedT(easedT, data.averagedT);
		} else {

			t = easedT;
		}
		Evaluate(data, t, translation, rotation, fovY);
		break;
	}
	case PreviewMode::Play: {

		// 時間の更新
		data.timer.Update();

		// 終了後タイマーをリセットするか固定する
		if (data.timer.IsReached()) {
			if (state.isLoop) {

				data.timer.Reset();
			} else {

				data.timer.current_ = data.timer.target_;
			}
		}
		float t = 0.0f;
		if (data.useAveraging && !data.averagedT.empty()) {

			t = LerpKeyframe::GetReparameterizedT(data.timer.easedT_, data.averagedT);
		} else {

			t = data.timer.easedT_;
		}
		Evaluate(data, t, translation, rotation, fovY);
		break;
	}
	}
	// カメラに適応
	ApplyToCamera(*camera, translation, rotation, fovY, false);
}

void CameraPathController::Evaluate(const CameraPathData& data, float t,
	Vector3& outTranslation, Quaternion& outRotation, float& outFovY) const {

	// 座標の補間
	std::vector<Vector3> points;
	points.reserve(data.keyframes.size());
	for (auto& keyframe : data.keyframes) {

		points.emplace_back(keyframe.demoObject->GetTransform().GetWorldPos());
	}

	if (LerpKeyframe::GetValue<Vector3>(points, t, data.lerpType).Length() <= 28.0f) {

		int a = 0;
		a++;
	}
	outTranslation = LerpKeyframe::GetValue<Vector3>(points, t, data.lerpType);

	// 画角の補間
	std::vector<float> fovs;
	fovs.reserve(data.keyframes.size());
	for (auto& keyframe : data.keyframes) {

		fovs.emplace_back(keyframe.fovY);
	}
	outFovY = LerpKeyframe::GetValue<float>(fovs, t, LerpKeyframe::Type::None);

	// 回転の補間
	// キーフレームが1個なら最初の値を返す
	if (data.keyframes.size() < 2) {
		outRotation = data.keyframes.front().demoObject->GetRotation();
		return;
	}
	const int n = static_cast<int>(data.keyframes.size());
	const float division = 1.0f / (n - 1);
	int index = static_cast<int>(std::floor(t / division));
	index = std::clamp(index, 0, n - 2);
	float lt = (t - index * division) / division;

	const Quaternion& rotation = data.keyframes[index].demoObject->GetRotation();
	const Quaternion& nextRotation = data.keyframes[index + 1].demoObject->GetRotation();
	outRotation = Quaternion::Slerp(rotation, nextRotation, lt);
}

void CameraPathController::EvaluateAtKey(const CameraPathData& data, int keyIndex,
	Vector3& outTranslation, Quaternion& outRotation, float& outFovY) const {

	const auto& keyframe = data.keyframes[keyIndex];
	outTranslation = keyframe.demoObject->GetTransform().GetWorldPos();
	outRotation = keyframe.demoObject->GetRotation();
	outFovY = keyframe.fovY;
}

void CameraPathController::ApplyToCamera(BaseCamera& camera, const Vector3& translation,
	const Quaternion& rotation, float fovY, bool isUseGame) const {

	if (!isUseGame) {

		LineRenderer::GetInstance()->DrawOBB(translation,
			Vector3::AnyInit(2.4f), rotation, Color::Cyan());
	}

	camera.SetTranslation(translation);
	camera.SetRotation(Quaternion::Normalize(rotation));
	camera.SetFovY(fovY);
	// quaternionで更新
	camera.UpdateView(BaseCamera::UpdateMode::Quaternion);
}