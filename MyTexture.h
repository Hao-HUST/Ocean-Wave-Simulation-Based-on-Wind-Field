#pragma once
#include <D3DX11.h>

//HRESULT CreateTextureSRVFromWTFile( ID3D11Device* pd3dDevice, const char* filename , ID3D11ShaderResourceView** ppSRV);

class MyTexture
{
public:
	int m_width;
	int m_height;
	ID3D11ShaderResourceView* m_pSRV;

	MyTexture();
	~MyTexture();
	HRESULT InitResource(  ID3D11Device* pd3dDevice, const char* filename );
	HRESULT InitResource( int width, int height, ID3D11ShaderResourceView* SRV );
	inline ID3D11ShaderResourceView* GetSRV() {return m_pSRV;}
	void    Release();
};