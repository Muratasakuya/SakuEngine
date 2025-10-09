#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/PostProcessType.h>
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBuffer.h>
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/PostProcessUpdaterBase.h>
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>
#include <Engine/Core/Graphics/PostProcess/Core/ComputePostProcessor.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessPipeline.h>
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Utility/Helper/Algorithm.h>

// c++
#include <vector>
#include <unordered_map>
// front
class SRVDescriptor;
class SceneView;
class DxCommand;
class Asset;

//============================================================================
//	PostProcessSystem class
//============================================================================
class PostProcessSystem :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void Init(ID3D12Device8* device, class DxShaderCompiler* shaderComplier,
		SRVDescriptor* srvDescriptor, Asset* asset, SceneView* sceneView);

	void Update();

	// postProcess作成
	void Create(const std::vector<PostProcessType>& processes);

	// postProcess実行
	void Execute(DxCommand* dxCommand,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputSRVGPUHandle,      // 色
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputMaskSRVGPUHandle); // ポストエフェクトマスク
	void ExecuteDebugScene(DxCommand* dxCommand,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputSRVGPUHandle,      // 色
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputMaskSRVGPUHandle); // ポストエフェクトマスク

	// 最終的なtextureをframeBufferに描画する
	void RenderFrameBuffer(DxCommand* dxCommand);

	// imgui
	void ImGui() override;

	void ToWrite(DxCommand* dxCommand);

	//--------- accessor -----------------------------------------------------

	// processの追加、削除
	void AddProcess(PostProcessType process);
	void RemoveProcess(PostProcessType process);
	void ClearProcess();

	// postProcessに使うtextureの設定
	void InputProcessTexture(const std::string& textureName, PostProcessType process);

	// buffer更新クラスの登録
	void RegisterUpdater(std::unique_ptr<PostProcessUpdaterBase> updater);
	void RemoveUpdater(PostProcessType type);

	// 更新クラスの呼び出し
	void Start(PostProcessType type);
	void Stop(PostProcessType type);
	void Reset(PostProcessType type);

	template <typename T>
	void SetParameter(const T& parameter, PostProcessType process);
	void SetDepthFrameBufferGPUHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& handle) { depthFrameBurferGPUHandle_ = handle; }

	PostProcessPipeline* GetPipeline() const { return pipeline_.get(); }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetCopySRVGPUHandle() const { return copyTextureProcess_->GetSRVGPUHandle(); }

	template <typename T>
	T* GetUpdater(PostProcessType type) const;

	// singleton
	static PostProcessSystem* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//- dependencies variables -----------------------------------------------

	uint32_t width_;
	uint32_t height_;
	ID3D12Device* device_;
	SRVDescriptor* srvDescriptor_;
	Asset* asset_;
	SceneView* sceneView_;

	//--------- variables ----------------------------------------------------

	static PostProcessSystem* instance_;

	// pipeline
	std::unique_ptr<PostProcessPipeline> pipeline_;
	std::unique_ptr<PipelineState> offscreenPipeline_;

	std::vector<PostProcessType> initProcesses_;   // 初期化済み
	std::vector<PostProcessType> activeProcesses_; // 適用するプロセス

	// postProcess処理を行うmap
	std::unordered_map<PostProcessType, std::unique_ptr<ComputePostProcessor>> processors_;

	// buffers
	std::unordered_map<PostProcessType, std::unique_ptr<IPostProcessBuffer>> buffers_;
	// buffer更新
	std::unordered_map<PostProcessType, std::unique_ptr<PostProcessUpdaterBase>> updaters_;

	// debugSceneのα値を調整するためのプロセス
	std::unique_ptr<ComputePostProcessor> copyTextureProcess_;

	// frameBufferに描画するGPUHandle、最終的な結果
	D3D12_GPU_DESCRIPTOR_HANDLE frameBufferGPUHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE depthFrameBurferGPUHandle_;

	// editor
	int selectedProcessIndex_ = -1; // 選択されているactiveProcessesのインデックス
	bool needSort_ = false;         // Drag&Drop完了フラグ

	//--------- functions ----------------------------------------------------

	// update
	void ApplyUpdatersToBuffers();

	// helper
	void CreateCBuffer(PostProcessType type);
	void ExecuteCBuffer(ID3D12GraphicsCommandList* commandList, PostProcessType type);
	void BeginTransition(PostProcessType type, DxCommand* dxCommand);

	PostProcessSystem() :IGameEditor("PostProcessSystem") {}
	~PostProcessSystem() = default;
	PostProcessSystem(const PostProcessSystem&) = delete;
	PostProcessSystem& operator=(const PostProcessSystem&) = delete;
};

//============================================================================
//	PostProcessSystem templateMethods
//============================================================================

template<typename T>
inline void PostProcessSystem::SetParameter(const T& parameter, PostProcessType process) {

	if (Algorithm::Find(buffers_, process, true)) {

		buffers_[process]->SetParameter((void*)&parameter, sizeof(T));
	}
}

template<typename T>
inline T* PostProcessSystem::GetUpdater(PostProcessType type) const {

	static_assert(std::is_base_of_v<PostProcessUpdaterBase, T>,
		"T must derive from PostProcessUpdaterBase");
	if (!Algorithm::Find(updaters_, type)) {
		return nullptr;
	}
	return dynamic_cast<T*>(updaters_.at(type).get());
}