#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/GameTimer.h>
#include <Engine/Utility/StateTimer.h>
#include <Engine/Utility/AnimationLoop.h>
#include <Lib/MathUtils/Algorithm.h>
#include <Lib/MathUtils/MathUtils.h>

// c++
#include <format>
#include <optional>
// imgui
#include <imgui.h>

//============================================================================
//	SimpleAnimation enum class
//============================================================================

// 補間の仕方
enum class SimpleAnimationType {

	None,  // start -> end
	Return // end -> start
};

//============================================================================
//	SimpleAnimation class
//============================================================================
template <typename T>
class SimpleAnimation {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SimpleAnimation() = default;
	~SimpleAnimation() = default;

	void ImGui(const std::string& label);

	// 0.0fから1.0fの間で補間された値を取得
	void LerpValue(T& value);

	// 動き出し開始
	void Start();
	// リセット
	void Reset();

	// json
	void ToJson(Json& data);
	void FromJson(const Json& data);

	//--------- accessor -----------------------------------------------------

	void SetStart(const T& start) { move_.start = start; }
	void SetEnd(const T& end) { move_.end = end; }
	void SetAnimationType(SimpleAnimationType type) { type_ = type; }

	bool IsStart() const { return isRunning_; }
	bool IsFinished() const { return isFinished_; }

	float GetProgress() const { return timer_.t_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	struct Move {

		T start;               // 開始値
		T end;                 // 終了値
	};

	//--------- variables ----------------------------------------------------

	SimpleAnimationType type_;
	bool isRunning_ = false;
	bool isFinished_ = false;

	StateTimer timer_;
	float rawT_ = 0.0f;
	AnimationLoop loop_;
	Move move_;

	// imguiのサイズ
	const float itemSize_ = 224.0f;
};

//============================================================================
//	SimpleAnimation templateMethods
//============================================================================

template<typename T>
inline void SimpleAnimation<T>::LerpValue(T& value) {

	// ループが開始していないときは何も処理をしない
	if (!isRunning_) {

		// 値を固定
		if (type_ == SimpleAnimationType::None) {

			value = move_.end;
		} else if (type_ == SimpleAnimationType::Return) {

			value = move_.start;
		}
		return;
	}

	// 時間の更新処理
	timer_.Update();
	rawT_ = timer_.current_ / (std::max)(0.0001f, timer_.target_);
	float easedT = EasedValue(timer_.easeingType_, loop_.LoopedT(rawT_));

	T from = move_.start;
	T to = move_.end;
	// 逆向き補間なら値を入れ替える
	if (type_ == SimpleAnimationType::Return) {
		std::swap(from, to);
	}

	// 値の補間処理
	value = Algorithm::Lerp<T>(from, to, easedT);

	// ループ数が最後まで行けば終了
	if (loop_.GetLoopCount() <= std::floor(rawT_)) {

		isFinished_ = true;
		isRunning_ = false;
	}
}

template<typename T>
inline void SimpleAnimation<T>::Start() {

	isRunning_ = true;
	isFinished_ = false;
}

template<typename T>
inline void SimpleAnimation<T>::Reset() {

	isRunning_ = false;
	isFinished_ = false;
	timer_.Reset();
}

template<typename T>
inline void SimpleAnimation<T>::ImGui(const std::string& label) {

	ImGui::PushItemWidth(itemSize_);
	ImGui::PushID(label.c_str());

	ImGuiTreeNodeFlags windowFlag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf;
	if (ImGui::CollapsingHeader("AnimValue", windowFlag)) {

		if constexpr (std::is_same_v<T, float>) {

			ImGui::DragFloat("start", &move_.start, 0.01f);
			ImGui::DragFloat("end", &move_.end, 0.01f);
		} else if constexpr (std::is_same_v<T, int>) {

			ImGui::DragInt("start", &move_.start, 1);
			ImGui::DragInt("end", &move_.end, 1);
		} else if constexpr (std::is_same_v<T, Vector2>) {

			ImGui::DragFloat2("start", &move_.start.x, 0.01f);
			ImGui::DragFloat2("end", &move_.end.x, 0.01f);
		} else if constexpr (std::is_same_v<T, Vector3>) {

			ImGui::DragFloat3("start", &move_.start.x, 0.01f);
			ImGui::DragFloat3("end", &move_.end.x, 0.01f);
		} else if constexpr (std::is_same_v<T, Color>) {

			ImGui::ColorEdit4("start", &move_.start.a);
			ImGui::ColorEdit4("end", &move_.end.a);
		}
	}
	if (ImGui::CollapsingHeader("Timer", windowFlag)) {

		timer_.ImGui("Time", false);
	}
	if (ImGui::CollapsingHeader("Loop", windowFlag)) {

		loop_.ImGuiLoopParam(false);
	}

	ImGui::PopID();
	ImGui::PopItemWidth();
}

template<typename T>
inline void SimpleAnimation<T>::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	loop_.FromLoopJson(data);
	timer_.FromJson(data.value("Timer", Json()));

	// moveの値を適応
	if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int>) {

		move_.start = data.value("move_.start", 0.0f);
		move_.end = data.value("move_.end", 0.0f);
	} else {

		move_.start = T::FromJson(data["move_.start"]);
		move_.end = T::FromJson(data["move_.end"]);
	}
}

template<typename T>
inline void SimpleAnimation<T>::ToJson(Json& data) {

	loop_.ToLoopJson(data["Loop"]);
	timer_.ToJson(data["Timer"]);

	// moveの値を保存
	if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int>) {

		data["move_.start"] = move_.start;
		data["move_.end"] = move_.end;
	} else {

		data["move_.start"] = move_.start.ToJson();
		data["move_.end"] = move_.end.ToJson();
	}
}