#include "ModelLoader.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Asset/TextureManager.h>
#include <Engine/Asset/Filesystem.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	ModelLoader classMethods
//============================================================================

void ModelLoader::Init(TextureManager* textureManager) {

	textureManager_ = nullptr;
	textureManager_ = textureManager;

	baseDirectoryPath_ = "./Assets/Models/";
	isCacheValid_ = false;

	// ワーカースレッド起動
	loadWorker_.Start([this](std::string&& name) {
		this->LoadAsync(std::move(name)); });
}

void ModelLoader::LoadSynch(const std::string& modelName) {

	// すでにロード済みなら何もしない
	{
		std::scoped_lock lk(modelMutex_);
		if (models_.contains(modelName)) {
			return;
		}
	}

	// 読み込み開始
	SpdLogger::Log("[Model][Begin] " + modelName);

	std::filesystem::path path;
	// 見つからなければ処理しない
	if (!Filesystem::FindByStem(baseDirectoryPath_, modelName, { ".obj", ".gltf" }, path)) {
		SpdLogger::Log("[Model][Missing] " + modelName);
		return;
	}

	// モデル読み込み処理
	ModelData modelData = LoadModelFile(path.string());
	SpdLogger::Log("[Model][Loaded] " + modelName);

	// 読み込みデータを設定
	{
		std::scoped_lock lk(modelMutex_);
		models_[modelName] = std::move(modelData);
		isCacheValid_ = false;
	}
}

void ModelLoader::Load(const std::string& modelName) {

	RequestLoadAsync(modelName);
	for (;;) {
		{
			std::scoped_lock lk(modelMutex_);
			if (models_.contains(modelName)) break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void ModelLoader::RequestLoadAsync(const std::string& modelName) {

	// 既にロード済みなら何もしない
	{
		std::scoped_lock lk(modelMutex_);
		if (models_.contains(modelName)) {
			return;
		}
	}
	// 重複チェック後にキューを追加
	auto& queue = loadWorker_.RefAsyncQueue();
	if (queue.IsClearCondition([&](const std::string& j) { return j == modelName; })) {
		return;
	}
	queue.AddQueue(modelName);
	SpdLogger::Log("[Model][Enqueue] " + modelName);
}

void ModelLoader::WaitAll() {

	for (;;) {
		if (loadWorker_.GetAsyncQueue().IsEmpty()) {

			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void ModelLoader::LoadAsync(std::string modelName) {

	// 重複読み込みを行わないようにチェック
	{
		std::scoped_lock lock(modelMutex_);
		if (models_.contains(modelName)) {
			return;
		}
	}

	// 読み込み開始
	SpdLogger::Log("[Model][Begin] " + modelName);

	std::filesystem::path path;
	// 見つからなければ処理しない
	if (!Filesystem::FindByStem(baseDirectoryPath_, modelName, { ".obj", ".gltf" }, path)) {
		SpdLogger::Log("[Model][Missing] " + modelName);
		return;
	}

	// モデル読み込み処理
	ModelData modelData = LoadModelFile(path.string());
	SpdLogger::Log("[Model][Loaded] " + modelName);

	// 読み込みデータを設定
	{
		std::scoped_lock lk(modelMutex_);
		models_[modelName] = std::move(modelData);
		isCacheValid_ = false;
	}
}

ModelData ModelLoader::LoadModelFile(const std::string& filePath) {

	ModelData modelData;            // 構築するModelData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals;   // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line;               // ファイルから読んだ1行を格納するもの

	modelData.fullPath = filePath; // フルパスを格納

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(),
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_SortByPType);

	// メッシュがないのには対応しない
	assert(scene->HasMeshes());

	// メッシュ解析
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {

		aiMesh* mesh = scene->mMeshes[meshIndex];

		// 法線がないMeshは今回は非対応
		assert(mesh->HasNormals());

		MeshModelData meshModelData;
		// 最初に頂点数分のメモリを確保しておく
		meshModelData.vertices.resize(mesh->mNumVertices);

		// vertex解析
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {

			aiVector3D pos = mesh->mVertices[vertexIndex];
			aiVector3D normal = mesh->mNormals[vertexIndex];
			aiVector3D texcoord{};

			// texcoordがある場合のみ設定
			if (mesh->HasTextureCoords(0)) {

				texcoord = mesh->mTextureCoords[0][vertexIndex];
			}

			// 座標系の変換
			meshModelData.vertices[vertexIndex].pos = { -pos.x,pos.y,pos.z,1.0f };
			meshModelData.vertices[vertexIndex].normal = { -normal.x,normal.y,normal.z };
			meshModelData.vertices[vertexIndex].texcoord = { texcoord.x,texcoord.y };
		}

		// index解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {

			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {

				uint32_t vertexIndex = face.mIndices[element];

				meshModelData.indices.push_back(vertexIndex);
			}
		}

		// bone解析
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {

			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);

			Matrix4x4 bindPoseMatrix =
				Matrix4x4::MakeAxisAffineMatrix({ scale.x,scale.y,scale.z }, { rotate.x,-rotate.y,-rotate.z,rotate.w }, { -translate.x,translate.y,translate.z });
			jointWeightData.inverseBindPoseMatrix = Matrix4x4::Inverse(bindPoseMatrix);

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {

				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, meshIndex, bone->mWeights[weightIndex].mVertexId });
			}
		}

		// material解析
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// _DIFFUSE
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {

			aiString textureName;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);
			meshModelData.textureName = textureName.C_Str();

			std::filesystem::path name(meshModelData.textureName.value());
			std::string identifier = name.stem().string();
			meshModelData.textureName = identifier;

			textureManager_->RequestLoadAsync(meshModelData.textureName.value());
		}
		// NORMALS
		if (material->GetTextureCount(aiTextureType_NORMALS) > 0 ||
			material->GetTextureCount(aiTextureType_HEIGHT) > 0) {

			aiString normalMapName;

			if (material->GetTexture(aiTextureType_NORMALS, 0, &normalMapName) != AI_SUCCESS) {

				material->GetTexture(aiTextureType_HEIGHT, 0, &normalMapName);
			}

			meshModelData.normalMapTexture = normalMapName.C_Str();

			std::filesystem::path normalNamePath(meshModelData.normalMapTexture.value());
			std::string normalIdentifier = normalNamePath.stem().string();
			meshModelData.normalMapTexture = normalIdentifier;

			textureManager_->RequestLoadAsync(meshModelData.normalMapTexture.value());
		}
		// BaseColor
		aiColor4D baseColor;
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor)) {

			meshModelData.baseColor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };
		}

		modelData.meshes.push_back(meshModelData);
	}

	// 階層構造の作成
	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

Node ModelLoader::ReadNode(aiNode* node) {

	Node result;

	// nodeのlocalMatrixを取得
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;

	// 列ベクトル形式を行ベクトル形式に転置
	aiLocalMatrix.Transpose();

	aiVector3D scale, translate;
	aiQuaternion rotate;

	// assimpの行列からSRTを抽出する関数
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x,scale.y ,scale.z };
	// X軸を反転、さらに回転方向が逆なので軸を反転させる
	result.transform.rotation = { rotate.x,-rotate.y ,-rotate.z,rotate.w };
	// X軸を反転
	result.transform.translation = { -translate.x,translate.y ,translate.z };
	result.localMatrix =
		Matrix4x4::MakeAxisAffineMatrix(result.transform.scale, result.transform.rotation, result.transform.translation);

	// Node名を格納
	result.name = node->mName.C_Str();
	// 子どもの数だけ確保
	result.children.resize(node->mNumChildren);

	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {

		// 再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}

bool ModelLoader::Search(const std::string& modelName) {

	std::scoped_lock lock(modelMutex_);
	return models_.find(modelName) != models_.end();
}

const ModelData& ModelLoader::GetModelData(const std::string& modelName) const {

	std::scoped_lock lock(modelMutex_);
	bool find = models_.find(modelName) != models_.end();
	if (!find) {

		LOG_WARN("not found model", modelName);
		ASSERT(find, "not found model" + modelName);
	}
	models_.at(modelName).isUse = true;
	return models_.at(modelName);
}

const std::vector<std::string>& ModelLoader::GetModelKeys() const {

	std::scoped_lock lock(modelMutex_);
	if (!isCacheValid_) {

		modelKeysCache_.clear();
		modelKeysCache_.reserve(models_.size());
		for (auto& [key, _] : models_) {

			modelKeysCache_.push_back(key);
		}
		// キャッシュを有効にする
		isCacheValid_ = true;
	}
	return modelKeysCache_;
}

void ModelLoader::ReportUsage(bool listAll) const {

	// ロード済みだが未使用の場合のログ出力
	std::vector<std::string> unused;
	unused.reserve(models_.size());
	for (const auto& [name, model] : models_) {
		if (!model.isUse) {

			unused.emplace_back(name);
		}
	}

	if (unused.empty()) {

		LOG_ASSET_INFO("[Model] Unused: 0");
	} else {

		LOG_ASSET_INFO("[Model] Unused: {}", unused.size());
		if (listAll) {
			for (auto& n : unused) {

				LOG_ASSET_INFO("  - {}", n);
			}
		}
	}

	// フォルダ内にあるにも関わらず未使用
	std::unordered_set<std::string> onDisk;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(baseDirectoryPath_)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		const auto ext = entry.path().extension().string();
		if (ext == ".obj" || ext == ".gltf") {

			onDisk.insert(entry.path().stem().string());
		}
	}

	std::unordered_set<std::string> loaded;
	loaded.reserve(models_.size());
	for (const auto& [name, _] : models_) {

		loaded.insert(name);
	}

	std::vector<std::string> notLoaded;
	notLoaded.reserve(onDisk.size());
	for (auto& stem : onDisk) {
		if (!loaded.contains(stem)) {

			notLoaded.emplace_back(stem);
		}
	}

	if (notLoaded.empty()) {

		LOG_ASSET_INFO("[Model] NotLoaded(on disk only): 0");
	} else {

		LOG_ASSET_INFO("[Model] NotLoaded(on disk only): {}", notLoaded.size());
		if (listAll) {
			for (auto& n : notLoaded) {

				LOG_ASSET_INFO("  - {}", n);
			}
		}
	}
}