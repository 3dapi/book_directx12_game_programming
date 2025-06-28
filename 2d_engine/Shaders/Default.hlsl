
SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

Texture2D    gDiffuseMap : register(t0);

cbuffer cbTransform : register(b0)
{
    float4x4 tmWorld;
	float4x4 tmTexture;
};

cbuffer cbPass : register(b1)
{
    float4x4 tmView;
    float4x4 tmProj;
    float4x4 tmViewProj;
};

cbuffer cbMaterial : register(b2)
{
	float4   diffAlbedo;
	float4x4 tmTexCoord;
};

struct VS_IN
{
	float3 p    : POSITION;
    float3 n    : NORMAL;
	float2 t    : TEXCOORD;
};

struct VS_OUT
{
	float4 p    : SV_POSITION;
	float2 t    : TEXCOORD;
};

VS_OUT VS(VS_IN vin)
{
	VS_OUT vo = (VS_OUT)0;
    float4 p = mul(float4(vin.p, 1.0f), tmWorld);
    vo.p = mul(p, tmViewProj);

	float4 texC = mul(float4(vin.t, 0.0f, 1.0f), tmTexture);
	vo.t = mul(texC, tmTexCoord).xy;

    return vo;
}

float4 PS(VS_OUT pin) : SV_Target
{
    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.t) * diffAlbedo;
	clip(diffuseAlbedo.a - 0.1f);
    return diffuseAlbedo;
}


