#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <deque>
#include <mutex>
#include <optional>

//============================================================================
//	AssetAsyncQueue class
//	非同期ジョブの追加・待機・取り出しを行うスレッドセーフなFIFOキュー
//	AssetLoadWorker と組み合わせ、資産ロードなどのバックグラウンド処理を支える
//============================================================================
template<class Tx>
class AssetAsyncQueue {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	AssetAsyncQueue() = default;
	~AssetAsyncQueue() = default;

	// キュー末尾にジョブを追加し、待機中のスレッドを起床させる
	void AddQueue(Tx job);

	// stopフラグが立つかジョブ投入までブロックし、先頭ジョブを取得して削除
	// 停止時は std::nullopt を返す
	std::optional<Tx> PopBlock(std::atomic_bool& stop);

	// 任意条件に合致するジョブの存在を確認（重複投入の抑止に使用）
	template<class Ty>
	bool IsClearCondition(Ty pre);
	// ジョブが空かどうかを取得（監視や待機終了判定に使用）
	bool IsEmpty() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	mutable std::mutex jobMutex_;
	std::condition_variable jobCondition_;
	std::deque<Tx> jobs_;
};

//============================================================================
//	AssetAsyncQueue templateMethods
//============================================================================

template<class Tx>
inline void AssetAsyncQueue<Tx>::AddQueue(Tx job) {

	{
		std::scoped_lock lock(jobMutex_);
		jobs_.push_back(job);
	}
	// 1スレッド起床させる
	jobCondition_.notify_one();
}

template<class Tx>
inline std::optional<Tx> AssetAsyncQueue<Tx>::PopBlock(std::atomic_bool& stop) {

	std::unique_lock lock(jobMutex_);
	jobCondition_.wait(lock, [&] { return stop || !jobs_.empty(); });

	if (stop) {

		return std::nullopt;
	}

	// キューの先頭を取得
	Tx job = std::move(jobs_.front());
	jobs_.pop_front();
	return job;
}

template<class Tx>
template<class Ty>
inline bool AssetAsyncQueue<Tx>::IsClearCondition(Ty pre) {

	std::scoped_lock lock(jobMutex_);
	for (auto& job : jobs_) {
		if (pre(job)) {

			return true;
		}
	}
	return false;
}

template<class Tx>
inline bool AssetAsyncQueue<Tx>::IsEmpty() const {

	std::scoped_lock lock(jobMutex_);
	return jobs_.empty();
}