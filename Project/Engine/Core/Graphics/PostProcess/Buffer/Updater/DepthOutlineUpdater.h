#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>

//============================================================================
//	DepthOutlineUpdater class
//============================================================================
class DepthOutlineUpdater :
	public IPostProcessUpdater<DepthBasedOutlineForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DepthOutlineUpdater() = default;
	~DepthOutlineUpdater() = default;

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

	PostProcessType GetType() const override { return PostProcessType::DepthBasedOutline; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson() override;
	void SaveJson() override;
};