#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Object/Data/Sprite.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/MathLib/Matrix4x4.h>

// c++
#include <array>
#include <memory>
// front
class DxCommand;
class SceneConstBuffer;
class SRVDescriptor;
class DxShaderCompiler;

//============================================================================
//	SpriteRenderer class
//============================================================================
class SpriteRenderer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SpriteRenderer() = default;
	~SpriteRenderer() = default;

	void Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

	void Rendering(SpriteLayer layer, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unique_ptr<PipelineState> pipeline_;
};