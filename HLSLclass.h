#pragma once
class HLSLclass
{
public:
	HLSLclass(void);
	~HLSLclass(void);

	virtual HRESULT CompileVSInputLayout( ID3D11Device* pd3dDevice, WCHAR* file_name, LPCSTR func_name, const D3D11_INPUT_ELEMENT_DESC * mesh_layout_desc,int elementCounts );
	virtual HRESULT CompileGS( ID3D11Device* pd3dDevice, WCHAR* file_name, LPCSTR func_name );
	virtual HRESULT CompilePS_Solid( ID3D11Device* pd3dDevice, WCHAR* file_name, LPCSTR func_name );
	virtual HRESULT CompilePS_Wireframe( ID3D11Device* pd3dDevice, WCHAR* file_name, LPCSTR func_name );

	void Release();
	
public:
	ID3D11VertexShader* m_pVS;
	ID3D11GeometryShader* m_pGS;
	ID3D11PixelShader* m_pPS_Wireframe ;
	ID3D11PixelShader* m_pPS_Solid;
	ID3D11InputLayout* m_pLayout;
};

static HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
