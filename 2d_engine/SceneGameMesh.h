#pragma once
#ifndef _SceneGameMesh_H_
#define _SceneGameMesh_H_

#include "Common/G2.Constants.h"
#include "Common/G2.Geometry.h"
#include "common/G2.ConstantsWin.h"
#include "Common/GameTimer.h"
#include "AppCommon.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace G2;

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
	void SetupUploadChain();
	int	 UpdateUploadChain();

	void UpdateCamera(const GameTimer& gt);
	void BuildBox();
	void UpdateBox(const GameTimer& gt);
    void DrawBox(ID3D12GraphicsCommandList* cmdList);

public:
	vector<unique_ptr<ShaderUploadChain>>	m_subLst	;
	ShaderUploadChain*						m_subCur	{};
	int										m_subIdx	{};
	RenderItem			m_wireBox{};

	XMFLOAT3	m_tmEyePos = { 0.0f, 0.0f, 0.0f };
	float		mTheta = 1.5f*XM_PI;
	float		mPhi = XM_PIDIV2 - 0.1f;
	float		mRadius = 50.0f;
};

#endif // _SceneGameMesh_H_
