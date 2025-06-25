
#pragma once
#ifndef __G2_FACTORY_TEXTURE_H__
#define __G2_FACTORY_TEXTURE_H__

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "G2.Factory.h"

using namespace Microsoft::WRL;

namespace G2 {

struct TD3D_TEXTURE
{
	std::string name;
	std::string file;
	ComPtr<ID3D12Resource>	rs{};	// resource
	ComPtr<ID3D12Resource>	uh{};	// upload heap
};

class FactoryTexture : public IG2Factory<TD3D_TEXTURE>
{
public:
	static FactoryTexture* instance();
protected:
	TD3D_TEXTURE* ResourceLoad(const std::any& optional) override;
	TD3D_TEXTURE* ResourceFind(const std::string& name) override;
	int ResourceUnLoad(const std::string& name)         override;
	int ResourceUnLoadAll()                             override;
};

} // namespace G2
#endif // __G2_FACTORY_TEXTURE_H__
