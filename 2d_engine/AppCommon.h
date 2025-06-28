#pragma once
#ifndef __APPCOMMON_H__
#define __APPCOMMON_H__

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

using std::vector;
using std::unique_ptr;
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace G2;

struct RenderItem
{
	RenderItem() = default;

	// descriptor
	ComPtr<ID3D12DescriptorHeap>	srvDesc{};
	D3D_PRIMITIVE_TOPOLOGY			primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	// vertex buffer
	G2::StaticResBufVtx				vtx{};
	G2::StaticResBufIdx				idx{};
	// const buufer
	ShaderConstTransform			cbTrs;
	ShaderConstPass					cbPss;
	ShaderConstMaterial				cbMtl;
};

#endif __APPCOMMON_H__
