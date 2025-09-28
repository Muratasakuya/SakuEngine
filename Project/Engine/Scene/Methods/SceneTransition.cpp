#include "SceneTransition.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	SceneTransition classMethods
//============================================================================

void SceneTransition::Init() {

	isTransition_ = false;
	isBeginTransitionFinished_ = false;
	isLoadingFinished_ = false;
	isLoadEndFinished_ = false;

	state_ = TransitionState::Begin;
}

void SceneTransition::Update() {

	if (!isTransition_) {
		return;
	}

	assert(transition_);
	switch (state_) {
	case TransitionState::Begin: {

		transition_->BeginUpdate();
		if (transition_->GetState() == TransitionState::Load) {

			// 次の状態へ遷移
			state_ = TransitionState::Load;
			isBeginTransitionFinished_ = true;
		}
		break;
	}
	case TransitionState::Load: {

		// 読み込みが完了次第次に進める
		transition_->SetLoadingFinished(isLoadingFinished_);
		transition_->LoadUpdate();
		if (transition_->GetState() == TransitionState::LoadEnd) {

			state_ = TransitionState::LoadEnd;
		}
		break;
	}
	case TransitionState::LoadEnd: {

		// 読み込み終了後更新
		transition_->LoadEndUpdate();
		if (transition_->GetState() == TransitionState::End) {

			state_ = TransitionState::End;
			isLoadEndFinished_ = true;
		}
		break;
	}
	case TransitionState::End: {

		transition_->EndUpdate();
		if (transition_->GetState() == TransitionState::Begin) {

			// 最初の状態に戻す
			state_ = TransitionState::Begin;
			isTransition_ = false;
			isLoadingFinished_ = false;
		}
		break;
	}
	}
	transition_->Update();
}

void SceneTransition::SetTransition(std::unique_ptr<ITransition> transition) {

	transition_.reset();

	isTransition_ = true;
	transition_ = std::move(transition);
}

void SceneTransition::SetTransition() {

	isTransition_ = true;
}

void SceneTransition::ImGui() {

	ImGui::Text("CurrentTransition : %s", EnumAdapter<TransitionState>::ToString(state_));
	if (!isTransition_) {
		return;
	}

	transition_->ImGui();
}

bool SceneTransition::ConsumeLoadEndFinished() {

	bool result = isLoadEndFinished_;
	isLoadEndFinished_ = false;
	return result;
}