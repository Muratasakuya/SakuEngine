#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Command/ParticleCommand.h>
#include <Engine/Effect/Particle/System/ParticleSystem.h>

// c++
#include <string>

//============================================================================
//	GameEffect class
//	エディターで作成したエフェクトをゲーム上で扱うためのクラス
//============================================================================
class GameEffect {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameEffect() = default;
	~GameEffect() = default;

	// ParticleSystemの作成
	void CreateParticleSystem(const std::string& filePath);

	// モジュールのコマンド適応
	void SendCommand(const ParticleCommand& command);

	// リセット
	void ResetEmitFlag();

	//----------- emit -------------------------------------------------------

	// 一定間隔
	void FrequencyEmit();
	// 強制発生
	void Emit(bool emitOnce = false);

	//--------- accessor -----------------------------------------------------

	void SetParent(const BaseTransform& parent);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ParticleSystem* particleSystem_;

	// 1回だけ発生させるか
	bool emitOnce_;
	bool hasParent_;
};