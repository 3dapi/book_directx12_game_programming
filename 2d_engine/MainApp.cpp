//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// MainApp

#include <d3d12.h>
#include "MainApp.h"
#include "Common/DDSTextureLoader.h"
#include "Common/G2.FactoryTexture.h"
#include "Common/G2.FactoryShader.h"
#include "Common/G2.FactorySIgnature.h"
#include "Common/G2.FactoryPipelineState.h"
#include "Common/G2.Geometry.h"
#include "Common/G2.Util.h"

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
	float aspectRatio = *any_cast<float*>(IG2GraphicsD3D::instance()->getAttrib(ATT_ASPECTRATIO));
	 m_wireBox.cbPss.tmProj = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, aspectRatio, 1.0f, 1000.0f);

	ThrowIfFailed(d3dCommandList->Reset(d3dCommandAlloc, nullptr));

	// 1. load texutre
	auto tex_manager = FactoryTexture::instance();
	tex_manager->Load("grassTex", "Textures/grass.dds");
	tex_manager->Load("fenceTex", "Textures/WireFence.dds");

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

	auto pls_manager = FactoryPipelineState::instance();
	SetupUploadChain();

	BuildBox();

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

	float aspectRatio = *any_cast<float*>(IG2GraphicsD3D::instance()->getAttrib(ATT_ASPECTRATIO));
	m_wireBox.cbPss.tmProj = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, aspectRatio, 1.0f, 1000.0f);
	return S_OK;
}

int MainApp::Update(const std::any& t)
{
	// Cycle through the circular frame resource array.
	UpdateUploadChain();

	int hr = S_OK;
	GameTimer gt = std::any_cast<GameTimer>(t);
	OnKeyboardInput(gt);
	UpdateCamera(gt);


	UpdateBox(gt);

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
	XMFLOAT4 clearColor = { 0.0f, 0.4f, 0.6f, 1.0f };
	d3dCommandList->ClearRenderTargetView(d3dBackBufferV, (float*)&clearColor, 0, nullptr);
	d3dCommandList->ClearDepthStencilView(d3dDepthV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	// Specify the buffers we are going to render to.
	d3dCommandList->OMSetRenderTargets(1, &d3dBackBufferV, true, &d3dDepthV);


	// Box 그리기
	DrawBox(d3dCommandList);


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
	m_tmEyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
	m_tmEyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
	m_tmEyePos.y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(20, 40, -40, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_wireBox.cbPss.tmView = XMMatrixLookAtLH(pos, target, up);

	//constant world transform
	//auto currObjectCB = m_subCur->m_cnstTranform.get();
	//{
	//	XMMATRIX world = XMLoadFloat4x4(&m_cnstWorld.tmWorld);
	//	ShaderConstTransform objConstants;
	//	XMStoreFloat4x4(&objConstants.tmWorld, XMMatrixTranspose(world));

	//	currObjectCB->CopyData(0, objConstants);
	//}
}

void MainApp::UpdateBox(const GameTimer& gt)
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

int MainApp::UpdateUploadChain()
{
	auto d3d = IG2GraphicsD3D::instance();
	int hr = S_OK;
	// Cycle through the circular frame resource array.
	auto rscCount = d3dUtil::getFrameRscCount();
	m_subIdx = (m_subIdx + 1) % rscCount;
	m_subCur = m_subLst[m_subIdx].get();

	return S_OK;
}

void MainApp::BuildBox()
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




void MainApp::SetupUploadChain()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());

	int frameReouseNum = d3dUtil::getFrameRscCount();
	for (int i = 0; i < frameReouseNum; ++i)
	{
		m_subLst.push_back(std::make_unique<ShaderUploadChain>(d3dDevice, 1, 1, 1));
	}
}

void MainApp::DrawBox(ID3D12GraphicsCommandList* cmdList)
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



	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ShaderConstTransform));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ShaderConstMaterial));

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
