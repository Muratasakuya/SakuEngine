#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleSpawnModule.h>
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>
#include <Engine/Effect/Particle/Module/ParticleModuleRegistry.h>

// c++
#include <memory>
#include <vector>

//============================================================================
//	ParticlePhase class
//============================================================================
class ParticlePhase {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticlePhase() = default;
	~ParticlePhase() = default;

	//--------- functions ----------------------------------------------------

	void Init(Asset* asset, ParticlePrimitiveType primitiveType);

	// 発生処理
	// 一定間隔
	void FrequencyEmit(std::list<CPUParticle::ParticleData>& particles, float deltaTime);
	// 強制発生
	void Emit(std::list<CPUParticle::ParticleData>& particles);

	// 更新処理
	void UpdateParticle(CPUParticle::ParticleData& particle, float deltaTime);
	void UpdateEmitter();

	// モジュールのコマンド適応
	void ApplyCommand(const ParticleCommand& command);

	// editor
	void ImGui();

	// helpers
	// 発生モジュールの選択
	void SetSpawner(ParticleSpawnModuleID id);
	// 更新モジュール
	// 追加
	void AddUpdater(ParticleUpdateModuleID id);
	// 削除
	void RemoveUpdater(uint32_t index);
	// 入れ替え
	void SwapUpdater(uint32_t from, uint32_t to);

	// json
	Json ToJson() const;
	void FromJson(const Json& data);

	//--------- accessor -----------------------------------------------------

	float GetLifeTime() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Asset* asset_;
	ParticlePrimitiveType primitiveType_;

	// 発生間隔
	float duration_;
	// 現在の経過時間
	float elapsed_;
	// 適応するポストエフェクトのビット
	uint32_t postProcessMask_;
	// emitterとして処理しない
	bool notEmit_ = false;

	using SpawnRegistry = ParticleModuleRegistry<ICPUParticleSpawnModule, ParticleSpawnModuleID>;
	using UpdateRegistry = ParticleModuleRegistry<ICPUParticleUpdateModule, ParticleUpdateModuleID>;

	std::vector<std::unique_ptr<ICPUParticleUpdateModule>> updaters_;

	std::array<std::unique_ptr<ICPUParticleSpawnModule>, static_cast<size_t>(ParticleSpawnModuleID::Count)> spawnerCache_;
	ICPUParticleSpawnModule* spawner_ = nullptr;
	ParticleSpawnModuleID currentSpawnId_ = ParticleSpawnModuleID::Count;

	// editor
	ParticleSpawnModuleID selectSpawnModule_;
	int selectedUpdater_ = -1;
};