
#pragma once

#include <Windows.h>
#include "G2.ConstantsWin.h"
#include "d3dUtil.h"
#include "GameTimer.h"

using namespace std;


class D3DWinApp : public G2::IG2AppFrameWin
{
protected:
    virtual ~D3DWinApp();
public:
	int		init(const std::any& initialValue = {})				override;
	int		Run()												override;
	LRESULT	MsgProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)	override;
	int		Resize(bool update=true)							override;
	int		Update(const std::any& t)							override { return S_OK; };
	int		Render()											override { return S_OK; };
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
    HINSTANCE	mhAppInst			{};	// application instance handle
    HWND		mhMainWnd			{};	// main window handle
	bool		mAppPaused			{};	// is the application paused?
	bool		mMinimized			{};	// is the application minimized?
	bool		mMaximized			{};	// is the application maximized?
	bool		mResizing			{};	// are the resize bars being dragged?
    bool		mFullscreenState	{}; // fullscreen enabled
	bool		m_msaa4State		{};
	::SIZE		m_screenSize		{1280, 600};
	bool		m_willResize		{};

	GameTimer mTimer;
};

