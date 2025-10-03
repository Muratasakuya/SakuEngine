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

	void SetSpanSetter(int objectID, const std::string& spanName,
		std::function<void(float)> setter);

	void SetSelectedObject(int index);
	void SetSelectedMetric(int index);

	// 同期状態の通知
	void SetSynchToggleHandler(int objectID, std::function<void(bool)> handler);
	void NotifySynchState(int objectID, bool external) const;

	// 選択中のオブジェクト進捗度
	std::optional<float> GetSelectedValue() const;
	// 名前からIDの取得
	int FindObjectID(const std::string& name) const;
	bool DriveSpanByGlobalT(int objectId, const std::string& spanName, float globalT);

	// IDから補間値を取得する
	bool GetSpanStart(int objectID, const std::string& spanName, float* outStart) const;
	bool GetSpanEnd(int objectID, const std::string& spanName, float* outEnd) const;
	bool GetSpanLocal(int objectID, const std::string& spanName, float* outLocal) const;
	bool GetOverallValue(int objectID, const std::string& overallName, float* outValue) const;
	// 進捗名
	std::vector<std::string> GetOverallNames(int objectID) const;
	std::vector<std::string> GetSpanNames(int objectID) const;

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

	// 進捗ハンドル
	struct SpanHandle {

		// 処理補間値の取得
		std::function<float()> getStart;
		std::function<float()> getEnd;
		std::function<float()> getT;

		// 処理補間値の設定
		std::function<void(float)> setT;
	};
	struct SpanKey {

		int objectID;
		std::string name;
		bool operator==(const SpanKey& hash) const;
	};
	struct SpanKeyHash {

		size_t operator()(const SpanKey& key) const noexcept;
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
	// 各進捗ごとのハンドル
	std::unordered_map<SpanKey, SpanHandle, SpanKeyHash> spans_;
	// 同期通知
	std::unordered_map<int, std::function<void(bool)>> synchHandlers_;

	// エディター
	int selectedObject_ = -1;
	int selectedMetric_ = -1;
	bool scrubProgressEnabled_ = false;
	float scrubOverall_ = 0.0f;

	//--------- functions ----------------------------------------------------

	// helper
	bool IsValidObject(int index) const;
	bool IsValidMetric(int index) const;

	ActionProgressMonitor() :IGameEditor("ActionProgressMonitor") {}
	~ActionProgressMonitor() = default;
	ActionProgressMonitor(const ActionProgressMonitor&) = delete;
	ActionProgressMonitor& operator=(const ActionProgressMonitor&) = delete;
};