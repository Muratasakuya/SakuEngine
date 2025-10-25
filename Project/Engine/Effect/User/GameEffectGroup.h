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
//	複数のパーティクルシステムをゲーム上でまとめて扱う
//============================================================================
class GameEffectGroup :
	public IGameObject {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameEffectGroup() = default;
	~GameEffectGroup() = default;

	// オブジェクト作成
	void Init(const std::string& name, const std::string& groupName);
	void DerivedInit() override {}
	// .jsonファイルからエフェクトデータを読み込み、作成する
	void LoadJson(const std::string& fileName);

	// 更新処理
	void Update();

	// エディターの表示
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
		ExternalStop     // 外部入力
	};
	// 処理依存相手
	struct Dependency {

		int systemIndex = -1; // 対象のグループ内のシステムインデックス
		int groupIndex = -1;  // ↑が所持しているグループのインデックス
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
		ParticleLifeEndMode lifeEndMode;
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
		ParticleSystem* system; // パーティクル管理
		std::string name;       // 名前

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

	// トランスフォーム、すべてのグループエフェクトの原点
	EffectTransform* transform_;

	// エディター
	int selectGroupIndex_ = -1;   // 選択中のグループ
	JsonSaveState jsonSaveState_; // 保存処理用
	// レイアウト
	float leftChildWidth_;
	int displaySystemCount_;

	//--------- functions ----------------------------------------------------

	// json
	void SaveJson(const std::string& filePath);

	// エディター
	void DisplayInformation();
	void SaveAndLoad();

	// 左側の枠の処理
	void EditLeftChild();
	void AddParticleSystem();
	void SelectParticleSystem();

	// 右側の枠の処理
	void EditRightChild();
	void EditGroup();

	// レイアウト
	void EditLayout();
	void ApplyLayout();
	void SaveLayout();

	// グループ配列の名前を取得
	std::vector<std::string> GetGroupNames() const;
};