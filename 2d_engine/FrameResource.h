#pragma once

#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/G2.Geometry.h"


// Stores the resources needed for the CPU to build the command lists
// for a frame.  

struct ShaderConstTransform
{
	DirectX::XMFLOAT4X4 tmWorld = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 tmTex = MathHelper::Identity4x4();
};


struct ShaderConstPass
{
	DirectX::XMFLOAT4X4 tmView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 tmProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 tmViewProj = MathHelper::Identity4x4();
};

struct ShaderConstMaterial
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT4X4 tmTexCoord = MathHelper::Identity4x4();
};


struct FrameResource
{
public:
    
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;

    std::unique_ptr<UploadBuffer<ShaderConstTransform>> m_cnstTranform  = nullptr;
    std::unique_ptr<UploadBuffer<ShaderConstPass>     > m_cnstPass     = nullptr;
    std::unique_ptr<UploadBuffer<ShaderConstMaterial> > m_cnsgbMaterial = nullptr;
};


// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.
struct Material
{
	ShaderConstMaterial   matConst;
	int NumFramesDirty = d3dUtil::getFrameRscCount();
};

