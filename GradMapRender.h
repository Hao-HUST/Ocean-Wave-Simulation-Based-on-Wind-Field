#pragma once
#include <D3DX11.h>
#include "OrthoCamera.h"
#include "RenderTexture.h"
#include "HLSLclass.h"
#include "RenderRect.h"


#define PAD16(n) (((n)+15)/16*16)

class GradMapRender
{
public:
	struct Const_Per_Call
	{
		D3DXMATRIX	matWorldViewProj;
		D3DXVECTOR2 detaUV;
	};

	OrthoCamera				  m_Camera;
	ID3D11Buffer*             m_pPerCallCB;
	RenderTexture*			  m_pRenderTexture;
	HLSLclass*				  m_pShader_RenderGradMap;
	RenderRect*				  m_pMesh;
	ID3D11SamplerState*       m_pTexSampler;
	ID3D11RasterizerState*    m_pRS_Solid;

	float                    m_r_pixelWidth; 
	float                    m_r_pixelHeight;
	

public:
	GradMapRender();
	~GradMapRender();

	HRESULT  InitResource( ID3D11Device* pd3dDevice, int width, int height );
	HRESULT  ResizedSwapChain(ID3D11Device* pd3dDevice, int width, int height );
	void     Release();
	void     RenderGradMap(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView* pHeightMap);
	
	ID3D11ShaderResourceView*  GetGradMapSRV();

};