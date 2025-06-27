#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, UINT waveVertCount)
{
    m_cnstbPass     = std::make_unique<UploadBuffer<ShaderConstPass>>(device, passCount, true);
    m_cnsgbMaterial = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
    m_cnsgbMObject  = std::make_unique<UploadBuffer<ShaderConstTransform>>(device, objectCount, true);
    m_vtxWaves      = std::make_unique<UploadBuffer<G2::VTX_NT>>(device, waveVertCount, false);
}
