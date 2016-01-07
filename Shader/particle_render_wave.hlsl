Texture2D    g_texture          :register(t0);
SamplerState g_sampler          :register(s0);

cbuffer cbChangePerCall : register(b4)
{
	float4x4	g_matWorldViewProj;
	float       g_amp;
}

struct VS_INPUT
{
    float4 Position	 : POSITION0;
	float2 Texcoord  : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position	 : SV_POSITION;
	float2 Texcoord  : TEXCOORD0;
};


VS_OUTPUT ParticleVS( VS_INPUT Input)
{
	VS_OUTPUT Output;
	Output.Texcoord = Input.Texcoord;
	Output.Position =  mul(Input.Position, g_matWorldViewProj);
	return Output;
}

float4 ParticlePS( VS_OUTPUT input ) : SV_Target
{
	float2 texCoord = input.Texcoord;
	float3 value =  g_texture.Sample( g_sampler, texCoord)*g_amp*1.2*1.1;
    return float4( value.x,value.y,value.z,1 );
	//return float4(value,1);
	//return float4(0,value.y,0,1);
}