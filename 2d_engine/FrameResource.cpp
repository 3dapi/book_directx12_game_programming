#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
    m_cnstTranform  = std::make_unique<UploadBuffer<ShaderConstTransform>>(device, objectCount, true);
    m_cnstPass     = std::make_unique<UploadBuffer<ShaderConstPass>>(device, passCount, true);
    m_cnsgbMaterial = std::make_unique<UploadBuffer<ShaderConstMaterial>>(device, materialCount, true);
}
