
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

#define KEY_TEX_00	"TEX_0"
#define KEY_TEX_01	"TEX_1"
#define KEY_TEX_02	"TEX_2"
#define KEY_TEX_03	"TEX_3"
#define KEY_TEX_04	"TEX_4"
#define KEY_TEX_05	"TEX_5"
#define KEY_TEX_06	"TEX_6"
#define KEY_TEX_07	"TEX_7"
#define KEY_TEX_08	"TEX_8"
#define KEY_TEX_09	"TEX_9"
#define KEY_TEX_0A	"TEX_A"
#define KEY_TEX_0B	"TEX_B"

struct TD3D_TEST1
{
	std::string				n;		// name
	ID3D12RootSignature* r{};	// root signature
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
	inline static const char* keys[]
	{
		"TEX_0", "TEX_1", "TEX_2", "TEX_3", "TEX_4", "TEX_5", "TEX_6", "TEX_7", "TEX_8", "TEX_9", "TEX_A", "TEX_B",
	};
};

class FactorySignature : public IG2Factory<FactorySignature, TD3D_ROOTSIGNATURE >
{
public:
	static FactorySignature* instance();
	TD3D_ROOTSIGNATURE* ResourceLoad();
public:
	TD3D_ROOTSIGNATURE* ResourceFind(const std::string& name) override;
	// find ID3D12Resource*
	ID3D12RootSignature* FindRes(const std::string& name);
protected:
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> samplerRegister();
protected:
	bool	m_isLoaded{};
};

} // namespace G2

#endif // __G2_FACTORY_ROOT_SIGNATURE_H__
