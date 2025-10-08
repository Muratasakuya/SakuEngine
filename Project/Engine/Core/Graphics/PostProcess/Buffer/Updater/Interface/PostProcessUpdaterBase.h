#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/PostProcessType.h>

// c++
#include <utility>
#include <cstddef>
// front
class SceneView;

//============================================================================
//	PostProcessUpdaterBase class
//============================================================================
class PostProcessUpdaterBase {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PostProcessUpdaterBase() = default;
	virtual ~PostProcessUpdaterBase() = default;

	// 初期化処理
	virtual void Init() = 0;

	// 更新処理
	virtual void Update() = 0;

	// imgui
	virtual void ImGui() = 0;

	// 呼び出し
	// 開始
	virtual void Start() = 0;
	// 停止
	virtual void Stop() = 0;
	// リセット
	virtual void Reset() = 0;

	//--------- accessor -----------------------------------------------------

	void SetSceneView(SceneView* sceneView) { sceneView_ = sceneView; }

	// 処理を行うpostProcessの種類
	virtual PostProcessType GetType() const = 0;

	// バッファデータ更新用データの取得
	virtual std::pair<const void*, size_t> GetBufferData() const = 0;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	SceneView* sceneView_;
};