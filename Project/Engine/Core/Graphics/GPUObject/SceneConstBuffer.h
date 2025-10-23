#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/Raytracing/RaytracingStructures.h>
#include <Engine/Core/Graphics/GPUObject/DitherStructures.h>
#include <Engine/Effect/Particle/Structures/ParticleStructures.h>
#include <Engine/Scene/Light/PunctualLight.h>
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Matrix4x4.h>

//============================================================================
//	SceneConstBuffer class
//	シーン共通の各種定数バッファ(CBV)をまとめ、セットコマンドを提供する。
//============================================================================
class SceneConstBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SceneConstBuffer() = default;
	~SceneConstBuffer() = default;

	// 各種CBVを作成し、使用可能な状態に構築する
	void Create(ID3D12Device* device);

	// シーンビューから必要な定数を更新する
	void Update(class SceneView* sceneView);

	// メインパスに必要なCBVをルートへセットする
	void SetMainPassCommands(bool debugEnable, ID3D12GraphicsCommandList* commandList);
	// ビュー×射影行列関連のCBVをルートへセットする
	void SetViewProCommand(bool debugEnable, ID3D12GraphicsCommandList* commandList, UINT rootIndex);
	// ビュー依存のパラメータCBVをルートへセットする
	void SetPerViewCommand(bool debugEnable, ID3D12GraphicsCommandList* commandList, UINT rootIndex);
	// 正射影用のCBVをルートへセットする
	void SetOrthoProCommand(ID3D12GraphicsCommandList* commandList, UINT rootIndex);
	// レイトレース用シーンCBVをルートへセットする
	void SetRaySceneCommand(ID3D12GraphicsCommandList* commandList, UINT rootIndex);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// camera3D
	// main
	DxConstBuffer<Matrix4x4> viewProjectionBuffer_;
	DxConstBuffer<Vector3> cameraPosBuffer_;
	// debug
	DxConstBuffer<Matrix4x4> debugSceneViewProjectionBuffer_;
	DxConstBuffer<Vector3> debugSceneCameraPosBuffer_;
	// camera2D
	DxConstBuffer<Matrix4x4> orthoProjectionBuffer_;

	// light
	DxConstBuffer<PunctualLight> lightBuffer_;

	// rayScene
	DxConstBuffer<RaySceneForGPU> raySceneBuffer_;

	// dither
	DxConstBuffer<DitherForGPU> ditherBuffer_;

	// perView
	DxConstBuffer<ParticleCommon::PerViewForGPU> perViewBuffer_;
	DxConstBuffer<ParticleCommon::PerViewForGPU> debugScenePerViewBuffer_;
};