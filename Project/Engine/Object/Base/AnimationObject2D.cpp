#include "AnimationObject2D.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/Algorithm.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Engine/Utility/Animation/ValueSource/LerpValueSource.h>

//============================================================================
//	AnimationObject2D classMethods
//============================================================================

void AnimationObject2D::DerivedInit() {

	// 非アクティブ状態
	currentActiveKey_ = std::nullopt;
}

void AnimationObject2D::AddAnimationKey(const std::string& key) {

	// キーの追加
	animationKeys_.emplace_back(key);

	// 追加したキーでチャネルを初期化、関数の登録
	// 座標
	translationChannels_[key].valueSource = std::make_unique<LerpValueSource<Vector2>>();
	translationChannels_[key].getter = [](const AnimationObject2D& object) { return object.GetTranslation(); };
	translationChannels_[key].setter = [](AnimationObject2D& object, const Vector2& value) { object.SetTranslation(value); };

	// サイズ
	sizeChannels_[key].valueSource = std::make_unique<LerpValueSource<Vector2>>();
	sizeChannels_[key].getter = [](const AnimationObject2D& object) { return object.GetSize(); };
	sizeChannels_[key].setter = [](AnimationObject2D& object, const Vector2& value) { object.SetSize(value); };

	// スケール
	scaleChannels_[key].valueSource = std::make_unique<LerpValueSource<Vector2>>();
	scaleChannels_[key].getter = [](const AnimationObject2D& object) { return object.GetTransform().sizeScale; };
	scaleChannels_[key].setter = [](AnimationObject2D& object, const Vector2& value) { object.SetSizeScale(value); };

	// 回転
	rotationChannels_[key].valueSource = std::make_unique<LerpValueSource<float>>();
	rotationChannels_[key].getter = [](const AnimationObject2D& object) { return object.GetTransform().rotation; };
	rotationChannels_[key].setter = [](AnimationObject2D& object, const float& value) { object.SetRotation(value); };

	// 色
	colorChannels_[key].valueSource = std::make_unique<LerpValueSource<Color>>();
	colorChannels_[key].getter = [](const AnimationObject2D& object) { return object.GetColor(); };
	colorChannels_[key].setter = [](AnimationObject2D& object, const Color& value) { object.SetColor(value); };

	// アルファ
	alphaChannels_[key].valueSource = std::make_unique<LerpValueSource<float>>();
	alphaChannels_[key].getter = [](const AnimationObject2D& object) { return object.GetColor().a; };
	alphaChannels_[key].setter = [](AnimationObject2D& object, const float& value) { object.SetAlpha(value); };
}

void AnimationObject2D::UpdateAnimations() {

	// キーがなければ処理しない
	if (animationKeys_.empty() || !currentActiveKey_.has_value()) {
		return;
	}

	// アクティブなキー
	std::string key = currentActiveKey_.value();

	// アニメーションの更新
	translationChannels_.at(key).Update(*this);
	sizeChannels_.at(key).Update(*this);
	scaleChannels_.at(key).Update(*this);
	rotationChannels_.at(key).Update(*this);
	colorChannels_.at(key).Update(*this);
	alphaChannels_.at(key).Update(*this);
}

void AnimationObject2D::StartAnimations(const std::string& key) {

	// 存在しないキーでは処理しない
	if (!Algorithm::Find(animationKeys_, key)) {
		return;
	}

	currentActiveKey_ = key;

	// enableがtrueのアニメーションの補間を開始する、内部判定してる
	translationChannels_.at(key).Start(*this);
	sizeChannels_.at(key).Start(*this);
	scaleChannels_.at(key).Start(*this);
	rotationChannels_.at(key).Start(*this);
	colorChannels_.at(key).Start(*this);
	alphaChannels_.at(key).Start(*this);
}

void AnimationObject2D::ClearActiveAnimationKey() {

	currentActiveKey_ = std::nullopt;
}

void AnimationObject2D::DerivedImGui() {

	// キーがなければ処理しない
	if (animationKeys_.empty()) {
		return;
	}

	ImGuiHelper::ComboFromStrings("Animation Keys", &selectedAnimationKeyIndex_,
		animationKeys_, static_cast<int32_t>(animationKeys_.size() + 1));

	ImGui::Separator();

	// アニメーション処理を行うかのフラグ設定
	ToggleAnimationActiveState(true);
	ToggleAnimationActiveState(false);

	// 現在のキー
	std::string key = animationKeys_[selectedAnimationKeyIndex_];

	// enableがtrueのアニメーションだけ表示する
	ShowChannel("Translation", key, translationChannels_);
	ShowChannel("Size", key, sizeChannels_);
	ShowChannel("Scale", key, scaleChannels_);
	ShowChannel("Rotation", key, rotationChannels_);
	ShowChannel("Color", key, colorChannels_);
	ShowChannel("Alpha", key, alphaChannels_);
}

void AnimationObject2D::ToggleAnimationActiveState(bool isActive) {

	// 現在のキー
	std::string key = animationKeys_[selectedAnimationKeyIndex_];
	const char* label = isActive ? "Active" : "Inactive";

	if (EnumAdapter<AnimationType>::Combo(label, &editEnableAnimationType_)) {
		switch (editEnableAnimationType_) {
		case AnimationObject2D::AnimationType::Translation:

			translationChannels_[key].isEnable = isActive;
			break;
		case AnimationObject2D::AnimationType::Size:

			sizeChannels_[key].isEnable = isActive;
			break;
		case AnimationObject2D::AnimationType::Scale:

			scaleChannels_[key].isEnable = isActive;
			break;
		case AnimationObject2D::AnimationType::Rotation:

			rotationChannels_[key].isEnable = isActive;
			break;
		case AnimationObject2D::AnimationType::Color:

			colorChannels_[key] .isEnable = isActive;
			break;
		case AnimationObject2D::AnimationType::Alpha:

			alphaChannels_[key].isEnable = isActive;
			break;
		}
	}
}

void AnimationObject2D::ApplyJsonAndAnimation(const Json& data) {

	if (animationKeys_.empty() || data.empty()) {
		return;
	}

	// 基底クラスのjson適応
	GameObject2D::ApplyJson(data);

	for (const auto& key : animationKeys_) {

		// アニメーションのjson適応
		translationChannels_.at(key).FromJson(data.at(key).at("TranslationChannels"));
		sizeChannels_.at(key).FromJson(data.at(key).at("SizeChannels"));
		scaleChannels_.at(key).FromJson(data.at(key).at("ScaleChannels"));
		rotationChannels_.at(key).FromJson(data.at(key).at("RotationChannels"));
		colorChannels_.at(key).FromJson(data.at(key).at("ColorChannels"));
		alphaChannels_.at(key).FromJson(data.at(key).at("AlphaChannels"));
	}
}

void AnimationObject2D::SaveJsonAndAnimation(Json& data) {

	// 基底クラスのjson保存
	GameObject2D::SaveJson(data);

	for (const auto& key : animationKeys_) {

		// アニメーションのjson保存
		translationChannels_[key].ToJson(data[key]["TranslationChannels"]);
		sizeChannels_[key].ToJson(data[key]["SizeChannels"]);
		scaleChannels_[key].ToJson(data[key]["ScaleChannels"]);
		rotationChannels_[key].ToJson(data[key]["RotationChannels"]);
		colorChannels_[key].ToJson(data[key]["ColorChannels"]);
		alphaChannels_[key].ToJson(data[key]["AlphaChannels"]);
	}
}