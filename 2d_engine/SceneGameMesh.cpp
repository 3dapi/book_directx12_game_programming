//***************************************************************************************
// SceneGameMesh.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <d3d12.h>
#include "SceneGameMesh.h"
#include "Common/DDSTextureLoader.h"
#include "Common/G2.FactoryTexture.h"
#include "Common/G2.FactoryShader.h"
#include "Common/G2.FactorySIgnature.h"
#include "Common/G2.FactoryPipelineState.h"
#include "Common/G2.Util.h"
#include "Common/GeometryGenerator.h"
#include "SceneGameMesh.h"
#include "FrameResource.h"

using namespace G2;

SceneGameMesh::SceneGameMesh()
{
}

SceneGameMesh::~SceneGameMesh()
{
}

int SceneGameMesh::Init(const std::any& initialValue /* = */)
{
	auto d3d         = IG2GraphicsD3D::instance();
	auto d3dDevice   = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto aspectRatio = *std::any_cast<float*>(d3d->getAttrib(ATT_ASPECTRATIO));

	// 1. load texture
	auto tex_manager = FactoryTexture::instance();
	tex_manager->Load("fenceTex", "Textures/WireFence.dds");

	BuildFrameResources();
	BuildBox();

	return S_OK;
}

int SceneGameMesh::Destroy()
{
	return S_OK;
}

int SceneGameMesh::Update(const std::any& t)
{
	UpdateFrameResource();
	GameTimer gt = std::any_cast<GameTimer>(t);
	return S_OK;
}

int SceneGameMesh::Render()
{
	int hr = S_OK;

	// Box 그리기
	DrawBox();

	return S_OK;
}

void SceneGameMesh::BuildFrameResources()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());

	int frameReouseNum = d3dUtil::getFrameRscCount();
	for (int i = 0; i < frameReouseNum; ++i)
	{
		m_frameRscLst.push_back(std::make_unique<FrameResource>(d3dDevice, 1, 1, 1));
	}
}

int SceneGameMesh::UpdateFrameResource()
{
	auto d3d = IG2GraphicsD3D::instance();
	int hr = S_OK;
	// Cycle through the circular frame resource array.
	auto rscCount = d3dUtil::getFrameRscCount();
	m_frameRscIdx = (m_frameRscIdx + 1) % rscCount;
	m_frameRscCur = m_frameRscLst[m_frameRscIdx].get();

	return S_OK;
}

void SceneGameMesh::BuildBox()
{
	auto d3d = IG2GraphicsD3D::instance();
	auto d3dDevice       = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());
	auto cmdList         = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());
	auto srvDescHeapSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_wireBox = new RenderItem;

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

	m_wireBox->vtx.Init(vertices.data(), vbByteSize, sizeof(G2::VTX_NT), d3dDevice, cmdList);
	m_wireBox->idx.Init(indices.data(),  ibByteSize, DXGI_FORMAT_R16_UINT, d3dDevice, cmdList);


	// constant values
	//--------------------------------------------------------------------------
	m_wireBox->Mat = new Material;
	m_wireBox->Mat->matConst.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	XMStoreFloat4x4(&m_wireBox->m_World, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	m_wireBox->primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// setup srv decriptor
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	// 텍스처 갯수 : 10개의 경우
	srvHeapDesc.NumDescriptors = 10;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_wireBox->srvDesc)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_wireBox->srvDesc->GetCPUDescriptorHandleForHeapStart());
	auto texture_factory = FactoryTexture::instance();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto fenceTex = texture_factory->FindRes("fenceTex");
	srvDesc.Format = fenceTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 3;
	// 1장이면 안써도 됨
	hDescriptor.Offset(0, srvDescHeapSize);
	d3dDevice->CreateShaderResourceView(fenceTex, &srvDesc, hDescriptor);
}

void SceneGameMesh::UpdateBox(const GameTimer& gt)
{
	auto currObjectCB = m_frameRscCur->m_cnstTranform.get();

	//constant world transform
	{
		XMMATRIX world = XMLoadFloat4x4(&m_wireBox->m_World);
		ShaderConstTransform objConstants;
		XMStoreFloat4x4(&objConstants.tmWorld, XMMatrixTranspose(world));

		currObjectCB->CopyData(0, objConstants);
	}

	// constant pass
	auto currPassCB = m_frameRscCur->m_cnstPass.get();
	{
		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);

		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&m_cnstPass.tmView, XMMatrixTranspose(view));
		XMStoreFloat4x4(&m_cnstPass.tmProj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&m_cnstPass.tmViewProj, XMMatrixTranspose(viewProj));

		currPassCB->CopyData(0, m_cnstPass);
	}

	// const material
	auto currMaterialCB = m_frameRscCur->m_cnsgbMaterial.get();
	{
		XMMATRIX matTransform = XMLoadFloat4x4(&m_wireBox->Mat->matConst.tmTexCoord);

		ShaderConstMaterial matConstants;
		matConstants.DiffuseAlbedo = m_wireBox->Mat->matConst.DiffuseAlbedo;
		XMStoreFloat4x4(&matConstants.tmTexCoord, XMMatrixTranspose(matTransform));

		currMaterialCB->CopyData(0, matConstants);
	}
}

void SceneGameMesh::DrawBox()
{
	auto d3d = IG2GraphicsD3D::instance();
	auto d3dDevice       = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());
	auto cmdList         = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());
	auto srvDescHeapSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	FrameResource* currentRsc = m_frameRscCur;// std::any_cast<FrameResource*>(IG2AppFrame::instance()->getAttrib(EAPP_ATT_CUR_CB));

	auto pls_manager = FactoryPipelineState::instance();
	auto signature = FactorySignature::instance()->FindRes("TEX_1");
	cmdList->SetGraphicsRootSignature(signature);

	// shader reource view desciptor 설정: texture 정보
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_wireBox->srvDesc.Get() };
	cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto pls = pls_manager->FindRes("PLS_ALPHATEST");
	cmdList->SetPipelineState(pls);



	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ShaderConstTransform));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ShaderConstMaterial));

	auto objectCB = currentRsc->m_cnstTranform->Resource();
	auto passCB = currentRsc->m_cnstPass->Resource();
	auto matCB = currentRsc->m_cnsgbMaterial->Resource();


	cmdList->IASetVertexBuffers(0, 1, &m_wireBox->vtx.VertexBufferView());
	cmdList->IASetIndexBuffer(&m_wireBox->idx.IndexBufferView());
	cmdList->IASetPrimitiveTopology(m_wireBox->primitive);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_tex_hp(m_wireBox->srvDesc->GetGPUDescriptorHandleForHeapStart());
	// m_wireBox->srvDesc 에 텍스처가 여러게 바인딩 되었을 때 해당 인덱스
	gpu_tex_hp.Offset(0, srvDescHeapSize);

	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress();

	cmdList->SetGraphicsRootDescriptorTable(0, gpu_tex_hp);
	// t0가 사용중이라 상수 버퍼 b0는 1에서부터 시작
	cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
	cmdList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

	cmdList->DrawIndexedInstanced(m_wireBox->idx.entryCount, 1, 0, 0, 0);
}
