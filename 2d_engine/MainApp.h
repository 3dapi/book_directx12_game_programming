
#pragma once

#include <array>
#include <memory>
#include <vector>

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>

#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "FrameResource.h"
#include "Common/D3DWinApp.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;


struct RenderItem
{
	RenderItem() = default;

	MeshGeometry*				Geo = nullptr;
	Material*					 Mat = nullptr;
	ComPtr<ID3D12DescriptorHeap> srvDesc = nullptr;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


    XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	int NumFramesDirty = d3dUtil::getFrameRscCount();
	UINT ObjCBIndex = -1;
};

enum class RenderLayer : int
{
	AlphaTested=0,
	Count
};

class MainApp : public D3DWinApp
{
public:
	MainApp();
	virtual ~MainApp();
	int		init(const std::any& initialValue = {})			override;
	int		destroy()										override;
	int		Resize(bool update)								override;
	int		Update(const std::any& t)						override;
	int		Render()										override;

	void	OnMouseDown(WPARAM btnState, const ::POINT& )	override;
	void	OnMouseUp(WPARAM btnState, const ::POINT& )		override;
	void	OnMouseMove(WPARAM btnState, const ::POINT& )	override;

private:
    void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	int	 UpdateFrameResource();

	void BuildBox();
    void BuildFrameResources();
    void DrawBox(ID3D12GraphicsCommandList* cmdList);

    float GetHillsHeight(float x, float z)const;
    XMFLOAT3 GetHillsNormal(float x, float z)const;

private:

	std::vector<std::unique_ptr<FrameResource>> m_frameRscLst;
	FrameResource*	m_frameRscCur = nullptr;
	int				m_frameRscIdx = 0;

    UINT mCbvSrvDescriptorSize = 0;

	RenderItem*		m_wireBox{};

	// Render items divided by PSO.


    ShaderConstPass m_cnstPass;

	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f*XM_PI;
    float mPhi = XM_PIDIV2 - 0.1f;
    float mRadius = 50.0f;

    POINT mLastMousePos	{};
};
