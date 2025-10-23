#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>
#include <Engine/Utility/Animation/LerpKeyframe.h>

//============================================================================
//	ParticleUpdateKeyframePathModule class
//============================================================================
class ParticleUpdateKeyframePathModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateKeyframePathModule() = default;
	~ParticleUpdateKeyframePathModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetCommand(const ParticleCommand& command) override;

	const char* GetName() const override { return "KeyframePath"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::KeyframePath;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 補間タイプ
	LerpKeyframe::Type type_;
	// 補間キー
	std::vector<Vector3> keys_;
	// 親として動かす座標、回転
	Vector3 parentTranslation_;
	Quaternion parentRotation_;

	// 強さ（半径）
	ParticleCommon::LerpValue<float> swirlRadius_;
	EasingType swirlRadiusEasing_;
	float swirlTurns_;  // 0.0fから1.0fの間で何回ループするか
	float swirlPhase_;  // 角度の初期オフセット

	// 発生したパーティクルに角度でオフセットをかける
	bool  spawnAngleEnable_; // 有効フラグ
	bool spawnAngleWrap_;    // 1周したら角度をリセットするか
	float spawnAngleStart_;  // 開始角
	float spawnAngleStride_; // 角度間隔
	float spawnAngleJitter_; // ノイズ

	// 連続した角度を設定するインデックス
	uint64_t spawnAngleSerial_ = 0;

	// キーフレーム線の描画
	bool isDrawKeyframe_ = true;

	//--------- functions ----------------------------------------------------

	// helper
	Vector3 GetTangent(float t) const;
};