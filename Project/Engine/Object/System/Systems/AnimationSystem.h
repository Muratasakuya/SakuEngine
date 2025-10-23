#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>

//============================================================================
//	AnimationSystem class
//	アニメーションデータの更新
//============================================================================
class AnimationSystem :
	public ISystem {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	AnimationSystem() = default;
	~AnimationSystem() = default;

	Archetype Signature() const override;

	void Update(ObjectPoolManager& ObjectPoolManager) override;
};