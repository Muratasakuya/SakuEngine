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
		for (const auto& object : objects_) {

			names.push_back(object.name);
		}
		// オブジェクトの名前リスト
		ImGuiHelper::SelectableListFromStrings("Objects", &selectedObject_, names, 4);

		if (IsValidObject(selectedObject_)) {

			// メトリクス選択
			auto& object = objects_[selectedObject_];
			std::vector<std::string> metrics;
			std::vector<int> overallIndexMap;
			metrics.reserve(object.metrics.size());
			overallIndexMap.reserve(object.metrics.size());
			for (int i = 0; i < static_cast<int>(object.metrics.size()); ++i) {

				// 全体進捗マップを作成
				if (object.metrics[i].kind == MetricKind::Overall) {

					metrics.push_back(object.metrics[i].name);
					overallIndexMap.push_back(i);
				}
			}
			// 全体進捗の名前リスト
			static int selectedOverallIndexInList = 0;
			ImGuiHelper::SelectableListFromStrings("Overall",
				&selectedOverallIndexInList, metrics, 4);
			if (!overallIndexMap.empty()) {

				selectedMetric_ = overallIndexMap[std::clamp(selectedOverallIndexInList, 0, static_cast<int>(overallIndexMap.size()) - 1)];
			}

			if (IsValidMetric(selectedMetric_)) {

				// 進捗バーの高さ
				const float bigHeight = 24.0f;
				float overallRaw = Algorithm::Clamp(object.metrics[selectedMetric_].getter());
				float overallShown = scrubProgressEnabled_ ? scrubOverall_ : overallRaw;

				// 手動進捗
				{
					ImGui::SeparatorText("Drag");
					ImGui::PushItemWidth(160.0f);
					if (ImGui::DragFloat("Overall", &scrubOverall_, 0.001f, 0.0f, 1.0f)) {

						scrubProgressEnabled_ = true;
					}
					if (ImGui::Button("Reset##Overall")) {

						scrubProgressEnabled_ = false;
						scrubOverall_ = overallRaw;
					}
					ImGui::PopItemWidth();
					ImGui::Separator();
				}

				// 全体進捗表示
				{
					ImGui::SeparatorText("Selected Overall");
					ImGui::SetWindowFontScale(0.8f);

					// オブジェクト、進捗名
					ImGui::Text("Object : %s", object.name.c_str());
					ImGui::Text("Overall: %s  (%5.2f %%)", object.metrics[selectedMetric_].name.c_str(), overallShown * 100.0f);

					// 進捗率
					ImGui::ProgressBar(overallShown, ImVec2(-FLT_MIN, bigHeight), "");

					ImGui::Dummy(ImVec2(0, 8.0f));
					ImGui::SetWindowFontScale(1.0f);
				}
				// 登録されている個別進捗
				{
					ImGui::SeparatorText("Spans");
					ImGui::SetWindowFontScale(0.8f);

					// 基準の行の左上座標と使用可能幅を取得
					const ImVec2 rowOrigin = ImGui::GetCursorScreenPos();
					const float fullWidth = ImGui::GetContentRegionAvail().x;
					const float labelHeight = ImGui::GetTextLineHeightWithSpacing();
					const float rowGap = 8.0f;

					int row = 0;
					for (const auto& metric : object.metrics) {

						if (metric.kind == MetricKind::Overall) {
							continue;
						}

						// 進捗開始、終了地点
						float start = Algorithm::Clamp(metric.start());
						float end = Algorithm::Clamp(metric.end());
						// 開始地点の方が値が大きければ入れ替え
						if (end < start) {

							std::swap(start, end);
						}
						// 進捗の長さ
						float width = (std::max)(0.0f, end - start);

						// ローカルの進捗率
						float localFromOverall = 0.0f;
						if (0.0f < width) {

							float t = (overallShown - start) / width;
							localFromOverall = Algorithm::Clamp(t);
						}
						float localShown = scrubProgressEnabled_ ? localFromOverall
							: Algorithm::Clamp(metric.local());

						const float rowY = rowOrigin.y + row * (labelHeight + bigHeight + rowGap);
						const float posX = rowOrigin.x + fullWidth * start;

						// 進捗名
						ImGui::SetCursorScreenPos(ImVec2(posX, rowY));
						ImGui::Text("ProgressName: %s [%.2f -> %.2f] local: %5.1f%%",
							metric.name.c_str(), start, end, localShown * 100.0f);
						// 進捗率
						ImGui::SetCursorScreenPos(ImVec2(posX, rowY + labelHeight));
						ImGui::ProgressBar(localShown, ImVec2(fullWidth * width, bigHeight), "");
						++row;
					}
					ImGui::SetWindowFontScale(1.0f);
				}
			}
		}
		ImGuiHelper::EndFramedChild();
	}
}