#include "PlayerStunHUD.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	PlayerStunHUD classMethods
//============================================================================

void PlayerStunHUD::Init() {

	// 経過率背景
	progressBarBackground_ = std::make_unique<GameObject2D>();
	progressBarBackground_->Init("stunProgressBarBackground",
		"stunProgressBarBackground", "PlayerStunHUD");

	// 経過率
	progressBar_ = std::make_unique<GameObject2D>();
	progressBar_->Init("stunProgressBar", "stunProgressBar", "PlayerStunHUD");

	// 経過率文字
	chainAttackText_ = std::make_unique<GameObject2D>();
	chainAttackText_->Init("CHAINATTACK", "CHAINATTACK", "PlayerStunHUD");

	// アイコン
	for (uint32_t index = 0; index < iconCount_; ++index) {

		stunChainIcon_[index] = std::make_unique<GameObject2D>();
		stunChainIcon_[index]->Init("chainPlayerIcon", "chainPlayerIcon", "PlayerStunHUD");

		stunChainIconRing_[index] = std::make_unique<GameObject2D>();
		stunChainIconRing_[index]->Init("chainPlayerIconRing", "chainPlayerIconRing", "PlayerStunHUD");
	}

	// 入力
	keyInput_.Init("rightMouseClick", "leftMouseClick", "mouseCancel");
	gamepadInput_.Init("RBButton", "LBButton", "gamepadCancel");

	// タイマー
	restTimerDisplay_ = std::make_unique<GameTimerDisplay>();
	restTimerDisplay_->Init("dd:dd:dd", "timeNumber", "timeSymbol", "restTimer", "PlayerStunHUD");

	isVaild_ = false;
	isCountFinished_ = false;
	currentState_ = State::Begin;

	// json適応
	ApplyJson();

	// 最初は表示を切る
	SetSize(Vector2::AnyInit(0.0f));
}

void PlayerStunHUD::SetSize(const Vector2& size) {

	// 最初は表示を行わない、サイズを0.0fにする
	progressBarBackground_->SetSize(size);
	progressBar_->SetSize(size);
	chainAttackText_->SetSize(size);
	for (uint32_t index = 0; index < iconCount_; ++index) {

		stunChainIcon_[index]->SetSize(size);
		stunChainIconRing_[index]->SetSize(size);
	}
	keyInput_.SetSize(size);
	keyInput_.cancel->SetSize(size);
	gamepadInput_.SetSize(size);
	gamepadInput_.cancel->SetSize(size);

	restTimerDisplay_->SetTimerSize(size);
	restTimerDisplay_->SetSymbolSize(size);
}

void PlayerStunHUD::SetTargetSize(float lerpT) {

	const Vector2 zero = Vector2::AnyInit(0.0f);

	// それぞれを目標サイズまで補間する
	progressBarBackground_->SetSize(Vector2::Lerp(zero, progressBarBackgroundSize_, lerpT));
	progressBar_->SetSize(Vector2::Lerp(zero, progressBarSize_, lerpT));
	chainAttackText_->SetSize(Vector2::Lerp(zero, chainAttackTextSize_, lerpT));

	for (uint32_t index = 0; index < iconCount_; ++index) {

		stunChainIcon_[index]->SetSize(Vector2::Lerp(zero, iconSize_, lerpT));
		stunChainIconRing_[index]->SetSize(Vector2::Lerp(iconRingSize_, iconSize_, lerpT));
	}

	Vector2 lerpInputSize = Vector2::Lerp(zero, chainInputSize_, lerpT);
	Vector2 lerpInputCancelSize = Vector2::Lerp(zero, cancelInputSize_, lerpT);
	keyInput_.SetSize(lerpInputSize);
	keyInput_.cancel->SetSize(lerpInputCancelSize);
	gamepadInput_.SetSize(lerpInputSize);
	gamepadInput_.cancel->SetSize(lerpInputCancelSize);

	restTimerDisplay_->SetTimerSize(Vector2::Lerp(zero, timerSize_, lerpT));
	restTimerDisplay_->SetSymbolSize(Vector2::Lerp(zero, timerSymbolSize_, lerpT));
}

void PlayerStunHUD::SetTargetTranslation(float lerpT) {

	const float barTranslationY = std::lerp(barTranslationY_, barTranslationY_ + cancelUnderOffsetY_, lerpT);
	const float cancelTranslationY = std::lerp(cancelTranslationY_, cancelTranslationY_ + cancelUnderOffsetY_, lerpT);

	// ProgressBar
	const Vector2 barPos(centerTranslationX_, barTranslationY);
	progressBarBackground_->SetTranslation(barPos);
	progressBar_->SetTranslation(barPos);
	chainAttackText_->SetTranslation(barPos);

	// Timer
	restTimerDisplay_->SetTranslation(
		Vector2(timerTranslationX_, std::lerp(timerTranslationY_, timerTranslationY_ + cancelUnderOffsetY_, lerpT)));

	// Cancel
	const Vector2 cancelPos(centerTranslationX_, cancelTranslationY);
	keyInput_.cancel->SetTranslation(cancelPos);
	gamepadInput_.cancel->SetTranslation(cancelPos);

	// 右入力
	const Vector2 rightPos(centerTranslationX_ + endOffsetX_, cancelTranslationY);
	keyInput_.rightChain->SetTranslation(rightPos);
	gamepadInput_.rightChain->SetTranslation(rightPos);
	stunChainIcon_[0]->SetTranslation(
		Vector2(centerTranslationX_ + endOffsetX_, barTranslationY));
	stunChainIconRing_[0]->SetTranslation(
		Vector2(centerTranslationX_ + endOffsetX_, barTranslationY));

	// 左入力
	const Vector2 leftPos(centerTranslationX_ - endOffsetX_, cancelTranslationY);
	keyInput_.leftChain->SetTranslation(leftPos);
	gamepadInput_.leftChain->SetTranslation(leftPos);
	stunChainIcon_[1]->SetTranslation(
		Vector2(centerTranslationX_ - endOffsetX_, barTranslationY));
	stunChainIconRing_[1]->SetTranslation(
		Vector2(centerTranslationX_ - endOffsetX_, barTranslationY));
}

void PlayerStunHUD::SetAlpha(float alpha) {

	progressBarBackground_->SetAlpha(alpha);
	progressBar_->SetAlpha(alpha);
	chainAttackText_->SetAlpha(alpha);

	for (uint32_t index = 0; index < iconCount_; ++index) {

		stunChainIcon_[index]->SetAlpha(alpha);
		stunChainIconRing_[index]->SetAlpha(alpha);
	}

	keyInput_.rightChain->SetAlpha(alpha);
	keyInput_.leftChain->SetAlpha(alpha);
	keyInput_.cancel->SetAlpha(alpha);
	gamepadInput_.rightChain->SetAlpha(alpha);
	gamepadInput_.leftChain->SetAlpha(alpha);
	gamepadInput_.cancel->SetAlpha(alpha);

	restTimerDisplay_->SetAlpha(alpha);
}

float PlayerStunHUD::CalcBlinkAlpha(float progress, int blinkCount) {

	return 0.5f * (1.0f + std::cos(progress * blinkCount * pi));
}

void PlayerStunHUD::SetVaild() {

	// 有効にしてanimationを行い、カウントを開始する
	isVaild_ = true;
}

void PlayerStunHUD::SetCancel() {

	// キャンセル状態に強制遷移させる
	currentState_ = State::Cancel;
	restTimer_ = 0.0f;

	// 経過時間表示の更新処理
	restTimerDisplay_->Update(restTimer_);
}

void PlayerStunHUD::Update() {

	// 状態の更新
	UpdateState();
}

void PlayerStunHUD::UpdateState() {

	// 有効の時のみ
	if (!isVaild_) {
		return;
	}

	switch (currentState_) {
	case PlayerStunHUD::State::Begin: {

		UpdateBeginAnimation();
		break;
	}
	case PlayerStunHUD::State::Count: {

		UpdateCount();
		break;
	}
	case PlayerStunHUD::State::Cancel: {

		UpdateCancel();
		break;
	}
	}
}

void PlayerStunHUD::UpdateBeginAnimation() {

	// 時間を経過させる
	beginAnimationTimer_ += GameTimer::GetDeltaTime();
	float lerpT = beginAnimationTimer_ / beginAnimationTime_;
	lerpT = EasedValue(beginAnimationEasingType_, lerpT);

	// サイズを補間する
	SetTargetSize(lerpT);
	// alpha点滅
	float alpha = CalcBlinkAlpha(lerpT, beginAlphaBlinkingCount_);
	SetAlpha(alpha);

	// 時間経過したら次の状態に遷移させる
	if (beginAnimationTime_ < beginAnimationTimer_) {

		// 1.0fに設定する
		SetAlpha(1.0f);

		currentState_ = State::Count;
		beginAnimationTimer_ = 0.0f;
	}
}

void PlayerStunHUD::UpdateCount() {

	// 時間をマイナスしていく
	restTimer_ -= GameTimer::GetDeltaTime();
	float progress = restTimer_ / restTime_;
	// alpha点滅
	float alpha = CalcBlinkAlpha(progress, restAlphaBlinkingCount_);
	for (uint32_t index = 0; index < iconCount_; ++index) {

		stunChainIconRing_[index]->SetAlpha(alpha);
	}
	// 経過率をサイズで表す
	progressBar_->SetSize(Vector2(std::lerp(0.0f, progressBarSize_.x, progress), progressBarSize_.y));

	// 経過時間表示の更新処理
	restTimerDisplay_->Update(restTimer_);

	// 時間経過した == 選択無し
	if (restTimer_ < 0.0f) {

		// キャンセル状態にする
		currentState_ = State::Cancel;
		restTimer_ = 0.0f;
	}
}

void PlayerStunHUD::UpdateCancel() {

	// 時間経過を進める
	cancelTimer_ += GameTimer::GetDeltaTime();
	float lerpT = cancelTimer_ / cancelTime_;
	float easedT = EasedValue(cancelEasingType_, lerpT);

	// 座標を下に下げる
	SetTargetTranslation(easedT);

	// alpha値を0.0fに近づけていく
	SetAlpha(std::clamp(1.0f - lerpT, 0.0f, 1.0f));

	// 時間経過したら全てリセットする
	if (cancelTime_ < cancelTimer_) {

		isVaild_ = false;

		currentState_ = State::Begin;
		cancelTimer_ = 0.0f;
		restTimer_ = restTime_;

		// 座標を元の座標に戻す
		UpdateLayout();
	}
}

void PlayerStunHUD::UpdateLayout() {

	// ProgressBar
	const Vector2 barPos(centerTranslationX_, barTranslationY_);
	progressBarBackground_->SetTranslation(barPos);
	progressBar_->SetTranslation(barPos);
	chainAttackText_->SetTranslation(barPos);

	// Timer
	restTimerDisplay_->SetOffset(Vector2(timerOffsetX_, 0.0f));
	restTimerDisplay_->SetTranslation(
		Vector2(timerTranslationX_, timerTranslationY_));

	// Cancel
	const Vector2 cancelPos(centerTranslationX_, cancelTranslationY_);
	keyInput_.cancel->SetTranslation(cancelPos);
	gamepadInput_.cancel->SetTranslation(cancelPos);

	// 右入力
	const Vector2 rightPos(centerTranslationX_ + endOffsetX_, cancelTranslationY_);
	keyInput_.rightChain->SetTranslation(rightPos);
	gamepadInput_.rightChain->SetTranslation(rightPos);
	stunChainIcon_[0]->SetTranslation(
		Vector2(centerTranslationX_ + endOffsetX_, barTranslationY_));
	stunChainIconRing_[0]->SetTranslation(
		Vector2(centerTranslationX_ + endOffsetX_, barTranslationY_));

	// 左入力
	const Vector2 leftPos(centerTranslationX_ - endOffsetX_, cancelTranslationY_);
	keyInput_.leftChain->SetTranslation(leftPos);
	gamepadInput_.leftChain->SetTranslation(leftPos);
	stunChainIcon_[1]->SetTranslation(
		Vector2(centerTranslationX_ - endOffsetX_, barTranslationY_));
	stunChainIconRing_[1]->SetTranslation(
		Vector2(centerTranslationX_ - endOffsetX_, barTranslationY_));
}

void PlayerStunHUD::UpdateSize() {

	stunChainIcon_[0]->SetSize(iconSize_);
	stunChainIcon_[1]->SetSize(iconSize_);
	stunChainIconRing_[0]->SetSize(iconRingSize_);
	stunChainIconRing_[1]->SetSize(iconRingSize_);

	restTimerDisplay_->SetTimerSize(timerSize_);
	restTimerDisplay_->SetSymbolSize(timerSymbolSize_);

	progressBarBackground_->SetSize(progressBarBackgroundSize_);
	progressBar_->SetSize(progressBarSize_);
	chainAttackText_->SetSize(chainAttackTextSize_);

	// 右入力
	keyInput_.rightChain->SetSize(chainInputSize_);
	gamepadInput_.rightChain->SetSize(chainInputSize_);
	// 左入力
	keyInput_.leftChain->SetSize(chainInputSize_);
	gamepadInput_.leftChain->SetSize(chainInputSize_);
	// キャンセル
	keyInput_.cancel->SetSize(cancelInputSize_);
	gamepadInput_.cancel->SetSize(cancelInputSize_);
}

void PlayerStunHUD::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::Checkbox("isVaild", &isVaild_);

	ImGui::DragFloat("restTime", &restTime_, 0.01f);
	ImGui::DragFloat("restTimer", &restTimer_, 0.01f);
	if (std::numeric_limits<float>::epsilon() < restTime_) {

		ImGui::ProgressBar(restTimer_ / restTime_, ImVec2(256.0f, 0.0f));
	}
	if (ImGui::Button("Reset Timer")) {

		restTimer_ = restTime_;
	}

	ImGui::DragFloat("beginAnimationTime_", &beginAnimationTime_, 0.01f);
	ImGui::DragFloat("cancelTime", &cancelTime_, 0.01f);
	ImGui::DragFloat("cancelUnderOffsetY", &cancelUnderOffsetY_, 1.0f);

	bool layoutChanged = false;

	layoutChanged |= ImGui::DragFloat("timerTranslationX", &timerTranslationX_);
	layoutChanged |= ImGui::DragFloat("barTranslationY", &barTranslationY_);
	Easing::SelectEasingType(beginAnimationEasingType_,"beginAnimationEasingType_");
	Easing::SelectEasingType(cancelEasingType_,"cancelEasingType_");
	layoutChanged |= ImGui::DragFloat("timerTranslationY", &timerTranslationY_);
	layoutChanged |= ImGui::DragFloat("cancelTranslationY", &cancelTranslationY_);
	layoutChanged |= ImGui::DragFloat("endOffsetX", &endOffsetX_);
	layoutChanged |= ImGui::DragFloat("timerOffsetX", &timerOffsetX_);
	layoutChanged |= ImGui::DragInt("beginAlphaBlinkingCount", &beginAlphaBlinkingCount_);
	layoutChanged |= ImGui::DragInt("restAlphaBlinkingCount", &restAlphaBlinkingCount_);
	layoutChanged |= ImGui::DragFloat2("timerSize", &timerSize_.x);
	layoutChanged |= ImGui::DragFloat2("timerSymbolSize", &timerSymbolSize_.x);
	layoutChanged |= ImGui::DragFloat2("progressBarSize", &progressBarSize_.x);
	layoutChanged |= ImGui::DragFloat2("progressBarBackgroundSize", &progressBarBackgroundSize_.x);
	layoutChanged |= ImGui::DragFloat2("chainAttackTextSize", &chainAttackTextSize_.x);
	layoutChanged |= ImGui::DragFloat2("chainInputSize", &chainInputSize_.x);
	layoutChanged |= ImGui::DragFloat2("cancelInputSize", &cancelInputSize_.x);
	layoutChanged |= ImGui::DragFloat2("iconSize", &iconSize_.x);
	layoutChanged |= ImGui::DragFloat2("iconRingSize", &iconRingSize_.x);

	if (ImGui::ColorEdit4("iconRingColor", &iconRingColor_.r) ||
		ImGui::DragFloat("iconRingEmissive", &iconRingEmissive_, 0.1f)) {

		for (uint32_t index = 0; index < iconCount_; ++index) {

			stunChainIconRing_[index]->SetColor(iconRingColor_);
			stunChainIconRing_[index]->SetEmissionColor(
				Vector3(iconRingColor_.r, iconRingColor_.g, iconRingColor_.b));
			stunChainIconRing_[index]->SetEmissiveIntensity(iconRingEmissive_);
		}
	}

	if (layoutChanged) {

		UpdateLayout();
		UpdateSize();

		// 経過時間表示の更新処理
		restTimerDisplay_->Update(restTimer_);
	}
}

void PlayerStunHUD::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Player/stunHudParameter.json", data)) {
		return;
	}

	restTime_ = JsonAdapter::GetValue<float>(data, "restTime_");
	beginAnimationTime_ = JsonAdapter::GetValue<float>(data, "beginAnimationTime_");
	cancelUnderOffsetY_ = JsonAdapter::GetValue<float>(data, "cancelUnderOffsetY_");
	cancelTime_ = JsonAdapter::GetValue<float>(data, "cancelTime_");
	timerTranslationX_ = JsonAdapter::GetValue<float>(data, "timerTranslationX_");
	barTranslationY_ = JsonAdapter::GetValue<float>(data, "barTranslationY_");
	timerTranslationY_ = JsonAdapter::GetValue<float>(data, "timerTranslationY_");
	cancelTranslationY_ = JsonAdapter::GetValue<float>(data, "cancelTranslationY_");
	endOffsetX_ = JsonAdapter::GetValue<float>(data, "endOffsetX_");
	timerOffsetX_ = JsonAdapter::GetValue<float>(data, "timerOffsetX_");
	iconRingEmissive_ = JsonAdapter::GetValue<float>(data, "iconRingEmissive_");
	beginAnimationEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "beginAnimationEasingType_"));
	cancelEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "cancelEasingType_"));
	beginAlphaBlinkingCount_ = JsonAdapter::GetValue<int>(data, "beginAlphaBlinkingCount_");
	restAlphaBlinkingCount_ = JsonAdapter::GetValue<int>(data, "restAlphaBlinkingCount_");
	timerSize_ = JsonAdapter::ToObject<Vector2>(data["timerSize_"]);
	timerSymbolSize_ = JsonAdapter::ToObject<Vector2>(data["timerSymbolSize_"]);
	iconSize_ = JsonAdapter::ToObject<Vector2>(data["iconSize_"]);
	iconRingSize_ = JsonAdapter::ToObject<Vector2>(data["iconRingSize_"]);
	progressBarBackgroundSize_ = JsonAdapter::ToObject<Vector2>(data["progressBarBackgroundSize_"]);
	progressBarSize_ = JsonAdapter::ToObject<Vector2>(data["progressBarSize_"]);
	chainAttackTextSize_ = JsonAdapter::ToObject<Vector2>(data["chainAttackTextSize_"]);
	chainInputSize_ = JsonAdapter::ToObject<Vector2>(data["chainInputSize_"]);
	cancelInputSize_ = JsonAdapter::ToObject<Vector2>(data["cancelInputSize_"]);
	iconRingColor_ = JsonAdapter::ToObject<Color>(data["iconRingColor_"]);

	// 時間を設定
	restTimer_ = restTime_;

	// 座標を設定
	UpdateLayout();
	// サイズを設定
	UpdateSize();

	for (uint32_t index = 0; index < iconCount_; ++index) {

		stunChainIconRing_[index]->SetColor(iconRingColor_);
		stunChainIconRing_[index]->SetEmissionColor(
			Vector3(iconRingColor_.r, iconRingColor_.g, iconRingColor_.b));
		stunChainIconRing_[index]->SetEmissiveIntensity(iconRingEmissive_);
	}
}

void PlayerStunHUD::SaveJson() {

	Json data;

	data["restTime_"] = restTime_;
	data["beginAnimationTime_"] = beginAnimationTime_;
	data["cancelUnderOffsetY_"] = cancelUnderOffsetY_;
	data["cancelTime_"] = cancelTime_;
	data["timerTranslationX_"] = timerTranslationX_;
	data["barTranslationY_"] = barTranslationY_;
	data["timerTranslationY_"] = timerTranslationY_;
	data["cancelTranslationY_"] = cancelTranslationY_;
	data["endOffsetX_"] = endOffsetX_;
	data["timerOffsetX_"] = timerOffsetX_;
	data["beginAlphaBlinkingCount_"] = beginAlphaBlinkingCount_;
	data["restAlphaBlinkingCount_"] = restAlphaBlinkingCount_;
	data["iconRingEmissive_"] = iconRingEmissive_;
	data["beginAnimationEasingType_"] = static_cast<int>(beginAnimationEasingType_);
	data["cancelEasingType_"] = static_cast<int>(cancelEasingType_);
	data["timerSize_"] = JsonAdapter::FromObject<Vector2>(timerSize_);
	data["timerSymbolSize_"] = JsonAdapter::FromObject<Vector2>(timerSymbolSize_);
	data["iconSize_"] = JsonAdapter::FromObject<Vector2>(iconSize_);
	data["iconRingSize_"] = JsonAdapter::FromObject<Vector2>(iconRingSize_);
	data["progressBarBackgroundSize_"] = JsonAdapter::FromObject<Vector2>(progressBarBackgroundSize_);
	data["progressBarSize_"] = JsonAdapter::FromObject<Vector2>(progressBarSize_);
	data["chainAttackTextSize_"] = JsonAdapter::FromObject<Vector2>(chainAttackTextSize_);
	data["chainInputSize_"] = JsonAdapter::FromObject<Vector2>(chainInputSize_);
	data["cancelInputSize_"] = JsonAdapter::FromObject<Vector2>(cancelInputSize_);
	data["iconRingColor_"] = JsonAdapter::FromObject<Color>(iconRingColor_);

	JsonAdapter::Save("Player/stunHudParameter.json", data);
}

void PlayerStunHUD::ChainInput::Init(const std::string& rightTex, const std::string& leftTex,
	const std::string& cancelTex) {

	rightChain = std::make_unique<GameObject2D>();
	rightChain->Init(rightTex, rightTex, "PlayerStunHUD");

	leftChain = std::make_unique<GameObject2D>();
	leftChain->Init(leftTex, leftTex, "PlayerStunHUD");

	cancel = std::make_unique<GameObject2D>();
	cancel->Init(cancelTex, cancelTex, "PlayerStunHUD");
}

void PlayerStunHUD::ChainInput::SetSize(const Vector2& size) {

	rightChain->SetSize(size);
	leftChain->SetSize(size);
}