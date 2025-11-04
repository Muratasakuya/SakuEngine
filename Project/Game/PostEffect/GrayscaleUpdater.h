#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	GrayscaleUpdater class
//	グレースケールの更新
//============================================================================
class GrayscaleUpdater :
	public IPostProcessUpdater<GrayscaleForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GrayscaleUpdater() = default;
	~GrayscaleUpdater() = default;

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

	// グレースケール解除
	void ClearGrayScale() { currentState_ = State::None; }
	// 固定グレースケール
	void SetConstantGrayScale() { currentState_ = State::Constant; }

	PostProcessType GetType() const override { return PostProcessType::Grayscale; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		None,     // 何もしない
		Updating, // 更新中
		WaitGray, // グレースケール維持中
		Return,   // 元に戻す
		Constant, // 固定グレースケール
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// グレースケールの割合アニメーション(0.0f~1.0f)
	SimpleAnimation<float> rateAnimation_;

	// 補間終了後の待ち時間
	StateTimer waitGrayTimer_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson() override;
	void SaveJson() override;
};