
#include <tuple>
#include <string>
#include "d3dUtil.h"
#include "G2.ConstantsWin.h"
#include "G2.FactoryPipelineState.h"
#include "G2.Geometry.h"
#include "G2.Util.h"


using std::string;
namespace G2 {

FactoryPipelineState* FactoryPipelineState::instance()
{
	static FactoryPipelineState inst;
	if (!inst.m_isLoaded)
	{
		inst.Load();
	}
	return &inst;
}

struct VertexBuffer
	{
		float    p[3];     // POSITION (12 bytes)
		uint8_t  d[4];     // COLOR (4 bytes)
		float    t[2];     // TEXCOORD (8 bytes)
		uint32_t i;        // TEXSLICE (4 bytes)
	};
TD3D_PIPELINESTATE* FactoryPipelineState::ResourceLoad()
{
	/*
	auto d3d            = IG2GraphicsD3D::instance();
	auto d3dDevice      = std::any_cast<ID3D12Device*>(d3d->getDevice());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc{};
		opaquePsoDesc.InputLayout = { layout2d.data(), (UINT)layout2d.size() };
		opaquePsoDesc.pRootSignature = m_rootSignature.Get();
		opaquePsoDesc.VS = { pshader_vs->GetBufferPointer(), pshader_vs->GetBufferSize() };
		opaquePsoDesc.PS = { pshader_ps->GetBufferPointer(), pshader_ps->GetBufferSize() };
		opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		opaquePsoDesc.SampleMask = UINT_MAX;
		opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		opaquePsoDesc.NumRenderTargets = 1;
		opaquePsoDesc.RTVFormats[0] = d3dFmtBack;
		opaquePsoDesc.SampleDesc.Count = d3dMsaa4State ? 4 : 1;
		opaquePsoDesc.SampleDesc.Quality = d3dMsaa4State ? (d3dMsaa4Quality - 1) : 0;
		opaquePsoDesc.DSVFormat = d3dFmtDepth;
	hr = d3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_pipelineState["OPAQUE"]));
	if (FAILED(hr))
		return nullptr;

	//
	// PSO for transparent objects
	//
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = {};
	transparentPsoDesc.InputLayout = { layout2d.data(), (UINT)layout2d.size() };
	transparentPsoDesc.pRootSignature = m_rootSignature.Get();
	transparentPsoDesc.VS = { pshader_vs->GetBufferPointer(), pshader_vs->GetBufferSize() };
	transparentPsoDesc.PS = { pshader_ps->GetBufferPointer(), pshader_ps->GetBufferSize() };
	transparentPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	transparentPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	transparentPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	transparentPsoDesc.SampleMask = UINT_MAX;
	transparentPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	transparentPsoDesc.NumRenderTargets = 1;
	transparentPsoDesc.RTVFormats[0] = d3dFmtBack;
	transparentPsoDesc.SampleDesc.Count = d3dMsaa4State ? 4 : 1;
	transparentPsoDesc.SampleDesc.Quality = d3dMsaa4State ? (d3dMsaa4Quality - 1) : 0;
	transparentPsoDesc.DSVFormat = d3dFmtDepth;
	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	hr = d3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_pipelineState["TRANSPARENT"]));
	if (FAILED(hr))
		return hr;

	//
	// PSO for alpha tested objects
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = {};
	alphaTestedPsoDesc.InputLayout = { layout2d.data(), (UINT)layout2d.size() };
	alphaTestedPsoDesc.pRootSignature = m_rootSignature.Get();
	alphaTestedPsoDesc.VS = { pshader_vs->GetBufferPointer(), pshader_vs->GetBufferSize() };
	alphaTestedPsoDesc.PS = { pshader_ps->GetBufferPointer(), pshader_ps->GetBufferSize() };
	alphaTestedPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	alphaTestedPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	alphaTestedPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	alphaTestedPsoDesc.SampleMask = UINT_MAX;
	alphaTestedPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	alphaTestedPsoDesc.NumRenderTargets = 1;
	alphaTestedPsoDesc.RTVFormats[0] = d3dFmtBack;
	alphaTestedPsoDesc.SampleDesc.Count = d3dMsaa4State ? 4 : 1;
	alphaTestedPsoDesc.SampleDesc.Quality = d3dMsaa4State ? (d3dMsaa4Quality - 1) : 0;
	alphaTestedPsoDesc.DSVFormat = d3dFmtDepth;
	auto pshader_a = shader_manager->FindRes("alphaTestedPS");

	alphaTestedPsoDesc.PS = { pshader_a->GetBufferPointer(), pshader_a->GetBufferSize() };
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	hr = d3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&m_pipelineState["ALPHATEST"]));
	if (FAILED(hr))
		return hr;

*/
	auto ret = m_db["OPAQUE"].get();
	m_isLoaded = true;
	return ret;
}

TD3D_PIPELINESTATE* FactoryPipelineState::Add(std::string& name, ID3D12PipelineState* pls, bool replace)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		// 있으면 리턴
		if (!replace)
		{
			return {};
		}
		else
		{
			// 삭제.
			m_db.erase(itr);
		}
	}
	// 저장.
	m_db[name] = std::make_unique<TD3D_PIPELINESTATE>();
	m_db[name]->n = name;
	m_db[name]->r = pls;
	auto item = this->Find(name);
	return item;
}

TD3D_PIPELINESTATE* FactoryPipelineState::ResourceFind(const string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}
	return {};
}

ID3D12PipelineState* FactoryPipelineState::FindRes(const std::string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get()->r;
	}
	return {};
}

} // namespace G2
