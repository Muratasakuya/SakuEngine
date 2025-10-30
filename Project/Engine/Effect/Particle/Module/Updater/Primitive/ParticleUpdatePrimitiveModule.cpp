#include "ParticleUpdatePrimitiveModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

// updaters
#include <Engine/Effect/Particle/Module/Updater/Primitive/Derived/ParticlePlaneUpdater.h>
#include <Engine/Effect/Particle/Module/Updater/Primitive/Derived/ParticleRingUpdater.h>
#include <Engine/Effect/Particle/Module/Updater/Primitive/Derived/ParticleCylinderUpdater.h>
#include <Engine/Effect/Particle/Module/Updater/Primitive/Derived/ParticleCrescentUpdater.h>
#include <Engine/Effect/Particle/Module/Updater/Primitive/Derived/ParticleLightningUpdater.h>

//============================================================================
//	ParticleUpdatePrimitiveModule classMethods
//============================================================================

void ParticleUpdatePrimitiveModule::SetCommand(const ParticleCommand& command) {

	// updaterにコマンドを送る
	for (const auto& updater : updaters_) {

		updater->SetCommand(command);
	}
}

void ParticleUpdatePrimitiveModule::Init() {

	// 初期化値、全ての形状を初期化
	// Plane
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Plane)] = std::make_unique<ParticlePlaneUpdater>();
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Plane)]->Init();
	// Ring
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Ring)] = std::make_unique<ParticleRingUpdater>();
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Ring)]->Init();
	// Cylinder
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Cylinder)] = std::make_unique<ParticleCylinderUpdater>();
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Cylinder)]->Init();
	// Crescent
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Crescent)] = std::make_unique<ParticleCrescentUpdater>();
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Crescent)]->Init();
	// Lightning
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Lightning)] = std::make_unique<ParticleLightningUpdater>();
	updaters_[static_cast<uint32_t>(ParticlePrimitiveType::Lightning)]->Init();
}

void ParticleUpdatePrimitiveModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// 形状別で更新
	type_ = particle.primitive.type;
	if (const auto& updater = updaters_[static_cast<uint32_t>(type_)]) {

		updater->Update(particle, easingType_);
	}
}

void ParticleUpdatePrimitiveModule::ImGui() {

	Easing::SelectEasingType(easingType_, GetName());
	if (const auto& updater = updaters_[static_cast<uint32_t>(type_)]) {

		updater->ImGui();
	}
}

Json ParticleUpdatePrimitiveModule::ToJson() {

	Json data;
	data["easingType"] = EnumAdapter<EasingType>::ToString(easingType_);
	for (const auto& updater : updaters_) {

		updater->ToJson(data);
	}
	return data;
}

void ParticleUpdatePrimitiveModule::FromJson(const Json& data) {

	// イージング
	const auto& easingType = EnumAdapter<EasingType>::FromString(data.value("easingType", "EaseInSine"));
	easingType_ = easingType.value();

	for (const auto& updater : updaters_) {

		updater->FromJson(data);
	}
}