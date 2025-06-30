
#pragma once
#ifndef _G2_CONSTANTSWIN_H_
#define _G2_CONSTANTSWIN_H_

#include "G2.Constants.h"
#include <Windows.h>

namespace G2 {

enum EG2GRAPHICS_D3D
{
	EG2_GRAPHICS_D3D_NONE			= 0x0000,

	// attribute
	ATT_BASE						= 0x1000,
	ATT_ASPECTRATIO					,					// float*
	ATT_DEVICE						,					// ID3D12Device*
	ATT_SCREEN_SIZE					,					// ::SIZE*

	ATT_DEVICE_BACKBUFFER_FORAT		= 0x1201,			// DXGI_FORMAT*
	ATT_DEVICE_DEPTH_STENCIL_FORAT	,					// DXGI_FORMAT*
	ATT_DEVICE_CURRENT_FRAME_INDEX	,					// UINT*
	ATT_DEVICE_CURRENT_FENCE_INDEX	,					// UINT64*
	ATT_DEVICE_VIEWPORT				,					// D3D12_VIEWPORT*
	ATT_DEVICE_SCISSOR_RECT			,					// D3D12_RECT*
	ATT_DEVICE_MSAASTATE4X_STATE	,					// bool*
	ATT_DEVICE_MSAASTATE4X_QUALITY	,					// UINT*

	// command
	CMD_BASE						= 0x2000,
	CMD_MSAASTATE4X					,
	CMD_SCREEN_RESIZE				,
	CMD_WAIT_GPU					,
	CMD_FENCE_WAIT					,
	CMD_PRESENT						,
	CMD_COMMAND_BEGIN				,
	CMD_COMMAND_END					,
};

struct IG2AppFrameWin : public IG2AppFrame
{
	static IG2AppFrameWin* instance();

	virtual int		Run()													= 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)		= 0;
	virtual int		Resize(bool update=true)								= 0;
	virtual int		Update(const std::any& t)								= 0;
	virtual int		Render()												= 0;
	virtual void	OnMouseDown(WPARAM btnState, const ::POINT& p)			= 0;
	virtual void	OnMouseUp(WPARAM btnState, const ::POINT&)				= 0;
	virtual void	OnMouseMove(WPARAM btnState, const ::POINT&)			= 0;
};

typedef struct IG2GraphicsD3D* PENGINE_D3D;
struct IG2GraphicsD3D : public IG2Graphics
{
	static PENGINE_D3D instance();

	virtual std::any getDevice()						= 0;
	virtual std::any getRootSignature()					= 0;
	virtual std::any getCommandAllocator()				= 0;
	virtual std::any getCommandQueue()					= 0;
	virtual std::any getCommandList()					= 0;
	virtual std::any getBackBufferView()				= 0;
	virtual std::any getDepthStencilView()				= 0;
	virtual std::any getFence()							= 0;
	virtual int      getCurrentBackbufferdex() const	= 0;
	virtual std::any getCurrentBackBuffer()				= 0;
};

} // namespace G2 {

#endif // _G2ConstantsWin_H_
