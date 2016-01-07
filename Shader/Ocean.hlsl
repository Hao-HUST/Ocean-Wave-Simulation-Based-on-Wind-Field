
Texture2D    g_PerlinNoiseMap                          :register(t0);
Texture2D    g_OpenWaveHeightMap1                      :register(t1);
Texture2D    g_OpenWaveGradMap1                        :register(t2);
Texture1D    g_FrsenlMap                               :register(t3);


SamplerState g_samplerLinearBorder        :register(s0);
SamplerState g_samplerLinearWrap          :register(s1);

#define DOUBLE_PI 6.2832
#define PERIIN_BLEND_BEGIN 150
#define PERLIN_BLEND_END   5000



cbuffer cbChangePerApp : register(b0)
{
	float4x4    g_matPerlinTex;
	float4      g_PerlinScale;
	float4		g_SunDir;
	float4      g_SunColor;
	float4      g_WaterColor;
	float4      g_SkyColor;
	float4      g_PerlinOctave;     
	float4      g_PerlinOctaveAmp;
	float       g_OceanDeep;
}

cbuffer cbChangePerFrame : register(b1)
{
	float4x4    g_matViewProj;
	float4x4    g_matOpenWaveTex1;
	float4      g_OpenWaveScale1;
	float3      g_EyePt;
	float       g_time;
	float2      g_PerlinMovement;
}

cbuffer cbChangePerCall : register(b2)
{
	float4x4	g_matWorld;
}


struct VS_INPUT
{
    float4 Position	 : POSITION0;
	
};

struct VS_OUTPUT
{
    float4 Position	 : SV_POSITION;
	float4 WorldPos  : POSITION0;
	float4 HeightInfo :POSITION1; //x水高，y陆地高,w:完全处于内陆，即使陆地高<水高
	float4 vsColor   : COLOR0;
};

VS_OUTPUT OceanVS( VS_INPUT Input)
{
	VS_OUTPUT Output;
	float4 vsColor;
	float  perlin_value = 0;
	float3 OcaenLevel = float3(0,g_OceanDeep,0);
	
	
	float4 world_pos = mul(Input.Position,g_matWorld);
	Output.WorldPos = world_pos;

	float eye_dist = length( g_EyePt.xyz -world_pos.xyz );
	float lod = eye_dist/2000;
	float perlin_blend_param =saturate( (eye_dist-PERIIN_BLEND_BEGIN)/(PERLIN_BLEND_END-PERIIN_BLEND_BEGIN) );



	float3 openWave = float3(0,0,0);
	float2 open_wave_tex1 = mul( world_pos, g_matOpenWaveTex1 ).xz;
	openWave += 7*g_OpenWaveHeightMap1.SampleLevel(g_samplerLinearWrap,open_wave_tex1,lod).xyz;

	OcaenLevel.y += lerp( openWave.y,perlin_value, perlin_blend_param);

	
	OcaenLevel.xz += openWave.xz;
	vsColor = float4(0.5,0.5,0.9,1.0);
	world_pos.xz+=OcaenLevel.xz;
	

	world_pos.y = OcaenLevel.y;
	Output.Position =  mul( world_pos, g_matViewProj);
	Output.HeightInfo = float4(OcaenLevel.y,0,1,1);
	Output.vsColor = vsColor;
	return Output;
}


float4 OceanPS_WireFrame( VS_OUTPUT input ) : SV_Target
{
	return  input.vsColor ;
}

float4 OceanPS_Solid( VS_OUTPUT input ) : SV_Target
{

	float3  eye_vec = g_EyePt - input.WorldPos.xyz;
	float3  eye_dir = normalize( eye_vec );
	float   eye_dist = length( eye_vec );


	float dist_2d = length(eye_vec.xz);
	float blend_factor = (50 - dist_2d) / (50 - 5000);
	float   perlin_blend_param =saturate( (eye_dist-PERIIN_BLEND_BEGIN)/(PERLIN_BLEND_END-PERIIN_BLEND_BEGIN) );
	float4  color = g_WaterColor;
	float4  waterColor = g_WaterColor;
	float3  WaterNoraml = float3(0,1,0);

	float2  open_wave_grad = float2(0,0);
	float2  perlin_wave_grad = float2(0,0);


	float2 perlinTex = mul( input.WorldPos, g_matPerlinTex).xz;
	float   deep = g_OceanDeep;





		//open wave
		float2 open_wave_tex1 = mul(input.WorldPos, g_matOpenWaveTex1).xz;
		float2 openWaveGrad1 = g_OpenWaveGradMap1.Sample(g_samplerLinearWrap,open_wave_tex1).xy;
		float2 open_wave_grad1 = float2( openWaveGrad1.x*g_OpenWaveScale1.y/2/g_OpenWaveScale1.x, -openWaveGrad1.y*g_OpenWaveScale1.y/2/g_OpenWaveScale1.x);



		open_wave_grad = open_wave_grad1;

		//perlin wave
		
		float2 perlin_grad1 = g_PerlinNoiseMap.Sample(g_samplerLinearWrap,perlinTex*g_PerlinOctave.x-g_PerlinMovement).xz;
		float2 perlin_grad2 = g_PerlinNoiseMap.Sample(g_samplerLinearWrap,perlinTex*g_PerlinOctave.y-g_PerlinMovement).xz;
		float2 perlin_grad3 = g_PerlinNoiseMap.Sample(g_samplerLinearWrap,perlinTex*g_PerlinOctave.z-g_PerlinMovement).xz;
		float2 perlin_grad =  perlin_grad1*g_PerlinOctaveAmp.x+perlin_grad2*g_PerlinOctaveAmp.y+perlin_grad3*g_PerlinOctaveAmp.z;
		perlin_wave_grad = float2( perlin_grad.x*g_PerlinScale.y/2/g_PerlinScale.x, -perlin_grad.y*g_PerlinScale.y/2/g_PerlinScale.x );

		float2 water_grad = open_wave_grad;
		water_grad = lerp( water_grad,perlin_wave_grad, perlin_blend_param);


		WaterNoraml = float3( -water_grad.x, 1, -water_grad.y);
		WaterNoraml = normalize( WaterNoraml );
		float cos_a = dot( eye_dir, WaterNoraml );
		float frsenl = g_FrsenlMap.Sample(g_samplerLinearBorder,abs(cos_a)).x;
		
		float color_deep = min(g_OceanDeep, deep+10 );
		waterColor = float4(0.07f, 0.15f, 0.2f,0.5f);
		waterColor = lerp(waterColor, g_SkyColor,frsenl);
		color = waterColor;
		color *= 1+dot(WaterNoraml,-g_SunDir.xyz);
		float3 refleck_eye_dir = reflect( -eye_dir, WaterNoraml );
		float  cos_spec = clamp( dot(refleck_eye_dir,-g_SunDir.xyz ),0,1);
		float  sun_spec = pow( cos_spec, 20 );
		color+=sun_spec*g_SunColor;


	return  color  ;
}