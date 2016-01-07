#include "DXUT.h"
#include "SDKmisc.h"
#include "OpenSeaWave.h"
#include "LoadFieldDataFromFile.h"


#define  COS_ARRAY_SIZE 1024
#define  G              9.81f


#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16


OpenSeaWave::OpenSeaWave(void)
{
}


OpenSeaWave::~OpenSeaWave(void)
{
}



HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
	HRESULT hr = S_OK;

	// find the file
	WCHAR str[MAX_PATH];
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szFileName ) );

	// open the file
	HANDLE hFile = CreateFile( str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
		return E_FAIL;

	// Get the file size
	LARGE_INTEGER FileSize;
	GetFileSizeEx( hFile, &FileSize );

	// create enough space for the file data
	BYTE* pFileData = new BYTE[ FileSize.LowPart ];
	if( !pFileData )
		return E_OUTOFMEMORY;

	// read the data in
	DWORD BytesRead;
	if( !ReadFile( hFile, pFileData, FileSize.LowPart, &BytesRead, NULL ) )
		return E_FAIL; 

	CloseHandle( hFile );

	// Compile the shader
	char pFilePathName[MAX_PATH];        
	WideCharToMultiByte(CP_ACP, 0, str, -1, pFilePathName, MAX_PATH, NULL, NULL);
	ID3DBlob* pErrorBlob;
	hr = D3DCompile( pFileData, FileSize.LowPart, pFilePathName, NULL, NULL, szEntryPoint, szShaderModel, D3D10_SHADER_ENABLE_STRICTNESS, 0, ppBlobOut, &pErrorBlob );

	delete []pFileData;

	if( FAILED(hr) )
	{
		OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		SAFE_RELEASE( pErrorBlob );
		return hr;
	}
	SAFE_RELEASE( pErrorBlob );

	return S_OK;
}



void createTexture(ID3D11Device* pd3dDevice, UINT width, UINT height, DXGI_FORMAT format,
	ID3D11Texture2D** ppTex)
{
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory( &texDesc, sizeof(D3D11_TEXTURE2D_DESC) );
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DYNAMIC;
	texDesc.BindFlags =  D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc.MiscFlags = 0;
	

	pd3dDevice->CreateTexture2D( &texDesc, NULL, ppTex );
	assert(*ppTex);
}


void createTextureAndViews(ID3D11Device* pd3dDevice, UINT width, UINT height, DXGI_FORMAT format,
	ID3D11Texture2D** ppTex, ID3D11ShaderResourceView** ppSRV, ID3D11RenderTargetView** ppRTV)
{
	// Create 2D texture
	D3D11_TEXTURE2D_DESC tex_desc;
	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 0;
	tex_desc.ArraySize = 1;
	tex_desc.Format = format;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	pd3dDevice->CreateTexture2D(&tex_desc, NULL, ppTex);
	assert(*ppTex);

	// Create shader resource view
	(*ppTex)->GetDesc(&tex_desc);
	if (ppSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = tex_desc.MipLevels;
		srv_desc.Texture2D.MostDetailedMip = 0;

		pd3dDevice->CreateShaderResourceView(*ppTex, &srv_desc, ppSRV);
		assert(*ppSRV);
	}

	// Create render target view
	if (ppRTV)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		rtv_desc.Format = format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Texture2D.MipSlice = 0;

		pd3dDevice->CreateRenderTargetView(*ppTex, &rtv_desc, ppRTV);
		assert(*ppRTV);
	}	
	
}






//===========================================================================================================

WaveParticle::WaveParticle()
{
	m_pGradMap = NULL;
	m_pParticleRender = NULL;
	m_pPartileSourceGen = NULL;
}

WaveParticle::~WaveParticle()
{

}

HRESULT WaveParticle::InitResource_1(ID3D11Device* pd3dDevice, UINT u_grid_dim, UINT v_grid_dim, float width, float length)
{
	HRESULT hr = S_OK;


	int backbuf_width = u_grid_dim;
	int backbuf_height = v_grid_dim;

	m_length = length;
	m_width =  width; 

	m_width_pixelCounts = backbuf_width;
	m_height_pixelCounts = backbuf_height;

	m_pWindFieldData = new LoadFieldDataFromFile();
	m_pWindFieldData->Initialise("data_4.txt");
	m_pPartileSourceGen = new TestCase2();
	m_pPartileSourceGen->InitParticleGens( 10000, m_pWindFieldData->GetUdiv(), m_pWindFieldData->GetVdiv(), D3DXVECTOR2(0,0), D3DXVECTOR2( m_width, m_length),m_pWindFieldData->GetDirection() );

	m_pParticleRender = new ParticleRender;
	assert( m_pParticleRender );
	hr = m_pParticleRender->InitResource(pd3dDevice, backbuf_width, backbuf_height, m_pPartileSourceGen->GetMaxParticleNum() );
	V_RETURN(hr);

	m_pGradMap = new GradMapRender();
	assert( m_pGradMap );
	hr = m_pGradMap->InitResource(pd3dDevice,backbuf_width, backbuf_height );
	V_RETURN(hr);

	return hr;
}

void WaveParticle::Release()
{
	SafeReleaseDelete( m_pGradMap );
	SafeReleaseDelete( m_pParticleRender );
	SafeReleaseDelete( m_pPartileSourceGen );
	SafeReleaseDelete(m_pWindFieldData);

}


HRESULT WaveParticle::ResizedSwapChain(ID3D11Device* pd3dDevice,  int backbuf_width, int backbuf_height )
{
	HRESULT hr = S_OK;

	m_width_pixelCounts = backbuf_width;
	m_height_pixelCounts = backbuf_height;

	hr = m_pGradMap->ResizedSwapChain(pd3dDevice, backbuf_width, backbuf_height );
	V_RETURN(hr);
	hr = m_pParticleRender->ResizedSwapChain(pd3dDevice, backbuf_width, backbuf_height );
	V_RETURN(hr);
	return hr;
}

void WaveParticle::Update( float time , float deatTime )
{
	m_pPartileSourceGen->Update( time, deatTime );
	ID3D11Device* pd3dDevice = DXUTGetD3D11Device();
	ID3D11DeviceContext*  pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	m_pParticleRender->RenderWaveToTexture(pd3dDevice, pd3dImmediateContext, time,m_pPartileSourceGen->GetRenderParticleBuf(), m_pPartileSourceGen->GetParticleCounts(), D3DXVECTOR2(0,0), D3DXVECTOR2(m_width,m_length));
	m_pGradMap->RenderGradMap(pd3dImmediateContext, m_pParticleRender->GetHeightMapSRV() );

	num_particles = m_pPartileSourceGen->GetParticleCounts();
}




