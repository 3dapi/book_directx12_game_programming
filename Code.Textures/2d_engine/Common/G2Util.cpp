// Implementation of the CD3DApp class.
//
//////////////////////////////////////////////////////////////////////

#include <any>
#include <algorithm>
#include <cassert>
#include <memory>
#include <Windows.h>
#include <winerror.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "G2Util.h"
#include "DDSTextureLoader.h"

std::wstring G2::stringMultiByte2WString(const std::string& str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, {}, 0);
	std::wstring wstr(len, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
	return wstr;
}

HRESULT G2::DXCompileShaderFromFile(const std::string& szFileName
	, const std::string& szEntryPoint
	, const std::string& szShaderModel
	, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags{};
#ifdef _DEBUG
	dwShaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	auto wFileName = G2::stringMultiByte2WString(szFileName);
	ID3DBlob* pErrorBlob{};
	hr = D3DCompileFromFile(wFileName.c_str(), {}, {}, szEntryPoint.c_str(), szShaderModel.c_str(), dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			SAFE_RELEASE(pErrorBlob);
		}
		return hr;
	}
	SAFE_RELEASE(pErrorBlob);

	return S_OK;
}

HRESULT G2::DXCreateDDSTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList
	, const std::string& szFileName
	, ComPtr<ID3D12Resource>& texture
	, ComPtr<ID3D12Resource>& uploadHeap)
{
	auto wFileName = G2::stringMultiByte2WString(szFileName);
	HRESULT hr = DirectX::CreateDDSTextureFromFile12(device, cmdList, wFileName.c_str(), texture, uploadHeap);
	return hr;
}
