#include "SpriteRenderer.h"

//============================================================================
//	include
//============================================================================
// Graphics
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/GPUObject/SceneConstBuffer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/Data/Material.h>
#include <Engine/Object/System/Systems/SpriteBufferSystem.h>

//============================================================================
//	SpriteRenderer classMethods
//============================================================================

void SpriteRenderer::Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor,
	DxShaderCompiler* shaderCompiler) {

	// pipeline作成
	pipeline_ = std::make_unique<PipelineState>();
	pipeline_->Create("Object2D.json",
		device, srvDescriptor, shaderCompiler);
}

void SpriteRenderer::Rendering(SpriteLayer layer,
	SceneConstBuffer* sceneBuffer, DxCommand* dxCommand) {

	// 描画情報取得
	const auto& system = ObjectManager::GetInstance()->GetSystem<SpriteBufferSystem>();
	const auto& spriteData = system->GetSpriteData(layer);

	if (spriteData.empty()) {
		return;
	}

	// commandList取得
	ID3D12GraphicsCommandList* commandList = dxCommand->GetCommandList();

	// frameBufferへの描画処理
	// pipeline設定
	commandList->SetGraphicsRootSignature(pipeline_->GetRootSignature());
	commandList->SetPipelineState(pipeline_->GetGraphicsPipeline());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// lightViewProjection
	sceneBuffer->SetOrthoProCommand(commandList, 3);
	// index
	commandList->IASetIndexBuffer(&spriteData.front().sprite->GetIndexBuffer().GetIndexBufferView());

	std::optional<BlendMode> currentBlendMode = std::nullopt;
	for (const auto& buffer : spriteData) {

		// 別のブレンドが設定されていればpipelineを再設定する
		const auto blendMode = buffer.sprite->GetBlendMode();
		if (!currentBlendMode || blendMode != currentBlendMode.value()) {

			commandList->SetPipelineState(pipeline_->GetGraphicsPipeline(blendMode));
			currentBlendMode = blendMode;
		}

		// vertex
		commandList->IASetVertexBuffers(0, 1, &buffer.sprite->GetVertexBuffer().GetVertexBufferView());

		// texture
		commandList->SetGraphicsRootDescriptorTable(0, buffer.sprite->GetTextureGPUHandle());
		// alphaTexture
		if (buffer.sprite->UseAlphaTexture()) {

			commandList->SetGraphicsRootDescriptorTable(1, buffer.sprite->GetAlphaTextureGPUHandle());
		}
		// matrix
		commandList->SetGraphicsRootConstantBufferView(2, buffer.transform->GetBuffer().GetResource()->GetGPUVirtualAddress());
		// material
		commandList->SetGraphicsRootConstantBufferView(4, buffer.material->GetBuffer().GetResource()->GetGPUVirtualAddress());

		// 描画処理
		commandList->DrawIndexedInstanced(Sprite::GetIndexNum(), 1, 0, 0, 0);
	}
}