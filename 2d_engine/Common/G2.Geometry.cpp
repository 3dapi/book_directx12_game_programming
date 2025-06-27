
#include <tuple>
#include <string>
#include "d3dUtil.h"
#include "G2.ConstantsWin.h"
#include "d3dUtil.h"
#include "G2.Util.h"
#include "G2.Geometry.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace G2 {

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

	UINT StaticResBuf::Count()
	{
		UINT ret = size / stride;
		return ret;
	}

	int StaticResBuf::CreateDefaultBufferWithUploader(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	int hr = S_OK;
	if (!device || cmdList == 0)
		return E_INVALIDARG;

	gpu.Reset();
	upLoader.Reset();
	
	// Create the actual default buffer resource.
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cpuData.size()),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(gpu.GetAddressOf()));
	if (FAILED(hr)) return hr;

	// In order to copy CPU memory data into our default buffer, we need to create an intermediate upload heap. 
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cpuData.size()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(upLoader.GetAddressOf()));
	if (FAILED(hr)) return hr;

	 // 원본에서 바로 복사
	D3D12_SUBRESOURCE_DATA subResourceData = {cpuData.data(), (LONG_PTR)cpuData.size(), (LONG_PTR)cpuData.size() };

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, gpu.Get(), upLoader.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return S_OK;
}

// resource vertex buffer

D3D12_VERTEX_BUFFER_VIEW StaticResBufVtx::VertexBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW ret;
	ret.BufferLocation = gpu ? gpu->GetGPUVirtualAddress() : 0;
	ret.StrideInBytes  = stride;
	ret.SizeInBytes    = size;
	return ret;
}

int StaticResBufVtx::Init(const void* buf_ptr, size_t buf_size, size_t vtx_stride, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	int hr = S_OK;
	if (!buf_ptr || buf_size == 0)
	{
		return E_INVALIDARG;
	}
	stride = (UINT)vtx_stride;
	size   = (UINT)buf_size;
	if (!cpuData.empty())
	{
		cpuData.clear();
	}
	cpuData.resize(buf_size);
	uint8_t* src_ptr = (uint8_t*)buf_ptr;
	memcpy(cpuData.data(), buf_ptr, buf_size);
	//std::copy(src_ptr, src_ptr + buf_size, cpuData.begin());
	hr = this->CreateDefaultBufferWithUploader(device, cmdList);
	return hr;
}

// resource index buffer

D3D12_INDEX_BUFFER_VIEW StaticResBufIdx::IndexBufferView() const
{
	D3D12_INDEX_BUFFER_VIEW ret;
	ret.BufferLocation = gpu->GetGPUVirtualAddress();
	ret.Format = idxFormat;
	ret.SizeInBytes = (UINT)cpuData.size();
	return ret;
}

int StaticResBufIdx::Init(const void* buf_ptr, size_t buf_size, size_t idx_stride,	DXGI_FORMAT format, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	int hr = S_OK;
	if (!buf_ptr || buf_size == 0)
	{
		return E_INVALIDARG;
	}
	idxFormat = format;
	size      = (UINT)buf_size;
	stride    = (UINT)idx_stride;
	if (!cpuData.empty())
	{
		cpuData.clear();
	}
	cpuData.resize(buf_size);
	uint8_t* src_ptr = (uint8_t*)buf_ptr;
	memcpy(cpuData.data(), buf_ptr, buf_size);
	//std::copy(src_ptr, src_ptr + buf_size, cpuData.begin());
	hr = this->CreateDefaultBufferWithUploader(device, cmdList);
	return hr;
}

} // namespace G2
