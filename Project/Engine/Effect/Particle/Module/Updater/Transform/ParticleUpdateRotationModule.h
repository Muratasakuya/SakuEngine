#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

//============================================================================
//	ParticleUpdateRotationModule class
//	回転を更新するモジュール
//============================================================================
class ParticleUpdateRotationModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateRotationModule() = default;
	~ParticleUpdateRotationModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetCommand(const ParticleCommand& command) override;

	const char* GetName() const override { return "Rotation"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Rotation;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 更新の種類
	enum class UpdateType {

		Slerp,           // 補間
		AngularVelocity, // 角速度
		LookToVelocity   // 速度方向を向く
	};

	// 回転軸の固定
	enum class LockAxisType {

		None,
		AxisX,
		AxisY,
		AxisZ,
	};

	//--------- variables ----------------------------------------------------

	// 回転補間
	// 追加回転数
	int slerpExtraTurns_ = 0;
	bool slerpPreferLongArc_ = false;
	ParticleCommon::LerpValue<Quaternion> lerpRotation_;

	// 回転加算
	Vector3 angleAxis_;      // 軸
	float angleSpeedRadian_; // 回転速度

	// 外部設定
	std::optional<Quaternion> setRotation_;

	// ビルボードの種類
	ParticleBillboardType billboardType_;
	EasingType easing_;
	UpdateType updateType_;

	// 回転固定
	LockAxisType lockAxisType_;
	float lockAxisAngle_;

	//--------- functions ----------------------------------------------------

	static void ToAxisAngle(const Quaternion& rotation, Vector3& axis, float& angle);
	Quaternion UpdateRotation(CPUParticle::ParticleData& particle, float deltaTime) const;
	Quaternion LockAxis(const Quaternion& rotation) const;
	void UpdateMatrix(CPUParticle::ParticleData& particle, const Quaternion& rotation);
};