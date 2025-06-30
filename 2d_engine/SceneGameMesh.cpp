//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// SceneGameMesh

#include <d3d12.h>
#include "DDSTextureLoader.h"
#include "Common/D12DDSTextureLoader.h"
#include "Common/G2.FactoryTexture.h"
#include "Common/G2.FactoryShader.h"
#include "Common/G2.FactorySIgnature.h"
#include "Common/G2.FactoryPipelineState.h"
#include "Common/G2.Geometry.h"
#include "Common/G2.Util.h"
#include "Common/GeometryGenerator.h"
#include "SceneGameMesh.h"

#include <pix.h>
#include "CommonStates.h"

using std::any_cast;

SceneGameMesh::SceneGameMesh() noexcept
{
}

SceneGameMesh::~SceneGameMesh()
{
	Destroy();
}

int SceneGameMesh::Init(const std::any&)
{
	auto d3d = IG2GraphicsD3D::instance();
	auto device       = std::any_cast<ID3D12Device*              >(d3d->getDevice());
	float aspectRatio = *any_cast<float*>(IG2GraphicsD3D::instance()->getAttrib(ATT_ASPECTRATIO));
	m_boxCbPss.tmProj = XMMatrixPerspectiveFovLH(0.25f * XM_PI, aspectRatio, 1.0f, 1000.0f);

	d3d->command(CMD_COMMAND_BEGIN);
	m_cbUploader = std::make_unique<ShaderUploadChain>(device, 1, 1, 1);
	BuildBox();
	d3d->command(CMD_COMMAND_END);
	return S_OK;
}

int SceneGameMesh::Destroy()
{
	m_boxVtx.~StaticResBufVtx();
	m_boxIdx.~StaticResBufIdx();
	FactoryPipelineState::instance()->UnLoadAll();
	FactorySignature::instance()->UnLoadAll();
	FactoryShader::instance()->UnLoadAll();
	FactoryTexture::instance()->UnLoadAll();
	return S_OK;
}

int SceneGameMesh::Update(const std::any& t)
{
	GameTimer gt = std::any_cast<GameTimer>(t);
	// Cycle through the circular frame resource array.

	int hr = S_OK;
	UpdateCamera(gt);
	UpdateBox(gt);

	m_world = XMMatrixRotationY(gt.TotalTime() * XM_PIDIV4);

	const XMFLOAT3 eye(0.0f, 0.7f, -8.0f);
	const XMFLOAT3 at(0.0f, -0.1f, 0.0f);
	const XMFLOAT3 up(0.0f,  1.0f, 0.0f);
	//m_view = Matrix::CreateLookAt(eye, at, up);
	m_view = XMMatrixLookAtRH(XMLoadFloat3(&eye), XMLoadFloat3(&at), XMLoadFloat3(&up));

	return S_OK;
}

int SceneGameMesh::Render()
{
	// Box 그리기
	DrawBox();

	return S_OK;
}


void SceneGameMesh::UpdateCamera(const GameTimer& t)
{
	GameTimer gt = std::any_cast<GameTimer>(t);
	float aspectRatio = *any_cast<float*>(IG2GraphicsD3D::instance()->getAttrib(ATT_ASPECTRATIO));
	m_boxCbPss.tmProj = XMMatrixPerspectiveFovRH(0.25f * XM_PI, aspectRatio, 1.0f, 5000.0f);

	// Convert Spherical to Cartesian coordinates.
	m_tmEyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
	m_tmEyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
	m_tmEyePos.y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(20, 40, -40, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_boxCbPss.tmView = XMMatrixLookAtRH(pos, target, up);
}

void SceneGameMesh::UpdateBox(const GameTimer& gt)
{
	//constant world transform
	auto currObjectCB = m_cbUploader->m_cnstTrs.get();
	{
		currObjectCB->CopyData(0, m_boxCbTrs);
	}

	// constant pass
	auto currPassCB = m_cbUploader->m_cnstPss.get();
	{
		m_boxCbPss.tmViewProj = XMMatrixMultiply(m_boxCbPss.tmView, m_boxCbPss.tmProj);
		currPassCB->CopyData(0, m_boxCbPss);
	}

	// const material
	auto currMaterialCB = m_cbUploader->m_cnstMtl.get();
	{
		currMaterialCB->CopyData(0, m_boxCbMtl);
	}
}

void SceneGameMesh::BuildBox()
{
	int hr = 0;
	auto d3d = IG2GraphicsD3D::instance();
	auto device   = std::any_cast<ID3D12Device*              >(d3d->getDevice());
	auto cmdList  = std::any_cast<ID3D12GraphicsCommandList* >(d3d->getCommandList());
	UINT srvDescSize     = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);

	std::vector<G2::VTX_NT> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].p;
		vertices[i].p = p;
		vertices[i].n = box.Vertices[i].n;
		vertices[i].t = box.Vertices[i].t;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(G2::VTX_NT);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	m_boxVtx.Init(vertices.data(), vbByteSize, sizeof(G2::VTX_NT), device, cmdList);
	m_boxIdx.Init(indices.data(),  ibByteSize, DXGI_FORMAT_R16_UINT, device, cmdList);


	// constant values
	//--------------------------------------------------------------------------
	m_boxCbMtl.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_boxCbTrs.tmWorld = XMMatrixTranslation(3.0f, 2.0f, -9.0f);
	m_boxPrimitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// setup srv decriptor
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	// 텍스처 갯수 : 10개의 경우
	srvHeapDesc.NumDescriptors = 10;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_boxSrvDesc)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_boxSrvDesc->GetCPUDescriptorHandleForHeapStart());
	auto texture_factory = FactoryTexture::instance();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto fenceTex = texture_factory->FindRes("fenceTex");
	srvDesc.Format = fenceTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 3;
	// 1장이면 안써도 됨
	hDescriptor.Offset(0, srvDescSize);
	device->CreateShaderResourceView(fenceTex, &srvDesc, hDescriptor);

	return;
}

void SceneGameMesh::DrawBox()
{
	int hr = S_OK;
	auto d3d        = IG2GraphicsD3D::instance();
	auto device     = std::any_cast<ID3D12Device*              >(d3d->getDevice());
	auto cmdList    = std::any_cast<ID3D12GraphicsCommandList* >(d3d->getCommandList());

	UINT srvDescSize    = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto pls_manager    = FactoryPipelineState::instance();

	// root signature
	auto signature = FactorySignature::instance()->FindRes("TEX_1");
	cmdList->SetGraphicsRootSignature(signature);


	// shader reource view desciptor 설정: texture 정보
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_boxSrvDesc.Get() };
	cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto pls = pls_manager->FindRes("PLS_ALPHATEST");
	cmdList->SetPipelineState(pls);



	UINT objCBByteSize = G2::alignTo256(sizeof(ShaderConstTransform));
	UINT matCBByteSize = G2::alignTo256(sizeof(ShaderConstMaterial));

	auto cbTrs = m_cbUploader->m_cnstTrs->Resource();
	auto cbPss = m_cbUploader->m_cnstPss->Resource();
	auto cbMtl = m_cbUploader->m_cnstMtl->Resource();


	cmdList->IASetVertexBuffers(0, 1, &m_boxVtx.VertexBufferView());
	cmdList->IASetIndexBuffer(&m_boxIdx.IndexBufferView());
	cmdList->IASetPrimitiveTopology(m_boxPrimitive);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_tex_hp(m_boxSrvDesc->GetGPUDescriptorHandleForHeapStart());
	// srvDesc 에 텍스처가 여러게 바인딩 되었을 때 해당 인덱스
	gpu_tex_hp.Offset(0, srvDescSize);

	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = cbTrs->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = cbMtl->GetGPUVirtualAddress();

	cmdList->SetGraphicsRootDescriptorTable(0, gpu_tex_hp);
	// t0가 사용중이라 상수 버퍼 b0는 1에서부터 시작
	cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
	cmdList->SetGraphicsRootConstantBufferView(2, cbPss->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

	cmdList->DrawIndexedInstanced(m_boxIdx.entryCount, 1, 0, 0, 0);
}
