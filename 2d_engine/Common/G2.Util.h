
#pragma once
#ifndef _G2_Util_H_
#define _G2_Util_H_

#include <cstdarg>
#include <cstdio>
#include <string>
#include <tuple>
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

inline std::wstring ansiToWstr(const std::string& str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	if (1 >= len)
		return  L"";
	std::wstring wstr(len - 1, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], len);
	return wstr;
}

inline void debugToOutputWindow(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	va_list args_copy;
	va_copy(args_copy, args);
	int length = std::vsnprintf(nullptr, 0, format, args_copy);
	va_end(args_copy);

	if (length > 0) {
		++length;		// for null char
		std::string buffer(length, 0);
		std::vsnprintf(&buffer[0], length, format, args);
		OutputDebugStringA(buffer.c_str());
	}

	va_end(args);
}

inline UINT alignTo256(UINT byteSize)
{
	// Constant buffers must be a multiple of the minimum hardware
	// allocation size (usually 256 bytes).  So round up to nearest
	// multiple of 256.  We do this by adding 255 and then masking off
	// the lower 2 bytes which store all bits < 256.
	// Example: Suppose byteSize = 300.
	// (300 + 255) & ~255
	// 555 & ~255
	// 0x022B & ~0x00ff
	// 0x022B & 0xff00
	// 0x0200
	// 512
	return (byteSize + 255) & ~255;
}

ID3DBlob* DXCompileShaderFromFile(const std::string& fileName, const std::string& shaderModel, const std::string& entryPoint, const void* macros = {});
//HRESULT		DXCreateDDSTextureFromFile(ID3D12Device* device , ID3D12GraphicsCommandList* cmdList, const std::string& szFileName , ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& uploadHeap);

std::pair<std::vector<uint8_t>, int>	// buffer, result
readFileBinary(const std::string& fileName);
ComPtr<ID3DBlob> readFileToBlob(const std::string& fileName);

class DXException
{
public:
	DXException() = default;
	DXException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber);
	std::wstring ToString()const;

	HRESULT     errorCode = S_OK;
	std::string functionName;
	std::string fileName;
	int         lineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x) {                              \
    if(FAILED(x)) {                                     \
		throw DXException(x, __func__ , __FILE__, __LINE__);  \
	}                                                   \
}
#endif


} // namespace G2

#endif // _G2_Util_H_
