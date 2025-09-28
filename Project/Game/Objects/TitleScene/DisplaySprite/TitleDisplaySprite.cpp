#include "TitleDisplaySprite.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Input/Input.h>
#include <Engine/Collision/CollisionGeometry.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Objects/Common/GameButtonBlinkingUpdater.h>

//============================================================================
//	TitleDisplaySprite classMethods
//============================================================================

void TitleDisplaySprite::InitSprites() {

	const std::string groupName = "TitleDisplaySprite";

	// 使用するスプライトの生成
	name_ = std::make_unique<GameObject2D>();
	name_->Init("title", "title", groupName);

	start_ = std::make_unique<GameButton>();
	start_->Init("gameStartText", groupName);
	start_->RegisterUpdater(GameButtonResponseType::AnyMouseClick,
		std::make_unique<GameButtonBlinkingUpdater>());
	start_->RegisterUpdater(GameButtonResponseType::Focus,
		std::make_unique<GameButtonBlinkingUpdater>());

	operateDevice_ = std::make_unique<GameObject2D>();
	operateDevice_->Init("gamePadOrMouse", "gamePadOrMouse", groupName);
}

void TitleDisplaySprite::InitNavigator() {

	buttonFocusNavigator_ = std::make_unique<GamecButtonFocusNavigator>();

	// 入力処理に応じた処理の設定
	buttonFocusNavigator_->SetOnConfirm([&](ButtonFocusGroup group, int index) {
		if (group == ButtonFocusGroup::Top) {
			if (index == 0) {

				// スタート入力時の処理
			} else {

				finishUI_->ConfirmPowerByPad();
			}
		} else if (group == ButtonFocusGroup::Select) {
			if (index == 0) {

				finishUI_->ConfirmCancelByPad();
			} else {

				finishUI_->ConfirmOKByPad();
			}
		}
		});
	buttonFocusNavigator_->Init(ButtonFocusGroup::Top, { start_.get(), finishUI_->GetPowerButton() });
}

void TitleDisplaySprite::SetSpritePos() {

	// 各スプライトの座標を設定する
	// 名前
	name_->SetCenterTranslation();
	// 開始文字
	start_->SetTranslation(Vector2(Config::kWindowWidthf / 2.0f,
		Config::kWindowHeightf - (start_->GetSize().y * 2.0f)));
}

void TitleDisplaySprite::Init() {

	const std::string groupName = "TitleDisplaySprite";

	// 各機能、スプライトの初期化
	InitSprites();

	finishUI_ = std::make_unique<GameFinishUI>();
	finishUI_->Init();

	// 操作入力管理初期化
	InitNavigator();

	// json適応
	ApplyJson();
}

void TitleDisplaySprite::Update() {

	// パッド操作入力時の更新
	UpdateInputGamepad();
	// ゲーム開始チェック
	CheckGameStart();

	start_->Update();
	finishUI_->Update();
}

void TitleDisplaySprite::UpdateInputGamepad() {

	const auto& type = Input::GetInstance()->GetType();
	// パッド操作入力時のみ処理
	if (type == InputType::Keyboard) {
		return;
	}

	// 画面状態に応じてグループを切替
	if (finishUI_->IsSelectMenuReady()) {
		// キャンセル、OKの選択
		if (currentFocusGroup_ != ButtonFocusGroup::Select) {

			buttonFocusNavigator_->SetGroup(ButtonFocusGroup::Select,
				{ finishUI_->GetCancelButton(), finishUI_->GetOKButton() }, 0);
			currentFocusGroup_ = ButtonFocusGroup::Select;
		}
	} else {
		if (currentFocusGroup_ != ButtonFocusGroup::Top) {

			buttonFocusNavigator_->SetGroup(ButtonFocusGroup::Top,
				{ start_.get(), finishUI_->GetPowerButton() }, 0);
			currentFocusGroup_ = ButtonFocusGroup::Top;
		}
	}
	// 入力判定処理
	buttonFocusNavigator_->Update();
}

void TitleDisplaySprite::CheckGameStart() {

	if (isGameStart_ || finishUI_->IsSelectState()) {
		return;
	}

	// 入力状態に応じて判定方法を変える
	const auto& inputType = Input::GetInstance()->GetType();
	if (inputType == InputType::Keyboard) {

		// マウス入力があれば
		if (start_->GetHoverAtRelease()) {

			isGameStart_ = true;
		}
	} else if (inputType == InputType::GamePad) {

		// 開始にフォーカスが合っている状態でAボタンで開始
		if (!start_->GetCurrentActive(GameButtonResponseType::Focus)) {
			return;
		}
		if (Input::GetInstance()->TriggerGamepadButton(GamePadButtons::A)) {

			isGameStart_ = true;
		}
	}
}

void TitleDisplaySprite::ImGui() {

	ImGui::SetWindowFontScale(0.8f);
	ImGui::PushItemWidth(224.0f);

	if (ImGui::Button("Save")) {

		SaveJson();
	}
	if (ImGui::BeginTabBar("TitleDisplaySprite")) {
		if (ImGui::BeginTabItem("Base")) {

			bool edit = false;
			edit |= name_->ImGuiSize();
			edit |= start_->ImGuiSize();
			if (edit) {

				SetSpritePos();
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("StartButton")) {

			start_->ImGui();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("FinishUI")) {

			finishUI_->ImGui();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
	ImGui::SetWindowFontScale(1.0f);
}

void TitleDisplaySprite::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Title/titleDisplaySprite.json", data)) {
		return;
	}

	name_->SetSize(JsonAdapter::ToObject<Vector2>(data["name_Size"]));
	start_->SetSize(JsonAdapter::ToObject<Vector2>(data["start_Size"]));
	operateDevice_->SetSize(Vector2::FromJson(data.value("device_Size", Json())));
	operateDevice_->SetTranslation(Vector2::FromJson(data.value("device_Pos", Json())));

	finishUI_->ApplyJson(data["FinishUI"]);

	if (data.contains("start_")) {

		start_->FromJson(data["start_"]);
	}

	// 値設定
	SetSpritePos();
}

void TitleDisplaySprite::SaveJson() {

	Json data;

	data["name_Size"] = name_->GetSize().ToJson();
	data["start_Size"] = start_->GetSize().ToJson();
	data["device_Size"] = operateDevice_->GetSize().ToJson();
	data["device_Pos"] = operateDevice_->GetTranslation().ToJson();

	finishUI_->SaveJson(data["FinishUI"]);

	start_->ToJson(data["start_"]);

	JsonAdapter::Save("Title/titleDisplaySprite.json", data);
}