#include "CPUParticleGroup.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/ParticleConfig.h>
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	CPUParticleGroup classMethods
//============================================================================

void CPUParticleGroup::Create(ID3D12Device* device,
	Asset* asset, ParticlePrimitiveType primitiveType) {

	asset_ = nullptr;
	asset_ = asset;

	// 初期化値
	// 全ての形状を初期化しておく
	emitter_.Init();

	blendMode_ = BlendMode::kBlendModeAdd;

	// buffer作成
	BaseParticleGroup::CreatePrimitiveBuffer(device, primitiveType, kMaxCPUParticles);
	// structuredBuffer(SRV)
	transformBuffer_.CreateSRVBuffer(device, kMaxCPUParticles);
	materialBuffer_.CreateSRVBuffer(device, kMaxCPUParticles);
	textureInfoBuffer_.CreateSRVBuffer(device, kMaxCPUParticles);
}

void CPUParticleGroup::CreateFromJson(ID3D12Device* device, Asset* asset, const Json& data, bool useGame) {

	asset_ = nullptr;
	asset_ = asset;

	useGame_ = useGame;

	// 初期化値
	// 全ての形状を初期化しておく
	emitter_.Init();

	// jsonからデータ取得
	FromJson(data, asset_);

	// 作成するバッファの数
	bool isUseGame = useGame_ && gameMaxParticleCount_ != 0;
	uint32_t maxParticle = isUseGame ? gameMaxParticleCount_ : kMaxCPUParticles;

	// buffer作成
	BaseParticleGroup::CreatePrimitiveBuffer(device, primitiveBuffer_.type, maxParticle);
	// structuredBuffer(SRV)
	transformBuffer_.CreateSRVBuffer(device, maxParticle);
	materialBuffer_.CreateSRVBuffer(device, maxParticle);
	textureInfoBuffer_.CreateSRVBuffer(device, maxParticle);
}

void CPUParticleGroup::Update() {

	// フェーズの更新処理
	UpdatePhase();
}

void CPUParticleGroup::FrequencyEmit() {

	const float deltaTime = GameTimer::GetDeltaTime();

	for (auto& phase : phases_) {

		// 一定間隔で発生させる
		phase->FrequencyEmit(particles_, deltaTime);

		// emitterの更新
		phase->UpdateEmitter();
	}
}

void CPUParticleGroup::Emit() {

	for (auto& phase : phases_) {

		// 強制的に発生させる
		phase->Emit(particles_);

		// emitterの更新
		phase->UpdateEmitter();
	}
}

void CPUParticleGroup::ApplyCommand(const ParticleCommand& command) {

	// フェーズがない場合は処理しない
	if (phases_.empty()) {
		return;
	}

	for (const auto& phase : phases_) {

		phase->ApplyCommand(command);
	}
}

void CPUParticleGroup::UpdatePhase() {

	// フェーズがない場合は処理しない
	if (phases_.empty()) {
		return;
	}

	const float deltaTime = GameTimer::GetDeltaTime();

	// 転送データのリサイズ
	ResizeTransferData(static_cast<uint32_t>(particles_.size()));

	// 全てのparticleに対して更新処理を行う
	uint32_t particleIndex = 0;
	for (auto it = particles_.begin(); it != particles_.end();) {

		auto& particle = *it;
		// フェーズインデックスが範囲外にならないように制御
		particle.phaseIndex = (std::min)(particle.phaseIndex, static_cast<uint32_t>(phases_.size() - 1));

		// 現在のフェーズで更新
		phases_[particle.phaseIndex]->UpdateParticle(particle, deltaTime);

		// 削除、フェーズ判定処理
		if (particle.lifeTime <= particle.currentTime) {
			// 次のフェーズがあれば次に移る
			if (particle.phaseIndex + 1 < phases_.size()) {

				// フェーズを進める
				++particle.phaseIndex;
				// リセット
				particle.currentTime = 0.0f;
				particle.progress = 0.0f;
				// 次のフェーズの生存時間で初期化
				particle.lifeTime = phases_[particle.phaseIndex]->GetLifeTime();
				continue;
			} else {

				// 削除
				it = particles_.erase(it);
				continue;
			}
		}

		// bufferに渡すデータの更新処理
		UpdateTransferData(particleIndex, *it);

		// indexを進める
		++it;
		++particleIndex;
	}

	// instance数を更新
	numInstance_ = static_cast<uint32_t>(particles_.size());
	// buffer転送
	TransferBuffer();
}

void CPUParticleGroup::UpdateTransferData(uint32_t particleIndex,
	const CPUParticle::ParticleData& particle) {

	// transform
	transferTransforms_[particleIndex] = particle.transform;
	// 親の設定
	if (parentTransform_) {

		transferTransforms_[particleIndex].aliveParent = true;
		transferTransforms_[particleIndex].parentMatrix = parentTransform_->matrix.world;
	} else {

		transferTransforms_[particleIndex].aliveParent = false;
		transferTransforms_[particleIndex].parentMatrix = Matrix4x4::MakeIdentity4x4();
	}

	// material
	transferMaterials_[particleIndex] = particle.material;
	// texture
	transferTextureInfos_[particleIndex] = particle.textureInfo;
	// primitive
	switch (primitiveBuffer_.type) {
	case ParticlePrimitiveType::Plane: {

		transferPrimitives_.plane[particleIndex] = particle.primitive.plane;
		break;
	}
	case ParticlePrimitiveType::Ring: {

		transferPrimitives_.ring[particleIndex] = particle.primitive.ring;
		break;
	}
	case ParticlePrimitiveType::Cylinder: {

		transferPrimitives_.cylinder[particleIndex] = particle.primitive.cylinder;
		break;
	}
	case ParticlePrimitiveType::Crescent: {

		transferPrimitives_.crescent[particleIndex] = particle.primitive.crescent;
		break;
	}
	}
}

void CPUParticleGroup::TransferBuffer() {

	// transform
	transformBuffer_.TransferData(transferTransforms_);
	// material
	materialBuffer_.TransferData(transferMaterials_);
	// texture
	textureInfoBuffer_.TransferData(transferTextureInfos_);
	// primitive
	switch (primitiveBuffer_.type) {
	case ParticlePrimitiveType::Plane: {

		primitiveBuffer_.plane.TransferData(transferPrimitives_.plane);
		break;
	}
	case ParticlePrimitiveType::Ring: {

		primitiveBuffer_.ring.TransferData(transferPrimitives_.ring);
		break;
	}
	case ParticlePrimitiveType::Cylinder: {

		primitiveBuffer_.cylinder.TransferData(transferPrimitives_.cylinder);
		break;
	}
	case ParticlePrimitiveType::Crescent: {

		primitiveBuffer_.crescent.TransferData(transferPrimitives_.crescent);
		break;
	}
	}
}

void CPUParticleGroup::ResizeTransferData(uint32_t size) {

	// transform
	transferTransforms_.resize(size);
	// material
	transferMaterials_.resize(size);
	// textureInfo
	transferTextureInfos_.resize(size);
	// primitive
	switch (primitiveBuffer_.type) {
	case ParticlePrimitiveType::Plane: {

		transferPrimitives_.plane.resize(size);
		break;
	}
	case ParticlePrimitiveType::Ring: {

		transferPrimitives_.ring.resize(size);
		break;
	}
	case ParticlePrimitiveType::Cylinder: {

		transferPrimitives_.cylinder.resize(size);
		break;
	}
	case ParticlePrimitiveType::Crescent: {

		transferPrimitives_.crescent.resize(size);
		break;
	}
	}
}

void CPUParticleGroup::AddPhase() {

	// phase追加
	phases_.emplace_back(std::make_unique<ParticlePhase>());
	ParticlePhase* phase = phases_.back().get();
	phase->Init(asset_, primitiveBuffer_.type);
	phase->SetSpawner(ParticleSpawnModuleID::Sphere);

	selectedPhase_ = static_cast<int>(phases_.size() - 1);
}

void CPUParticleGroup::ImGui() {

	ImGui::Text("numInstance: %d / %d", numInstance_, useGame_ ? gameMaxParticleCount_ : kMaxCPUParticles);
	ImGui::Checkbox("useGame", &useGame_);
	ImGui::SeparatorText("Phases");

	// 追加ボタン
	if (ImGui::Button("Add Phase")) {

		AddPhase();
	}

	if (!phases_.empty()) {

		ImGui::BeginChild("PhaseList", ImVec2(88.0f, 0.0f), ImGuiChildFlags_Border);
		for (size_t i = 0; i < phases_.size(); ++i) {

			ImGui::PushID(static_cast<int>(i));
			bool selected = (selectedPhase_ == static_cast<int>(i));
			if (ImGui::Selectable(("Phase" + std::to_string(i)).c_str(), selected)) {

				selectedPhase_ = static_cast<int>(i);
			}

			ImGui::SameLine();
			if (ImGui::SmallButton("X")) {

				phases_.erase(phases_.begin() + i);
				selectedPhase_ = std::clamp(selectedPhase_, 0, static_cast<int>(phases_.size()) - 1);
				ImGui::PopID();
				break;
			}

			ImGui::PopID();
		}
		ImGui::EndChild();
		ImGui::SameLine();
	}

	// 選択フェーズ値操作
	ImGui::BeginChild("PhaseEdit", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border);
	if (0 <= selectedPhase_ && selectedPhase_ < static_cast<int>(phases_.size())) {

		ImGui::PushItemWidth(160.0f);

		EnumAdapter<BlendMode>::Combo("blendMode", &blendMode_);
		ImGui::DragInt("gameMaxParticleCount", &gameMaxParticleCount_, 1, 0, kMaxCPUParticles);

		phases_[selectedPhase_]->ImGui();

		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
}

Json CPUParticleGroup::ToJson() const {

	Json data;

	//============================================================================
	//	GroupParameters
	//============================================================================

	data["primitive"] = EnumAdapter<ParticlePrimitiveType>::ToString(primitiveBuffer_.type);
	data["blendMode"] = EnumAdapter<BlendMode>::ToString(blendMode_);

	data["textureName"] = textureName_;
	data["gameMaxParticleCount_"] = gameMaxParticleCount_;

	//============================================================================
	//	PhasesParameters
	//============================================================================

	for (auto& phase : phases_) {

		data["phases"].push_back(phase->ToJson());
	}
	return data;
}

void CPUParticleGroup::FromJson(const Json& data, Asset* asset) {

	//============================================================================
	//	GroupParameters
	//============================================================================

	const auto& primitive = EnumAdapter<ParticlePrimitiveType>::FromString(data["primitive"]);
	primitiveBuffer_.type = primitive.value();
	const auto& blendMode = EnumAdapter<BlendMode>::FromString(data["blendMode"]);
	blendMode_ = blendMode.value();

	textureName_ = data.value("textureName", "circle");
	gameMaxParticleCount_ = data.value("gameMaxParticleCount_", 0);

	//============================================================================
	//	PhasesParameters
	//============================================================================

	phases_.clear();
	for (auto& phaseData : data["phases"]) {

		auto phase = std::make_unique<ParticlePhase>();
		phase->Init(asset, primitiveBuffer_.type);
		phase->FromJson(phaseData);
		phases_.push_back(std::move(phase));
	}
}