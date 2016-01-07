cbuffer cbChangePerCall : register(b4)
{
	float4x4	g_matWorldViewProj;
}

struct VS_INPUT
{
    float4 Position	 : POSITION0;
	float  Amp  : COLOR0;
};

struct VS_OUTPUT
{
    float4 Position	 : SV_POSITION;
	float  Color	 : COLOR0;
};


VS_OUTPUT ParticleVS( VS_INPUT Input)
{
	VS_OUTPUT Output;
	Output.Color = Input.Amp;
	Output.Position =  mul(Input.Position, g_matWorldViewProj);
	return Output;
}

float4 ParticlePS( VS_OUTPUT input ) : SV_Target
{
	return float4(input.Color, input.Color,input.Color, 1 );
}