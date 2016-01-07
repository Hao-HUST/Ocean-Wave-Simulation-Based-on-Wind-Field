#pragma once
#include <D3DX11.h>


#define CS_DEBUG_BUFFER

#define PAD16(n) (((n)+15)/16*16)

class OpenSeaWave
{
public:
	OpenSeaWave(void);
	virtual ~OpenSeaWave(void);

public:
	virtual HRESULT InitResource_1( ID3D11Device* pd3dDevice, UINT u_grid_dim, UINT v_grid_dim, float width, float length) = 0;
	virtual HRESULT ResizedSwapChain(ID3D11Device* pd3dDevice,  int backbuf_width, int backbuf_height ) = 0;
	virtual void    Release() = 0;


	virtual D3DXVECTOR2 GetSize() = 0;
	virtual D3DXVECTOR4 GetDimScale()  = 0;
	virtual D3DXMATRIX  GetMatrix()  = 0;
	virtual ID3D11ShaderResourceView*  GetHeightMapSRV()  = 0;
	virtual ID3D11ShaderResourceView*  GetGradientMapSRV()  = 0;
	virtual void        Update( float time , float deatTime ) = 0;


	virtual size_t     GetParticleCounts() =0;
};

#define COS_WAVE      1
#define FFT_WAVE      2
#define PARTICLE_WAVE 3


#include "ParticleRender.h"
#include "GradMapRender.h"


class WaveParticle:public OpenSeaWave
{
public:
	WaveParticle();
	~WaveParticle();

	ParticleSourceMan*          m_pPartileSourceGen;
	ParticleRender*             m_pParticleRender;
	GradMapRender*              m_pGradMap;

	LoadFieldDataFromFile*      m_pWindFieldData;

	HRESULT InitResource_1( ID3D11Device* pd3dDevice,UINT u_grid_dim, UINT v_grid_dim, float width, float length );
	HRESULT ResizedSwapChain(ID3D11Device* pd3dDevice, int backbuf_width, int backbuf_height );
	void    Release();

	inline virtual D3DXVECTOR2 GetSize() {return D3DXVECTOR2(m_width,m_length);}
	inline D3DXVECTOR4 GetDimScale() {return D3DXVECTOR4(m_width/m_width_pixelCounts,1, m_length/m_height_pixelCounts,1); }
	inline D3DXMATRIX  GetMatrix() { return D3DXMATRIX(1/m_width,0,0,0, 0,1,0,0,  0,0,-1/m_length,0, 0,0,0,1);}
	inline ID3D11ShaderResourceView* GetHeightMapSRV(){ return m_pParticleRender->GetHeightMapSRV();}
	inline ID3D11ShaderResourceView* GetGradientMapSRV() { return m_pGradMap->GetGradMapSRV(); }

	void        Update( float time , float deatTime );


	size_t      GetParticleCounts()  {return num_particles;}

public:
	int m_width_pixelCounts;
	int m_height_pixelCounts;

	float m_length;
	float m_width;

	int   num_particles;
};



