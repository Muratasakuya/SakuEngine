#include "DelayedHitstop.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>

// imgui
#include <imgui.h>

//============================================================================
//	DelayedHitstop classMethods
//============================================================================

void DelayedHitstop::Start() {

	// リセットしてから開始
	Reset();
	isStart_ = true;
}

void DelayedHitstop::Reset() {

	// リセット
	isStart_ = false;
	currentIndex_ = 0;
	delayElapsed_ = 0.0f;
}

void DelayedHitstop::Update() {

	// 開始していなければ処理しない
	if (!isStart_) {
		return;
	}

	// インデックスの範囲が超えたら処理を停止する
	if (static_cast<uint32_t>(hitstopInfos_.size()) <= currentIndex_) {

		Reset();
		return;
	}
	// 念のためインデックスの範囲を制御
	currentIndex_ = std::clamp(currentIndex_, 0u, static_cast<uint32_t>(hitstopInfos_.size() - 1));

	// 遅延時間の経過
	delayElapsed_ += GameTimer::GetDeltaTime();

	// 時間経過したら
	const auto& currentInfo = hitstopInfos_[currentIndex_];
	if (currentInfo.delay <= delayElapsed_) {

		// ヒットストップ開始
		GameTimer::StartHitStop(currentInfo.duration, currentInfo.timeScale);

		// リセットしてインデックスを進める
		delayElapsed_ = 0.0f;
		++currentIndex_;
	}
}

void DelayedHitstop::ImGui(const std::string& name, bool isSeparate) {

	if (isSeparate) {

		ImGui::SeparatorText(name.c_str());
	}
	ImGui::PushID(name.c_str());

	ImGui::Text(std::format("isStart: {}", isStart_).c_str());
	ImGui::Text(std::format("currentIndex: {}", currentIndex_).c_str());
	ImGui::Text(std::format("delayElapsed: {:.4f}", delayElapsed_).c_str());

	ImGui::Separator();

	if (ImGui::Button("Add HitStop")) {

		HitStopInfo info{};
		hitstopInfos_.emplace_back(info);
	}

	ImGui::Spacing();

	// 編集・削除・並び替え
	int32_t eraseIndex = -1;

	for (size_t i = 0; i < hitstopInfos_.size(); ++i) {
		auto& info = hitstopInfos_[i];

		ImGui::PushID(static_cast<int32_t>(i));

		// 見出し
		ImGui::Separator();
		ImGui::Text("HitStop %zu", i);

		// 値編集
		ImGui::DragFloat("TimeScale", &info.timeScale, 0.01f);
		ImGui::DragFloat("Duration", &info.duration, 0.01f);
		ImGui::DragFloat("Delay", &info.delay, 0.01f);

		// 並び替え／削除ボタン
		if (ImGui::SmallButton("Up") && i > 0) {

			std::swap(hitstopInfos_[i], hitstopInfos_[i - 1]);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Down") && i + 1 < hitstopInfos_.size()) {
			std::swap(hitstopInfos_[i], hitstopInfos_[i + 1]);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Delete")) {

			eraseIndex = static_cast<int32_t>(i);
		}

		ImGui::PopID();
	}

	// ループ外で削除
	if (eraseIndex >= 0 && eraseIndex < static_cast<int>(hitstopInfos_.size())) {
		
		// 情報削除
		hitstopInfos_.erase(hitstopInfos_.begin() + eraseIndex);
		// 再生中インデックスの補正
		if (currentIndex_ >= hitstopInfos_.size()) {

			currentIndex_ = (hitstopInfos_.empty()) ? 0u : static_cast<uint32_t>(hitstopInfos_.size() - 1);
		}
	}

	ImGui::PopID();
}

void DelayedHitstop::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	hitstopInfos_.clear();
	for (const auto& infoData : data["hitstops"]) {

		HitStopInfo info;
		info.timeScale = infoData.value("timeScale", 0.0f);
		info.duration = infoData.value("duration", 0.2f);
		info.delay = infoData.value("delay", 0.4f);

		// 情報追加
		hitstopInfos_.emplace_back(info);
	}
}

void DelayedHitstop::ToJson(Json& data) const {

	data["hitstops"] = Json::array();
	for (const auto& info : hitstopInfos_) {

		Json infoData;
		infoData["timeScale"] = info.timeScale;
		infoData["duration"] = info.duration;
		infoData["delay"] = info.delay;

		// 情報追加
		data["hitstops"].emplace_back(infoData);
	}
}