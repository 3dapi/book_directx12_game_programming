#pragma once
#ifndef _SceneSpine_H_
#define _SceneSpine_H_

#include <vector>
#include <d3d11.h>
#include <DirectxMath.h>
#include <spine/spine.h>

using namespace std;
using namespace DirectX;

class SceneSpine : public spine::TextureLoader
{
protected:
	ID3D11RasterizerState*		m_stateRater		{};
	ID3D11BlendState*			m_stateBlend		{};
	ID3D11DepthStencilState*	m_stateDepthWrite	{};
	ID3D11VertexShader*			m_shaderVtx			{};
	ID3D11PixelShader*			m_shaderPxl			{};
	ID3D11InputLayout*			m_vtxLayout			{};
	
	ID3D11Buffer*				m_bufPos			{};
	ID3D11Buffer*				m_bufDif			{};
	ID3D11Buffer*				m_bufTex			{};

	size_t						m_bufVtxCount		{};
	ID3D11Buffer*				m_bufIdx			{};
	size_t						m_bufIdxCount		{};
	ID3D11Buffer*				m_cnstMVP			{};
	XMMATRIX					m_tmMVP				{};

	ID3D11ShaderResourceView*	m_spineTexture		{};
	ID3D11SamplerState*			m_sampLinear		{};

	::SIZE						m_screenSize		{1280,640};
	spine::Skeleton*			m_spineSkeleton		{};
	spine::AnimationState*		m_spineAniState		{};
	spine::SkeletonData*		m_spineSkeletonData	{};
	spine::Atlas*				m_spineAtlas		{};

public:
	SceneSpine();
	virtual ~SceneSpine();
	int		Init();
	int		Destroy();
	int		Update(double deltaTime);
	int		Render();
	void	SetMVP(const XMMATRIX& v) { m_tmMVP =v ;}

	void load(spine::AtlasPage& page,const spine::String& path) override;
	void unload(void* texture) override;
};

#endif
