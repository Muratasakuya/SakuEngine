#include "MeshCommandContext.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/Mesh/Mesh.h>

//============================================================================
//	MeshCommandContext classMethods
//============================================================================

void MeshCommandContext::DispatchMesh(ID3D12GraphicsCommandList6* commandList,
	UINT instanceCount, uint32_t meshIndex, IMesh* mesh) {

	// 処理するinstanceがない場合は早期リターン
	if (instanceCount == 0) {
		return;
	}

	// 頂点bufferはskinnedMeshかそうじゃないかで処理を変更する
	if (mesh->IsSkinned()) {

		commandList->SetGraphicsRootShaderResourceView(0,
			static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex).GetResource()->GetGPUVirtualAddress());
	} else {

		commandList->SetGraphicsRootShaderResourceView(0,
			static_cast<StaticMesh*>(mesh)->GetVertexBuffer(meshIndex).GetResource()->GetGPUVirtualAddress());
	}
	// その他のbufferを設定
	commandList->SetGraphicsRootShaderResourceView(1,
		mesh->GetUniqueVertexIndexBuffer(meshIndex).GetResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootShaderResourceView(2,
		mesh->GetMeshletBuffer(meshIndex).GetResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootShaderResourceView(3,
		mesh->GetPrimitiveIndexBuffer(meshIndex).GetResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(5,
		mesh->GetMeshInstanceData(meshIndex).GetResource()->GetGPUVirtualAddress());

	// threadGroupCountXの最大値
	const UINT maxThreadGroupCount = 65535;
	const UINT meshletCount = mesh->GetMeshletCount(meshIndex);

	// threadGroup数
	UINT totalThreadGroupCountX = meshletCount * instanceCount;

	// 最大数を超過した場合、分割して実行
	if (totalThreadGroupCountX > maxThreadGroupCount) {

		// 1度に実行できる最大instance数を計算
		UINT maxInstancesPerDispatch = maxThreadGroupCount / meshletCount;

		// 分割して実行
		UINT remainingInstances = instanceCount;
		while (remainingInstances > 0) {

			UINT dispatchInstanceCount = (std::min)(remainingInstances, maxInstancesPerDispatch);
			UINT dispatchThreadGroupCountX = dispatchInstanceCount * meshletCount;

			// 実行
			commandList->DispatchMesh(dispatchThreadGroupCountX, 1, 1);

			// 残りのinstance数を更新
			remainingInstances -= dispatchInstanceCount;
		}
	}
	// 超過しない場合はそのまま実行
	else {
		commandList->DispatchMesh(totalThreadGroupCountX, 1, 1);
	}
}