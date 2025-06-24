
#pragma once

#include <Windows.h>
#include "G2ConstantsWin.h"
#include "d3dUtil.h"
#include "GameTimer.h"

using namespace std;


class D3DWinApp : public IG2AppFrameWin
{
protected:
    virtual ~D3DWinApp();
public:
	int		init(const std::any& initialValue = {})				override;
	int		Run()												override;
	LRESULT	MsgProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)	override;
	void	OnResize(bool update=true)							override;
	void	Update(const std::any& t)							override {};
	void	Render()											override {};
	void	OnMouseDown(WPARAM btnState, const ::POINT& p)		override {};
	void	OnMouseUp(WPARAM btnState, const ::POINT&)			override {};
	void	OnMouseMove(WPARAM btnState, const ::POINT&)		override {};

	HINSTANCE	AppInst() const;
	HWND		MainWnd() const;
	float		AspectRatio() const;
	void		Set4xMsaaState(bool value);
	int			Render3D();

protected:
	bool		InitMainWindow();
	void		CalculateFrameStats();

protected:
    std::wstring mMainWndCaption = L"d3d App";
    HINSTANCE	mhAppInst = nullptr; // application instance handle
    HWND		mhMainWnd = nullptr; // main window handle
	bool		mAppPaused = false;  // is the application paused?
	bool		mMinimized = false;  // is the application minimized?
	bool		mMaximized = false;  // is the application maximized?
	bool		mResizing = false;   // are the resize bars being dragged?
    bool		mFullscreenState = false;// fullscreen enabled
	bool		m4xMsaaState		{};
	::SIZE		m_screenSize {1280, 600};

	GameTimer mTimer;
};

