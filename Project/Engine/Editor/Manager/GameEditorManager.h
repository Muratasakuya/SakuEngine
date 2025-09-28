#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>

// c++
#include <cstdint>
#include <vector>

//============================================================================
//	GameEditorManager class
//============================================================================
class GameEditorManager {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameEditorManager() = default;
	~GameEditorManager() = default;

	void AddEditor(IGameEditor* editor);
	void RemoveEditor(IGameEditor* editor);

	// editorの選択
	void SelectEditor();
	// 選択したeditorの選択
	void EditEditor();

	//--------- accessor -----------------------------------------------------

	void SetSelectObjectID(uint32_t id);

	// singleton
	static GameEditorManager* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	static GameEditorManager* instance_;

	std::vector<IGameEditor*> editors_;

	IGameEditor* selectedEditor_;

	std::optional<uint32_t> selectedIndex_ = std::nullopt;
};