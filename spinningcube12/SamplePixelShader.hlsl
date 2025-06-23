struct INPUT_PS
{
	float4 p : SV_POSITION;
	float4 d : COLOR0;
};

float4 main(INPUT_PS vsi) : SV_TARGET
{
	return vsi.d;
}
