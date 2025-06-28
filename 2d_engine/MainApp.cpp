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

#include "SceneGameMesh.h"

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

	// 1. load texture
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
	
	{
		auto scene = std::make_unique<SceneGameMesh>();
		if (scene)
		{
			int hr = scene->Init();
			if (FAILED(hr))
			{
				scene = {};
				return E_FAIL;
			}
		}
		m_sceneBox = std::move(scene);
	}

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

std::any MainApp::getAttrib(int nAttrib)
{
	//switch ((EAPP_ATTRIB)nAttrib)
	//{
	//	case EAPP_ATTRIB::EAPP_ATT_CUR_CB:
	//		return m_frameRscCur;
	//}
	return std::any();
}

int MainApp::setAttrib(int nAttrib, const std::any& v)
{
	return E_FAIL;
}

int MainApp::command(int nCmd, const std::any& v)
{
	return E_FAIL;
}

int MainApp::Resize(bool up)
{
	int hr = D3DWinApp::Resize(up);
	return S_OK;
}

int MainApp::Update(const std::any& t)
{
	int hr = S_OK;
	auto aspectRatio = *std::any_cast<float*>(IG2GraphicsD3D::instance()->getAttrib(ATT_ASPECTRATIO));
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, aspectRatio, 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);


	// Cycle through the circular frame resource array.
	UpdateFrameResource();

	GameTimer gt = std::any_cast<GameTimer>(t);
	OnKeyboardInput(gt);
	UpdateCamera(gt);

	// instance update
	this->m_sceneBox->Update(t);

	// finally update constant buffer
	this->UpdateConstBuffer();
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

	//// root signature
	//auto signature = FactorySignature::instance()->FindRes("TEX_1");
	//d3dCommandList->SetGraphicsRootSignature(signature);


	// Box 그리기
	m_sceneBox->Render();


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
	XMVECTOR pos = XMVectorSet(20, 40, -40, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	//constant world transform
	//auto currObjectCB = m_frameRscCur->m_cnstTranform.get();
	//{
	//	XMMATRIX world = XMLoadFloat4x4(&m_cnstWorld.tmWorld);
	//	ShaderConstTransform objConstants;
	//	XMStoreFloat4x4(&objConstants.tmWorld, XMMatrixTranspose(world));

	//	currObjectCB->CopyData(0, objConstants);
	//}
}

void MainApp::UpdateConstBuffer()
{
//	auto currObjectCB = m_frameRscCur->m_cnstTranform.get();

	////constant world transform
	//{
	//	XMMATRIX world = XMLoadFloat4x4(&m_World);
	//	ShaderConstTransform objConstants;
	//	XMStoreFloat4x4(&objConstants.tmWorld, XMMatrixTranspose(world));

	//	currObjectCB->CopyData(0, objConstants);
	//}

	//// constant pass
	//auto currPassCB = m_frameRscCur->m_cnstPass.get();
	//{
	//	XMMATRIX view = XMLoadFloat4x4(&mView);
	//	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	//	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	//	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	//	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	//	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	//	XMStoreFloat4x4(&m_cnstPass.tmView, XMMatrixTranspose(view));
	//	XMStoreFloat4x4(&m_cnstPass.tmProj, XMMatrixTranspose(proj));
	//	XMStoreFloat4x4(&m_cnstPass.tmViewProj, XMMatrixTranspose(viewProj));

	//	currPassCB->CopyData(0, m_cnstPass);
	//}

	//// const material
	//auto currMaterialCB = m_frameRscCur->m_cnsgbMaterial.get();
	//{
	//	XMMATRIX matTransform = XMLoadFloat4x4(&m_wireBox->Mat->matConst.tmTexCoord);

	//	ShaderConstMaterial matConstants;
	//	matConstants.DiffuseAlbedo = m_wireBox->Mat->matConst.DiffuseAlbedo;
	//	XMStoreFloat4x4(&matConstants.tmTexCoord, XMMatrixTranspose(matTransform));

	//	currMaterialCB->CopyData(0, matConstants);
	//}
}

int MainApp::UpdateFrameResource()
{
	auto d3d = IG2GraphicsD3D::instance();
	int hr = S_OK;
	// Cycle through the circular frame resource array.
	auto rscCount = d3dUtil::getFrameRscCount();
//	m_frameRscIdx = (m_frameRscIdx + 1) % rscCount;
//	m_frameRscCur = m_frameRscLst[m_frameRscIdx].get();

	return S_OK;
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
