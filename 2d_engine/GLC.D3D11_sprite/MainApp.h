#pragma once
#ifndef _MainApp_H_
#define _MainApp_H_

#include <vector>
#include <d3d11.h>
#include <DirectxMath.h>
#include <spine/spine.h>
#include "GameTimer.h"

using namespace DirectX;

struct Vertex {
	XMFLOAT3		p{};
	union
	{
		uint8_t		d[4]{};
		uint32_t	c;
	};
	XMFLOAT2		t{};
};

class MainApp : public spine::TextureLoader
{
protected:
	ID3D11RasterizerState*		m_stateRater		{};
	ID3D11BlendState*			m_stateBlend		{};
	ID3D11DepthStencilState*	m_depthWriteEnabled	{};
	ID3D11VertexShader*			m_shaderVtx			{};
	ID3D11PixelShader*			m_shaderPxl			{};
	ID3D11InputLayout*			m_vtxLayout			{};
	ID3D11Buffer*				m_bufVtx			{};
	int							m_bufVtxCount		{};
	ID3D11Buffer*				m_bufIdx			{};
	int							m_bufIdxCount		{};
	ID3D11Buffer*				m_cnstWorld			{};
	ID3D11Buffer*				m_cnstView			{};
	ID3D11Buffer*				m_cnstProj			{};

	ID3D11ShaderResourceView*	m_srvTexture		{};
	ID3D11SamplerState*			m_sampLinear		{};

	XMMATRIX					m_mtView			{};
	XMMATRIX					m_mtProj			{};
	XMMATRIX					m_mtWorld			{};

	::SIZE						m_screenSize		{1280,640};
	spine::Skeleton*			m_spineSkeleton		{};
	spine::AnimationState*		m_spineAniState		{};
	spine::SkeletonData*		m_spineSkeletonData	{};
	spine::Atlas*				m_spineAtlas		{};

	GameTimer					mTimer				;
public:
	MainApp();
	virtual ~MainApp();
	virtual int Init();
	virtual int Destroy();
	virtual int Update();
	virtual int Render();

	void load(spine::AtlasPage& page,const spine::String& path) override;
	void unload(void* texture) override;
};

#endif
