#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Updater/Primitive/Interface/IParticlePrimitiveUpdater.h>

//============================================================================
//	BaseParticlePrimitiveUpdater class
//============================================================================
template <typename T>
class BaseParticlePrimitiveUpdater :
	public IParticlePrimitiveUpdater {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BaseParticlePrimitiveUpdater() = default;
	virtual ~BaseParticlePrimitiveUpdater() = default;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 補間値
	T start_;
	T target_;
};