#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/InputStructures.h>
#include <Game/Objects/Base/IGameButtonResponseUpdater.h>

//============================================================================
//	GameButton enum class
//============================================================================

// アクティブ状態条件
enum class GameButtonCollisionType {

	Mouse,  // マウス操作
	Key,    // キーボード操作
	GamePad // パッド操作
};

// 応答条件
enum class GameButtonResponseType {

	OnMouse,       // マウスが上に来た
	MouseClick,    // 上にある状態でクリック(Trigger)
	OnMouseClick,  // 上にある状態でクリック(Press)
	AnyMouseClick, // 上にある状態でクリック後、離した位置によってレスポンスを変える
	Focus,         // フォーカス中はアクティブ
};

//============================================================================
//	GameButton class
//	ゲームで使えるボタンを楽に処理する、Updaterと組み合わせて使う
//============================================================================
class GameButton :
	public GameObject2D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameButton() = default;
	~GameButton() = default;

	void Init(const std::string& textureName, const std::string& groupName);
	// 全て登録後に呼び出す
	void FromJson(const Json& data);

	void Update();

	// 更新用クラスの登録
	void RegisterUpdater(GameButtonResponseType type, std::unique_ptr<IGameButtonResponseUpdater> updater);

	// editor
	void ImGui();

	// json
	void ToJson(Json& data);

	//--------- accessor -----------------------------------------------------

	void SetCollisionType(GameButtonCollisionType type) { collisionType_ = type; }
	void SetResponseType(GameButtonResponseType type) { responseType_ = type; }
	void SetEnableCollision(bool enable) { checkCollisionEnable_ = enable; }
	void SetFocusActive(bool active) { currentActive_[GameButtonResponseType::Focus] = active; }

	bool GetHoverAtRelease() const;
	bool GetCurrentHover() const { return hoverNow_; }
	bool GetCurrentActive(GameButtonResponseType type) const { return currentActive_.at(type); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 判定を取る状態か
	bool checkCollisionEnable_;

	// アクティブ状態になる判定
	GameButtonCollisionType collisionType_;
	GameButtonResponseType responseType_;
	// マウス
	MouseButton mouseButton_;

	// 入力状態
	std::unordered_map<GameButtonResponseType, bool> currentActive_;
	std::unordered_map<GameButtonResponseType, bool> preActive_;
	std::unordered_map<GameButtonResponseType, bool> inactiveRunning_;

	// レスポンスに応じて更新処理を行う
	std::unordered_map<GameButtonResponseType,
		std::unique_ptr<IGameButtonResponseUpdater>> responseUpdaters_;

	// 現在の入力状態
	bool hoverNow_ = false;   // ホバー中
	bool triggerNow_ = false; // 押し始め
	bool pushNow_ = false;    // 押しっぱ
	bool releaseNow_ = false; // 離した瞬間

	// Any...で使う
	bool anyPressActive_ = false;         // 入力中...
	bool anyPressStartedOnHover_ = false; // 入力始め
	bool hoverAtRelease_ = false;         // 離した瞬間の位置

	// parameters
	Vector2 collisionSize_;

	//--------- functions ----------------------------------------------------

	// update
	void UpdateResponses();

	// helper
	void DetectCollision();
	void DetectMouseCollision();
	void EvaluateAnyMouseClick();
};