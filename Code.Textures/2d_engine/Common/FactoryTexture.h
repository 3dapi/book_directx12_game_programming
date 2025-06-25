
#pragma once
#ifndef __FACTORY_TEXTURE_H__
#define __FACTORY_TEXTURE_H__

#include <algorithm>
#include <any>
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include "d3dUtil.h"
#include "d3dx12/d3dx12.h"
#include "DDSTextureLoader.h"
#include "MathHelper.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DirectX::PackedVector;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template <typename T>
class IG2Factory
{
protected:
	std::unordered_map<std::string, std::unique_ptr<T>> m_db;
public:
	T* Load(const std::any& optional)
	{
		return ResourceLoad(optional);
	}
	T* Find(const std::string& name)
	{
		return ResourceFind(name);
	}
	int UnLoad(const std::string& name)
	{
		return ResourceUnLoad(name);
	}
	int UnLoadAll()
	{
		return ResourceUnLoadAll();
	}
protected:
	virtual T* ResourceLoad(const std::any& optional)   = 0;
	virtual T* ResourceFind(const std::string& name)    = 0;
	virtual int ResourceUnLoad(const std::string& name) = 0;
	virtual int ResourceUnLoadAll()                     = 0;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

struct Texture
{
	std::string name;
	std::string file;
	ComPtr<ID3D12Resource>	rs{};	// resource
	ComPtr<ID3D12Resource>	uh{};	// upload heap
};

class FactoryTexture : public IG2Factory<Texture>
{
public:
	static FactoryTexture* instance();
protected:
	Texture* ResourceLoad(const std::any& optional)  override;
	Texture* ResourceFind(const std::string& name)   override;
	int ResourceUnLoad(const std::string& name)      override;
	int ResourceUnLoadAll()                          override;
};

#endif // __FACTORY_TEXTURE_H__
