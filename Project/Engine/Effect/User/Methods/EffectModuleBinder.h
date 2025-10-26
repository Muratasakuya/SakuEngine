#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Methods/EffectStructures.h>

// front
class ParticleSystem;

//============================================================================
//	EffectModuleBinder class
//	エフェクトモジュールにコマンド設定を適応する
//============================================================================
class EffectModuleBinder {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectModuleBinder() = default;
	~EffectModuleBinder() = default;

	// 発生前にモジュールに設定を適応する
	static void ApplyPreEmit(ParticleSystem* system, const EffectModuleSetting& module,
		const Vector3& worldPos, const EffectCommandContext& context);
	// 更新時にモジュールに設定を適応する
	static void ApplyUpdate(ParticleSystem* system, const EffectModuleSetting& module,
		const Vector3& parentTranslation, const Quaternion& parentRotation,
		const EffectCommandContext& context);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	// 引き数で受け取った値でコマンドを作成して返す
	template <typename T>
	static ParticleCommand MakeCommand(ParticleCommandTarget target, ParticleCommandID id, const T& value);
};

//============================================================================
//	EffectModuleBinder templateMethods
//============================================================================

template<typename T>
inline ParticleCommand EffectModuleBinder::MakeCommand(
	ParticleCommandTarget target, ParticleCommandID id, const T& value) {

	ParticleCommand command{};
	command.target = target;
	command.id = id;
	command.value = value;
	return command;
}