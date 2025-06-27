
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
// resource vertex buffer

D3D12_VERTEX_BUFFER_VIEW ResBufVtx::VertexBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW ret;
	ret.BufferLocation  = gpu ? gpu->GetGPUVirtualAddress() : 0;
	ret.StrideInBytes   = stride;
	ret.SizeInBytes     = size;
	return ret;
}

int ResBufVtx::UpdateVtx(const void* buf_ptr, size_t buf_size, size_t vtx_stride, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	int hr = S_OK;
	if (!buf_ptr || buf_size == 0)
		return E_INVALIDARG;

	cpb.Reset();
	hr = D3DCreateBlob(buf_size, &cpb);
	if (FAILED(hr))
		return hr;

	memcpy(cpb->GetBufferPointer(), buf_ptr, buf_size);
	size   = (UINT)buf_size;
	stride = (UINT)vtx_stride;

	gpu.Reset();
	upLoader.Reset();

	// Create the actual default buffer resource.
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)
		, D3D12_HEAP_FLAG_NONE
		, &CD3DX12_RESOURCE_DESC::Buffer(size)
		, D3D12_RESOURCE_STATE_COMMON
		, nullptr
		, IID_PPV_ARGS(gpu.GetAddressOf()));

	// In order to copy CPU memory data into our default buffer, we need to create an intermediate upload heap. 
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(upLoader.GetAddressOf()));
	if (FAILED(hr))
	{
		cpb.Reset();
		gpu.Reset();
		return hr;
	}

	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {buf_ptr, size, size};
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, gpu.Get(), upLoader.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return S_OK;
}

int ResBufVtx::UpdateVtx1(const void* buf_ptr, size_t buf_size, size_t vtx_stride, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	int hr = S_OK;
	if (!buf_ptr || buf_size == 0)
		return E_INVALIDARG;

	cpb.Reset();
	hr = D3DCreateBlob(buf_size, &cpb);
	if (FAILED(hr))
		return hr;

	memcpy(cpb->GetBufferPointer(), buf_ptr, buf_size);  // CPU 메모리에 보관
	size = (UINT)buf_size;
	stride = (UINT)vtx_stride;

	gpu.Reset();
	upLoader.Reset();

	// Create the actual default buffer resource.
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(gpu.GetAddressOf()));
	if (FAILED(hr))
		return hr;

	// In order to copy CPU memory data into our default buffer, we need to create an intermediate upload heap. 
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(upLoader.GetAddressOf()));
	if (FAILED(hr))
		return hr;

	 // CPU 보관된 데이터에서 복사
	D3D12_SUBRESOURCE_DATA subResourceData = { cpb->GetBufferPointer(), size, size };

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, gpu.Get(), upLoader.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return S_OK;
}

int ResBufVtx::UpdateVtx2(const void* buf_ptr, size_t buf_size, size_t vtx_stride, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	int hr = S_OK;
	if (!buf_ptr || buf_size == 0)
		return E_INVALIDARG;

	// cpb 제거
	size = (UINT)buf_size;
	stride = (UINT)vtx_stride;

	cpb.Reset();
	gpu.Reset();
	upLoader.Reset();

	// Create the actual default buffer resource.
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(gpu.GetAddressOf()));
	if (FAILED(hr)) return hr;

	// In order to copy CPU memory data into our default buffer, we need to create an intermediate upload heap. 
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(upLoader.GetAddressOf()));
	if (FAILED(hr)) return hr;

	 // 원본에서 바로 복사
	D3D12_SUBRESOURCE_DATA subResourceData = {buf_ptr, size, size };

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, gpu.Get(), upLoader.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return S_OK;
}





} // namespace G2
