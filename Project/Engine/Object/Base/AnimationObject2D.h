#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>
#include <Engine/Utility/Animation/ValueSource/AnimationChannel.h>

// imgui
#include <imgui.h>

//============================================================================
//	AnimationObject2D class
//	アニメーション機能を持つ2Dオブジェクトの基底クラス
//============================================================================
class AnimationObject2D :
	public GameObject2D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	AnimationObject2D() = default;
	~AnimationObject2D() = default;

	// 初期化
	void DerivedInit() override;

	// キーの追加
	void AddAnimationKey(const std::string& key);

	// アニメーションの更新
	void UpdateAnimations();

	// エディター
	void DerivedImGui() override;

	// json
	void ApplyJsonAndAnimation(const Json& data);
	void SaveJsonAndAnimation(Json& data);

	// 指定キーのenableがtrueのアニメーションの補間を開始する
	void StartAnimations(const std::string& key);
	// アニメーションキーのクリア
	void ClearActiveAnimationKey();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// アニメーションの種類
	enum class AnimationType {

		Translation, // 座標の移動補間
		Size,        // サイズ補間
		Scale,       // スケール補間
		Rotation,    // 回転補間
		Color,       // 色補間
		Alpha,       // アルファ補間
	};

	//--------- variables ----------------------------------------------------

	// キーの種類
	std::vector<std::string> animationKeys_;
	// 現在アクティブなキー
	std::optional<std::string> currentActiveKey_;

	// アニメーション処理を行う値の変数
	// 座標
	std::unordered_map<std::string, AnimationChannel<AnimationObject2D, Vector2>> translationChannels_;
	// サイズ
	std::unordered_map<std::string, AnimationChannel<AnimationObject2D, Vector2>> sizeChannels_;
	// スケール
	std::unordered_map<std::string, AnimationChannel<AnimationObject2D, Vector2>> scaleChannels_;
	// 回転
	std::unordered_map<std::string, AnimationChannel<AnimationObject2D, float>> rotationChannels_;
	// 色
	std::unordered_map<std::string, AnimationChannel<AnimationObject2D, Color>> colorChannels_;
	// アルファ
	std::unordered_map<std::string, AnimationChannel<AnimationObject2D, float>> alphaChannels_;

	// エディター
	int32_t selectedAnimationKeyIndex_ = 0;
	// 指定アニメーションの有効、無効を切り替える
	AnimationType editEnableAnimationType_;

	//--------- functions ----------------------------------------------------

	// 指定アニメーションの有効と無効を切り替える
	void ToggleAnimationActiveState(bool isActive);

	// アニメーションImGuiの表示
	template <typename T>
	void ShowChannel(const char* headerLabel, const std::string& key, T& channels);
};

//============================================================================
//	AnimationObject2D templateMethods
//============================================================================

template <typename T>
inline void AnimationObject2D::ShowChannel(
	const char* headerLabel, const std::string& key, T& channels) {

	auto it = channels.find(key);
	if (it == channels.end()) {
		return;
	}

	auto& channel = it->second;
	// trueの時だけ表示する
	if (!channel.isEnable) {
		return;
	}
	if (ImGui::CollapsingHeader(headerLabel)) {
		channel.ImGui(headerLabel);
	}
}