#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/InputMapper.h>
#include <Engine/Object/Base/GameObject2D.h>
#include <Engine/Utility/Enum/Direction.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Game/Objects/Common/Input/SelectUIInputAction.h>

//============================================================================
//	GameUIFocusNavigator class
//	入力に応じたUIフォーカス移動を管理するクラス
//============================================================================
class GameUIFocusNavigator {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameUIFocusNavigator() = default;
	~GameUIFocusNavigator() = default;

	void Init(const std::string& groupName);

	void Update();

	void ImGui();

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ナビゲーターの状態
	enum class NavigateState {

		Disable, // 入力を受け付けずフォーカス処理をさせない(全てのUIをUnfocusedにする)
		Update,  // ナビゲート中(ナビゲート開始時にisDefaultFocus = trueになっているUIをフォーカスで始める)
	};

	// アニメーションの種類
	enum class AnimationType {

		Translation, // 座標の移動補間(Vector2)
		Scale,       // スケール補間(float)
		Rotation,    // 回転補間(float)
		Size,        // サイズ補間(Vector2)
		Color,       // 色補間(Color)
		Alpha,       // アルファ補間(float)
	};

	// UIの状態
	enum class UIState :
		uint32_t {

		NowFocused = 0,   // フォーカスされた(トリガー判定)
		Focused = 1,      // フォーカスされている(更新)

		NowUnfocused = 2, // フォーカスが外れた(トリガー判定)
		Unfocused = 3,    // フォーカスされていない(更新)

		Decided = 4,      // 決定された(トリガー判定)
		Deciding = 5,     // 決定されている(更新)
	};

	// フォーカスされるUIの受付UI情報
	struct EntryRule {

		Vector2Int from;       // どの座標からか
		Direction2D direction; // どの方向入力か
	};

	// UI構造体
	struct UI {

		// UIの名前
		std::string name;

		// 現在の状態
		UIState state = UIState::Focused;
		// 起動時、UI表示始めにデフォルトでフォーカスにするか
		bool isDefaultFocus = true;

		// 表示用スプライト
		std::vector<std::unique_ptr<GameObject2D>> sprites;

		// UIの位置しているマップ位置
		Vector2Int ownMapCoordinate;
		std::vector<EntryRule> entryRules;

		// 表示スプライトの親
		std::unique_ptr<Transform2D> parentTransform;

		// 各状態用アニメーション
		// 例: UIStateでトリガー判定があればUIStateの+1したStateのアニメーションを再生して更新する
		//std::vector<std::unordered_map<UIState, std::unordered_map<AnimationType, SimpleAnimation*>>> stateAnimations;

		UI() = default;
		~UI() = default;

		 // コピー禁止
		UI(const UI&) = delete;
		UI& operator=(const UI&) = delete;

		// ムーブ許可
		UI(UI&&) noexcept = default;
		UI& operator=(UI&&) noexcept = default;
	};

	//--------- variables ----------------------------------------------------

	// ナビゲーターの状態
	NavigateState currentState_;
	NavigateState prevState_;

	// Navigatorグループの名前
	std::string groupName_;

	// 入力クラス
	std::unique_ptr<InputMapper<SelectUIInputAction>> inputMapper_;
	// UIリスト
	std::vector<UI> uiList_;

	// フォーカス処理
	int focusedUIIndex_ = -1;                    // 現在フォーカスしているUIのインデックス
	Vector2Int currentCoordinate_;               // 現在のマップ位置
	std::vector<int32_t> inputAcceptMapNumbers_; // 入力を受け付けるマップ番号のリスト

	//std::unordered_map<AnimationType, std::vector<std::unique_ptr<SimpleAnimation>>> animations_;

	// エディター
	// 編集中の選択インデックス
	int selectedUIIndex_ = -1;
	int selectedSpriteIndex_ = -1;

	InputImGui addUIName_;        // 追加するUIの名前
	InputImGui addUISpriteName_;  // 追加するUI内のSpriteの名前
	JsonSaveState jsonSaveState_; // json保存用状態

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// TabItem: CheckCurrentMap
	void CheckCurrentMap();

	// TabItem: UI
	void EditUI();
	// UI追加、削除
	void AddUI();
	void AddSprite();
	void RemoveUI();
	void RemoveSprite();
	// UI読み込み、保存
	void LoadUI(const std::string& outRelPath);
	void SaveUI(const std::string& outRelPath);

	// TabItem: Sprite
	void EditSprite();

	// 指定indexのUIをフォーカスにする
	void SetFocus(int newIndex, bool asTrigger);
	// 1フレームでの状態遷移
	void StepStates();

	int FindUIIndexByCoord(const Vector2Int& coordinate) const;
	bool CanFocusUIFrom(const UI& ui, const Vector2Int& from, Direction2D direction) const;
	Vector2Int DirectionDelta(Direction2D direction);
};