#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleSpawnModule.h>
#include <Engine/Effect/Particle/Module/Spawner/ParticleSpawnModuleUpdater.h>

//============================================================================
//	ParticleSpawnPolygonVertexModule class
//	多角形頂点発生モジュール
//============================================================================
class ParticleSpawnPolygonVertexModule :
	public ICPUParticleSpawnModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleSpawnPolygonVertexModule() = default;
	~ParticleSpawnPolygonVertexModule() = default;

	void Init() override;

	void Execute(std::list<CPUParticle::ParticleData>& particles) override;

	void UpdateEmitter() override;
	void DrawEmitter() override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetCommand(const ParticleCommand& command) override;

	const char* GetName() const override { return "SpawnPolygonVertex"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleSpawnModuleID ID = ParticleSpawnModuleID::PolygonVertex;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 多角形のインスタンス
	struct PolygonInstance {

		float scale;
		Vector3 rotation;
		bool active = true;

		std::vector<Vector3> prevVertices;
		ParticlePolygonVertexUpdater updater;
	};

	//--------- variables ----------------------------------------------------

	// パーティクル間の補間処理を行うか
	bool isInterpolate_;
	// 移動していなければ発生させないか
	bool notMoveEmit_;
	bool updateEnable_;
	bool multiEmit_;

	Vector3 emitterRotation_;
	Vector3 translation_;
	bool useBillboardRotation_ = false;
	Matrix4x4 billboardRotation_;

	// 多角形
	int vertexCount_; // 頂点数
	float scale_;     // サイズ

	ParticleValue<uint32_t> emitPerVertex_;   // 各頂点の発生数
	std::vector<Vector3> prevVertices_;       // 前フレームの頂点位置
	ParticleValue<float> interpolateSpacing_; // パーティクル間の距離

	// マルチ発生の設定
	bool useMulti_ = false; // インスタンスで複数発生させるかどうか
	int spawnBurstCount_;   // 0=無限、>0=この回数だけ起動
	int spawned_;           // 起動済み数
	int maxConcurrent_;     // 同時最大発生数
	StateTimer spawnTimer_; // 起動間隔管理
	Vector3 offsetRotation_; // オフセット回転

	bool isSelfUpdate_;
	ParticlePolygonVertexUpdater updater_;
	std::vector<PolygonInstance> instances_;

	//--------- functions ----------------------------------------------------

	// 発生可能かどうかのフラグ
	bool EnableEmit();
	// 間を補間して発生
	void InterpolateEmit(std::list<CPUParticle::ParticleData>& particles);
	// 通常発生
	void NoneEmit(std::list<CPUParticle::ParticleData>& particles);

	// N頂点の多角形頂点を計算
	std::vector<Vector3> CalcVertices() const;
	std::vector<Vector3> CalcVertices(float scale, const Vector3& rotation) const;

	// 1インスタンスの発生処理
	void SpawnInstance();
	void EmitForInstance(PolygonInstance& instance, std::list<CPUParticle::ParticleData>& particles);
};