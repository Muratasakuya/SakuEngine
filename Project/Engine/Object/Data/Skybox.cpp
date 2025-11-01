#include "Skybox.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>

// imgui
#include <imgui.h>

//============================================================================
//	Skybox classMethods
//============================================================================

void Skybox::CreateVertexBuffer(ID3D12Device* device) {

	// 頂点データ設定
	std::vector<Vector4> vertices = {

		// 右面
		{1.0f,  1.0f, -1.0f, 1.0f},
		{1.0f,  1.0f,  1.0f, 1.0f},
		{1.0f, -1.0f, -1.0f, 1.0f},
		{1.0f, -1.0f,  1.0f, 1.0f},

		// 左面
		{-1.0f,  1.0f,  1.0f, 1.0f},
		{-1.0f,  1.0f, -1.0f, 1.0f},
		{-1.0f, -1.0f,  1.0f, 1.0f},
		{-1.0f, -1.0f, -1.0f, 1.0f},

		// 上面
		{-1.0f, 1.0f,  1.0f, 1.0f},
		{ 1.0f, 1.0f,  1.0f, 1.0f},
		{-1.0f, 1.0f, -1.0f, 1.0f},
		{ 1.0f, 1.0f, -1.0f, 1.0f},

		// 下面
		{-1.0f, -1.0f, -1.0f, 1.0f},
		{ 1.0f, -1.0f, -1.0f, 1.0f},
		{-1.0f, -1.0f,  1.0f, 1.0f},
		{ 1.0f, -1.0f,  1.0f, 1.0f},

		// 前面
		{-1.0f,  1.0f, 1.0f, 1.0f},
		{ 1.0f,  1.0f, 1.0f, 1.0f},
		{-1.0f, -1.0f, 1.0f, 1.0f},
		{ 1.0f, -1.0f, 1.0f, 1.0f},

		// 背面
		{ 1.0f,  1.0f, -1.0f, 1.0f},
		{-1.0f,  1.0f, -1.0f, 1.0f},
		{ 1.0f, -1.0f, -1.0f, 1.0f},
		{-1.0f, -1.0f, -1.0f, 1.0f},
	};

	// 頂点インデックスデータ設定
	std::vector<uint32_t> indices = {

		// 右面
		0, 2, 1,  1, 2, 3,

		// 左面
		4, 6, 5,  5, 6, 7,

		// 上面
		8, 10, 9, 9, 10, 11,

		// 下面
		12, 14, 13, 13, 14, 15,

		// 前面
		16, 17, 18, 18, 17, 19,

		// 背面
		20, 21, 22, 22, 21, 23
	};

	// buffer作成
	vertexBuffer_.CreateBuffer(device, static_cast<UINT>(vertices.size()));
	indexBuffer_.CreateBuffer(device, static_cast<UINT>(indices.size()));
	// index数設定
	indexCount_ = static_cast<UINT>(indices.size());

	// これ以上頂点の位置は変わらないのでbuffer転送
	vertexBuffer_.TransferData(vertices);
	indexBuffer_.TransferData(indices);
}

void Skybox::CreateCBuffer(ID3D12Device* device, uint32_t textureIndex) {

	// cBufferに渡す値の初期化
	transform_.Init();
	// 初期化値で320.0fにスケーリング
	const float scale = 640.0f;
	transform_.scale = Vector3::AnyInit(scale);
	transform_.UpdateMatrix();

	material_.color = Color::Convert(0x181818ff);
	material_.textureIndex = textureIndex;

	uvTransform_.scale = Vector3::AnyInit(1.0f);
	prevUVTransform_.scale = Vector3::AnyInit(1.0f);
	// uvの更新
	material_.uvTransform = Matrix4x4::MakeAffineMatrix(
		uvTransform_.scale, uvTransform_.rotate, uvTransform_.translation);

	// buffer作成
	matrixBuffer_.CreateBuffer(device);
	materialBuffer_.CreateBuffer(device);

	// 1度bufferを転送する
	matrixBuffer_.TransferData(transform_.matrix.world);
	materialBuffer_.TransferData(material_);
}

void Skybox::Create(ID3D12Device* device, uint32_t textureIndex, uint32_t objectIndex) {

	// data取得
	tag_ = ObjectManager::GetInstance()->GetData<ObjectTag>(objectIndex);
	// シーンを切り替えても破棄しないようにする
	tag_->destroyOnLoad = false;

	// 頂点buffer作成
	CreateVertexBuffer(device);

	// cBuffer作成
	CreateCBuffer(device, textureIndex);
}

void Skybox::Update() {

	// transform更新
	transform_.UpdateMatrix();
	// buffer転送
	matrixBuffer_.TransferData(transform_.matrix.world);

	// material更新
	UpdateUVTransform();
	// buffer転送
	materialBuffer_.TransferData(material_);
}

void Skybox::UpdateUVTransform() {

	// 値に変更がなければ更新しない
	if (uvTransform_ == prevUVTransform_) {
		return;
	}

	// uvの更新
	material_.uvTransform = Matrix4x4::MakeAffineMatrix(
		uvTransform_.scale, uvTransform_.rotate, uvTransform_.translation);

	// 値を保存
	prevUVTransform_ = uvTransform_;
}

void Skybox::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	ImGui::SeparatorText("Transform");

	transform_.ImGui(itemSize);

	ImGui::SeparatorText("Color");

	ImGui::ColorEdit4("color", &material_.color.r);

	// UV
	ImGui::SeparatorText("UV");

	// transform
	ImGui::DragFloat2("uvTranslate", &uvTransform_.translation.x, 0.01f);
	ImGui::SliderAngle("uvRotate", &uvTransform_.rotate.z);
	ImGui::DragFloat2("uvScale", &uvTransform_.scale.x, 0.01f);

	ImGui::PopItemWidth();
}