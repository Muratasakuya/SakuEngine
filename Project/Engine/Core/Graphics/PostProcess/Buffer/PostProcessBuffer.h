#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>

//============================================================================
//	IPostProcessBuffer class
//	ポストプロセス用定数バッファの共通インターフェース。更新/描画用のアクセスを提供。
//============================================================================
class IPostProcessBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPostProcessBuffer() = default;
	virtual ~IPostProcessBuffer() = default;

	// バッファを生成し、ルートパラメータのインデックスを設定する
	virtual void Init(ID3D12Device* device, UINT rootIndex) = 0;

	// CPU側パラメータをGPUへ転送する
	virtual void Update() = 0;

	// パラメータのimgui編集UIを表示する
	virtual void ImGui() = 0;

	//--------- accessor -----------------------------------------------------

	// 汎用パラメータセット(任意の型をメモリコピー)
	virtual void SetParameter(const void* parameter, size_t size) = 0;

	// CBリソースを取得する
	virtual ID3D12Resource* GetResource() const = 0;

	// ルートパラメータのインデックスを取得する
	UINT GetRootIndex() const { return rootIndex_; };
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	UINT rootIndex_ = 0;
};

//============================================================================
//	PostProcessBuffer class
//	T型のパラメータを保持し、DxConstBuffer<T>に転送する実装クラス。
//============================================================================
template <typename T>
class PostProcessBuffer :
	public IPostProcessBuffer, public DxConstBuffer<T> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PostProcessBuffer() = default;
	~PostProcessBuffer() = default;

	// 定数バッファを作成し、ルートインデックスを記録する
	void Init(ID3D12Device* device, UINT rootIndex) override;

	// 直近のparameter_をGPUへ転送する
	void Update() override;

	// パラメータ編集UIを表示する(TにImGui()がある前提)
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 任意データをparameter_へコピーする
	void SetParameter(const void* parameter, size_t size) override;

	// CBリソースを取得する
	ID3D12Resource* GetResource() const override;
	// 現在のパラメータ値を取得する
	T GetParameter() const { return parameter_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	T parameter_;
};

//============================================================================
//	PostProcessBuffer templateMethods
//============================================================================

template<typename T>
inline void PostProcessBuffer<T>::Init(ID3D12Device* device, UINT rootIndex) {

	rootIndex_ = rootIndex;

	DxConstBuffer<T>::CreateBuffer(device);
}

template<typename T>
inline void PostProcessBuffer<T>::Update() {

	DxConstBuffer<T>::TransferData(parameter_);
}

template<typename T>
inline void PostProcessBuffer<T>::ImGui() {

	parameter_.ImGui();
}

template<typename T>
inline void PostProcessBuffer<T>::SetParameter(const void* parameter, size_t size) {

	std::memcpy(&parameter_, parameter, size);
}

template<typename T>
inline ID3D12Resource* PostProcessBuffer<T>::GetResource() const {

	return DxConstBuffer<T>::GetResource();
}