#include "CameraPathData.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/TagSystem.h>

//============================================================================
//	CameraPathData classMethods
//============================================================================

void CameraPathData::KeyframeParam::Init(bool isUseGame) {

	// キーフレーム初期値
	fovY = 0.54f;

	// デモ用オブジェクトを作成
	demoObject = std::make_unique<GameObject3D>();
	demoObject->Init("demoCamera", "demoCamera", "Editor");

	Json data;
	if (!JsonAdapter::LoadCheck(demoCameraJsonPath, data)) {
		return;
	}
	// 見た目を設定
	demoObject->ApplyTransform(data);
	demoObject->ApplyMaterial(data);
	// ゲームで使用する場合描画しない
	if (isUseGame) {

		demoObject->SetMeshRenderView(MeshRenderView::None);
	} else {

		demoObject->SetMeshRenderView(MeshRenderView::Scene);
	}
}

void CameraPathData::KeyframeParam::FromJson(const Json& data) {

	fovY = data.value("fovY", 54.0f);

	translation = Vector3::FromJson(data.value("translation", Json()));
	rotation = Quaternion::FromJson(data.value("rotation", Json()));

	demoObject->SetTranslation(translation);
	demoObject->SetRotation(rotation);
}

void CameraPathData::KeyframeParam::ToJson(Json& data) {

	data["fovY"] = fovY;

	// demoObjectから渡す
	data["translation"] = demoObject->GetTranslation().ToJson();
	data["rotation"] = demoObject->GetRotation().ToJson();
}

void CameraPathData::ApplyJson(const std::string& fileName, bool isUseGame) {

	Json data;
	// 読み込めなければ処理しない
	if (!JsonAdapter::LoadCheck(fileName, data)) {
		return;
	}

	// ゲームで使用する場合は線描画をしない
	if (isUseGame) {

		isDrawLine3D = false;
	}

	objectName = data.value("objectName", "objectName");
	overallName = data.value("overallName", "overallName");

	followTarget = data.value("followTarget", false);
	followRotation = data.value("followRotation", true);
	targetName = data.value("targetName", "");
	divisionCount = data.value("divisionCount", 64);
	useAveraging = data.value("useAveraging", false);

	lerpType = EnumAdapter<LerpKeyframe::Type>::FromString(data.value("lerpType", "None")).value();
	timer.FromJson(data["Timer"]);

	// 追従先の設定が必要なら設定
	target = nullptr;
	if (followTarget && !targetName.empty()) {

		ObjectManager* objectManager = ObjectManager::GetInstance();
		TagSystem* tagSystem = objectManager->GetSystem<TagSystem>();
		for (const auto& [id, tagPtr] : tagSystem->Tags()) {
			if (tagPtr && tagPtr->name == targetName) {

				target = objectManager->GetData<Transform3D>(id);
				break;
			}
		}
	}

	// キーフレームを構築
	keyframes.clear();
	const Json& kfs = data["Keyframes"];

	std::vector<uint32_t> indices;
	indices.reserve(kfs.size());
	for (auto it = kfs.begin(); it != kfs.end(); ++it) {

		indices.push_back(std::stoi(it.key()));
	}
	// indexを0から順になるようにソート
	std::sort(indices.begin(), indices.end());

	// キーフレーム設定
	for (uint32_t index : indices) {

		const std::string key = std::to_string(index);
		KeyframeParam keyframe;
		keyframe.Init(isUseGame);
		keyframe.FromJson(kfs[key]);
		keyframes.emplace_back(std::move(keyframe));
	}

	// 平均化処理
	averagedT.clear();
	if (useAveraging) {

		std::vector<Vector3> points;
		points.reserve(keyframes.size());
		for (const auto& keyframe : keyframes) {

			points.emplace_back(keyframe.translation);
		}
		averagedT = LerpKeyframe::AveragingPoints<Vector3>(points, divisionCount, lerpType);
	}
}

void CameraPathData::SaveJson(const std::string& fileName) {

	Json data;

	data["objectName"] = objectName;
	data["overallName"] = overallName;

	data["followTarget"] = followTarget;
	data["followRotation"] = followRotation;
	data["targetName"] = targetName;
	data["divisionCount"] = divisionCount;
	data["useAveraging"] = useAveraging;

	data["lerpType"] = EnumAdapter<LerpKeyframe::Type>::ToString(lerpType);
	timer.ToJson(data["Timer"]);

	// 全てのキーフレームを保存
	uint32_t index = 0;
	for (auto& keyframe : keyframes) {

		keyframe.ToJson(data["Keyframes"][std::to_string(index)]);
		++index;
	}
	JsonAdapter::Save(fileName, data);
}

std::vector<Vector3> CameraPathData::CollectTranslationPoints() const {

	// 補間座標の取得
	std::vector<Vector3> points;
	points.reserve(keyframes.size());
	for (auto& keyframe : keyframes) {

		points.push_back(keyframe.demoObject->GetTransform().GetWorldPos());
	}
	return points;
}