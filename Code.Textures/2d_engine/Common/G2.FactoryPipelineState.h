
#pragma once
#ifndef __G2_FACTORY_PIPELINESTATE_H__
#define __G2_FACTORY_PIPELINESTATE_H__

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "G2.Factory.h"

using namespace Microsoft::WRL;

namespace G2 {

struct TD3D_PS							// pipeline state
{
	std::string					name;	// shader name
	ComPtr<ID3D12PipelineState>	ps{};	// pipeline state
};

class FactoryPipelineState : public IG2Factory<FactoryPipelineState, TD3D_PS>
{
public:
	static FactoryPipelineState* instance();

	TD3D_PS* ResourceLoad(const std::string& name);

public:
	TD3D_PS* ResourceFind(const std::string& name) override;
};

} // namespace G2

#endif // __G2_FACTORY_PIPELINESTATE_H__

