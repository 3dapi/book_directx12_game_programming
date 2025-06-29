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

ID3DBlob* DXCompileShaderFromFile(const std::string& fileName, const std::string& shaderModel, const std::string& entryPoint, const void* macros)
{
	UINT shaderFlags{};
#ifdef _DEBUG
	shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	auto wFileName = ansiToWstr(fileName);
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

std::pair<std::vector<uint8_t>,int> readFileBinary(const std::string& fileName)
{
	size_t fileSize;
	std::vector<uint8_t> ret_b;
	int ret_hr=E_FAIL;
	try
	{
		std::ifstream fs(fileName,std::ios::binary | std::ifstream::in);
		if(!fs.is_open())
			return { {}, (int)(0x80000000 | ERROR_DEV_NOT_EXIST)};
		fs.seekg(0,std::ios_base::end);
		fileSize = (size_t)fs.tellg();
		fs.seekg(0,std::ios_base::beg);
		if(0 < fileSize)
		{
			ret_b.resize(fileSize);
			fs.read((char*)ret_b.data(), fileSize);
		}
		fs.close();
		ret_hr = S_OK;
	}
	catch(const std::ios_base::failure&)
	{
		return { {}, (int)(0x80000000 | ERROR_FT_READ_FAILURE)};
	}
	catch(const std::runtime_error&)
	{
		return { {}, (int)(0x80000000 | ERROR_FT_READ_FAILURE)};
	}
	return std::make_pair(ret_b, ret_hr);
}

ComPtr<ID3DBlob> readFileToBlob(const std::string& fileName)
{
	ComPtr<ID3DBlob> ret_blob;
	auto [buf, hr] = readFileBinary(fileName);
	if (FAILED(hr) || buf.empty())
		return ret_blob;
	hr = D3DCreateBlob((SIZE_T)buf.size(), ret_blob.GetAddressOf());
	if(SUCCEEDED(hr))
	{
		uint8_t* ptr  = (uint8_t*)ret_blob->GetBufferPointer();
		memcpy(ptr, buf.data(), buf.size());
	}
	return ret_blob;
}



DXException::DXException(HRESULT _hr, const std::string& _functionName,const std::string& _fileName,int _lineNumber)
	: errorCode(_hr), functionName(_functionName), fileName(_fileName), lineNumber(_lineNumber)
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

