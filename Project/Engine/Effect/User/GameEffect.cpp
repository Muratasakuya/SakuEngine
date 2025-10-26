#include "GameEffect.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Core/ParticleManager.h>

//============================================================================
//	GameEffect classMethods
//============================================================================

void GameEffect::CreateParticleSystem(const std::string& filePath) {

	// Managerに渡して作成
	particleSystem_ = ParticleManager::GetInstance()->CreateParticleSystem(filePath);

	// 初期化値
	hasParent_ = false;
}

void GameEffect::ResetEmitFlag() {

	// リセット
	emitOnce_ = false;
}

void GameEffect::SendCommand(const ParticleCommand& command) {

	// コマンド設定AA
	particleSystem_->ApplyCommand(command);
}

void GameEffect::FrequencyEmit() {

	// 一定間隔で発生
	particleSystem_->FrequencyEmit();
}

void GameEffect::Emit(bool emitOnce) {

	// 発生済みならリセットするまで発生できない
	if (emitOnce_) {
		return;
	}

	// 強制的に発生
	particleSystem_->Emit();

	emitOnce_ = emitOnce;
}

void GameEffect::SetParent(const BaseTransform& parent) {

	if (hasParent_) {
		return;
	}

	particleSystem_->SetParent(parent);
	hasParent_ = true;
}