//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	TitleViewCamera class
//	タイトルシーンの視点
//============================================================================
class TitleViewCamera :
	public BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TitleViewCamera() = default;
	~TitleViewCamera() = default;

	void Init();

	void Update() override;

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// Rotate
	float rotateSpeed_;  // 回転速度
	float initRotateX_;  // X軸回転角
	float eulerRotateX_; // X軸回転角
	Vector3 viewPoint_;  // 注視点
	float viewOffset_;   // 注視点からのオフセット距離

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};