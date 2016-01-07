#include "DXUT.h"
#include "HLSLclass.h"
#include "SDKmisc.h"

static HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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
	hr = D3DCompile( pFileData, FileSize.LowPart, pFilePathName, NULL, NULL, szEntryPoint, szShaderModel, D3D10_SHADER_ENABLE_STRICTNESS|D3D10_SHADER_DEBUG|D3D10_SHADER_SKIP_OPTIMIZATION, 0, ppBlobOut, &pErrorBlob );

	delete []pFileData;

	if( FAILED(hr) )
	{
		OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		MessageBoxA( NULL,(char*)pErrorBlob->GetBufferPointer(), "ERRO", MB_OK );
		SAFE_RELEASE( pErrorBlob );
		return hr;
	}
	SAFE_RELEASE( pErrorBlob );

	return S_OK;
}

HLSLclass::HLSLclass(void)
{
	 m_pVS = NULL;
	 m_pGS = NULL;
	 m_pPS_Wireframe = NULL ;
	 m_pPS_Solid = NULL;
	 m_pLayout = NULL;
}


HLSLclass::~HLSLclass(void)
{
}

HRESULT HLSLclass::CompileVSInputLayout(ID3D11Device* pd3dDevice, WCHAR* file_name, LPCSTR func_name, const D3D11_INPUT_ELEMENT_DESC * mesh_layout_desc,int elementCounts)
{
	HRESULT hr = S_OK;
	ID3DBlob* pBlobGCVS = NULL;

	hr = CompileShaderFromFile(file_name, func_name, "vs_5_0", &pBlobGCVS);
	assert(pBlobGCVS);

	hr = pd3dDevice->CreateVertexShader(pBlobGCVS->GetBufferPointer(), pBlobGCVS->GetBufferSize(), NULL, &m_pVS);
	assert(m_pVS);

	hr = pd3dDevice->CreateInputLayout(mesh_layout_desc, elementCounts, pBlobGCVS->GetBufferPointer(), pBlobGCVS->GetBufferSize(), &m_pLayout);
	assert(m_pLayout);

	SAFE_RELEASE(pBlobGCVS);

	return hr;
}

HRESULT HLSLclass::CompileGS(ID3D11Device* pd3dDevice, WCHAR* file_name, LPCSTR func_name)
{
	HRESULT hr = S_OK;
	return hr;
}

HRESULT HLSLclass::CompilePS_Solid(ID3D11Device* pd3dDevice, WCHAR* file_name, LPCSTR func_name)
{
	HRESULT hr = S_OK;

	ID3DBlob* pBlobGCPS_Solid = NULL;
	hr = CompileShaderFromFile(file_name, func_name,  "ps_5_0", &pBlobGCPS_Solid);
	assert(pBlobGCPS_Solid );

	hr = pd3dDevice->CreatePixelShader(pBlobGCPS_Solid->GetBufferPointer(), pBlobGCPS_Solid->GetBufferSize(), NULL, &m_pPS_Solid);
	assert( m_pPS_Solid);

	SAFE_RELEASE( pBlobGCPS_Solid );
	
	return hr;
}

HRESULT HLSLclass::CompilePS_Wireframe(ID3D11Device* pd3dDevice, WCHAR* file_name, LPCSTR func_name)
{
	HRESULT hr = S_OK;
	ID3DBlob* pBlobGCPS_Wireframe = NULL;

	hr = CompileShaderFromFile(file_name, func_name, "ps_5_0", &pBlobGCPS_Wireframe);
	assert(pBlobGCPS_Wireframe);

	hr = pd3dDevice->CreatePixelShader(pBlobGCPS_Wireframe->GetBufferPointer(), pBlobGCPS_Wireframe->GetBufferSize(), NULL, &m_pPS_Wireframe);
	assert(m_pPS_Wireframe);

	SAFE_RELEASE(pBlobGCPS_Wireframe);
	return hr;
}

void HLSLclass::Release()
{
	SAFE_RELEASE( m_pVS );
	SAFE_RELEASE( m_pLayout );
	SAFE_RELEASE( m_pPS_Solid );
	SAFE_RELEASE( m_pPS_Wireframe );
	SAFE_RELEASE( m_pGS );
}