#include "GameResultDisplay.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Utility/JsonAdapter.h>
#include <Engine/Utility/EnumAdapter.h>
#include <Engine/Config.h>
#include <Game/Objects/Common/GameButtonBlinkingUpdater.h>
#include <Lib/Adapter/RandomGenerator.h>

//============================================================================
//	GameResultDisplay classMethods
//============================================================================

void GameResultDisplay::Init() {

	// スプライト初期化
	// 背景
	background_ = std::make_unique<GameObject2D>();
	background_->Init("bgCheckerboard", "background", "GameResultDisplay");
	background_->SetCenterTranslation();
	background_->SetSize(Vector2(Config::kWindowWidthf, Config::kWindowHeightf));
	background_->SetUVScaleX(96.0f);
	background_->SetUVScaleY(80.0f);
	// 最初は表示しない
	background_->SetAlpha(0.0f);

	// 時間表示
	resultTime_ = std::make_unique<GameTimerDisplay>();
	resultTime_->Init("dd:dd:dd", "toughnessNumber",
		"toughnessTimeSymbol", "resultTime", "GameResultDisplay");
	// 最初は表示しない
	resultTime_->SetAlpha(0.0f);

	// クリア文字表示
	clearText_ = std::make_unique<GameObject2D>();
	clearText_->Init("clearText", "clearText", "GameResultDisplay");
	clearText_->SetEmissiveIntensity(1.72f);
	// 最初は表示しない
	clearText_->SetAlpha(0.0f);

	// ボタン
	// 左
	leftButton_ = std::make_unique<GameButton>();
	leftButton_->Init("titleButton", "GameResultDisplay");
	leftButton_->SetAlpha(0.0f);
	leftButton_->RegisterUpdater(GameButtonResponseType::AnyMouseClick,
		std::make_unique<GameButtonBlinkingUpdater>());
	leftButton_->RegisterUpdater(GameButtonResponseType::Focus,
		std::make_unique<GameButtonBlinkingUpdater>());
	// 右
	rightButton_ = std::make_unique<GameButton>();
	rightButton_->Init("retryButton", "GameResultDisplay");
	rightButton_->SetAlpha(0.0f);
	rightButton_->RegisterUpdater(GameButtonResponseType::AnyMouseClick,
		std::make_unique<GameButtonBlinkingUpdater>());
	rightButton_->RegisterUpdater(GameButtonResponseType::Focus,
		std::make_unique<GameButtonBlinkingUpdater>());

	// アニメーション初期化
	InitAnimations();

	// ナビゲーター初期化
	InitNavigator();

	// json適応
	ApplyJson();
}

void GameResultDisplay::InitAnimations() {

	// アニメーションの初期化
	clearPosAnim_ = std::make_unique<SimpleAnimation<Vector2>>();
	clearSizeAnim_ = std::make_unique<SimpleAnimation<Vector2>>();
	clearTimePosAnim_ = std::make_unique<SimpleAnimation<Vector2>>();
	leftButtonAnim_ = std::make_unique<SimpleAnimation<Vector2>>();
	rightButtonAnim_ = std::make_unique<SimpleAnimation<Vector2>>();


}

void GameResultDisplay::InitNavigator() {

	buttonFocusNavigator_ = std::make_unique<GamecButtonFocusNavigator>();

	// 入力処理に応じた処理の設定
	buttonFocusNavigator_->SetOnConfirm([&](ButtonFocusGroup, int index) {
		if (index == 0) {

			// 左ボタンの決定
			ConfirmLeftByPad();
		} else {

			// 右ボタンの決定
			ConfirmRightByPad();
		}
		});
	buttonFocusNavigator_->Init(ButtonFocusGroup::Top, { leftButton_.get(), rightButton_.get() });
}

void GameResultDisplay::Measurement() {

	// 時間計測中...
	resultTimer_.Update();
}

void GameResultDisplay::StartDisplay() {

	// 表示する
	currentState_ = State::BeginTime;
	resultTime_->SetAlpha(1.0f);
	background_->SetColor(Color(0.034f, 0.034f, 0.034f, 0.741f));
	randomDisplayTimer_.Reset();
	randomSwitchIndex_ = 0;

	// 最初の乱数値
	lastRandomTime_ = RandomGenerator::Generate<float>(0.0f, randomTimeMax_);

	// 次の切り替え閾値
	auto calcThreshold = [&](int index) {
		float u = float(index + 1) / float(randomSwitchCount_ + 1);
		return std::pow(u, randomSwitchBias_); };
	nextSwitchT_ = (randomSwitchCount_ > 0) ? calcThreshold(0) : 1.0f;
}

void GameResultDisplay::Update() {

	// 各状態の更新
	switch (currentState_) {
	case GameResultDisplay::State::BeginTime: {

		UpdateBeginTime();
		break;
	}
	case GameResultDisplay::State::Result: {

		UpdateResult();
		break;
	}
	case GameResultDisplay::State::Select: {

		// パッド入力の更新
		UpdateInputGamepad();

		// ボタンの更新
		leftButton_->Update();
		rightButton_->Update();
		break;
	}
	}
}

void GameResultDisplay::UpdateBeginTime() {

	// 時間を進める
	randomDisplayTimer_.Update();

	// 最終的に表示する値
	const float resultTime = resultTimer_.current_ / 100.0f;

	// 収束値を設定
	const float maxSpanAtStart = (std::max)(resultTime, randomTimeMax_ - resultTime);
	const float span = maxSpanAtStart * (1.0f - randomDisplayTimer_.easedT_);

	float min = (std::max)(0.0f, resultTime - span);
	float max = (std::min)(randomTimeMax_, resultTime + span);

	// 切り替え予約
	auto calcThreshold = [&](int index) {
		float u = float(index + 1) / float(randomSwitchCount_ + 1);
		return std::pow(u, randomSwitchBias_); };

	float progress = randomDisplayTimer_.easedT_;
	while (randomSwitchIndex_ < randomSwitchCount_ && progress >= nextSwitchT_) {

		// 区間内で新しい乱数をセット
		lastRandomTime_ = RandomGenerator::Generate<float>(min, max);
		++randomSwitchIndex_;
		nextSwitchT_ = (randomSwitchIndex_ < randomSwitchCount_) ?
			calcThreshold(randomSwitchIndex_) : 1.0f;
	}

	// 表示値する値を設定
	const float display = randomDisplayTimer_.IsReached() ?
		resultTime : lastRandomTime_ / 100.0f;

	resultTime_->Update(display);

	// タイマーが終わったらしばらく表示後状態遷移
	if (randomDisplayTimer_.IsReached()) {

		// 待ち時間
		displayWaitTimer_.Update();
		if (0.5f < displayWaitTimer_.t_) {

			resultTime_->SetAlpha(0.0f);
			resultTime_->SetOffset(Vector2(timerOffsetX_ / timeResultOffsetScale_, 0.0f));
			resultTime_->SetTimerSize(timerSize_ / timeResultSizeScale_);
			resultTime_->SetSymbolSize(timerSymbolSize_ / timeResultSizeScale_);
		}

		if (displayWaitTimer_.IsReached()) {

			currentState_ = State::Result;

			// アニメーション開始
			clearPosAnim_->Start();
			clearSizeAnim_->Start();
			clearTimePosAnim_->Start();
			leftButtonAnim_->Start();
			rightButtonAnim_->Start();
			clearText_->SetAlpha(1.0f);
		}
	}
}

void GameResultDisplay::UpdateResult() {

	// クリア文字
	{
		Vector2 lerpPos{};
		clearPosAnim_->LerpValue(lerpPos);
		clearText_->SetTranslation(lerpPos);

		Vector2 lerpSize{};
		clearSizeAnim_->LerpValue(lerpSize);
		clearText_->SetSize(lerpSize);
	}
	// クリア時間
	{
		Vector2 lerpPos{};
		clearTimePosAnim_->LerpValue(lerpPos);
		resultTime_->SetTranslation(lerpPos);
		resultTime_->SetAlpha(EasedValue(EasingType::EaseInExpo, clearTimePosAnim_->GetProgress()));
	}

	// ボタン
	{
		// 左
		Vector2 lerpPos{};
		leftButtonAnim_->LerpValue(lerpPos);
		leftButton_->SetTranslation(lerpPos);
		leftButton_->SetAlpha(EasedValue(EasingType::EaseInExpo, leftButtonAnim_->GetProgress()));
		// 右
		rightButtonAnim_->LerpValue(lerpPos);
		rightButton_->SetTranslation(lerpPos);
		rightButton_->SetAlpha(EasedValue(EasingType::EaseInExpo, rightButtonAnim_->GetProgress()));
	}

	// 補間終了後
	if (leftButtonAnim_->IsFinished() &&
		rightButtonAnim_->IsFinished()) {

		currentState_ = State::Select;

		// ボタンを有効化する
		buttonFocusNavigator_->SetGroup(ButtonFocusGroup::Top,
			{ leftButton_.get(), rightButton_.get() }, 0);
	}
}

void GameResultDisplay::UpdateInputGamepad() {

	const auto& type = Input::GetInstance()->GetType();
	const bool nowGamepad = (type != InputType::Keyboard);

	if (!nowGamepad) {

		// マウスに切り替わった瞬間だけ全フォーカス解除
		if (wasGamepad_) {
			buttonFocusNavigator_->SetGroup(ButtonFocusGroup::Top,
				{ leftButton_.get(), rightButton_.get() }, 0);
		}

		// マウス判定を有効化
		leftButton_->SetEnableCollision(true);
		rightButton_->SetEnableCollision(true);

		wasGamepad_ = false;
		return;
	}

	// パッド操作入力更新
	buttonFocusNavigator_->Update();
	wasGamepad_ = true;
}

void GameResultDisplay::ConfirmLeftByPad() {

	// タイトルに戻す
	resultSelect_ = ResultSelect::Title;
}

void GameResultDisplay::ConfirmRightByPad() {

	// ゲーム再プレイ
	resultSelect_ = ResultSelect::Retry;
}

void GameResultDisplay::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::DragFloat("resultTimer_.current", &resultTimer_.current_, 0.1f);
	ImGui::DragFloat("randomTimeMax", &randomTimeMax_, 0.1f);
	ImGui::DragInt("randomSwitchCount", &randomSwitchCount_, 1);
	ImGui::DragFloat("randomSwitchBias", &randomSwitchBias_, 0.05f);
	ImGui::Text("switch %d / %d", randomSwitchIndex_, randomSwitchCount_);

	if (EnumAdapter<State>::Combo("currentState", &currentState_)) {

		if (currentState_ == State::BeginTime) {

			clearPosAnim_->Reset();
			clearSizeAnim_->Reset();
			clearTimePosAnim_->Reset();
			leftButtonAnim_->Reset();
			rightButtonAnim_->Reset();

			randomDisplayTimer_.Reset();
			displayWaitTimer_.Reset();
			randomSwitchIndex_ = 0;
			// 最初の乱数値
			lastRandomTime_ = RandomGenerator::Generate<float>(0.0f, randomTimeMax_);

			// 次の切り替え閾値
			auto calcThreshold = [&](int index) {
				float u = float(index + 1) / float(randomSwitchCount_ + 1);
				return std::pow(u, randomSwitchBias_); };
			nextSwitchT_ = (randomSwitchCount_ > 0) ? calcThreshold(0) : 1.0f;

			resultTime_->SetAlpha(1.0f);
			resultTime_->SetOffset(Vector2(timerOffsetX_, 0.0f));
			resultTime_->SetTranslation(timerTranslation_);
			resultTime_->SetTimerSize(timerSize_);
			resultTime_->SetSymbolSize(timerSymbolSize_);
		}
	}

	ImGui::SeparatorText("BeginTime");
	{

		ImGui::DragFloat("timeResultOffsetScale", &timeResultOffsetScale_, 0.01f);
		ImGui::DragFloat("timeResultSizeScale", &timeResultSizeScale_, 0.01f);
		if (ImGui::DragFloat2("timerTranslation", &timerTranslation_.x, 1.0f)) {

			resultTime_->SetTranslation(timerTranslation_);
		}
		if (ImGui::DragFloat("timerOffsetX", &timerOffsetX_, 0.1f)) {

			resultTime_->SetOffset(Vector2(timerOffsetX_, 0.0f));
		}
		if (ImGui::DragFloat2("timerSize", &timerSize_.x, 0.1f)) {

			resultTime_->SetTimerSize(timerSize_);
		}
		if (ImGui::DragFloat2("timerSymbolSize", &timerSymbolSize_.x, 0.1f)) {

			resultTime_->SetSymbolSize(timerSymbolSize_);
		}

		randomDisplayTimer_.ImGui("RandomDisplayTime");
		displayWaitTimer_.ImGui("DisplayWaitTime");
	}

	ImGui::SeparatorText("Result");

	ImGui::DragFloat2("buttonSize", &buttonSize_.x, 0.1f);
	if (ImGui::BeginTabBar("ResultTab")) {
		if (ImGui::BeginTabItem("Animations")) {

			ImGui::SeparatorText("ClearText");

			clearPosAnim_->ImGui("ClearPosAnim");
			clearSizeAnim_->ImGui("ClearSizeAnim");

			ImGui::SeparatorText("Time");

			clearTimePosAnim_->ImGui("ClearTimePosAnim");

			ImGui::SeparatorText("Time");

			leftButtonAnim_->ImGui("LeftButtonPosAnim");
			rightButtonAnim_->ImGui("RightButtonPosAnim");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("LeftButton")) {

			leftButton_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("RightButton")) {

			rightButton_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void GameResultDisplay::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Result/resultDisplay.json", data)) {
		return;
	}

	timerTranslation_ = Vector2::FromJson(data.value("timerTranslation_", Json()));
	timerOffsetX_ = data.value("timerOffsetX_", 4.0f);
	randomTimeMax_ = data.value("randomTimeMax_", 4.0f);
	randomSwitchCount_ = data.value("randomSwitchCount_", 10);
	randomSwitchBias_ = data.value("randomSwitchBias_", 1.5f);
	timeResultOffsetScale_ = data.value("timeResultOffsetScale_", 1.5f);
	timeResultSizeScale_ = data.value("timeResultSizeScale_", 1.5f);
	timerSize_ = Vector2::FromJson(data.value("timerSize_", Json()));
	timerSymbolSize_ = Vector2::FromJson(data.value("timerSymbolSize_", Json()));
	buttonSize_ = Vector2::FromJson(data.value("buttonSize_", Json()));
	randomDisplayTimer_.FromJson(data["RandomDisplayTime"]);
	displayWaitTimer_.FromJson(data.value("DisplayWaitTime", Json()));

	// 値を適応
	resultTime_->SetOffset(Vector2(timerOffsetX_, 0.0f));
	resultTime_->SetTranslation(timerTranslation_);
	resultTime_->SetTimerSize(timerSize_);
	resultTime_->SetSymbolSize(timerSymbolSize_);
	leftButton_->SetSize(buttonSize_);
	rightButton_->SetSize(buttonSize_);

	clearPosAnim_->FromJson(data.value("ClearPosAnim", Json()));
	clearSizeAnim_->FromJson(data.value("ClearSizeAnim", Json()));
	clearTimePosAnim_->FromJson(data.value("ClearTimePosAnim", Json()));
	leftButtonAnim_->FromJson(data.value("LeftButtonPosAnim", Json()));
	rightButtonAnim_->FromJson(data.value("RightButtonPosAnim", Json()));
	rightButtonAnim_->FromJson(data.value("RightButtonPosAnim", Json()));
	leftButton_->FromJson(data.value("LeftButton", Json()));
	rightButton_->FromJson(data.value("RightButton", Json()));
}

void GameResultDisplay::SaveJson() {

	Json data;

	// BeginTime
	data["timerTranslation_"] = timerTranslation_.ToJson();
	data["timerOffsetX_"] = timerOffsetX_;
	data["randomTimeMax_"] = randomTimeMax_;
	data["randomSwitchCount_"] = randomSwitchCount_;
	data["randomSwitchBias_"] = randomSwitchBias_;
	data["timeResultOffsetScale_"] = timeResultOffsetScale_;
	data["timeResultSizeScale_"] = timeResultSizeScale_;
	data["timerSize_"] = timerSize_.ToJson();
	data["timerSymbolSize_"] = timerSymbolSize_.ToJson();
	data["buttonSize_"] = buttonSize_.ToJson();
	randomDisplayTimer_.ToJson(data["RandomDisplayTime"]);
	displayWaitTimer_.ToJson(data["DisplayWaitTime"]);

	// Result
	clearPosAnim_->ToJson(data["ClearPosAnim"]);
	clearSizeAnim_->ToJson(data["ClearSizeAnim"]);
	clearTimePosAnim_->ToJson(data["ClearTimePosAnim"]);
	leftButtonAnim_->ToJson(data["LeftButtonPosAnim"]);
	rightButtonAnim_->ToJson(data["RightButtonPosAnim"]);
	leftButton_->ToJson(data["LeftButton"]);
	rightButton_->ToJson(data["RightButton"]);

	JsonAdapter::Save("Result/resultDisplay.json", data);
}