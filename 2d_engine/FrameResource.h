#pragma once

#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/G2.Geometry.h"

struct ShaderConstTransform
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};

struct ShaderConstPass
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
};


// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct FrameResource
{
public:
    
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;

    std::unique_ptr<UploadBuffer<ShaderConstPass>    > m_cnstbPass     = nullptr;
    std::unique_ptr<UploadBuffer<MaterialConstants>> m_cnsgbMaterial = nullptr;
    std::unique_ptr<UploadBuffer<ShaderConstTransform>>   m_cnsgbMObject  = nullptr;
};