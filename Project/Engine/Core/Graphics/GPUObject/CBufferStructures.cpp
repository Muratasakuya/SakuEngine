#include "CBufferStructures.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>

// imgui
#include <imgui.h>

//============================================================================
//	CBufferStructures
//============================================================================

void TransformationMatrix::Update(const BaseTransform* parent, const Vector3& scale,
	const Quaternion& rotation, const Vector3& translation,
	const std::optional<Matrix4x4>& billboardMatrix) {

	// billboardMatrixに値が入っていればbillboardMatrixでrotateを計算する
	if (billboardMatrix.has_value()) {

		Matrix4x4 scaleMatrix = Matrix4x4::MakeScaleMatrix(scale);
		Matrix4x4 translateMatrix = Matrix4x4::MakeTranslateMatrix(translation);
		Quaternion billboardRot = Quaternion::FromRotationMatrix(billboardMatrix.value());

		// 回転行列取得
		Quaternion normalizedRotation = Quaternion::Normalize(rotation);
		Matrix4x4 fullRotMat = Quaternion::MakeRotateMatrix(normalizedRotation);

		Vector3 xAxis = Vector3::TransferNormal(Vector3(1.0f, 0.0f, 0.0f), fullRotMat);
		Vector3 zAxis = Vector3::TransferNormal(Vector3(0.0f, 0.0f, 1.0f), fullRotMat);

		// XZだけの回転行列作成
		Vector3 newZ = Vector3::Normalize(zAxis);
		Vector3 newX = Vector3::Normalize(xAxis);
		Vector3 newY = Vector3::Normalize(Vector3::Cross(newZ, newX));
		newX = Vector3::Normalize(Vector3::Cross(newY, newZ));

		// XZの回転行列からquaternionを取得
		Matrix4x4 xzRotMatrix = {
			newX.x, newX.y, newX.z, 0.0f,
			newY.x, newY.y, newY.z, 0.0f,
			newZ.x, newZ.y, newZ.z, 0.0f,
			0.0f,   0.0f,   0.0f,   1.0f };
		Quaternion xzRotation = Quaternion::FromRotationMatrix(xzRotMatrix);

		// Y軸はbillboard、XZはrotation
		Quaternion finalRotation = Quaternion::Multiply(Quaternion::Conjugate(billboardRot), xzRotation);
		finalRotation = Quaternion::Normalize(finalRotation);

		Matrix4x4 rotateMatrix = Quaternion::MakeRotateMatrix(finalRotation);
		world = scaleMatrix * rotateMatrix * translateMatrix;
	} else {

		world = Matrix4x4::MakeAxisAffineMatrix(
			scale, rotation, translation);
	}
	if (parent) {
		world = world * parent->matrix.world;
	}
	worldInverseTranspose = Matrix4x4::Transpose(Matrix4x4::Inverse(world));
}

void SpriteMaterialForGPU::Init() {

	color = Color::White();
	useVertexColor = false;
	useAlphaColor = false;
	emissiveIntensity = 0.0f;
	alphaReference = 0.0f;
	emissionColor = Vector3::AnyInit(1.0f);
	uvTransform = Matrix4x4::MakeIdentity4x4();
	postProcessMask = 0;
}

void SpriteMaterialForGPU::ImGui() {

	ImGui::ColorEdit4("color", &color.r);
	ImGui::Text("R:%4.3f G:%4.3f B:%4.3f A:%4.3f",
		color.r, color.g,
		color.b, color.a);
}