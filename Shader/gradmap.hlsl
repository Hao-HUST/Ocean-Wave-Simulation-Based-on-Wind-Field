Texture2D    g_texture          :register(t0);
SamplerState g_sampler          :register(s0);

cbuffer cbChangePerCall : register(b4)
{
	float4x4	g_matWorldViewProj;
	float2      g_r_detaUV;
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

VS_OUTPUT VS( VS_INPUT input )
{
	VS_OUTPUT Output;
	Output.Texcoord = input.Texcoord;
	Output.Position  =  mul(input.Position, g_matWorldViewProj);
	return Output;  
}


float4 PS( VS_OUTPUT input ) : SV_Target
{
	float2 top_tex = input.Texcoord+float2( 0,-g_r_detaUV.y );
	float2 left_tex = input.Texcoord+float2( - g_r_detaUV.x, 0 );
	float2 right_tex = input.Texcoord+float2( g_r_detaUV.x, 0 );
	float2 bottom_tex = input.Texcoord+float2(0, g_r_detaUV.y);
	
	float top_value =  g_texture.Sample( g_sampler, top_tex ).y;
	float bottom_value =  g_texture.Sample( g_sampler, bottom_tex ).y;
	float left_value =  g_texture.Sample( g_sampler, left_tex ).y;
	float right_value =  g_texture.Sample( g_sampler, right_tex ).y;

    return float4( right_value - left_value,  bottom_value - top_value, 0,0 );
}