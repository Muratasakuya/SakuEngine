#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxDevice.h>
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/Pipeline/DxShaderCompiler.h>

// directX
#include <dxgidebug.h>
#include <dxgi1_6.h>

//============================================================================
//	GraphicsPlatform class
//	DirectX12のDevice/Command/ShaderCompilerを初期化・保持し、機能チェックも行う。
//============================================================================
class GraphicsPlatform {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GraphicsPlatform() = default;
	~GraphicsPlatform() = default;

	// DxDevice/DxCommand/DxShaderCompilerの生成と初期化を行う
	void Init();

	// コマンドキュー終端などの後処理を実行し、各リソースを破棄する
	void Finalize(HWND hwnd);

	//--------- accessor -----------------------------------------------------

	// デバイス/ファクトリ/コマンド/シェーダコンパイラを取得する

	ID3D12Device8* GetDevice() const { return dxDevice_->Get(); }
	IDXGIFactory7* GetDxgiFactory() const { return dxDevice_->GetDxgiFactory(); }

	DxCommand* GetDxCommand() const { return dxCommand_.get(); }
	DxShaderCompiler* GetDxShaderCompiler() const { return dxShaderComplier_.get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unique_ptr<DxDevice> dxDevice_;

	std::unique_ptr<DxCommand> dxCommand_;

	std::unique_ptr<DxShaderCompiler> dxShaderComplier_;

	//--------- functions ----------------------------------------------------

	// デバッグレイヤ有効化やFeatureチェックを含むデバイス初期化を行う
	void InitDXDevice();
};