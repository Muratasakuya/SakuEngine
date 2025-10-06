#pragma once

//============================================================================
//	include
//============================================================================

//============================================================================
//	FollowCameraStructures
//============================================================================

// 状態の種類
enum class FollowCameraState {

	Follow,     // 通常追従状態
	SwitchAlly, // プレイヤーが味方を切り替えるかチェック中
	AllyAttack, // 味方の攻撃中
	StunAttack, // プレイヤーの攻撃
};

// 現在の状態の次に設定されていれば行う処理
enum class FollowCameraOverlayState {

	Shake, // 画面シェイク処理
};

// 追従先
enum class FollowCameraTargetType {

	Player,
	PlayerAlly,
	BossEnemy
};