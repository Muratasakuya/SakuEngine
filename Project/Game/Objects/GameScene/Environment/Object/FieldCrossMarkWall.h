#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

//============================================================================
//	FieldCrossMarkWall class
//	行けない場所を示す十字マークの壁、押し戻し処理はしていない
//============================================================================
class FieldCrossMarkWall :
	public GameObject3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FieldCrossMarkWall() = default;
	~FieldCrossMarkWall() = default;

	// 初期化
	void DerivedInit() override;

	// 更新
	void Update() override;

	// エディター
	void DerivedImGui() override;

	/*-------- collision ----------*/

	// 衝突コールバック関数
	void OnCollisionEnter([[maybe_unused]] const CollisionBody* collisionBody) override;
	void OnCollisionStay([[maybe_unused]] const CollisionBody* collisionBody) override;
	void OnCollisionExit([[maybe_unused]] const CollisionBody* collisionBody) override;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		Idle,
		Entering,
		Staying,
		Exiting
	};

	//--------- variables ----------------------------------------------------

	State state_ = State::Idle;

	// parameters
	float lerpTimer_; // 補間時間経過
	float lerpTime_;  // 補間時間

	float blinkingSpacing_; // 点滅間隔
	float blinkTimer_;      // 点滅経過時間

	Color initColor_;    // 初期色
	Color targetColor_;  // 目標色
	Color startColor_;   // 補間開始色
	Color currentColor_; // 現在の値

	float startEmissive_;   // 補間開始発光度
	float currentEmissive_; // 現在の値

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};