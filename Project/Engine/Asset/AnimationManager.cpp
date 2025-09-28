#include "AnimationManager.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Asset/ModelLoader.h>
#include <Engine/Asset/Filesystem.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	AnimationManager classMethods
//============================================================================

void AnimationManager::Init(ID3D12Device* device,
	SRVDescriptor* srvDescriptor, ModelLoader* modelLoader) {

	device_ = nullptr;
	device_ = device;

	srvDescriptor_ = nullptr;
	srvDescriptor_ = srvDescriptor;

	modelLoader_ = nullptr;
	modelLoader_ = modelLoader;

	baseDirectoryPath_ = "./Assets/Models/";

	// ワーカースレッド起動
	loadWorker_.Start([this](AnimationAsyncKey&& key) {
		this->LoadAsync(std::move(key)); });
}

void AnimationManager::Load(const std::string& animationName, const std::string& modelName) {

	RequestLoadAsync(animationName, modelName);
	// モデルとアニメが来るまで待つ
	for (;;) {
		{
			std::scoped_lock lk(animMutex_);
			if (animations_.find(modelName) != animations_.end()) break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void AnimationManager::RequestLoadAsync(const std::string& animationName, const std::string& modelName) {

	auto& queue = loadWorker_.RefAsyncQueue();

	// 処理中のキューにあるなら処理させない
	if (queue.IsClearCondition([&](const AnimationAsyncKey& j) {
		return j.animName == animationName && j.modelName == modelName;
		})) {
		return;
	}
	// 重複チェック後にキューを追加
	queue.AddQueue(AnimationAsyncKey{ animationName, modelName });
	SpdLogger::Log("[Animation][Enqueue] anim:" + animationName + " model:" + modelName);
}

void AnimationManager::WaitAll() {

	for (;;) {
		if (loadWorker_.GetAsyncQueue().IsEmpty()) {

			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void AnimationManager::LoadAsync(AnimationAsyncKey key) {

	// 必要なモデルがまだ読み込みされていなければ処理しない
	if (!modelLoader_->Search(key.modelName)) {

		SpdLogger::Log("[Animation][WaitModel] anim:" + key.animName + " model:" + key.modelName);
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		loadWorker_.RefAsyncQueue().AddQueue(std::move(key));
		return;
	}

	std::filesystem::path filePath;
	// 見つからなければ処理しない
	if (!Filesystem::FindByStem(baseDirectoryPath_, key.animName, { ".gltf" }, filePath)) {

		SpdLogger::Log("[Animation][Missing] anim:" + key.animName);
		return;
	}

	// アニメーションが存在していない場合はエラーにする
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.string(), 0);
	if (!scene || scene->mNumAnimations == 0) {

		SpdLogger::Log("[Animation][NoClips] anim:" + key.animName);
		ASSERT(FALSE, "[Animation][NoClips] anim:" + key.animName);
		return;
	}

	// アニメーション解析処理
	std::unordered_map<std::string, AnimationData> localAnimations{};
	for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {

		aiAnimation* animAssimp = scene->mAnimations[i];
		AnimationData anim;

		// アニメーションの名前設定
		const std::string newName = (scene->mNumAnimations == 1) ?
			key.modelName : (key.modelName + "_" + animAssimp->mName.C_Str());

		anim.duration = static_cast<float>(animAssimp->mDuration / animAssimp->mTicksPerSecond);
		for (uint32_t c = 0; c < animAssimp->mNumChannels; ++c) {

			aiNodeAnim* nodeAnim = animAssimp->mChannels[c];
			NodeAnimation& node = anim.nodeAnimations[nodeAnim->mNodeName.C_Str()];
			// T
			for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; ++k) {

				aiVectorKey& kv = nodeAnim->mPositionKeys[k];
				KeyframeVector3 f; f.time = float(kv.mTime / animAssimp->mTicksPerSecond);
				f.value = { -kv.mValue.x, kv.mValue.y, kv.mValue.z };
				node.translate.keyframes.push_back(f);
			}
			// R
			for (uint32_t k = 0; k < nodeAnim->mNumRotationKeys; ++k) {

				aiQuatKey& kv = nodeAnim->mRotationKeys[k];
				KeyframeQuaternion f; f.time = float(kv.mTime / animAssimp->mTicksPerSecond);
				f.value = { kv.mValue.x, -kv.mValue.y, -kv.mValue.z, kv.mValue.w };
				node.rotate.keyframes.push_back(f);
			}
			// S
			for (uint32_t k = 0; k < nodeAnim->mNumScalingKeys; ++k) {

				aiVectorKey& kv = nodeAnim->mScalingKeys[k];
				KeyframeVector3 f; f.time = float(kv.mTime / animAssimp->mTicksPerSecond);
				f.value = { kv.mValue.x, kv.mValue.y, kv.mValue.z };
				node.scale.keyframes.push_back(f);
			}
		}
		localAnimations.emplace(newName, std::move(anim));
	}

	// 読み込み完了
	SpdLogger::Log("[Animation][Loaded] anim=" + key.animName);
	{
		std::scoped_lock lk(animMutex_);
		for (auto& [name, animation] : localAnimations) {

			animations_[name] = std::move(animation);
		}

		// クラスター、骨データ作成
		if (!skeletons_.contains(key.modelName)) {

			skeletons_[key.modelName] = CreateSkeleton(modelLoader_->GetModelData(key.modelName).rootNode);
		}
		if (!skinClusters_.contains(key.modelName)) {

			skinClusters_[key.modelName] = CreateSkinCluster(key.modelName, key.modelName);
		}
	}
	SpdLogger::Log("[Animation][Registered] model:" + key.modelName + "animations:" + std::to_string(localAnimations.size()));
}

Skeleton AnimationManager::CreateSkeleton(const Node& rootNode) {

	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// 名前とIndexのマッピングを行う
	for (const auto& joint : skeleton.joints) {

		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
}

int32_t AnimationManager::CreateJoint(const Node& node, const std::optional<int32_t> parent, std::vector<Joint>& joints) {

	Joint joint;
	joint.name = node.name;
	joint.isParentTransform = false;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = Matrix4x4::MakeIdentity4x4();
	joint.transform = node.transform;
	// 現在登録されている数をIndexにする
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	// SkeletonのJoint列に追加
	joints.push_back(joint);

	for (const auto& child : node.children) {

		// 子Jointを作成し、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}

	return joint.index;
}

SkinCluster AnimationManager::CreateSkinCluster(const std::string& modelName, const std::string& animationName) {

	SkinCluster skinCluster;

	// size確保
	skinCluster.mappedPalette.resize(skeletons_[animationName].joints.size());

	// inverseBindPoseMatrixを格納する場所を作成して単位行列で埋める
	skinCluster.inverseBindPoseMatrices.resize(skeletons_[animationName].joints.size());
	std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(),
		[]() { return Matrix4x4::MakeIdentity4x4(); });

	// ModelDataを解析してInfluenceを埋める
	for (const auto& jointWeight : modelLoader_->GetModelData(modelName).skinClusterData) {

		// jointWeight.firstはjoint名なので、skeletonに対象となるjointが含まれているか判断
		auto it = skeletons_[animationName].jointMap.find(jointWeight.first);
		// 存在しないjoint名だったら次に進める
		if (it == skeletons_[animationName].jointMap.end()) {
			continue;
		}

		// (*it).secondにはjointのIndexが入っているので、該当のIndexのInverseBindPoseMatrixを代入
		skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
	}

	return skinCluster;
}

const AnimationData& AnimationManager::GetAnimationData(const std::string& animationName) const {

	std::scoped_lock lk(animMutex_);
	bool find = animations_.find(animationName) != animations_.end();
	if (!find) {

		LOG_WARN("not found animation", animationName);
		ASSERT(find, "not found animation" + animationName);
	}
	return animations_.at(animationName);
}

const Skeleton& AnimationManager::GetSkeletonData(const std::string& animationName) const {

	std::scoped_lock lk(animMutex_);
	bool find = skeletons_.find(animationName) != skeletons_.end();
	if (!find) {

		LOG_WARN("not found animation", animationName);
		ASSERT(find, "not found animation" + animationName);
	}
	return skeletons_.at(animationName);
}

const SkinCluster& AnimationManager::GetSkinClusterData(const std::string& animationName) const {

	std::scoped_lock lk(animMutex_);
	bool find = skinClusters_.find(animationName) != skinClusters_.end();
	if (!find) {

		LOG_WARN("not found animation", animationName);
		ASSERT(find, "not found animation" + animationName);
	}
	return skinClusters_.at(animationName);
}