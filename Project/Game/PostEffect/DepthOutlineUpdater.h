#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	DepthOutlineUpdater class
//	深度ベースの輪郭強調エフェクト
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

	// 色指定して呼びだす
	void Start(const Color& color, float edgeScale);

	// 呼び出し
	// 開始
	void Start() override {}
	// 停止
	void Stop() override {}
	// リセット
	void Reset() override;

	//--------- accessor -----------------------------------------------------

	// このUpdaterが扱うポストプロセス種別を返す
	PostProcessType GetType() const override { return PostProcessType::DepthBasedOutline; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson() override;
	void SaveJson() override;
};