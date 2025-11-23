#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>

// c++
#include <vector>

//============================================================================
//	DelayedHitstop class
//	時間差でヒットストップを発生させるクラス
//============================================================================
class DelayedHitstop {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DelayedHitstop() = default;
	~DelayedHitstop() = default;

	// 更新
	void Update();
	
	// ヒットストップ開始
	void Start();
	// リセット
	void Reset();

	// エディター
	void ImGui(const std::string& name, bool isSeparate = true);

	// json
	void FromJson(const Json& data);
	void ToJson(Json& data) const;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ヒットストップ情報
	struct HitStopInfo {

		float timeScale = 0.0f; // スケール値
		float duration = 0.2f;  // 持続時間
		float delay = 0.4f;     // 発生までの遅延時間
	};

	//--------- variables ----------------------------------------------------

	// 開始フラグ
	bool isStart_ = false;

	// ヒットストップ情報
	std::vector<HitStopInfo> hitstopInfos_;

	// 現在の処理インデックス
	uint32_t currentIndex_ = 0;
	// 遅延時間の経過
	float delayElapsed_ = 0.0f;

	//--------- functions ----------------------------------------------------

};