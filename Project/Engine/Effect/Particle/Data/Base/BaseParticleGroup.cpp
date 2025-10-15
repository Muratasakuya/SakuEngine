#include "BaseParticleGroup.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/TagSystem.h>
#include <Engine/Effect/Particle/ParticleConfig.h>

// imgui
#include <imgui.h>

//============================================================================
//	BaseParticleGroup classMethods
//============================================================================

void BaseParticleGroup::SetParent(bool isSet, const BaseTransform& parent) {

	if (isSet) {

		parentTransform_ = &parent;
	} else {

		parentTransform_ = nullptr;
	}
}

D3D12_GPU_VIRTUAL_ADDRESS BaseParticleGroup::GetPrimitiveBufferAdress() const {

	switch (primitiveBuffer_.type) {
	case ParticlePrimitiveType::Plane: {

		return primitiveBuffer_.plane.GetResource()->GetGPUVirtualAddress();
	}
	case ParticlePrimitiveType::Ring: {

		return primitiveBuffer_.ring.GetResource()->GetGPUVirtualAddress();
	}
	case ParticlePrimitiveType::Cylinder: {

		return primitiveBuffer_.cylinder.GetResource()->GetGPUVirtualAddress();
	}
	case ParticlePrimitiveType::Crescent: {

		return primitiveBuffer_.crescent.GetResource()->GetGPUVirtualAddress();
	}
	case ParticlePrimitiveType::Count: {

		ASSERT(false, "ParticlePrimitiveType::Count is not buffer");
		return primitiveBuffer_.plane.GetResource()->GetGPUVirtualAddress();
	}
	}

	// フォロースルー
	ASSERT(false, "ParticlePrimitiveType::Count is not buffer");
	return primitiveBuffer_.plane.GetResource()->GetGPUVirtualAddress();
}

void BaseParticleGroup::CreatePrimitiveBuffer(ID3D12Device* device,
	ParticlePrimitiveType primitiveType, uint32_t maxParticle) {

	primitiveBuffer_.type = primitiveType;

	// 各形状で生成、この時点で転送してしまう
	switch (primitiveBuffer_.type) {
	case ParticlePrimitiveType::Plane: {

		PlaneForGPU plane{};
		plane.Init();

		std::vector<PlaneForGPU> planes(maxParticle, plane);
		primitiveBuffer_.plane.CreateSRVBuffer(device, maxParticle);
		primitiveBuffer_.plane.TransferData(planes);
		break;
	}
	case ParticlePrimitiveType::Ring: {

		RingForGPU ring{};
		ring.Init();

		std::vector<RingForGPU> rings(maxParticle, ring);
		primitiveBuffer_.ring.CreateSRVBuffer(device, maxParticle);
		primitiveBuffer_.ring.TransferData(rings);
		break;
	}
	case ParticlePrimitiveType::Cylinder: {

		CylinderForGPU cylinder{};
		cylinder.Init();

		std::vector<CylinderForGPU> cylinders(maxParticle, cylinder);
		primitiveBuffer_.cylinder.CreateSRVBuffer(device, maxParticle);
		primitiveBuffer_.cylinder.TransferData(cylinders);
		break;
	}
	case ParticlePrimitiveType::Crescent: {

		CrescentForGPU crescent{};
		crescent.Init();

		std::vector<CrescentForGPU> crescents(maxParticle, crescent);
		primitiveBuffer_.crescent.CreateSRVBuffer(device, maxParticle);
		primitiveBuffer_.crescent.TransferData(crescents);
		break;
	}
	}
}

void BaseParticleGroup::CreateTrailBuffer(ID3D12Device* device,
	ParticlePrimitiveType primitiveType, uint32_t maxParticle) {

	// トレイルに対応しているタイプでのみ作成
	if (primitiveType == ParticlePrimitiveType::Plane) {

		trailHeaderBuffer_.CreateSRVBuffer(device, maxParticle);
		trailVertexBuffer_.CreateSRVBuffer(device, maxParticle);
		trailTextureInfoBuffer_.CreateSRVBuffer(device, maxParticle);
	}
}

void BaseParticleGroup::DrawEmitter() {

	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	const uint32_t division = 8;
	const Color color = Color::Red(0.6f);

	Vector3 parentTranslation{};
	// 親の座標
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}

	// まだbufferが作成されていなければ作成する
	switch (emitter_.shape) {
	case ParticleEmitterShape::Sphere: {

		lineRenderer->DrawSphere(division, emitter_.sphere.radius,
			parentTranslation + emitter_.sphere.translation, color);
		break;
	}
	case ParticleEmitterShape::Hemisphere: {

		lineRenderer->DrawHemisphere(division, emitter_.hemisphere.radius,
			parentTranslation + emitter_.hemisphere.translation, emitter_.hemisphere.rotationMatrix, color);
		break;
	}
	case ParticleEmitterShape::Box: {

		lineRenderer->DrawOBB(parentTranslation + emitter_.box.translation,
			emitter_.box.size, emitter_.box.rotationMatrix, color);
		break;
	}
	case ParticleEmitterShape::Cone: {

		lineRenderer->DrawCone(division, emitter_.cone.baseRadius, emitter_.cone.topRadius,
			emitter_.cone.height, parentTranslation + emitter_.cone.translation, emitter_.cone.rotationMatrix, color);
		break;
	}
	}
}

void BaseParticleGroup::EditEmitter() {

	switch (emitter_.shape) {
	case ParticleEmitterShape::Sphere: {

		ImGui::DragFloat("radius", &emitter_.sphere.radius, 0.01f);
		ImGui::DragFloat3("translation", &emitter_.sphere.translation.x, 0.05f);

		// 動いていない間も座標は共有する
		emitter_.hemisphere.translation = emitter_.sphere.translation;
		emitter_.box.translation = emitter_.sphere.translation;
		emitter_.cone.translation = emitter_.sphere.translation;
		break;
	}
	case ParticleEmitterShape::Hemisphere: {

		ImGui::DragFloat("radius", &emitter_.hemisphere.radius, 0.01f);
		ImGui::DragFloat3("rotation", &emitterRotation_.x, 0.01f);
		ImGui::DragFloat3("translation", &emitter_.hemisphere.translation.x, 0.05f);

		// 動いていない間も座標は共有する
		emitter_.sphere.translation = emitter_.hemisphere.translation;
		emitter_.box.translation = emitter_.hemisphere.translation;
		emitter_.cone.translation = emitter_.hemisphere.translation;
		break;
	}
	case ParticleEmitterShape::Box: {

		ImGui::DragFloat3("size", &emitter_.box.size.x, 0.1f);
		ImGui::DragFloat3("rotation", &emitterRotation_.x, 0.01f);
		ImGui::DragFloat3("translation", &emitter_.box.translation.x, 0.05f);

		// 動いていない間も座標は共有する
		emitter_.sphere.translation = emitter_.box.translation;
		emitter_.hemisphere.translation = emitter_.box.translation;
		emitter_.cone.translation = emitter_.box.translation;
		break;
	}
	case ParticleEmitterShape::Cone: {

		ImGui::DragFloat("baseRadius", &emitter_.cone.baseRadius, 0.01f);
		ImGui::DragFloat("topRadius", &emitter_.cone.topRadius, 0.01f);
		ImGui::DragFloat("height", &emitter_.cone.height, 0.01f);
		ImGui::DragFloat3("rotation", &emitterRotation_.x, 0.01f);
		ImGui::DragFloat3("translation", &emitter_.cone.translation.x, 0.05f);

		// 動いていない間も座標は共有する
		emitter_.sphere.translation = emitter_.cone.translation;
		emitter_.hemisphere.translation = emitter_.cone.translation;
		emitter_.box.translation = emitter_.cone.translation;
		break;
	}
	}
}

bool BaseParticleGroup::ImGuiParent() {

	// 必要なシステム取得
	ObjectManager* objectManager = ObjectManager::GetInstance();
	TagSystem* tagSystem = objectManager->GetSystem<TagSystem>();
	const auto& tags = tagSystem->Tags();

	// すべてクリア
	parentIDs_.clear();
	parentNames_.clear();

	// 0番目に設定なしを入れる
	parentNames_.emplace_back("None");
	parentIDs_.push_back(0);

	for (const auto& [id, tag] : tags) {
		if (objectManager->GetData<Transform3D>(id)) {

			parentIDs_.push_back(id);
			parentNames_.emplace_back(tag->name);
		}
	}

	// 現在選択されているtransformの取得
	int current = 0;
	if (parentTransform_) {
		for (size_t i = 1; i < parentIDs_.size(); ++i) {
			if (objectManager->GetData<Transform3D>(parentIDs_[i]) == parentTransform_) {

				current = static_cast<int>(i);
				break;
			}
		}
	}

	// 親のリストをコンボ表示
	bool edit = ImGui::BeginCombo("##ParentCombo", parentNames_[current].c_str());
	if (edit) {
		for (int i = 0; i < static_cast<int>(parentNames_.size()); ++i) {

			bool selected = (current == i);
			if (ImGui::Selectable(parentNames_[i].c_str(), selected)) {

				current = i;
			}
			if (selected) {

				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// 親の設定
	if (current == 0) {

		SetParent(false, Transform3D());
	} else {

		SetParent(true, *objectManager->GetData<Transform3D>(parentIDs_[current]));
	}

	// Combo操作のリザルトを返す
	return edit;
}