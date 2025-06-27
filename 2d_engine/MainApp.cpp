//***************************************************************************************
// MainApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <d3d12.h>
#include "MainApp.h"
#include "Common/DDSTextureLoader.h"
#include "Common/G2.FactoryTexture.h"
#include "Common/G2.FactoryShader.h"
#include "Common/G2.FactorySIgnature.h"
#include "Common/G2.FactoryPipelineState.h"
#include "Common/G2.Geometry.h"
#include "Common/G2.Util.h"

using namespace G2;

static MainApp* g_pMain{};

G2::IG2AppFrame* G2::IG2AppFrame::instance()
{
	if (!g_pMain)
	{
		g_pMain = new MainApp;
	}
	return g_pMain;
}

MainApp::MainApp()
{
	d3dUtil::setFrameReourceNumer(2);
}

MainApp::~MainApp()
{
}

int MainApp::init(const std::any& initialValue /* = */)
{
	int hr = D3DWinApp::init(initialValue);
	if (FAILED(hr))
		return hr;

	auto d3d = IG2GraphicsD3D::instance();
	auto d3dDevice       = std::any_cast<ID3D12Device*              >(d3d->getDevice());
	auto d3dCommandList  = std::any_cast<ID3D12GraphicsCommandList* >(d3d->getCommandList());
	auto d3dCommandAlloc = std::any_cast<ID3D12CommandAllocator*    >(d3d->getCommandAllocator());
	auto d3dCommandQue   = std::any_cast<ID3D12CommandQueue*        >(d3d->getCommandQueue());

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(d3dCommandList->Reset(d3dCommandAlloc, nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	mCbvSrvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// 1. load texutre
	auto tex_manager = FactoryTexture::instance();
	tex_manager->Load("grassTex", "Textures/grass.dds");
	tex_manager->Load("fenceTex", "Textures/WireFence.dds");

	//2 
	BuildDescriptorHeaps();

	// 3. Build Shaders And Input Layouts
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	auto shader_manager = FactoryShader::instance();
	shader_manager->Load("standardVS"   , "Shaders/Default.hlsl"   , "vs_5_0", "VS"                  );
	shader_manager->Load("opaquePS"     , "Shaders/Default.hlsl"   , "ps_5_0", "PS", defines         );
	shader_manager->Load("alphaTestedPS", "Shaders/Default.hlsl"   , "ps_5_0", "PS", alphaTestDefines);

	
	BuildLandGeometry();
	BuildBoxGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	auto pls_manager = FactoryPipelineState::instance();

	// Execute the initialization commands.
	ThrowIfFailed(d3dCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { d3dCommandList };
	d3dCommandQue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	hr = d3d->command(CMD_FLUSH_COMMAND_QUEUE);

	return S_OK;
}

int MainApp::destroy()
{
	FactoryPipelineState::instance()->UnLoadAll();
	FactorySignature::instance()->UnLoadAll();
	FactoryShader::instance()->UnLoadAll();
	FactoryTexture::instance()->UnLoadAll();
	return S_OK;
}

int MainApp::Resize(bool up)
{
	int hr = D3DWinApp::Resize(up);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
	return S_OK;
}

int MainApp::Update(const std::any& t)
{
	int hr = S_OK;
	GameTimer gt = std::any_cast<GameTimer>(t);
	OnKeyboardInput(gt);
	UpdateCamera(gt);

	// Cycle through the circular frame resource array.
	UpdateFrameResource();

	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);

	return S_OK;
}

int MainApp::Render()
{
	int hr = S_OK;
	auto d3d = IG2GraphicsD3D::instance();
	auto d3dDevice       = std::any_cast<ID3D12Device*              >(d3d->getDevice());
	auto d3dCommandList  = std::any_cast<ID3D12GraphicsCommandList* >(d3d->getCommandList());
	auto d3dCommandAlloc = std::any_cast<ID3D12CommandAllocator*    >(d3d->getCommandAllocator());
	auto d3dCommandQue   = std::any_cast<ID3D12CommandQueue*        >(d3d->getCommandQueue());
	auto d3dViewport     = std::any_cast<D3D12_VIEWPORT*            >(d3d->getAttrib(ATT_DEVICE_VIEWPORT));
	auto d3dScissor      = std::any_cast<D3D12_RECT*                >(d3d->getAttrib(ATT_DEVICE_SCISSOR_RECT));
	auto d3dBackBuffer   = std::any_cast<ID3D12Resource*            >(d3d->getCurrentBackBuffer());
	auto d3dBackBufferV  = std::any_cast<CD3DX12_CPU_DESCRIPTOR_HANDLE>(d3d->getBackBufferView());
	auto d3dDepthV       = std::any_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(d3d->getDepthStencilView());
	auto pls_manager     = FactoryPipelineState::instance();

	// 0. d3d 작업 완료 대기.
	hr = d3d->command(EG2GRAPHICS_D3D::CMD_FENCE_WAIT);

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	hr = d3dCommandAlloc->Reset();
	if (FAILED(hr))
		return hr;

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	hr = d3dCommandList->Reset(d3dCommandAlloc, nullptr);
	if (FAILED(hr))
		return hr;

	d3dCommandList->RSSetViewports(1, d3dViewport);
	d3dCommandList->RSSetScissorRects(1, d3dScissor);

	// Indicate a state transition on the resource usage.
	d3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	d3dCommandList->ClearRenderTargetView(d3dBackBufferV, (float*)&m_cnstbPass.FogColor, 0, nullptr);
	d3dCommandList->ClearDepthStencilView(d3dDepthV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	// Specify the buffers we are going to render to.
	d3dCommandList->OMSetRenderTargets(1, &d3dBackBufferV, true, &d3dDepthV);


	// root signature
	auto signature = FactorySignature::instance()->FindRes("TEX_1");
	d3dCommandList->SetGraphicsRootSignature(signature);


	// shader reource view desciptor 설정: texture 정보
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvDescriptorHeap.Get() };
	d3dCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	
	// 지형 그리기

	// 알파 테스용 가운데 블록
	auto pls = pls_manager->FindRes("PLS_ALPHATEST");
	d3dCommandList->SetPipelineState(pls);
	DrawRenderItems(d3dCommandList);


	// Indicate a state transition on the resource usage.
	d3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	hr = d3dCommandList->Close();
	if (FAILED(hr))
		return hr;

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { d3dCommandList };
	d3dCommandQue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 1. 화면 표시.
	hr = d3d->command(EG2GRAPHICS_D3D::CMD_PRESENT);
	// 2. GPU가 해당 작업을 완료할 때까지 대기.
	hr = d3d->command(EG2GRAPHICS_D3D::CMD_FLUSH_COMMAND_QUEUE);

	return S_OK;
}

void MainApp::OnMouseDown(WPARAM btnState, const ::POINT& p)
{
	mLastMousePos = p;
}

void MainApp::OnMouseUp(WPARAM btnState, const ::POINT& p)
{
	ReleaseCapture();
}

void MainApp::OnMouseMove(WPARAM btnState, const ::POINT& p)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(p.x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(p.y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += p.x;
		mPhi += p.y;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.2f * static_cast<float>(p.x - mLastMousePos.x);
		float dy = 0.2f * static_cast<float>(p.y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
	}

	mLastMousePos = p;
}

void MainApp::OnKeyboardInput(const GameTimer& gt)
{
}

void MainApp::UpdateCamera(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	mEyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
	mEyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
	mEyePos.y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(mEyePos.x, 150, -100, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
}

void MainApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = m_frameRscCur->m_cnsgbMObject.get();

	// Only update the cbuffer data if the constants have changed.  
	// This needs to be tracked per frame resource.
	if (m_wireBox->NumFramesDirty > 0)
	{
		XMMATRIX world = XMLoadFloat4x4(&m_wireBox->World);
		XMMATRIX texTransform = XMLoadFloat4x4(&m_wireBox->TexTransform);

		ShaderConstTransform objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

		currObjectCB->CopyData(m_wireBox->ObjCBIndex, objConstants);

		// Next FrameResource need to be updated too.
		m_wireBox->NumFramesDirty--;
	}
}

void MainApp::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = m_frameRscCur->m_cnsgbMaterial.get();
	// Only update the cbuffer data if the constants have changed.  If the cbuffer
	// data changes, it needs to be updated for each FrameResource.

	if (m_wireMaterial->NumFramesDirty > 0)
	{
		XMMATRIX matTransform = XMLoadFloat4x4(&m_wireMaterial->MatTransform);

		MaterialConstants matConstants;
		matConstants.DiffuseAlbedo = m_wireMaterial->DiffuseAlbedo;
		matConstants.FresnelR0 = m_wireMaterial->FresnelR0;
		matConstants.Roughness = m_wireMaterial->Roughness;
		XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

		currMaterialCB->CopyData(m_wireMaterial->MatCBIndex, matConstants);

		// Next FrameResource need to be updated too.
		m_wireMaterial->NumFramesDirty--;
	}
}

void MainApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&m_cnstbPass.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_cnstbPass.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_cnstbPass.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_cnstbPass.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_cnstbPass.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_cnstbPass.InvViewProj, XMMatrixTranspose(invViewProj));
	m_cnstbPass.EyePosW = mEyePos;
	m_cnstbPass.RenderTargetSize = XMFLOAT2((float)m_screenSize.cx, (float)m_screenSize.cy);
	m_cnstbPass.InvRenderTargetSize = XMFLOAT2(1.0f / m_screenSize.cx, 1.0f / m_screenSize.cy);
	m_cnstbPass.NearZ = 1.0f;
	m_cnstbPass.FarZ = 1000.0f;
	m_cnstbPass.TotalTime = gt.TotalTime();
	m_cnstbPass.DeltaTime = gt.DeltaTime();
	m_cnstbPass.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	m_cnstbPass.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	m_cnstbPass.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	m_cnstbPass.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	m_cnstbPass.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	m_cnstbPass.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	m_cnstbPass.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = m_frameRscCur->m_cnstbPass.get();
	currPassCB->CopyData(0, m_cnstbPass);
}

int MainApp::UpdateFrameResource()
{
	auto d3d = IG2GraphicsD3D::instance();
	int hr = S_OK;
	// Cycle through the circular frame resource array.
	auto rscCount = d3dUtil::getFrameRscCount();
	m_frameRscIdx = (m_frameRscIdx + 1) % rscCount;
	m_frameRscCur = m_frameRscLst[m_frameRscIdx].get();

	return S_OK;
}

void MainApp::BuildDescriptorHeaps()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 2;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	auto texture_factory = FactoryTexture::instance();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto grassTex = texture_factory->FindRes("grassTex");
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	d3dDevice->CreateShaderResourceView(grassTex, &srvDesc, hDescriptor);

	auto fenceTex = texture_factory->FindRes("fenceTex");
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);							// next descriptor
	srvDesc.Format = fenceTex->GetDesc().Format;
	d3dDevice->CreateShaderResourceView(fenceTex, &srvDesc, hDescriptor);
}

void MainApp::BuildLandGeometry()
{
}

void MainApp::BuildBoxGeometry()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());
	auto d3dCommandList = std::any_cast<ID3D12GraphicsCommandList*>(IG2GraphicsD3D::instance()->getCommandList());

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

	m_wireGeo = new MeshGeometry;

	m_wireGeo->vtx.Init(vertices.data(), vbByteSize, sizeof(G2::VTX_NT), d3dDevice, d3dCommandList);
	m_wireGeo->idx.Init(indices.data(),  ibByteSize, sizeof(std::uint16_t), DXGI_FORMAT_R16_UINT, d3dDevice, d3dCommandList);
}


void MainApp::BuildFrameResources()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());

	int frameReouseNum = d3dUtil::getFrameRscCount();
	for (int i = 0; i < frameReouseNum; ++i)
	{
		m_frameRscLst.push_back(std::make_unique<FrameResource>(d3dDevice, 1, 1, 1));
	}
}

void MainApp::BuildMaterials()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());

	m_wireMaterial = new Material;
		m_wireMaterial->MatCBIndex = 0;
	m_wireMaterial->DiffuseSrvHeapIndex = 1;
	m_wireMaterial->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_wireMaterial->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	m_wireMaterial->Roughness = 0.25f;
}

void MainApp::BuildRenderItems()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());

	m_wireBox = new RenderItem;
	XMStoreFloat4x4(&m_wireBox->World, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	m_wireBox->ObjCBIndex = 0;
	m_wireBox->Mat = m_wireMaterial;
	m_wireBox->Geo = m_wireGeo;
	m_wireBox->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void MainApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList)
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ShaderConstTransform));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_frameRscCur->m_cnsgbMObject->Resource();
	auto passCB = m_frameRscCur->m_cnstbPass->Resource();
	auto matCB = m_frameRscCur->m_cnsgbMaterial->Resource();


	cmdList->IASetVertexBuffers(0, 1, &m_wireBox->Geo->vtx.VertexBufferView());
	cmdList->IASetIndexBuffer(&m_wireBox->Geo->idx.IndexBufferView());
	cmdList->IASetPrimitiveTopology(m_wireBox->PrimitiveType);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_tex_hp(m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	gpu_tex_hp.Offset(m_wireBox->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress();

	cmdList->SetGraphicsRootDescriptorTable(0, gpu_tex_hp);
	// t0가 사용중이라 상수 버퍼 b0는 1에서부터 시작
	cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
	cmdList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

	cmdList->DrawIndexedInstanced(m_wireBox->Geo->idx.Count(), 1, 0, 0, 0);
}

float MainApp::GetHillsHeight(float x, float z)const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 MainApp::GetHillsNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}
