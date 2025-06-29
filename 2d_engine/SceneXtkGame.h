#pragma once
#ifndef _SceneXtkGame_h_
#define _SceneXtkGame_h_

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


class SceneXtkGame: public IG2Scene
{
public:
	SceneXtkGame()  noexcept;
	virtual ~SceneXtkGame();

	int		Type()									override { return EAPP_SCENE::EAPP_SCENE_XTK; }
	int		Init(const std::any& initialValue ={})	override;
	int		Destroy()								override;
	int		Update(const std::any& t)				override;
	int		Render()								override;
protected:
	void Tick();
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void NewAudioDevice();

	void XM_CALLCONV DrawGrid(DirectX::FXMVECTOR xAxis,DirectX::FXMVECTOR yAxis,DirectX::FXMVECTOR origin,size_t xdivs,size_t ydivs,DirectX::GXMVECTOR color);


	// Rendering loop timer.
	DX::StepTimer                           m_timer;

	// Input devices.
	std::unique_ptr<DirectX::GamePad>           m_gamePad;
	std::unique_ptr<DirectX::Keyboard>          m_keyboard;
	std::unique_ptr<DirectX::Mouse>             m_mouse;

	DirectX::GamePad::ButtonStateTracker        m_gamePadButtons;
	DirectX::Keyboard::KeyboardStateTracker     m_keyboardButtons;

	// DirectXTK objects.
	std::unique_ptr<DirectX::CommonStates>                                  m_states;
	std::unique_ptr<DirectX::DescriptorHeap>                                m_resourceDescriptors;
	std::unique_ptr<DirectX::BasicEffect>                                   m_lineEffect;
	std::unique_ptr<DirectX::BasicEffect>                                   m_shapeEffect;
	std::unique_ptr<DirectX::Model>                                         m_model;
	DirectX::Model::EffectCollection                                        m_modelEffects;
	std::unique_ptr<DirectX::EffectTextureFactory>                          m_modelResources;
	std::unique_ptr<DirectX::GeometricPrimitive>                            m_shape;
	std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
	std::unique_ptr<DirectX::SpriteFont>                                    m_font;

	std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
	std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
	std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
	std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
	std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;

	Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_texture1;
	Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_texture2;

	uint32_t                                                                m_audioEvent;
	float                                                                   m_audioTimerAcc;

	bool                                                                    m_retryDefault;

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

#endif //_SceneXtkGame_h_
