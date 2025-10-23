#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>

//============================================================================
//	Transform3DSystem class
//	3DTransformの更新システム
//============================================================================
class Transform3DSystem :
	public ISystem {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Transform3DSystem() = default;
	~Transform3DSystem() = default;

	Archetype Signature() const override;

	void Update(ObjectPoolManager& ObjectPoolManager) override;
};

//============================================================================
//	Transform2DSystem class
//	2DTransformの更新システム
//============================================================================
class Transform2DSystem :
	public ISystem {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Transform2DSystem() = default;
	~Transform2DSystem() = default;

	Archetype Signature() const override;

	void Update(ObjectPoolManager& ObjectPoolManager) override;
};