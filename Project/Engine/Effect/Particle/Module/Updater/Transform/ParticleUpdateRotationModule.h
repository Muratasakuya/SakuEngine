#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

//============================================================================
//	ParticleUpdateRotationModule class
//============================================================================
class ParticleUpdateRotationModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateRotationModule() = default;
	~ParticleUpdateRotationModule() = default;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	bool SetCommand(const ParticleCommand& command) override;

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

		Lerp,
		ConstantAdd,
		MoveToDirection
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
	ParticleCommon::LerpValue<Vector3> lerpRotation_;

	// 回転加算
	Vector3 addRotation_;

	// 回転固定
	Vector3 lockAxis_;

	// 外部設定
	std::optional<Vector3> setRotation_;
	std::optional<Matrix4x4> setRotationMatrix_;

	// ビルボードの種類
	ParticleBillboardType billboardType_;

	EasingType easing_;
	UpdateType updateType_;
	LockAxisType lockAxisType_;

	//--------- functions ----------------------------------------------------

	Vector3 UpdateRotation(CPUParticle::ParticleData& particle) const;
	Vector3 LockAxis(const Vector3& rotation);
	void UpdateMatrix(CPUParticle::ParticleData& particle, const Vector3& rotation);
};