
#pragma once
#ifndef __G2_FACTORY_PIPELINESTATE_H__
#define __G2_FACTORY_PIPELINESTATE_H__

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "G2.Factory.h"

using namespace Microsoft::WRL;

namespace G2 {

inline static const char* D3D_PIPELINESTATE[] = {
	// for 3d
	"PLS_OPAQUE",
	"PLS_TRANSPARENT",
	"PLS_ALPHATEST",
	"PLS_TREES",
	// for 2d
	"PLS2D_OPAQUE",
	"PLS2D_TRANSPARENT",
	"PLS2D_ALPHATEST",
	// ...
};

struct TD3D_PIPELINESTATE
{
	std::string				n;		// name
	ID3D12PipelineState*	r{};	// pipeline state
	~TD3D_PIPELINESTATE() {
		if (r) {
			r->Release();
			r = {};
		}
	}
};

class FactoryPipelineState : public IG2Factory<FactoryPipelineState, TD3D_PIPELINESTATE>
{
public:
	static FactoryPipelineState* instance();
	TD3D_PIPELINESTATE* ResourceLoad();
	TD3D_PIPELINESTATE* Add(std::string& name, ID3D12PipelineState* pls, bool replace=false);
public:
	TD3D_PIPELINESTATE* ResourceFind(const std::string& name) override;
	ID3D12PipelineState* FindRes(const std::string& name);
protected:
	bool	m_isLoaded{};
};

} // namespace G2

#endif // __G2_FACTORY_PIPELINESTATE_H__

