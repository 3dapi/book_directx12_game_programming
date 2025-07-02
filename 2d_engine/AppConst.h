#pragma once
#ifndef __APPCONST_H__
#define __APPCONST_H__

#include <any>
#include <array>
#include <memory>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12/d3dx12.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace G2;

enum EAPP_CONST
{
	EAPP_FRAME_RESOURCE_CHAIN_NUMBER	= 2,		// Cycle through the circular frame resource array.
};

enum EAPP_ATTRIB
{
	EAPP_ATT_WIN_HWND					= 0x0101,	// window handle
	EAPP_ATT_WIN_HINST					,			//
	EAPP_ATT_XTK_GRAPHICS_MEMORY		,			//
	EAPP_ATT_XTK_BATCH					,			//
	EAPP_ATT_CUR_CB			= 0x0301	,			// current const buffer
};
enum EAPP_CMD
{
};


enum EAPP_SCENE
{
	EAPP_SCENE_NONE			= 0,
	EAPP_SCENE_MESH,
	EAPP_SCENE_XTK,
	EAPP_SCENE_SPINE,
	EAPP_SCENE_BEGIN		=11,
	EAPP_SCENE_LOBBY		,
	EAPP_SCENE_PLAY			,
};

#endif __APPCONST_H__