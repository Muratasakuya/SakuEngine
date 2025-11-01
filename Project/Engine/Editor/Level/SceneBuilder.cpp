#include "SceneBuilder.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Asset/AssetEditor.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

// object
#include <Game/Objects/GameScene/Environment/Object/FieldCrossMarkWall.h>

//============================================================================
//	SceneBuilder classMethods
//============================================================================

void SceneBuilder::Init(const std::string& jsonPath) {

	jsonPath_ = jsonPath;

	idDeleteOnSameName_ = true;

	leftChildSize_ = ImVec2(320.0f, 320.0f);
	buttonSize_ = ImVec2(256.0f, 32.0f);
}

void SceneBuilder::ImGui() {

	// layout
	ImGui::BeginGroup();

	// jsonファイル受け取り
	ImGui::BeginChild("RecieveChild##SceneBuilder", leftChildSize_, true);
	ImGui::SeparatorText("Recieve File");

	RecieveFile();
	ImGui::EndChild();

	// 横並びにする
	ImGui::SameLine();
}

void SceneBuilder::CreateObjectsMap(std::unordered_map<Level::ObjectType,
	std::vector<std::unique_ptr<GameObject3D>>>& objectsMap) {

	// ファイルを読み込んでjsonデータを取得
	Json data = JsonAdapter::Load(fileName_.value());

	// sceneFileかチェックする
	if (!data.contains("name")) {
		return;
	}
	if (data["name"] != "scene") {
		return;
	}

	LOG_INFO("create sceneObjects...: {}", fileName_.value());

	// 一度すべて破棄
	objectsMap.clear();

	// 作成処理
	for (const auto& obj : data["objects"]) {

		BuildObjects(obj, objectsMap);
	}

	LOG_INFO("create sceneObjects finished: {}", fileName_.value());
}

bool SceneBuilder::IsMeshObjectCreatable(const Json& obj) const {

	return obj.value("type", "") == "MESH" && obj.value("entity_flag", false);
}

void SceneBuilder::BuildObjects(const Json& obj,
	std::unordered_map<Level::ObjectType,
	std::vector<std::unique_ptr<GameObject3D>>>& objectsMap) {

	// "MESHかどうかチェック"
	if (IsMeshObjectCreatable(obj)) {

		// 種類取得
		const std::string objectTypeName = obj.value("entity_type", "None");
		Level::ObjectType objectType = GetObjectType(objectTypeName);

		auto& entities = objectsMap[objectType];

		// 同名削除処理
		const std::string identifier = obj.value("name", "");
		if (idDeleteOnSameName_) {

			HandleDuplicateObject(entities, identifier);
		}

		// 生成処理
		auto newobject = CreateObject(obj, objectType);

		// transform反映
		ApplyTransform(*newobject, obj);
		// material反映
		Json materialData = LoadObjectFile(identifier);
		ApplyMaterial(*newobject, materialData);
		// collision反映
		ApplyCollision(*newobject, obj);

		// シーンが破棄されても削除しない
		newobject->SetDestroyOnLoad(false);

		// 登録
		entities.emplace_back(std::move(newobject));
	}

	// 子を作成する
	if (obj.contains("children")) {
		for (const auto& child : obj["children"]) {

			BuildObjects(child, objectsMap);
		}
	}
}

Json SceneBuilder::LoadObjectFile(const std::string& identifier) {

	Json data;
	const std::string& newIdentifier = Algorithm::RemoveAfterUnderscore(identifier);
	if (!JsonAdapter::LoadCheck(jsonPath_ + newIdentifier + ".json", data)) {

		// 読み込めなかった場合空のJsonを返す
		return Json();
	}

	return data;
}

void SceneBuilder::HandleDuplicateObject(std::vector<std::unique_ptr<GameObject3D>>& objects,
	const std::string& identifier) {

	objects.erase(std::remove_if(objects.begin(), objects.end(),
		[&](const std::unique_ptr<GameObject3D>& object) {
			return object->GetIdentifier() == identifier; }),
			objects.end());
}

std::unique_ptr<GameObject3D> SceneBuilder::CreateObject(
	const Json& obj, Level::ObjectType objectType) {

	auto object = CreateObjectPtr(objectType);

	const std::string modelName = obj.value("modelName", "");

	object->Init(modelName, modelName, "Scene");
	object->SetIdentifier(obj.value("name", ""));

	return object;
}

std::unique_ptr<GameObject3D> SceneBuilder::CreateObjectPtr(Level::ObjectType objectType) {

	switch (objectType) {
	case Level::ObjectType::None: {

		return std::make_unique<GameObject3D>();
	}
	case Level::ObjectType::CrossMarkWall: {

		return std::make_unique<FieldCrossMarkWall>();
	}
	}
	return nullptr;
}

void SceneBuilder::ApplyTransform(GameObject3D& object, const Json& obj) {

	if (!obj.contains("transform")) {
		return;
	}

	const Json& transform = obj["transform"];

	// 平行移動
	if (transform.contains("translation")) {

		const auto& T = transform["translation"];
		object.SetTranslation(Vector3(T[0].get<float>(), T[2].get<float>(), T[1].get<float>()));
	}

	// スケール
	if (transform.contains("scaling")) {

		const auto& S = transform["scaling"];
		object.SetScale(Vector3(S[0].get<float>(), S[2].get<float>(), S[1].get<float>()));
	}

	// 回転
	if (transform.contains("rotation_quaternion")) {

		const auto& R = transform["rotation_quaternion"];
		object.SetRotation(
			Quaternion(R[0].get<float>(), R[2].get<float>(),
				-R[1].get<float>(), R[3].get<float>()).Normalize());
	}
}

void SceneBuilder::ApplyMaterial(GameObject3D& object, const Json& data) {

	// 空の場合処理しない
	if (data.empty()) {
		return;
	}

	object.ApplyMaterial(data);

	// マテリアルの共通設定
	object.SetPostProcessMask(Bit_Bloom | Bit_RadialBlur |
		Bit_Glitch | Bit_Grayscale | Bit_Vignette | Bit_CRTDisplay);
}

void SceneBuilder::ApplyCollision(GameObject3D& object, const Json& data) {

	if (!data.contains("collision")) {
		return;
	}

	// 有効な型かチェックする
	if (CheckCollisionValid<FieldCrossMarkWall>(object)) {

		// colliderを設定
		object.BuildBodies(data["collision"]);
	}
}

void SceneBuilder::Reset() {

	fileName_ = std::nullopt;
}

void SceneBuilder::SetFile(const std::string& sceneFile) {

	std::string loadName = "Level/" + sceneFile + ".json";
	// 作成元設定
	fileName_ = loadName;
}

void SceneBuilder::RecieveFile() {

	ImGui::Checkbox("idDeleteOnSameName", &idDeleteOnSameName_);

	ImGui::Button("Load File", buttonSize_);
	if (ImGui::BeginDragDropTarget()) {
		if (const auto* payload = ImGuiHelper::DragDropPayload(PendingType::None)) {

			// 名前の設定
			// 読み込めたら作成させる
			std::string loadName = "Level/" + std::string(payload->name) + ".json";
			if (JsonAdapter::LoadAssert(loadName)) {

				// 作成元設定
				fileName_ = loadName;
			}
		}
		ImGui::EndDragDropTarget();
	}
}

Level::ObjectType SceneBuilder::GetObjectType(const std::string& objectTypeName) {

	Level::ObjectType objectType = Level::ObjectType::None;

	if (objectTypeName == "None") {

		objectType = Level::ObjectType::None;
	} else if (objectTypeName == "CrossMarkWall") {

		objectType = Level::ObjectType::CrossMarkWall;
	}

	return objectType;
}