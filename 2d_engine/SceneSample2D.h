
#pragma once
#ifndef _SceneSample2D_H_
#define _SceneSample2D_H_

#include <map>
#include <vector>
#include <any>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>

#include "Common/G2.Constants.h"
#include "Common/G2.Geometry.h"
#include "common/G2.ConstantsWin.h"
#include "common/G2.Util.h"
#include "AppCommon.h"
#include "AppCommonXTK.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace G2;

class SceneSample2D: public G2::IG2Scene
{
protected:
	

public:
	SceneSample2D();
	virtual ~SceneSample2D();

	int		Type()						override { return EAPP_SCENE::EAPP_SCENE_SPINE; }
	int		Init(const std::any& ={})	override;
	int		Destroy()					override;
	int		Update(const std::any& t)	override;
	int		Render()					override;
};

#endif
