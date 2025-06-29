#pragma once
#ifndef __APPCOMMONXTK_H__
#define __APPCOMMONXTK_H__

#include <array>
#include <memory>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12/d3dx12.h>
#include "AppConst.h"
#include "ShaderUploadChain.h"

#include "VertexTypes.h"
#include "PrimitiveBatch.h"
#include "Effects.h"				// BasicEffect
#include "Model.h"					// #include "GraphicsMemory.h"

#include "GamePad.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Audio.h"					// SoundEffect
#include "GeometricPrimitive.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "DescriptorHeap.h"
#include "CommonStates.h"



typedef PrimitiveBatch<DirectX::VertexPositionColor>		XTK_BATCH;

#endif __APPCOMMONXTK_H__
