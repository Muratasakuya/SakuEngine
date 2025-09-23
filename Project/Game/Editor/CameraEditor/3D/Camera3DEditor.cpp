#include "Camera3DEditor.h"

//============================================================================
//	include
//============================================================================

//============================================================================
//	Camera3DEditor classMethods
//============================================================================

Camera3DEditor* Camera3DEditor::instance_ = nullptr;

Camera3DEditor* Camera3DEditor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new Camera3DEditor();
	}
	return instance_;
}

void Camera3DEditor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}