
#include <tuple>
#include <string>
#include "d3dUtil.h"
#include "G2.ConstantsWin.h"
#include "d3dUtil.h"
#include "G2.Util.h"
#include "G2.FactorySignature.h"

using std::string;
namespace G2 {

FactorySignature* FactorySignature::instance()
{
	static FactorySignature inst;
	if (!inst.m_isLoaded)
	{
		inst.Load();
	}
	return &inst;
}

TD3D_ROOTSIGNATURE* FactorySignature::ResourceLoad()
{
	auto d3d            = IG2GraphicsD3D::instance();
	auto d3dDevice      = std::any_cast<ID3D12Device*>(d3d->getDevice());

	constexpr std::array<ED3D_ROOTSIGNATURE, 9> types = {
		ED3D_TEX0_CONST06,
		ED3D_TEX1_CONST08,
		ED3D_TEX2_CONST08,
		ED3D_TEX3_CONST08,
		ED3D_TEX4_CONST08,
		ED3D_TEX5_CONST08,
		ED3D_TEX6_CONST08,
		ED3D_TEX7_CONST08,
		ED3D_TEX8_CONST08,
	};

	for (auto type : types)
	{
		const int texrSize  = (type & 0xFF00) >> 8;
		const int constSize = (type & 0x00FF);
		const int rangeSize = texrSize + constSize;

		std::vector<CD3DX12_DESCRIPTOR_RANGE> texTable(rangeSize);
		std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameter(rangeSize);

		if (0 < texrSize)
		{
			for (int i = 0; i < texrSize; ++i)
			{
				texTable[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i);
				slotRootParameter[i].InitAsDescriptorTable(1, &texTable[i], D3D12_SHADER_VISIBILITY_PIXEL);		// register t~~
			}
		}

		for (int i = texrSize, j=0; i < rangeSize; ++i, ++j)
		{
			slotRootParameter[i].InitAsConstantBufferView(j);		// register b~~
		}

		auto samps = samplerRegister();

		// => D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc((UINT)slotRootParameter.size(), slotRootParameter.data(), (UINT)samps.size(), samps.data(), (D3D12_ROOT_SIGNATURE_FLAGS)0x0001);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig{};
		ComPtr<ID3DBlob> errorBlob{};
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			continue;
		}
		ID3D12RootSignature* rootSignature{};
		hr = d3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
		if (FAILED(hr))
		{
			char buf_err[128]{};
			::sprintf_s(buf_err, "CreateRootSignature failed: type: 0x%04X", type);
			::OutputDebugStringA(buf_err);
			continue;
		}

		// 이름 : "TEX0n"
		char name[32];
		sprintf_s(name, "TEX_%X", texrSize);

		// 저장
		m_db[name] = std::make_unique<TD3D_ROOTSIGNATURE>();
		m_db[name]->n = name;
		m_db[name]->r = std::move(rootSignature);
		continue;
	}
	auto ret = m_db["TEX_0"].get();
	m_isLoaded = true;
	return ret;
}

TD3D_ROOTSIGNATURE* FactorySignature::ResourceFind(const string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}
	return nullptr;
}

ID3D12RootSignature* FactorySignature::FindRes(const std::string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get()->r;
	}
	return {};
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> FactorySignature::samplerRegister()
{
	auto d3dDevice = std::any_cast<ID3D12Device*>(IG2GraphicsD3D::instance()->getDevice());

	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

} // namespace G2
