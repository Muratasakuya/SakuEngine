#include "RadialBlurUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	RadialBlurUpdater classMethods
//============================================================================

void RadialBlurUpdater::Init() {

	// 使用するタイプを追加
	LerpRadialBlur value{};
	value.width.SetDragValue(0.001f);
	lerpValues_.emplace(RadialBlurType::Parry, value);
	lerpValues_.emplace(RadialBlurType::BeginStun, value);

	// 初期状態を設定
	currentState_ = State::None;
	type_ = RadialBlurType::Parry;

	// json適応
	ApplyJson();
}

void RadialBlurUpdater::SetAnimationType(SimpleAnimationType type) {

	for (auto& value : std::views::values(lerpValues_)) {

		value.center.SetAnimationType(type);
		value.numSamples.SetAnimationType(type);
		value.width.SetAnimationType(type);
	}
}

void RadialBlurUpdater::Update() {

	switch (currentState_) {
	case RadialBlurUpdater::State::Updating:
	case RadialBlurUpdater::State::Return:

		// 時間を更新して各値を補間
		LerpRadialBlur& value = lerpValues_[type_];

		// ブラー中心
		value.center.LerpValue(bufferData_.center);
		// サンプリング数
		value.numSamples.LerpValue(bufferData_.numSamples);
		// ブラー強度
		value.width.LerpValue(bufferData_.width);

		// 全ての補間処理が終了したら終わり
		if (value.center.IsFinished() &&
			value.numSamples.IsFinished() &&
			value.width.IsFinished()) {

			currentState_ = State::None;
		}
		break;
	}
}

void RadialBlurUpdater::ImGui() {

	SaveButton();

	ImGui::Separator();

	// 値操作する状態を選択
	EnumAdapter<RadialBlurType>::Combo("Type", &type_);
	if (EnumAdapter<State>::Combo("State", &currentState_)) {
		switch (currentState_) {
		case RadialBlurUpdater::State::None:

			Reset();
			break;
		case RadialBlurUpdater::State::Updating:

			Start();
			SetAnimationType(SimpleAnimationType::None);
			break;
		case RadialBlurUpdater::State::Return:

			Start();
			SetAnimationType(SimpleAnimationType::Return);
			break;
		case RadialBlurUpdater::State::Stop:

			Stop();
			break;
		}
	}

	ImGui::Separator();

	LerpRadialBlur& value = lerpValues_[type_];

	value.center.ImGui("Center", false);
	ImGui::Spacing();

	value.numSamples.ImGui("NumSamples", false);
	ImGui::Spacing();

	value.width.ImGui("Width", false);
	ImGui::Spacing();
}

void RadialBlurUpdater::Start() {

	// 処理を開始させる
	LerpRadialBlur& value = lerpValues_[type_];
	value.center.Start();
	value.numSamples.Start();
	value.width.Start();
}

void RadialBlurUpdater::Stop() {

	// 値を止める
	LerpRadialBlur& value = lerpValues_[type_];
	value.center.Stop();
	value.numSamples.Stop();
	value.width.Stop();

	// Stopに設定
	currentState_ = State::Stop;
}

void RadialBlurUpdater::Reset() {

	// 値をリセット
	LerpRadialBlur& value = lerpValues_[type_];
	value.center.Reset(false);
	value.numSamples.Reset(false);
	value.width.Reset(false);
}

void RadialBlurUpdater::ApplyJson() {

	Json data;
	if (!LoadFile(data)) {
		return;
	}

	for (auto& [type, value] : lerpValues_) {

		value.center.FromJson(data[EnumAdapter<RadialBlurType>::ToString(type)]["Center"]);
		value.numSamples.FromJson(data[EnumAdapter<RadialBlurType>::ToString(type)]["NumSamples"]);
		value.width.FromJson(data[EnumAdapter<RadialBlurType>::ToString(type)]["Width"]);
	}
}

void RadialBlurUpdater::SaveJson() {

	Json data;

	for (auto& [type, value] : lerpValues_) {

		value.center.ToJson(data[EnumAdapter<RadialBlurType>::ToString(type)]["Center"]);
		value.numSamples.ToJson(data[EnumAdapter<RadialBlurType>::ToString(type)]["NumSamples"]);
		value.width.ToJson(data[EnumAdapter<RadialBlurType>::ToString(type)]["Width"]);
	}

	SaveFile(data);
}