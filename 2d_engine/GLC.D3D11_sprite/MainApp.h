#pragma once
#ifndef _MainApp_H_
#define _MainApp_H_

#include <map>
#include <vector>
#include <d3d11.h>
#include <DirectxMath.h>
#include <spine/spine.h>
#include "GameTimer.h"
#include "G2Util.h"

using namespace std;
using namespace DirectX;


struct VtxSequenceSpine
{
	int				drawOrder	{};
	enum ESPINE_ATTACHMENT_TYPE { ESPINE_MESH_ATTACH=1, ESPINE_MESH_REGION, };
	int				meshType	{};
	ID3D11Buffer*	bufPos		{};
	ID3D11Buffer*	bufTex		{};
	ID3D11Buffer*	bufDif		{};
	ID3D11Buffer*	bufIdx		{};
	size_t			countVtx	{};
	size_t			countIdx	{};
	D3D_PRIMITIVE_TOPOLOGY	primitve {D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP};
	~VtxSequenceSpine()	{
		G2::SAFE_RELEASE(bufPos);
		G2::SAFE_RELEASE(bufTex);
		G2::SAFE_RELEASE(bufDif);
		G2::SAFE_RELEASE(bufIdx);
		countVtx = {};
		countIdx = {};
	}
	int resourceBinding(int order, ESPINE_ATTACHMENT_TYPE attachmentType, size_t vertexCout, size_t indexCout =0);
	int draw(ID3D11DeviceContext* d3dContext, ID3D11ShaderResourceView* texSRV);
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
	
	map<int, VtxSequenceSpine>	m_vtxSeq			;
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
