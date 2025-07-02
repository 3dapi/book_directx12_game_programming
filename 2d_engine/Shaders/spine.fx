//--------------------------------------------------------------------------------------
// for spine
SamplerState	smpLinear				: register(s0);			// sampler state
Texture2D		texDif					: register(t0);			// diffuse texture

cbuffer			cbTrs					: register(b0)
{
	float4x4	tmMVP;
}

//--------------------------------------------------------------------------------------
struct VS_IN
{
	float2 p : POSITION;
	float4 d : COLOR;
	float2 t : TEXCOORD;
};
struct PS_IN
{
	float4 p : SV_POSITION;
	float4 d : COLOR;
	float2 t : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
PS_IN main_vtx( VS_IN v )
{
	PS_IN o = (PS_IN)0;
	o.p = mul( tmMVP, float4(v.p, 0.0F, 1.0F) );
	o.d = v.d;
	o.t = v.t;
	return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shader

float4 main_pxl( PS_IN v) : SV_Target0
{
	float4 finalColor = texDif.Sample( smpLinear, v.t );
	finalColor *= v.d;
	return finalColor;
}
