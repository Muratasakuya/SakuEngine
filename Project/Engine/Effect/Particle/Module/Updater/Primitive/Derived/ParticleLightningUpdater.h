#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Updater/Primitive/Interface/BaseParticlePrimitiveUpdater.h>

//============================================================================
//	ParticleLightningUpdater class
//	雷型のパーティクルを更新するクラス
//============================================================================
class ParticleLightningUpdater :
	public BaseParticlePrimitiveUpdater<LightningForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleLightningUpdater() = default;
	~ParticleLightningUpdater() = default;

	void Init() override;

	void Update(CPUParticle::ParticleData& particle, EasingType easingType) override;

	void ImGui() override;

	// json
	void FromJson(const Json& data) override;
	void ToJson(Json& data) const override;

	//--------- accessor -----------------------------------------------------

	ParticlePrimitiveType GetType() override { return ParticlePrimitiveType::Lightning; }

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 進行方向に雷の終了地点を向けるか
	bool isLookAtEnd_;

	// 開始地点と終了地点をデバッグ表示するか
	bool isDrawDebugPoint_;
};