SamplerState g_samplerLinearBorder        :register(s0);
Texture2D    g_HeightFieldMap             :register(t0);
Texture2D    g_GradMap                    :register(t1);

cbuffer cbChangePerCall : register(b2)
{
	float4x4    g_matViewProj;
	float4x4    g_matWorld;
	float4      g_EyePt;
	float4      g_SunDir;
	float4      g_Scale;
}

struct VS_INPUT
{
    float4 Position	 : POSITION0;
	
};

struct VS_OUTPUT
{
	float4 WorldPos  : POSITION0;
    float4 Position	 : SV_POSITION;
	float2 Texcoord  : TEXCOORD0;
};

VS_OUTPUT OpenSeaRenderVS( VS_INPUT Input)
{
	VS_OUTPUT Output;

	float2 texCoord = float2( Input.Position.x, 1-Input.Position.z);
	float4 world_pos = mul(Input.Position,g_matWorld);
	float  sample_height = g_HeightFieldMap.SampleLevel(g_samplerLinearBorder,texCoord,0).y;
	world_pos.y += sample_height*g_Scale.y;

	Output.WorldPos =  world_pos;
	Output.Texcoord =  texCoord;
	Output.Position =  mul( world_pos, g_matViewProj);

	return Output;
}


float4 OpenSeaRenderPS_Solid( VS_OUTPUT input ) : SV_Target
{

	float3 eye_vec = g_EyePt.xyz - input.WorldPos.xyz;
	float3 eye_dir = normalize( eye_vec );


	float2 grad = g_GradMap.Sample(g_samplerLinearBorder, input.Texcoord).xy;

	float3 normal = float3( -grad.x*g_Scale.y/2.0f/g_Scale.x,1, -grad.y*g_Scale.y/2.0f/g_Scale.x);
	normal = normalize( normal );
	return float4(1,1,1,1)*dot( -g_SunDir, normal);
}

float4 OpenSeaRenderPS_Wireframe( VS_OUTPUT input ) : SV_Target
{

	return float4(1,1,1,1);
}