#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>

// c++
#include <functional>

//============================================================================
//	ActionProgressMonitor class
//============================================================================
class ActionProgressMonitor :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void ImGui() override;

	// オブジェクト追加
	int AddObject(const std::string& name);
	// 進捗度の追加
	// 全体
	int AddOverall(int objectIndex, const std::string& name, std::function<float()> overallGetter);
	int AddSpan(int objectIndex, const std::string& name, std::function<float()> startGetter,
		std::function<float()> endGetter, std::function<float()> localGetter);

	//--------- accessor -----------------------------------------------------

	void SetSelectedObject(int index);
	void SetSelectedMetric(int index);

	// 選択中のオブジェクト進捗度
	std::optional<float> GetSelectedValue() const;

	// singleton
	static ActionProgressMonitor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structurec ----------------------------------------------------

	// 進捗の種類
	enum class MetricKind {

		Overall, // 全体
		Span     // 個別
	};

	// 進捗ごとの経路
	struct Metric {

		std::string name;
		MetricKind kind = MetricKind::Overall;

		std::function<float()> getter;

		std::function<float()> start; // 全体の開始位置
		std::function<float()> end;   // 全体の終了位置
		std::function<float()> local; // 区間内の位置
	};
	struct Object {

		std::string name;
		std::vector<Metric> metrics;
	};

	//--------- variables ----------------------------------------------------

	static ActionProgressMonitor* instance_;

	// 進捗を保持するオブジェクト
	std::vector<Object> objects_;

	// エディター
	int selectedObject_ = -1;
	int selectedMetric_ = -1;

	//--------- functions ----------------------------------------------------

	// helper
	bool IsValidObject(int index) const;
	bool IsValidMetric(int index) const;

	ActionProgressMonitor() :IGameEditor("ActionProgressMonitor") {}
	~ActionProgressMonitor() = default;
	ActionProgressMonitor(const ActionProgressMonitor&) = delete;
	ActionProgressMonitor& operator=(const ActionProgressMonitor&) = delete;
};