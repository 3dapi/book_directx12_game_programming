#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, UINT waveVertCount)
{
    ThrowIfFailed(device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandListAlloc.GetAddressOf())));

  //  FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, 1, true);
    cnstbPass     = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
    cnsgbMaterial = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
    cnsgbMObject  = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);

    WavesVB = std::make_unique<UploadBuffer<Vertex>>(device, waveVertCount, false);
}

FrameResource::~FrameResource()
{

}