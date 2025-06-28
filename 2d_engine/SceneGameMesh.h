#pragma once
#ifndef _SceneGameMesh_H_
#define _SceneGameMesh_H_

#include "Common/G2.Constants.h"
#include "Common/G2.Geometry.h"
#include "common/G2.ConstantsWin.h"
#include "Common/GameTimer.h"
#include "AppConst.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace G2;

struct RenderItem
{
	RenderItem() = default;

	::G2::StaticResBufVtx	vtx{};
	::G2::StaticResBufIdx	idx{};
	Material* Mat = nullptr;
	ComPtr<ID3D12DescriptorHeap> srvDesc = nullptr;
	D3D_PRIMITIVE_TOPOLOGY		primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


	XMFLOAT4X4 m_World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	bool bUpdated = true;
};

class SceneGameMesh : public IG2Scene
{
public:
	SceneGameMesh();
	virtual ~SceneGameMesh();
	int		Type()									override { return EAPP_SCENE::EAPP_SCENE_MESH; }
	int		Init(const std::any& initialValue = {})	override;
	int		Destroy()								override;
	int		Update(const std::any& t)				override;
	int		Render()								override;

protected:
	void BuildFrameResources();
	int	 UpdateFrameResource();
	void BuildBox();
	void UpdateBox(const GameTimer& gt);
	void DrawBox();

protected:
	std::vector<std::unique_ptr<FrameResource>> m_frameRscLst;
	FrameResource*	m_frameRscCur = nullptr;
	int				m_frameRscIdx = 0;

	ShaderConstPass		m_cnstPass;
	XMFLOAT3			mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4			mView = MathHelper::Identity4x4();
	XMFLOAT4X4			mProj = MathHelper::Identity4x4();

	RenderItem*			m_wireBox{};
};

#endif // _SceneGameMesh_H_
