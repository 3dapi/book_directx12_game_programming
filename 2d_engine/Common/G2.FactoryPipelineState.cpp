
#include <tuple>
#include <string>
#include "d3dUtil.h"
#include "G2.ConstantsWin.h"
#include "G2.FactoryShader.h"
#include "G2.FactorySignature.h"
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
	int hr = S_OK;
	auto d3d = IG2GraphicsD3D::instance();
	auto d3dDevice       = std::any_cast<ID3D12Device*  >(d3d->getDevice());
	auto d3dFmtBack      = *std::any_cast<DXGI_FORMAT*  >(d3d->getAttrib(EG2GRAPHICS_D3D::ATT_DEVICE_BACKBUFFER_FORAT));
	auto d3dFmtDepth     = *std::any_cast<DXGI_FORMAT*  >(d3d->getAttrib(EG2GRAPHICS_D3D::ATT_DEVICE_DEPTH_STENCIL_FORAT));
	auto d3dMsaa4State   = *std::any_cast<bool*         >(d3d->getAttrib(EG2GRAPHICS_D3D::ATT_DEVICE_MSAASTATE4X_STATE));
	auto d3dMsaa4Quality = *std::any_cast<UINT*         >(d3d->getAttrib(EG2GRAPHICS_D3D::ATT_DEVICE_MSAASTATE4X_QUALITY));
	auto shader_manager = FactoryShader::instance();

	// for opaque objects
	{
		ID3D12PipelineState* pls{};
		auto shader_vs = shader_manager->FindRes("standardVS");
		auto shader_ps = shader_manager->FindRes("opaquePS");
		auto signature = FactorySignature::instance()->FindRes(KEY_TEX_01);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC plsDesc{};
			plsDesc.InputLayout = { VTX_NT::INPUT_LAYOUT.data(), (UINT)VTX_NT::INPUT_LAYOUT.size() };
			plsDesc.pRootSignature = signature;
			plsDesc.VS = { shader_vs->GetBufferPointer(), shader_vs->GetBufferSize() };
			plsDesc.PS = { shader_ps->GetBufferPointer(), shader_ps->GetBufferSize() };
			plsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			plsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			plsDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			plsDesc.SampleMask = UINT_MAX;
			plsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			plsDesc.NumRenderTargets = 1;
			plsDesc.RTVFormats[0] = d3dFmtBack;
			plsDesc.SampleDesc.Count = d3dMsaa4State ? 4 : 1;
			plsDesc.SampleDesc.Quality = d3dMsaa4State ? (d3dMsaa4Quality - 1) : 0;
			plsDesc.DSVFormat = d3dFmtDepth;
		hr = d3dDevice->CreateGraphicsPipelineState(&plsDesc, IID_PPV_ARGS(&pls));
		if (SUCCEEDED(hr))
		{
			// 저장
			string name = "PLS_OPAQUE";
			m_db[name] = std::make_unique<TD3D_PIPELINESTATE>();
			m_db[name]->n = name;
			m_db[name]->r = pls;
		}
	}
	// for transparent objects
	{
		ID3D12PipelineState* pls{};
		auto shader_vs = shader_manager->FindRes("standardVS");
		auto shader_ps = shader_manager->FindRes("opaquePS");
		auto signature = FactorySignature::instance()->FindRes(KEY_TEX_01);
		D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc{};
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
		D3D12_GRAPHICS_PIPELINE_STATE_DESC plsDesc = {};
			plsDesc.InputLayout = { VTX_NT::INPUT_LAYOUT.data(), (UINT)VTX_NT::INPUT_LAYOUT.size() };
			plsDesc.pRootSignature = signature;
			plsDesc.VS = { shader_vs->GetBufferPointer(), shader_vs->GetBufferSize() };
			plsDesc.PS = { shader_ps->GetBufferPointer(), shader_ps->GetBufferSize() };
			plsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			plsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			plsDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			plsDesc.SampleMask = UINT_MAX;
			plsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			plsDesc.NumRenderTargets = 1;
			plsDesc.RTVFormats[0] = d3dFmtBack;
			plsDesc.SampleDesc.Count = d3dMsaa4State ? 4 : 1;
			plsDesc.SampleDesc.Quality = d3dMsaa4State ? (d3dMsaa4Quality - 1) : 0;
			plsDesc.DSVFormat = d3dFmtDepth;
			plsDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
		hr = d3dDevice->CreateGraphicsPipelineState(&plsDesc, IID_PPV_ARGS(&pls));
		if (SUCCEEDED(hr))
		{
			// 저장
			string name = "PLS_TRANSPARENT";
			m_db[name] = std::make_unique<TD3D_PIPELINESTATE>();
			m_db[name]->n = name;
			m_db[name]->r = pls;
		}
	}
	// for alpha tested objects
	{
		ID3D12PipelineState* pls{};
		auto shader_vs = shader_manager->FindRes("standardVS");
		auto shader_ps = shader_manager->FindRes("alphaTestedPS");
		auto signature = FactorySignature::instance()->FindRes(KEY_TEX_01);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC plsDesc {};
			plsDesc.InputLayout = { VTX_NT::INPUT_LAYOUT.data(), (UINT)VTX_NT::INPUT_LAYOUT.size() };
			plsDesc.pRootSignature = signature;
			plsDesc.VS = { shader_vs->GetBufferPointer(), shader_vs->GetBufferSize() };
			plsDesc.PS = { shader_ps->GetBufferPointer(), shader_ps->GetBufferSize() };
			plsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			plsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			plsDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			plsDesc.SampleMask = UINT_MAX;
			plsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			plsDesc.NumRenderTargets = 1;
			plsDesc.RTVFormats[0] = d3dFmtBack;
			plsDesc.SampleDesc.Count = d3dMsaa4State ? 4 : 1;
			plsDesc.SampleDesc.Quality = d3dMsaa4State ? (d3dMsaa4Quality - 1) : 0;
			plsDesc.DSVFormat = d3dFmtDepth;
			plsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		hr = d3dDevice->CreateGraphicsPipelineState(&plsDesc, IID_PPV_ARGS(&pls));
		if (SUCCEEDED(hr))
		{
			// 저장
			string name = "PLS_ALPHATEST";
			m_db[name] = std::make_unique<TD3D_PIPELINESTATE>();
			m_db[name]->n = name;
			m_db[name]->r = pls;
		}
	}
	
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
