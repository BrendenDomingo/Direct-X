struct VSOut
{
	float4 pos : SV_POSITION;
	float4 normal : NORMAL;
};

struct PS_OUTPUT 
{
	float4 color : SV_TARGET;
};

PS_OUTPUT main(VSOut input)
{
	PS_OUTPUT output;

	output.color = input.normal;

	output.color.a = 1;

	return output;
}