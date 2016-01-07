#pragma once
#include "OrthoCamera.h"
#include "RenderTexture.h"
#include "HLSLclass.h"
#include "RenderRect.h"
#include "Particle.h"
#include "MyTexture.h"

#define PAD16(n) (((n)+15)/16*16)

class ParticleRender
{
public:
	ParticleRender(void);
	~ParticleRender(void);

	HRESULT InitResource( ID3D11Device* pd3dDevice, int width, int height , size_t particlMaxCounts);
	HRESULT ResizedSwapChain(ID3D11Device* pd3dDevice, int width, int height  );
	void Release();
	void RenderPointToTexture(  ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext ,RenderParticle* pVB, size_t particleCounts,  D3DXVECTOR2 cameraPos, D3DXVECTOR2 area );
	void RenderWaveToTexture(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext , float time ,RenderParticle* pVB, size_t particleCounts,  D3DXVECTOR2 cameraPos, D3DXVECTOR2 area );

	inline ID3D11ShaderResourceView* GetHeightMapSRV(){ return m_pRenderTexture->GetShaderResourceView();}

	
public:
	struct Const_Per_Call_RenderPoint
	{
		D3DXMATRIX	matWorldViewProj;
	};

	struct   Const_Per_Call_RenderWave
	{
		D3DXMATRIX	matWorldViewProj;
		float       amp;
	};


	struct RenderPoint
	{
		D3DXVECTOR3 pos;
		float       color;
	};

public:
	OrthoCamera          m_Camera;
	RenderTexture*		 m_pRenderTexture;
	HLSLclass*           m_pShader_RenderPoint;
	HLSLclass*           m_pShader_RenderWave;

	ID3D11Buffer*		 m_pPerCallCB_RenderPoint;
	ID3D11Buffer*		 m_pPerCallCB_RenderWave;

	RenderRect*          m_pWaveRect;
	ID3D11Buffer*        m_pParticleVB ;
	SIZE_T               m_maxParticleCounts;
	ID3D11RasterizerState*  m_pRS;

	ID3D11BlendState*    m_pAlpha_On;
	ID3D11BlendState*    m_pAlpha_Off;

	ID3D11DepthStencilState* m_pDepth_On;
	ID3D11DepthStencilState* m_pDepth_Off;

	ID3D11SamplerState*        m_pTexSampler;
	MyTexture*                 m_pBasicWaveMap;
};

