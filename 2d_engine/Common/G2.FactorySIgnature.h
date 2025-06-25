
#pragma once
#ifndef __G2_FACTORY_ROOT_SIGNATURE_H__
#define __G2_FACTORY_ROOT_SIGNATURE_H__

#include <array>
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include "G2.Factory.h"

using namespace Microsoft::WRL;

namespace G2 {

enum ED3D_ROOTSIGNATURE
{
	ED3D_TEX0_CONST06	= 0X0006,
	ED3D_TEX1_CONST08	= 0X0108,
	ED3D_TEX2_CONST08	= 0X0208,
	ED3D_TEX3_CONST08	= 0X0308,
	ED3D_TEX4_CONST08	= 0X0408,
	ED3D_TEX5_CONST08	= 0X0508,
	ED3D_TEX6_CONST08	= 0X0608,
	ED3D_TEX7_CONST08	= 0X0708,
	ED3D_TEX8_CONST08	= 0X0808,
};

struct TD3D_ROOTSIGNATURE
{
	std::string				n;		// name
	ID3D12RootSignature*	r{};	// root signature
	~TD3D_ROOTSIGNATURE() {
		if (r) {
			r->Release();
			r = {};
		}
	}
};

class FactorySignature : public IG2Factory<FactorySignature, TD3D_ROOTSIGNATURE >
{
public:
	static FactorySignature* instance();

	TD3D_ROOTSIGNATURE* ResourceLoad();

public:
	TD3D_ROOTSIGNATURE* ResourceFind(const std::string& name) override;
	int ResourceUnLoad(const std::string& name) override;

protected:
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> staticSamplers();
};

} // namespace G2

#endif // __G2_FACTORY_ROOT_SIGNATURE_H__
