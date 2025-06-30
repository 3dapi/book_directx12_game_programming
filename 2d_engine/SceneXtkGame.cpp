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
#include "SceneXtkGame.h"
#include "DirectXHelpers.h"
#include "ResourceUploadBatch.h"
#include "GraphicsMemory.h"
#include "MainApp.h"

using std::any_cast;

SceneXtkGame::SceneXtkGame() noexcept
{
}

SceneXtkGame::~SceneXtkGame()
{
	if(m_audEngine)
	{
		m_audEngine->Suspend();
	}
}

// Initialize the Direct3D resources required to run.
int SceneXtkGame::Init(const std::any&)
{
	auto d3d               = IG2GraphicsD3D::instance();
	auto device            = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto formatBackBuffer  = *any_cast<DXGI_FORMAT*>(d3d->getAttrib(ATT_DEVICE_BACKBUFFER_FORAT));
	auto formatDepthBuffer = *any_cast<DXGI_FORMAT*>(d3d->getAttrib(ATT_DEVICE_DEPTH_STENCIL_FORAT));

	auto hWindow     = std::any_cast<HWND>(IG2AppFrame::instance()->getAttrib(EAPP_ATTRIB::EAPP_ATT_WIN_HWND));


	m_gamePad = std::make_unique<GamePad>();
	m_keyboard = std::make_unique<Keyboard>();

	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(hWindow);

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	// Create DirectXTK for Audio objects
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags |= AudioEngine_Debug;
#endif

	m_audEngine = std::make_unique<AudioEngine>(eflags);

	m_audioEvent = 0;
	m_audioTimerAcc = 10.f;
	m_retryDefault = false;

	m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(),L"assets/adpcmdroid.xwb");

	m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(),L"assets/MusicMono_adpcm.wav");
	m_effect1 = m_soundEffect->CreateInstance();
	m_effect2 = m_waveBank->CreateInstance(10);

	m_effect1->Play(true);
	m_effect2->Play();

	return S_OK;
}

int SceneXtkGame::Destroy()
{
	return 0;
}

#pragma region Frame Update
// Executes the basic game loop.
void SceneXtkGame::Tick()
{
	m_timer.Tick([&]()
	{
		Update(m_timer);
	});

	// Only update audio engine once per frame
	if(!m_audEngine->IsCriticalError() && m_audEngine->Update())
	{
		// Setup a retry in 1 second
		m_audioTimerAcc = 1.f;
		m_retryDefault = true;
	}

	Render();
}

// Updates the world.
int SceneXtkGame::Update(const std::any& t)
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

	m_lineEffect->SetView(m_view);
	m_lineEffect->SetWorld(XMMatrixIdentity());

	m_shapeEffect->SetView(m_view);

	m_audioTimerAcc -= (float)gt.DeltaTime();
	if(m_audioTimerAcc < 0)
	{
		if(m_retryDefault)
		{
			m_retryDefault = false;
			if(m_audEngine->Reset())
			{
				// Restart looping audio
				m_effect1->Play(true);
			}
		}
		else
		{
			m_audioTimerAcc = 4.f;

			m_waveBank->Play(m_audioEvent++);

			if(m_audioEvent >= 11)
				m_audioEvent = 0;
		}
	}

	const auto pad = m_gamePad->GetState(0);
	if(pad.IsConnected())
	{
		m_gamePadButtons.Update(pad);

		if(pad.IsViewPressed())
		{
		}
	}
	else
	{
		m_gamePadButtons.Reset();
	}

	const auto kb = m_keyboard->GetState();
	m_keyboardButtons.Update(kb);

	if(kb.Escape)
	{
	}

	PIXEndEvent();

	return S_OK;
}
#pragma endregion

#pragma region Frame Render

// Draws the scene.

static bool isFirstRender = true;
int SceneXtkGame::Render()
{
	auto d3d = IG2GraphicsD3D::instance();
	auto device    = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto cmdList  = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());

	// Don't try to render anything before the first Update.
	if(isFirstRender)
	{
		isFirstRender = false;
		return S_OK;
	}


	PIXBeginEvent(cmdList,PIX_COLOR_DEFAULT,L"Render");

	// Draw procedurally generated dynamic grid
	const XMVECTORF32 xaxis ={20.f,0.f,0.f};
	const XMVECTORF32 yaxis ={0.f,0.f,20.f};
	DrawGrid(xaxis,yaxis,g_XMZero,20,20,Colors::Gray);

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

	// Draw 3D object
	PIXBeginEvent(cmdList,PIX_COLOR_DEFAULT,L"Draw teapot");
	XMMATRIX local = m_world * XMMatrixTranslation(-2.f,-2.f,-4.f);
	m_shapeEffect->SetWorld(local);
	m_shapeEffect->Apply(cmdList);
	m_shape->Draw(cmdList);
	PIXEndEvent(cmdList);

	// Draw model
	PIXBeginEvent(cmdList,PIX_COLOR_DEFAULT,L"Draw model");
	const XMVECTORF32 scale ={0.01f,0.01f,0.01f};
	const XMVECTORF32 translate ={3.f,-2.f,-4.f};
	const XMVECTOR rotate = XMQuaternionRotationRollPitchYaw(XM_PI / 2.f,0.f,-XM_PI / 2.f);
	local = m_world * XMMatrixTransformation(g_XMZero,XMQuaternionIdentity(), scale, g_XMZero, rotate, translate);
	Model::UpdateEffectMatrices(m_modelEffects,local,m_view,m_projection);
	heaps[0] = m_modelResources->Heap();
	cmdList->SetDescriptorHeaps(_countof(heaps),heaps);
	m_model->Draw(cmdList,m_modelEffects.begin());
	PIXEndEvent(cmdList);

	return S_OK;
}

void XM_CALLCONV SceneXtkGame::DrawGrid(DirectX::FXMVECTOR xAxis,DirectX::FXMVECTOR yAxis,DirectX::FXMVECTOR origin,size_t xdivs,size_t ydivs,DirectX::GXMVECTOR color)
{
	auto d3d = IG2GraphicsD3D::instance();
	auto device    = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto cmdList  = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());
	auto pBatch       = std::any_cast<XTK_BATCH* >(IG2AppFrame::instance()->getAttrib(EAPP_ATTRIB::EAPP_ATT_XTK_BATCH));

	PIXBeginEvent(cmdList,PIX_COLOR_DEFAULT,L"Draw grid");

	m_lineEffect->Apply(cmdList);

	pBatch->Begin(cmdList);

	xdivs = std::max<size_t>(1,xdivs);
	ydivs = std::max<size_t>(1,ydivs);

	for(size_t i = 0; i <= xdivs; ++i)
	{
		float fPercent = float(i) / float(xdivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(xAxis,fPercent);
		vScale = XMVectorAdd(vScale,origin);

		const VertexPositionColor v1(XMVectorSubtract(vScale,yAxis),color);
		const VertexPositionColor v2(XMVectorAdd(vScale,yAxis),color);
		pBatch->DrawLine(v1,v2);
	}

	for(size_t i = 0; i <= ydivs; i++)
	{
		float fPercent = float(i) / float(ydivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(yAxis,fPercent);
		vScale = XMVectorAdd(vScale,origin);

		const VertexPositionColor v1(XMVectorSubtract(vScale,xAxis),color);
		const VertexPositionColor v2(XMVectorAdd(vScale,xAxis),color);
		pBatch->DrawLine(v1,v2);
	}

	pBatch->End();

	PIXEndEvent(cmdList);
}
#pragma endregion

#pragma region Message Handlers

void SceneXtkGame::NewAudioDevice()
{
	if(m_audEngine && !m_audEngine->IsAudioDevicePresent())
	{
		// Setup a retry in 1 second
		m_audioTimerAcc = 1.f;
		m_retryDefault = true;
	}
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void SceneXtkGame::CreateDeviceDependentResources()
{
	auto d3d               = IG2GraphicsD3D::instance();
	auto device            = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto formatBackBuffer  = *any_cast<DXGI_FORMAT*>(d3d->getAttrib(ATT_DEVICE_BACKBUFFER_FORAT));
	auto formatDepthBuffer = *any_cast<DXGI_FORMAT*>(d3d->getAttrib(ATT_DEVICE_DEPTH_STENCIL_FORAT));

	auto pGraphicsMemory   = std::any_cast<GraphicsMemory* >(IG2AppFrame::instance()->getAttrib(EAPP_ATTRIB::EAPP_ATT_XTK_GRAPHICS_MEMORY));

	m_states = std::make_unique<CommonStates>(device);
	m_resourceDescriptors = std::make_unique<DescriptorHeap>(device, Descriptors::Count);
	m_shape = GeometricPrimitive::CreateTeapot(4.f,8);

	// SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
	m_model = Model::CreateFromSDKMESH(device,L"assets/tiny.sdkmesh");

	{
		ResourceUploadBatch resourceUpload(device);

		resourceUpload.Begin();

		m_model->LoadStaticBuffers(device,resourceUpload);

		ThrowIfFailed(CreateDDSTextureFromFile(device,resourceUpload,L"assets/seafloor.dds",m_texture1.ReleaseAndGetAddressOf()) );
		CreateShaderResourceView(device,m_texture1.Get(),m_resourceDescriptors->GetCpuHandle(Descriptors::SeaFloor));
		ThrowIfFailed( CreateWICTextureFromFileEx(device,resourceUpload,L"assets/texture/res_checker.png", 0, D3D12_RESOURCE_FLAG_NONE, WIC_LOADER_DEFAULT, m_checkerRsc.ReleaseAndGetAddressOf()) );

		CreateShaderResourceView(device,m_checkerRsc.Get(),m_resourceDescriptors->GetCpuHandle(Descriptors::WindowsLogo));

		const RenderTargetState rtState(formatBackBuffer, formatDepthBuffer);
		{
			SpriteBatchPipelineStateDescription pd(rtState);
			m_sprites = std::make_unique<SpriteBatch>(device,resourceUpload,pd);
		}

		{
			EffectPipelineStateDescription pd(
				&VertexPositionColor::InputLayout,
				CommonStates::Opaque,
				CommonStates::DepthNone,
				CommonStates::CullNone,
				rtState,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

			m_lineEffect = std::make_unique<BasicEffect>(device,EffectFlags::VertexColor,pd);
		}

		{
			EffectPipelineStateDescription pd(
				&GeometricPrimitive::VertexType::InputLayout,
				CommonStates::Opaque,
				CommonStates::DepthDefault,
				CommonStates::CullNone,
				rtState);

			m_shapeEffect = std::make_unique<BasicEffect>(device,EffectFlags::PerPixelLighting | EffectFlags::Texture,pd);
			m_shapeEffect->EnableDefaultLighting();
			m_shapeEffect->SetTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::SeaFloor),m_states->LinearWrap());
		}

		m_modelResources = m_model->LoadTextures(device,resourceUpload, L"assets");

		{
			const EffectPipelineStateDescription psd(
				nullptr,
				CommonStates::Opaque,
				CommonStates::DepthDefault,
				CommonStates::CullNone,
				rtState);

			m_modelEffects = m_model->CreateEffects(psd,psd,m_modelResources->Heap(),m_states->Heap());
		}

		m_font = std::make_unique<SpriteFont>(device,resourceUpload,
			L"assets/SegoeUI_18.spritefont",
			m_resourceDescriptors->GetCpuHandle(Descriptors::SegoeFont),
			m_resourceDescriptors->GetGpuHandle(Descriptors::SegoeFont));

		// Upload the resources to the GPU.
		auto cmdQueue      = std::any_cast<ID3D12CommandQueue*>(d3d->getCommandQueue());
		auto uploadResourcesFinished = resourceUpload.End(cmdQueue);

		// Wait for the command list to finish executing
		d3d->command(EG2GRAPHICS_D3D::CMD_WAIT_GPU);
		
		// Wait for the upload thread to terminate
		uploadResourcesFinished.wait();
	}
}

// Allocate all memory resources that change on a window SizeChanged event.
void SceneXtkGame::CreateWindowSizeDependentResources()
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

	m_lineEffect->SetProjection(m_projection);
	m_shapeEffect->SetProjection(m_projection);

	m_sprites->SetViewport(d3dViewport);
}

#pragma endregion

