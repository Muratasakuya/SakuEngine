#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/ValueSource/Interface/IValueSource.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>

//============================================================================
//	LerpValueSource class
//	補間された値を取得
//============================================================================
template <typename T>
class LerpValueSource :
	public IValueSource<T> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	LerpValueSource() = default;
	~LerpValueSource() = default;

	// 更新開始
	void Start(const T& base) override;

	// 値の更新
	void Update() override;

	// 終了判定
	bool IsFinished() const override;

	// リセット
	void Reset() override;

	// エディター
	void ImGui(const char* label) override;

	// json
	void FromJson(const Json& data) override;
	void ToJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	// 値の取得
	T GetValue() const override { return current_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 基準値
	T base_;
	// 現在値
	T current_;

	// 値補間
	SimpleAnimation<T> animation_;
};

//============================================================================
//	LerpValueSource templateMethods
//============================================================================

template<typename T>
inline void LerpValueSource<T>::Start(const T& base) {

	// 補間開始
	base_ = base;
	animation_.Start();
}

template<typename T>
inline void LerpValueSource<T>::Update() {

	// 値の補間更新、current_に渡す
	animation_.LerpValue(current_);
}

template<typename T>
inline bool LerpValueSource<T>::IsFinished() const {

	return animation_.IsFinished();
}

template<typename T>
inline void LerpValueSource<T>::Reset() {

	animation_.Reset();
}

template<typename T>
inline void LerpValueSource<T>::ImGui(const char* label) {

	animation_.ImGui(label, false);
}

template<typename T>
inline void LerpValueSource<T>::FromJson(const Json& data) {

	animation_.FromJson(data);
}

template<typename T>
inline void LerpValueSource<T>::ToJson(Json& data) {

	animation_.ToJson(data);
}