
SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

Texture2D    gTexDiffuse : register(t0);

cbuffer cbTrs : register(b0)
{
    float4x4 tmWorld;
	float4x4 tmTexture;
};

cbuffer cbPss : register(b1)
{
    float4x4 tmView;
    float4x4 tmProj;
    float4x4 tmViewProj;
};

cbuffer cbMtl : register(b2)
{
	float4   gDiffAlbedo;
	float4x4 tmTexCoord;
};

struct VS_IN
{
	float3 p    : POSITION;
	float3 n    : NORMAL;
	float2 t    : TEXCOORD;
};

struct PS_IN
{
	float4 p    : SV_POSITION;
	float2 t    : TEXCOORD;
};

PS_IN VS(VS_IN vin)
{
	PS_IN vo = (PS_IN)0;
	float4 p = mul(tmWorld, float4(vin.p, 1.0f));
	vo.p = mul(tmViewProj, p);

	float4 texC = mul(tmTexture, float4(vin.t, 0.0f, 1.0f));
	vo.t = mul(tmTexCoord, texC).xy;

	return vo;
}

float4 PS(PS_IN pin) : SV_Target
{
	float4 diffuse = gTexDiffuse.Sample(gsamAnisotropicWrap, pin.t) * gDiffAlbedo;
	clip(diffuse.a - 0.1f);
	return diffuse;
}


