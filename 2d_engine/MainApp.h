
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
#include "Common/D3DWinApp.h"
#include "AppCommon.h"

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

protected:
	void OnKeyboardInput(const GameTimer& gt);

	void SetupUploadChain();
	int	 UpdateUploadChain();

	void UpdateCamera(const GameTimer& gt);
	void BuildBox();
	void UpdateBox(const GameTimer& gt);
    void DrawBox(ID3D12GraphicsCommandList* cmdList);

    float GetHillsHeight(float x, float z)const;
    XMFLOAT3 GetHillsNormal(float x, float z)const;

protected:
	vector<unique_ptr<ShaderUploadChain>>	m_subLst	;
	ShaderUploadChain*						m_subCur	{};
	int										m_subIdx	{};
	RenderItem			m_wireBox{};

	XMFLOAT3	m_tmEyePos = { 0.0f, 0.0f, 0.0f };
	float		mTheta = 1.5f*XM_PI;
	float		mPhi = XM_PIDIV2 - 0.1f;
	float		mRadius = 50.0f;
	POINT		mLastMousePos	{};
};
