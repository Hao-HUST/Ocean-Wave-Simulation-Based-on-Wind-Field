#include "DXUT.h"
#include "OpenSeaRender.h"
#include "mystd.h"


OpenSeaRender::OpenSeaRender( int max_Level)
{
	m_pShader_RenderHeightMap = NULL;
	m_pSampleLinearBorder = NULL;
	
	m_pPerCallCB = NULL;
	m_MaxLevel = max_Level;

	m_pMeshFactory = NULL;
}


OpenSeaRender::~OpenSeaRender(void)
{
}

HRESULT OpenSeaRender::InitResource(ID3D11Device* pd3dDevice)
{
	HRESULT hr = S_OK;

	m_pMeshFactory =new MeshFactory();
	assert( m_pMeshFactory );
	
	m_pShader_RenderHeightMap = new HLSLclass();
	assert( m_pShader_RenderHeightMap );

	//shader
	D3D11_INPUT_ELEMENT_DESC mesh_layout_desc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	int counts = sizeof(mesh_layout_desc)/sizeof(D3D11_INPUT_ELEMENT_DESC);

	hr = m_pShader_RenderHeightMap->CompileVSInputLayout( pd3dDevice, L"shader/OpenSeaRender.hlsl", "OpenSeaRenderVS", mesh_layout_desc, counts );
	V_RETURN(hr);
	hr = m_pShader_RenderHeightMap->CompilePS_Wireframe( pd3dDevice, L"shader/OpenSeaRender.hlsl", "OpenSeaRenderPS_Wireframe");
	V_RETURN(hr);
	hr = m_pShader_RenderHeightMap->CompilePS_Solid( pd3dDevice, L"shader/OpenSeaRender.hlsl", "OpenSeaRenderPS_Solid");
	V_RETURN(hr);

	//constan buffer
	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_DYNAMIC;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb_desc.MiscFlags = 0;    
	cb_desc.ByteWidth = PAD16( sizeof(Const_Per_Call) );
	cb_desc.StructureByteStride = 0;
	hr = pd3dDevice->CreateBuffer(&cb_desc, NULL, &m_pPerCallCB);
	assert(m_pPerCallCB);

	//Raster
	D3D11_RASTERIZER_DESC ras_desc;
	ras_desc.FillMode = D3D11_FILL_SOLID; 
	ras_desc.CullMode = D3D11_CULL_NONE; 
	ras_desc.FrontCounterClockwise = FALSE; 
	ras_desc.DepthBias = 0;
	ras_desc.SlopeScaledDepthBias = 0.0f;
	ras_desc.DepthBiasClamp = 0.0f;
	ras_desc.DepthClipEnable= TRUE;
	ras_desc.ScissorEnable = FALSE;
	ras_desc.MultisampleEnable = TRUE;
	ras_desc.AntialiasedLineEnable = FALSE;

	hr = pd3dDevice->CreateRasterizerState(&ras_desc, &m_pRasterState_Solid);
	assert(m_pRasterState_Solid);

	ras_desc.FillMode = D3D11_FILL_WIREFRAME;
	hr = pd3dDevice->CreateRasterizerState(&ras_desc, &m_pRasterState_WireFrame);
	assert( m_pRasterState_WireFrame );

	//sampler
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pd3dDevice->CreateSamplerState( &sampDesc, &m_pSampleLinearBorder );
	assert( m_pSampleLinearBorder );

	return hr;
}

void OpenSeaRender::Release()
{
	SAFE_RELEASE(m_pPerCallCB);
	SAFE_RELEASE( m_pRasterState_Solid );
	SAFE_RELEASE( m_pRasterState_WireFrame );
	SAFE_RELEASE( m_pSampleLinearBorder );
	SafeReleaseDelete( m_pMeshFactory );
	SafeReleaseDelete( m_pShader_RenderHeightMap );
}

void OpenSeaRender::RenderMesh(ID3D11DeviceContext* pd3dContext, const CBaseCamera& camera, float time, OpenSeaWave* pOpenSea )
{
	if( IsInViewArea(camera, 100 ) == false )
		return;

	int lod = DetemineLod(camera);
	int level = min(max(m_MaxLevel - lod, 0),m_MaxLevel);
	
	D3DXMATRIX matView = *camera.GetViewMatrix();
	D3DXMATRIX matProj = *camera.GetProjMatrix();
	D3DXMATRIX matVP =  matView * matProj;

	// VS & PS
	pd3dContext->VSSetShader(m_pShader_RenderHeightMap->m_pVS, NULL, 0);
	pd3dContext->GSSetShader( NULL, NULL, 0 );


	LODMesh* tempMish = m_pMeshFactory->GetLevelMesh( level );

	// IA setup
	pd3dContext->IASetIndexBuffer(tempMish->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer* vbs[1] = { tempMish->GetVertexBuffer()  };
	UINT strides[1] = { tempMish->GetVertexSize() };
	UINT offsets[1] = {0};
	pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
	pd3dContext->IASetInputLayout(m_pShader_RenderHeightMap->m_pLayout );

	D3DXVECTOR2 areaSize = pOpenSea->GetSize();

	// Matrices
	D3DXMATRIX matModelTrans,matScale, matWorld;
	D3DXMatrixIdentity( &matWorld );
	D3DXVECTOR3 vecModelTrans = tempMish->m_ModelTrans;
	D3DXMatrixTranslation( &matModelTrans,vecModelTrans.x, vecModelTrans.y, vecModelTrans.z );
	D3DXMatrixScaling( &matScale, areaSize.x, 1.0f, areaSize.y );
	matWorld = matModelTrans*matScale;

	//constant
	Const_Per_Call call_consts;
	D3DXVECTOR3 vec3EyePos = *camera.GetEyePt();
	D3DXMatrixTranspose(&call_consts.matViewProj, &matVP);
	D3DXMatrixTranspose(&call_consts.matWorld, &matWorld);
	call_consts.vec4eyePos = D3DXVECTOR4( vec3EyePos.x, vec3EyePos.y, vec3EyePos.z, 1.0 );
	call_consts.vec4sunDir = D3DXVECTOR4( 0.93f, -0.3f, 0.07f, 0);
	call_consts.vec4OpenSeaRenderScale = pOpenSea->GetDimScale();


	// Update constant buffer
	D3D11_MAPPED_SUBRESOURCE mapped_res;            
	pd3dContext->Map(m_pPerCallCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	assert(mapped_res.pData);
	*(Const_Per_Call*)mapped_res.pData = call_consts;
	pd3dContext->Unmap(m_pPerCallCB, 0);

	ID3D11Buffer* cbs[1];
	cbs[0] = m_pPerCallCB;
	pd3dContext->VSSetConstantBuffers(2, 1, cbs);
	pd3dContext->PSSetConstantBuffers(2, 1, cbs);

	//texture
	ID3D11ShaderResourceView*  vs_srvs[] = { pOpenSea->GetHeightMapSRV()};
	pd3dContext->VSSetShaderResources( 0, 1, vs_srvs );

	ID3D11SamplerState*      vs_samplers[] = { m_pSampleLinearBorder };
	pd3dContext->VSSetSamplers( 0, 1 , vs_samplers );


	//render
	pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dContext->DrawIndexed(tempMish->GetIndexNum(), 0, 0);

	vbs[0] = NULL;
	pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);

	vs_srvs[0] = NULL;
	pd3dContext->VSSetShaderResources( 0, 1, vs_srvs );
}


void OpenSeaRender::RenderWireframe(ID3D11DeviceContext* pd3dContext, const CBaseCamera& camera, float time, OpenSeaWave* pOpenSea)
{
	pd3dContext->RSSetState( m_pRasterState_WireFrame );
	pd3dContext->PSSetShader(m_pShader_RenderHeightMap->m_pPS_Wireframe, NULL, 0);

	RenderMesh(pd3dContext,camera, time, pOpenSea );

	pd3dContext->RSSetState( m_pRasterState_Solid );
}

void OpenSeaRender::RenderSolid(ID3D11DeviceContext* pd3dContext, const CBaseCamera& camera, float time,OpenSeaWave* pOpenSea)
{
	pd3dContext->RSSetState( m_pRasterState_Solid );
	pd3dContext->PSSetShader(m_pShader_RenderHeightMap->m_pPS_Solid, NULL, 0);

	//texture
	ID3D11ShaderResourceView*  ps_srvs[] = { pOpenSea->GetGradientMapSRV() };
	pd3dContext->PSSetShaderResources( 1, 1, ps_srvs );

	ID3D11SamplerState*      ps_samplers[] = { m_pSampleLinearBorder };
	pd3dContext->PSSetSamplers( 0, 1 , ps_samplers );

	//render
	RenderMesh(pd3dContext,camera, time, pOpenSea);

	ps_srvs[0] = NULL;
	pd3dContext->PSSetShaderResources( 1, 1, ps_srvs );
}

int OpenSeaRender::DetemineLod( const CBaseCamera& camera )
{
	return 0;
}

bool OpenSeaRender::IsInViewArea(const CBaseCamera& camera , float max_height)
{
	return true;
}