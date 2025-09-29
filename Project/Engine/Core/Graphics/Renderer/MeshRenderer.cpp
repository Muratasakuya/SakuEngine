#include "MeshRenderer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/Core/Graphics/PostProcess/RenderTexture.h>
#include <Engine/Core/Graphics/PostProcess/DepthTexture.h>
#include <Engine/Core/Graphics/GPUObject/SceneConstBuffer.h>
#include <Engine/Core/Graphics/Context/MeshCommandContext.h>
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/InstancedMeshSystem.h>
#include <Engine/Object/System/Systems/SkyboxRenderSystem.h>
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
	MeshCommandContext commandContext{};

	const auto& meshes = system->GetMeshes();
	auto instancingBuffers = system->GetInstancingData();
	const auto& viewMap = system->GetRenderViewPerModel();

	if (meshes.empty()) {
		return;
	}
	if (!rayScene_->GetTLASResource()) {
		return;
	}

	// renderTextureへの描画処理
	// pipeline設定
	commandList->SetGraphicsRootSignature(meshShaderPipeline_->GetRootSignature());
	commandList->SetPipelineState(meshShaderPipeline_->GetGraphicsPipeline());

	// 共通のbuffer設定
	sceneBuffer->SetMainPassCommands(debugEnable, commandList);
	// allTexture
	commandList->SetGraphicsRootDescriptorTable(11, srvDescriptor_->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	// RayQuery
	// TLAS
	commandList->SetGraphicsRootShaderResourceView(8, rayScene_->GetTLASResource()->GetGPUVirtualAddress());
	// scene情報
	sceneBuffer->SetRaySceneCommand(commandList, 15);

	// skyboxがあるときのみ、とりあえず今は
	if (skyBoxSystem->IsCreated()) {

		// environmentTexture
		commandList->SetGraphicsRootDescriptorTable(12,
			srvDescriptor_->GetGPUHandle(skyBoxSystem->GetTextureIndex()));
	}

	for (const auto& [name, mesh] : meshes) {

		// 作成しきれていないメッシュをスキップ
		if (!system->IsReady(name)) {
			 continue;
		}
		// 描画先のビットが立っていなければ描画しない
		if (auto it = viewMap.find(name); it != viewMap.end()) {

			const uint8_t mask = static_cast<uint8_t>(it->second);
			const uint8_t want = static_cast<uint8_t>(debugEnable ? MeshRenderView::Scene : MeshRenderView::Game);
			if ((mask & want) == 0) {
				continue;
			}
		} else {
			continue;
		}

		// meshごとのmatrix設定
		commandList->SetGraphicsRootShaderResourceView(4,
			instancingBuffers[name].matrixBuffer.GetResource()->GetGPUVirtualAddress());

		for (uint32_t meshIndex = 0; meshIndex < mesh->GetMeshCount(); ++meshIndex) {

			// meshごとのmaterial、lighting設定
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

void MeshRenderer::BeginSkinnedTransition(bool debugEnable, uint32_t meshIndex, IMesh* mesh, DxCommand* dxCommand) {

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	if (!debugEnable) {

		// skinnedMeshなら頂点を読める状態にする
		if (mesh->IsSkinned()) {

			dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource() },
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
	}
#else
	// skinnedMeshなら頂点を読める状態にする
	if (mesh->IsSkinned()) {

		dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource() },
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}
#endif
}

void MeshRenderer::EndSkinnedTransition(bool debugEnable, uint32_t meshIndex, IMesh* mesh, DxCommand* dxCommand) {

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	if (debugEnable) {

		// skinnedMeshなら頂点を書き込み状態に戻す
		if (mesh->IsSkinned()) {

			dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource() },
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}
#else
	// skinnedMeshなら頂点を書き込み状態に戻す
	if (mesh->IsSkinned()) {

		dxCommand->TransitionBarriers({ static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource() },
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
#endif
}