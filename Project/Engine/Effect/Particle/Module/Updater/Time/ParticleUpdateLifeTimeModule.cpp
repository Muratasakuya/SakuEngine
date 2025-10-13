#include "ParticleUpdateLifeTimeModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateLifeTimeModule classMethods
//============================================================================

void ParticleUpdateLifeTimeModule::Init() {

}

void ParticleUpdateLifeTimeModule::Execute(
	CPUParticle::ParticleData& particle, float deltaTime) {

	// ライフタイムを進める
	particle.currentTime += deltaTime;
	// 進捗を計算
	particle.progress = particle.currentTime / particle.lifeTime;
}

void ParticleUpdateLifeTimeModule::ImGui() {

	EnumAdapter<ParticleLifeEndMode>::Combo("EndMode", &endMode_);
}

Json ParticleUpdateLifeTimeModule::ToJson() {

	Json data;

	data["endMode_"] = EnumAdapter<ParticleLifeEndMode>::ToString(endMode_);

	return data;
}

void ParticleUpdateLifeTimeModule::FromJson(const Json& data) {

	endMode_ = EnumAdapter<ParticleLifeEndMode>::FromString(data.value("endMode_", "Advance")).value();
}