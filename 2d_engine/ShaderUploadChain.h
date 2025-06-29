#pragma once

#include <d3d12.h>
#include "Common/UploadBuffer.h"
#include "Common/G2.Geometry.h"
using DirectX::XMMATRIX;
using DirectX::XMFLOAT4;

struct ShaderConstTransform
{
	XMMATRIX tmWorld = XMMatrixIdentity();
	XMMATRIX tmTex = XMMatrixIdentity();
};

struct ShaderConstPass
{
	XMMATRIX tmView = XMMatrixIdentity();
	XMMATRIX tmProj = XMMatrixIdentity();
	XMMATRIX tmViewProj = XMMatrixIdentity();
};

struct ShaderConstMaterial
{
	XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMMATRIX tmTexCoord = XMMatrixIdentity();
};

struct ShaderUploadChain
{
	std::unique_ptr<UploadBuffer<ShaderConstTransform>	>   m_cnstTrs ={};
	std::unique_ptr<UploadBuffer<ShaderConstPass>       >   m_cnstPss ={};
	std::unique_ptr<UploadBuffer<ShaderConstMaterial>   >   m_cnstMtl ={};

	ShaderUploadChain(const ShaderUploadChain& rhs) = delete;
	ShaderUploadChain& operator=(const ShaderUploadChain& rhs) = delete;
	ShaderUploadChain(ID3D12Device* device,UINT cbTrsCount=1,UINT cbPssCount=1,UINT cbMtlCount=1)
	{
		m_cnstTrs   = std::make_unique<UploadBuffer<ShaderConstTransform    > >(device,cbTrsCount,true);
		m_cnstPss   = std::make_unique<UploadBuffer<ShaderConstPass         > >(device,cbPssCount,true);
		m_cnstMtl   = std::make_unique<UploadBuffer<ShaderConstMaterial     > >(device,cbMtlCount,true);
	}
};
