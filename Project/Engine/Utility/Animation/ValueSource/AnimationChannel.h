#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/ValueSource/Interface/IValueSource.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// c++
#include <functional>
#include <memory>

//============================================================================
//	AnimationChannel enum class
//============================================================================

// アニメーションの適用方法
enum class AnimationApplyMode {

	Absolute,
	Additive
};

//============================================================================
//	AnimationChannel class
//	値の適用処理を行うチャネル
//============================================================================
template <class Target, typename T>
struct AnimationChannel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	AnimationChannel() = default;
	~AnimationChannel() = default;

	//--------- variables ----------------------------------------------------

	// 値の取得関数
	std::function<T(const Target&)> getter;
	// 値の設定関数
	std::function<void(Target&, const T&)> setter;

	// 補間などを行った値
	std::unique_ptr<IValueSource<T>> valueSource;

	// 処理をするかどうか、falseならなにもしない
	bool isEnable = false;

	// 状態
	bool isActive = false;
	// アニメーションの適用方法
	AnimationApplyMode applyMode = AnimationApplyMode::Absolute;
	// 基準値
	T baseValue{};

	// 値に対しての処理を開始させる
	void Start(Target& object);

	// 更新処理を行い、値を適用する
	void Update(Target& object);

	// リセット
	void Reset();

	// エディター
	void ImGui(const char* label);

	// json
	void FromJson(const Json& data);
	void ToJson(Json& data);
};

//============================================================================
//	AnimationChannel templateMethods
//============================================================================

template<class Target, typename T>
inline void AnimationChannel<Target, T>::Start(Target& object) {

	// 無効状態の時は処理しない
	if (!isEnable) {
		return;
	}

	// アクティブにする
	isActive = true;

	// 基準値を取得して開始
	baseValue = getter(object);
	valueSource->Start(baseValue);
}

template<class Target, typename T>
inline void AnimationChannel<Target, T>::Update(Target& object) {

	// 無効状態の時は処理しない
	if (!isEnable) {
		return;
	}

	// 非アクティブ状態の時は処理しない
	if (!isActive) {
		return;
	}

	// 値の更新
	valueSource->Update();
	// 更新した値を取得する
	T value = valueSource->GetValue();
	if constexpr (!std::is_same_v<T, Color>) {
		if (applyMode == AnimationApplyMode::Additive) {

			value = baseValue + value;
		}
	}
	setter(object, value);

	// 処理が終了したら非アクティブ状態にする
	if (valueSource->IsFinished()) {

		isActive = false;
	}
}

template<class Target, typename T>
inline void AnimationChannel<Target, T>::Reset() {

	// 無効状態の時は処理しない
	if (!isEnable) {
		return;
	}

	// リセット
	isActive = false;
	valueSource->Reset();
}

template<class Target, typename T>
inline void AnimationChannel<Target, T>::ImGui(const char* label) {

	valueSource->ImGui(label);
}

template<class Target, typename T>
inline void AnimationChannel<Target, T>::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	isEnable = data.value("isEnable", true);
	applyMode = EnumAdapter<AnimationApplyMode>::FromString(data.value("applyMode", "Absolute")).value();
	valueSource->FromJson(data["ValueSource"]);
}

template<class Target, typename T>
inline void AnimationChannel<Target, T>::ToJson(Json& data) {

	data["isEnable"] = isEnable;
	data["applyMode"] = EnumAdapter<AnimationApplyMode>::ToString(applyMode);
	valueSource->ToJson(data["ValueSource"]);
}