🎮 D3D12 Shader Resource View(SRV) 생성 및 사용 과정

---

## 📌 1. GPU 리소스 생성

먼저 GPU에 텍스처, 버퍼 등의 리소스를 생성합니다.

```cpp
ComPtr<ID3D12Resource> myTexture;
// 리소스 생성 과정 생략 (CreateCommittedResource 등)
```

---

## 📌 2. 디스크립터 힙 생성 (SRV용)

SRV/CBV/UAV용 디스크립터 힙을 생성합니다. 이 힙은 쉐이더에서 접근할 수 있도록 Shader Visible로 설정해야 합니다.

```cpp
D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
srvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
srvHeapDesc.NumDescriptors = 4; // 필요한 개수만큼 설정
srvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;
device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap));
```

---

## 📌 3. 디스크립터(SRV) 생성

CPU 핸들로 디스크립터 힙에 리소스 설명서(SRV)를 작성합니다.

```cpp
D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
srvDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
srvDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
srvDesc.Texture2D.MostDetailedMip = 0;
srvDesc.Texture2D.MipLevels       = -1; // 전체 mip 사용

CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

device->CreateShaderResourceView(myTexture.Get(), &srvDesc, hDescriptor);
```

📌 `hDescriptor`는 CPU용 핸들입니다. GPU는 이 주소를 직접 사용하지 않습니다.

---

## 📌 4. GPU 핸들로 디스크립터 바인딩

GPU가 사용할 수 있도록 디스크립터 힙과 GPU 핸들을 Command List에 바인딩합니다.

```cpp
// 1. 디스크립터 힙을 바인딩
ID3D12DescriptorHeap* heaps[] = { mSrvDescriptorHeap.Get() };
cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

// 2. GPU 핸들 가져오기
CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

// 3. Root Signature의 Descriptor Table 슬롯에 바인딩
cmdList->SetGraphicsRootDescriptorTable(0, gpuHandle);  // 0번 루트 슬롯에 연결
```

---

## 📌 5. 쉐이더에서 SRV 사용

HLSL 쉐이더 코드에서 `register(t0)` 등으로 SRV에 접근할 수 있습니다.

```hlsl
Texture2D gTex : register(t0);
SamplerState gSampler : register(s0);

float4 PSMain(float2 texcoord : TEXCOORD) : SV_Target
{
    return gTex.Sample(gSampler, texcoord);
}
```

---

## 📌 정리 요약

| 단계 | 설명 |
|------|------|
| ① 리소스 생성 | GPU 리소스를 생성 (텍스처, 버퍼 등) |
| ② 디스크립터 힙 생성 | SRV/CBV/UAV 용으로 Shader Visible 힙 생성 |
| ③ SRV 작성 | `CreateShaderResourceView()`로 CPU 핸들에 SRV 설명서 작성 |
| ④ 디스크립터 바인딩 | GPU 핸들을 `SetGraphicsRootDescriptorTable()`에 바인딩 |
| ⑤ 쉐이더 사용 | `register(t#)`로 SRV 접근 |

---

## 📌 핸들 종류 비교

| 핸들 | 사용 주체 | 목적 |
|------|------------|------|
| `D3D12_CPU_DESCRIPTOR_HANDLE` | CPU | 디스크립터 작성용 (`Create*View`) |
| `D3D12_GPU_DESCRIPTOR_HANDLE` | GPU | 쉐이더에서 리소스 읽기용 |
| `SetGraphicsRootDescriptorTable()` | GPU | Root Signature에 GPU 핸들 바인딩 |
