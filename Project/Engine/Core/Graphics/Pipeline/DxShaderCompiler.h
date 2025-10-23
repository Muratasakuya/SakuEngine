#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
#include <dxcapi.h>
// c++
#include <vector>
// json
#include <Externals/nlohmann/json.hpp>
// namespace using
namespace fs = std::filesystem;
using Json = nlohmann::json;

//============================================================================
//	DxShaderCompiler class
//	DXCでHLSLをDXILにコンパイルし、必要なシェーダBlob群を生成する。
//============================================================================
class DxShaderCompiler {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxShaderCompiler() = default;
	~DxShaderCompiler() = default;

	// DXCの初期化(コンパイラインターフェース/インクルードハンドラ等の準備)
	void Init();

	// JSON定義を基に各ステージのHLSLをコンパイルし、Blob配列を返す
	void Compile(const Json& json, std::vector<ComPtr<IDxcBlob>>& shaderBlobs);

	void CompileShader(
		const std::string& fileName,
		const std::wstring& filePath,
		const wchar_t* profile,
		ComPtr<IDxcBlob>& shaderBlob,
		const wchar_t* entry);
	void CompileShaderLibrary(
		const std::wstring& filePath,
		const std::wstring& exports,
		ComPtr<IDxcBlob>& shaderBlob);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<IDxcUtils> dxcUtils_;
	ComPtr<IDxcCompiler3> dxcCompiler_;
	ComPtr<IDxcIncludeHandler> includeHandler_;
};