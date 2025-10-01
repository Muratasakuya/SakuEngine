#include "ActionProgressMonitor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/Algorithm.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

//============================================================================
//	ActionProgressMonitor classMethods
//============================================================================

ActionProgressMonitor* ActionProgressMonitor::instance_ = nullptr;

ActionProgressMonitor* ActionProgressMonitor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ActionProgressMonitor();
	}
	return instance_;
}

void ActionProgressMonitor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

int ActionProgressMonitor::AddObject(const std::string& name) {

	objects_.emplace_back(Object{ .name = name,.metrics = {} });
	return static_cast<int>(objects_.size() - 1);
}

int ActionProgressMonitor::AddOverall(int objectIndex, const std::string& name,
	std::function<float()> overallGetter) {

	// 存在しない場合は無効なIDを返す
	if (!IsValidObject(objectIndex)) {

		return -1;
	}
	auto& object = objects_[objectIndex];

	// 全体進捗設定
	Metric metric;
	metric.name = name;
	metric.kind = MetricKind::Overall;
	metric.getter = std::move(overallGetter);
	object.metrics.emplace_back(std::move(metric));
	return static_cast<int>(object.metrics.size() - 1);
}

int ActionProgressMonitor::AddSpan(int objectIndex, const std::string& name,
	std::function<float()> startGetter, std::function<float()> endGetter,
	std::function<float()> localGetter) {

	// 存在しない場合は無効なIDを返す
	if (!IsValidObject(objectIndex)) {

		return -1;
	}
	auto& object = objects_[objectIndex];

	// 個別進捗設定
	Metric metric;
	metric.name = name;
	metric.kind = MetricKind::Span;
	metric.start = std::move(startGetter);
	metric.end = std::move(endGetter);
	metric.local = std::move(localGetter);
	object.metrics.emplace_back(std::move(metric));
	return static_cast<int>(object.metrics.size() - 1);
}

void ActionProgressMonitor::SetSelectedObject(int index) {

	if (IsValidObject(index)) {

		selectedObject_ = index;
	}
}

void ActionProgressMonitor::SetSelectedMetric(int index) {

	if (IsValidMetric(index)) {

		selectedMetric_ = index;
	}
}

std::optional<float> ActionProgressMonitor::GetSelectedValue() const {

	if (!IsValidObject(selectedObject_) || !IsValidMetric(selectedMetric_)) {

		return std::nullopt;
	}
	return Algorithm::Clamp(objects_[selectedObject_].metrics[selectedMetric_].getter());
}

bool ActionProgressMonitor::IsValidObject(int index) const {

	return (0 <= index) && (index < static_cast<int>(objects_.size()));
}

bool ActionProgressMonitor::IsValidMetric(int index) const {

	if (!IsValidObject(selectedObject_)) {
		return false;
	}
	return (0 <= index) && (index < static_cast<int>(objects_[selectedObject_].metrics.size()));
}

void ActionProgressMonitor::ImGui() {

	if (ImGuiHelper::BeginFramedChild("ProgressMonitor", nullptr, ImVec2(0.0f, 0.0f))) {

		// オブジェクト選択
		std::vector<std::string> names;
		names.reserve(objects_.size());
		for (const auto& object : objects_) names.push_back(object.name);
		ImGuiHelper::SelectableListFromStrings("Objects", &selectedObject_, names, 8);

		if (IsValidObject(selectedObject_)) {
			auto& object = objects_[selectedObject_];

			// メトリクス選択
			std::vector<std::string> metrics;
			std::vector<int> overallIndexMap;
			metrics.reserve(object.metrics.size());
			overallIndexMap.reserve(object.metrics.size());
			for (int i = 0; i < (int)object.metrics.size(); ++i) {
				if (object.metrics[i].kind == MetricKind::Overall) {

					metrics.push_back(object.metrics[i].name);
					overallIndexMap.push_back(i);
				}
			}
			static int selectedOverallIdxInList = 0;
			ImGuiHelper::SelectableListFromStrings("Overall (select as baseline)", &selectedOverallIdxInList, metrics, 8);

			// 実際に選択された Overall の object.metrics 内インデックスへ
			if (!overallIndexMap.empty()) {
				selectedMetric_ = overallIndexMap[(size_t)std::clamp(selectedOverallIdxInList, 0, (int)overallIndexMap.size() - 1)];
			}

			if (IsValidMetric(selectedMetric_)) {

				float overall = Algorithm::Clamp(object.metrics[selectedMetric_].getter());

				ImGui::SeparatorText("Selected Overall");
				ImGui::Text("Object : %s", object.name.c_str());
				ImGui::Text("Overall: %s  (%5.2f %%)", object.metrics[selectedMetric_].name.c_str(), overall * 100.0f);

				// 上段：全体バー（ImGui::ProgressBar）
				const float bigH = 22.0f;
				// ProgressBar は ItemWidth に依存しないので寸法を直接渡す
				ImGui::ProgressBar(overall, ImVec2(-FLT_MIN, bigH), ""); // overlay文字は任意
				ImGui::Dummy(ImVec2(0, 8.0f));

				// 下段：Span を「開始位置オフセット＋長さ」で ProgressBar 表示
				ImGui::SeparatorText("Spans within Overall");

				// 基準となる “行の左上座標” と “使用可能幅” を固定しておく
				const ImVec2 rowOrigin = ImGui::GetCursorScreenPos();
				const float  fullW = ImGui::GetContentRegionAvail().x;

				int row = 0;
				for (const auto& m : object.metrics) {
					if (m.kind != MetricKind::Span) continue;

					float s = Algorithm::Clamp(m.start ? m.start() : 0.0f); // start (overall 0..1)
					float e = Algorithm::Clamp(m.end ? m.end() : 1.0f); // end   (overall 0..1)
					if (e < s) std::swap(s, e);
					float w = (std::max)(0.0f, e - s);                      // 長さ (0..1)
					float local = Algorithm::Clamp(m.local ? m.local() : 0.0f); // 区間内 0..1

					// 進捗バーを “全体幅 * s” だけ右にオフセットして描く
					float x0 = rowOrigin.x + fullW * s;
					float width = fullW * w;
					float y0 = rowOrigin.y + row * (bigH + 6.0f);

					// カーソルをオフセット先に移動して、その幅で ProgressBar を描く
					ImGui::SetCursorScreenPos(ImVec2(x0, y0));
					ImGui::ProgressBar(local, ImVec2(width, bigH), "");

					// ラベル（Start/End/Local）を同じ行に
					ImGui::SameLine();
					ImGui::SetCursorScreenPos(ImVec2(rowOrigin.x + fullW + 8.0f, y0 + 2.0f));
					ImGui::Text("%s  start:%4.2f  end:%4.2f  local:%5.1f%%",
						m.name.c_str(), s, e, local * 100.0f);

					++row;
				}
				ImGui::Dummy(ImVec2(0, row * (bigH + 6.0f) + 8.0f));

				// 凡例（Start/End を“全体の左座標”で表示）
				if (ImGui::BeginTable("Legend", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
					ImGui::TableSetupColumn("Span");
					ImGui::TableSetupColumn("Start (overall)");
					ImGui::TableSetupColumn("End (overall)");
					ImGui::TableSetupColumn("Local %");
					ImGui::TableHeadersRow();
					for (const auto& m : object.metrics) {
						if (m.kind != MetricKind::Span) continue;
						float s = Algorithm::Clamp(m.start ? m.start() : 0.0f);
						float e = Algorithm::Clamp(m.end ? m.end() : 1.0f);
						if (e < s) std::swap(s, e);
						float local = Algorithm::Clamp(m.local ? m.local() : 0.0f);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(m.name.c_str());
						ImGui::TableSetColumnIndex(1); ImGui::Text("%4.2f", s);
						ImGui::TableSetColumnIndex(2); ImGui::Text("%4.2f", e);
						ImGui::TableSetColumnIndex(3); ImGui::Text("%5.2f %%", local * 100.0f);
					}
					ImGui::EndTable();
				}
			}
		}
		ImGuiHelper::EndFramedChild();
	}
}