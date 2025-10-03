#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetStructure.h>
#include <Engine/Utility/Enum/ObjectUpdateMode.h>

// c++
#include <execution>
#include <ranges> 

// front 
class Asset;

//============================================================================
//	SkinnedAnimation class
//============================================================================
class SkinnedAnimation {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SkinnedAnimation() = default;
	~SkinnedAnimation() = default;

	void Init(const std::string& animationName, Asset* asset);

	void Update(const Matrix4x4& worldMatrix);

	void ImGui(float itemSize);

	//--------- accessor -----------------------------------------------------

	//========================================================================
	//	アニメーション設定
	//========================================================================

	// 新しいアニメーションの登録
	void SetAnimationData(const std::string& animationName);
	// アニメーション再生
	void SetPlayAnimation(const std::string& animationName, bool roopAnimation);
	// 切り替えアニメーション
	void SwitchAnimation(const std::string& nextAnimName, bool loopAnimation, float transitionDuration);
	// アニメーションリセット
	void ResetAnimation();
	// 再生速度の設定
	void SetPlaybackSpeed(float playbackSpeed) { playbackSpeed_ = playbackSpeed; }
	// 更新方法の設定
	void SetUpdateMode(ObjectUpdateMode mode) { updateMode_ = mode; }
	// 再生時間の設定
	void SetCurrentAnimTime(float time);

	// 登録されているアニメーションの名前
	const std::string& GetCurrentAnimationName() const { return currentAnimationName_; }
	std::vector<std::string> GetAnimationNames() const;

	// 現在のアニメーション再生時間
	float GetCurrentAnimTime() const { return currentAnimationTimers_[updateModeIndex_]; }
	// アニメーション進捗(0.0f -> 1.0f)
	float GetProgress() const { return animationProgress_; }
	// 再生速度
	float GetPlaybackSpeed() const { return playbackSpeed_; }
	// アニメーションの終了時間
	float GetAnimationDuration(const std::string& animationName) const;
	// リピート回数
	int GetRepeatCount() const { return repeatCount_; }
	// 遷移中か
	bool IsTransition() const { return inTransition_; }
	// アニメーションが終了したか
	bool IsAnimationFinished() const { return animationFinish_; }

	//========================================================================
	//	キーイベント
	//========================================================================

	// アニメーションのキーフレームのイベント位置をファイルから取得して登録
	void SetKeyframeEvent(const std::string& fileName);
	// 指定のキーイベントに到達したか(トリガー判定)
	bool IsEventKey(const std::string& keyEvent, uint32_t frameIndex);

	// キーイベントまでの時間を取得
	float GetEventTime(const std::string& animName, const std::string& keyEvent, uint32_t frameIndex) const;

	//========================================================================
	//	ジョイント
	//========================================================================

	// 親として更新するjointを設定
	void SetParentJoint(const std::string& jointName);

	// 親となるトランスフォームを取得
	const Transform3D* FindJointTransform(const std::string& name) const;

	//========================================================================
	//	デバッグ
	//========================================================================

	// 骨のデバッグ表示
	void SetDebugViewBone(bool enable) { isDisplayBone_ = enable; }

	//========================================================================
	//	バッファデータ
	//========================================================================
	
	const std::vector<WellForGPU>& GetWellForGPU() const { return skinCluster_.mappedPalette; }
	const Skeleton& GetSkeleton() const { return skeleton_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Asset* asset_;

	// 更新方法
	ObjectUpdateMode updateMode_ = ObjectUpdateMode::None;
	uint32_t updateModeIndex_ = static_cast<uint32_t>(updateMode_);

	// アニメーションはstringで名前ごとに保存して使うようにする
	std::unordered_map<std::string, AnimationData> animationData_;
	// 使用するジョイントをstd::stringで名前ごとに記録
	std::unordered_map<std::string, std::vector<const NodeAnimation*>> jointAnimationTracks_;
	Skeleton skeleton_;
	SkinCluster skinCluster_;

	// キーフレームイベント
	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> eventKeyTables_;
	std::unordered_map<std::string, int> prevFrameIndexPerKey_;

	// アニメーション
	std::string currentAnimationName_;
	std::array<float, static_cast<uint32_t>(ObjectUpdateMode::Count)> currentAnimationTimers_;
	float playbackSpeed_;  // アニメーションの再生速度
	bool roopAnimation_;   // ループするかどうか
	bool animationFinish_; // 現在のanimationが終了したかどうか

	int repeatCount_;      // リピート回数

	bool inTransition_;        // 遷移中かどうか
	float transitionTimer_;    // 遷移管理タイマー
	float transitionDuration_; // 遷移時間

	// 切り替え前のanimation
	std::string oldAnimationName_;
	float oldAnimationTimer_;
	// 切り替え後のanimation
	std::string nextAnimationName_;
	float nextAnimationTimer_;

	// animationの経過率
	float animationProgress_;

	// 骨の線描画を行うかどうか
	bool isDisplayBone_;
	std::vector<std::vector<int>> children_;

	//--------- variables ----------------------------------------------------

	// joint更新
	void ApplyAnimation(float timer);
	void UpdateSkeleton(const Matrix4x4& worldMatrix);
	void UpdateSkinCluster();

	// blend処理
	void BlendAnimation(Skeleton& skeleton,
		const AnimationData& oldAnimationData, float oldAnimTime,
		const AnimationData& nextAnimationData, float nextAnimTime, float alpha);

	// helper
	int CurrentFrameIndex() const;
	void DrawEventTimeline(const std::vector<int>& frames, int currentFrame,
		int totalFrames, float barWidth, float barHeight);
	void DebugDrawBone(const Matrix4x4& worldMatrix);
};