
#pragma once

#include <array>
#include <memory>
#include <vector>

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "Common/G2.Constants.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "Common/D3DWinApp.h"
#include "AppConst.h"
#include "FrameResource.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace G2;

class MainApp : public D3DWinApp
{
public:
	MainApp();
	virtual ~MainApp();
	int		init(const std::any& initialValue = {})			override;
	int		destroy()										override;
	std::any getAttrib(int nAttrib)							override;
	int		setAttrib(int nAttrib, const std::any & v = {})	override;
	int		command(int nCmd, const std::any & v = {})		override;
	int		Resize(bool update)								override;
	int		Update(const std::any& t)						override;
	int		Render()										override;

	void	OnMouseDown(WPARAM btnState, const ::POINT&)	override;
	void	OnMouseUp(WPARAM btnState, const ::POINT&)		override;
	void	OnMouseMove(WPARAM btnState, const ::POINT&)	override;

protected:
	void	OnKeyboardInput(const GameTimer& gt);
	void	UpdateCamera(const GameTimer& gt);
	int		UpdateFrameResource();
	void	UpdateConstBuffer();
	float	GetHillsHeight(float x, float z)const;
	XMFLOAT3 GetHillsNormal(float x, float z)const;

protected:
	std::unique_ptr<IG2Scene>		m_sceneBox	{};

protected:
	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f*XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	POINT mLastMousePos	{};
};
