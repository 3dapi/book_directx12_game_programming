#include <d3d12.h>
#include "DDSTextureLoader.h"
#include "Common/D12DDSTextureLoader.h"
#include "Common/G2.Geometry.h"
#include "Common/GameTimer.h"
#include <pix.h>
#include "CommonStates.h"
#include "DirectXHelpers.h"
#include "ResourceUploadBatch.h"
#include "GraphicsMemory.h"

#include "Common/G2.Constants.h"
#include "Common/G2.Geometry.h"
#include "common/G2.ConstantsWin.h"
#include "common/G2.Util.h"
#include "AppCommon.h"
#include "AppCommonXTK.h"
#include "SceneSample2D.h"

using std::any_cast;
using namespace DirectX;

namespace
{
	struct VS_IN
	{
		XMFLOAT2 pos;
		XMFLOAT4 col;
		XMFLOAT2 tex;
	};

	constexpr VS_IN quad[]=
	{
		{{-1.0f, 1.0f}, {1, 1, 1, 1}, {0, 0}},
		{{1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0}},
		{{-1.0f, -1.0f}, {1, 1, 1, 1}, {0, 1}},
		{{1.0f, -1.0f}, {1, 1, 1, 1}, {1, 1}},
	};

	constexpr uint16_t indices[]={0, 1, 2, 2, 1, 3};

	ComPtr<ID3D12Resource>           m_texture;
	ComPtr<ID3D12DescriptorHeap>     m_srvHeap;
	ComPtr<ID3D12Resource>           m_constBuffer;
	ComPtr<ID3D12PipelineState>      m_pipelineState;
	ComPtr<ID3D12RootSignature>      m_rootSignature;
	ComPtr<ID3D12Resource>           m_vertexBuffer;
	ComPtr<ID3D12Resource>           m_indexBuffer;
}

SceneSample2D::SceneSample2D() {}
SceneSample2D::~SceneSample2D() { Destroy(); }

int SceneSample2D::Init(const std::any&)
{
	auto d3d      =IG2GraphicsD3D::instance();
	auto device   =std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto cmdQueue =std::any_cast<ID3D12CommandQueue*>(d3d->getCommandQueue());

	
	// 텍스처 로드
	ResourceUploadBatch upload(device);
	upload.Begin();
	ThrowIfFailed(CreateWICTextureFromFile(device, upload, L"assets/texture/res_checker.png", m_texture.ReleaseAndGetAddressOf()));
	auto texDesc=m_texture->GetDesc();

	// SRV Heap 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvDesc={};
	srvDesc.NumDescriptors=1;
	srvDesc.Type=D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDesc.Flags=D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&m_srvHeap));

	// SRV 생성
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc={};
	viewDesc.Shader4ComponentMapping=D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.Format=texDesc.Format;
	viewDesc.ViewDimension=D3D12_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels=texDesc.MipLevels;
	device->CreateShaderResourceView(m_texture.Get(), &viewDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

	upload.End(cmdQueue).wait();

	// 상수 버퍼 생성
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(256),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constBuffer)
	);

	// 정점 버퍼 생성
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(quad)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)
	);
	void* mapped=nullptr;
	m_vertexBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, quad, sizeof(quad));
	m_vertexBuffer->Unmap(0, nullptr);

	// 인덱스 버퍼 생성
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)
	);
	m_indexBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, indices, sizeof(indices));
	m_indexBuffer->Unmap(0, nullptr);

	// 루트 시그니처 및 PSO 생성
	// (셰이더 컴파일 포함해서, spine.fx를 기반으로 루트 시그니처와 파이프라인 생성하는 코드 필요)

	return S_OK;
}

int SceneSample2D::Destroy()
{
	return S_OK;
}

int SceneSample2D::Update(const std::any&)
{
	auto mvp=XMMatrixIdentity();
	void* mapped=nullptr;
	D3D12_RANGE range={0, 0};
	if(SUCCEEDED(m_constBuffer->Map(0, &range, &mapped))) {
		memcpy(mapped, &mvp, sizeof(mvp));
		m_constBuffer->Unmap(0, nullptr);
	}
	return S_OK;
}

int SceneSample2D::Render()
{
	auto d3d      =  IG2GraphicsD3D::instance();
	auto device   = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto cmdList  = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());

	cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
	cmdList->SetPipelineState(m_pipelineState.Get());

	ID3D12DescriptorHeap* heaps[]={m_srvHeap.Get()};
	cmdList->SetDescriptorHeaps(1, heaps);

	cmdList->SetGraphicsRootConstantBufferView(0, m_constBuffer->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_VERTEX_BUFFER_VIEW vbView={};
	vbView.BufferLocation=m_vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes=sizeof(quad);
	vbView.StrideInBytes=sizeof(VS_IN);
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	D3D12_INDEX_BUFFER_VIEW ibView={};
	ibView.BufferLocation=m_indexBuffer->GetGPUVirtualAddress();
	ibView.SizeInBytes=sizeof(indices);
	ibView.Format=DXGI_FORMAT_R16_UINT;
	cmdList->IASetIndexBuffer(&ibView);

	cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	return S_OK;
}
