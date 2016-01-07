#include "DXUT.h"
#include "GradMapRender.h"

GradMapRender::GradMapRender()
{
	m_pRenderTexture = NULL;
	m_pPerCallCB = NULL;
	m_pShader_RenderGradMap = NULL;
	m_pRS_Solid = NULL;
	m_pMesh = NULL;
	m_pTexSampler = NULL;
}

GradMapRender::~GradMapRender()
{
}

HRESULT GradMapRender::InitResource(ID3D11Device* pd3dDevice, int width, int height )
{
	HRESULT hr = S_OK;

	m_pMesh = new RenderRect(2,2);
	assert( m_pMesh );
	hr = m_pMesh->InitResource(pd3dDevice);
	V_RETURN(hr);

	//camera
	m_Camera.SetViewParam( 0,10,0);
	m_Camera.SetOrthoProjectionParam(1.0f,1.0f,1.0f,100.0f);

	//RTT
	m_r_pixelHeight = 1.0f/height;
	m_r_pixelWidth = 1.0f/width;

	m_pRenderTexture = new RenderTexture();
	hr = m_pRenderTexture->InitResource( pd3dDevice, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT );
	V_RETURN(hr);

	//shader
	m_pShader_RenderGradMap = new HLSLclass;
	assert( m_pShader_RenderGradMap );


	D3D11_INPUT_ELEMENT_DESC mesh_layout_desc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	int counts = sizeof(mesh_layout_desc)/sizeof(D3D11_INPUT_ELEMENT_DESC);
	hr = m_pShader_RenderGradMap->CompileVSInputLayout(pd3dDevice, L"Shader/gradmap.hlsl", "VS",mesh_layout_desc,  counts);
	V_RETURN(hr);
	hr = m_pShader_RenderGradMap->CompilePS_Solid(pd3dDevice,  L"Shader/gradmap.hlsl", "PS");
	V_RETURN(hr);

	//const buffer
	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_DYNAMIC;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb_desc.MiscFlags = 0;    
	cb_desc.ByteWidth = PAD16( sizeof(Const_Per_Call) );
	cb_desc.StructureByteStride = 0;
	hr = pd3dDevice->CreateBuffer(&cb_desc, NULL, &m_pPerCallCB);
	assert(m_pPerCallCB);

	//sampler
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pd3dDevice->CreateSamplerState( &sampDesc, &m_pTexSampler );
	assert( m_pTexSampler );


	D3D11_RASTERIZER_DESC ras_desc;
	ras_desc.FillMode = D3D11_FILL_SOLID; 
	ras_desc.CullMode = D3D11_CULL_BACK; 
	ras_desc.FrontCounterClockwise = FALSE; 
	ras_desc.DepthBias = 0;
	ras_desc.SlopeScaledDepthBias = 0.0f;
	ras_desc.DepthBiasClamp = 0.0f;
	ras_desc.DepthClipEnable= TRUE;
	ras_desc.ScissorEnable = FALSE;
	ras_desc.MultisampleEnable = TRUE;
	ras_desc.AntialiasedLineEnable = FALSE;
	hr = pd3dDevice->CreateRasterizerState(&ras_desc, &m_pRS_Solid);
	assert(m_pRS_Solid);
	return hr;
}

void GradMapRender::Release()
{
	SafeReleaseDelete( m_pShader_RenderGradMap);
	SafeReleaseDelete( m_pRenderTexture );
	SafeReleaseDelete( m_pMesh );
	SAFE_RELEASE( m_pTexSampler );
	SAFE_RELEASE( m_pPerCallCB );
	SAFE_RELEASE( m_pRS_Solid );
}


ID3D11ShaderResourceView* GradMapRender::GetGradMapSRV()
{	
	assert( m_pRenderTexture );
	return  m_pRenderTexture->GetShaderResourceView();
}

HRESULT GradMapRender::ResizedSwapChain(ID3D11Device* pd3dDevice, int width, int height )
{
	DXGI_FORMAT old_format = m_pRenderTexture->GetFormat();
	m_pRenderTexture->Release();
	m_r_pixelHeight = 1.0f/height;
	m_r_pixelWidth = 1.0f/width;
	return m_pRenderTexture->InitResource(pd3dDevice, width, height, old_format );
}

void GradMapRender::RenderGradMap(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView* pHeightMap )
{
	ID3D11RenderTargetView* backbufferRTV =  DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* backbufferDSV =  DXUTGetD3D11DepthStencilView();


	m_pRenderTexture->SetRenderTarget( pd3dContext  );
	m_pRenderTexture->ClearRenderTarget(pd3dContext, 0.0,0.0,0.0,0.0 );

	pd3dContext->RSSetState( m_pRS_Solid );

	// VS & PS
	pd3dContext->VSSetShader(m_pShader_RenderGradMap->m_pVS, NULL, 0);
	pd3dContext->GSSetShader( NULL, NULL, 0 );
	pd3dContext->PSSetShader(m_pShader_RenderGradMap->m_pPS_Solid, NULL, 0);

	// Matrices
	m_Camera.SetViewParam( 0, 100.0f, 0 );
	m_Camera.SetOrthoProjectionParam( 1.0f, 1.0f, 1.0f, 1000.0f);

	D3DXMATRIX matView = m_Camera.GetViewMatrix();
	D3DXMATRIX matProj = m_Camera.GetOrthoProjectionMatrix();

	D3DXMATRIX matWVP = matView * matProj;

	// IA setup
	pd3dContext->IASetIndexBuffer(m_pMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer* vbs[1] = { m_pMesh->GetVertexBuffer()  };
	UINT strides[1] = {m_pMesh->GetVertexSize()};
	UINT offsets[1] = {0};
	pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
	pd3dContext->IASetInputLayout(m_pShader_RenderGradMap->m_pLayout);

	//constant
	Const_Per_Call call_consts;
	D3DXMatrixTranspose(&call_consts.matWorldViewProj, &matWVP);
	call_consts.detaUV = D3DXVECTOR2( m_r_pixelWidth, m_r_pixelHeight );

	// Update constant buffer
	D3D11_MAPPED_SUBRESOURCE mapped_res;            
	pd3dContext->Map(m_pPerCallCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	assert(mapped_res.pData);
	*(Const_Per_Call*)mapped_res.pData = call_consts;
	pd3dContext->Unmap(m_pPerCallCB, 0);

	ID3D11Buffer* cbs[1];
	cbs[0] = m_pPerCallCB;
	pd3dContext->VSSetConstantBuffers(4, 1, cbs);
	pd3dContext->PSSetConstantBuffers(4, 1, cbs);

	//texture
	ID3D11ShaderResourceView*  ps_srvs[] = { pHeightMap };
	pd3dContext->PSSetShaderResources( 0, 1, ps_srvs );
	ID3D11SamplerState*      ps_samplers[] = {m_pTexSampler};
	pd3dContext->PSSetSamplers( 0, 1 , ps_samplers );

	//render
	pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dContext->DrawIndexed(m_pMesh->GetIndexCounts(), 0, 0);

	ps_srvs[0] = NULL;
	pd3dContext->PSSetShaderResources( 0, 1, ps_srvs );

	vbs[0] = NULL;
	pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);

	pd3dContext->OMSetRenderTargets(1, &backbufferRTV, backbufferDSV );
	pd3dContext->ClearDepthStencilView( backbufferDSV, D3D11_CLEAR_DEPTH,1.0 , 0 );

	//genmips
	pd3dContext->GenerateMips( m_pRenderTexture->GetShaderResourceView() );
}