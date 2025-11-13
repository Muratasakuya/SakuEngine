#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>

//============================================================================
//	DefaultDistortionUpdater class
//	デフォルトの歪み更新
//============================================================================
class DefaultDistortionUpdater :
	public IPostProcessUpdater<DefaultDistortionForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DefaultDistortionUpdater() = default;
	~DefaultDistortionUpdater() = default;

	// 初期化処理
	void Init() override;

	// 更新処理
	void Update() override;

	// imgui
	void ImGui() override;

	// 呼び出し
	// 開始
	void Start() override {}
	// 停止
	void Stop() override {}
	// リセット
	void Reset() override {}

	//--------- accessor -----------------------------------------------------

	PostProcessType GetType() const override { return PostProcessType::DefaultDistortion; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// UVの更新を行う
	Vector2 translation_; // 座標
	Vector2 scale_;       // スケール
	float rotationZ_;     // Z回転

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson() override;
	void SaveJson() override;
};