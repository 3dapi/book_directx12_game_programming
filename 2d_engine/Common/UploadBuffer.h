
#pragma once
#ifndef _UploadBuffer_h_
#define _UploadBuffer_h_

#include "G2.Util.h"

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device,UINT elementCount,bool isConstBuff=true)
		: m_isconstBuf(isConstBuff)
	{
		m_elementSize = sizeof(T);
		if(isConstBuff)
			m_elementSize = G2::alignTo256(sizeof(T));

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_elementSize*elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)));

		ThrowIfFailed(m_buffer->Map(0,nullptr,reinterpret_cast<void**>(&m_ptr)));
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
	~UploadBuffer()
	{
		if(m_buffer != nullptr)
			m_buffer->Unmap(0,nullptr);
		m_ptr = nullptr;
	}
	ID3D12Resource* Resource()const
	{
		return m_buffer.Get();
	}
	void CopyData(int elementIndex,const T& data)
	{
		memcpy(&m_ptr[elementIndex*m_elementSize],&data,sizeof(T));
	}
	uint8_t* PtrMapping()
	{
		return m_ptr;
	}
	UINT PtrSize() const
	{
		return m_elementSize;
	}
	bool PtrIsConstBuffer() const
	{
		return m_isconstBuf;
	}
protected:
	ComPtr<ID3D12Resource> m_buffer;
	uint8_t* m_ptr = nullptr;

	UINT m_elementSize = 0;
	bool m_isconstBuf = false;
};

#endif
