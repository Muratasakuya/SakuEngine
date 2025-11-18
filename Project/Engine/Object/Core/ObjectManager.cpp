#include "ObjectManager.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>

// data
#include <Engine/Object/Data/Transform.h>
#include <Engine/Object/Data/Material.h>
#include <Engine/Object/Data/SkinnedAnimation.h>
#include <Engine/Object/Data/ObjectTag.h>
#include <Engine/Object/Data/MeshRender.h>
// systems
#include <Engine/Object/System/Systems/TransformSystem.h>
#include <Engine/Object/System/Systems/MaterialSystem.h>
#include <Engine/Object/System/Systems/AnimationSystem.h>
#include <Engine/Object/System/Systems/InstancedMeshSystem.h>
#include <Engine/Object/System/Systems/SpriteBufferSystem.h>
#include <Engine/Object/System/Systems/SkyboxRenderSystem.h>
#include <Engine/Object/System/Systems/TagSystem.h>

//============================================================================
//	ObjectManager classMethods
//============================================================================

ObjectManager* ObjectManager::instance_ = nullptr;

ObjectManager* ObjectManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ObjectManager();
	}
	return instance_;
}

void ObjectManager::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void ObjectManager::Init(ID3D12Device* device, Asset* asset, DxCommand* dxCommand) {

	asset_ = nullptr;
	asset_ = asset;

	device_ = nullptr;
	device_ = device;

	objectPoolManager_ = std::make_unique<ObjectPoolManager>();
	systemManager_ = std::make_unique<SystemManager>();

	// system登録
	systemManager_->AddSystem<Transform3DSystem>();
	systemManager_->AddSystem<Transform2DSystem>();
	systemManager_->AddSystem<AnimationSystem>();
	systemManager_->AddSystem<MaterialSystem>();
	systemManager_->AddSystem<SpriteMaterialSystem>();
	systemManager_->AddSystem<TagSystem>();
	systemManager_->AddSystem<SpriteBufferSystem>();
	systemManager_->AddSystem<SkyboxRenderSystem>();
	systemManager_->AddSystem<InstancedMeshSystem>(device, asset, dxCommand);
	systemManager_->GetSystem<InstancedMeshSystem>()->StartBuildWorker();

	ImGuiObjectEditor::GetInstance()->Init();
}

void ObjectManager::UpdateData() {

	systemManager_->UpdateData(*objectPoolManager_.get());
}

void ObjectManager::UpdateBuffer() {

	systemManager_->UpdateBuffer(*objectPoolManager_.get());
}

uint32_t ObjectManager::CreateObjects(const std::string& modelName,
	const std::string& name, const std::string& groupName,
	const std::optional<std::string>& animationName) {

	LOG_SCOPE_MS_LABEL(modelName);

	// object作成
	uint32_t object = BuildEmptyobject(name, groupName);
	// 必要なdataを作成
	auto* transform = objectPoolManager_->AddData<Transform3D>(object);
	auto* materialsPtr = objectPoolManager_->AddData<Material, true>(object);
	auto* meshRender = objectPoolManager_->AddData<MeshRender>(object);

	// 各dataを初期化
	// transform
	transform->Init();
	// instancingのデータ名を設定
	transform->SetInstancingName(modelName);

	// material
	const ModelData& modelData = asset_->GetModelData(modelName);
	auto& materials = *materialsPtr;
	systemManager_->GetSystem<MaterialSystem>()->Init(
		materials, modelData, asset_);

	// meshRender
	meshRender->Init(modelName);

	if (animationName.has_value()) {

		// animation処理がある場合はdataを追加
		auto* animation = objectPoolManager_->AddData<SkinnedAnimation>(object);
		// 初期化
		animation->Init(*animationName, asset_);

		LOG_INFO("created object3D: name: [{}] skinnedMesh: [{}] animation: [{}]", name, modelName, animationName.value());
	} else {

		LOG_INFO("created object3D: name: [{}] staticMesh: [{}]", name, modelName);
	}
	return object;
}

uint32_t ObjectManager::CreateSkybox(const std::string& textureName) {

	LOG_SCOPE_MS_LABEL("skybox");

	// object作成
	uint32_t object = BuildEmptyobject("skybox", "Environment");
	// 必要なdataを作成
	auto* skybox = objectPoolManager_->AddData<Skybox>(object);

	// dataを初期化
	skybox->Create(device_, asset_->GetTextureGPUIndex(textureName), object);
	LOG_INFO("created skybox: textureName: [{}]", textureName);

	return object;
}

uint32_t ObjectManager::CreateEffect(const std::string& name, const std::string& groupName) {

	LOG_SCOPE_MS_LABEL("effect");

	// object作成
	uint32_t object = BuildEmptyobject(name, groupName);
	// 必要なdataを作成
	auto* transform = objectPoolManager_->AddData<EffectTransform>(object);

	// dataを初期化
	transform->Init();
	LOG_INFO("created effect: name: [{}]", name);

	return object;
}

uint32_t ObjectManager::CreateObject2D(const std::string& textureName,
	const std::string& name, const std::string& groupName) {

	LOG_SCOPE_MS_LABEL(textureName);

	// object作成
	uint32_t object = BuildEmptyobject(name, groupName);
	// 必要なdataを作成
	auto* transform = objectPoolManager_->AddData<Transform2D>(object);
	auto* material = objectPoolManager_->AddData<SpriteMaterial>(object);

	// 各dataを初期化
	// transform
	transform->Init(device_);
	// material
	material->Init(device_);
	// sprite
	objectPoolManager_->AddData<Sprite>(object,
		device_, asset_, textureName, *transform);
	LOG_INFO("created object2D: name: [{}] textureName: [{}]", name, textureName);

	return object;
}

uint32_t ObjectManager::BuildEmptyobject(const std::string& name, const std::string& groupName) {

	// object作成
	uint32_t object = objectPoolManager_->Create();
	// tag設定
	auto* tag = objectPoolManager_->AddData<ObjectTag>(object);
	tag->identifier = name;
	tag->name = systemManager_->GetSystem<TagSystem>()->CheckName(name);
	tag->groupName = groupName;

	return object;
}

void ObjectManager::Destroy(uint32_t object) {

	objectPoolManager_->Destroy(object);
}

void ObjectManager::DestroyAll() {

	// すべて走査して破棄
	Archetype mask{};
	auto objects = objectPoolManager_->View(mask);
	for (uint32_t id : objects) {

		// 破棄フラグがたっていなければ破棄しない
		const auto& tag = objectPoolManager_->GetData<ObjectTag>(id);
		if (!tag->destroyOnLoad) {
			continue;
		}
		objectPoolManager_->Destroy(id);
	}
}