﻿//--------------------------------------------------------------------------------------
// File: DDSTextureLoader.h
//
// Functions for loading a DDS texture and creating a Direct3D 11 runtime resource for it
//
// Note these functions are useful as a light-weight runtime loader for DDS files. For
// a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma once
#endif
#ifndef __D12DDSTEXTURELOADER_H__
#define __D12DDSTEXTURELOADER_H__

#include <wrl.h>
#include <d3d11_1.h>
#include "d3dx12.h"

#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>

#pragma warning(pop)

#if defined(_MSC_VER) && (_MSC_VER<1610) && !defined(_In_reads_)
#define _In_reads_(exp)
#define _Out_writes_(exp)
#define _In_reads_bytes_(exp)
#define _In_reads_opt_(exp)
#define _Outptr_opt_
#endif

#ifndef _Use_decl_annotations_
#define _Use_decl_annotations_
#endif

using namespace Microsoft::WRL;

namespace DirectX
{
    enum D12DDS_ALPHA_MODE
    {
        D12DDS_ALPHA_MODE_UNKNOWN       = 0,
        D12DDS_ALPHA_MODE_STRAIGHT      = 1,
        D12DDS_ALPHA_MODE_PREMULTIPLIED = 2,
        D12DDS_ALPHA_MODE_OPAQUE        = 3,
        D12DDS_ALPHA_MODE_CUSTOM        = 4,
    };

    // Standard version
    HRESULT D12CreateDDSTextureFromMemory( _In_ ID3D11Device* d3dDevice,
                                        _In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
                                        _In_ size_t ddsDataSize,
                                        _Outptr_opt_ ID3D11Resource** texture,
                                        _Outptr_opt_ ID3D11ShaderResourceView** textureView,
                                        _In_ size_t maxsize = 0,
                                        _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
                                      );

	HRESULT D12CreateDDSTextureFromMemory12(_In_ ID3D12Device* device,
		                                 _In_ ID3D12GraphicsCommandList* cmdList,
		                                 _In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
		                                 _In_ size_t ddsDataSize,
		                                 _Out_ ComPtr<ID3D12Resource>& texture,
		                                 _Out_ ComPtr<ID3D12Resource>& textureUploadHeap,
		                                 _In_ size_t maxsize = 0,
		                                 _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
		                                 );

    HRESULT D12CreateDDSTextureFromFile( _In_ ID3D11Device* d3dDevice,
                                      _In_z_ const wchar_t* szFileName,
                                      _Outptr_opt_ ID3D11Resource** texture,
                                      _Outptr_opt_ ID3D11ShaderResourceView** textureView,
                                      _In_ size_t maxsize = 0,
                                      _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
                                    );

	HRESULT D12CreateDDSTextureFromFile12(_In_ ID3D12Device* device,
		                               _In_ ID3D12GraphicsCommandList* cmdList,
		                               _In_z_ const wchar_t* szFileName,
		                               _Out_ ComPtr<ID3D12Resource>& texture,
		                               _Out_ ComPtr<ID3D12Resource>& textureUploadHeap,
		                               _In_ size_t maxsize = 0,
		                               _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
		                               );

    // Standard version with optional auto-gen mipmap support
    HRESULT D12CreateDDSTextureFromMemory( _In_ ID3D11Device* d3dDevice,
                                        _In_opt_ ID3D11DeviceContext* d3dContext,
                                        _In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
                                        _In_ size_t ddsDataSize,
                                        _Outptr_opt_ ID3D11Resource** texture,
                                        _Outptr_opt_ ID3D11ShaderResourceView** textureView,
                                        _In_ size_t maxsize = 0,
                                        _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
                                      );

    HRESULT D12CreateDDSTextureFromFile( _In_ ID3D11Device* d3dDevice,
                                      _In_opt_ ID3D11DeviceContext* d3dContext,
                                      _In_z_ const wchar_t* szFileName,
                                      _Outptr_opt_ ID3D11Resource** texture,
                                      _Outptr_opt_ ID3D11ShaderResourceView** textureView,
                                      _In_ size_t maxsize = 0,
                                      _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
                                    );

    // Extended version
    HRESULT D12CreateDDSTextureFromMemoryEx( _In_ ID3D11Device* d3dDevice,
                                          _In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
                                          _In_ size_t ddsDataSize,
                                          _In_ size_t maxsize,
                                          _In_ D3D11_USAGE usage,
                                          _In_ unsigned int bindFlags,
                                          _In_ unsigned int cpuAccessFlags,
                                          _In_ unsigned int miscFlags,
                                          _In_ bool forceSRGB,
                                          _Outptr_opt_ ID3D11Resource** texture,
                                          _Outptr_opt_ ID3D11ShaderResourceView** textureView,
                                          _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
                                      );

    HRESULT D12CreateDDSTextureFromFileEx( _In_ ID3D11Device* d3dDevice,
                                        _In_z_ const wchar_t* szFileName,
                                        _In_ size_t maxsize,
                                        _In_ D3D11_USAGE usage,
                                        _In_ unsigned int bindFlags,
                                        _In_ unsigned int cpuAccessFlags,
                                        _In_ unsigned int miscFlags,
                                        _In_ bool forceSRGB,
                                        _Outptr_opt_ ID3D11Resource** texture,
                                        _Outptr_opt_ ID3D11ShaderResourceView** textureView,
                                        _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
                                    );

    // Extended version with optional auto-gen mipmap support
    HRESULT D12CreateDDSTextureFromMemoryEx( _In_ ID3D11Device* d3dDevice,
                                          _In_opt_ ID3D11DeviceContext* d3dContext,
                                          _In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
                                          _In_ size_t ddsDataSize,
                                          _In_ size_t maxsize,
                                          _In_ D3D11_USAGE usage,
                                          _In_ unsigned int bindFlags,
                                          _In_ unsigned int cpuAccessFlags,
                                          _In_ unsigned int miscFlags,
                                          _In_ bool forceSRGB,
                                          _Outptr_opt_ ID3D11Resource** texture,
                                          _Outptr_opt_ ID3D11ShaderResourceView** textureView,
                                          _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
                                      );

    HRESULT D12CreateDDSTextureFromFileEx( _In_ ID3D11Device* d3dDevice,
                                        _In_opt_ ID3D11DeviceContext* d3dContext,
                                        _In_z_ const wchar_t* szFileName,
                                        _In_ size_t maxsize,
                                        _In_ D3D11_USAGE usage,
                                        _In_ unsigned int bindFlags,
                                        _In_ unsigned int cpuAccessFlags,
                                        _In_ unsigned int miscFlags,
                                        _In_ bool forceSRGB,
                                        _Outptr_opt_ ID3D11Resource** texture,
                                        _Outptr_opt_ ID3D11ShaderResourceView** textureView,
                                        _Out_opt_ D12DDS_ALPHA_MODE* alphaMode = nullptr
                                    );
}

#endif
