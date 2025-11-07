#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/IPostProcessUpdater.h>

//============================================================================
//	PlayerAfterImageUpdater class
//	プレイヤーの残像表現エフェクトの更新
//============================================================================
class PlayerAfterImageUpdater :
	public IPostProcessUpdater<PlayerAfterImageForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAfterImageUpdater() = default;
	~PlayerAfterImageUpdater() = default;

	// 初期化処理
	void Init() override;

	// 更新処理
	void Update() override;

	// imgui
	void ImGui() override;

	// 色指定して呼びだす
	void Start(const Color& color);

	// 呼び出し
	// 開始
	void Start() override {}
	// 停止
	void Stop() override {}
	// リセット
	void Reset() override;

	//--------- accessor -----------------------------------------------------

	PostProcessType GetType() const override { return PostProcessType::PlayerAfterImage; }
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