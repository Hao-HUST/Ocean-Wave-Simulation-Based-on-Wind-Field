#include "DXUT.h"
#include "RenderTexture.h"


RenderTexture::RenderTexture()
{
	m_pRenderTargetTexture = NULL;
	m_pRenderTargetView = NULL;
	m_pShaderResourceView = NULL;
	m_pDepthStencilView = NULL;
	m_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

RenderTexture::~RenderTexture()
{

}

HRESULT RenderTexture::InitResource(ID3D11Device* pd3dDevice,int width, int height,DXGI_FORMAT format)
{
	HRESULT hr = S_OK;

	m_format = format;
	
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory( &texDesc, sizeof(D3D11_TEXTURE2D_DESC) );
	texDesc.Width = (UINT)width;
	texDesc.Height =(UINT)height;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = pd3dDevice->CreateTexture2D( &texDesc, NULL, &m_pRenderTargetTexture );
	assert( m_pRenderTargetTexture );


	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	hr = pd3dDevice->CreateRenderTargetView( m_pRenderTargetTexture, &rtvDesc, &m_pRenderTargetView );
	assert(m_pRenderTargetView );

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = pd3dDevice->CreateShaderResourceView( m_pRenderTargetTexture, &srvDesc, &m_pShaderResourceView);
	assert( m_pShaderResourceView );

	//create depth stencil texture  
	D3D11_TEXTURE2D_DESC descDepth;  
	descDepth.Width = (UINT)width;  
	descDepth.Height = (UINT)height;  
	descDepth.MipLevels = 1;  
	descDepth.ArraySize = 1;  
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT ;  
	descDepth.SampleDesc.Count = 1;  
	descDepth.SampleDesc.Quality = 0;  
	descDepth.Usage = D3D11_USAGE_DEFAULT;  
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;  
	descDepth.CPUAccessFlags = 0;  
	descDepth.MiscFlags = 0;  

	hr =  pd3dDevice->CreateTexture2D( &descDepth, NULL, &m_pDepthStencilTexture ); 
	V_RETURN(hr);
	 // Create the depth stencil view  
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV; 
	ZeroMemory(&descDSV,sizeof( descDSV ) );
	descDSV.Format = descDepth.Format;  
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;  

	hr = pd3dDevice->CreateDepthStencilView( m_pDepthStencilTexture, &descDSV, &m_pDepthStencilView );  
	V_RETURN(hr);

	return hr;
}

void RenderTexture::Release()
{
	SAFE_RELEASE( m_pRenderTargetTexture );
	SAFE_RELEASE( m_pRenderTargetView );
	SAFE_RELEASE( m_pShaderResourceView );

	SAFE_RELEASE( m_pDepthStencilTexture );
	SAFE_RELEASE( m_pDepthStencilView );
}

void RenderTexture::SetRenderTarget(ID3D11DeviceContext* pd3dcontext )
{
	pd3dcontext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView  );
}

void RenderTexture::ClearRenderTarget( ID3D11DeviceContext* pd3dcontext , float red, float green, float blue, float  alpha)
{
	float color[4]={ red, green, blue, alpha };
	pd3dcontext->ClearRenderTargetView( m_pRenderTargetView, color );
	pd3dcontext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH,1.0 , 0 );
}

ID3D11ShaderResourceView* RenderTexture::GetShaderResourceView()
{
	return m_pShaderResourceView;
}