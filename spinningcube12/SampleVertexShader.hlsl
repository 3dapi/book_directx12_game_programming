

cbuffer MVP0 : register(b0)
{
	matrix mtWld;
	matrix mtViw;
	matrix mtPrj;
};

struct INPUT_VS
{
	float4 p: POSITION;
	float4 d: COLOR0;
};

struct INPUT_PS
{
	float4 p : SV_POSITION;
	float4 d : COLOR0;
};

INPUT_PS main(INPUT_VS vsi)
{
	INPUT_PS o;
	float4 p = vsi.p;
	p = mul(mtWld, p);
	p = mul(mtViw, p);
	p = mul(mtPrj, p);
	o.p = p;
	o.d = vsi.d;
	return o;
}
