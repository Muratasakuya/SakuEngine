#include "BossEnemyIState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Config.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyIState classMethods
//============================================================================

void BossEnemyIState::LookTarget(BossEnemy& bossEnemy, const Vector3& target) {

	const float deltaTime = GameTimer::GetScaledDeltaTime();

	// 前方ベクトルを取得
	Vector3 bossPos = bossEnemy.GetTranslation();
	bossPos.y = 0.0f;

	// 回転を計算して設定
	Quaternion bossRotation = Quaternion::LookTarget(bossPos, target,
		Vector3(0.0f, 1.0f, 0.0f), bossEnemy.GetRotation(), rotationLerpRate_ * deltaTime);
	bossEnemy.SetRotation(bossRotation);
}

void BossEnemyIState::SlashEffect::Emit(BossEnemy& bossEnemy) {

	// エフェクト発生
	effect->Emit(bossEnemy.GetRotation() * effectOffset);
}

void BossEnemyIState::SlashEffect::Update(BossEnemy& bossEnemy) {

	// 剣エフェクトの更新、親の回転を設定する
	effect->SetParentRotation(effectNodeName,
		Quaternion::Normalize(bossEnemy.GetRotation()), ParticleUpdateModuleID::Rotation);
	effect->Update();
}