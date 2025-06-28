
SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

Texture2D    gTexDiifuse : register(t0);

cbuffer cbTrs : register(b0)
{
    float4x4 g_tmMVP;
};

cbuffer cbMtl : register(b2)
{
	float4   gDiffAlbedo;
};

struct VS_IN
{
	float3 p : POSITION;
	float4 d : COLOR;
	float2 t : TEXCOORD;
};

struct PS_IN
{
	float4 p    : SV_POSITION;
	float2 t    : TEXCOORD;
};

float4 PS(PS_IN pin) : SV_Target
{
    float4 diffuseAlbedo = gTexDiifuse.Sample(gsamAnisotropicWrap, pin.t) * gDiffAlbedo;
	clip(diffuseAlbedo.a - 0.1f);
    return diffuseAlbedo;
}
PS_IN VS(VS_IN vin)
{
	PS_IN o = (PS_IN)0;
	o.p = mul(float4(vin.p, 1.0f), g_tmMVP);
	o.d = vin.d;
	o.t = vin.t;
	return o;
}

float4 PS(PS_IN pin) : SV_Target
{
	float4 diffuse = gTexDiifuse.Sample(gsamAnisotropicWrap, pin.t) * gDiffAlbedo;
	clip(diffuse.a - 0.1f);
	return diffuse;
}

