#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/System/ParticleSystem.h>
#include <Engine/Effect/Particle/Command/ParticleCommand.h>
#include <Engine/Object/Base/Interface/IGameObject.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

//============================================================================
//	GameEffectGroup class
//============================================================================
class GameEffectGroup :
	public IGameObject {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameEffectGroup() = default;
	~GameEffectGroup() = default;

	void Init(const std::string& name, const std::string& groupName);
	void DerivedInit() override {}

	void LoadJson(const std::string& fileName);

	void Update();

	void ImGui() override;
	void DerivedImGui() override {}

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 発生の仕方
	enum class EmitMode {

		Once,      // 1回だけ発生
		Always,    // 常に発生させる
		EmitCount, // インターバルで指定回数発生
		Manual     // 外部からの入力で停止させる
	};
	// 停止条件
	enum class StopCondition {

		None,
		AfterDuration,   // 時間経過で
		OnParticleEmpty, // 対象のグループのパーティクルがすべてなくなったら
		OnParticlePhase, // 対象のグループのパーティクルが指定のフェーズを完了させたら
		ExternalStop     // 外部入力
	};
	// 処理依存相手
	struct Dependency {

		int systemIndex_ = -1; // 対象のシステムのインデックス
		int groupIndex_ = -1;  // ↑が所持しているグループのインデックス
	};
	// 発生設定
	struct EmitSetting {

		EmitMode mode = EmitMode::Once;
		int count;      // 発生回数
		float delay;    // 発生呼び出し後発生させるまでの時間
		float interval; // 連続で発生させる間隔
		float duration; // 発生から止まるまでの時間
	};
	// 停止設定
	struct StopSetting {

		StopCondition condition;
		Dependency dependency;
	};
	// 発生、更新時の設定
	struct ModuleSetting {

		// 発生モジュール
		Vector3 spawnPos;
		Vector3 spawnRotate;

		// 寿命の管理
		std::optional<ParticleLifeEndMode> lifeEndMode;
	};
	// ランタイム
	struct Runtime {

		bool pending; // 発生後、開始待ち
		bool active;  // 発生中
		int emitted;  // 発生回数
		float timer;  // 経過時間
		float emitTimer; // インターバル管理
	};

	// 発生データ
	struct GroupData {

		// システム
		ParticleSystem* system;     // パーティクル管理
		std::string systemJsonPath; // システムのファイル名

		// 発生設定
		EmitSetting emit;
		// 停止設定
		StopSetting stop;
		// モジュール設定
		ModuleSetting module;
		// ランタイム設定
		Runtime runtime;
	};

	//--------- variables ----------------------------------------------------

	// jsonの保存パス
	const std::string baseJsonPath_ = "GameEffectGroup/";

	// パーティクルを処理するグループ
	std::vector<GroupData> groups_;

	// トランスフォーム
	EffectTransform* transform_;

	// エディター
	JsonSaveState jsonSaveState_; // 保存処理用

	//--------- functions ----------------------------------------------------

	// json
	void SaveJson(const std::string& filePath);

	// エディター
	void SaveAndLoad();
};