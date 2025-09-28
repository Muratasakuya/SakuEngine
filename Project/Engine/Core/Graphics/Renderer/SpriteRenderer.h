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

//============================================================================
//	enum class
//============================================================================

// 描画モード
enum class RenderMode {

	IrrelevantPostProcess,
	ApplyPostProcess,

	Count
};

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

	void Init(ID3D12Device8* device, class SRVDescriptor* srvDescriptor,
		class DxShaderCompiler* shaderCompiler);

	// postProcessをかける
	void ApplyPostProcessRendering(SpriteLayer layer, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
	// postProcessをかけない
	void IrrelevantRendering(SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unordered_map<RenderMode, std::unique_ptr<PipelineState>> pipelines_;
};