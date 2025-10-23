#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/MathLib/MathUtils.h>

// imgui
#include <imgui.h>

//============================================================================
//	ParticleValue class
//	ランダム、定数値を扱うクラス
//============================================================================

// 値の種類
enum class ParticleValueType {

	Constant,
	Random
};
// 定数
template<typename T>
struct ParticleConstantValue {

	T value;
};
// ランダム値
template<typename T>
struct ParticleRandomRangeValue {

	T min;
	T max;

	// ランダム値を生成して取得
	T GetRandomValue() const {
		return RandomGenerator::Generate(min, max);
	}
};

//============================================================================
//	ParticleValue class
//============================================================================
template<typename T>
class ParticleValue {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleValue() = default;
	~ParticleValue() = default;

	//--------- functions ----------------------------------------------------

	// 定数とランダム値に同じ値を設定
	static ParticleValue<T> SetValue(T value);
	// 定数かランダムの値を取得
	T GetValue() const;

	// 分けて処理
	void EditDragValue(const std::string& label);
	void EditColor(const std::string& label);

	void ApplyJson(const Json& data, const std::string& name);
	void SaveJson(Json& data, const std::string& name);

	//--------- variables ----------------------------------------------------

	ParticleValueType valueType;

	// 値
	ParticleConstantValue<T> constant;  // 定数
	ParticleRandomRangeValue<T> random; // ランダム

	// 使用する値
	T value;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	void SelectType(const std::string& label);
};

//============================================================================
//	ParticleValue templateMethods
//============================================================================

template<typename T>
inline ParticleValue<T> ParticleValue<T>::SetValue(T value) {

	ParticleValue<T> particleValue{};
	particleValue.constant.value = value;
	particleValue.random.min = value;
	particleValue.random.max = value;
	return particleValue;
}

template<typename T>
inline T ParticleValue<T>::GetValue() const {

	if (valueType == ParticleValueType::Constant) {

		// 定数値を返す
		return constant.value;
	} else if (valueType == ParticleValueType::Random) {

		// ランダム値を返す
		return random.GetRandomValue();
	}
	// ここを通ることはほぼありえないけど定数値を返しておく
	return constant.value;
}

template<typename T>
inline void ParticleValue<T>::SelectType(const std::string& label) {

	// 定数値かランダムを設定する
	const char* kItems[] = { "Constant", "Random" };
	int comboIndex = static_cast<int>(valueType);
	ImGui::SetNextItemWidth(132.0f);
	ImGui::SetCursorPosX(320.0f);
	if (ImGui::Combo(("##Type" + label).c_str(), &comboIndex, kItems, IM_ARRAYSIZE(kItems))) {

		valueType = static_cast<ParticleValueType>(comboIndex);
	}
}

template<typename T>
inline void ParticleValue<T>::EditDragValue(const std::string& label) {

	ImGui::PushID(label.c_str());

	// 分岐処理
	if constexpr (std::is_same_v<T, uint32_t>) {
		if (valueType == ParticleValueType::Constant) {

			ImGui::DragInt(label.c_str(), reinterpret_cast<int*>(&constant.value), 1, 0, 128);
		} else if (valueType == ParticleValueType::Random) {

			ImGui::DragInt((label + ":Min").c_str(), reinterpret_cast<int*>(&random.min), 1, 0, 128);
			// 定数値かランダムを設定する
			ImGui::SameLine();
			SelectType(label);

			ImGui::DragInt((label + ":Max").c_str(), reinterpret_cast<int*>(&random.max), 1, 0, 128);
		}
	} else if constexpr (std::is_same_v<T, float>) {
		if (valueType == ParticleValueType::Constant) {

			ImGui::DragFloat(label.c_str(), &constant.value, 0.001f);
		} else if (valueType == ParticleValueType::Random) {

			ImGui::DragFloat((label + ":Min").c_str(), &random.min, 0.001f);
			// 定数値かランダムを設定する
			ImGui::SameLine();
			SelectType(label);

			ImGui::DragFloat((label + ":Max").c_str(), &random.max, 0.001f);
		}
	} else if constexpr (std::is_same_v<T, Vector3>) {
		if (valueType == ParticleValueType::Constant) {

			ImGui::DragFloat3(label.c_str(), &constant.value.x, 0.001f);
		} else if (valueType == ParticleValueType::Random) {

			ImGui::DragFloat3((label + ":Min").c_str(), &random.min.x, 0.001f);
			// 定数値かランダムを設定する
			ImGui::SameLine();
			SelectType(label);

			ImGui::DragFloat3((label + ":Max").c_str(), &random.max.x, 0.001f);
		}
	}

	if (valueType == ParticleValueType::Constant) {

		// 定数値かランダムを設定する
		ImGui::SameLine();
		SelectType(label);
	}

	ImGui::PopID();
}

template<typename T>
inline void ParticleValue<T>::EditColor(const std::string& label) {

	ImGui::PushID(label.c_str());

	if constexpr (std::is_same_v<T, Vector3>) {
		if (valueType == ParticleValueType::Constant) {

			ImGui::ColorEdit3(label.c_str(), &constant.value.x);
		} else if (valueType == ParticleValueType::Random) {

			ImGui::ColorEdit3((label + ":Min").c_str(), &random.min.x);
			// 定数値かランダムを設定する
			ImGui::SameLine();
			SelectType(label);

			ImGui::ColorEdit3((label + ":Max").c_str(), &random.max.x);
		}
	} else if constexpr (std::is_same_v<T, Color>) {
		if (valueType == ParticleValueType::Constant) {

			ImGui::ColorEdit4(label.c_str(), &constant.value.r);
		} else if (valueType == ParticleValueType::Random) {

			ImGui::ColorEdit4((label + ":Min").c_str(), &random.min.r);
			// 定数値かランダムを設定する
			ImGui::SameLine();
			SelectType(label);

			ImGui::ColorEdit4((label + ":Max").c_str(), &random.max.r);
		}
	}

	if (valueType == ParticleValueType::Constant) {

		// 定数値かランダムを設定する
		ImGui::SameLine();
		SelectType(label);
	}

	ImGui::PopID();
}

template<typename T>
inline void ParticleValue<T>::ApplyJson(const Json& data, const std::string& name) {

	valueType = data[name]["valueType"];

	// 分岐処理
	if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, float>) {

		constant.value = data[name]["constant"];
		random.min = data[name]["randomMin"];
		random.max = data[name]["randomMax"];
	} else if constexpr (std::is_same_v<T, Vector3>) {

		constant.value = JsonAdapter::ToObject<Vector3>(data[name]["constant"]);
		random.min = JsonAdapter::ToObject<Vector3>(data[name]["randomMin"]);
		random.max = JsonAdapter::ToObject<Vector3>(data[name]["randomMax"]);
	} else if constexpr (std::is_same_v<T, Color>) {

		constant.value = JsonAdapter::ToObject<Color>(data[name]["constant"]);
		random.min = JsonAdapter::ToObject<Color>(data[name]["randomMin"]);
		random.max = JsonAdapter::ToObject<Color>(data[name]["randomMax"]);
	}
}

template<typename T>
inline void ParticleValue<T>::SaveJson(Json& data, const std::string& name) {

	data[name]["valueType"] = static_cast<int>(valueType);

	// 分岐処理
	if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, float>) {

		data[name]["constant"] = constant.value;
		data[name]["randomMin"] = random.min;
		data[name]["randomMax"] = random.max;
	} else if constexpr (std::is_same_v<T, Vector3>) {

		data[name]["constant"] = JsonAdapter::FromObject<Vector3>(constant.value);
		data[name]["randomMin"] = JsonAdapter::FromObject<Vector3>(random.min);
		data[name]["randomMax"] = JsonAdapter::FromObject<Vector3>(random.max);
	} else if constexpr (std::is_same_v<T, Color>) {

		data[name]["constant"] = JsonAdapter::FromObject<Color>(constant.value);
		data[name]["randomMin"] = JsonAdapter::FromObject<Color>(random.min);
		data[name]["randomMax"] = JsonAdapter::FromObject<Color>(random.max);
	}
}