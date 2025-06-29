
#include <any>
#include <algorithm>
#include <cassert>
#include <memory>
#include <tuple>
#include <Windows.h>
#include <winerror.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "G2.Util.h"
#include "d3dUtil.h"


namespace G2 {

ID3DBlob* DXCompileShaderFromFile(const std::string& fileName, const std::string& shaderModel, const std::string& entryPoint, const void* macros)
{
	UINT shaderFlags{};
#ifdef _DEBUG
	shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	auto wFileName = mbToWstr(fileName);
	ComPtr<ID3DBlob> pErrorBlob{};
	ID3DBlob* pBlobRet{};
	int hr = D3DCompileFromFile(wFileName.c_str(), (const D3D_SHADER_MACRO*)macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), shaderModel.c_str(), shaderFlags, 0, &pBlobRet, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
		return {};
	}
	return pBlobRet;
}

} // namespace G2

