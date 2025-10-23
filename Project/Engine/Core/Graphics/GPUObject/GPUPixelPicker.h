#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/GPUObject/DxReadbackBuffer.h>
#include <Engine/Core/Graphics/GPUObject/DxStructuredBuffer.h>
#include <Engine/MathLib/MathUtils.h>

// front
class SRVDescriptor;
class DxCommand;
class SceneView;

//============================================================================
//	GPUPixelPicker class
//	レンダー結果からGPUでピクセル/オブジェクトのピッキングを行い結果を取得する。
//============================================================================
class GPUPixelPicker {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GPUPixelPicker() = default;
	~GPUPixelPicker() = default;

	// 必要なパイプライン/バッファ/ディスクリプタを初期化する
	void Init(ID3D12Device8* device, DxShaderCompiler* shaderCompiler, SRVDescriptor* srvDescriptor);

	// カメラ/入力/テクスチャサイズ等のピック条件を更新する
	void Update(SceneView* sceneView, const Vector2& textureSize, const Vector2& input);

	// GPUでピッキングを実行し、読み戻しまで行う
	void Execute(DxCommand* dxCommand, ID3D12Resource* tlasResource);

	//--------- accessor -----------------------------------------------------

	bool IsActive() const { return isActivePixelPick_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ピッキング処理用データ
	struct PickingData {

		uint32_t inputPixelX;
		uint32_t inputPixelY;
		uint32_t textureWidth;
		uint32_t textureHeight;
		Matrix4x4 inverseViewProjection;
		Vector3 cameraWorldPos;
		float rayMax;
	};

	//--------- variables ----------------------------------------------------

	std::unique_ptr<PipelineState> pipeline_;

	// ピッキング処理で取得した情報
	int pickedID_;

	// buffers
	DxStructuredBuffer<uint32_t> outputIDBuffer_;
	DxConstBuffer<PickingData> pickingBuffer_;
	DxReadbackBuffer<int> readbackIDBuffer_;
	PickingData pickingData_;

	// 処理が有効かどうか
	bool isActivePixelPick_ = true;

	//--------- functions ----------------------------------------------------

	// 内部ヘルパ: 読み戻し結果をCPU側にコピーする
	void CopyReadbackResource(DxCommand* dxCommand);
	// 内部ヘルパ: 現在のピック対象情報をCBVへ設定する
	void SetPickObject();
};