#include "GameUIFocusNavigator.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>
#include <Game/Objects/Common/Input/Device/SelectUIKeyInput.h>
#include <Game/Objects/Common/Input/Device/SelectUIGamePadInput.h>

//============================================================================
//	GameUIFocusNavigator classMethods
//============================================================================

void GameUIFocusNavigator::Init(const std::string& groupName) {

	// 初期化値
	currentState_ = NavigateState::Disable;
	groupName_ = groupName;

	// 入力クラスの初期化
	Input* input = Input::GetInstance();
	inputMapper_ = std::make_unique<InputMapper<SelectUIInputAction>>();
	inputMapper_->AddDevice(std::make_unique<SelectUIKeyInput>(input));
	inputMapper_->AddDevice(std::make_unique<SelectUIGamePadInput>(input));

	// json適用
	ApplyJson();
}

void GameUIFocusNavigator::AddUI() {

	UI ui{};
	// 名前設定
	ui.name = addUIName_.inputText;
	// 親Transform初期化
	ui.parentTransform = std::make_unique<Transform2D>();
	ui.parentTransform->Init(nullptr);
	// デフォルト値
	ui.isDefaultFocus = uiList_.empty(); // 最初に作るUIをデフォルトにする
	ui.state = UIState::Unfocused;
	ui.mapNumber = 0;

	// スプライトを初期化
	std::unique_ptr<GameObject2D> sprite = std::make_unique<GameObject2D>();
	sprite->Init("white", addUISpriteName_.inputText, groupName_);
	// 親設定
	sprite->SetParent(*ui.parentTransform.get());

	// デフォルトのスプライト表示
	ui.sprites.emplace_back(std::move(sprite));

	// 方向マップ初期化
	ui.directionMap.emplace(Direction2D::Up, 0);
	ui.directionMap.emplace(Direction2D::Bottom, 0);
	ui.directionMap.emplace(Direction2D::Left, 0);
	ui.directionMap.emplace(Direction2D::Right, 0);

	// UIリストに追加
	uiList_.emplace_back(std::move(ui));

	// 追加されたらエディター選択対象にする
	selectedUIIndex_ = static_cast<int>(uiList_.size()) - 1;
}

void GameUIFocusNavigator::AddSprite() {

	// 範囲外チェック
	if (selectedUIIndex_ < 0 || static_cast<int>(uiList_.size()) <= selectedUIIndex_) {
		return;
	}

	UI& ui = uiList_[selectedUIIndex_];
	// スプライトを初期化
	std::unique_ptr<GameObject2D> sprite = std::make_unique<GameObject2D>();
	sprite->Init("white", addUISpriteName_.inputText, groupName_);
	// 親設定
	sprite->SetParent(*ui.parentTransform);
	// デフォルトのスプライト表示
	ui.sprites.emplace_back(std::move(sprite));

	// 追加されたらエディター選択対象にする
	selectedSpriteIndex_ = static_cast<int>(ui.sprites.size()) - 1;
}

void GameUIFocusNavigator::RemoveUI() {

	// 範囲外チェック
	if (selectedUIIndex_ < 0 || static_cast<int>(uiList_.size()) <= selectedUIIndex_) {
		return;
	}

	// フォーカス情報の更新
	if (focusedUIIndex_ == selectedUIIndex_) {

		// フォーカス中のUIを削除したらフォーカス情報をリセット
		focusedUIIndex_ = -1;
	}
	// 削除
	uiList_.erase(uiList_.begin() + selectedUIIndex_);

	// 選択インデックスの補正
	// 一つも無くなったら-1にする
	if (uiList_.empty()) {

		selectedUIIndex_ = -1;
	}
	// 選択中のUIが最後のUIだったら一つ前のUIを選択するようにする
	else {

		selectedUIIndex_ = std::clamp(selectedUIIndex_, 0, static_cast<int>(uiList_.size()) - 1);
	}
}

void GameUIFocusNavigator::RemoveSprite() {

	// 範囲外チェック
	if (selectedUIIndex_ < 0 || static_cast<int>(uiList_.size()) <= selectedUIIndex_) {
		return;
	}
	UI& ui = uiList_[selectedUIIndex_];
	// 範囲外チェック
	if (selectedSpriteIndex_ < 0 || static_cast<int>(ui.sprites.size() <= selectedSpriteIndex_)) {
		return;
	}
	// 削除
	ui.sprites.erase(ui.sprites.begin() + selectedSpriteIndex_);

	// 選択インデックスの補正
	// 一つも無くなったら-1にする
	if (ui.sprites.empty()) {

		selectedSpriteIndex_ = -1;
	}
	// 選択中のSpriteが最後のSpriteだったら一つ前のSpriteを選択するようにする
	else {

		selectedSpriteIndex_ = std::clamp(selectedSpriteIndex_, 0, static_cast<int>(ui.sprites.size()) - 1);
	}
}

void GameUIFocusNavigator::Update() {

	// 状態遷移開始時の処理
	if (prevState_ != currentState_) {
		// Disable状態のとき
		if (currentState_ == NavigateState::Disable) {

			// すべてUnfocusedにする
			for (auto& ui : uiList_) {

				ui.state = UIState::Unfocused;
			}
			focusedUIIndex_ = -1;
		}
		// Update処理が始まったとき
		else {

			// デフォルトでフォーカスされるUIを探してフォーカスを当てる
			int startIndex = -1;
			for (int i = 0; i < static_cast<int>(uiList_.size()); ++i) {
				if (uiList_[i].isDefaultFocus && uiList_[i].mapNumber == currentMapNumber_) {

					startIndex = i;
					break;
				}
			}
			// デフォルトでフォーカスされるUIがなかったら
			if (startIndex < 0) {
				for (int i = 0; i < static_cast<int>(uiList_.size()); ++i) {
					// マップ番号が同じでデフォルトフォーカスのUIを設定する
					if (uiList_[i].isDefaultFocus) {

						startIndex = i;
						break;
					}
				}
				// この時点でもなかったら0番目を設定する
				if (startIndex < 0 && !uiList_.empty()) {

					startIndex = 0;
				}
			}
			// フォーカス開始
			SetFocus(startIndex, true);
		}
		// 状態を更新
		prevState_ = currentState_;
	}

	// Disable状態のときはここで終了
	if (currentState_ == NavigateState::Disable) {
		return;
	}
	// 
	if (!IsInputAccepted()) {

		// 
		StepStates();
		return;
	}

	// 入力によるナビゲーション処理
	auto moveBy = [&](Direction2D dir, SelectUIInputAction action) {

		// なにも入力がなければ処理しない
		if (!inputMapper_->IsTriggered(action)) {
			return;
		}
		// 範囲外チェック
		if (focusedUIIndex_ < 0 || static_cast<int>(uiList_.size()) <= focusedUIIndex_) {
			return;
		}

		// 選択されたUIの参照を取得
		UI& currentUI = uiList_[focusedUIIndex_];
		auto it = currentUI.directionMap.find(dir);
		if (it == currentUI.directionMap.end()) {
			return;
		}

		// 移動先のマップ番号を取得
		int32_t nextMap = it->second;
		// 移動先のUIインデックスを取得
		int nextIndex = FindUIIndexByMapNumber(nextMap);
		// 0以上、現在のフォーカスUIと異なるインデックスなら移動
		if (0 <= nextIndex && nextIndex != focusedUIIndex_) {

			// マップ番号を更新
			currentMapNumber_ = nextMap;
			SetFocus(nextIndex, true);
		}
		};

	// 各方向への移動処理
	moveBy(Direction2D::Left, SelectUIInputAction::Left);
	moveBy(Direction2D::Right, SelectUIInputAction::Right);
	moveBy(Direction2D::Up, SelectUIInputAction::Up);
	moveBy(Direction2D::Bottom, SelectUIInputAction::Down);

	// 決定入力判定
	if (0 <= focusedUIIndex_ && focusedUIIndex_ < static_cast<int>(uiList_.size())) {

		// 選択されたUIの参照を取得
		UI& currentUI = uiList_[focusedUIIndex_];
		// Decided判定を受けた次のフレームからDecidingにする
		if (currentUI.state == UIState::Decided) {

			currentUI.state = UIState::Deciding;
		}
		// トリガー判定でDecidedにする
		if (currentUI.state != UIState::Decided && inputMapper_->IsTriggered(SelectUIInputAction::Decide)) {

			currentUI.state = UIState::Decided;
		}
	}

	// 各UIの状態更新
	StepStates();
}

void GameUIFocusNavigator::SetFocus(int newIndex, bool asTrigger) {

	// 範囲外チェック
	if (newIndex < 0 || static_cast<int>(uiList_.size()) <= newIndex) {
		focusedUIIndex_ = -1;
		return;
	}

	if (focusedUIIndex_ != newIndex) {

		// 以前フォーカスされていたUIの状態をNowUnfocusedにする
		if (0 <= focusedUIIndex_ && focusedUIIndex_ < static_cast<int>(uiList_.size())) {

			uiList_[focusedUIIndex_].state = UIState::NowUnfocused;
		}

		// 新たにフォーカスされたUIの状態をNowFocusedにする
		focusedUIIndex_ = newIndex;
		uiList_[focusedUIIndex_].state = UIState::NowFocused;
	} else if (!asTrigger) {

		// 同じUIにフォーカスを当てるときトリガー判定でなければFocused状態にする
		uiList_[focusedUIIndex_].state = UIState::Focused;
	}
}

int GameUIFocusNavigator::FindUIIndexByMapNumber(int32_t mapNumber) const {

	for (int i = 0; i < static_cast<int>(uiList_.size()); ++i) {
		if (uiList_[i].mapNumber == mapNumber) {
			return i;
		}
	}
	return -1;
}

void GameUIFocusNavigator::StepStates() {

	// 状態を1つ進めて更新中にする
	for (auto& ui : uiList_) {
		switch (ui.state) {
		case UIState::NowFocused: {

			ui.state = UIState::Focused;
			break;
		}
		case UIState::NowUnfocused: {

			ui.state = UIState::Unfocused;
			break;
		}
		case UIState::Decided: {

			ui.state = UIState::Deciding;
			break;
		}
		}

		// 親Transformの行列更新
		ui.parentTransform->UpdateMatrix();
	}
}

bool GameUIFocusNavigator::IsInputAccepted() const {

	if (inputAcceptMapNumbers_.empty()) {
		return true;
	}
	return std::find(inputAcceptMapNumbers_.begin(), inputAcceptMapNumbers_.end(), currentMapNumber_) !=
		inputAcceptMapNumbers_.end();
}

void GameUIFocusNavigator::ImGui() {

	// 保存
	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::Separator();
	ImGui::PushItemWidth(224.0f);

	// ナビゲーターの状態
	EnumAdapter<NavigateState>::Combo("currrentState", &currentState_);
	ImGui::SeparatorText("Navigator Status");
	ImGui::Text("Focused Index : %d", focusedUIIndex_);
	ImGui::Text("Current Map : %d", currentMapNumber_);
	if (0 <= focusedUIIndex_ && focusedUIIndex_ < static_cast<int>(uiList_.size())) {

		const UI& ui = uiList_[focusedUIIndex_];
		ImGui::Text("Focused UI : %s", ui.name.c_str());
		ImGui::Text("UI State : %s", EnumAdapter<UIState>::ToString(ui.state));
	}

	if (ImGui::BeginTabBar("GameUIFocusNavigator")) {
		if (ImGui::BeginTabItem("UIEdit")) {

			EditUI();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("SpriteEdit")) {

			EditSprite();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::PopItemWidth();
}

void GameUIFocusNavigator::EditUI() {

	// 左右で分割
	const float fullW = ImGui::GetContentRegionAvail().x;
	const float leftW = std::floor(fullW * 0.45f);

	//============================================================================
	//	UI追加/削除/保存/読込 + UI一覧
	//============================================================================
	ImGui::BeginChild("UIEditLeftPane", ImVec2(leftW, 0.0f), true);
	{
		// 入力欄
		ImGuiHelper::InputText("UI Name", addUIName_);
		ImGuiHelper::InputText("Sprite Name", addUISpriteName_);

		if (ImGui::Button("Add UI")) {
			// 入力がなければ追加できないようにする
			if (!addUIName_.inputText.empty() && !addUISpriteName_.inputText.empty()) {

				// すでに同じ名前のUIがあれば追加できないようにする
				bool duplicated = false;
				for (const auto& ui : uiList_) {
					if (ui.name == addUIName_.inputText) {

						// 同じ名前があった
						duplicated = true;
						break;
					}
				}
				if (!duplicated) {

					// UI追加
					AddUI();
				}
			}
		}
		ImGui::SameLine();

		// 選択中のUIの削除
		if (ImGui::Button("Remove UI")) {

			RemoveUI();
		}

		// 保存と読み込み
		{
			std::string outRefPath{};
			ImGui::SameLine();
			if (ImGui::Button("Save UI")) {

				jsonSaveState_.showPopup = true;
			}
			if (ImGuiHelper::SaveJsonModal("Save UI", "GameUIFocusNavigator/UI/",
				"GameUIFocusNavigator/UI/", jsonSaveState_, outRefPath)) {

				SaveUI(outRefPath);
			}
			ImGui::SameLine();
			if (ImGui::Button("Load UI")) {

				if (ImGuiHelper::OpenJsonDialog(outRefPath)) { LoadUI(outRefPath); }
			}
		}

		// UI一覧
		std::vector<std::string> names;
		for (const auto& ui : uiList_) {

			names.emplace_back(ui.name);
		}

		int prev = selectedUIIndex_;
		ImGuiHelper::SelectableListFromStrings("UIList", &selectedUIIndex_, names, 8);
		// UIを切り替えたらSprite選択もリセット
		if (selectedUIIndex_ != prev) {

			selectedSpriteIndex_ = -1;
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	//============================================================================
	//選択UIの詳細+ DirectionMap + Sprite一覧
	//============================================================================
	ImGui::BeginChild("UIEditRightPane", ImVec2(0.0f, 0.0f), true);
	{
		if (0 <= selectedUIIndex_ && selectedUIIndex_ < static_cast<int>(uiList_.size())) {

			UI& ui = uiList_[selectedUIIndex_];

			ImGui::SeparatorText(ui.name.c_str());

			// 状態表示
			ImGui::Text("State: %s", EnumAdapter<UIState>::ToString(ui.state));

			// ボタンで指定UIのフォーカス
			ImGui::Checkbox("isDefaultFocus", &ui.isDefaultFocus);
			if (ImGui::Button("Focus this UI")) {

				currentMapNumber_ = ui.mapNumber;
				SetFocus(FindUIIndexByMapNumber(currentMapNumber_), true);
			}

			ImGui::DragInt("mapNumber", &ui.mapNumber, 1);

			// 自身のUIマップ位置
			{
				int up = ui.directionMap[Direction2D::Up];
				int down = ui.directionMap[Direction2D::Bottom];
				int left = ui.directionMap[Direction2D::Left];
				int right = ui.directionMap[Direction2D::Right];
				if (ImGui::DragInt("Up", &up, 1)) {
					ui.directionMap[Direction2D::Up] = up;
				}
				if (ImGui::DragInt("Down", &down, 1)) {
					ui.directionMap[Direction2D::Bottom] = down;
				}
				if (ImGui::DragInt("Left", &left, 1)) {
					ui.directionMap[Direction2D::Left] = left;
				}
				if (ImGui::DragInt("Right", &right, 1)) {
					ui.directionMap[Direction2D::Right] = right;
				}
			}

			// Spriteリスト
			{
				std::vector<std::string> spriteNames;
				for (size_t i = 0; i < ui.sprites.size(); ++i) {

					spriteNames.emplace_back(ui.sprites[i]->GetTag().name);
				}
				ImGuiHelper::SelectableListFromStrings("SpriteList", &selectedSpriteIndex_, spriteNames, 4);
			}
		} else {

			ImGui::TextDisabled("Select a UI from the left list.");
		}
	}
	ImGui::EndChild();
}

void GameUIFocusNavigator::EditSprite() {

	// 範囲外チェック	
	if (selectedUIIndex_ < 0 || static_cast<int>(uiList_.size() <= selectedUIIndex_)) {
		ImGui::TextDisabled("Select a UI from the UIEdit tab.");
		return;
	}
	UI& ui = uiList_[selectedUIIndex_];
	// スプライト範囲外チェック
	if (selectedSpriteIndex_ < 0 || static_cast<int>(ui.sprites.size()) <= selectedSpriteIndex_) {
		ImGui::TextDisabled("Select a Sprite from the UIEdit tab (Sprites list).");
		return;
	}

	// 左右に分割
	const float fullW = ImGui::GetContentRegionAvail().x;
	const float leftW = std::floor(fullW * 0.45f);

	// 左：親Transform
	ImGui::BeginChild("SpriteEditLeft", ImVec2(leftW, 0.0f), true);
	{
		ImGui::SeparatorText("Parent Transform");

		ui.parentTransform->ImGui(192.0f, 24.0f);
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// 右：選択中Spriteの編集
	ImGui::BeginChild("SpriteEditRight", ImVec2(0.0f, 0.0f), true);
	{
		ImGui::SeparatorText("Sprite");

		ui.sprites[selectedSpriteIndex_]->ImGui();
	}
	ImGui::EndChild();
}

void GameUIFocusNavigator::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("GameUIFocusNavigator/" + groupName_ + ".json", data)) {
		return;
	}
}

void GameUIFocusNavigator::SaveJson() {

	Json data;

	JsonAdapter::Save("GameUIFocusNavigator/" + groupName_ + ".json", data);
}

void GameUIFocusNavigator::LoadUI(const std::string& outRelPath) {

	Json data;
	if (!JsonAdapter::LoadCheck(outRelPath, data)) {
		return;
	}
	// dataからUI情報を読み込み追加する
}

void GameUIFocusNavigator::SaveUI(const std::string& outRelPath) {

	Json data;

	// 選択中のUI情報をdataに保存する

	JsonAdapter::Save(outRelPath, data);
}