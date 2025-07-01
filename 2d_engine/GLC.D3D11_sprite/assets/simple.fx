//--------------------------------------------------------------------------------------
// Constant Buffer Variables

cbuffer MVP0 : register( b0 ) { matrix mtWld;}      // world matrix
cbuffer MVP1 : register( b1 ) { matrix mtViw;}      // view matrix
cbuffer MVP2 : register( b2 ) { matrix mtPrj;}      // projection matrix

Texture2D    gTexDif   : register(t0);              // diffuse texture
SamplerState gSmpLinear : register(s0);              // sampler state

//--------------------------------------------------------------------------------------
struct VS_IN
{
    float4 p : POSITION;
    float4 d : COLOR0;
    float2 t : TEXCOORD0;
};
struct PS_IN
{
    float4 p : SV_POSITION;
    float4 d : COLOR0;
    float2 t : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
PS_IN main_vtx( VS_IN v )
{
    PS_IN o = (PS_IN)0;
    o.p = mul( mtWld, v.p );
    o.p = mul( mtViw, o.p );
    o.p = mul( mtPrj, o.p );
    o.d = v.d;
    o.t = v.t;
    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shader

float4 main_pxl( PS_IN v) : SV_Target0
{
    float4 finalColor = gTexDif.Sample( gSmpLinear, v.t );
    finalColor *= v.d;
    return finalColor;
}
