
#pragma once
#ifndef _SceneSpine_D3D12_H_
#define _SceneSpine_D3D12_H_

#include <map>
#include <vector>
#include <any>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>
#include <spine/spine.h>

#include "Common/G2.Constants.h"
#include "Common/G2.Geometry.h"
#include "common/G2.ConstantsWin.h"
#include "common/G2.Util.h"
#include "AppCommon.h"
#include "AppCommonXTK.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace G2;

struct VtxSequenceSpine
{
	enum ESPINE_ATTACHMENT_TYPE { ESPINE_MESH_ATTACH=1,ESPINE_MESH_REGION, };

	int					drawOrder	{};
	int					meshType	{};
	ID3D12Resource*		bufPos		{};
	ID3D12Resource*		bufTex		{};
	ID3D12Resource*		bufDif		{};
	ID3D12Resource*		bufIdx		{};
	size_t				countVtx	{};
	size_t				countIdx	{};
	D3D12_PRIMITIVE_TOPOLOGY primitive{D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP};

	ID3D12Resource*		uploadPos	{};
	ID3D12Resource*		uploadTex	{};
	ID3D12Resource*		uploadDif	{};
	ID3D12Resource*		uploadIdx	{};

	int resourceBinding(int order, void* attachment, ESPINE_ATTACHMENT_TYPE attachmentType, size_t vertexCount, size_t indexCount = 0);
	int draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE texHandle);

	~VtxSequenceSpine();
};

class SceneSpine: public spine::TextureLoader,  public G2::IG2Scene
{
protected:
	ID3D12Resource*						m_cnstMVP				{};
	XMMATRIX							m_tmMVP					= XMMatrixIdentity();
	ID3D12RootSignature*				m_rootSignature			{};
	ID3D12PipelineState*				m_pipelineState			{};

	std::vector<std::string>			m_spineAnimations		;
	std::map<int,VtxSequenceSpine*>		m_spineSequence			;
	ID3D12DescriptorHeap*				m_srvHeap				{};
	UINT								m_srvHeapIndex			{};
	spine::Skeleton*					m_spineSkeleton			{};
	ID3D12DescriptorHeap*				m_samplerHeap			{};
	spine::AnimationState*				m_spineAniState			{};
	spine::SkeletonData*				m_spineSkeletonData		{};
	spine::Atlas*						m_spineAtlas			{};

public:
	SceneSpine();
	virtual ~SceneSpine();

	// spine
	void	load(spine::AtlasPage& page,const spine::String& path) override;
	void	unload(void* texture) override;
	// IG2Scene
	int		Type()									override { return EAPP_SCENE::EAPP_SCENE_SPINE; }
	int		Init(const std::any& ={})	override;
	int		Destroy()					override;
	int		Update(const std::any& t)	override;
	int		Render()					override;

	void SetMVP(const XMMATRIX& tmMVP);

protected:
	void InitSpine();
	void UpdateSpineSequence();
	void UpdateSpineBuffer();
	void SetupSpineSequence(int order,void* attachment,VtxSequenceSpine::ESPINE_ATTACHMENT_TYPE attachmentType);
	
};

#endif
