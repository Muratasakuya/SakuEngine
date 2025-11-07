#include "MeshRenderer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/Core/Graphics/PostProcess/Texture/RenderTexture.h>
#include <Engine/Core/Graphics/PostProcess/Texture/DepthTexture.h>
#include <Engine/Core/Graphics/GPUObject/SceneConstBuffer.h>
#include <Engine/Core/Graphics/Context/MeshCommandContext.h>
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/InstancedMeshSystem.h>
#include <Engine/Config.h>

//============================================================================
//	MeshRenderer classMethods
//============================================================================

void MeshRenderer::Init(ID3D12Device8* device, DxShaderCompiler* shaderCompiler, SRVDescriptor* srvDescriptor) {

	srvDescriptor_ = nullptr;
	srvDescriptor_ = srvDescriptor;

	meshShaderPipeline_ = std::make_unique<PipelineState>();
	meshShaderPipeline_->Create("MeshStandard.json", device, srvDescriptor, shaderCompiler);

	// skybox用pipeline作成
	skyboxPipeline_ = std::make_unique<PipelineState>();
	skyboxPipeline_->Create("Skybox.json", device, srvDescriptor, shaderCompiler);

	// rayScene初期化
	rayScene_ = std::make_unique<RaytracingScene>();
	rayScene_->Init(device);
}

void MeshRenderer::UpdateRayScene(DxCommand* dxCommand) {

	// commandList取得
	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	// 描画情報取得
	const auto& system = ObjectManager::GetInstance()->GetSystem<InstancedMeshSystem>();

	const auto& meshes = system->GetMeshes();
	auto instancingBuffers = system->GetInstancingData();
	if (meshes.empty()) {
		return;
	}

	// TLAS更新処理
	std::vector<IMesh*> meshPtrs;
	meshPtrs.reserve(meshes.size());
	for (const auto& [name, mesh] : meshes) {

		// 作成しきれていないメッシュをスキップ
		if (!system->IsReady(name)) {
			continue;
		}
		meshPtrs.emplace_back(mesh.get());

		// BLASに渡す前に頂点を遷移
		if (mesh->IsSkinned()) {
			for (uint32_t meshIndex = 0; meshIndex < mesh->GetMeshCount(); ++meshIndex) {

				dxCommand->TransitionBarriers(
					{ static_cast<SkinnedMesh*>(mesh.get())->GetOutputVertexBuffer(meshIndex).GetResource() },
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				);
			}
		}
	}

	// BLAS更新
	rayScene_->BuildBLASes(commandList, meshPtrs);
	std::vector<RayTracingInstance> rtInstances = system->CollectRTInstances(rayScene_.get());
	// TLAS更新
	rayScene_->BuildTLAS(commandList, rtInstances);
}

void MeshRenderer::Rendering(bool debugEnable, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand) {

	// commandList取得
	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	const auto& skyBoxSystem = ObjectManager::GetInstance()->GetSystem<SkyboxRenderSystem>();
	if (skyBoxSystem->IsCreated()) {

		// skybox描画
		// pipeline設定
		commandList->SetGraphicsRootSignature(skyboxPipeline_->GetRootSignature());
		commandList->SetPipelineState(skyboxPipeline_->GetGraphicsPipeline());

		// viewPro
		sceneBuffer->SetViewProCommand(debugEnable, commandList, 1);
		// texture
		commandList->SetGraphicsRootDescriptorTable(2,
			srvDescriptor_->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		skyBoxSystem->Render(commandList);
	}

	// 描画情報取得
	const auto& system = ObjectManager::GetInstance()->GetSystem<InstancedMeshSystem>();
	const auto& meshes = system->GetMeshes();
	auto instancingBuffers = system->GetInstancingData();
	const auto& renderData = system->GetRenderData();

	// TLASが作成されていなければ描画しない
	if (meshes.empty() || !rayScene_->GetTLASResource()) {
		return;
	}

	// 絶対に被らないブレンドモードで初期化
	BlendMode currentBlendMode = BlendMode::kBlendModeCount;
	MeshCommandContext commandContext{};
	for (const auto& [name, mesh] : meshes) {

		// 作成しきれていないメッシュをスキップ
		if (!system->IsReady(name)) {
			continue;
		}
		// 描画先のビットが立っていなければ描画しない
		if (auto it = renderData.find(name); it != renderData.end()) {

			const uint8_t mask = static_cast<uint8_t>(it->second.renderView);
			const uint8_t want = static_cast<uint8_t>(debugEnable ? MeshRenderView::Scene : MeshRenderView::Game);
			// ビットが被っていなければ描画しない
			if ((mask & want) == 0) {
				continue;
			}

			// 違うブレンドモードならパイプラインを再設定
			if (currentBlendMode != it->second.blendMode) {

				// ブレンドモード更新
				currentBlendMode = it->second.blendMode;
				// パイプライン設定
				SetPipeline(debugEnable, *skyBoxSystem, sceneBuffer, commandList, currentBlendMode);
			}
		}

		// 行列バッファ設定
		commandList->SetGraphicsRootShaderResourceView(4,
			instancingBuffers[name].matrixBuffer.GetResource()->GetGPUVirtualAddress());

		// マルチメッシュ描画
		for (uint32_t meshIndex = 0; meshIndex < mesh->GetMeshCount(); ++meshIndex) {

			// マテリアル、ライティング設定
			commandList->SetGraphicsRootShaderResourceView(9,
				instancingBuffers[name].materialsBuffer[meshIndex].GetResource()->GetGPUVirtualAddress());
			commandList->SetGraphicsRootShaderResourceView(10,
				instancingBuffers[name].lightingBuffer[meshIndex].GetResource()->GetGPUVirtualAddress());

			// 状態遷移前処理
			BeginSkinnedTransition(debugEnable, meshIndex, mesh.get(), dxCommand);

			// 描画処理
			commandContext.DispatchMesh(commandList, instancingBuffers[name].numInstance, meshIndex, mesh.get());

			// 状態遷移後処理
			EndSkinnedTransition(debugEnable, meshIndex, mesh.get(), dxCommand);
		}
	}
}

void MeshRenderer::SetPipeline(bool debugEnable, const SkyboxRenderSystem& skybox,
	SceneConstBuffer* sceneBuffer, ID3D12GraphicsCommandList6* commandList, BlendMode blendMode) {

	// パイプラインのセット
	commandList->SetGraphicsRootSignature(meshShaderPipeline_->GetRootSignature());
	commandList->SetPipelineState(meshShaderPipeline_->GetGraphicsPipeline(blendMode));

	// 共通のバッファ設定
	sceneBuffer->SetMainPassCommands(debugEnable, commandList);
	// SRVのセット
	commandList->SetGraphicsRootDescriptorTable(11, srvDescriptor_->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	// レイインスタンスバッファのセット(TLAS)
	commandList->SetGraphicsRootShaderResourceView(8, rayScene_->GetTLASResource()->GetGPUVirtualAddress());
	sceneBuffer->SetRaySceneCommand(commandList, 15);

	// skyboxが作成済みの場合のみ
	if (skybox.IsCreated()) {

		// 環境テクスチャ
		commandList->SetGraphicsRootDescriptorTable(12, srvDescriptor_->GetGPUHandle(skybox.GetTextureIndex()));
	}
}

void MeshRenderer::BeginSkinnedTransition(bool debugEnable, uint32_t meshIndex, IMesh* mesh, DxCommand* dxCommand) {

	// skinnedMeshなら頂点を読める状態にする
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	if (!debugEnable) {
		if (mesh->IsSkinned()) {

			dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource() },
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
	}
#else
	if (mesh->IsSkinned()) {

		dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource() },
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
#endif
}

void MeshRenderer::EndSkinnedTransition(bool debugEnable, uint32_t meshIndex, IMesh* mesh, DxCommand* dxCommand) {

	// skinnedMeshなら頂点を書き込み状態に戻す
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	if (debugEnable) {
		if (mesh->IsSkinned()) {

			dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource() },
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}
#else
	if (mesh->IsSkinned()) {

		dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource() },
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
#endif
}