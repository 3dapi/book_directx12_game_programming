Texture2DArray g_textures : register(t0);
SamplerState   g_sampTex  : register(s0);

cbuffer cbTM : register(b0)
{
	float4x4 g_tmMVP;
};
cbuffer cbMaterial : register(b1)
{
	float4 gDiffuse;
};

struct VS_IN
{
	float3 p : POSITION;
	float4 d : COLOR;
	float2 t : TEXCOORD;
	uint   i : TEXCOORD1;
};

struct PS_IN
{
	float4 p : SV_POSITION;
	float4 d : COLOR;
	float2 t : TEXCOORD;
	uint   i : SV_InstanceID;
};

PS_IN VS(VS_IN vin)
{
	PS_IN o = (PS_IN)0;
	o.p = mul(g_tmMVP, float4(vin.p, 1.0f));
	o.d = vin.d;
	o.t = vin.t;
	o.i = vin.i;
	return o;
}

float4 PS(PS_IN pin) : SV_Target
{
	float3 uvw = float3(pin.t, (float)pin.i);
	float4 texColor = g_textures.Sample(g_sampTex, uvw);
	return texColor * pin.d * gDiffuse;
}
