#pragma once
#include <D3DX11.h>

class RenderTexture
{
public:
	RenderTexture();
	~RenderTexture();

	HRESULT InitResource( ID3D11Device* pd3dDevice,int width, int height,DXGI_FORMAT format); 
	void    Release();

	void SetRenderTarget( ID3D11DeviceContext* pd3dcontext );
	void ClearRenderTarget( ID3D11DeviceContext* pd3dcontext, float red, float green, float blue, float  alpha );
	ID3D11ShaderResourceView*  GetShaderResourceView();
	DXGI_FORMAT                GetFormat() const { return m_format ;}

private:
	ID3D11Texture2D*           m_pRenderTargetTexture;
	ID3D11RenderTargetView*    m_pRenderTargetView;
	ID3D11ShaderResourceView*  m_pShaderResourceView;

	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D*        m_pDepthStencilTexture;
	DXGI_FORMAT             m_format;
};
