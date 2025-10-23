#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>

//============================================================================
//	DepthOutlineUpdater class
//	深度ベースの輪郭強調エフェクト(DepthBasedOutline)のパラメータ更新/保存/UI編集を行う。
//============================================================================
class DepthOutlineUpdater :
	public IPostProcessUpdater<DepthBasedOutlineForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DepthOutlineUpdater() = default;
	~DepthOutlineUpdater() = default;

	// 初期化処理(jsonから既定値を読み込む)
	void Init() override;

	// 更新処理(カメラ行列などシーン依存値をGPU用構造体へ反映)
	void Update() override;

	// imgui(パラメータ編集と保存ボタンを提供)
	void ImGui() override;

	// 呼び出し
	// 開始
	void Start() override {}
	// 停止
	void Stop() override {}
	// リセット
	void Reset() override {}

	//--------- accessor -----------------------------------------------------

	// このUpdaterが扱うポストプロセス種別を返す
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