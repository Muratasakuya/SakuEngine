#include "ParticleUpdateVelocityModule.h"

//============================================================================
//	ParticleUpdateVelocityModule classMethods
//============================================================================

void ParticleUpdateVelocityModule::Execute(
	CPUParticle::ParticleData& particle, float deltaTime) {

	particle.transform.translation += particle.velocity * deltaTime;
}

void ParticleUpdateVelocityModule::ImGui() {

	curveFloat_.EditSelectCurve();
	curveVec2_.EditSelectCurve();
	curveVec3_.EditSelectCurve();
	curveColor_.EditSelectCurve();
}