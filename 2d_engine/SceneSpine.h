#pragma once
#ifndef _SceneSpine_h_
#define _SceneSpine_h_

#include <any>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <VertexTypes.h>

#include "Common/G2.Constants.h"
#include "Common/G2.Geometry.h"
#include "common/G2.ConstantsWin.h"
#include "Common/GameTimer.h"
#include "AppCommon.h"
#include "AppCommonXTK.h"
#include "StepTimer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;


class SceneSpine : public IG2Scene
{
public:
	SceneSpine()  noexcept;
	virtual ~SceneSpine();

	int		Type()									override { return EAPP_SCENE::EAPP_SCENE_XTK; }
	int		Init(const std::any& initialValue ={})	override;
	int		Destroy()								override;
	int		Update(const std::any& t)				override;
	int		Render()								override;
protected:
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();

	// DirectXTK objects.
	std::unique_ptr<DirectX::CommonStates>                                  m_states;
	std::unique_ptr<DirectX::DescriptorHeap>                                m_resourceDescriptors;
	std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
	std::unique_ptr<DirectX::SpriteFont>                                    m_font;

	Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_checkerRsc;

	XMMATRIX                                             m_world;
	XMMATRIX                                             m_view;
	XMMATRIX                                             m_projection;

	// Descriptors
	enum Descriptors
	{
		WindowsLogo,
		SeaFloor,
		SegoeFont,
		Count = 256
	};
};

#endif //_SceneSpine_h_
