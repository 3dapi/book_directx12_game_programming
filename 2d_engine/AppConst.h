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

enum EAPP_ATTRIB
{
	EAPP_ATT_CUR_CB		= 0x0101,		// current const buffer
};
enum EAPP_CMD
{
};


enum EAPP_SCENE
{
	EAPP_SCENE_NONE  = 0,
	EAPP_SCENE_MESH,
	EAPP_SCENE_BEGIN,
	EAPP_SCENE_LOBBY,
	EAPP_SCENE_PLAY,
};

#endif __APPCONST_H__