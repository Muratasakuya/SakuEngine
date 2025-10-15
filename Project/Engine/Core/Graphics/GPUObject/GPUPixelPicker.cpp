#include "GPUPixelPicker.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Scene/SceneView.h>

//============================================================================
//	GPUPixelPicker classMethods
//============================================================================

void GPUPixelPicker::Init(ID3D12Device8* device,
	DxShaderCompiler* shaderCompiler, SRVDescriptor* srvDescriptor) {

	// pipeline作成
	pipeline_ = std::make_unique<PipelineState>();
	pipeline_->Create("GPUPixelPicker.json", device, srvDescriptor, shaderCompiler);

	// buffer作成
	// id取得用
	const UINT kMaxIDCount = 1;
	outputIDBuffer_.CreateUAVBuffer(device, kMaxIDCount);
	// ピッキング処理用データ
	pickingBuffer_.CreateBuffer(device);
	readbackIDBuffer_.CreateBuffer(device);
	pickedID_ = 0;
}

void GPUPixelPicker::Update(SceneView* sceneView,
	const Vector2& textureSize, const Vector2& input) {

	BaseCamera* camera = sceneView->GetSceneCamera();

	// ピッキング処理用データの更新
	pickingData_.inputPixelX = static_cast<uint32_t>(input.x);
	pickingData_.inputPixelY = static_cast<uint32_t>(input.y);
	pickingData_.textureWidth = static_cast<uint32_t>(textureSize.x);
	pickingData_.textureHeight = static_cast<uint32_t>(textureSize.y);
	pickingData_.cameraWorldPos = camera->GetTransform().GetWorldPos();
	pickingData_.inverseViewProjection = Matrix4x4::Inverse(camera->GetViewProjectionMatrix());
	// 定数値
	pickingData_.rayMax = 10000.0f;

	// buffer転送
	pickingBuffer_.TransferData(pickingData_);
}

void GPUPixelPicker::Execute(DxCommand* dxCommand, ID3D12Resource* tlasResource) {

	if (tlasResource == nullptr || !ImGuiObjectEditor::GetInstance()->IsPickActive()) {
		return;
	}

	// commandList取得
	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	// pipeline設定
	commandList->SetComputeRootSignature(pipeline_->GetRootSignature());
	commandList->SetPipelineState(pipeline_->GetComputePipeline());

	// RayQuery
	// TLAS
	commandList->SetComputeRootShaderResourceView(0, tlasResource->GetGPUVirtualAddress());
	// outputID
	commandList->SetComputeRootUnorderedAccessView(1,
		outputIDBuffer_.GetResource()->GetGPUVirtualAddress());
	// pickingData
	commandList->SetComputeRootConstantBufferView(2,
		pickingBuffer_.GetResource()->GetGPUVirtualAddress());
	// 実行
	commandList->Dispatch(1, 1, 1);

	// UAVバリア
	dxCommand->UAVBarrier(outputIDBuffer_.GetResource());

	// リソースのコピー処理、CPUで扱えるようにする
	CopyReadbackResource(dxCommand);

	// エディターに通知
	SetPickObject();
}

void GPUPixelPicker::CopyReadbackResource(DxCommand* dxCommand) {

	// Compute -> Copy
	dxCommand->TransitionBarriers({ outputIDBuffer_.GetResource() },
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	// コピー
	dxCommand->GetCommandList()->CopyResource(readbackIDBuffer_.GetResource(), outputIDBuffer_.GetResource());

	// Copy -> Compute
	dxCommand->TransitionBarriers({ outputIDBuffer_.GetResource() },
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// IDをCPUで扱えるようにする
	pickedID_ = readbackIDBuffer_.GetReadbackData();
}

void GPUPixelPicker::SetPickObject() {

	if (pickedID_ != 0u && !ImGuiObjectEditor::GetInstance()->IsUsingGuizmo()) {

		ImGuiObjectEditor::GetInstance()->SelectById(pickedID_);
	}
}