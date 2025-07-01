#include <any>
#include <Windows.h>
#include <d3d11.h>
#include <DirectxColors.h>
#include <WICTextureLoader.h>
#include "G2Base.h"
#include "MainApp.h"
#include "G2Util.h"
using namespace DirectX;
using namespace spine;

inline std::wstring ansiToWstr(const std::string& str)
{
	int len = MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,nullptr,0);
	if(1 >= len)
		return  L"";
	std::wstring wstr(len - 1,0);
	MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,&wstr[0],len);
	return wstr;
}

namespace spine {
	SpineExtension *getDefaultExtension()
	{
		static SpineExtension* _default_spineExtension = new DefaultSpineExtension;
		return _default_spineExtension;
	}
}

MainApp::MainApp()
{
}

MainApp::~MainApp()
{
	Destroy();
}

void MainApp::load(spine::AtlasPage& page,const spine::String& path) {
	auto fileName = path.buffer();
	HRESULT hr = S_OK;
	auto d3dDevice  = std::any_cast<ID3D11Device*>(IG2GraphicsD3D::getInstance()->GetDevice());
	auto d3dContext = std::any_cast<ID3D11DeviceContext*>(IG2GraphicsD3D::getInstance()->GetContext());
	ID3D11Resource*				textureRsc{};
	ID3D11ShaderResourceView*	textureView{};
	auto wstr_file = ansiToWstr(fileName);
	hr  = DirectX::CreateWICTextureFromFile(d3dDevice, d3dContext, wstr_file.c_str(), &textureRsc, &textureView);
	if(SUCCEEDED(hr))
	{
		m_srvTexture = textureView;
		page.texture = m_srvTexture;
	}
}

void MainApp::unload(void* texture) {
	texture = nullptr;
}

int MainApp::Init()
{
	HRESULT hr = S_OK;
	auto d3dDevice  = std::any_cast<ID3D11Device*>(IG2GraphicsD3D::getInstance()->GetDevice());
	auto d3dContext = std::any_cast<ID3D11DeviceContext*>(IG2GraphicsD3D::getInstance()->GetContext());

	// create vertex shader
	// 1.1 Compile the vertex shader
	ID3DBlob* pBlob{};
	hr = G2::DXCompileShaderFromFile("assets/simple.fx", "main_vtx", "vs_4_0", &pBlob);
	if (FAILED(hr))
	{
		MessageBox({}, "The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
		return hr;
	}
	// 1.2 Create the vertex shader
	hr = d3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), {}, &m_shaderVtx);
	if (FAILED(hr))
		return hr;

	// 1.3 create vertexLayout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM , 0, 0 + sizeof(XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 0 + sizeof(XMFLOAT3) + sizeof(uint32_t), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = d3dDevice->CreateInputLayout(layout, numElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_vtxLayout);
	G2::SAFE_RELEASE(pBlob);
	if (FAILED(hr))
		return hr;

	// 2.1 Compile the pixel shader
	hr = G2::DXCompileShaderFromFile("assets/simple.fx", "main_pxl", "ps_4_0", &pBlob);
	if (FAILED(hr))
	{
		MessageBox({}, "Failed ComplThe FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
		return hr;
	}
	// 2.2 Create the pixel shader
	hr = d3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), {}, &m_shaderPxl);
	G2::SAFE_RELEASE(pBlob);
	if (FAILED(hr))
		return hr;

	// 3. Create vertex buffer

	m_bufVtxCount = 4;
	D3D11_BUFFER_DESC spineBd = {};
	spineBd.Usage = D3D11_USAGE_DYNAMIC;
	spineBd.ByteWidth = sizeof(Vertex) * m_bufVtxCount;
	spineBd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	spineBd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = d3dDevice->CreateBuffer(&spineBd,nullptr,&m_bufVtx);
	if (FAILED(hr))
		return hr;


	// 5. Create the constant buffer
	// 5.1 world
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(m_mtWorld);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = d3dDevice->CreateBuffer(&bd, {}, &m_cnstWorld);
	if (FAILED(hr))
		return hr;

	// 5.2 view
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(m_mtView);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = d3dDevice->CreateBuffer(&bd, {}, &m_cnstView);
	if (FAILED(hr))
		return hr;

	// 5.3 projection matrtix
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(m_mtProj);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = d3dDevice->CreateBuffer(&bd, {}, &m_cnstProj);
	if (FAILED(hr))
		return hr;

	// 6. setup the world, view, projection matrix
	// View, Projection Matrix
	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0f, 0.0f, -1000.0f, 0.0f );
	XMVECTOR At = XMVectorSet ( 0.0f, 0.0f,   0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet ( 0.0f, 1.0f,   0.0f, 0.0f );
	m_mtView = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	auto screeSize = std::any_cast<::SIZE*>(IG2GraphicsD3D::getInstance()->GetAttrib(ATTRIB_CMD::ATTRIB_SCREEN_SIZE));
	m_mtProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, screeSize->cx / (FLOAT)screeSize->cy, 1.0f, 5000.0f);

	// 7. Initialize the world matrix
	m_mtWorld = XMMatrixIdentity();


	// 8. create texture sampler state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = d3dDevice->CreateSamplerState(&sampDesc, &m_sampLinear);
	if (FAILED(hr))
		return hr;

	// 9. create texture sampler state
	{
		D3D11_RASTERIZER_DESC rasterDesc ={};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_NONE;		// 또는 D3D11_CULL_NONE, D3D11_CULL_FRONT
		rasterDesc.FrontCounterClockwise = TRUE;	// CCW 면이 앞면 (보통 OpenGL 스타일)
		hr = d3dDevice->CreateRasterizerState(&rasterDesc,&m_stateRater);
		if(SUCCEEDED(hr)) {
			d3dContext->RSSetState(m_stateRater);
		}
	}
	{
		D3D11_BLEND_DESC blendDesc ={};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = d3dDevice->CreateBlendState(&blendDesc,&m_stateBlend);
		if(FAILED(hr))
		{
			return hr;
		}
	}
	
	Bone::setYDown(false);

	m_spineAtlas = new Atlas("../assets/spine/spineboy-pma.atlas", this);
	SkeletonBinary binary(m_spineAtlas);
	m_spineSkeletonData = binary.readSkeletonDataFile("../assets/spine/spineboy-pro.skel");


	m_spineSkeleton = new Skeleton(m_spineSkeletonData);
	m_spineSkeleton->setPosition(0.0F, 0.0F);
	m_spineSkeleton->setScaleX(0.4f);
	m_spineSkeleton->setScaleY(0.4f);

	AnimationStateData animationStateData(m_spineSkeletonData);
	animationStateData.setDefaultMix(0.2f);
	m_spineAniState = new AnimationState(&animationStateData);
	m_spineAniState->setAnimation(0,"portal",true);
	m_spineAniState->addAnimation(0,"run",true,0);

	mTimer.Reset();
	return S_OK;
}

int MainApp::Destroy()
{
	G2::SAFE_RELEASE(m_shaderVtx);
	G2::SAFE_RELEASE(m_shaderPxl);
	G2::SAFE_RELEASE(m_vtxLayout);
	G2::SAFE_RELEASE(m_bufVtx	);
	G2::SAFE_RELEASE(m_cnstWorld);
	G2::SAFE_RELEASE(m_cnstView	);
	G2::SAFE_RELEASE(m_cnstProj	);

	G2::SAFE_RELEASE(m_srvTexture);
	G2::SAFE_RELEASE(m_sampLinear);
	return S_OK;
}

int MainApp::Update()
{
	mTimer.Tick();

	auto t = mTimer.DeltaTime();

	// Update and apply the animation state to the skeleton
	m_spineAniState->update(t);
	m_spineAniState->apply(*m_spineSkeleton);

	// Update the skeleton time (used for physics)
	m_spineSkeleton->update(t);

	// Calculate the new pose
	m_spineSkeleton->updateWorldTransform(spine::Physics_Update);

	return S_OK;
}

int MainApp::Render()
{
	auto d3dDevice  = std::any_cast<ID3D11Device*>(IG2GraphicsD3D::getInstance()->GetDevice());
	auto d3dContext = std::any_cast<ID3D11DeviceContext*>(IG2GraphicsD3D::getInstance()->GetContext());

	// 0. render state
	d3dContext->RSSetState(m_stateRater);
	float blendFactor[4] ={0,0,0,0};
	UINT sampleMask = 0xffffffff;
	d3dContext->OMSetBlendState(m_stateBlend,blendFactor,sampleMask);

	// 1. Update constant value
	d3dContext->UpdateSubresource(m_cnstWorld, 0, {}, &m_mtWorld, 0, 0);
	d3dContext->UpdateSubresource(m_cnstView , 0, {}, &m_mtView , 0, 0);
	d3dContext->UpdateSubresource(m_cnstProj , 0, {}, &m_mtProj , 0, 0);

	// 2. set the constant buffer
	d3dContext->VSSetConstantBuffers(0, 1, &m_cnstWorld);
	d3dContext->VSSetConstantBuffers(1, 1, &m_cnstView);
	d3dContext->VSSetConstantBuffers(2, 1, &m_cnstProj);
	
	// 3. set vertex shader
	d3dContext->VSSetShader(m_shaderVtx,{},0);

	// 4. set the input layout
	d3dContext->IASetInputLayout(m_vtxLayout);

	// 5. set the pixel shader
	d3dContext->PSSetShader(m_shaderPxl,{},0);

	// 6. set the sampler state
	d3dContext->PSSetSamplers(0,1,&m_sampLinear);

	// 7. primitive topology
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	auto drawOrder = m_spineSkeleton->getDrawOrder();

	for(size_t i = 0; i < drawOrder.size(); ++i) {
		spine::Slot* slot = drawOrder[i];
		spine::Attachment* attachment = slot->getAttachment();
		if(!attachment) continue;

		if(attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
			auto* region = static_cast<spine::RegionAttachment*>(attachment);

			// 정점 좌표 계산
			float worldVertices[8];
			region->computeWorldVertices(*slot,worldVertices,0,2);

			auto uvs = region->getUVs();

			// RegionAttachment → TextureRegion
			spine::TextureRegion* texRegion = region->getRegion();
			if(!texRegion)
				continue;

			// TextureRegion → AtlasRegion
			auto* atlasRegion = reinterpret_cast<spine::AtlasRegion*>(texRegion);

			// AtlasPage → rendererObject
			auto* page = atlasRegion->page;
			auto* texSRV = reinterpret_cast<ID3D11ShaderResourceView*>(page->texture);
			if(!texSRV)
				continue;

			// 색상 계산
			spine::Color c = m_spineSkeleton->getColor();
			c.a *= slot->getColor().a;
			c.r *= slot->getColor().r;
			c.g *= slot->getColor().g;
			c.b *= slot->getColor().b;

			spine::Color rColor = region->getColor();
			c.a *= rColor.a;
			c.r *= rColor.r;
			c.g *= rColor.g;
			c.b *= rColor.b;

			uint32_t rgba =
				(uint32_t(c.a * 255) << 24) |
				(uint32_t(c.r * 255) << 16) |
				(uint32_t(c.g * 255) << 8)  |
				(uint32_t(c.b * 255) << 0);

			// 정점 구성
			Vertex vertices[4];
			for(int v = 0; v < 4; ++v) {
				vertices[v].p.x = worldVertices[v * 2];
				vertices[v].p.y = worldVertices[v * 2 + 1];
				vertices[v].p.z = 0;
				vertices[v].t.x = uvs[v * 2];
				vertices[v].t.y = uvs[v * 2 + 1];
				vertices[v].c = rgba; // DWORD 색상
			}

			// 8. 렌더링 정점 복사
			D3D11_MAPPED_SUBRESOURCE mapped ={};
			if(SUCCEEDED(d3dContext->Map(m_bufVtx,0,D3D11_MAP_WRITE_DISCARD,0,&mapped))) {
				memcpy(mapped.pData,vertices,sizeof(vertices));
				d3dContext->Unmap(m_bufVtx,0);
			}

			// 9. 바인딩
			UINT stride = sizeof(Vertex),offset = 0;
			d3dContext->IASetVertexBuffers(0,1,&m_bufVtx,&stride,&offset);
			d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			d3dContext->PSSetShaderResources(0,1,&texSRV);

			// 10. draw
			d3dContext->Draw(m_bufVtxCount, 0);
		}
	}

	return S_OK;
}

