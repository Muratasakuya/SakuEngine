#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>

//============================================================================
//	enum class
//============================================================================

// ループの仕方
enum class AnimationLoopType {

	Repeat,
	PingPong,
};

//============================================================================
//	AnimationLoop class
//	0.0f~1.0fの範囲でループ制御
//============================================================================
class AnimationLoop {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	AnimationLoop() = default;
	~AnimationLoop() = default;

	// 0.0f~1.0fの範囲でループ制御
	float LoopedT(float rawT);

	// エディター
	void ImGuiLoopParam(bool isSeparate = true);

	// json
	void ToLoopJson(Json& data);
	void FromLoopJson(const Json& data);

	//--------- accessor -----------------------------------------------------

	// ループ回数の設定
	void SetLoopCount(uint32_t count) { loopCount_ = count; }

	// ループが1回終了したか
	bool IsReachedEnd(float prevRawT, float currentRawT,
		float start = 0.0f, float end = 1.0f) const;

	// ループ回数の取得
	uint32_t GetLoopCount() const { return loopCount_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ループ制御
	uint32_t loopCount_ = 1;      // 回数
	AnimationLoopType loopType_; // 種類
};