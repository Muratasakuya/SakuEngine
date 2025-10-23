#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/Sprite.h>

// front
class Transform2D;
class SpriteMaterial;

//============================================================================
//	structure
//============================================================================

struct SpriteData {

	Transform2D* transform;
	SpriteMaterial* material;
	Sprite* sprite;
};

//============================================================================
//	SpriteBufferSystem class
//	2Dスプライトのバッファを管理するシステム
//============================================================================
class SpriteBufferSystem :
	public ISystem {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SpriteBufferSystem() = default;
	~SpriteBufferSystem() = default;

	Archetype Signature() const override;

	void Update(ObjectPoolManager& ObjectPoolManager) override;

	//--------- accessor -----------------------------------------------------

	const std::vector<SpriteData>& GetSpriteData(SpriteLayer layer) { return spriteDataMap_[layer]; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unordered_map<SpriteLayer, std::vector<SpriteData>> spriteDataMap_;
};