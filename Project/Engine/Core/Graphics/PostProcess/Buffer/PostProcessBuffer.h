#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>

//============================================================================
//	IPostProcessBuffer class
//============================================================================
class IPostProcessBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPostProcessBuffer() = default;
	virtual ~IPostProcessBuffer() = default;

	virtual void Init(ID3D12Device* device, UINT rootIndex) = 0;

	virtual void Update() = 0;

	virtual void ImGui() = 0;

	//--------- accessor -----------------------------------------------------

	virtual void SetParameter(const void* parameter, size_t size) = 0;

	virtual ID3D12Resource* GetResource() const = 0;

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

	void Init(ID3D12Device* device, UINT rootIndex) override;

	void Update() override;

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	void SetParameter(const void* parameter, size_t size) override;

	ID3D12Resource* GetResource() const override;
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