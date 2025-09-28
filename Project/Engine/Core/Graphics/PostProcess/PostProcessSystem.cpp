#include "PostProcessSystem.h"
#include "PostProcessSystem.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Context/PostProcessCommandContext.h>
#include <Engine/Core/Graphics/PostProcess/RenderTexture.h>
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBufferSize.h>
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Asset/Asset.h>
#include <Engine/Config.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	PostProcessSystem classMethods
//============================================================================

PostProcessSystem* PostProcessSystem::instance_ = nullptr;

PostProcessSystem* PostProcessSystem::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new PostProcessSystem();
	}
	return instance_;
}

void PostProcessSystem::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void PostProcessSystem::AddProcess(PostProcessType process) {

	// 追加
	if (Algorithm::Find(initProcesses_, process, true)) {

		activeProcesses_.push_back(process);
	}
}

void PostProcessSystem::RemoveProcess(PostProcessType process) {

	// 削除
	if (Algorithm::Find(activeProcesses_, process, true)) {

		activeProcesses_.erase(std::remove(activeProcesses_.begin(),
			activeProcesses_.end(), process), activeProcesses_.end());
	}
}

void PostProcessSystem::ClearProcess() {

	// 全て削除
	activeProcesses_.clear();
	activeProcesses_.push_back(PostProcessType::CopyTexture);
}

void PostProcessSystem::InputProcessTexture(
	const std::string& textureName, PostProcessType process) {

	// texture設定
	if (Algorithm::Find(initProcesses_, process, true)) {

		processors_[process]->SetProcessTexureGPUHandle(asset_->GetGPUHandle(textureName));
	}
}

void PostProcessSystem::Init(ID3D12Device8* device,
	DxShaderCompiler* shaderComplier, SRVDescriptor* srvDescriptor, Asset* asset) {

	width_ = Config::kWindowWidth;
	height_ = Config::kWindowHeight;

	device_ = nullptr;
	device_ = device;

	srvDescriptor_ = nullptr;
	srvDescriptor_ = srvDescriptor;

	asset_ = nullptr;
	asset_ = asset;

	// pipeline初期化
	pipeline_ = std::make_unique<PostProcessPipeline>();
	pipeline_->Init(device, srvDescriptor_, shaderComplier);

	// offscreenPipeline初期化
	offscreenPipeline_ = std::make_unique<PipelineState>();
	offscreenPipeline_->Create("CopyTexture.json", device, srvDescriptor_, shaderComplier);

	// copy用プロセス
	copyTextureProcess_ = std::make_unique<ComputePostProcessor>();
	copyTextureProcess_->Init(device_, srvDescriptor_, width_, height_);
}

void PostProcessSystem::Create(const std::vector<PostProcessType>& processes) {

	if (!initProcesses_.empty() || processes.empty()) {
		return;
	}

	// 使用可能なprocessを入れる
	initProcesses_.assign(processes.begin(), processes.end());

	initProcesses_.push_back(PostProcessType::CopyTexture);
	activeProcesses_.push_back(PostProcessType::CopyTexture);

	// 使用できるPostProcessを初期化する
	for (const auto& process : initProcesses_) {

		// processor作成
		processors_[process] = std::make_unique<ComputePostProcessor>();
		processors_[process]->Init(device_, srvDescriptor_, width_, height_);

		// buffer作成
		CreateCBuffer(process);

		// pipeline作成
		pipeline_->Create(process);

		if (process == PostProcessType::DepthBasedOutline) {

			processors_[process]->SetProcessTexureGPUHandle(depthFrameBurferGPUHandle_);
		}
	}
}

void PostProcessSystem::Update(SceneView* sceneView) {

	// scene情報が必要な場合のみ
	if (Algorithm::Find(activeProcesses_, PostProcessType::DepthBasedOutline, false)) {

		auto* buffer = static_cast<PostProcessBuffer<DepthBasedOutlineForGPU>*>(
			buffers_[PostProcessType::DepthBasedOutline].get());

		const auto& parameter = buffer->GetParameter();

		// 渡す値の設定
		DepthBasedOutlineForGPU parama{};
		parama.projectionInverse = Matrix4x4::Inverse(sceneView->GetCamera()->GetProjectionMatrix());
		parama.edgeScale = parameter.edgeScale;
		parama.threshold = parameter.threshold;
		parama.color = parameter.color;

		buffer->SetParameter(&parama, sizeof(DepthBasedOutlineForGPU));
	}
}

void PostProcessSystem::Execute(const D3D12_GPU_DESCRIPTOR_HANDLE& inputSRVGPUHandle, DxCommand* dxCommand) {

	// なにもプロセスがない場合はα値がバグらないようにcopyを行う
	if (activeProcesses_.empty()) {

		activeProcesses_.push_back(PostProcessType::CopyTexture);
	}

	auto commandList = dxCommand->GetCommandList();
	PostProcessCommandContext commandContext{};
	// 入力画像のGPUHandle取得
	D3D12_GPU_DESCRIPTOR_HANDLE inputGPUHandle = inputSRVGPUHandle;

	for (const auto& process : activeProcesses_) {

		if (processors_.find(process) != processors_.end()) {

			// pipeline設定
			pipeline_->SetPipeline(commandList, process);
			// buffer設定
			ExecuteCBuffer(commandList, process);
			// 実行
			commandContext.Execute(process, commandList, processors_[process].get(), inputGPUHandle);

			BeginTransition(process, dxCommand);

			// 出力を次のpostProcessの入力にする
			inputGPUHandle.ptr = NULL;
			inputGPUHandle = processors_[process]->GetSRVGPUHandle();
		}
	}

	// 最終的なframeBufferに設定するGPUHandleの設定
	frameBufferGPUHandle_ = inputGPUHandle;
}

void PostProcessSystem::BeginTransition(PostProcessType process, DxCommand* dxCommand) {

	if (process == activeProcesses_.back()) {

		// 最後の処理はframeBufferに描画するためにPixelShaderに遷移させる
		// UnorderedAccess -> PixelShader
		dxCommand->TransitionBarriers({ processors_[process]->GetOutputTextureResource() },
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	} else {

		// UnorderedAccess -> ComputeShader
		dxCommand->TransitionBarriers({ processors_[process]->GetOutputTextureResource() },
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
}

void PostProcessSystem::ExecuteDebugScene(
	const D3D12_GPU_DESCRIPTOR_HANDLE& inputSRVGPUHandle, DxCommand* dxCommand) {

	auto commandList = dxCommand->GetCommandList();
	PostProcessCommandContext commandContext{};

	// pipeline設定
	pipeline_->SetPipeline(commandList, PostProcessType::CopyTexture);
	// 実行
	commandContext.Execute(PostProcessType::CopyTexture, commandList, copyTextureProcess_.get(), inputSRVGPUHandle);

	// UnorderedAccess -> PixelShader
	dxCommand->TransitionBarriers({ copyTextureProcess_->GetOutputTextureResource() },
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void PostProcessSystem::RenderFrameBuffer(DxCommand* dxCommand) {

	auto commandList = dxCommand->GetCommandList();

	// frameBufferへの描画
	commandList->SetGraphicsRootSignature(offscreenPipeline_->GetRootSignature());
	commandList->SetPipelineState(offscreenPipeline_->GetGraphicsPipeline());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(0, frameBufferGPUHandle_);

	const UINT vertexCount = 3;
	commandList->DrawInstanced(vertexCount, 1, 0, 0);
}

void PostProcessSystem::ImGui() {

	if (needSort_) {

		std::sort(activeProcesses_.begin(), activeProcesses_.end());
		needSort_ = false;
	}

	ImGui::SetWindowFontScale(0.8f);

	using EA = EnumAdapter<PostProcessType>;

	if (ImGui::BeginChild("##checklist", ImVec2(240.0f, 0.0f), true)) {
		ImGui::TextDisabled("Available");
		for (auto process : initProcesses_) {

			bool enabled = Algorithm::Find(activeProcesses_, process, false);
			if (ImGui::Checkbox(EA::ToString(process), &enabled)) {

				enabled ? AddProcess(process) : RemoveProcess(process);
				// 選択インデックスの整合を取る
				if (!enabled && selectedProcessIndex_ >= static_cast<int>(activeProcesses_.size())) {

					selectedProcessIndex_ = -1;
				}
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	if (ImGui::BeginChild("##activelist", ImVec2(240.0f, 0.0f), true)) {

		ImGui::TextDisabled("ActiveProcessList");
		for (int i = 0; i < activeProcesses_.size(); ++i) {

			PostProcessType process = activeProcesses_[i];
			bool isSel = (selectedProcessIndex_ == (int)i);

			ImGui::Selectable(EA::ToString(process), isSel,
				ImGuiSelectableFlags_DontClosePopups);

			// 選択
			if (ImGui::IsItemClicked()) {

				selectedProcessIndex_ = i;
			}

			// Drag & Drop
			if (ImGui::BeginDragDropSource()) {

				ImGui::SetDragDropPayload("POST_PROCESS_IDX", &i, sizeof(int));
				ImGui::TextUnformatted(EA::ToString(process));
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload("POST_PROCESS_IDX")) {

					size_t src = *static_cast<const size_t*>(pl->Data);
					size_t dst = i;
					if (src != dst) {

						// 入れ替え処理
						std::swap(activeProcesses_[src], activeProcesses_[dst]);
						needSort_ = true;
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// パラメータ操作
	ImGui::BeginChild("##params", ImVec2(0.0f, 0.0f), true);
	if (selectedProcessIndex_ >= 0 && selectedProcessIndex_ < static_cast<int>(activeProcesses_.size())) {

		PostProcessType process = activeProcesses_[selectedProcessIndex_];
		// buffer があるタイプだけ
		if (auto it = buffers_.find(process); it != buffers_.end()) {

			it->second->ImGui();
		} else {

			ImGui::TextDisabled("No parameters");
		}
	}
	ImGui::EndChild();

	ImGui::SetWindowFontScale(1.0f);
}

void PostProcessSystem::ToWrite(DxCommand* dxCommand) {

	if (activeProcesses_.empty()) {
		return;
	}

	for (const auto& process : activeProcesses_) {
		if (process == activeProcesses_.back()) {

			// PixelShader -> UnorderedAccess
			dxCommand->TransitionBarriers(
				{ processors_[process]->GetOutputTextureResource() },
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		} else {

			// ComputeShader -> UnorderedAccess
			dxCommand->TransitionBarriers(
				{ processors_[process]->GetOutputTextureResource() },
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// PixelShader -> UnorderedAccess
	dxCommand->TransitionBarriers(
		{ copyTextureProcess_->GetOutputTextureResource() },
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
#endif
}

void PostProcessSystem::CreateCBuffer(PostProcessType type) {

	switch (type) {
	case PostProcessType::Bloom: {

		auto buffer = std::make_unique<PostProcessBuffer<BloomForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::HorizontalBlur: {

		auto buffer = std::make_unique<PostProcessBuffer<HorizonBlurForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::VerticalBlur: {

		auto buffer = std::make_unique<PostProcessBuffer<VerticalBlurForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::RadialBlur: {

		auto buffer = std::make_unique<PostProcessBuffer<RadialBlurForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::GaussianFilter: {

		auto buffer = std::make_unique<PostProcessBuffer<GaussianFilterForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::BoxFilter: {
		// bufferなし
		break;
	}
	case PostProcessType::Dissolve: {

		auto buffer = std::make_unique<PostProcessBuffer<DissolveForGPU>>();
		buffer->Init(device_, 3);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::Random: {

		auto buffer = std::make_unique<PostProcessBuffer<RandomForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::Vignette: {

		auto buffer = std::make_unique<PostProcessBuffer<VignetteForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::Grayscale: {
		// bufferなし
		break;
	}
	case PostProcessType::SepiaTone: {
		// bufferなし
		break;
	}
	case PostProcessType::LuminanceBasedOutline: {

		auto buffer = std::make_unique<PostProcessBuffer<LuminanceBasedOutlineForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::DepthBasedOutline: {

		auto buffer = std::make_unique<PostProcessBuffer<DepthBasedOutlineForGPU>>();
		buffer->Init(device_, 3);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::Lut: {

		auto buffer = std::make_unique<PostProcessBuffer<LutForGPU>>();
		buffer->Init(device_, 3);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::Glitch: {

		auto buffer = std::make_unique<PostProcessBuffer<GlitchForGPU>>();
		buffer->Init(device_, 3);

		buffers_[type] = std::move(buffer);
		break;
	}
	case PostProcessType::CRTDisplay: {

		auto buffer = std::make_unique<PostProcessBuffer<CRTDisplayForGPU>>();
		buffer->Init(device_, 2);

		buffers_[type] = std::move(buffer);
		break;
	}
	}
}

void PostProcessSystem::ExecuteCBuffer(
	ID3D12GraphicsCommandList* commandList, PostProcessType type) {

	if (Algorithm::Find(buffers_, type)) {

		// buffer更新
		buffers_[type]->Update();

		UINT rootIndex;
		D3D12_GPU_VIRTUAL_ADDRESS adress{};

		rootIndex = buffers_[type]->GetRootIndex();
		adress = buffers_[type]->GetResource()->GetGPUVirtualAddress();

		commandList->SetComputeRootConstantBufferView(rootIndex, adress);
	}
}