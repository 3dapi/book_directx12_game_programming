
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
	ComPtr<ID3DBlob>	r {};	// resource
	~TD3D_SHADER() {
		r.Reset();
	}
};

class FactoryShader : public IG2Factory<FactoryShader, TD3D_SHADER>
{
public:
	static FactoryShader* instance();

	TD3D_SHADER* ResourceLoad(const std::string& name,
		const std::string& file,
		const std::string& sm,
		const std::string& ep,
		const void* macros = nullptr);

public:
	// 필요 시 아래 함수들을 오버라이드 가능
	TD3D_SHADER* ResourceFind(const std::string& name) override;
	int ResourceUnLoad(const std::string& name) override;
	// find ID3DBlob*
	ID3DBlob* FindRes(const std::string& name);
};

} // namespace G2

#endif // __G2_FACTORY_SHADER_H__
