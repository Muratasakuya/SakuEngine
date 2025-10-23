#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/Skybox.h>

// directX
#include <d3d12.h>

//============================================================================
//	SkyboxRenderSystem class
//	スカイボックスの描画を行うシステム
//============================================================================
class SkyboxRenderSystem :
	public ISystem {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SkyboxRenderSystem() = default;
	~SkyboxRenderSystem() = default;

	Archetype Signature() const override;

	void Update(ObjectPoolManager& ObjectPoolManager) override;

	void Render(ID3D12GraphicsCommandList* commandList);

	//--------- accessor -----------------------------------------------------

	bool IsCreated() const { return isCreated_; }

	uint32_t GetTextureIndex() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const Skybox* data_;

	bool isCreated_;
};