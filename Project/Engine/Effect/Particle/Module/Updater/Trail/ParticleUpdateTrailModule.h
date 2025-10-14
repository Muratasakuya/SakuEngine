#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

// front
class SceneView;

//============================================================================
//	ParticleUpdateTrailModule class
//============================================================================
class ParticleUpdateTrailModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateTrailModule() = default;
	~ParticleUpdateTrailModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;
	void BuildTransferData(uint32_t particleIndex, const CPUParticle::ParticleData& particle,
		std::vector<ParticleCommon::TrailHeaderForGPU>& transferTrailHeaders,
		std::vector<ParticleCommon::TrailVertexForGPU>& transferTrailVertices,
		const SceneView* sceneView);

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "Trail"; }

	int GetMaxPoints() const { return maxPoints_; }
	int GetSubdivPerSegment() const { return subdivPerSegment_; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Trail;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	bool enable_;       // デフォルトでfalse
	bool isDrawOrigin_; // トレイル元を描画するか
	bool faceCamera_;   // カメラフェイシング帯

	// 幅
	ParticleCommon::LerpValue<float> width_;
	EasingType widthEasing_;

	float lifeTime_;       // 寿命
	float minDistance_;    // 発生移動距離
	float minTime_;        // 時間間隔で発生
	int maxPoints_;        // ノード最大数
	int subdivPerSegment_; // ノード間のサブ分割数
	float uvTileLength_;   // タイル長
};