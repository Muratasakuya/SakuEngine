#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Curve/CurveValueEditor.h>
#include <Engine/MathLib/Vector2.h>
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Vector4.h>
#include <Engine/Utility/Animation/LerpKeyframe.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// imgui
#include <imgui.h>
#include <imgui_internal.h>
// c++
#include <vector>

//============================================================================
//	CurveValue class
//============================================================================
template <typename T>
class CurveValue {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CurveValue(const char* label);
	~CurveValue() = default;

	// 進捗(0.0f~1.0f)に応じた値の取得
	T GetValue(float progress) const;

	// imgui
	bool EditSelectCurve();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ラベル
	const char* label_;

	// 値のキー
	std::vector<T> keys_{};
	// x1,y1,x2,y2
	float bezier_[4]{};

	//--------- functions ----------------------------------------------------

	// imgui
	bool ImGuiEditorUI();

	// helper
	static T DefaultValue() { return T{}; }
	// 値フィールドの編集
	static bool EditValueField(const char* label, T& v);
	static T EvalLinearChain(const std::vector<T>& keys, float t);
	static float BezierValueScalar(float dt01, const float P[4]);
	template<int steps>
	static void BezierTable(ImVec2 P[4], ImVec2 results[steps + 1]);
	static bool ImGuiBezier(const char* label, float P[4]);
};

//============================================================================
//	CurveValue templateMethods
//============================================================================

template<typename T>
inline CurveValue<T>::CurveValue(const char* label) :label_(label) {

	// デフォルト値
	bezier_[0] = 0.390f;
	bezier_[1] = 0.575f;
	bezier_[2] = 0.565f;
	bezier_[3] = 1.000f;
	// 2点初期化
	keys_.resize(2);

	// エディターに登録
	CurveValueEditor::GetInstance()->Registry(this, [this]() {
		return this->ImGuiEditorUI(); });
}

template<typename T>
inline T CurveValue<T>::GetValue(float progress) const {

	float t = std::clamp(progress, 0.0f, 1.0f);
	if (keys_.empty()) {
		return T{};
	}
	if (keys_.size() == 1) {
		return keys_.front();
	}

	float remapped = BezierValueScalar(t, bezier_);
	// N点対応
	return EvalLinearChain(keys_, remapped);
}

template<typename T>
inline bool CurveValue<T>::EditSelectCurve() {

	ImGui::SeparatorText(label_);
	ImGui::SameLine();
	ImGui::PushID(label_);
	if (ImGui::SmallButton("Select Curve")) {

		CurveValueEditor::GetInstance()->Open(this);
	}
	ImGui::PopID();

	// 値に変更があったか関数呼び出し元から取得
	return CurveValueEditor::GetInstance()->ConsumeChanged(this);
}

template<typename T>
inline bool CurveValue<T>::ImGuiEditorUI() {

	bool changed = false;
	ImGui::SeparatorText(label_);
	changed |= ImGuiBezier("##bezier", bezier_);
	return changed;
}

template<typename T>
inline bool CurveValue<T>::EditValueField(const char* label, T& v) {

	if constexpr (std::is_same_v<T, float>) {

		return ImGui::DragFloat(label, &v, 0.01f);
	} else if constexpr (std::is_same_v<T, Vector2>) {

		float tmp[2] = { v.x, v.y };
		bool changed = ImGui::DragFloat2(label, tmp, 0.01f);
		if (changed) {
			v.x = tmp[0];
			v.y = tmp[1];
		}
		return changed;
	} else if constexpr (std::is_same_v<T, Vector3>) {

		float tmp[3] = { v.x, v.y, v.z };
		bool changed = ImGui::DragFloat3(label, tmp, 0.01f);
		if (changed) {
			v.x = tmp[0];
			v.y = tmp[1];
			v.z = tmp[2];
		}
		return changed;
	} else if constexpr (std::is_same_v<T, Color>) {

		float tmp[4] = { v.r, v.g, v.b, v.a };
		bool changed = ImGui::ColorEdit4(label, tmp, ImGuiColorEditFlags_Float);
		if (changed) {
			v.r = tmp[0];
			v.g = tmp[1];
			v.b = tmp[2];
			v.a = tmp[3];
		}
		return changed;
	} else {

		// 未対応型は処理しない
		return false;
	}
}

template<typename T>
inline T CurveValue<T>::EvalLinearChain(const std::vector<T>& keys, float t) {

	// 補間処理対応している型か
	if constexpr (LerpKeyframe::Lerpable<T>) {

		// Linearで返す
		return LerpKeyframe::GetValue<T>(keys, t, LerpKeyframe::Type::Linear);
	}
	// Colorは未実装、とりあえずここで作って処理
	else if constexpr (std::is_same_v<T, Color>) {

		const size_t n = keys.size();
		if (n == 0) {

			return Color{};
		}
		if (n == 1) {

			return keys[0];
		}
		const size_t division = n - 1;
		const float area = 1.0f / float(division);
		size_t index = std::min<size_t>(size_t(t / area), division - 1);
		const float localT = (t - index * area) / area;
		return Color::Lerp(keys[index], keys[index + 1], localT);
	} else {

		static_assert(!sizeof(T*), "CurveValue: unsupported type for EvalLinearChain");
	}
}

template<typename T>
inline float CurveValue<T>::BezierValueScalar(float dt01, const float P[4]) {

	enum { STEPS = 256 };
	ImVec2 Q[4] = { {0,0}, {P[0], P[1]}, {P[2], P[3]}, {1,1} };
	ImVec2 results[STEPS + 1];
	CurveValue::BezierTable<STEPS>(Q, results);
	int idx = (int)((dt01 < 0 ? 0 : dt01 > 1 ? 1 : dt01) * STEPS);
	return results[idx].y;
}

template<typename T>
template<int steps>
inline void CurveValue<T>::BezierTable(ImVec2 P[4], ImVec2 results[steps + 1]) {

	static float C[(steps + 1) * 4], * K = nullptr;
	if (!K) {

		K = C;
		for (unsigned step = 0; step <= steps; ++step) {

			float t = (float)step / (float)steps;
			C[step * 4 + 0] = (1 - t) * (1 - t) * (1 - t);
			C[step * 4 + 1] = 3 * (1 - t) * (1 - t) * t;
			C[step * 4 + 2] = 3 * (1 - t) * t * t;
			C[step * 4 + 3] = t * t * t;
		}
	}
	for (unsigned step = 0; step <= steps; ++step) {

		ImVec2 point = {
		K[step * 4 + 0] * P[0].x + K[step * 4 + 1] * P[1].x + K[step * 4 + 2] * P[2].x + K[step * 4 + 3] * P[3].x,
		K[step * 4 + 0] * P[0].y + K[step * 4 + 1] * P[1].y + K[step * 4 + 2] * P[2].y + K[step * 4 + 3] * P[3].y
		};
		results[step] = point;
	}
}

template<typename T>
inline bool CurveValue<T>::ImGuiBezier(const char* label, float P[4]) {

	enum { SMOOTHNESS = 64 };
	constexpr float CURVE_WIDTH = 4.0f;
	constexpr float LINE_WIDTH = 1.0f;
	constexpr float GRAB_RADIUS = 6.0f;
	constexpr float GRAB_BORDER = 2.0f;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	ImGuiWindow* Window = ImGui::GetCurrentWindow();
	if (Window->SkipItems) {

		return false;
	}

	bool changed = false;
	bool hovered = ImGui::IsItemActive() || ImGui::IsItemHovered();
	ImGui::Dummy(ImVec2(0, 3));

	ImGui::PushID(label);

	const float avail = ImGui::GetContentRegionAvail().x;
	const float dim = ImMin(avail, 128.f);
	ImVec2 canvas(dim, dim);

	ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + canvas);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0)) {

		return changed;
	}

	hovered = ImGui::IsMouseHoveringRect(bb.Min, ImVec2(bb.Min.x + avail, bb.Min.y + dim));

	// background grid
	for (int i = 0; i <= canvas.x; i += (int)(canvas.x / 4)) {
		DrawList->AddLine(ImVec2(bb.Min.x + i, bb.Min.y), ImVec2(bb.Min.x + i, bb.Max.y), ImGui::GetColorU32(ImGuiCol_TextDisabled));
	}
	for (int i = 0; i <= canvas.y; i += (int)(canvas.y / 4)) {

		DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + i), ImVec2(bb.Max.x, bb.Min.y + i), ImGui::GetColorU32(ImGuiCol_TextDisabled));
	}

	// evalCurve
	ImVec2 Q[4] = { {0,0}, {P[0], P[1]}, {P[2], P[3]}, {1,1} };
	ImVec2 results[SMOOTHNESS + 1];
	CurveValue::BezierTable<SMOOTHNESS>(Q, results);

	// handles
	char buf[128];
	sprintf_s(buf, sizeof(buf), "0##%s", label);
	for (int i = 0; i < 2; ++i) {

		ImGui::PushID(i);

		ImVec2 pos = ImVec2(P[i * 2 + 0], 1 - P[i * 2 + 1]) * (bb.Max - bb.Min) + bb.Min;
		ImGui::SetCursorScreenPos(pos - ImVec2(GRAB_RADIUS, GRAB_RADIUS));
		ImGui::InvisibleButton("handle", ImVec2(2 * GRAB_RADIUS, 2 * GRAB_RADIUS));
		if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {

			ImGui::SetTooltip("(%4.3f, %4.3f)", P[i * 2 + 0], P[i * 2 + 1]);
		}
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {

			P[i * 2 + 0] += ImGui::GetIO().MouseDelta.x / canvas.x;
			P[i * 2 + 1] -= ImGui::GetIO().MouseDelta.y / canvas.y;
			P[i * 2 + 0] = std::clamp(P[i * 2 + 0], 0.0f, 1.0f);
			P[i * 2 + 1] = std::clamp(P[i * 2 + 1], 0.0f, 1.0f);
			changed = true;
		}
		ImGui::PopID();
	}

	// ImGui::PushID(label);
	ImGui::PopID();
	if (hovered || changed) {

		DrawList->PushClipRectFullScreen();
	}
	{
		ImColor color(ImGui::GetStyle().Colors[ImGuiCol_PlotLines]);
		for (int i = 0; i < SMOOTHNESS; ++i) {

			ImVec2 p = { results[i + 0].x, 1 - results[i + 0].y };
			ImVec2 q = { results[i + 1].x, 1 - results[i + 1].y };
			ImVec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
			ImVec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x, q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
			DrawList->AddLine(r, s, color, CURVE_WIDTH);
		}
		float luma = (ImGui::IsItemActive() || ImGui::IsItemHovered()) ? 0.5f : 1.0f;
		ImVec4 pink(1.00f, 0.00f, 0.75f, luma), cyan(0.00f, 0.75f, 1.00f, luma);
		ImVec4 white(ImGui::GetStyle().Colors[ImGuiCol_Text]);
		ImVec2 p1 = ImVec2(P[0], 1 - P[1]) * (bb.Max - bb.Min) + bb.Min;
		ImVec2 p2 = ImVec2(P[2], 1 - P[3]) * (bb.Max - bb.Min) + bb.Min;
		DrawList->AddLine(ImVec2(bb.Min.x, bb.Max.y), p1, ImColor(white), LINE_WIDTH);
		DrawList->AddLine(ImVec2(bb.Max.x, bb.Min.y), p2, ImColor(white), LINE_WIDTH);
		DrawList->AddCircleFilled(p1, GRAB_RADIUS, ImColor(white));
		DrawList->AddCircleFilled(p1, GRAB_RADIUS - GRAB_BORDER, ImColor(pink));
		DrawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(white));
		DrawList->AddCircleFilled(p2, GRAB_RADIUS - GRAB_BORDER, ImColor(cyan));
	}
	if (hovered || changed) {

		DrawList->PopClipRect();
	}
	ImGui::SetCursorScreenPos(ImVec2(bb.Min.x, bb.Max.y + GRAB_RADIUS));
	return changed;
}