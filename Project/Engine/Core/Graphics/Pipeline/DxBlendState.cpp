#include "DxBlendState.h"

//============================================================================
//	DxBlendState classMethods
//============================================================================

void DxBlendState::Create(BlendMode blendMode, D3D12_RENDER_TARGET_BLEND_DESC& blendDesc) {

	blendDesc = {};
	blendDesc.BlendEnable = false;
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// 各ブレンドモードの設定をswitch文で行う
	switch (blendMode) {
		// 通常αブレンド
	case BlendMode::kBlendModeNormal:

		blendDesc.BlendEnable = true;
		blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

		break;

		// 加算
	case BlendMode::kBlendModeAdd:

		blendDesc.BlendEnable = true;
		blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.DestBlend = D3D12_BLEND_ONE;
		blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

		break;

		// 減算
	case BlendMode::kBlendModeSubtract:

		blendDesc.BlendEnable = true;
		blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.DestBlend = D3D12_BLEND_ONE;
		blendDesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

		break;

		// 乗算
	case BlendMode::kBlendModeMultiply:

		blendDesc.BlendEnable = true;
		blendDesc.SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.DestBlend = D3D12_BLEND_SRC_COLOR;
		blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

		break;

		// スクリーン
	case BlendMode::kBlendModeScreen:

		blendDesc.BlendEnable = true;
		blendDesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		blendDesc.DestBlend = D3D12_BLEND_ONE;
		blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

		break;
	}
}