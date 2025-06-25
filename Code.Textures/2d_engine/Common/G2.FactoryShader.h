
#pragma once
#ifndef __G2_FACTORY_SHADER_H__
#define __G2_FACTORY_SHADER_H__

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "G2.Factory.h"

using namespace Microsoft::WRL;

namespace G2 {

struct TD3D_SHADER
{
	std::string			name;	// shader name
	std::string			file;	// shader file
	std::string			sm	;	// shader model vs_5_0, ps_5_0
	std::string			ep	;	// shader main function entry point
	ComPtr<ID3DBlob>	rs{};	// resource
};

class FactoryShader : public IG2Factory<TD3D_SHADER>
{
public:
	static FactoryShader* instance();
protected:
	// optional: name, file, shader model, entrypoint
	TD3D_SHADER* ResourceLoad(const std::any& optional)  override;
	TD3D_SHADER* ResourceFind(const std::string& name)   override;
	int ResourceUnLoad(const std::string& name)     override;
	int ResourceUnLoadAll()                         override;
};

} // namespace G2

#endif // __G2_FACTORY_SHADER_H__
