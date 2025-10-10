#include "ICPUParticleSpawnModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Asset/AssetEditor.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

//============================================================================
//	ICPUParticleSpawnModule classMethods
//============================================================================

void ICPUParticleSpawnModule::InitCommonData() {

	// 初期化値
	// Emit
	emitCount_ = ParticleValue<uint32_t>::SetValue(4);
	lifeTime_ = ParticleValue<float>::SetValue(0.8f);

	// TextureInfo
	textureInfo_.samplerType = 0;
	textureInfo_.useNoiseTexture = false;
	// デフォルトのテクスチャで初期化
	textureName_ = "circle";
	noiseTextureName_ = "noise";
	textureInfo_.colorTextureIndex = asset_->GetTextureGPUIndex(textureName_);
	textureInfo_.noiseTextureIndex = asset_->GetTextureGPUIndex(noiseTextureName_);

	// 移動速度
	moveSpeed_ = ParticleValue<float>::SetValue(1.6f);
}

void ICPUParticleSpawnModule::SetCommonData(CPUParticle::ParticleData& particle) {

	// 生存時間
	particle.lifeTime = lifeTime_.GetValue();

	// テクスチャ情報
	particle.textureInfo = textureInfo_;

	// プリミティブ
	particle.primitive = primitive_;
}

void ICPUParticleSpawnModule::SetPrimitiveType(ParticlePrimitiveType type) {

	primitive_.type = type;
	// 形状で初期化
	switch (type) {
	case ParticlePrimitiveType::Plane:

		primitive_.plane.Init();
		break;
	case ParticlePrimitiveType::Ring:

		primitive_.ring.Init();
		break;
	case ParticlePrimitiveType::Cylinder:

		primitive_.cylinder.Init();
		break;
	case ParticlePrimitiveType::Crescent:

		primitive_.crescent.Init();
		break;
	}
}

void ICPUParticleSpawnModule::SetParent(bool isSet, const BaseTransform& parent) {

	if (isSet) {

		parentTransform_ = &parent;
	} else {

		parentTransform_ = nullptr;
	}
}

void ICPUParticleSpawnModule::ShareCommonParam(ICPUParticleSpawnModule* other) {

	// Emit
	emitCount_ = other->emitCount_;

	// TextureInfo
	textureName_ = other->textureName_;
	noiseTextureName_ = other->noiseTextureName_;
	textureInfo_ = other->textureInfo_;

	// 移動速度
	moveSpeed_ = other->moveSpeed_;

	// Primitive
	primitive_ = other->primitive_;

	// 親の設定
	if (other->parentTransform_) {

		SetParent(true, *other->parentTransform_);
	} else {

		SetParent(false, Transform3D());
	}
}

void ICPUParticleSpawnModule::ImGuiRenderParam() {

	ImGui::SeparatorText("Texture");

	// ドラッグアンドドロップ処理
	DragAndDropTexture();

	ImGui::DragInt("samplerType", &textureInfo_.samplerType, 1, 0, 1);
	ImGui::SameLine();
	ImGui::Text("    : %s", EnumAdapter<ParticleCommon::SamplerType>::GetEnumName(textureInfo_.samplerType));
}

void ICPUParticleSpawnModule::ImGuiPrimitiveParam() {

	switch (primitive_.type) {
	case ParticlePrimitiveType::Plane:

		ImGui::DragFloat2("size", &primitive_.plane.size.x, 0.01f);
		ImGui::DragFloat2("pivot", &primitive_.plane.pivot.x, 0.01f);

		if (EnumAdapter<ParticlePlaneType>::Combo("planeType", &planeType_)) {

			primitive_.plane.mode = static_cast<uint32_t>(planeType_);
		}
		break;
	case ParticlePrimitiveType::Ring:

		ImGui::DragInt("divide", &primitive_.ring.divide, 1, 3, 32);
		ImGui::DragFloat("outerRadius", &primitive_.ring.outerRadius, 0.01f);
		ImGui::DragFloat("innerRadius", &primitive_.ring.innerRadius, 0.01f);
		break;
	case ParticlePrimitiveType::Cylinder:

		ImGui::DragInt("divide", &primitive_.cylinder.divide, 1, 3, 32);
		ImGui::DragFloat("topRadius", &primitive_.cylinder.topRadius, 0.01f);
		ImGui::DragFloat("bottomRadius", &primitive_.cylinder.bottomRadius, 0.01f);
		ImGui::DragFloat("height", &primitive_.cylinder.height, 0.01f);
		break;
	case ParticlePrimitiveType::Crescent:

		ImGui::DragInt("divide", &primitive_.crescent.divide, 1, 3, 30);
		ImGui::DragInt("uvMode", &primitive_.crescent.uvMode, 1, 0, 1);
		ImGui::DragFloat("outerRadius", &primitive_.crescent.outerRadius, 0.01f);
		ImGui::DragFloat("innerRadius", &primitive_.crescent.innerRadius, 0.01f);
		ImGui::DragFloat("startAngle", &primitive_.crescent.startAngle, 0.01f);
		ImGui::DragFloat("endAngle", &primitive_.crescent.endAngle, 0.01f);
		ImGui::DragFloat("lattice", &primitive_.crescent.lattice, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("thickness", &primitive_.crescent.thickness, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat2("pivot", &primitive_.crescent.pivot.x, 0.01f);

		ImGui::ColorEdit4("outerColor", &primitive_.crescent.outerColor.r);
		ImGui::ColorEdit4("innerColor", &primitive_.crescent.innerColor.r);
		break;
	}
}

void ICPUParticleSpawnModule::ImGuiEmitParam() {

	emitCount_.EditDragValue("emitCount");
	lifeTime_.EditDragValue("lifeTime");

	moveSpeed_.EditDragValue("moveSpeed");
}

void ICPUParticleSpawnModule::DragAndDropTexture() {

	// 表示サイズ
	const float imageSize = 88.0f;

	// 使用しているtextureの名前を表示
	ImGuiHelper::ImageButtonWithLabel("texture", textureName_,
		(ImTextureID)asset_->GetGPUHandle(textureName_).ptr, { imageSize, imageSize });

	std::string textureName = ImGuiHelper::DragDropPayloadString(PendingType::Texture);
	if (!textureName.empty()) {

		// textureを設定
		textureName_ = textureName;
		// indexを設定
		textureInfo_.colorTextureIndex = asset_->GetTextureGPUIndex(textureName_);
	}
	ImGui::SameLine();
	ImGuiHelper::ImageButtonWithLabel("noiseTexture", noiseTextureName_,
		(ImTextureID)asset_->GetGPUHandle(noiseTextureName_).ptr, { imageSize, imageSize });

	std::string noiseTextureName = ImGuiHelper::DragDropPayloadString(PendingType::Texture);
	if (!noiseTextureName.empty()) {

		// textureを設定
		noiseTextureName_ = noiseTextureName;
		// indexを設定
		textureInfo_.noiseTextureIndex = asset_->GetTextureGPUIndex(noiseTextureName_);
	}
}

void ICPUParticleSpawnModule::ToCommonJson(Json& data) {

	const std::string key = "common";

	//============================================================================
	//	EmitParameters
	//============================================================================

	emitCount_.SaveJson(data[key], "emitCount");
	lifeTime_.SaveJson(data[key], "lifeTime");
	moveSpeed_.SaveJson(data[key], "moveSpeed");

	//============================================================================
	//	TextureParameters
	//============================================================================

	data[key]["textureName"] = textureName_;
	data[key]["noiseTextureName"] = noiseTextureName_;

	data[key]["samplerType"] = textureInfo_.samplerType;
	data[key]["useNoiseTexture"] = textureInfo_.useNoiseTexture;

	//============================================================================
	//	PrimitiveParameters
	//============================================================================

	data[key]["primitive"]["shape"] = EnumAdapter<ParticlePrimitiveType>::ToString(primitive_.type);

	switch (primitive_.type) {
	case ParticlePrimitiveType::Plane:

		data[key]["primitive"]["plane"]["size"] = primitive_.plane.size.ToJson();
		data[key]["primitive"]["plane"]["pivot"] = primitive_.plane.pivot.ToJson();
		data[key]["primitive"]["plane"]["mode"] = EnumAdapter<ParticlePlaneType>::ToString(planeType_);
		break;
	case ParticlePrimitiveType::Ring:

		data[key]["primitive"]["ring"]["divide"] = primitive_.ring.divide;
		data[key]["primitive"]["ring"]["outerRadius"] = primitive_.ring.outerRadius;
		data[key]["primitive"]["ring"]["innerRadius"] = primitive_.ring.innerRadius;
		break;
	case ParticlePrimitiveType::Cylinder:

		data[key]["primitive"]["cylinder"]["divide"] = primitive_.cylinder.divide;
		data[key]["primitive"]["cylinder"]["topRadius"] = primitive_.cylinder.topRadius;
		data[key]["primitive"]["cylinder"]["bottomRadius"] = primitive_.cylinder.bottomRadius;
		data[key]["primitive"]["cylinder"]["height"] = primitive_.cylinder.height;
		break;
	case ParticlePrimitiveType::Crescent:

		data[key]["primitive"]["crescent"]["divide"] = primitive_.crescent.divide;
		data[key]["primitive"]["crescent"]["uvMode"] = primitive_.crescent.uvMode;
		data[key]["primitive"]["crescent"]["outerRadius"] = primitive_.crescent.outerRadius;
		data[key]["primitive"]["crescent"]["innerRadius"] = primitive_.crescent.innerRadius;
		data[key]["primitive"]["crescent"]["startAngle"] = primitive_.crescent.startAngle;
		data[key]["primitive"]["crescent"]["endAngle"] = primitive_.crescent.endAngle;
		data[key]["primitive"]["crescent"]["lattice"] = primitive_.crescent.lattice;
		data[key]["primitive"]["crescent"]["thickness"] = primitive_.crescent.thickness;
		data[key]["primitive"]["crescent"]["pivot"] = primitive_.crescent.pivot.ToJson();
		data[key]["primitive"]["crescent"]["outerColor"] = primitive_.crescent.outerColor.ToJson();
		data[key]["primitive"]["crescent"]["innerColor"] = primitive_.crescent.innerColor.ToJson();
		break;
	}
}

void ICPUParticleSpawnModule::FromCommonJson(const Json& data) {

	const std::string key = "common";

	//============================================================================
	//	EmitParameters
	//============================================================================

	emitCount_.ApplyJson(data[key], "emitCount");
	lifeTime_.ApplyJson(data[key], "lifeTime");
	moveSpeed_.ApplyJson(data[key], "moveSpeed");

	//============================================================================
	//	TextureParameters
	//============================================================================

	textureName_ = data[key]["textureName"].get<std::string>();
	noiseTextureName_ = data[key]["noiseTextureName"].get<std::string>();

	// 存在していなければ読み込む
	if (!asset_->SearchTexture(textureName_)) {
		asset_->LoadTexture(textureName_, AssetLoadType::Async);
	}
	if (!asset_->SearchTexture(noiseTextureName_)) {
		asset_->LoadTexture(noiseTextureName_, AssetLoadType::Async);
	}

	textureInfo_.colorTextureIndex = asset_->GetTextureGPUIndex(textureName_);
	textureInfo_.noiseTextureIndex = asset_->GetTextureGPUIndex(noiseTextureName_);
	textureInfo_.samplerType = data[key]["samplerType"].get<int32_t>();
	textureInfo_.useNoiseTexture = data[key]["useNoiseTexture"].get<int32_t>();

	//============================================================================
	//	PrimitiveParameters
	//============================================================================

	const auto& primitive = data[key]["primitive"];
	const auto& shape = EnumAdapter<ParticlePrimitiveType>::FromString(primitive["shape"]);
	primitive_.type = shape.value();

	switch (primitive_.type) {
	case ParticlePrimitiveType::Plane: {

		primitive_.plane.size = primitive_.plane.size.FromJson(primitive["plane"]["size"]);
		primitive_.plane.pivot = primitive_.plane.pivot.FromJson(primitive["plane"]["pivot"]);

		const auto& planeType = EnumAdapter<ParticlePlaneType>::FromString(primitive["plane"]["mode"]);
		primitive_.plane.mode = static_cast<uint32_t>(planeType.value());
		break;
	}
	case ParticlePrimitiveType::Ring: {

		primitive_.ring.divide = primitive["ring"].value("divide", 8);
		primitive_.ring.outerRadius = primitive["ring"].value("outerRadius", 4.0f);
		primitive_.ring.innerRadius = primitive["ring"].value("innerRadius", 2.0f);
		break;
	}
	case ParticlePrimitiveType::Cylinder: {

		primitive_.cylinder.divide = primitive["cylinder"].value("divide", 8);
		primitive_.cylinder.topRadius = primitive["cylinder"].value("topRadius", 1.0f);
		primitive_.cylinder.bottomRadius = primitive["cylinder"].value("bottomRadius", 1.0f);
		primitive_.cylinder.height = primitive["cylinder"].value("height", 2.0f);
		break;
	}
	case ParticlePrimitiveType::Crescent: {

		primitive_.crescent.divide = primitive["crescent"].value("divide", 8);
		primitive_.crescent.uvMode = primitive["crescent"].value("uvMode", 1);
		primitive_.crescent.outerRadius = primitive["crescent"].value("outerRadius", 4.0f);
		primitive_.crescent.innerRadius = primitive["crescent"].value("innerRadius", 2.0f);
		primitive_.crescent.startAngle = primitive["crescent"].value("startAngle", pi / 6.0f);
		primitive_.crescent.endAngle = primitive["crescent"].value("endAngle", pi * 5.0f / 6.0f);
		primitive_.crescent.lattice = primitive["crescent"].value("lattice", 0.5f);
		primitive_.crescent.thickness = primitive["crescent"].value("thickness", 0.1f);
		primitive_.crescent.pivot = primitive_.crescent.pivot.FromJson(primitive["crescent"]["pivot"]);
		primitive_.crescent.outerColor = primitive_.crescent.outerColor.FromJson(primitive["crescent"]["outerColor"]);
		primitive_.crescent.innerColor = primitive_.crescent.innerColor.FromJson(primitive["crescent"]["innerColor"]);
		break;
	}
	}
}