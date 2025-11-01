#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	CRTDisplayUpdater class
//	CRTディスプレイエフェクトの更新
//============================================================================
class CRTDisplayUpdater :
	public IPostProcessUpdater<CRTDisplayForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CRTDisplayUpdater() = default;
	~CRTDisplayUpdater() = default;

	// 初期化処理
	void Init() override;

	// 更新処理
	void Update() override;

	// imgui
	void ImGui() override;

	// 呼び出し
	// 開始
	void Start() override;
	// 停止
	void Stop() override {}
	// リセット
	void Reset() override {}

	//--------- accessor -----------------------------------------------------

	PostProcessType GetType() const override { return PostProcessType::CRTDisplay; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		None,   // 無効
		Start,  // ゲーム起動したとき
		Update, // 更新中
		End     // ゲーム終了したとき
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson() override;
	void SaveJson() override;
};