
#pragma once

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
#include "Common/d3dUtil.h"
#include "Common/d3dx12.h"
#include "Common/DDSTextureLoader.h"
#include "Common/MathHelper.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct IG2Factory
{
	virtual int Load(const std::string& name, std::any optional = {}) = 0;
};

struct Texture
{
	std::string Name;
	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

struct FactoryTexture : public IG2Factory
{
protected:
	std::unordered_map<std::string, std::unique_ptr<Texture> > m_texture;
public:
	virtual ~FactoryTexture();
	virtual int Load(const std::string& name, std::any optional = {});
};
