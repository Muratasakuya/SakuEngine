#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

//============================================================================
//	PlayerWeapon class
//	プレイヤーの武器
//============================================================================
class PlayerWeapon :
	public GameObject3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerWeapon() = default;
	~PlayerWeapon() = default;

	void Update() override;

	void DerivedImGui() override;

	// json
	void ApplyJson(const Json& data);
	void SaveJson(Json& data);

	//--------- accessor -----------------------------------------------------

	// 剣先の座標を取得
	const Vector3& GetTipTranslation() const { return tipTranslation_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// parameters
	Transform3D initTransform_; // 初期化時の値

	// 剣先の座標
	Vector3 tipTranslation_;
	// オフセット
	Vector3 tipOffset_;

	//--------- functions ----------------------------------------------------

	// helper
	void SetInitTransform();
};