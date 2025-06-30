//
// Game.cpp
//

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
#include "SceneSpine.h"
#include "DirectXHelpers.h"
#include "ResourceUploadBatch.h"
#include "GraphicsMemory.h"
#include "MainApp.h"

using std::any_cast;

SceneSpine::SceneSpine() noexcept
{
}

SceneSpine::~SceneSpine()
{
}

// Initialize the Direct3D resources required to run.
int SceneSpine::Init(const std::any&)
{
	auto d3d               = IG2GraphicsD3D::instance();
	auto device            = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto formatBackBuffer  = *any_cast<DXGI_FORMAT*>(d3d->getAttrib(ATT_DEVICE_BACKBUFFER_FORAT));
	auto formatDepthBuffer = *any_cast<DXGI_FORMAT*>(d3d->getAttrib(ATT_DEVICE_DEPTH_STENCIL_FORAT));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	return S_OK;
}

int SceneSpine::Destroy()
{
	return 0;
}

#pragma region Frame Update

// Updates the world.
int SceneSpine::Update(const std::any& t)
{
	GameTimer gt = std::any_cast<GameTimer>(t);
	// Cycle through the circular frame resource array.

	PIXBeginEvent(PIX_COLOR_DEFAULT,L"Update");

	const XMFLOAT3 eye(0.0f,0.7f,1.5f);
	const XMFLOAT3 at(0.0f,-0.1f,0.0f);
	const XMFLOAT3 up(0.0f,1.0f,0.0f);

	//m_view = Matrix::CreateLookAt(eye, at, up);
	m_view = XMMatrixLookAtRH(XMLoadFloat3(&eye),XMLoadFloat3(&at),XMLoadFloat3(&up));
	m_world = XMMatrixRotationY(float(gt.TotalTime() * XM_PIDIV4));

	PIXEndEvent();

	return S_OK;
}
#pragma endregion

#pragma region Frame Render

// Draws the scene.

static bool isFirstRender = true;
int SceneSpine::Render()
{
	return S_OK;
	auto d3d = IG2GraphicsD3D::instance();
	auto device   = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto cmdList  = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());

	// Don't try to render anything before the first Update.
	if(isFirstRender)
	{
		isFirstRender = false;
		return S_OK;
	}

	PIXBeginEvent(cmdList,PIX_COLOR_DEFAULT,L"Render");

	// Set the descriptor heaps
	ID3D12DescriptorHeap* heaps[] ={ (ID3D12DescriptorHeap*)m_resourceDescriptors->Heap(),(ID3D12DescriptorHeap*)m_states->Heap()};
	cmdList->SetDescriptorHeaps(_countof(heaps),heaps);

	// Draw sprite
	PIXBeginEvent(cmdList,PIX_COLOR_DEFAULT,L"Draw sprite");
	m_sprites->Begin(cmdList);
	m_sprites->Draw(m_resourceDescriptors->GetGpuHandle(Descriptors::WindowsLogo), DirectX::GetTextureSize(m_checkerRsc.Get()), XMFLOAT2(10,75));

	m_font->DrawString(m_sprites.get(),L"DirectXTK12 Simple Sample",XMFLOAT2(100,10),Colors::Yellow);
	m_sprites->End();
	PIXEndEvent(cmdList);

	return S_OK;
}

#pragma endregion

#pragma region Message Handlers

#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void SceneSpine::CreateDeviceDependentResources()
{
	auto d3d               = IG2GraphicsD3D::instance();
	auto device            = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto formatBackBuffer  = *any_cast<DXGI_FORMAT*>(d3d->getAttrib(ATT_DEVICE_BACKBUFFER_FORAT));
	auto formatDepthBuffer = *any_cast<DXGI_FORMAT*>(d3d->getAttrib(ATT_DEVICE_DEPTH_STENCIL_FORAT));

	auto pGraphicsMemory   = std::any_cast<GraphicsMemory* >(IG2AppFrame::instance()->getAttrib(EAPP_ATTRIB::EAPP_ATT_XTK_GRAPHICS_MEMORY));

	m_states = std::make_unique<CommonStates>(device);
	m_resourceDescriptors = std::make_unique<DescriptorHeap>(device, Descriptors::Count);

	{
		ResourceUploadBatch resourceUpload(device);
		resourceUpload.Begin();

		ThrowIfFailed( CreateWICTextureFromFileEx(device,resourceUpload,L"assets/texture/res_checker.png", 0, D3D12_RESOURCE_FLAG_NONE, WIC_LOADER_DEFAULT, m_checkerRsc.ReleaseAndGetAddressOf()) );
		CreateShaderResourceView(device,m_checkerRsc.Get(),m_resourceDescriptors->GetCpuHandle(Descriptors::WindowsLogo));

		const RenderTargetState rtState(formatBackBuffer, formatDepthBuffer);
		{
			SpriteBatchPipelineStateDescription pd(rtState);
			m_sprites = std::make_unique<SpriteBatch>(device,resourceUpload,pd);
		}

		m_font = std::make_unique<SpriteFont>(device,resourceUpload,
			L"assets/SegoeUI_18.spritefont",
			m_resourceDescriptors->GetCpuHandle(Descriptors::SegoeFont),
			m_resourceDescriptors->GetGpuHandle(Descriptors::SegoeFont));

		// Upload the resources to the GPU.
		auto cmdQueue      = std::any_cast<ID3D12CommandQueue*>(d3d->getCommandQueue());
		// Wait for the command list to finish executing
		auto uploadResourcesFinished = resourceUpload.End(cmdQueue);
		
		// Wait for the upload thread to terminate
		uploadResourcesFinished.wait();
	}
}

// Allocate all memory resources that change on a window SizeChanged event.
void SceneSpine::CreateWindowSizeDependentResources()
{
	auto d3d = IG2GraphicsD3D::instance();
	auto d3dViewport     = *std::any_cast<D3D12_VIEWPORT*>(d3d->getAttrib(ATT_DEVICE_VIEWPORT));
	float aspectRatio    = *any_cast<float*             >(d3d->getAttrib(ATT_ASPECTRATIO));

	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if(aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	m_projection = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio,  0.1f, 5000.0f);
	m_sprites->SetViewport(d3dViewport);
}

#pragma endregion

