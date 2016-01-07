#include "DXUT.h"
#include "MyTexture.h"



MyTexture::MyTexture()
{
	m_pSRV = NULL;
	m_height = 0;
	m_width = 0;
}


MyTexture::~MyTexture()
{
	
}


HRESULT MyTexture::InitResource(  ID3D11Device* pd3dDevice, const char* filename )
{
	HRESULT hr = S_OK;
	FILE* pInput = NULL;
	fopen_s( &pInput, filename, "rb");
	if( pInput == NULL )
	{
		MessageBoxA( NULL, "Can not load Texture file ! ", filename, MB_OK );
		return S_FALSE;
	}

	DWORD width = 0;
	DWORD length = 0;
	DWORD format = 0;
	fread( &width, sizeof(width), 1, pInput );
	fread( &length, sizeof( length), 1, pInput );
	fread( &format, sizeof( format), 1, pInput );

	assert( width != 0 );
	assert( length != 0 );
	assert( format == 4 );

	m_width = width;
	m_height = length;

	int    pixelCounts = width*length ;
	float* pValues = new float[ 4*pixelCounts ];
	fread( pValues, sizeof(float),4*pixelCounts, pInput );

	D3D11_TEXTURE2D_DESC texDes;
	ZeroMemory( &texDes, sizeof(texDes) );
	texDes.Width = width;
	texDes.Height = length;
	texDes.MipLevels = 0;
	texDes.ArraySize = 1;
	texDes.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDes.SampleDesc.Count = 1;
	texDes.SampleDesc.Quality = 0;
	texDes.Usage = D3D11_USAGE_DEFAULT;
	texDes.BindFlags = D3D11_BIND_SHADER_RESOURCE ;
	texDes.CPUAccessFlags = 0;
	texDes.MiscFlags = 0 ;

	ID3D11Texture2D* pTexture2D = NULL;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = pValues;
	data.SysMemPitch =  4*sizeof( float )*width;
	data.SysMemSlicePitch = 0;
	hr = pd3dDevice->CreateTexture2D( &texDes, NULL, &pTexture2D );
	V_RETURN(hr);

	//auto gener mipmaps
	ID3D11DeviceContext* pd3dContext = DXUTGetD3D11DeviceContext();
	pd3dContext->UpdateSubresource( pTexture2D, 0, NULL, pValues, data.SysMemPitch, 0 );
	D3DX11FilterTexture( pd3dContext, pTexture2D, 0, D3DX11_FILTER_BOX );

	delete [] pValues;
	fclose( pInput);

	pTexture2D->GetDesc(&texDes);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDes.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDes.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = pd3dDevice->CreateShaderResourceView( pTexture2D, &srvDesc, &m_pSRV  );
	V_RETURN(hr);
	SAFE_RELEASE( pTexture2D );

	return hr;
}

HRESULT MyTexture::InitResource( int width, int height, ID3D11ShaderResourceView* SRV )
{
	if( width<=0 || height <=0)
		return S_FALSE;
	m_width = width;
	m_height = height;
	m_pSRV = SRV;
	return S_OK;
}


void    MyTexture::Release()
{
	SAFE_RELEASE( m_pSRV );
}