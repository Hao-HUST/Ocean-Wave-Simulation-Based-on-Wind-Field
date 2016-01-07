#pragma once
#include "HLSLclass.h"
#include "MeshFactory.h"
#include "MyTexture.h"
#include "DXUTcamera.h"
#include "OpenSeaWave.h"


#define PAD16(n) (((n)+15)/16*16)

class OpenSeaRender
{
public:

	struct Const_Per_Call
	{
		D3DXMATRIX	matViewProj;
		D3DXMATRIX  matWorld;
		D3DXVECTOR4 vec4eyePos;
		D3DXVECTOR4 vec4sunDir;
		D3DXVECTOR4 vec4OpenSeaRenderScale;
	};

	OpenSeaRender(int max_Level);
	~OpenSeaRender(void);

public:
	HRESULT InitResource(ID3D11Device* pd3dDevice);
	void    Release();

	void    RenderWireframe(ID3D11DeviceContext* pd3dContext, const CBaseCamera&  camera, float time, OpenSeaWave* pOpenSea);
	void    RenderSolid(ID3D11DeviceContext* pd3dContext, const CBaseCamera&  camera, float time,OpenSeaWave* pOpenSea);
	

	HLSLclass*    m_pShader_RenderHeightMap;
	MeshFactory*  m_pMeshFactory;
	ID3D11Buffer* m_pPerCallCB;
	int           m_MaxLevel;

	ID3D11RasterizerState*  m_pRasterState_WireFrame;
	ID3D11RasterizerState*  m_pRasterState_Solid;

	ID3D11SamplerState*     m_pSampleLinearBorder;

private:
	int        DetemineLod( const CBaseCamera& camera );
	bool       IsInViewArea( const CBaseCamera& camera , float max_height);
	void	   RenderMesh(ID3D11DeviceContext* pd3dContext, const CBaseCamera&  camera, float time,OpenSeaWave* pOpenSea );
};

