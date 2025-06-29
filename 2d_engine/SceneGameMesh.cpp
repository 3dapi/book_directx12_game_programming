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
}

int SceneGameMesh::Init(const std::any&)
{
	auto d3d = IG2GraphicsD3D::instance();
	auto device       = std::any_cast<ID3D12Device*              >(d3d->getDevice());
	float aspectRatio    = *any_cast<float*>(IG2GraphicsD3D::instance()->getAttrib(ATT_ASPECTRATIO));
	 m_wireBox.cbPss.tmProj = XMMatrixPerspectiveFovLH(0.25f * XM_PI, aspectRatio, 1.0f, 1000.0f);

	SetupUploadChain();
	BuildBox();

	auto formatBackBuffer  = *any_cast<DXGI_FORMAT*>(IG2GraphicsD3D::instance()->getAttrib(ATT_DEVICE_BACKBUFFER_FORAT	));
	auto formatDepthBuffer = *any_cast<DXGI_FORMAT*>(IG2GraphicsD3D::instance()->getAttrib(ATT_DEVICE_DEPTH_STENCIL_FORAT));

	const RenderTargetState rtState(formatBackBuffer, formatDepthBuffer);
	{
		EffectPipelineStateDescription pd(
			&VertexPositionColor::InputLayout,
			CommonStates::Opaque,
			CommonStates::DepthNone,
			CommonStates::CullNone,
			rtState,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

		m_lineEffect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, pd);
	}
	return S_OK;
}

int SceneGameMesh::Destroy()
{
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
	UpdateUploadChain();
	int hr = S_OK;
	UpdateCamera(gt);
	UpdateBox(gt);

	m_world = XMMatrixRotationY(gt.TotalTime() * XM_PIDIV4);

	const XMFLOAT3 eye(0.0f, 0.7f, -8.0f);
	const XMFLOAT3 at(0.0f, -0.1f, 0.0f);
	const XMFLOAT3 up(0.0f,  1.0f, 0.0f);
	//m_view = Matrix::CreateLookAt(eye, at, up);
	m_view = XMMatrixLookAtRH(XMLoadFloat3(&eye), XMLoadFloat3(&at), XMLoadFloat3(&up));

	m_lineEffect->SetView(m_view);

	XMMATRIX projLH = m_wireBox.cbPss.tmProj;
	XMMATRIX flipZ = XMMatrixScaling(1.0f, 1.0f, -1.0f); // Z축 반전

	XMMATRIX projRH = flipZ * projLH;

	m_lineEffect->SetProjection(projRH);
	m_lineEffect->SetWorld(XMMatrixIdentity());

	return S_OK;
}

int SceneGameMesh::Render()
{
	int hr = S_OK;
	auto d3d = IG2GraphicsD3D::instance();
	auto d3dDevice       = std::any_cast<ID3D12Device*              >(d3d->getDevice());
	auto d3dCommandList  = std::any_cast<ID3D12GraphicsCommandList* >(d3d->getCommandList());

	// Box 그리기
	DrawBox(d3dCommandList);

	// Draw procedurally generated dynamic grid
	const XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
	const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
	DrawGrid(xaxis, yaxis, g_XMZero, 20, 20, Colors::Gray);

	return S_OK;
}


void SceneGameMesh::UpdateCamera(const GameTimer& t)
{
	GameTimer gt = std::any_cast<GameTimer>(t);
	float aspectRatio = *any_cast<float*>(IG2GraphicsD3D::instance()->getAttrib(ATT_ASPECTRATIO));
	m_wireBox.cbPss.tmProj = XMMatrixPerspectiveFovRH(0.25f * XM_PI, aspectRatio, 1.0f, 5000.0f);

	// Convert Spherical to Cartesian coordinates.
	m_tmEyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
	m_tmEyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
	m_tmEyePos.y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(20, 40, -40, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_wireBox.cbPss.tmView = XMMatrixLookAtRH(pos, target, up);

	//constant world transform
	//auto currObjectCB = m_subCur->m_cnstTranform.get();
	//{
	//	XMMATRIX world = XMLoadFloat4x4(&m_cnstWorld.tmWorld);
	//	ShaderConstTransform objConstants;
	//	XMStoreFloat4x4(&objConstants.tmWorld, XMMatrixTranspose(world));

	//	currObjectCB->CopyData(0, objConstants);
	//}
}

void SceneGameMesh::UpdateBox(const GameTimer& gt)
{
	//constant world transform
	auto currObjectCB = m_subCur->m_cnstTrs.get();
	{
		currObjectCB->CopyData(0, m_wireBox.cbTrs);
	}

	// constant pass
	auto currPassCB = m_subCur->m_cnstPss.get();
	{
		m_wireBox.cbPss.tmViewProj = XMMatrixMultiply(m_wireBox.cbPss.tmView, m_wireBox.cbPss.tmProj);
		currPassCB->CopyData(0, m_wireBox.cbPss);
	}

	// const material
	auto currMaterialCB = m_subCur->m_cnstMtl.get();
	{
		currMaterialCB->CopyData(0, m_wireBox.cbMtl);
	}
}

int SceneGameMesh::UpdateUploadChain()
{
	auto d3d = IG2GraphicsD3D::instance();
	int hr = S_OK;
	m_subIdx = (m_subIdx + 1) % int(EAPP_CONST::EAPP_FRAME_RESOURCE_CHAIN_NUMBER);
	m_subCur = m_subLst[m_subIdx].get();

	return S_OK;
}

void SceneGameMesh::BuildBox()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());
	auto d3dCommandList = std::any_cast<ID3D12GraphicsCommandList*>(IG2GraphicsD3D::instance()->getCommandList());
	UINT srvDescSize    = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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

	m_wireBox.vtx.Init(vertices.data(), vbByteSize, sizeof(G2::VTX_NT), d3dDevice, d3dCommandList);
	m_wireBox.idx.Init(indices.data(),  ibByteSize, DXGI_FORMAT_R16_UINT, d3dDevice, d3dCommandList);


	// constant values
	//--------------------------------------------------------------------------
	m_wireBox.cbMtl.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_wireBox.cbTrs.tmWorld = XMMatrixTranslation(3.0f, 2.0f, -9.0f);
	m_wireBox.primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// setup srv decriptor
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	// 텍스처 갯수 : 10개의 경우
	srvHeapDesc.NumDescriptors = 10;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_wireBox.srvDesc)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_wireBox.srvDesc->GetCPUDescriptorHandleForHeapStart());
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
	d3dDevice->CreateShaderResourceView(fenceTex, &srvDesc, hDescriptor);
}




void SceneGameMesh::SetupUploadChain()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());
	for (int i = 0; i < int(EAPP_CONST::EAPP_FRAME_RESOURCE_CHAIN_NUMBER); ++i)
	{
		m_subLst.push_back(std::make_unique<ShaderUploadChain>(d3dDevice, 1, 1, 1));
	}
}

void SceneGameMesh::DrawBox(ID3D12GraphicsCommandList* cmdList)
{
	auto d3dDevice      = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());
	UINT srvDescSize    = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto pls_manager    = FactoryPipelineState::instance();

	// root signature
	auto signature = FactorySignature::instance()->FindRes("TEX_1");
	cmdList->SetGraphicsRootSignature(signature);


	// shader reource view desciptor 설정: texture 정보
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_wireBox.srvDesc.Get() };
	cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto pls = pls_manager->FindRes("PLS_ALPHATEST");
	cmdList->SetPipelineState(pls);



	UINT objCBByteSize = G2::alignTo256(sizeof(ShaderConstTransform));
	UINT matCBByteSize = G2::alignTo256(sizeof(ShaderConstMaterial));

	auto cbTrs = m_subCur->m_cnstTrs->Resource();
	auto cbPss = m_subCur->m_cnstPss->Resource();
	auto cbMtl = m_subCur->m_cnstMtl->Resource();


	cmdList->IASetVertexBuffers(0, 1, &m_wireBox.vtx.VertexBufferView());
	cmdList->IASetIndexBuffer(&m_wireBox.idx.IndexBufferView());
	cmdList->IASetPrimitiveTopology(m_wireBox.primitive);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_tex_hp(m_wireBox.srvDesc->GetGPUDescriptorHandleForHeapStart());
	// m_wireBox.srvDesc 에 텍스처가 여러게 바인딩 되었을 때 해당 인덱스
	gpu_tex_hp.Offset(0, srvDescSize);

	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = cbTrs->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = cbMtl->GetGPUVirtualAddress();

	cmdList->SetGraphicsRootDescriptorTable(0, gpu_tex_hp);
	// t0가 사용중이라 상수 버퍼 b0는 1에서부터 시작
	cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
	cmdList->SetGraphicsRootConstantBufferView(2, cbPss->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

	cmdList->DrawIndexedInstanced(m_wireBox.idx.entryCount, 1, 0, 0, 0);
}

void XM_CALLCONV SceneGameMesh::DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color)
{
	auto d3d = IG2GraphicsD3D::instance();
	auto d3dDevice    = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto commandList  = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());
	auto pBatch       = std::any_cast<XTK_BATCH* >(IG2AppFrame::instance()->getAttrib(EAPP_ATTRIB::EAPP_ATT_XTK_BATCH));

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Draw grid");

	m_lineEffect->Apply(commandList);

	pBatch->Begin(commandList);

	xdivs = std::max<size_t>(1, xdivs);
	ydivs = std::max<size_t>(1, ydivs);

	for (size_t i = 0; i <= xdivs; ++i)
	{
		float fPercent = float(i) / float(xdivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
		vScale = XMVectorAdd(vScale, origin);

		const VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
		const VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
		pBatch->DrawLine(v1, v2);
	}

	for (size_t i = 0; i <= ydivs; i++)
	{
		float fPercent = float(i) / float(ydivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
		vScale = XMVectorAdd(vScale, origin);

		const VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
		const VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
		pBatch->DrawLine(v1, v2);
	}

	pBatch->End();

	PIXEndEvent(commandList);
}
