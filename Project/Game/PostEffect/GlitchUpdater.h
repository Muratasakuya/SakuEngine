#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	GlitchUpdater class
//============================================================================
class GlitchUpdater :
	public IPostProcessUpdater<GlitchForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GlitchUpdater() = default;
	~GlitchUpdater() = default;

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

	PostProcessType GetType() const override { return PostProcessType::Glitch; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		None,     // 処理しない
		Updating, // 更新中
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 時間経過
	StateTimer timer_;            // 1処理にかかる時間
	StateTimer convergenceTimer_; // ランダム発生間隔
	int maxRandomCount_; // ランダム処理回数
	int currentCount_;   // 処理回数
	float startIntencityRange_;  // 開始時の強度の範囲
	float intencityRange_;       // 強度の範囲

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson() override;
	void SaveJson() override;
};