#pragma once

#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/G2.Geometry.h"


// Stores the resources needed for the CPU to build the command lists
// for a frame.  
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