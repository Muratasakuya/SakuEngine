#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleSpawnModule.h>

//============================================================================
//	ParticleSpawnCircleModule class
//	円発生モジュール
//============================================================================
class ParticleSpawnCircleModule :
	public ICPUParticleSpawnModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleSpawnCircleModule() = default;
	~ParticleSpawnCircleModule() = default;

	void Init() override;

	void Execute(std::list<CPUParticle::ParticleData>& particles) override;

	void DrawEmitter() override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetCommand(const ParticleCommand& command) override;

	const char* GetName() const override { return "Circle"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleSpawnModuleID ID = ParticleSpawnModuleID::Circle;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 発生方法
	enum class SpawnMode {

		Random,       // ランダム発生
		EvenPerFrame, // 発生の個数に応じて等間隔
		Progressive,  // 発生させるごとに発生角度を進める
	};

	//--------- variables ----------------------------------------------------

	// 発生方法
	SpawnMode mode_;

	float radius_; // 円の半径

	Vector3 translation_; // 座標
	Quaternion rotation_; // 回転

	// angle設定
	float angleMin_; // 最小角度
	float angleMax_; // 最大角度
	bool clockwise_; // 進める符号の方向

	float stepAngle_;    // Progressiveの角度の進み
	float currentAngle_; // 現在の角度

	// エディター
	int circleDivision_; // 分割数

	//--------- functions ----------------------------------------------------
	
	// 角度を進める
	void UpdateAdvanceProgressive(uint32_t emitCount);
};