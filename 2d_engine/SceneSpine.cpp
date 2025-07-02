#include <d3d12.h>
#include "DDSTextureLoader.h"
#include "Common/D12DDSTextureLoader.h"
#include "Common/G2.FactoryTexture.h"
#include "Common/G2.FactoryShader.h"
#include "Common/G2.FactorySIgnature.h"
#include "Common/G2.FactoryPipelineState.h"
#include "Common/G2.Geometry.h"
#include "Common/G2.Util.h"
#include "Common/GameTimer.h"
#include <pix.h>
#include "CommonStates.h"
#include "SceneSpine.h"
#include "DirectXHelpers.h"
#include "ResourceUploadBatch.h"
#include "GraphicsMemory.h"
#include "SceneSpine.h"

using std::any_cast;
using namespace DirectX;
using namespace spine;

namespace spine {
	SpineExtension* getDefaultExtension() {
		static SpineExtension* _default_spineExtension = new DefaultSpineExtension;
		return _default_spineExtension;
	}
}

VtxSequenceSpine::~VtxSequenceSpine()
{
	SAFE_RELEASE(	bufPos		);
	SAFE_RELEASE(	bufTex		);
	SAFE_RELEASE(	bufDif		);
	SAFE_RELEASE(	bufIdx		);
	SAFE_RELEASE(	uploadPos	);
	SAFE_RELEASE(	uploadTex	);
	SAFE_RELEASE(	uploadDif	);
	SAFE_RELEASE(	uploadIdx	);
}

int VtxSequenceSpine::resourceBinding(int order, void* attachment, ESPINE_ATTACHMENT_TYPE attachmentType, size_t vertexCount, size_t indexCount /*=0*/)
{
    HRESULT hr = S_OK;
    auto d3d		=  IG2GraphicsD3D::instance();
	auto d3dDevice	= std::any_cast<ID3D12Device*>(d3d->getDevice());
	if(!d3dDevice)
		return E_FAIL;
	if(0 >= vertexCount)
		return E_FAIL;

    this->drawOrder = order;
    this->meshType  = attachmentType;
    this->countVtx  = vertexCount;
    this->countIdx  = indexCount;

    auto createBuffer = [&](ID3D12Resource** dst, size_t byteSize) -> HRESULT
	{
        CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        return d3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(dst));
    };

    // vertex buffers
    {
        size_t sizePos = vertexCount * sizeof(XMFLOAT2);
        size_t sizeTex = vertexCount * sizeof(XMFLOAT2);
        size_t sizeCol = vertexCount * sizeof(uint32_t);
        if (FAILED(createBuffer(&bufPos, sizePos))) return E_FAIL;
        if (FAILED(createBuffer(&bufTex, sizeTex))) return E_FAIL;
        if (FAILED(createBuffer(&bufDif, sizeCol))) return E_FAIL;
        this->primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    }

    // index buffer
    if (indexCount > 0)
    {
        size_t sizeIdx = indexCount * sizeof(uint16_t);
        if (FAILED(createBuffer(&bufIdx, sizeIdx))) return E_FAIL;
        this->primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    return S_OK;
}


int VtxSequenceSpine::draw(ID3D12GraphicsCommandList* cmdList, D3D12_GPU_DESCRIPTOR_HANDLE texHandle)
{
	if (!bufPos || !bufTex || !bufDif)
        return E_FAIL;

	// 1. Vertex Buffer 설정
    D3D12_VERTEX_BUFFER_VIEW vbViews[3] = {};

    vbViews[0].BufferLocation = bufPos->GetGPUVirtualAddress();
    vbViews[0].SizeInBytes    = static_cast<UINT>(countVtx * sizeof(XMFLOAT2));
    vbViews[0].StrideInBytes  = sizeof(XMFLOAT2);

    vbViews[1].BufferLocation = bufDif->GetGPUVirtualAddress();
    vbViews[1].SizeInBytes    = static_cast<UINT>(countVtx * sizeof(uint32_t));
    vbViews[1].StrideInBytes  = sizeof(uint32_t);

    vbViews[2].BufferLocation = bufTex->GetGPUVirtualAddress();
    vbViews[2].SizeInBytes    = static_cast<UINT>(countVtx * sizeof(XMFLOAT2));
    vbViews[2].StrideInBytes  = sizeof(XMFLOAT2);

    cmdList->IASetVertexBuffers(0, 3, vbViews);
    cmdList->IASetPrimitiveTopology(primitive);

    // 2. 텍스처 설정: PS의 root descriptor table에 바인딩
    // ※ 이 예시는 root signature가 DescriptorTable 기반이라고 가정합니다.
    // cmdList->SetGraphicsRootDescriptorTable(1, texHandle);

    // 3. 인덱스 여부에 따라 그리기
    if (countIdx > 0 && bufIdx)
    {
        D3D12_INDEX_BUFFER_VIEW ibView = {};
        ibView.BufferLocation = bufIdx->GetGPUVirtualAddress();
        ibView.SizeInBytes    = static_cast<UINT>(countIdx * sizeof(uint16_t));
        ibView.Format         = DXGI_FORMAT_R16_UINT;

        cmdList->IASetIndexBuffer(&ibView);
        cmdList->DrawIndexedInstanced(static_cast<UINT>(countIdx), 1, 0, 0, 0);
    }
    else
    {
        cmdList->DrawInstanced(static_cast<UINT>(countVtx), 1, 0, 0);
    }

    return 0;
}

SceneSpine::SceneSpine()
{
}

SceneSpine::~SceneSpine()
{
	Destroy();
}

void SceneSpine::load(spine::AtlasPage& page, const spine::String& path) {
	auto d3d = IG2GraphicsD3D::instance();
	auto d3dDevice = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto cmdQueue  = std::any_cast<ID3D12CommandQueue*>(d3d->getCommandQueue());

	auto srvDescHeapSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	std::wstring wstr_file = G2::ansiToWstr(path.buffer());

	DirectX::ResourceUploadBatch uploadBatch(d3dDevice);
	uploadBatch.Begin();

	ID3D12Resource* texture = nullptr;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	auto result = DirectX::CreateWICTextureFromFileEx( d3dDevice, uploadBatch, wstr_file.c_str(), 0, D3D12_RESOURCE_FLAG_NONE, DirectX::WIC_LOADER_DEFAULT, &texture);
	if (FAILED(result))
		return;

	auto texIndex = m_srvHeapIndex++;

	auto handleCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), texIndex, srvDescHeapSize);
	d3dDevice->CreateShaderResourceView(texture, &srvDesc, handleCPU);

	// 업로드 수행
	auto future = uploadBatch.End(cmdQueue);
	future.wait();

	page.texture = new UINT(m_srvHeapIndex);
}


void SceneSpine::unload(void* texture) {
	if(texture)
	{
		delete static_cast<UINT*>(texture);
		texture = nullptr;
	}
}

int SceneSpine::Init(const std::any&)
{
	HRESULT hr = S_OK;
	auto d3d        =  IG2GraphicsD3D::instance();
	auto d3dDevice  = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto d3dCmdList = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());

	// 1. Compile vertex shader
	ID3DBlob* vsBlob = G2::DXCompileShaderFromFile("Shaders/spine.fx", "vs_5_0", "main_vtx");
	if (!vsBlob)
		return hr;

	// 2. Compile pixel shader
	ID3DBlob* psBlob = G2::DXCompileShaderFromFile("Shaders/spine.fx", "ps_5_0", "main_pxl");
	if (FAILED(hr))
		return hr;

	// 3. Create root signature
	CD3DX12_DESCRIPTOR_RANGE rangeCBV;
	rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE rangeSampler;
	rangeSampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
	
	CD3DX12_ROOT_PARAMETER rootParams[2];
	rootParams[0].InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParams[1].InitAsDescriptorTable(1, &rangeSampler, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.Init(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr))
		return hr;

	hr = d3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	if (FAILED(hr))
		return hr;

	// 5. Create pipeline state
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { VTX2D_DT::INPUT_LAYOUT.data(), (UINT)VTX2D_DT::INPUT_LAYOUT.size() };
		psoDesc.pRootSignature = m_rootSignature;
		psoDesc.VS = { reinterpret_cast<BYTE*>(vsBlob->GetBufferPointer()), vsBlob->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<BYTE*>(psBlob->GetBufferPointer()), psBlob->GetBufferSize() };
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
	hr = d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	if (FAILED(hr))
		return hr;

	// 6. Create constant buffer (MVP)
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(256);
	hr = d3dDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_cnstMVP));
	if (FAILED(hr))
		return hr;

	// 7. Create SRV descriptor heap for textures
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 32; // 필요한 텍스처 수만큼 조정
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
	if (FAILED(hr))
		return hr;


	// 7. Create sampler descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = 1;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = d3dDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap));
	if (FAILED(hr))
		return hr;

	D3D12_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDevice->CreateSampler(&sampDesc, m_samplerHeap->GetCPUDescriptorHandleForHeapStart());

	InitSpine();

	return S_OK;
}

int SceneSpine::Destroy()
{
	G2::SAFE_RELEASE(	m_cnstMVP			);
	G2::SAFE_RELEASE(	m_rootSignature		);
	G2::SAFE_RELEASE(	m_pipelineState		);
	if(!m_spineAnimations.empty())
	{
		m_spineAnimations.clear();
	}
	for(auto& [key, sqc] :  m_spineSequence)
	{
		delete sqc;
	}
	m_spineSequence.clear();
	G2::SAFE_DELETE(	m_spineSkeleton		);
	G2::SAFE_DELETE(	m_spineAniState		);
	G2::SAFE_DELETE(	m_spineSkeletonData	);
	G2::SAFE_DELETE(	m_spineAtlas		);
	G2::SAFE_RELEASE(	m_srvHeap			);

	return S_OK;
}

int SceneSpine::Update(const std::any& t)
{
	GameTimer gt = std::any_cast<GameTimer>(t);
	auto deltaTime = gt.DeltaTime();

	// Update and apply the animation state to the skeleton
	m_spineAniState->update(deltaTime);
	m_spineAniState->apply(*m_spineSkeleton);

	// Update the skeleton time (used for physics)
	m_spineSkeleton->update(deltaTime);

	m_spineSkeleton->updateWorldTransform(spine::Physics_Update);

	// update spine sequence
	UpdateSpineSequence();

	// update vertex
	UpdateSpineBuffer();

	// 4. setup the world, view, projection matrix
// View, Projection Matrix
// Initialize the view matrix
	XMVECTOR Eye=XMVectorSet(0.0f, 0.0f, -1000.0f, 0.0f);
	XMVECTOR At=XMVectorSet (0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up=XMVectorSet (0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX m_mtView=XMMatrixLookAtLH(Eye, At, Up);
	// Initialize the projection matrix

	auto d3d        = IG2GraphicsD3D::instance();
	auto screeSize  = *std::any_cast<::SIZE*>(d3d->getAttrib(EG2GRAPHICS_D3D::ATT_SCREEN_SIZE));
	XMMATRIX m_mtProj=XMMatrixPerspectiveFovLH(XM_PIDIV4, screeSize.cx / (FLOAT)screeSize.cy, 1.0f, 5000.0f);
	// 4.2 Initialize the world matrix
	XMMATRIX m_mtWorld=XMMatrixIdentity();

	XMMATRIX mvp = m_mtWorld * m_mtView * m_mtProj;
	SetMVP(mvp);
	return S_OK;
}

int SceneSpine::Render()
{
	auto d3d      =  IG2GraphicsD3D::instance();
	auto device   = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto cmdList  = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());

	cmdList->SetGraphicsRootSignature(m_rootSignature);
	cmdList->SetPipelineState(m_pipelineState);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->SetGraphicsRootConstantBufferView(0,m_cnstMVP->GetGPUVirtualAddress());

	for(auto& [order,sqc] : m_spineSequence) {
		sqc->draw(cmdList,m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	}

	return S_OK;
}

void SceneSpine::SetMVP(const XMMATRIX& tmMVP) {
	m_tmMVP = tmMVP;
	UINT8* ptr = nullptr;
	CD3DX12_RANGE readRange(0,0);
	m_cnstMVP->Map(0,&readRange,reinterpret_cast<void**>(&ptr));
	memcpy(ptr,&tmMVP,sizeof(XMMATRIX));
	m_cnstMVP->Unmap(0,nullptr);
}

void SceneSpine::InitSpine() {
	Bone::setYDown(false);
	m_spineAtlas = new Atlas("assets/spine/raptor/raptor-pma.atlas",this);
	SkeletonJson json(m_spineAtlas);
	m_spineSkeletonData = json.readSkeletonDataFile("assets/spine/raptor/raptor-pro.json");
	m_spineSkeleton = new Skeleton(m_spineSkeletonData);
	m_spineSkeleton->setPosition(0,-300);
	m_spineSkeleton->setScaleX(0.6f);
	m_spineSkeleton->setScaleY(0.6f);

	auto& animations = m_spineSkeletonData->getAnimations();
	for(int i = 0; i < animations.size(); ++i)
		m_spineAnimations.push_back(animations[i]->getName().buffer());

	AnimationStateData aniData(m_spineSkeletonData);
	aniData.setDefaultMix(0.2f);
	m_spineAniState = new AnimationState(&aniData);
	m_spineAniState->setAnimation(0,"gun-holster",false);
	m_spineAniState->addAnimation(0,"roar",false,0.8f);
	m_spineAniState->addAnimation(0,"walk",true,2.1f);
}

void SceneSpine::UpdateSpineSequence()
{
	auto drawOrder = m_spineSkeleton->getDrawOrder();
	for(size_t i = 0; i < drawOrder.size(); ++i)
	{
		spine::Slot* slot = drawOrder[i];
		spine::Attachment* attachment = slot->getAttachment();
		if(!attachment)
			continue;
		if(attachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
		{
			SetupSpineSequence((int)i, attachment, VtxSequenceSpine::ESPINE_MESH_ATTACH);
		}
		else if(attachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
		{
			SetupSpineSequence((int)i, attachment, VtxSequenceSpine::ESPINE_MESH_REGION);
		}
	}
}

void SceneSpine::UpdateSpineBuffer()
{
	auto d3d		=  IG2GraphicsD3D::instance();
	auto d3dDevice	= std::any_cast<ID3D12Device*>(d3d->getDevice());

    for (auto [drawOrder, curSqc] : m_spineSequence)
    {
        spine::Slot* slot = m_spineSkeleton->getDrawOrder()[drawOrder];
        spine::Attachment* attachment = slot->getAttachment();
        if (!attachment)
            continue;

        if (curSqc->meshType == VtxSequenceSpine::ESPINE_ATTACHMENT_TYPE::ESPINE_MESH_ATTACH)
        {
            auto* mesh = static_cast<spine::MeshAttachment*>(attachment);

            // 색상 계산
            spine::Color c = slot->getColor();
            spine::Color mColor = mesh->getColor();
            c.a *= mColor.a; c.r *= mColor.r; c.g *= mColor.g; c.b *= mColor.b;

            uint32_t rgba =
                (uint32_t(c.a * 255) << 24) |
                (uint32_t(c.b * 255) << 16) |
                (uint32_t(c.g * 255) << 8) |
                (uint32_t(c.r * 255) << 0);

            spine::TextureRegion* texRegion = mesh->getRegion();
            if (!texRegion) continue;

            auto* atlasRegion = reinterpret_cast<spine::AtlasRegion*>(texRegion);
            auto* texSRV = atlasRegion->page->texture;
            if (!texSRV) continue;

            // 위치
            {
                size_t bufSize = mesh->getWorldVerticesLength();
                float* ptr = nullptr;
                if (SUCCEEDED(curSqc->bufPos->Map(0, nullptr, reinterpret_cast<void**>(&ptr)))) {
                    mesh->computeWorldVertices(*slot, 0, bufSize, ptr, 0, 2);
                    curSqc->bufPos->Unmap(0, nullptr);
                }
            }

            // color
            {
                size_t vtxCount = mesh->getWorldVerticesLength() / 2;
                uint32_t* ptr = nullptr;
                if (SUCCEEDED(curSqc->bufDif->Map(0, nullptr, reinterpret_cast<void**>(&ptr)))) {
                    G2::avx2_memset32(ptr, rgba, vtxCount);
                    curSqc->bufDif->Unmap(0, nullptr);
                }
            }

            // UVs
            {
                const float* uvs = mesh->getUVs().buffer();
                size_t bufSize = mesh->getUVs().size();
                void* ptr = nullptr;
                if (SUCCEEDED(curSqc->bufTex->Map(0, nullptr, &ptr))) {
                    G2::avx2_memcpy(ptr, uvs, sizeof(float) * bufSize);
                    curSqc->bufTex->Unmap(0, nullptr);
                }
            }

            // indices
            {
                const uint16_t* indices = mesh->getTriangles().buffer();
                UINT indexCount = mesh->getTriangles().size();
                void* ptr = nullptr;
                if (SUCCEEDED(curSqc->bufIdx->Map(0, nullptr, &ptr))) {
                    G2::avx2_memcpy(ptr, indices, sizeof(uint16_t) * indexCount);
                    curSqc->bufIdx->Unmap(0, nullptr);
                }
            }
        }
        else if (curSqc->meshType == VtxSequenceSpine::ESPINE_ATTACHMENT_TYPE::ESPINE_MESH_REGION)
        {
            auto* region = static_cast<spine::RegionAttachment*>(attachment);
            spine::TextureRegion* texRegion = region->getRegion();
            if (!texRegion) continue;

            auto* atlasRegion = reinterpret_cast<spine::AtlasRegion*>(texRegion);
            auto* texSRV = atlasRegion->page->texture;
            if (!texSRV) continue;

            spine::Color c = m_spineSkeleton->getColor();
            spine::Color rColor = region->getColor();
            c.a *= slot->getColor().a * rColor.a;
            c.r *= slot->getColor().r * rColor.r;
            c.g *= slot->getColor().g * rColor.g;
            c.b *= slot->getColor().b * rColor.b;

            uint32_t rgba =
                (uint32_t(c.a * 255) << 24) |
                (uint32_t(c.r * 255) << 16) |
                (uint32_t(c.g * 255) << 8) |
                (uint32_t(c.b * 255) << 0);

            // 위치
            {
                float* ptr = nullptr;
                if (SUCCEEDED(curSqc->bufPos->Map(0, nullptr, reinterpret_cast<void**>(&ptr)))) {
                    region->computeWorldVertices(*slot, ptr, 0, 2);
                    curSqc->bufPos->Unmap(0, nullptr);
                }
            }

            // UVs
            {
                const float* uvs = region->getUVs().buffer();
                size_t bufSize = region->getUVs().size();
                void* ptr = nullptr;
                if (SUCCEEDED(curSqc->bufTex->Map(0, nullptr, &ptr))) {
                    G2::avx2_memcpy(ptr, uvs, sizeof(float) * bufSize);
                    curSqc->bufTex->Unmap(0, nullptr);
                }
            }

            // 색상
            {
				size_t vtxCount = 4;
                uint32_t* ptr = nullptr;
                if (SUCCEEDED(curSqc->bufDif->Map(0, nullptr, reinterpret_cast<void**>(&ptr)))) {
                    G2::avx2_memset32(ptr, rgba, vtxCount);
                    curSqc->bufDif->Unmap(0, nullptr);
                }
            }
        }
    }
}

void SceneSpine::SetupSpineSequence(int order, void* attachment, VtxSequenceSpine::ESPINE_ATTACHMENT_TYPE attachmentType)
{
	size_t vertexCount = 4;
	UINT   indexCount  = 0;
	VtxSequenceSpine* sequenceCur{};
	auto itr = m_spineSequence.find(order);
	if(itr != m_spineSequence.end())
	{
		sequenceCur = itr->second;
	}

	// 메시 타입 체크.
	if(VtxSequenceSpine::ESPINE_ATTACHMENT_TYPE::ESPINE_MESH_ATTACH == attachmentType)
	{
		auto* mesh = static_cast<spine::MeshAttachment*>(attachment);
		vertexCount = mesh->getWorldVerticesLength()/2;
		indexCount = mesh->getTriangles().size();
	}
	else if(VtxSequenceSpine::ESPINE_ATTACHMENT_TYPE::ESPINE_MESH_REGION == attachmentType)
	{
		vertexCount = 4;
		indexCount  = 0;
	}
	else
	{
		// 존재하는데 어느 타입도 아니라면 삭제
		if(sequenceCur)
		{
			delete sequenceCur;
		}
		m_spineSequence.erase(itr);
	}

	// 시퀀스 생성
	if(!sequenceCur)
	{
		sequenceCur = new VtxSequenceSpine;
		m_spineSequence.emplace(order,sequenceCur);
	}
	else if(   sequenceCur->meshType == attachmentType				// 같은 메시 타입
			&& sequenceCur->countVtx == vertexCount					// 같은 버텍스 크기
			&& sequenceCur->countIdx == indexCount					// 같은 인덱스 크기
	)
	{
		// 동일 정보라 할 일이 없음.
		return;
	}
	else
	{
		// 삭제만
		delete sequenceCur;
		sequenceCur = new VtxSequenceSpine;
		m_spineSequence[order] = sequenceCur;
	}
	// 새로운 버텍스 인덱스 생성
	sequenceCur->resourceBinding(order, attachment, attachmentType, vertexCount, indexCount);
}
