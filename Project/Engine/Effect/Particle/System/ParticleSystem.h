#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Data/GPUParticleGroup.h>
#include <Engine/Effect/Particle/Data/CPUParticleGroup.h>

// windows
#include <commdlg.h>
#include <windows.h>
// imgui
#include <imgui.h>
// c++
#include <filesystem>
// front
class Asset;

//============================================================================
//	ParticleSystem class
//============================================================================
class ParticleSystem {
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	template <typename T>
	struct NameGroup {

		std::string name;
		T group;
	};

	struct GroupHandle {

		ParticleType type;
		int index;
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleSystem() = default;
	~ParticleSystem() = default;

	void Init(ID3D12Device* device, Asset* asset, const std::string& name);

	void Update();

	// editor
	void ImGuiSelectedGroupEditor();
	void ImGuiGroupAdd();
	void ImGuiGroupSelect();
	void ImGuiSystemParameter();

	//--------- accessor -----------------------------------------------------

	void SetName(const std::string& name) { name_ = name; }
	void SetGroupName(uint32_t i, const std::string& name) { gpuGroups_[i].name = name; }
	void SelectGroup(int index) { selected_.index = index; }
	void SetParent(const BaseTransform& parent);

	const std::string& GetName() const { return name_; }
	const std::string& GetGroupName(uint32_t i) const { return gpuGroups_[i].name; }

	std::vector<NameGroup<GPUParticleGroup>>& GetGPUGroup() { return gpuGroups_; }
	std::vector<NameGroup<CPUParticleGroup>>& GetCPUGroup() { return cpuGroups_; }

	//---------- runtime -----------------------------------------------------

	void ApplyCommand(const ParticleCommand& command);

	// .jsonファイルから読み込んで作成する
	void LoadJson(const std::optional<std::string>& filePath = std::nullopt, bool useGame = false);

	// 一定間隔
	void FrequencyEmit();
	// 強制発生
	void Emit();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// Groupのコピー用データ
	struct CopyGroup {

		bool hasData;
		ParticleType type;
		Json data;
	};

	//--------- variables ----------------------------------------------------

	ID3D12Device* device_;
	Asset* asset_;

	// system名
	std::string name_;
	std::string loadFileName_;

	// GPU
	std::vector<NameGroup<GPUParticleGroup>> gpuGroups_;
	// CPU
	std::vector<NameGroup<CPUParticleGroup>> cpuGroups_;

	// runtime
	bool useGame_; // ゲーム側で使用する場合

	// allEmit
	// 全て同時に発生させる
	bool allEmitEnable_; // フラグ
	float allEmitTimer_; // 経過時間経過
	float allEmitTime_;  // 経過時間

	// editor
	CopyGroup copyGroup_;
	ParticleType particleType_;
	ParticlePrimitiveType primitiveType_;
	GroupHandle selected_{ ParticleType::GPU, -1 };
	GroupHandle renaming_{ ParticleType::GPU, -1 };
	int nextGroupId_ = 0;         // グループ添え字インデックス
	char renameBuffer_[128] = {}; // 改名入力用
	char fileBuffer_[128] = {};   // ファイル入力用
	bool showSavePopup_ = false;
	static constexpr int kSaveNameSize = 128;
	char jsonSaveInput_[kSaveNameSize] = {};
	// layout
	float comboWidth_ = 104.0f;
	float itemWidth_ = 160.0f;
	ImVec2 buttonSize_ = ImVec2(88.0f, 24.0f);

	//--------- functions ----------------------------------------------------

	// json
	void SaveJson();

	// update
	void UpdateAllEmit();

	// editor
	void AddGroup();
	void RemoveGroup();
	void HandleCopyPaste();

	// helper
	void EditLayout();
	bool ShowOpenJsonDialog(std::string& outRelPath);
};