#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/Interface/IGameObject.h>
#include <Engine/Effect/User/Methods/EffectNode.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

//============================================================================
//	EffectGroup class
//	複数のパーティクルシステムをゲーム上でまとめて扱う
//============================================================================
class EffectGroup :
	public IGameObject {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectGroup() = default;
	~EffectGroup() = default;

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

	//--------- runtime ------------------------------------------------------

	// グループの基準ワールド座標を設定し、Manual以外のノードの処理を開始する
	void Emit(const Vector3& worldPos);
	// ExternalStop指定ノードのみを停止状態にする
	void Stop();

	// Manual指定のノードを外部から開始する
	void StartNode(const std::string& nodeKey);
	// Manual指定のノードを外部から停止する
	void StopNode(const std::string& nodeKey);

	// 親の設定
	void SetParent(uint32_t anchorId) { parentAnchorId_ = anchorId; }
	// 親への追従を解除
	void ClearParent();

	// --------- accessor ----------------------------------------------------

	// ワールド座標を設定
	void SetWorldPos(const Vector3& worldPos) { effectWorldPos_ = worldPos; }

	// 特定のノードへのコマンド設定
	// 寿命管理設定
	void SetLifeEndMode(const std::string& nodeKey, ParticleLifeEndMode mode, bool isAllNode = true);
	// キーフレームパスの設定
	void SetKeyframePath(const std::string& nodeKey, const std::vector<Vector3>& keys);

	// すべてのノードが処理を終えているか
	bool IsFinishedAllNode() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// jsonの保存パス
	const std::string baseJsonPath_ = "GameEffectGroup/";

	// 親
	uint32_t parentAnchorId_ = 0u;
	std::string parentAnchorName_;

	// エフェクト基準位置
	Vector3 effectWorldPos_;

	// ノード
	std::vector<EffectNode> nodes_;

	// エディター
	int selectNodeIndex_ = -1;    // 選択中のノード
	JsonSaveState jsonSaveState_; // 保存処理用
	Vector3 editorEmitPos_;       // 発生位置テスト用
	// レイアウト
	float leftChildWidth_;
	int displaySystemCount_;

	//--------- functions ----------------------------------------------------

	// 親がいれば親のワールド座標、なければグループ基準座標を返す
	Vector3 ResolveAnchorPos() const;
	// 指定ノードの指定CPUグループの生存パーティクル数を取得する
	uint32_t QueryAlive(const std::string& nodeKey, int groupIndex) const;

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
	void EditNode();

	// ランタイム、テスト
	void EditorTestTab();
	void DisplayRuntimeNode(EffectNode& node);

	// レイアウト
	void EditLayout();
	void ApplyLayout();
	void SaveLayout();

	// ノード配列の名前を取得
	std::vector<std::string> GetNodeNames() const;
};