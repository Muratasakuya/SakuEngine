#include "InstancedMeshBuffer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Utility/Helper/Algorithm.h>

// meshoptimizer
#include <meshoptimizer.h>

//============================================================================
//	InstancedMeshBuffer classMethods
//============================================================================

void InstancedMeshBuffer::Init(ID3D12Device* device, Asset* asset) {

	device_ = nullptr;
	device_ = device;

	asset_ = nullptr;
	asset_ = asset;
}

void InstancedMeshBuffer::CreateBuffers(const std::string& name) {

	auto& meshGroup = meshGroups_[name];

	// transform
	// buffer作成
	meshGroup.matrixBuffer.CreateSRVBuffer(device_, meshGroup.maxInstance);

	// material、lighting
	// meshの数だけ作成する
	const size_t meshNum = meshGroup.meshNum;
	meshGroup.materialsBuffer.resize(meshNum);
	meshGroup.lightingBuffer.resize(meshNum);
	meshGroup.materialUploadData.resize(meshNum);
	meshGroup.lightingUploadData.resize(meshNum);

	for (uint32_t meshIndex = 0; meshIndex < meshNum; ++meshIndex) {

		// buffer作成
		meshGroup.materialsBuffer[meshIndex].CreateSRVBuffer(device_, meshGroup.maxInstance);
		meshGroup.lightingBuffer[meshIndex].CreateSRVBuffer(device_, meshGroup.maxInstance);
	}

	// skinnedMeshなら専用bufferを作成する
	if (!meshGroup.isSkinned) {
		return;
	}

	// well、influence
	CreateSkinnedMeshBuffers(name);
}

void InstancedMeshBuffer::CreateSkinnedMeshBuffers(const std::string& name) {

	auto& meshGroup = meshGroups_[name];

	// meshの数だけ作成する
	const size_t meshNum = meshGroup.meshNum;
	meshGroup.wells.resize(meshNum);
	meshGroup.influences.resize(meshNum);
	meshGroup.skinningInformations.resize(meshNum);
	// dataの数もここで決める
	meshGroup.wellUploadData.resize(meshNum);

	//　bone、骨の数
	const Skeleton skeleton = asset_->GetSkeletonData(name);
	const uint32_t boneSize = static_cast<uint32_t>(skeleton.joints.size());
	meshGroup.boneSize = boneSize;

	for (uint32_t meshIndex = 0; meshIndex < meshNum; ++meshIndex) {

		// 頂点数
		const uint32_t vertexSize = meshGroup.skinnedMesh->GetVertexCount(meshIndex);
		meshGroup.vertexSizes.emplace_back(vertexSize);

		// information
		meshGroup.skinningInformations[meshIndex].CreateBuffer(device_);

		// well
		// buffer作成
		meshGroup.wells[meshIndex].CreateSRVBuffer(device_, meshGroup.maxInstance * boneSize);

		// influence
		// buffer作成
		meshGroup.influences[meshIndex].CreateSRVBuffer(device_, vertexSize);
		// 0埋めしてweightを0にしておく
		std::vector<VertexInfluence> influence(vertexSize, {});

		// ModelDataを解析してInfluenceを埋める
		for (const auto& jointWeight : asset_->GetModelData(name).skinClusterData) {

			// jointWeight.firstはjoint名なので、skeletonに対象となるjointが含まれているか判断
			auto it = skeleton.jointMap.find(jointWeight.first);
			// 存在しないjoint名だったら次に進める
			if (it == skeleton.jointMap.end()) {
				continue;
			}

			// ジョイントのインデックス
			const uint32_t jointIndex = it->second;
			for (const auto& vertexWeight : jointWeight.second.vertexWeights) {

				// 対応しているmeshのみ処理する
				if (vertexWeight.meshIndex != meshIndex) {
					continue;
				}

				// 範囲外アクセス防止
				if (vertexWeight.vertexIndex >= influence.size()) {
					continue;
				}

				// 該当のvertexIndexのinfluence情報を参照しておく
				auto& currentInfluence = influence[vertexWeight.vertexIndex];
				for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
					// 0.0fが空いている状態
					if (currentInfluence.weights[index] == 0.0f) {

						currentInfluence.weights[index] = vertexWeight.weight;
						currentInfluence.jointIndices[index] = jointIndex;
						break;
					}
				}
			}
		}

		// これ以上更新する予定がないので転送
		meshGroup.influences[meshIndex].TransferData(influence);
		meshGroups_[name].skinningInformations[meshIndex].TransferData({
			.numVertices = meshGroups_[name].vertexSizes[meshIndex],
			.numBones = meshGroups_[name].boneSize });
	}
}

void InstancedMeshBuffer::Create(IMesh* mesh,
	const std::string& name, uint32_t numInstance) {

	// すでにある場合は作成しない
	if (Algorithm::Find(meshGroups_, name)) {
		return;
	}

	// 最大のinstance数設定
	meshGroups_[name].maxInstance = numInstance;
	// 描画を行うrenderingDataの設定
	meshGroups_[name].meshNum = mesh->GetMeshCount();
	// staticかskinnedか判定
	meshGroups_[name].isSkinned = mesh->IsSkinned();
	if (meshGroups_[name].isSkinned) {

		meshGroups_[name].skinnedMesh = mesh;
	}

	// bufferの作成
	CreateBuffers(name);
}

void InstancedMeshBuffer::SetUploadData(const std::string& name,
	const TransformationMatrix& matrix, const std::vector<Material>& materials,
	const SkinnedAnimation& animation) {

	meshGroups_[name].matrixUploadData.emplace_back(matrix);

	for (uint32_t meshIndex = 0; meshIndex < meshGroups_[name].materialUploadData.size(); ++meshIndex) {

		// material
		meshGroups_[name].materialUploadData[meshIndex].emplace_back(MaterialForGPU(
			materials[meshIndex].color,
			materials[meshIndex].textureIndex,
			materials[meshIndex].normalMapTextureIndex,
			materials[meshIndex].enableNormalMap,
			materials[meshIndex].enableDithering,
			materials[meshIndex].emissiveIntensity,
			materials[meshIndex].emissionColor,
			materials[meshIndex].uvMatrix));
		// lighting
		meshGroups_[name].lightingUploadData[meshIndex].emplace_back(LightingForGPU(
			materials[meshIndex].enableLighting,
			materials[meshIndex].enableHalfLambert,
			materials[meshIndex].enableBlinnPhongReflection,
			materials[meshIndex].enableImageBasedLighting,
			materials[meshIndex].castShadow,
			materials[meshIndex].phongRefShininess,
			materials[meshIndex].specularColor,
			materials[meshIndex].shadowRate,
			materials[meshIndex].environmentCoefficient));

		// skinnedMeshなら設定する
		if (meshGroups_[name].isSkinned) {

			const auto& wellData = animation.GetWellForGPU();
			// 連結させて追加
			meshGroups_[name].wellUploadData[meshIndex].insert(
				meshGroups_[name].wellUploadData[meshIndex].end(),
				wellData.begin(),
				wellData.end());
		}
	}

	// instance数インクリメント
	++meshGroups_[name].numInstance;

	// 最大instance数を超えたらエラー
	if (meshGroups_[name].numInstance > meshGroups_[name].maxInstance) {

		ASSERT(FALSE, "numInstance > maxInstance");
	}
}

void InstancedMeshBuffer::Update(DxCommand* dxCommand) {

	// 何もなければ処理をしない
	if (meshGroups_.empty()) {
		return;
	}

	for (auto& [name, meshGroup] : meshGroups_) {
		// instance数が0なら処理をしない
		if (meshGroup.numInstance == 0) {
			continue;
		}

		// buffer転送
		meshGroup.matrixBuffer.TransferData(meshGroup.matrixUploadData);
		for (uint32_t meshIndex = 0; meshIndex < meshGroup.meshNum; ++meshIndex) {

			meshGroup.materialsBuffer[meshIndex].TransferData(meshGroup.materialUploadData[meshIndex]);
			meshGroup.lightingBuffer[meshIndex].TransferData(meshGroup.lightingUploadData[meshIndex]);

			// skinnedMeshなら設定する
			if (meshGroups_[name].isSkinned) {

				meshGroups_[name].wells[meshIndex].TransferData(meshGroups_[name].wellUploadData[meshIndex]);

				ID3D12GraphicsCommandList* commandList = dxCommand->GetCommandList();
				SkinnedMesh* skinnedMesh = static_cast<SkinnedMesh*>(meshGroups_[name].skinnedMesh);

				// dispach処理
				commandList->SetComputeRootShaderResourceView(0,
					meshGroups_[name].wells[meshIndex].GetResource()->GetGPUVirtualAddress());
				commandList->SetComputeRootShaderResourceView(1,
					skinnedMesh->GetInputVertexBuffer(meshIndex).GetResource()->GetGPUVirtualAddress());
				commandList->SetComputeRootShaderResourceView(2,
					meshGroups_[name].influences[meshIndex].GetResource()->GetGPUVirtualAddress());
				commandList->SetComputeRootUnorderedAccessView(3,
					skinnedMesh->GetOutputVertexBuffer(meshIndex).GetResource()->GetGPUVirtualAddress());
				commandList->SetComputeRootConstantBufferView(4,
					meshGroups_[name].skinningInformations[meshIndex].GetResource()->GetGPUVirtualAddress());
				commandList->Dispatch(
					DxUtils::RoundUp(meshGroups_[name].vertexSizes[meshIndex], 1024),
					meshGroups_[name].numInstance, 1);
			}
		}
	}
}

void InstancedMeshBuffer::Reset() {

	// 何もなければ処理をしない
	if (meshGroups_.empty()) {
		return;
	}

	for (auto& meshGroup : std::views::values(meshGroups_)) {

		// dataクリア
		meshGroup.matrixUploadData.clear();

		for (uint32_t meshIndex = 0; meshIndex < meshGroup.meshNum; ++meshIndex) {
			if (meshGroup.materialUploadData[meshIndex].size() != 0) {

				meshGroup.materialUploadData[meshIndex].clear();
			}
			if (meshGroup.lightingUploadData[meshIndex].size() != 0) {

				meshGroup.lightingUploadData[meshIndex].clear();
			}

			// skinnedMeshの場合のみ
			if (meshGroup.isSkinned) {

				meshGroup.wellUploadData[meshIndex].clear();
			}
		}
		meshGroup.numInstance = 0;
	}
}