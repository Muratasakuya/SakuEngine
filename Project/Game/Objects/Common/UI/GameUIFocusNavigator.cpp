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

	// スプライトを初期化
	std::unique_ptr<GameObject2D> sprite = std::make_unique<GameObject2D>();
	sprite->Init("white", addUISpriteName_.inputText, groupName_);
	// 親設定
	sprite->SetParent(*ui.parentTransform.get());

	// デフォルトのスプライト表示
	ui.sprites.emplace_back(std::move(sprite));

	// 座標初期化
	ui.ownMapCoordinate = Vector2Int(0, 0);
	ui.entryRules.clear();

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

			// DisableからUpdateになった瞬間
			int startIndex = -1;
			// isDefaultFocusがtrueになっているUIかつ現在のマップ座標にいるUIを探す
			const int uiListSize = static_cast<int>(uiList_.size());
			for (int i = 0; i < uiListSize; ++i) {
				if (uiList_[i].isDefaultFocus && uiList_[i].ownMapCoordinate == currentCoordinate_) {

					startIndex = i;
					break;
				}
			}
			// なにも無ければ0番目をフォーカスさせる
			if (startIndex < 0 && !uiList_.empty()) {

				startIndex = 0;
			}
			SetFocus(startIndex, true);
		}
		// 状態を更新
		prevState_ = currentState_;
	}

	// Disable状態のときはここで終了
	if (currentState_ == NavigateState::Disable) {
		return;
	}

	// 入力によるナビゲーション処理
	auto MoveFromInput = [&](Direction2D direction, SelectUIInputAction action) {

		// 入力がなければ終了
		if (!inputMapper_->IsTriggered(action)) {
			return;
		}
		// 現在フォーカス中のUIがなければ終了
		if (focusedUIIndex_ < 0 || static_cast<int>(uiList_.size()) <= focusedUIIndex_) {
			return;
		}

		for (int i = 0; i < static_cast<int>(uiList_.size()); ++i) {
			if (i == focusedUIIndex_) {
				continue;
			}
			// 指定方向と現在座標からフォーカス可能なUIがあればそちらにフォーカスを移動する
			if (CanFocusUIFrom(uiList_[i], currentCoordinate_, direction)) {

				SetFocus(i, true);
				return;
			}
		}

		// 指定方向の座標を計算
		Vector2Int next = currentCoordinate_;
		next += DirectionDelta(direction);

		// 指定座標にいるUIを探す
		int index = FindUIIndexByCoord(next);
		// 存在しなければ何もしないで終了
		if (0 <= index && index != focusedUIIndex_) {

			// フォーカス移動
			SetFocus(index, true);
		}
		};

	// 各方向への移動処理
	MoveFromInput(Direction2D::Left, SelectUIInputAction::Left);
	MoveFromInput(Direction2D::Right, SelectUIInputAction::Right);
	MoveFromInput(Direction2D::Up, SelectUIInputAction::Up);
	MoveFromInput(Direction2D::Bottom, SelectUIInputAction::Down);

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

	// 新しい入力があれば
	if (focusedUIIndex_ != newIndex) {

		// 古いフォーカスUIの状態をNowUnfocusedにする
		if (0 <= focusedUIIndex_ && focusedUIIndex_ < static_cast<int>(uiList_.size())) {

			uiList_[focusedUIIndex_].state = UIState::NowUnfocused;
		}
		// インデックス、座標を更新
		focusedUIIndex_ = newIndex;
		currentCoordinate_ = uiList_[focusedUIIndex_].ownMapCoordinate;
		uiList_[focusedUIIndex_].state = UIState::NowFocused;
	} else if (!asTrigger) {

		uiList_[focusedUIIndex_].state = UIState::Focused;
	}
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

int GameUIFocusNavigator::FindUIIndexByCoord(const Vector2Int& coordinate) const {

	// 指定座標にいるUIを探す
	for (int i = 0; i < static_cast<int>(uiList_.size()); ++i) {
		if (uiList_[i].ownMapCoordinate == coordinate) {

			return i;
		}
	}
	return -1;
}

bool GameUIFocusNavigator::CanFocusUIFrom(const UI& ui, const Vector2Int& from, Direction2D direction) const {

	// 指定方向と座標からフォーカス可能か
	for (const auto& rule : ui.entryRules) {
		if (rule.direction == direction && rule.from == from) {

			return true;
		}
	}
	return false;
}

Vector2Int GameUIFocusNavigator::DirectionDelta(Direction2D direction) {

	// 各方向の座標を返す
	switch (direction) {
	case Direction2D::Left: {
		return Vector2Int(-1, 0);
	}
	case Direction2D::Right: {
		return Vector2Int(1, 0);
	}
	case Direction2D::Up: {
		return Vector2Int(0, 1);
	}
	case Direction2D::Bottom: {
		return Vector2Int(0, -1);
	}
	}
	return Vector2Int(0, 0);
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
	if (0 <= focusedUIIndex_ && focusedUIIndex_ < static_cast<int>(uiList_.size())) {

		const UI& ui = uiList_[focusedUIIndex_];
		ImGui::Text("Focused UI : %s", ui.name.c_str());
		ImGui::Text("UI State : %s", EnumAdapter<UIState>::ToString(ui.state));
	}

	if (ImGui::BeginTabBar("GameUIFocusNavigator")) {
		if (ImGui::BeginTabItem("CheckMap")) {

			CheckCurrentMap();
			ImGui::EndTabItem();
		}
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

void GameUIFocusNavigator::CheckCurrentMap() {

	// 座標範囲を算出
	int minX = currentCoordinate_.x, maxX = currentCoordinate_.x;
	int minY = currentCoordinate_.y, maxY = currentCoordinate_.y;
	for (const auto& ui : uiList_) {

		minX = (std::min)(minX, ui.ownMapCoordinate.x);
		maxX = (std::max)(maxX, ui.ownMapCoordinate.x);
		minY = (std::min)(minY, ui.ownMapCoordinate.y);
		maxY = (std::max)(maxY, ui.ownMapCoordinate.y);
	}
	// 表示に少し余白
	minX -= 1;
	maxX += 1;
	minY -= 1;
	maxY += 1;

	ImGui::Text("Current : (%d, %d)", currentCoordinate_.x, currentCoordinate_.y);

	// セルサイズ
	const float cell = 64.0f;
	const int cols = (maxX - minX + 1);
	if (cols <= 0) {
		return;
	}

	if (ImGui::BeginTable("MapTable", cols, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
		for (int y = maxY; y >= minY; --y) {
			ImGui::TableNextRow();
			for (int x = minX; x <= maxX; ++x) {

				ImGui::TableSetColumnIndex(x - minX);
				ImGui::PushID(x * 10007 + y);

				// セル描画
				ImGui::BeginGroup();
				ImGui::InvisibleButton("cell", ImVec2(cell, cell));

				// セル内表示を重ねる
				ImVec2 p0 = ImGui::GetItemRectMin();
				ImVec2 p1 = ImGui::GetItemRectMax();
				auto* drawList = ImGui::GetWindowDrawList();

				// 枠
				const bool isCurrent = currentCoordinate_ == Vector2Int(x, y);
				drawList->AddRect(p0, p1, isCurrent ? IM_COL32(80, 200, 255, 255) : IM_COL32(80, 80, 80, 255), 0.0f, 0, 1.0f);

				// UI名
				int index = FindUIIndexByCoord(Vector2Int(x, y));
				if (index >= 0) {

					const char* nm = uiList_[index].name.c_str();
					ImVec2 sz = ImGui::CalcTextSize(nm);
					ImVec2 c = ImVec2((p0.x + p1.x - sz.x) * 0.5f, (p0.y + p1.y - sz.y) * 0.5f);
					drawList->AddText(c, IM_COL32(255, 255, 255, 255), nm);
				} else {

					// 座標ラベル
					char buf[32]; snprintf(buf, sizeof(buf), "(%d,%d)", x, y);
					ImGui::CalcTextSize(buf);
					drawList->AddText(ImVec2(p0.x + 4, p0.y + 4), IM_COL32(160, 160, 160, 255), buf);
				}

				// クリックで現在地、フォーカスを変更
				if (ImGui::IsItemClicked()) {

					currentCoordinate_ = Vector2Int(x, y);
					if (0 <= index) {

						SetFocus(index, true);
					}
				}
				ImGui::EndGroup();
				ImGui::PopID();
			}
		}
		ImGui::EndTable();
	}
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
			ImGui::DragInt2("ownCoordinate", &ui.ownMapCoordinate.x, 1);
			if (ImGui::SmallButton("Focus")) {

				SetFocus(selectedUIIndex_, true);
			}

			// フォーカスされるUIの受付UI情報
			ImGui::SeparatorText("Entry Rules");
			if (ImGui::BeginTable("EntryRules", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {

				ImGui::TableSetupColumn("from.x");
				ImGui::TableSetupColumn("from.y");
				ImGui::TableSetupColumn("dir");
				ImGui::TableSetupColumn(" ");
				ImGui::TableHeadersRow();
				for (int i = 0; i < static_cast<int>(ui.entryRules.size()); ++i) {

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::DragInt(("##ex" + std::to_string(i)).c_str(), &ui.entryRules[i].from.x, 1);
					ImGui::TableSetColumnIndex(1);
					ImGui::DragInt(("##ey" + std::to_string(i)).c_str(), &ui.entryRules[i].from.y, 1);
					ImGui::TableSetColumnIndex(2);
					EnumAdapter<Direction2D>::Combo(("##edir" + std::to_string(i)).c_str(), &ui.entryRules[i].direction);
					ImGui::TableSetColumnIndex(3);
					if (ImGui::SmallButton(("Remove##er" + std::to_string(i)).c_str())) {

						ui.entryRules.erase(ui.entryRules.begin() + i);
						--i;
					}
				}
				ImGui::EndTable();
			}
			if (ImGui::SmallButton("Add Entry")) {

				EntryRule rule;
				rule.from = currentCoordinate_;
				rule.direction = Direction2D::Right;
				ui.entryRules.emplace_back(rule);
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