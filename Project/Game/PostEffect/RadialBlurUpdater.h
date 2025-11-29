#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	RadialBlurUpdater enum class
//============================================================================

// ゲーム内で使用するエフェクト
enum class RadialBlurType {

	Parry,     // パリィ時のブラー
	BeginStun, // スタンしたときのブラー
};

//============================================================================
//	RadialBlurUpdater class
//	ラジアルブラーの更新、中心からぼかす
//============================================================================
class RadialBlurUpdater :
	public IPostProcessUpdater<RadialBlurForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	RadialBlurUpdater() = default;
	~RadialBlurUpdater() = default;

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
	void Stop() override;
	// リセット
	void Reset() override;

	//--------- accessor -----------------------------------------------------

	// タイプを設定
	void SetBlurType(RadialBlurType type) { type_ = type; }
	void StartState() { currentState_ = State::Updating; }
	void StartReturnState();

	// ブラーの中心を設定
	void SetBlurCenter(const Vector2& center);
	// 自動で元に戻るようにするか設定
	void SetIsAutoReturn(bool isAuto) { isAutoReturn_ = isAuto; }

	PostProcessType GetType() const override { return PostProcessType::RadialBlur; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		None,     // 処理しない
		Updating, // 補間中
		Return,   // startに戻す
		Stop      // 停止中
	};

	// ラジアルブラー補間
	struct LerpRadialBlur {

		// 補間してデータに渡す
		SimpleAnimation<Vector2> center;
		SimpleAnimation<int> numSamples;
		SimpleAnimation<float> width;
	};

	//--------- variables ----------------------------------------------------

	// 現在のタイプ
	RadialBlurType type_;
	// 現在の状態
	State currentState_;

	// 自動で元の値に戻すか
	bool isAutoReturn_;

	// 補間に使用する値
	std::unordered_map<RadialBlurType, LerpRadialBlur> lerpValues_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson() override;
	void SaveJson() override;
	
	// helper
	void SetAnimationType(SimpleAnimationType type);
};