#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/CollisionBody.h>

// c++
#include <string>
#include <optional>
// front
class Transform3D;

//============================================================================
//	Collider class
//	ゲームオブジェクトに紐づく衝突ボディの生成/更新/編集と、当たりイベントのフックを提供する。
//============================================================================
class Collider {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Collider() = default;
	virtual ~Collider();

	// 親子関係を考慮して全CollisionBodyの形状パラメータをTransform3Dから更新する
	void UpdateAllBodies(const Transform3D& transform);

	// 新しい衝突ボディを生成してCollisionManagerに登録する(必要ならオフセットも追加)
	CollisionBody* AddCollider(const CollisionShape::Shapes& shape, bool autoAddOffset = false);
	// 衝突ボディをCollisionManagerから削除する
	void RemoveCollider(CollisionBody* collisionBody);

	// 衝突開始時に呼ばれるコールバック
	virtual void OnCollisionEnter([[maybe_unused]] const CollisionBody* collisionBody) {};
	// 衝突継続中に呼ばれるコールバック
	virtual void OnCollisionStay([[maybe_unused]] const CollisionBody* collisionBody) {};
	// 衝突終了時に呼ばれるコールバック
	virtual void OnCollisionExit([[maybe_unused]] const CollisionBody* collisionBody) {};

	// ImGui上で各ボディのオフセット編集UIを描画する
	void ImGui(float itemWidth);

	// jsonから各ボディのオフセット値を適用する
	void ApplyBodyOffset(const Json& data);
	// 各ボディのオフセット値をjsonへ保存する
	void SaveBodyOffset(Json& data);

	// json定義を元にボディ群を構築し、タイプ/ターゲットを設定する
	void BuildBodies(const Json& data);

	//--------- accessor -----------------------------------------------------

	// 子として扱うかどうかの設定(ワールド行列を使った更新に切替)
	void SetIsChild(bool isChild) { isChild_ = isChild; }
	// 直近フレームでEnter状態かどうか
	bool IsHit() const { return currentState_ == State::Enter; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::vector<CollisionBody*> bodies_;
	std::vector<CollisionShape::Shapes> bodyOffsets_;

	bool isChild_; // 子の場合は行列を使用して更新する

	//--------- functions ----------------------------------------------------

	// ColliderTypeの最下位ビット位置を1始まりのインデックスに変換する
	int ToIndexType(ColliderType type);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 当たり判定の状態を表す
	enum class State {

		None,
		Enter,
		Stay,
		Exit
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	//--------- functions ----------------------------------------------------

	// Transform3Dとオフセットから各形状のワールド情報を計算して更新する
	void UpdateSphereBody(CollisionBody* body, const Transform3D& transform, const CollisionShape::Sphere& offset);
	void UpdateAABBBody(CollisionBody* body, const Transform3D& transform, const CollisionShape::AABB& offset);
	void UpdateOBBBody(CollisionBody* body, const Transform3D& transform, const CollisionShape::OBB& offset);

	// 形状名/パラメータをjsonから読み取り、bodyOffsets_へ追加する
	bool SetShapeParamFromJson(const std::string& shapeName, const Json& data);
	// 自身/相手の衝突タイプをjsonから設定する
	void SetTypeFromJson(CollisionBody& body, const Json& data);

	// 文字列からColliderTypeへ変換する
	ColliderType ToColliderType(const std::string& name) const;

	// ImGui用編集ハンドラ(各形状のオフセット調整UI)
	void EditSphereBody(uint32_t index, CollisionShape::Sphere& offset);
	void EditAABBBody(uint32_t index, CollisionShape::AABB& offset);
	void EditOBBBody(uint32_t index, CollisionShape::OBB& offset);
};