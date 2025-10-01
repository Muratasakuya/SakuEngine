#include "AnimationSystem.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPoolManager.h>
#include <Engine/Object/Data/SkinnedAnimation.h>

//============================================================================
//	AnimationSystem classMethods
//============================================================================

Archetype AnimationSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<SkinnedAnimation>());
	return arch;
}

void AnimationSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	for (uint32_t object : ObjectPoolManager.View(Signature())) {

		auto* animation = ObjectPoolManager.GetData<SkinnedAnimation>(object);
		auto* transform = ObjectPoolManager.GetData<Transform3D>(object);
		animation->Update(transform->matrix.world);
	}
}