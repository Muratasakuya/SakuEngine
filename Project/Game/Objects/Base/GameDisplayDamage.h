#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Game/Objects/Base/GameDigitDisplay.h>

// c++
#include <deque>

//============================================================================
//	GameDisplayDamage class
//============================================================================
class GameDisplayDamage {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameDisplayDamage() = default;
	~GameDisplayDamage() = default;

	void Init(const std::string& textureName, const std::string& groupName,
		uint32_t damageDisplayMaxNum, uint32_t damageDigitMaxNum);

	void Update(const GameObject3D& object, const BaseCamera& camera);

	void ImGui();

	void ApplyJson(const Json& data);
	void SaveJson(Json& data);

	//--------- accessor -----------------------------------------------------

	void SetDamage(int damage);

	void SetAlpha(float alpha);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ダメージ表示の構造体
	struct DamagePopup {

		std::unique_ptr<GameDigitDisplay> digits;
		Vector2 basePos; // 表示座標
		float timer;     // 表示時間経過
		float outTimer;  // 消えるときの時間経過
		bool active;     // 表示している間
		int value;       // ダメージ値
	};

	//--------- variables ----------------------------------------------------

	// 受けたダメージ
	std::deque<int> receivedDamages_;

	// ダメージの表示
	std::vector<DamagePopup> damagePopups_;
	// 最大ダメージ表示数
	uint32_t damageDisplayMaxNum_;
	// 最大桁数
	uint32_t damageDigitMaxNum_;

	float totalAppearDuration_;  // 全ての桁を表示しきるまでの時間
	float damageStayTime_;       // 全ての桁を表示しきった後の待機時間
	float damageDisplayTime_;    // ダメージ表示の時間
	float damageOutTime_;        // ダメージ表示の消える時間
	float digitDisplayInterval_; // 桁表示の時間差
	float damageNumSpacing_;     // 数字の間の幅
	float bossScreenPosOffsetY_; // スクリーン座標のYオフセット
	float maxDamageEmissive_;    // 最大発光度

	Vector2 damageDisplayPosRandomRange_; // ダメージ表示のランダム範囲
	Vector2 damageDisplayMaxSize_;        // ダメージ表示の最大サイズ
	Vector2 damageDisplaySize_;           // ダメージ表示のサイズ
	EasingType damageDisplayEasingType_;  // ダメージ表示のイージングタイプ
	EasingType damageOutEasingType_;      // 消えるときのイージングタイプ
};