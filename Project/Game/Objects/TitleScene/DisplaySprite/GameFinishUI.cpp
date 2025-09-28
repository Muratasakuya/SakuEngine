#include "GameFinishUI.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Objects/Common/GameButtonBlinkingUpdater.h>

// imgui
#include <imgui.h>

//============================================================================
//	GameFinishUI classMethods
//============================================================================

void GameFinishUI::InitSprites() {

	const std::string groupName = "GameFinishUI";

	// 使用するスプライトの生成
	// 電源
	powerIcon_ = std::make_unique<GameButton>();
	powerIcon_->Init("powerIcon", groupName);
	powerIcon_->RegisterUpdater(GameButtonResponseType::AnyMouseClick,
		std::make_unique<GameButtonBlinkingUpdater>());
	powerIcon_->RegisterUpdater(GameButtonResponseType::Focus,
		std::make_unique<GameButtonBlinkingUpdater>());

	// 終了しますか表示の背景
	askFinishBackground_ = std::make_unique<GameObject2D>();
	askFinishBackground_->Init("gameFinishTextBackground", "gameFinishTextBackground", groupName);

	// 終了しますか表示
	askFinish_ = std::make_unique<GameObject2D>();
	askFinish_->Init("gameFinishText", "gameFinishText", groupName);

	// キャンセル
	selectCancel_ = std::make_unique<GameButton>();
	selectCancel_->Init("cancelText", groupName);
	selectCancel_->RegisterUpdater(GameButtonResponseType::AnyMouseClick,
		std::make_unique<GameButtonBlinkingUpdater>());
	selectCancel_->RegisterUpdater(GameButtonResponseType::Focus,
		std::make_unique<GameButtonBlinkingUpdater>());

	// OK -> Finish
	selectOK_ = std::make_unique<GameButton>();
	selectOK_->Init("okText", groupName);
	selectOK_->RegisterUpdater(GameButtonResponseType::AnyMouseClick,
		std::make_unique<GameButtonBlinkingUpdater>());
	selectOK_->RegisterUpdater(GameButtonResponseType::Focus,
		std::make_unique<GameButtonBlinkingUpdater>());
}

void GameFinishUI::InitAnimations() {

	// 各アニメーションの初期化
	finishSizeAnimation_ = std::make_unique<SimpleAnimation<Vector2>>();
	finishBackgroundSizeAnimation_ = std::make_unique<SimpleAnimation<Vector2>>();
	selectCancelSizeAnimation_ = std::make_unique<SimpleAnimation<Vector2>>();
	selectOKSizeAnimation_ = std::make_unique<SimpleAnimation<Vector2>>();
}

void GameFinishUI::InitAnimationSize() {

	// 補間サイズを設定
	finishSizeAnimation_->SetStart(Vector2(askFinish_->GetSize().x, 0.0f));
	finishSizeAnimation_->SetEnd(askFinish_->GetSize());

	finishBackgroundSizeAnimation_->SetStart(Vector2(askFinishBackground_->GetSize().x, 0.0f));
	finishBackgroundSizeAnimation_->SetEnd(askFinishBackground_->GetSize());

	selectCancelSizeAnimation_->SetStart(Vector2(selectCancel_->GetSize().x, 0.0f));
	selectCancelSizeAnimation_->SetEnd(selectCancel_->GetSize());

	selectOKSizeAnimation_->SetStart(Vector2(selectOK_->GetSize().x, 0.0f));
	selectOKSizeAnimation_->SetEnd(selectOK_->GetSize());
}

void GameFinishUI::SetSpritePos() {

	// 各スプライトの座標を設定する
	// 電源
	Vector2 powerPos = powerIconSize_ + (powerIconSize_ / 2.0f);
	powerPos.y = Config::kWindowHeightf - powerPos.y;
	powerIcon_->SetTranslation(powerPos);
	// 終了しますか表示の背景
	askFinishBackground_->SetCenterTranslation();
	// 終了しますか表示
	askFinish_->SetCenterTranslation();

	const Vector2 backgroundPos = askFinishBackground_->GetTranslation();
	const float selectPosY = backgroundPos.y + (askFinishBackground_->GetSize().y / 2.0f);

	// キャンセル
	selectCancel_->SetTranslation(Vector2(backgroundPos.x - selectButtonSpacing_, selectPosY));
	// OK -> Finish
	selectOK_->SetTranslation(Vector2(backgroundPos.x + selectButtonSpacing_, selectPosY));
}

void GameFinishUI::ConfirmCancelByPad() {

	if (currentState_ != State::Select ||
		currentSelectState_ != SelectState::Select) {
		return;
	}

	// パッド操作によるボタンの選択
	currentSelectState_ = SelectState::Decide;
	selectCancel_->SetEnableCollision(false);
	selectOK_->SetEnableCollision(false);

	ForEachAnimations([](SimpleAnimation<Vector2>* animations) {
		animations->Reset(); });
	ForEachAnimations([](SimpleAnimation<Vector2>* animations) {
		animations->SetAnimationType(SimpleAnimationType::Return); });
}

void GameFinishUI::ConfirmOKByPad() {

	if (currentState_ != State::Select ||
		currentSelectState_ != SelectState::Select) {
		return;
	}

	// パッド操作によるボタンの選択
	currentState_ = State::Finish;
	selectCancel_->SetEnableCollision(false);
	selectOK_->SetEnableCollision(false);

	// 表示を消す
	askFinish_->SetSize(Vector2::AnyInit(0.0f));
	askFinishBackground_->SetSize(Vector2::AnyInit(0.0f));
	selectCancel_->SetSize(Vector2::AnyInit(0.0f));
	selectOK_->SetSize(Vector2::AnyInit(0.0f));
}

void GameFinishUI::Init() {

	// スプライトの初期化
	InitSprites();
	// アニメーションの初期化
	InitAnimations();

	// 初期化値設定
	currentState_ = State::Power;
	currentSelectState_ = SelectState::Begin;
}

void GameFinishUI::Update() {

	// 判定更新
	UpdateEnableCollision();
	powerIcon_->Update();
	if (currentState_ == State::Select) {

		selectCancel_->Update();
		selectOK_->Update();
	}

	// ボタン選択のチェック
	CheckSelect();

	// 状態別の更新処理
	UpdateState();
}

void GameFinishUI::CheckSelect() {

	// 電源ボタンを押したらセレクト状態にする
	if (powerIcon_->GetHoverAtRelease()) {

		currentState_ = State::Select;
	}
}

void GameFinishUI::UpdateState() {

	switch (currentState_) {
	case GameFinishUI::State::Power: {
		break;
	}
	case GameFinishUI::State::Select: {

		// 選択
		UpdateSelect();
		break;
	}
	case GameFinishUI::State::Finish: {
		break;
	}
	}
}

void GameFinishUI::UpdateSelect() {

	switch (currentSelectState_) {
	case GameFinishUI::SelectState::Begin:

		// アニメーションの開始
		StartSizeAnimations();
		// サイズを補間する
		LerpSelectSpriteSize();

		// アニメーション終了後次に進める
		if (IsFinishedAllAnimations()) {

			currentSelectState_ = SelectState::Select;
		}
		break;
	case GameFinishUI::SelectState::Select:

		// キャンセルかOKを選択
		CheckGameFinish();
		break;
	case GameFinishUI::SelectState::Decide:


		// どちらかを選択したら表示を消す
		DisableSelectSprites();
		break;
	}
}

void GameFinishUI::CheckGameFinish() {

	// キャンセル入力
	if (selectCancel_->GetHoverAtRelease()) {

		// 選択済み
		currentSelectState_ = SelectState::Decide;
		selectCancel_->SetEnableCollision(false);
		selectOK_->SetEnableCollision(false);

		// アニメーションリセット
		ForEachAnimations([](SimpleAnimation<Vector2>* animations) {
			animations->Reset(); });

		// 反対に補間させる
		ForEachAnimations([](SimpleAnimation<Vector2>* animations) {
			animations->SetAnimationType(SimpleAnimationType::Return); });
		return;
	}
	// OK入力
	if (selectOK_->GetHoverAtRelease()) {

		// ゲーム終了、この時点でゲームを閉じる
		currentState_ = State::Finish;
		selectCancel_->SetEnableCollision(false);
		selectOK_->SetEnableCollision(false);

		// サイズも0.0fにして表示を消しておく
		askFinish_->SetSize(Vector2::AnyInit(0.0f));
		askFinishBackground_->SetSize(Vector2::AnyInit(0.0f));
		selectCancel_->SetSize(Vector2::AnyInit(0.0f));
		selectOK_->SetSize(Vector2::AnyInit(0.0f));
	}
}

void GameFinishUI::DisableSelectSprites() {

	// アニメーションが終了したら電源状態に戻す
	if (!IsFinishedAllAnimations()) {

		// アニメーションの開始
		StartSizeAnimations();

		// サイズ補間
		LerpSelectSpriteSize();
		return;
	}

	// 状態を元に戻す
	currentState_ = State::Power;
	currentSelectState_ = SelectState::Begin;

	// アニメーションリセット
	ForEachAnimations([](SimpleAnimation<Vector2>* animations) {
		animations->Reset(); });

	// 元通りに補間させる
	ForEachAnimations([](SimpleAnimation<Vector2>* animations) {
		animations->SetAnimationType(SimpleAnimationType::None); });

	// 表示を消す
	askFinish_->SetSize({ askFinish_->GetSize().x, 0.0f });
	askFinishBackground_->SetSize({ askFinishBackground_->GetSize().x, 0.0f });
	selectCancel_->SetSize({ selectCancel_->GetSize().x, 0.0f });
	selectOK_->SetSize({ selectOK_->GetSize().x, 0.0f });
}

void GameFinishUI::UpdateEnableCollision() {

	// 状態に応じて判定を取れないようにする
	switch (currentState_) {
	case State::Power: powerIcon_->SetEnableCollision(true);  break;
	default: powerIcon_->SetEnableCollision(false); break;
	}

	const bool enableSelect = (currentState_ == State::Select) &&
		(currentSelectState_ != SelectState::Decide);
	selectCancel_->SetEnableCollision(enableSelect);
	selectOK_->SetEnableCollision(enableSelect);
}

void GameFinishUI::LerpSelectSpriteSize() {

	// サイズ補間
	{
		Vector2 size = askFinish_->GetSize();
		finishSizeAnimation_->LerpValue(size);
		askFinish_->SetSize(size);
	}
	{
		Vector2 size = askFinishBackground_->GetSize();
		finishBackgroundSizeAnimation_->LerpValue(size);
		askFinishBackground_->SetSize(size);
	}
	{
		Vector2 size = selectCancel_->GetSize();
		selectCancelSizeAnimation_->LerpValue(size);
		selectCancel_->SetSize(size);
	}
	{
		Vector2 size = selectOK_->GetSize();
		selectOKSizeAnimation_->LerpValue(size);
		selectOK_->SetSize(size);
	}

	// サイズ更新と一緒に座標も更新する
	SetSpritePos();
}

void GameFinishUI::StartSizeAnimations() {

	if (!finishSizeAnimation_->IsStart()) {

		ForEachAnimations([](SimpleAnimation<Vector2>* animations) {
			animations->Start(); });
	}
}

bool GameFinishUI::IsFinishedAllAnimations() {

	bool result = finishSizeAnimation_->IsFinished() &&
		finishBackgroundSizeAnimation_->IsFinished() &&
		selectCancelSizeAnimation_->IsFinished() &&
		selectOKSizeAnimation_->IsFinished();

	return result;
}

void GameFinishUI::ImGui() {

	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));
	ImGui::Text("selectState:  %s", EnumAdapter<SelectState>::ToString(currentSelectState_));

	ImGui::Text(std::format("finishedAllAnimation: {}", IsFinishedAllAnimations()).c_str());

	if (ImGui::DragFloat("selectButtonSpacing", &selectButtonSpacing_, 0.1f)) {

		SetSpritePos();
	}

	// サイズ変更が必要な時に
	ImGuiSize();

	ImGui::SeparatorText("Button Settings");

	if (ImGui::BeginTabBar("TitleDisplaySprite")) {
		if (ImGui::BeginTabItem("Power")) {

			powerIcon_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Cancel")) {

			selectCancel_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("OK")) {

			selectOK_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Animations")) {

			ImGui::SeparatorText("finishSizeAnimation");
			finishSizeAnimation_->ImGui("finishSizeAnimation");

			ImGui::SeparatorText("finishBackgroundSizeAnimation");
			finishBackgroundSizeAnimation_->ImGui("finishBackgroundSizeAnimation");

			ImGui::SeparatorText("selectCancelSizeAnimation");
			selectCancelSizeAnimation_->ImGui("selectCancelSizeAnimation");

			ImGui::SeparatorText("selectOKSizeAnimation");
			selectOKSizeAnimation_->ImGui("selectOKSizeAnimation");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void GameFinishUI::ImGuiSize() {

	bool edit = false;
	edit |= powerIcon_->ImGuiSize();
	edit |= askFinishBackground_->ImGuiSize();
	edit |= askFinish_->ImGuiSize();
	edit |= selectCancel_->ImGuiSize();
	edit |= selectOK_->ImGuiSize();

	if (edit) {

		SetSpritePos();
	}
}

void GameFinishUI::ApplyJson(const Json& data) {

	selectButtonSpacing_ = data.value("selectButtonSpacing_", 4.0f);

	powerIconSize_ = JsonAdapter::ToObject<Vector2>(data["powerIcon_Size"]);
	powerIcon_->SetSize(powerIconSize_);
	askFinishBackground_->SetSize(JsonAdapter::ToObject<Vector2>(data["askFinishBackground_Size"]));
	askFinish_->SetSize(JsonAdapter::ToObject<Vector2>(data["askFinish_Size"]));
	selectCancel_->SetSize(JsonAdapter::ToObject<Vector2>(data["selectCancel_Size"]));
	selectOK_->SetSize(JsonAdapter::ToObject<Vector2>(data["selectOK_Size"]));

	if (data.contains("powerIcon_")) {

		powerIcon_->FromJson(data["powerIcon_"]);
	}
	if (data.contains("selectCancel_")) {

		selectCancel_->FromJson(data["selectCancel_"]);
	}
	if (data.contains("selectOK_")) {

		selectOK_->FromJson(data["selectOK_"]);
	}

	if (data.contains("Animations")) {

		finishSizeAnimation_->FromJson(data["Animations"]["finishSizeAnimation_"]);
		finishBackgroundSizeAnimation_->FromJson(data["Animations"]["finishBackgroundSizeAnimation_"]);
		selectCancelSizeAnimation_->FromJson(data["Animations"]["selectCancelSizeAnimation_"]);
		selectOKSizeAnimation_->FromJson(data["Animations"]["selectOKSizeAnimation_"]);
	}

	// 目標サイズを設定する
	InitAnimationSize();

	// 値設定
	SetSpritePos();

	// 最初は表示しないのでサイズを0.0fにする
	askFinish_->SetSize(Vector2(askFinish_->GetSize().x, 0.0f));
	askFinishBackground_->SetSize(Vector2(askFinishBackground_->GetSize().x, 0.0f));
	selectCancel_->SetSize(Vector2(selectCancel_->GetSize().x, 0.0f));
	selectOK_->SetSize(Vector2(selectOK_->GetSize().x, 0.0f));
}

void GameFinishUI::SaveJson(Json& data) {

	data["selectButtonSpacing_"] = selectButtonSpacing_;

	data["powerIcon_Size"] = powerIcon_->GetSize().ToJson();
	data["askFinishBackground_Size"] = askFinishBackground_->GetSize().ToJson();
	data["askFinish_Size"] = askFinish_->GetSize().ToJson();
	data["selectCancel_Size"] = selectCancel_->GetSize().ToJson();
	data["selectOK_Size"] = selectOK_->GetSize().ToJson();

	powerIcon_->ToJson(data["powerIcon_"]);
	selectCancel_->ToJson(data["selectCancel_"]);
	selectOK_->ToJson(data["selectOK_"]);

	finishSizeAnimation_->ToJson(data["Animations"]["finishSizeAnimation_"]);
	finishBackgroundSizeAnimation_->ToJson(data["Animations"]["finishBackgroundSizeAnimation_"]);
	selectCancelSizeAnimation_->ToJson(data["Animations"]["selectCancelSizeAnimation_"]);
	selectOKSizeAnimation_->ToJson(data["Animations"]["selectOKSizeAnimation_"]);
}

bool GameFinishUI::IsSelectFinish() const {

	bool result = currentState_ == State::Finish;
	return result;
}