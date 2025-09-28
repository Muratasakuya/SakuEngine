#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

// c++
#include <memory>
#include <vector>
#include <unordered_map>

//============================================================================
//	ParticleModuleRegistry class
//============================================================================
template<class Base, class EnumT>
class ParticleModuleRegistry {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleModuleRegistry() = default;
	~ParticleModuleRegistry() = default;

	//---------- using ------------------------------------------------------

	using CreateFn = std::unique_ptr<Base>(*)();

	//--------- functions -----------------------------------------------------

	std::unique_ptr<Base> Create(EnumT id) const;

	template<class ModuleT>
	void Register();

	//--------- accessor -----------------------------------------------------

	std::vector<const char*> GetNames() const;

	// singleton
	static ParticleModuleRegistry& GetInstance() {

		static ParticleModuleRegistry registry{};
		return registry;
	};
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unordered_map<EnumT, CreateFn> creators_;

	//--------- functions ----------------------------------------------------

	void RegisterFn(EnumT id, CreateFn fn);
};

//============================================================================
//	ParticleModuleRegistry templateMethods
//============================================================================

template<class Base, class EnumT>
inline std::unique_ptr<Base> ParticleModuleRegistry<Base, EnumT>::Create(EnumT id) const {

	auto it = creators_.find(id);
	return it != creators_.end() ? it->second() : nullptr;
}

template<class Base, class EnumT>
template<class ModuleT>
inline void ParticleModuleRegistry<Base, EnumT>::Register() {

	RegisterFn(ModuleT::ID, []() -> std::unique_ptr<Base> {
		return std::make_unique<ModuleT>(); });
}

template<class Base, class EnumT>
inline void ParticleModuleRegistry<Base, EnumT>::RegisterFn(EnumT id, CreateFn fn) {

	creators_[id] = fn;
}

template<class Base, class EnumT>
inline std::vector<const char*> ParticleModuleRegistry<Base, EnumT>::GetNames() const {

	std::vector<const char*> names{};
	for (auto [type, fn] : creators_) {

		names.emplace_back(EnumAdapter<EnumT>::ToString(type));
	}
	return names;
}