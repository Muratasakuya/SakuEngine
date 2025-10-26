#include "EffectEmitController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	EffectEmitController classMethods
//============================================================================

void EffectEmitController::Start(EffectNodeRuntime* runtime) {

	// ランタイムが無効なら何もしない
	if (!runtime) {
		return;
	}
	// アクティブ状態にして発生
	runtime->pending = true;
	runtime->active = true;
	runtime->timer = 0.0f;
	runtime->emitTimer = 0.0f;
	runtime->emitted = 0;
}

void EffectEmitController::Stop(EffectNodeRuntime* runtime) {

	// ランタイムが無効なら何もしない
	if (!runtime) {
		return;
	}
	// アクティブ状態を止めて動かないようにする
	runtime->active = false;
	runtime->pending = false;
}

void EffectEmitController::Reset(const EffectEmitSetting& emit, EffectNodeRuntime* runtime) {

	// ランタイムが無効なら何もしない
	if (!runtime) {
		return;
	}
	// 全ての値をリセット
	runtime->pending = true;
	runtime->active = emit.mode != EffectEmitMode::Manual;
	runtime->didFirstEmit = false;
	runtime->stopped = false;
	runtime->emitted = 0;
	runtime->timer = 0.0f;
	runtime->emitTimer = 0.0f;
}

bool EffectEmitController::Tick(const EffectEmitSetting& emit, EffectNodeRuntime* runtime) {

	// ランタイムが無効なら何もしない
	if (!runtime || !runtime->active) {
		return false;
	}

	// デルタタイム
	const float deltaTime = GameTimer::GetScaledDeltaTime();

	// 時間を進める
	runtime->timer += deltaTime;

	// 発生方法別に発生判定を行う
	switch (emit.mode) {
	case EffectEmitMode::Always:

		// 遅延時間以上経過、まだ発生していなければ発生させる
		if (runtime->pending && emit.delay <= runtime->timer) {

			runtime->pending = false;
		}
		return false;
	case EffectEmitMode::Once:

		// 遅延時間以上経過、まだ発生していなければ発生させる
		if (runtime->pending && emit.delay <= runtime->timer) {

			// 発生回数を1回にして発生済みにする
			runtime->emitted = 1;
			runtime->pending = false;
			runtime->didFirstEmit = true;
			runtime->emitTimer = 0.0f;
			return true;
		}
		runtime->emitTimer = 0.0f;
		return false;
	case EffectEmitMode::EmitCount:

		// 遅延待ち中かどうか
		if (runtime->pending) {
			// 遅延時間以上経過したら発生させる
			if (emit.delay <= runtime->timer) {

				// 発生回数をインクリメントして1回発生済みにする
				++runtime->emitted;
				runtime->emitTimer = 0.0f;
				runtime->pending = false;
				runtime->didFirstEmit = true;
				return true;
			}
			return false;
		} else {

			// 発生回数が上限に達していたら発生させない
			if (emit.count <= runtime->emitted) {
				runtime->emitTimer = 0.0f;
				return false;
			}

			// 発生間隔の時間を進める
			runtime->emitTimer += deltaTime;
			// 発生間隔以上経過していたら発生させる
			if (emit.interval <= runtime->emitTimer) {

				// 発生回数をインクリメントしてリセット
				++runtime->emitted;
				runtime->emitTimer = 0.0f;
				return true;
			}
			return false;
		}
	case EffectEmitMode::Manual:

		// 遅延待ち中かどうか
		if (runtime->pending) {
			if (emit.delay <= runtime->timer) {

				// 発生回数をインクリメントして1回発生済みにする
				++runtime->emitted;
				runtime->emitTimer = 0.0f;
				runtime->pending = false;
				runtime->didFirstEmit = true;
				return true;
			}
			return false;
		} else {

			// 発生間隔の時間を進める
			runtime->emitTimer += deltaTime;
			if (emit.interval <= runtime->emitTimer) { 
				
				// 発生回数をインクリメントしてリセット
				++runtime->emitted;
				runtime->emitTimer = 0.0f;
				return true;
			}
			return false;
		}
	}
	return false;
}