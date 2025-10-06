#pragma once

//============================================================================
//	include
//============================================================================

// front
class SceneView;
class SceneManager;

//============================================================================
//	Scene
//============================================================================

enum class Scene {

	Effect,
	Title,
	Game,
};

//============================================================================
//	IScene class
//============================================================================
class IScene {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IScene() = default;
	virtual ~IScene() = default;

	virtual void Init() = 0;

	virtual void Update() = 0;

	virtual void BeginFrame() {}
	virtual void EndFrame() {}

	//--------- accessor -----------------------------------------------------

	void SetPtr(SceneView* sceneView, SceneManager* sceneManager);

	bool IsFinishGame() const { return isFinishGame_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	SceneView* sceneView_;
	SceneManager* sceneManager_;

	bool isFinishGame_ = false;;
};