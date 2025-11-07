#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Async/AssetLoadWorker.h>
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/MeshRender.h>
#include <Engine/Core/Graphics/Mesh/MeshRegistry.h>
#include <Engine/Core/Graphics/GPUObject/InstancedMeshBuffer.h>
#include <Engine/Core/Graphics/Raytracing/RaytracingStructures.h>
#include <Engine/Scene/Methods/IScene.h>

// directX
#include <d3d12.h>
// c++
#include <unordered_set>
#include <xatomic.h>
// front
class DxCommand;
class Asset;
class RaytracingScene;

//============================================================================
//	InstancedMeshSystem class
//	メッシュごとのインスタンシングバッファを管理するシステム
//============================================================================
class InstancedMeshSystem :
	public ISystem {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// 初期化/終了
	InstancedMeshSystem(ID3D12Device* device,
		Asset* asset, DxCommand* dxCommand);
	~InstancedMeshSystem();

	// 非同期処理
	void StartBuildWorker();
	void StopBuildWorker();

	// メッシュのビルド要求
	void RequestBuild(const std::string& modelName,
		uint32_t maxInstStatic = Config::kMaxInstanceNum,
		uint32_t maxInstSkinned = 16);
	// シーンのビルド要求
	void BuildForSceneSynch(Scene scene);
	void RequestBuildForScene(Scene scene);

	// createBuffer
	void CreateStaticMesh(const std::string& modelName);
	void CreateSkinnedMesh(const std::string& modelName);

	Archetype Signature() const override;
	void Update(ObjectPoolManager& ObjectPoolManager) override;

	//--------- accessor -----------------------------------------------------

	const std::unordered_map<std::string, std::unique_ptr<IMesh>>& GetMeshes() const { return meshRegistry_->GetMeshes(); }
	const std::unordered_map<std::string, MeshInstancingData>& GetInstancingData() const { return instancedBuffer_->GetInstancingData(); }
	const std::unordered_map<std::string, MeshRender>& GetRenderData() const { return renderData_; }
	std::vector<RayTracingInstance> CollectRTInstances(const RaytracingScene* scene) const;

	// ビルド状況の取得
	bool IsReady(const std::string& name) const;
	bool IsBuilding() const;
	// シーンのビルド進捗取得
	float GetBuildProgressForScene(Scene scene) const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	struct MeshBuildJob {

		std::string name;     // 名前
		bool skinned;         // 骨の有無
		uint32_t maxInstance; // 最大数
	};

	//--------- variables ----------------------------------------------------

	ID3D12Device* device_;
	Asset* asset_;
	DxCommand* dxCommand_;

	std::unique_ptr<MeshRegistry> meshRegistry_;
	std::unique_ptr<InstancedMeshBuffer> instancedBuffer_;

	// モデルとオブジェクトIDの紐づけ
	std::unordered_map<std::string, std::vector<uint32_t>> objectIDsPerModel_;
	std::unordered_map<std::string, MeshRender> renderData_;

	AssetLoadWorker<MeshBuildJob> buildWorker_;
	// 重複処理回避用
	std::mutex instancedMutex_;
	std::unordered_set<std::string> requested_;

	// 進捗カウンタ
	std::atomic<uint32_t> pendingJobs_{};
	std::atomic<uint32_t> runningJobs_{};
};