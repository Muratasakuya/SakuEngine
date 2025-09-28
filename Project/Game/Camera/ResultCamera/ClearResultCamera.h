#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	ClearResultCamera class
//============================================================================
class ClearResultCamera :
	public BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ClearResultCamera() = default;
	~ClearResultCamera() = default;

	void Init();

	void Update() override;

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 現在の状態
	enum class State {

		None,   // 何もなし
		Begin,  // 近づく
		Rotate  // ピボット回転
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// parameters
	// Begin
	StateTimer animationTimer_; // 時間管理
	Vector3 startPos_;          // 開始座標
	Vector3 targetPos_;         // 目標座標

	// Rotate
	float rotateSpeed_;  // 回転速度
	float initRotateX_;  // X軸回転角
	float eulerRotateX_; // X軸回転角
	Vector3 viewPoint_;  // 注視点
	float viewOffset_;   // 注視点からのオフセット距離

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
	void UpdateAnimation();
	void UpdateRotate();
};