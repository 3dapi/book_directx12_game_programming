
#pragma once
#ifndef _G2_Util_H_
#define _G2_Util_H_

#include <string>
#include <vector>
#include <Windows.h>
#include <d3dcommon.h>
#include <d3d12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace G2 {


template<typename T>
inline void SAFE_DELETE(T*& p)
{
	if (p)
	{
		delete p;
		p = {};
	}
}
template<typename T>
inline void SAFE_DELETE_ARR(T*& p)
{
	if (p)
	{
		delete[] p;
		p = {};
	}
}

template<typename T>
inline void SAFE_RELEASE(T*& p)
{
	if (p)
	{
		p->Release();
		p = {};
	}
}
// NOTE: for new T*[count] raw pointer array
template<typename T>
inline void SAFE_RELEASE_ARRAY(T*& p, size_t count)
{
	if (p)
	{
		for (size_t i = 0; i < count; ++i)
		{
			if (p[i])
				p[i]->Release();
		}
		delete[] p;
		p = {};
	}
}
// NOTE: for std::vector 
template<typename T>
inline void SAFE_RELEASE_VECTOR(std::vector<T*>& vec) {
	for (auto& p : vec) {
		if (p) {
			p->Release();
			p = {};
		}
	}
	vec.clear();
}

inline std::wstring utf8ToWstr(const std::string& str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	if (1 >= len)
		return L"";
	std::wstring wstr(len - 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
	return wstr;
}

inline std::wstring mbToWstr(const std::string& str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	if (1 >= len)
		return  L"";
	std::wstring wstr(len - 1, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], len);
	return wstr;
}

ID3DBlob* DXCompileShaderFromFile(const std::string& fileName, const std::string& shaderModel, const std::string& entryPoint, const void* macros = {});
//HRESULT		DXCreateDDSTextureFromFile(ID3D12Device* device , ID3D12GraphicsCommandList* cmdList, const std::string& szFileName , ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& uploadHeap);

} // namespace G2

#endif // _G2_Util_H_
