#include "ShaderUploadChain.h"

ShaderUploadChain::ShaderUploadChain(ID3D12Device* device,UINT cbTrsCount, UINT cbPssCount, UINT cbMtlCount)
{
    m_cnstTrs   = std::make_unique<UploadBuffer<ShaderConstTransform    > >(device, cbTrsCount, true);
    m_cnstPss   = std::make_unique<UploadBuffer<ShaderConstPass         > >(device, cbPssCount, true);
    m_cnstMtl   = std::make_unique<UploadBuffer<ShaderConstMaterial     > >(device, cbMtlCount, true);
}
