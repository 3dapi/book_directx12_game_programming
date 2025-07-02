
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
#include "AppCommonXTK.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace G2;

class MainApp : public D3DWinApp
{
public:
	MainApp();
	virtual ~MainApp();
	std::any getAttrib(int nAttrib)							override;
	int		setAttrib(int nAttrib, const std::any& v = {})	override;
	int		command(int nCmd, const std::any& v = {})		override;
	int		init(const std::any& initialValue = {})			override;
	int		destroy()										override;
	int		Resize(bool update)								override;
	int		Update(const std::any& t)						override;
	int		Render()										override;

	void	OnMouseDown(WPARAM btnState, const ::POINT& )	override;
	void	OnMouseUp(WPARAM btnState, const ::POINT& )		override;
	void	OnMouseMove(WPARAM btnState, const ::POINT& )	override;
	void	OnKeyboardInput(const GameTimer& gt);

protected:
	std::unique_ptr<GraphicsMemory>			m_graphicsMemory;
	std::unique_ptr<XTK_BATCH>				m_batch;

	unique_ptr<IG2Scene>					m_pSceneMesh	{};
	unique_ptr<IG2Scene>					m_pSceneXKT		{};
	unique_ptr<IG2Scene>					m_pSceneSpine	{};
	unique_ptr<IG2Scene>					m_pSceneSample	{};
};
