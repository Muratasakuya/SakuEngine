#include "GPUParticleGroup.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetEditor.h>
#include <Engine/Asset/Asset.h>
#include <Engine/Effect/Particle/ParticleConfig.h>
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

// imgui
#include <imgui.h>

//============================================================================
//	GPUParticleGroup classMethods
//============================================================================

void GPUParticleGroup::Create(ID3D12Device* device, Asset* asset, ParticlePrimitiveType primitiveType) {

	asset_ = nullptr;
	asset_ = asset;

	// 初期化値
	emitter_.common.Init();
	emitter_.sphere.Init();
	noiseUpdate_.Init();

	frequency_ = 0.4f;
	frequencyTime_ = 0.0f;
	isInitialized_ = false;
	blendMode_ = BlendMode::kBlendModeAdd;
	scalingValue_ = 1.0f;

	// 最初のテクスチャを設定
	textureName_ = "redCircle";
	noiseTextureName_ = "noise";

	// buffer作成
	BaseParticleGroup::CreatePrimitiveBuffer(device, primitiveType, kMaxGPUParticles);
	parentBuffer_.CreateBuffer(device);
	noiseBuffer_.CreateBuffer(device);
	// 球でデフォルトで作成
	emitterBuffer_.common.CreateBuffer(device);
	emitterBuffer_.sphere.CreateBuffer(device);
	// structuredBuffer(UAV)
	transformBuffer_.CreateUAVBuffer(device, kMaxGPUParticles);
	materialBuffer_.CreateUAVBuffer(device, kMaxGPUParticles);
	particleBuffer_.CreateUAVBuffer(device, kMaxGPUParticles);
	// freeList
	freeListIndexBuffer_.CreateUAVBuffer(device, 1);
	freeListBuffer_.CreateUAVBuffer(device, kMaxGPUParticles);
}

void GPUParticleGroup::CreateFromJson(ID3D12Device* device, Asset* asset, const Json& data) {

	asset_ = nullptr;
	asset_ = asset;

	// jsonからデータ取得
	FromJson(data);

	// buffer作成
	BaseParticleGroup::CreatePrimitiveBuffer(device, primitiveBuffer_.type, kMaxGPUParticles);
	parentBuffer_.CreateBuffer(device);
	noiseBuffer_.CreateBuffer(device);
	// 形状別に作成
	emitterBuffer_.common.CreateBuffer(device);
	switch (emitter_.shape) {
	case ParticleEmitterShape::Sphere: {

		emitterBuffer_.sphere.CreateBuffer(device);
		break;
	}
	case ParticleEmitterShape::Hemisphere: {

		emitterBuffer_.hemisphere.CreateBuffer(device);
		break;
	}
	case ParticleEmitterShape::Box: {

		emitterBuffer_.box.CreateBuffer(device);
		break;
	}
	case ParticleEmitterShape::Cone: {

		emitterBuffer_.cone.CreateBuffer(device);
		break;
	}
	}

	// structuredBuffer(UAV)
	transformBuffer_.CreateUAVBuffer(device, kMaxGPUParticles);
	materialBuffer_.CreateUAVBuffer(device, kMaxGPUParticles);
	particleBuffer_.CreateUAVBuffer(device, kMaxGPUParticles);
	// freeList
	freeListIndexBuffer_.CreateUAVBuffer(device, 1);
	freeListBuffer_.CreateUAVBuffer(device, kMaxGPUParticles);
}

void GPUParticleGroup::Update() {

	// 現在使用中のエミッタを更新
	UpdateEmitter();

	// 親の情報を更新
	UpdateParent();

	// ノイズ情報更新
	UpdateNoise();
}

void GPUParticleGroup::FrequencyEmit() {

	// 時間を進める
	frequencyTime_ += GameTimer::GetDeltaTime();
	// 発生間隔を過ぎたら発生させる
	if (frequency_ <= frequencyTime_) {

		frequencyTime_ -= frequency_;
		emitter_.common.emit = true;
	} else {

		emitter_.common.emit = false;
	}

	// 現在使用中のエミッタを更新
	UpdateEmitter();
	// emitterの描画
	BaseParticleGroup::DrawEmitter();

	// 親の情報を更新
	UpdateParent();

	// ノイズ情報更新
	UpdateNoise();
}

void GPUParticleGroup::Emit() {

	// 強制的に発生させる
	emitter_.common.emit = true;
	isForcedEmit_ = true;
}

void GPUParticleGroup::SetIsForcedEmit(bool emit) {

	// 発生できないようにする
	emitter_.common.emit = emit;
	isForcedEmit_ = emit;
}

void GPUParticleGroup::ApplyCommand(const ParticleCommand& command) {

	// IDごとのコマンドを設定
	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			emitter_.sphere.translation = *translation;
			emitter_.hemisphere.translation = *translation;
			emitter_.box.translation = *translation;
			emitter_.cone.translation = *translation;
		}
		break;
	}
	case ParticleCommandID::Scaling: {
		if (const auto& scaling = std::get_if<float>(&command.value)) {

			scalingValue_ = *scaling;
		}
		break;
	}
	case ParticleCommandID::SetRotation: {
		if (const auto& rotation = std::get_if<Vector3>(&command.value)) {

			emitterRotation_ = *rotation;
		} else if (const auto& matrix = std::get_if<Matrix4x4>(&command.value)) {

			setRotationMatrix_ = *matrix;
		}
		break;
	}
	}
}

void GPUParticleGroup::UpdateEmitter() {

	// buffer転送
	emitterBuffer_.common.TransferData(emitter_.common);

	// 回転の更新
	switch (emitter_.shape) {
	case ParticleEmitterShape::Sphere: {

		ParticleEmitterSphere emitter{};
		emitter = emitter_.sphere;
		emitter.radius *= scalingValue_;

		// buffer転送
		emitterBuffer_.sphere.TransferData(emitter);
		break;
	}
	case ParticleEmitterShape::Hemisphere: {

		if (setRotationMatrix_.has_value()) {

			emitter_.hemisphere.rotationMatrix = setRotationMatrix_.value();
		} else {

			emitter_.hemisphere.rotationMatrix = Matrix4x4::MakeRotateMatrix(emitterRotation_);
		}

		ParticleEmitterHemisphere emitter{};
		emitter = emitter_.hemisphere;
		emitter.radius *= scalingValue_;

		// buffer転送
		emitterBuffer_.hemisphere.TransferData(emitter);
		break;
	}
	case ParticleEmitterShape::Box: {

		if (setRotationMatrix_.has_value()) {

			emitter_.box.rotationMatrix = setRotationMatrix_.value();
		} else {

			emitter_.box.rotationMatrix = Matrix4x4::MakeRotateMatrix(emitterRotation_);
		}

		ParticleEmitterBox emitter{};
		emitter = emitter_.box;
		emitter.size *= scalingValue_;

		// buffer転送
		emitterBuffer_.box.TransferData(emitter);
		break;
	}
	case ParticleEmitterShape::Cone: {

		if (setRotationMatrix_.has_value()) {

			emitter_.cone.rotationMatrix = setRotationMatrix_.value();
		} else {

			emitter_.cone.rotationMatrix = Matrix4x4::MakeRotateMatrix(emitterRotation_);
		}

		// スケーリング見送り

		// buffer転送
		emitterBuffer_.cone.TransferData(emitter_.cone);
		break;
	}
	}
}

void GPUParticleGroup::UpdateParent() {

	// 親の設定
	ParentForGPU paent{};
	if (parentTransform_) {

		paent.aliveParent = true;
		paent.parentMatrix = parentTransform_->matrix.world;
	} else {

		paent.aliveParent = false;
		paent.parentMatrix = Matrix4x4::MakeIdentity4x4();
	}

	// buffer転送
	parentBuffer_.TransferData(paent);
}

void GPUParticleGroup::UpdateNoise() {

	// buffer転送
	noiseBuffer_.TransferData(noiseUpdate_);
}

void GPUParticleGroup::ImGui(ID3D12Device* device) {

	ImGui::Text("kMaxParticle: %d", kMaxGPUParticles);

	if (ImGui::BeginTabBar("GPUParticleGroupTab")) {
		if (ImGui::BeginTabItem("Render")) {

			// 表示サイズ
			const float imageSize = 88.0f;

			// 使用しているtextureの名前を表示
			ImGuiHelper::ImageButtonWithLabel("texture", textureName_,
				(ImTextureID)asset_->GetGPUHandle(textureName_).ptr, { imageSize, imageSize });
			if (const auto* payload = ImGuiHelper::DragDropPayload(PendingType::Texture)) {

				// textureを設定
				textureName_ = payload->name;
			}
			ImGui::SameLine();
			ImGuiHelper::ImageButtonWithLabel("noiseTexture", noiseTextureName_,
				(ImTextureID)asset_->GetGPUHandle(noiseTextureName_).ptr, { imageSize, imageSize });
			if (const auto* payload = ImGuiHelper::DragDropPayload(PendingType::Texture)) {

				// textureを設定
				noiseTextureName_ = payload->name;
			}

			ImGui::Separator();

			EnumAdapter<BlendMode>::Combo("blendMode", &blendMode_);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Common")) {

			ImGui::DragInt("count", &emitter_.common.count, 1, 0, kMaxGPUParticles);
			ImGui::DragFloat("lifeTime", &emitter_.common.lifeTime, 0.01f);
			ImGui::DragFloat("frequency", &frequency_, 0.01f);
			ImGui::DragFloat("moveSpeed", &emitter_.common.moveSpeed, 0.01f);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Emitter")) {

			SelectEmitter(device);
			BaseParticleGroup::EditEmitter();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Transform")) {

			ImGui::DragFloat3("scale", &emitter_.common.scale.x, 0.01f);

			ImGui::SeparatorText("Parent");

			BaseParticleGroup::ImGuiParent();

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Material")) {

			ImGui::ColorEdit4("color", &emitter_.common.color.r);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Updater")) {

			EnumAdapter<GPUParticle::UpdateType>::Combo("type", &updateType_);

			// noise選択中
			if (updateType_ == GPUParticle::UpdateType::Noise) {

				ImGui::DragFloat("scale", &noiseUpdate_.scale, 0.01f, 0.0f, 8.0f);
				ImGui::DragFloat("strength", &noiseUpdate_.strength, 0.01f, 0.0f, 8.0f);
				ImGui::DragFloat("speed", &noiseUpdate_.speed, 0.01f, 0.0f, 8.0f);
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

D3D12_GPU_VIRTUAL_ADDRESS GPUParticleGroup::GetEmitterShapeBufferAdress() const {

	switch (emitter_.shape) {
	case ParticleEmitterShape::Sphere: {

		return emitterBuffer_.sphere.GetResource()->GetGPUVirtualAddress();
	}
	case ParticleEmitterShape::Hemisphere: {

		return emitterBuffer_.hemisphere.GetResource()->GetGPUVirtualAddress();
	}
	case ParticleEmitterShape::Box: {

		return emitterBuffer_.box.GetResource()->GetGPUVirtualAddress();
	}
	case ParticleEmitterShape::Cone: {

		return emitterBuffer_.cone.GetResource()->GetGPUVirtualAddress();
	}
	case ParticleEmitterShape::Count: {

		ASSERT(false, "ParticleEmitterType::Count is not buffer");
		return emitterBuffer_.sphere.GetResource()->GetGPUVirtualAddress();
	}
	}

	// フォロースルー
	ASSERT(false, "ParticleEmitterType::Count is not buffer");
	return emitterBuffer_.sphere.GetResource()->GetGPUVirtualAddress();
}

void GPUParticleGroup::SelectEmitter(ID3D12Device* device) {

	ParticleEmitterShape preShape = emitter_.shape;
	if (EnumAdapter<ParticleEmitterShape>::Combo("emitterShape", &emitter_.shape)) {

		// まだbufferが作成されていなければ作成する
		switch (emitter_.shape) {
		case ParticleEmitterShape::Sphere: {
			if (!emitterBuffer_.sphere.IsCreatedResource()) {

				emitter_.sphere.Init();
				emitterBuffer_.sphere.CreateBuffer(device);

				switch (preShape) {
				case ParticleEmitterShape::Hemisphere:

					emitter_.sphere.translation = emitter_.hemisphere.translation;
					break;
				case ParticleEmitterShape::Box:

					emitter_.sphere.translation = emitter_.box.translation;
					break;
				case ParticleEmitterShape::Cone:

					emitter_.sphere.translation = emitter_.cone.translation;
					break;
				}
			}
			break;
		}
		case ParticleEmitterShape::Hemisphere: {
			if (!emitterBuffer_.hemisphere.IsCreatedResource()) {

				emitter_.hemisphere.Init();
				emitterBuffer_.hemisphere.CreateBuffer(device);

				switch (preShape) {
				case ParticleEmitterShape::Sphere:

					emitter_.hemisphere.translation = emitter_.sphere.translation;
					break;
				case ParticleEmitterShape::Box:

					emitter_.hemisphere.translation = emitter_.box.translation;
					break;
				case ParticleEmitterShape::Cone:

					emitter_.hemisphere.translation = emitter_.cone.translation;
					break;
				}
			}
			break;
		}
		case ParticleEmitterShape::Box: {
			if (!emitterBuffer_.box.IsCreatedResource()) {

				emitter_.box.Init();
				emitterBuffer_.box.CreateBuffer(device);

				switch (preShape) {
				case ParticleEmitterShape::Sphere:

					emitter_.box.translation = emitter_.sphere.translation;
					break;
				case ParticleEmitterShape::Hemisphere:

					emitter_.box.translation = emitter_.hemisphere.translation;
					break;
				case ParticleEmitterShape::Cone:

					emitter_.box.translation = emitter_.cone.translation;
					break;
				}
			}
			break;
		}
		case ParticleEmitterShape::Cone: {
			if (!emitterBuffer_.cone.IsCreatedResource()) {

				emitter_.cone.Init();
				emitterBuffer_.cone.CreateBuffer(device);

				switch (preShape) {
				case ParticleEmitterShape::Sphere:

					emitter_.cone.translation = emitter_.sphere.translation;
					break;
				case ParticleEmitterShape::Hemisphere:

					emitter_.cone.translation = emitter_.hemisphere.translation;
					break;
				case ParticleEmitterShape::Box:

					emitter_.cone.translation = emitter_.box.translation;
					break;
				}
			}
			break;
		}
		}
	}
}

Json GPUParticleGroup::ToJson() const {

	Json data;

	//============================================================================
	//	GroupParameters
	//============================================================================

	data["primitive"] = EnumAdapter<ParticlePrimitiveType>::ToString(primitiveBuffer_.type);
	data["blendMode"] = EnumAdapter<BlendMode>::ToString(blendMode_);
	data["updateType"] = EnumAdapter<GPUParticle::UpdateType>::ToString(updateType_);

	data["textureName"] = textureName_;
	data["noiseTextureName"] = noiseTextureName_;
	data["frequency"] = frequency_;

	//============================================================================
	//	UpdaterParameters
	//============================================================================

	data["noise"]["scale"] = noiseUpdate_.scale;
	data["noise"]["strength"] = noiseUpdate_.strength;
	data["noise"]["speed"] = noiseUpdate_.speed;

	//============================================================================
	//	EmitterParameters
	//============================================================================

	data["emitterRotation"] = emitterRotation_.ToJson();
	data["emitterShape"] = EnumAdapter<ParticleEmitterShape>::ToString(emitter_.shape);

	data["common"]["count"] = emitter_.common.count;
	data["common"]["lifeTime"] = emitter_.common.lifeTime;
	data["common"]["moveSpeed"] = emitter_.common.moveSpeed;
	data["common"]["scale"] = emitter_.common.scale.ToJson();
	data["common"]["color"] = emitter_.common.color.ToJson();

	switch (emitter_.shape) {
	case ParticleEmitterShape::Sphere:

		data["sphere"]["radius"] = emitter_.sphere.radius;
		data["sphere"]["translation"] = emitter_.sphere.translation.ToJson();
		break;
	case ParticleEmitterShape::Hemisphere:

		data["hemisphere"]["radius"] = emitter_.hemisphere.radius;
		data["hemisphere"]["translation"] = emitter_.hemisphere.translation.ToJson();
		break;
	case ParticleEmitterShape::Box:

		data["box"]["size"] = emitter_.box.size.ToJson();
		data["box"]["translation"] = emitter_.box.translation.ToJson();
		break;
	case ParticleEmitterShape::Cone:

		data["cone"]["baseRadius"] = emitter_.cone.baseRadius;
		data["cone"]["topRadius"] = emitter_.cone.topRadius;
		data["cone"]["height"] = emitter_.cone.height;
		data["cone"]["translation"] = emitter_.cone.translation.ToJson();
		break;
	}
	return data;
}

void GPUParticleGroup::FromJson(const Json& data) {

	//============================================================================
	//	GroupParameters
	//============================================================================

	const auto& primitive = EnumAdapter<ParticlePrimitiveType>::FromString(data["primitive"]);
	primitiveBuffer_.type = primitive.value();
	const auto& blendMode = EnumAdapter<BlendMode>::FromString(data["blendMode"]);
	blendMode_ = blendMode.value();
	const auto& updateType = EnumAdapter<GPUParticle::UpdateType>::FromString(data["updateType"]);
	updateType_ = updateType.value();

	textureName_ = data.value("textureName", "circle");
	noiseTextureName_ = data.value("noiseTextureName", "noise");
	frequency_ = data.value("frequency", 0.4f);

	//============================================================================
	//	UpdaterParameters
	//============================================================================

	noiseUpdate_.scale = data["noise"]["scale"];
	noiseUpdate_.strength = data["noise"]["strength"];
	noiseUpdate_.speed = data["noise"]["speed"];

	//============================================================================
	//	EmitterParameters
	//============================================================================

	emitterRotation_ = emitterRotation_.FromJson(data["emitterRotation"]);
	const auto& emitterShape = EnumAdapter<ParticleEmitterShape>::FromString(data["emitterShape"]);
	emitter_.shape = emitterShape.value();

	const auto& commonData = data["common"];
	emitter_.common.count = commonData.value("count", 32);
	emitter_.common.lifeTime = commonData.value("lifeTime", 1.0f);
	emitter_.common.moveSpeed = commonData.value("moveSpeed", 1.0f);
	emitter_.common.scale = emitter_.common.scale.FromJson(commonData["scale"]);
	emitter_.common.color = emitter_.common.color.FromJson(commonData["color"]);

	switch (emitter_.shape) {
	case ParticleEmitterShape::Sphere: {

		const auto& emitterData = data["sphere"];
		emitter_.sphere.radius = emitterData.value("radius", 2.0f);
		emitter_.sphere.translation = emitter_.sphere.translation.FromJson(emitterData["translation"]);
		break;
	}
	case ParticleEmitterShape::Hemisphere: {

		const auto& emitterData = data["hemisphere"];
		emitter_.hemisphere.radius = emitterData.value("radius", 2.0f);
		emitter_.hemisphere.translation = emitter_.hemisphere.translation.FromJson(emitterData["translation"]);
		break;
	}
	case ParticleEmitterShape::Box: {

		const auto& emitterData = data["box"];
		emitter_.box.size = emitter_.box.size.FromJson(emitterData["size"]);
		emitter_.box.translation = emitter_.box.translation.FromJson(emitterData["translation"]);
		break;
	}
	case ParticleEmitterShape::Cone: {

		const auto& emitterData = data["cone"];
		emitter_.cone.baseRadius = emitterData.value("baseRadius", 0.4f);
		emitter_.cone.topRadius = emitterData.value("topRadius", 1.6f);
		emitter_.cone.height = emitterData.value("height", 1.6f);
		emitter_.cone.translation = emitter_.cone.translation.FromJson(emitterData["translation"]);
		break;
	}
	}
}