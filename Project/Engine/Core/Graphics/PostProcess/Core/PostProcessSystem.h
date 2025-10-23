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
//	ポストプロセス一式の初期化/実行/管理(UI含む)を担う統括クラス。
//============================================================================
class PostProcessSystem :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// デバイスやディスクリプタ等の依存を受け取り、パイプライン/コピー用プロセスを準備する
	void Init(ID3D12Device8* device, class DxShaderCompiler* shaderComplier,
		SRVDescriptor* srvDescriptor, Asset* asset, SceneView* sceneView);

	// 登録済みUpdaterを用いて各ポストプロセスの定数バッファを更新する
	void Update();

	// 使用可能なポストプロセス群を初期化し、Processor/Buffer/PSOを生成する
	void Create(const std::vector<PostProcessType>& processes);

	// ColorとMaskの入力SRVを元に、アクティブなプロセスを順番にディスパッチする
	void Execute(DxCommand* dxCommand,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputSRVGPUHandle,      // 色
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputMaskSRVGPUHandle); // ポストエフェクトマスク
	// デバッグビュー用にコピー処理のみ実行する
	void ExecuteDebugScene(DxCommand* dxCommand,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputSRVGPUHandle,      // 色
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputMaskSRVGPUHandle); // ポストエフェクトマスク

	// 最終結果のSRVを全画面三角形でスワップチェインRTへ描画する
	void RenderFrameBuffer(DxCommand* dxCommand);

	// imguiでAvailable/Active/Parameters/Updaterを操作するUIを表示する
	void ImGui() override;

	// 次フレームに備え、出力テクスチャをUAV書き込み状態へ戻す
	void ToWrite(DxCommand* dxCommand);

	//--------- accessor -----------------------------------------------------

	// アクティブなプロセス集合を追加/削除/初期化(Copyのみ)する
	void AddProcess(PostProcessType process);
	void RemoveProcess(PostProcessType process);
	void ClearProcess();

	// プロセスが参照する追加テクスチャ(例: 深度)のSRVを名前から設定する
	void InputProcessTexture(const std::string& textureName, PostProcessType process);

	// パラメータ更新クラスを登録/削除する
	void RegisterUpdater(std::unique_ptr<PostProcessUpdaterBase> updater);
	void RemoveUpdater(PostProcessType type);

	// Updaterの状態制御を行う
	void Start(PostProcessType type);
	void Stop(PostProcessType type);
	void Reset(PostProcessType type);

	// 指定型のパラメータを対応バッファへ書き込む(存在時のみ)
	template <typename T>
	void SetParameter(const T& parameter, PostProcessType process);
	// 深度ベースのエフェクトで利用する深度SRVハンドルを設定する
	void SetDepthFrameBufferGPUHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& handle) { depthFrameBurferGPUHandle_ = handle; }

	// パイプライン/コピー結果のSRVを取得する
	PostProcessPipeline* GetPipeline() const { return pipeline_.get(); }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetCopySRVGPUHandle() const { return copyTextureProcess_->GetSRVGPUHandle(); }

	// 型に一致するUpdaterのポインタを返す(未登録ならnullptr)
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

	// 全Updaterを実行して対応バッファへ最新値を反映する
	void ApplyUpdatersToBuffers();

	// タイプ別のCBを生成し、必要ならUpdaterも自動登録する
	void CreateCBuffer(PostProcessType type);
	// 指定プロセスのCB更新/ルート定数バインドを行う
	void ExecuteCBuffer(ID3D12GraphicsCommandList* commandList, PostProcessType type);
	// 最終かどうかで遷移先(PS/CS)を切り替え、UAVから適切な状態へ遷移する
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