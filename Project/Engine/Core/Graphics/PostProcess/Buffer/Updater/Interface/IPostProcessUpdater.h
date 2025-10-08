#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/Updater/Interface/PostProcessUpdaterBase.h>
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBufferSize.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

// imgui
#include <imgui.h>

//============================================================================
//	IPostProcessUpdater class
//============================================================================
template <typename T>
class IPostProcessUpdater :
	public PostProcessUpdaterBase {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPostProcessUpdater() = default;
	~IPostProcessUpdater() = default;

	// 初期化処理
	virtual void Init() = 0;

	// 更新処理
	virtual void Update() = 0;

	// imgui
	virtual void ImGui() = 0;

	// 呼び出し
	// 開始
	virtual void Start() = 0;
	// 停止
	virtual void Stop() = 0;
	// リセット
	virtual void Reset() = 0;

	//--------- accessor -----------------------------------------------------

	std::pair<const void*, size_t> GetBufferData() const override;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// jsonの保存パス
	const std::string kJsonBasePath_ = "PostProcessUpdater/";

	// bufferに渡す値
	T bufferData_;

	//--------- functions ----------------------------------------------------

	// json
	virtual void ApplyJson() {}
	virtual void SaveJson() {}
	bool LoadFile(Json& data);
	void SaveFile(const Json& data);

	// imgui
	void SaveButton();

	// helper
	std::string GetFileName() const;
};

//============================================================================
//	IPostProcessUpdater templateMethods
//============================================================================

template<typename T>
inline std::pair<const void*, size_t> IPostProcessUpdater<T>::GetBufferData() const {

	return{ &bufferData_, sizeof(T) };
}

template<typename T>
inline bool IPostProcessUpdater<T>::LoadFile(Json& data) {

	return JsonAdapter::LoadCheck(GetFileName(), data);
}

template<typename T>
inline void IPostProcessUpdater<T>::SaveFile(const Json& data) {

	JsonAdapter::Save(GetFileName(), data);
}

template<typename T>
inline void IPostProcessUpdater<T>::SaveButton() {

	if (ImGui::Button("Save")) {

		SaveJson();
	}
}

template<typename T>
inline std::string IPostProcessUpdater<T>::GetFileName() const {

	std::string name = Algorithm::DemangleType(typeid(*this).name());
	name = Algorithm::RemoveSubstring(name, "class ");
	return kJsonBasePath_ + Algorithm::AdjustLeadingCase(
		std::move(name), Algorithm::LeadingCase::Lower) + ".json";
}