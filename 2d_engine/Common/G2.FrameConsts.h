#pragma once
#ifndef __G2_FRAME_CONSTS_H__
#define __G2_FRAME_CONSTS_H__

#include <array>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12/d3dx12.h>

#include "UploadBuffer.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace G2 {

struct CB_RESOURCE // const buffer reousrce
{
	std::string name;
};

class FrameConst
{
protected:
	//std::unordered_map< std::unique_ptr<UploadBuffer<CB_RESOURCE> > >	m_vecResource;
public:
	FrameConst();
	~FrameConst();

	void Add   (const std::string& name);
	void Remove(const std::string& name);
	void RemoveAll();
	void UpdateAll();
};

} // namespace G2

#endif // __G2_FRAME_CONSTS_H__
