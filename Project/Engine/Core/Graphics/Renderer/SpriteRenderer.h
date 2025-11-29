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
//	2Dスプライト描画を担当。ブレンド別パイプラインを切替えつつバッチ描画を行う。
//============================================================================
class SpriteRenderer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SpriteRenderer() = default;
	~SpriteRenderer() = default;

	// スプライト用パイプラインを作成し初期化
	void Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

	// 指定レイヤのスプライト群を描画
	// ポストエフェクト有効
	void ApplyPostProcessRendering(SpriteLayer layer, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
	// ポストエフェクト無効
	void IrrelevantPostProcessRendering(SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 描画モード
	enum class RenderMode {

		IrrelevantPostProcess, // ポストエフェクト無効
		ApplyPostProcess,      // ポストエフェクト有効

		Count
	};


	//--------- variables ----------------------------------------------------

	std::unordered_map<RenderMode, std::unique_ptr<PipelineState>> pipelines_;
};