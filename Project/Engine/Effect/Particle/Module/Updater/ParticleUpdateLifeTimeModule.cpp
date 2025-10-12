#include "ParticleUpdateLifeTimeModule.h"

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

}

Json ParticleUpdateLifeTimeModule::ToJson() {

	Json data;

	data["empty"] = "empty";

	return data;
}

void ParticleUpdateLifeTimeModule::FromJson(const Json& data) {

	data;
}