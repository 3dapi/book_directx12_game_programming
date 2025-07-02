// for avx2 memcpy
#include <immintrin.h>
#include <cstdint>
#include <cstring>

#include <comdef.h>

#include <any>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <memory>
#include <tuple>
#include <Windows.h>
#include <winerror.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "G2.Util.h"

namespace G2 {

void avx2_memcpy(void* dst, const void* src, size_t size)
{
	// 작은 데이터는 memcpy로 처리 (성능 유리)
	if(size < 1024)
	{
		std::memcpy(dst,src,size);
		return;
	}

	uint8_t* d = reinterpret_cast<uint8_t*>(dst);
	const uint8_t* s = reinterpret_cast<const uint8_t*>(src);

	size_t simdSize = size & ~31ULL;  // 32바이트 배수로 정렬
	size_t remain   = size &  31ULL;  // 나머지 바이트

	// --- AVX2 복사 (32바이트 단위) ---
	for(size_t i = 0; i < simdSize; i += 32)
	{
		__m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(s + i));
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(d + i),v);
	}

	// --- 남은 바이트는 memcpy로 처리 ---
	if(remain > 0)
	{
		std::memcpy(d + simdSize,s + simdSize,remain);
	}
}

void avx2_memset32(void* dst, int32_t val, size_t count)
{
	int32_t* d = reinterpret_cast<int32_t*>(dst);

	if(count * sizeof(int32_t) < 1024)
	{
		for(size_t i = 0; i < count; ++i)
			d[i] = val;
		return;
	}

	size_t simdCount = count & ~7ULL;  // 8개씩 처리 (32B)
	size_t remain    = count & 7ULL;

	__m256i vec = _mm256_set1_epi32(val);  // 8 x int32_t

	for(size_t i = 0; i < simdCount; i += 8)
	{
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(d + i),vec);
	}

	for(size_t i = simdCount; i < count; ++i)
	{
		d[i] = val;
	}
}

ID3DBlob* DXCompileShaderFromFile(const std::string& fileName, const std::string& shaderModel, const std::string& entryPoint, const void* macros)
{
	UINT shaderFlags{};
#ifdef _DEBUG
	shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	auto wFileName = ansiToWstr(fileName);
	ComPtr<ID3DBlob> pErrorBlob{};
	ID3DBlob* pBlobRet{};
	int hr = D3DCompileFromFile(wFileName.c_str(),(const D3D_SHADER_MACRO*)macros,D3D_COMPILE_STANDARD_FILE_INCLUDE,entryPoint.c_str(),shaderModel.c_str(),shaderFlags,0,&pBlobRet,&pErrorBlob);
	if(FAILED(hr))
	{
		if(pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
		return {};
	}
	return pBlobRet;
}

std::pair<std::vector<uint8_t>,int> readFileBinary(const std::string& fileName)
{
	size_t fileSize;
	std::vector<uint8_t> ret_b;
	int ret_hr=E_FAIL;
	try
	{
		std::ifstream fs(fileName,std::ios::binary | std::ifstream::in);
		if(!fs.is_open())
			return {{},(int)(0x80000000 | ERROR_DEV_NOT_EXIST)};
		fs.seekg(0,std::ios_base::end);
		fileSize = (size_t)fs.tellg();
		fs.seekg(0,std::ios_base::beg);
		if(0 < fileSize)
		{
			ret_b.resize(fileSize);
			fs.read((char*)ret_b.data(),fileSize);
		}
		fs.close();
		ret_hr = S_OK;
	}
	catch(const std::ios_base::failure&)
	{
		return {{},(int)(0x80000000 | ERROR_FT_READ_FAILURE)};
	}
	catch(const std::runtime_error&)
	{
		return {{},(int)(0x80000000 | ERROR_FT_READ_FAILURE)};
	}
	return std::make_pair(ret_b,ret_hr);
}

ComPtr<ID3DBlob> readFileToBlob(const std::string& fileName)
{
	ComPtr<ID3DBlob> ret_blob;
	auto [buf,hr] = readFileBinary(fileName);
	if(FAILED(hr) || buf.empty())
		return ret_blob;
	hr = D3DCreateBlob((SIZE_T)buf.size(),ret_blob.GetAddressOf());
	if(SUCCEEDED(hr))
	{
		uint8_t* ptr  = (uint8_t*)ret_blob->GetBufferPointer();
		memcpy(ptr,buf.data(),buf.size());
	}
	return ret_blob;
}



DXException::DXException(HRESULT _hr,const std::string& _functionName,const std::string& _fileName,int _lineNumber)
	: errorCode(_hr),functionName(_functionName),fileName(_fileName),lineNumber(_lineNumber)
{
}
std::wstring DXException::ToString()const
{
	_com_error err(errorCode);
	std::wstring msg = err.ErrorMessage();
	auto file = ansiToWstr(fileName);
	auto method = ansiToWstr(functionName);
	auto line   = std::to_wstring(lineNumber);
	return method + L" failed. " + file + L": - " + line + L", err: " + msg;
}

} // namespace G2

