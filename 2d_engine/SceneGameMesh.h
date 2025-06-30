#pragma once
#ifndef _SceneGameMesh_H_
#define _SceneGameMesh_H_

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

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace G2;

class SceneGameMesh : public IG2Scene
{
public:
	SceneGameMesh()  noexcept;
	virtual ~SceneGameMesh();

	int		Type()									override { return EAPP_SCENE::EAPP_SCENE_MESH; }
	int		Init(const std::any& initialValue = {})	override;
	int		Destroy()								override;
	int		Update(const std::any& t)				override;
	int		Render()								override;

protected:
	void UpdateCamera(const GameTimer& gt);
	void BuildBox();
	void UpdateBox(const GameTimer& gt);
    void DrawBox();

public:
	unique_ptr<ShaderUploadChain>	m_cbUploader	{};
	
	// descriptor
	ComPtr<ID3D12DescriptorHeap>	m_boxSrvDesc{};
	D3D_PRIMITIVE_TOPOLOGY			m_boxPrimitive{D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST};
	// vertex buffer
	G2::StaticResBufVtx				m_boxVtx{};
	G2::StaticResBufIdx				m_boxIdx{};

	// const buufer
	ShaderConstTransform			m_boxCbTrs;
	ShaderConstPass					m_boxCbPss;
	ShaderConstMaterial				m_boxCbMtl;

	XMFLOAT3	m_tmEyePos = { 0.0f, 0.0f, 0.0f };
	float		mTheta = 1.5f*XM_PI;
	float		mPhi = XM_PIDIV2 - 0.1f;
	float		mRadius = 50.0f;

	XMMATRIX		m_world			= XMMatrixIdentity();
	XMMATRIX		m_view			= XMMatrixIdentity();;
};

#endif // _SceneGameMesh_H_
