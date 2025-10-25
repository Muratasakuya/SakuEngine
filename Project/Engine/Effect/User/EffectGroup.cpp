#include "EffectGroup.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Effect/Particle/Core/ParticleManager.h>
#include <Engine/Effect/Particle/System/ParticleSystem.h>
#include <Engine/Effect/User/Methods/EffectSequencer.h>

//============================================================================
//	EffectGroup classMethods
//============================================================================

void EffectGroup::Init(const std::string& name, const std::string& groupName) {

	// object作成
	objectId_ = objectManager_->CreateEffect(name, groupName);

	// data取得
	tag_ = objectManager_->GetData<ObjectTag>(objectId_);

	// editorに登録
	ImGuiObjectEditor::GetInstance()->Registerobject(objectId_, this);

	// レイアウト設定を初期化
	ApplyLayout();
}

void EffectGroup::Emit(const Vector3& worldPos) {

	effectWorldPos_ = worldPos;
	for (auto& node : nodes_) {

		// リセット
		node.Reset();
		// 外部発生でなければアクティブ状態にする
		if (node.emit.mode != EffectEmitMode::Manual) {

			node.runtime.active = true;
		}
	}
}

void EffectGroup::Stop() {

	for (auto& node : nodes_) {

		// 外部からの呼びだして止めるノードのみ非アクティブ状態にする
		if (node.stop.condition == EffectStopCondition::ExternalStop) {

			node.runtime.active = false;
			node.runtime.pending = false;
			node.runtime.stopped = true;
		}
	}
}

void EffectGroup::StartNode(const std::string& nodeKey) {

	for (auto& node : nodes_) {
		// 指定ノードと一致するノードがあるか
		if (node.key == nodeKey) {

			// 発生処理を開始させる
			node.emitController.Start(&node.runtime);
			break;
		}
	}
}

void EffectGroup::StopNode(const std::string& nodeKey) {

	for (auto& node : nodes_) {
		// 指定ノードと一致するノードがあるか
		if (node.key == nodeKey) {

			// 非アクティブ状態にさせる
			node.runtime.stopped = true;
			node.emitController.Stop(&node.runtime);
			break;
		}
	}
}

void EffectGroup::ClearParent() {

	parentAnchorId_ = 0;
	parentAnchorName_.clear();
}

Vector3 EffectGroup::ResolveAnchorPos() const {

	// 親がいない場合は自身の基準の座標を返す
	if (parentAnchorId_ == 0) {
		return effectWorldPos_;
	}
	// 親がいる場合、親のワールド座標を返す
	if (Transform3D* transform = objectManager_->GetData<Transform3D>(parentAnchorId_)) {

		return transform->GetWorldPos();
	}
	return effectWorldPos_;
}

uint32_t EffectGroup::QueryAlive(const std::string& nodeKey, int groupIndex) const {

	for (const auto& node : nodes_) {
		if (node.key == nodeKey && node.system) {

			// ノード内のパーティクルグループのインスタンス数を取得
			const auto& groups = node.system->GetCPUGroup();
			if (0 <= groupIndex && groupIndex < static_cast<int>(groups.size())) {

				return groups[groupIndex].group.GetNumInstance();
			}
		}
	}
	return 0;
}

void EffectGroup::Update() {

	// 依存判定用シグナルを収集
	std::unordered_map<std::string, EffectNodeSignals> signals;
	signals.reserve(nodes_.size());
	for (auto& node : nodes_) {

		EffectNodeSignals signal{};
		signal.started = node.runtime.started;
		signal.stopped = node.runtime.stopped;
		signal.firstEmitted = node.runtime.didFirstEmit;
		signals[node.key] = signal;
	}

	// 親のワールド位置を取得
	const Vector3 anchor = ResolveAnchorPos();

	for (auto& node : nodes_) {

		// Sequencerの依存条件を満たしていなければスキップ
		if (!EffectSequencer::CheckStartAfter(node.sequencer.startAfter, signals, [this](
			const EffectDependencyReference& ref) { return QueryAlive(ref.nodeKey, ref.groupIndex); })) {
			continue;
		}

		//  Manual以外は開始オフセットを処理して残っていればこのフレームはスキップ
		if (node.emit.mode != EffectEmitMode::Manual && 0.0f < node.runtime.startOffsetRemain) {

			// 残りを減算
			node.runtime.startOffsetRemain -= GameTimer::GetScaledDeltaTime();
			// まだ残っていれば次フレームで処理させる
			if (0.0f < node.runtime.startOffsetRemain) {
				continue;
			}
		}

		// 発生位置を計算
		Vector3 worldPos = Vector3(anchor.x + node.module.spawnPos.x,
			anchor.y + node.module.spawnPos.y, anchor.z + node.module.spawnPos.z);

		// ノードの更新
		node.Update(worldPos, [this](const EffectDependencyReference& ref) { return QueryAlive(ref.nodeKey, ref.groupIndex); });
	}
}

void EffectGroup::ImGui() {

	EditLayout();

	ImGui::SetWindowFontScale(0.64f);

	if (ImGui::BeginTabBar("GameEffectGroup")) {
		if (ImGui::BeginTabItem("Info")) {

			// エフェクトの情報を表示
			DisplayInformation();
			// 保存処理と読み込み処理
			SaveAndLoad();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Anchor")) {

			ImGuiHelper::SelectTagTarget("Parent", &parentAnchorId_, &parentAnchorName_);
			ImGui::Text("selected: %s (%u)", parentAnchorName_.c_str(), parentAnchorId_);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Test")) {

			EditorTestTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Effect")) {

			// 左側の枠
			EditLeftChild();

			ImGui::SameLine();

			// 右側の枠
			EditRightChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::SetWindowFontScale(1.0f);
}

void EffectGroup::EditLeftChild() {

	ImGui::BeginChild("##EditLeftChild", ImVec2(leftChildWidth_, 0.0f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollbar);

	// パーティクルシステムの追加
	AddParticleSystem();

	ImGui::SeparatorText("Select");

	// パーティクルシステムの選択
	SelectParticleSystem();

	ImGui::EndChild();
}

void EffectGroup::AddParticleSystem() {

	// フォルダから追加するパーティクルシステムを選択する
	if (ImGui::Button("Load")) {

		std::string fileName;
		if (ImGuiHelper::OpenJsonDialog(fileName)) {

			EffectNode node{};

			// システムを追加
			node.system = ParticleManager::GetInstance()->CreateParticleSystem(fileName);
			node.filePath = fileName;

			// ファイル名を取得
			std::filesystem::path pathfileName = fileName;
			node.name = pathfileName.stem().string();
			node.key = node.name;

			// ノード追加
			nodes_.push_back(node);
		}
	}
}

void EffectGroup::SelectParticleSystem() {

	if (nodes_.empty()) {
		return;
	}

	// ノードをすべて表示
	ImGuiHelper::SelectableListFromStrings("Nodes", &selectNodeIndex_,
		GetNodeNames(), displaySystemCount_);
}

void EffectGroup::EditRightChild() {

	ImGui::BeginChild("##EditRightChild", ImVec2(0.0f, 0.0f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

	// ノードの値操作
	EditNode();

	ImGui::EndChild();
}

void EffectGroup::EditNode() {

	if (nodes_.empty() || selectNodeIndex_ < 0) {
		return;
	}

	// ノードの最大サイズを取得
	int nodeSize = static_cast<int>(nodes_.size());
	// 選択ノードのインデックスを範囲内に収める
	selectNodeIndex_ = std::clamp(selectNodeIndex_, 0, nodeSize - 1);
	auto& node = nodes_[selectNodeIndex_];

	ImGui::SeparatorText(node.name.c_str());
	{
		char buf[128]{};
		std::snprintf(buf, sizeof(buf), "%s", node.key.c_str());
		if (ImGui::InputText("nodeKey", buf, sizeof(buf))) {
			node.key = buf;
		}
	}

	if (ImGui::BeginTabBar("EffectGroupNode")) {

		// 発生設定
		if (ImGui::BeginTabItem("EmitSetting")) {

			EnumAdapter<EffectEmitMode>::Combo("EmitMode", &node.emit.mode);
			ImGui::DragInt("count", &node.emit.count);
			ImGui::DragFloat("delay", &node.emit.delay, 0.01f);
			ImGui::DragFloat("interval", &node.emit.interval, 0.01f);
			ImGui::DragFloat("duration", &node.emit.duration, 0.01f);
			ImGui::EndTabItem();
		}
		// 停止設定
		if (ImGui::BeginTabItem("StopSetting")) {

			EnumAdapter<EffectStopCondition>::Combo("StopCondition", &node.stop.condition);
			if (node.stop.condition == EffectStopCondition::OnParticleEmpty) {

				// 参照ノード
				static int selectedNode = 0;
				auto names = GetNodeNames();
				// 選択ノードのインデックスを範囲内に収める
				selectedNode = std::clamp(selectedNode, 0, nodeSize - 1);
				ImGuiHelper::SelectableListFromStrings("Systems", &selectedNode, names, nodeSize + 1);
				if (0 <= selectedNode && selectedNode < nodeSize) {

					node.stop.emptyRef.nodeKey = nodes_[selectedNode].key;
					// CPUグループ列挙
					std::vector<std::string> groupNames{};
					for (const auto& group : nodes_[selectedNode].system->GetCPUGroup()) {
						groupNames.emplace_back(group.name);
					}
					// グループの数
					int groupCount = static_cast<int>(groupNames.size());
					// 参照グループのインデックスを範囲内に収める
					int groupIndex = (std::max)(0, node.stop.emptyRef.groupIndex);
					groupIndex = (std::min)(groupIndex, groupCount - 1);
					ImGuiHelper::SelectableListFromStrings("Groups", &groupIndex, groupNames, groupCount + 1);
					// 参照グループのインデックスを設定
					node.stop.emptyRef.groupIndex = groupIndex;
				}
			}
			ImGui::EndTabItem();
		}
		// モジュール設定
		if (ImGui::BeginTabItem("ModuleSetting")) {

			EnumAdapter<ParticleLifeEndMode>::Combo("LifeEndMode", &node.module.lifeEndMode);
			EnumAdapter<EffectPosePreset>::Combo("PosePreset", &node.module.posePreset);

			ImGui::DragFloat3("spawnPos", &node.module.spawnPos.x, 0.01f);
			ImGui::DragFloat3("spawnRotate", &node.module.spawnRotate.x, 0.01f);

			// 発生、更新モジュール設定
			{
				ImGui::SeparatorText("Spawner");

				ImGui::Checkbox("spawnerScaleEnable", &node.module.spawnerScaleEnable);
				ImGui::DragFloat("spawnerScaleValue", &node.module.spawnerScaleValue, 0.01f);
			}
			{
				ImGui::SeparatorText("Updater");

				ImGui::Checkbox("updaterScaleEnable", &node.module.updaterScaleEnable);
				ImGui::DragFloat("updaterScaleValue", &node.module.updaterScaleValue, 0.01f);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Sequencer")) {

			EnumAdapter<EffectSequencerStartCondition>::Combo("Condition", &node.sequencer.startAfter.condition);
			ImGui::DragFloat("startOffset", &node.sequencer.startOffset, 0.01f);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void EffectGroup::EditorTestTab() {

	ImGui::SeparatorText("Emit");
	ImGui::DragFloat3("worldPos", &editorEmitPos_.x, 0.01f);

	if (ImGui::Button("Emit")) {

		Emit(editorEmitPos_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {

		Stop();
	}

	// ノードのサイズ
	int noseSize = static_cast<int>(nodes_.size());

	// Manualノードの個別制御
	if (!nodes_.empty() && 0 <= selectNodeIndex_ && selectNodeIndex_ < noseSize) {

		auto& node = nodes_[selectNodeIndex_];

		ImGui::SeparatorText("Manual Control");
		ImGui::Text("nodeKey: %s", node.key.c_str());
		if (node.emit.mode == EffectEmitMode::Manual) {
			if (ImGui::Button("StartNode")) {

				StartNode(node.key);
			}
			ImGui::SameLine();
			if (ImGui::Button("StopNode")) {

				StopNode(node.key);
			}
		} else {

			ImGui::TextDisabled("This node is not Manual.");
		}
	}
	if (!nodes_.empty() && 0 <= selectNodeIndex_ && selectNodeIndex_ < noseSize) {

		ImGui::SeparatorText("Runtime");
		DisplayRuntimeNode(nodes_[selectNodeIndex_]);
	}
}

void EffectGroup::DisplayRuntimeNode(EffectNode& node) {

	// Runtime 基本
	ImGui::Text(std::format("active: {}", node.runtime.active).c_str());
	ImGui::Text(std::format("pending: {}", node.runtime.pending).c_str());
	ImGui::Text(std::format("started: {}", node.runtime.started).c_str());
	ImGui::Text(std::format("stopped: {}", node.runtime.stopped).c_str());
	ImGui::Text(std::format("didFirstEmit: {}", node.runtime.didFirstEmit).c_str());
	ImGui::Text("emitted: %d", node.runtime.emitted);
	ImGui::Text("timer: %.3f", node.runtime.timer);
	ImGui::Text("emitTimer: %.3f", node.runtime.emitTimer);
	ImGui::Text("startOffsetRemain: %.3f", node.runtime.startOffsetRemain);

	// このノード内のCPUグループの生存数
	if (node.system) {
		const auto& groups = node.system->GetCPUGroup();
		if (!groups.empty()) {

			ImGui::SeparatorText("Alive Instances");
			for (int index = 0; index < static_cast<int>(groups.size()); ++index) {

				const auto& group = groups[index];
				ImGui::Text("[%d] %s : %u", index, group.name.c_str(), group.group.GetNumInstance());
			}
		}
	}
}

void EffectGroup::DisplayInformation() {

	ImGui::Text("name: %s", tag_->name.c_str());
	ImGui::Text("objectId: %u", objectId_);

	ImGui::Separator();
}

void EffectGroup::SaveAndLoad() {

	// 保存処理呼び出し
	if (ImGui::Button("Save")) {

		jsonSaveState_.showPopup = true;
	}
	ImGui::SameLine();

	// 読み込んでデータを作成
	if (ImGui::Button("Load")) {

		std::string fileName;
		if (ImGuiHelper::OpenJsonDialog(fileName)) {

			LoadJson(fileName);
		}
	}
	// 保存処理
	{
		std::string fileName;
		if (ImGuiHelper::SaveJsonModal("Save CameraParam", baseJsonPath_.c_str(),
			baseJsonPath_.c_str(), jsonSaveState_, fileName)) {

			SaveJson(fileName);
		}
	}
	ImGui::Separator();
}

void EffectGroup::EditLayout() {

	ImGui::Begin("GameEffectGroupLayout");

	if (ImGui::Button("Save")) {

		SaveLayout();
	}

	ImGui::DragFloat("leftChildWidth", &leftChildWidth_, 0.1f);
	ImGui::DragInt("displaySystemCount_", &displaySystemCount_);

	ImGui::End();
}

void EffectGroup::LoadJson(const std::string& fileName) {

	Json data;
	if (!JsonAdapter::LoadCheck(fileName, data)) {
		return;
	}

	// ノードをクリア
	nodes_.clear();
	parentAnchorId_ = data.value("parentAnchorId_", 0);
	parentAnchorName_ = data.value("parentAnchorName_", "");

	if (auto it = data.find("Nodes"); it != data.end() && it->is_array()) {
		for (const auto& nodeData : *it) {

			EffectNode node{};
			node.key = nodeData.value("key", "");
			node.name = nodeData.value("name", node.key);

			// ParticleSystem
			if (auto system = nodeData.find("ParticleSystem"); system != nodeData.end()) {

				const std::string filePath = system->value("path", "");
				if (!filePath.empty()) {

					node.system = ParticleManager::GetInstance()->CreateParticleSystem(filePath);
					node.filePath = filePath;
				}
			}

			// Emit
			if (auto emit = nodeData.find("Emit"); emit != nodeData.end()) {

				node.emit.mode = EnumAdapter<EffectEmitMode>::FromString(emit->value("Mode", "Once")).value();
				node.emit.count = emit->value("count", 1);
				node.emit.delay = emit->value("delay", 0.0f);
				node.emit.interval = emit->value("interval", 0.0f);
				node.emit.duration = emit->value("duration", 0.0f);
			}

			// Stop
			if (auto stop = nodeData.find("Stop"); stop != nodeData.end()) {

				node.stop.condition = EnumAdapter<EffectStopCondition>::FromString(stop->value("Condition", "None")).value();
				node.stop.emptyRef.nodeKey = stop->value("emptyNodeKey", "");
				node.stop.emptyRef.groupIndex = stop->value("emptyGroupIndex", -1);
			}

			// Module
			if (auto module = nodeData.find("Module"); module != nodeData.end()) {

				node.module.lifeEndMode = EnumAdapter<ParticleLifeEndMode>::FromString(module->value("LifeEndMode", "Advance")).value();
				node.module.posePreset = EnumAdapter<EffectPosePreset>::FromString(module->value("PosePreset", "None")).value();
				node.module.spawnPos = Vector3::FromJson(module->value("spawnPos", Json()));
				node.module.spawnRotate = Vector3::FromJson(module->value("spawnRotate", Json()));
				// 発生モジュール
				node.module.spawnerScaleEnable = module->value("spawnerScaleEnable", false);
				node.module.spawnerScaleValue = module->value("spawnerScaleValue", 1.0f);
				// 更新モジュール
				node.module.updaterScaleEnable = module->value("updaterScaleEnable", false);
				node.module.updaterScaleValue = module->value("updaterScaleValue", 1.0f);
			}

			// Sequencer
			node.sequencer.startOffset = nodeData.value("startOffset", 0.0f);
			if (auto after = nodeData.find("StartAfter"); after != nodeData.end()) {

				node.sequencer.startAfter.condition = EnumAdapter<EffectSequencerStartCondition>::FromString(after->value("Condition", "None")).value();
				node.sequencer.startAfter.emptyRef.nodeKey = after->value("nodeKey", "");
				node.sequencer.startAfter.emptyRef.groupIndex = after->value("groupIndex", -1);
			}

			nodes_.push_back(node);
		}
	}
}

void EffectGroup::SaveJson(const std::string& filePath) {

	Json data;

	// 親
	data["parentAnchorId_"] = parentAnchorId_;
	data["parentAnchorName_"] = parentAnchorName_;

	// ノード配列
	Json nodes = Json::array();
	for (const auto& node : nodes_) {

		Json nodeData;

		// 基本情報
		nodeData["key"] = node.key;
		nodeData["name"] = node.name;

		// ParticleSystem
		{
			// 保存パスが未設定なら、name からの既定パスなどにフォールバック
			const std::string path = !node.filePath.empty() ? node.filePath : (node.name + ".json");
			nodeData["ParticleSystem"]["path"] = path;
		}

		// Emit
		{
			Json emit;
			emit["Mode"] = EnumAdapter<EffectEmitMode>::ToString(node.emit.mode);
			emit["count"] = node.emit.count;
			emit["delay"] = node.emit.delay;
			emit["interval"] = node.emit.interval;
			emit["duration"] = node.emit.duration;
			nodeData["Emit"] = emit;
		}

		// Stop
		{
			Json stop;
			stop["Condition"] = EnumAdapter<EffectStopCondition>::ToString(node.stop.condition);
			stop["emptyNodeKey"] = node.stop.emptyRef.nodeKey;
			stop["emptyGroupIndex"] = node.stop.emptyRef.groupIndex;
			nodeData["Stop"] = stop;
		}

		// Module
		{
			Json module;
			module["LifeEndMode"] = EnumAdapter<ParticleLifeEndMode>::ToString(node.module.lifeEndMode);
			module["PosePreset"] = EnumAdapter<EffectPosePreset>::ToString(node.module.posePreset);
			module["spawnPos"] = node.module.spawnPos.ToJson();
			module["spawnRotate"] = node.module.spawnRotate.ToJson();
			module["spawnerScaleEnable"] = node.module.spawnerScaleEnable;
			module["spawnerScaleValue"] = node.module.spawnerScaleValue;
			module["updaterScaleEnable"] = node.module.updaterScaleEnable;
			module["updaterScaleValue"] = node.module.updaterScaleValue;
			nodeData["Module"] = module;
		}

		// Sequencer
		nodeData["startOffset"] = node.sequencer.startOffset;
		{
			Json after;
			after["Condition"] = EnumAdapter<EffectSequencerStartCondition>::ToString(node.sequencer.startAfter.condition);
			after["nodeKey"] = node.sequencer.startAfter.emptyRef.nodeKey;
			after["groupIndex"] = node.sequencer.startAfter.emptyRef.groupIndex;
			nodeData["StartAfter"] = after;
		}

		nodes.push_back(nodeData);
	}
	data["Nodes"] = nodes;

	JsonAdapter::Save(filePath, data);
}

void EffectGroup::ApplyLayout() {

	Json data;
	if (!JsonAdapter::LoadCheck(baseJsonPath_ + "Layout/editorLayout.json", data)) {
		return;
	}

	leftChildWidth_ = data.value("leftChildWidth_", 192.0f);
	displaySystemCount_ = data.value("displaySystemCount_", 16);
}

void EffectGroup::SaveLayout() {

	Json data;

	data["leftChildWidth_"] = leftChildWidth_;
	data["displaySystemCount_"] = displaySystemCount_;

	JsonAdapter::Save(baseJsonPath_ + "Layout/editorLayout.json", data);
}

std::vector<std::string> EffectGroup::GetNodeNames() const {

	std::vector<std::string> nodeNames{};
	for (const auto& node : nodes_) {

		nodeNames.emplace_back(node.name);
	}
	return nodeNames;
}