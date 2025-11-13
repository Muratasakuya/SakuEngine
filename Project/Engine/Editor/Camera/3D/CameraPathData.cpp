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
	viewScale = demoObject->GetScale();
	// ゲームで使用する場合描画しない
	demoObject->SetScale(isUseGame ? Vector3::AnyInit(0.0f) : viewScale);
	demoObject->SetMeshRenderView(MeshRenderView::Scene);
}

void CameraPathData::KeyframeParam::FromJson(const Json& data) {

	fovY = data.value("fovY", 54.0f);

	translation = Vector3::FromJson(data.value("translation", Json()));
	rotation = Quaternion::Normalize(Quaternion::FromJson(data.value("rotation", Json())));
	stayTime = data.value("stayTime", 0.0f);

	demoObject->SetTranslation(translation);
	demoObject->SetRotation(rotation);
	demoObject->SetEulerRotation(Quaternion::ToEulerAngles(rotation));
}

void CameraPathData::KeyframeParam::ToJson(Json& data) {

	data["fovY"] = fovY;
	// ローカルを渡す
	data["translation"] = translation.ToJson();
	data["rotation"] = rotation.ToJson();
	data["stayTime"] = stayTime;
}

void CameraPathData::ApplyJson(const std::string& fileName, bool _isUseGame) {

	Json data;
	// 読み込めなければ処理しない
	if (!JsonAdapter::LoadCheck(fileName, data)) {
		return;
	}

	// ゲームで使用する場合は線描画をしない
	if (_isUseGame) {

		isDrawLine3D = false;
		isDrawKeyframe = false;
		this->isUseGame = _isUseGame;
	}

	name = data.value("name", "name");

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

			points.emplace_back(keyframe.demoObject->GetTransform().translation);
		}
		averagedT = LerpKeyframe::AveragingPoints<Vector3>(points, divisionCount, lerpType);
	}
}

void CameraPathData::SaveJson(const std::string& fileName) {

	Json data;

	data["name"] = name;

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

float CameraPathData::UpdateAndGetEffectiveEasedT() {

	const int n = static_cast<int>(keyframes.size());
	if (n <= 1) {

		// キーが1つしか無いなら常に0.0f
		lastEasedT = 0.0f;
		return 0.0f;
	}

	const float division = 1.0f / (n - 1);
	const float dt = GameTimer::GetScaledDeltaTime();
	// 待機中の時間経過
	if (staying) {

		stayRemain -= dt;
		const float holdT = division * std::clamp(stayKeyIndex, 0, n - 1);
		if (stayRemain <= 0.0f) {

			// 次フレームから再開させる
			staying = false;
			lastEasedT = holdT;
		}
		return holdT;
	}

	// 時間の計算
	const float targetT = (timer.target_ > 0.0f) ? timer.target_ : 1.0f;
	const float nextCurrent = timer.current_ + dt;
	const float nextTLinear = std::clamp(nextCurrent / targetT, 0.0f, 1.0f);
	const float nextEasedT = EasedValue(timer.easeingType_, nextTLinear);

	// 直前のeasedTの値
	const float prev = lastEasedT = (lastEasedT == 0.0f ? timer.easedT_ : lastEasedT);

	// キー境界を跨ぐかチェック
	int crossedIndex = -1;
	if (nextEasedT > prev) {

		const int startI = static_cast<int>(std::floor(prev / division)) + 1;
		const int endI = static_cast<int>(std::floor(nextEasedT / division));
		for (int i = (std::max)(1, startI); i <= (std::min)(n - 1, endI); ++i) {
			if (i >= 0 && i < n) {

				crossedIndex = i;
				break;
			}
		}
	}

	// 跨いだ先のキーに待ち時間があれば滞留に入る
	if (crossedIndex >= 0) {

		float hold = keyframes[crossedIndex].stayTime;
		if (hold > 0.0f) {

			// 滞留開始
			staying = true;
			stayRemain = hold;
			stayKeyIndex = crossedIndex;
			float holdT = division * crossedIndex;
			return holdT;
		}
	}

	// アニメーション時間更新
	timer.Update();
	// 最新のeasedTを保存
	lastEasedT = timer.easedT_;
	return timer.easedT_;
}