#include "GameEffectCommandHelper.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/GameEffect.h>
#include <Engine/Scene/Camera/BaseCamera.h>

//============================================================================
//	GameEffectCommandHelper classMethods
//============================================================================

void GameEffectCommandHelper::SendSpawnerEmit(GameEffect& effect, bool emit) {

	// 発生設定
	ParticleCommand command{};
	command.target = ParticleCommandTarget::Spawner;
	command.id = ParticleCommandID::SetEmitFlag;
	command.value = emit;
	effect.SendCommand(command);
}

void GameEffectCommandHelper::SendSpawnerBillboard(GameEffect& effect, const BaseCamera& camera) {

	// ビルボード回転設定
	ParticleCommand command{};
	command.target = ParticleCommandTarget::Spawner;
	command.id = ParticleCommandID::SetBillboardRotation;
	command.value = camera.GetBillboardMatrix();
	effect.SendCommand(command);
}

void GameEffectCommandHelper::SendSpawnerRotation(GameEffect& effect, const Matrix4x4& rotateMatrix) {

	// 回転設定
	ParticleCommand command{};
	command.target = ParticleCommandTarget::Spawner;
	command.id = ParticleCommandID::SetRotation;
	command.value = rotateMatrix;
	effect.SendCommand(command);
}

void GameEffectCommandHelper::SendSpawnerTranslation(GameEffect& effect, const Vector3& translation) {

	// 座標設定
	ParticleCommand command{};
	command.target = ParticleCommandTarget::Spawner;
	command.id = ParticleCommandID::SetTranslation;
	command.value = translation;
	effect.SendCommand(command);
}

void GameEffectCommandHelper::SendScaling(GameEffect& effect, float scalingValue) {

	// スケーリングを設定
	ParticleCommand command{};
	command.target = ParticleCommandTarget::Spawner;
	command.id = ParticleCommandID::Scaling;
	command.value = scalingValue;
	effect.SendCommand(command);
	command.target = ParticleCommandTarget::Updater;
	effect.SendCommand(command);
}

void GameEffectCommandHelper::ApplyAndSend(GameEffect& effect,
	const Quaternion& parentRotation, const Vector3& localPos) {

	// 回転を考慮した発生位置を計算
	const Vector3 emitPos = Quaternion::RotateVector(localPos, parentRotation);

	ParticleCommand command{};
	command.target = ParticleCommandTarget::Spawner;
	command.id = ParticleCommandID::SetTranslation;
	command.value = emitPos;
	effect.SendCommand(command);
}

void GameEffectCommandHelper::ApplyAndSend(GameEffect& effect, const Quaternion& parentRotation,
	const Vector3& localPos, const Vector3& localEuler) {

	// ローカル回転を計算
	const Quaternion localRotation = Quaternion::EulerToQuaternion(localEuler);
	const Vector3 forwardLocal = Quaternion::RotateVector(Vector3(0.0f, 0.0f, 1.0f), localRotation);
	const Vector3 upLocal = Quaternion::RotateVector(Vector3(0.0f, 1.0f, 0.0f), localRotation);

	// 親の回転でワールド方向を求める
	const Vector3 forwardWorld = Quaternion::RotateVector(forwardLocal, parentRotation);
	const Vector3 upWorld = Quaternion::RotateVector(upLocal, parentRotation);

	// ワールド回転を計算
	const Quaternion worldRotateMatrix = Quaternion::LookRotation(forwardWorld, upWorld);
	const Matrix4x4 rotateMatrix = Quaternion::MakeRotateMatrix(worldRotateMatrix);

	// 回転を考慮した発生位置を計算
	const Vector3 emitPos = Quaternion::RotateVector(localPos, parentRotation);

	// コマンドを設定
	{
		ParticleCommand command{};
		command.target = ParticleCommandTarget::Updater;
		command.id = ParticleCommandID::SetRotation;
		command.filter.updaterId = ParticleUpdateModuleID::Rotation;
		command.value = rotateMatrix;
		effect.SendCommand(command);
	}
	{
		ParticleCommand command{};
		command.target = ParticleCommandTarget::Spawner;
		command.id = ParticleCommandID::SetTranslation;
		command.value = emitPos;
		effect.SendCommand(command);
	}
}