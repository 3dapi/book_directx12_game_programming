#pragma once
#ifndef _G2ConstantsWin_H_
#define _G2ConstantsWin_H_

#include "G2Constants.h"
#include <Windows.h>

enum EG2GRAPHICS_D3D
{
	EG2_GRAPHICS_D3D_NONE = 0,
	ATT_BASE			= 1000,
	ATT_ASPECTRATIO,		// attribute
	ATT_DEVICE,
	ATT_SCREEN_SIZE,

	ATT_DEVICE_BACKBUFFER_FORAT = 1201,
	ATT_DEVICE_DEPTH_STENCIL_FORAT,
	ATT_DEVICE_CURRENT_FRAME_INDEX,
	ATT_DEVICE_VIEWPORT,
	ATT_DEVICE_SCISSOR_RECT,

	//
	CMD_BASE			= 2000,	// command
	CMD_MSAASTATE4X,
	CMD_SCREEN_RESIZE,
};

struct IG2AppFrameWin : public IG2AppFrame
{
	static IG2AppFrameWin* instance();
	virtual int		Run()												= 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)	= 0;
	virtual void	OnResize(bool update=true)							= 0;
	virtual void	Update(const std::any& t)							= 0;
	virtual void	Render()											= 0;
	virtual void	OnMouseDown(WPARAM btnState, const ::POINT& p)		= 0;
	virtual void	OnMouseUp(WPARAM btnState, const ::POINT&)			= 0;
	virtual void	OnMouseMove(WPARAM btnState, const ::POINT&)		= 0;
};

typedef struct IG2GraphicsD3D* PENGINE_D3D;
struct IG2GraphicsD3D : public IG2Graphics
{
	static PENGINE_D3D instance();

	virtual std::any getDevice()					= 0;
	virtual std::any getRootSignature()				= 0;
	virtual std::any getCommandAllocator()			= 0;
	virtual std::any getCommandQueue()				= 0;
	virtual std::any getCommandList()				= 0;
	virtual std::any getRenderTarget()				= 0;
	virtual std::any getRenderTargetView()			= 0;
	virtual std::any getDepthStencilView()			= 0;
	virtual int      getCurrentFrameIndex() const	= 0;
};
#endif // _G2ConstantsWin_H_
