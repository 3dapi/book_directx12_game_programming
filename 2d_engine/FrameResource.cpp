#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, UINT waveVertCount)
{
    m_cnstbPass     = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
    m_cnsgbMaterial = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
    m_cnsgbMObject  = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
    m_vtxWaves      = std::make_unique<UploadBuffer<Vertex>>(device, waveVertCount, false);
}

FrameResource::~FrameResource()
{

}