#include "DXUT.h"
#include "ParticleRender.h"
#include "mystd.h"

ParticleRender::ParticleRender(void)
{
	m_pParticleVB = NULL;
	m_pRenderTexture = NULL;
	
	m_pShader_RenderPoint = NULL ;
	m_pShader_RenderWave = NULL;


	m_pPerCallCB_RenderPoint = NULL ;
	m_pPerCallCB_RenderWave = NULL;


	m_pWaveRect = NULL;

	m_pTexSampler = NULL;


	m_pBasicWaveMap = NULL;

	m_pRS = NULL;
	m_pDepth_On = NULL;
	m_pDepth_Off = NULL;
	m_pAlpha_On = NULL;
	m_pAlpha_Off = NULL;
}


ParticleRender::~ParticleRender(void)
{
}


HRESULT ParticleRender::InitResource(ID3D11Device* pd3dDevice, int width, int height , size_t particlMaxCounts )
{
	HRESULT hr = S_OK;

	m_pWaveRect = new RenderRect(3,3);
	assert( m_pWaveRect );
	hr = m_pWaveRect->InitResource(pd3dDevice);
	V_RETURN(hr);

	//RenderToTexture
	m_pRenderTexture = new RenderTexture;
	hr = m_pRenderTexture->InitResource(pd3dDevice,  width,  height , DXGI_FORMAT_R32G32B32A32_FLOAT );
	V_RETURN(hr);


	SAFE_RELEASE( m_pParticleVB );
	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = particlMaxCounts * sizeof(RenderPoint);
	vb_desc.Usage = D3D11_USAGE_DYNAMIC;
	vb_desc.BindFlags =  D3D11_BIND_VERTEX_BUFFER ;
	vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vb_desc.MiscFlags = 0;
	vb_desc.StructureByteStride = sizeof(RenderPoint);
	pd3dDevice->CreateBuffer(&vb_desc, NULL, &m_pParticleVB);
	assert(m_pParticleVB);
	m_maxParticleCounts = particlMaxCounts;

	//shader
	{
		m_pShader_RenderPoint = new HLSLclass();
		assert( m_pShader_RenderPoint );

		D3D11_INPUT_ELEMENT_DESC mesh_layout_desc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",    0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

		};
		int counts = sizeof(mesh_layout_desc)/sizeof(D3D11_INPUT_ELEMENT_DESC);

		hr = m_pShader_RenderPoint->CompileVSInputLayout( pd3dDevice, L"Shader/particle_render_point.hlsl", "ParticleVS", mesh_layout_desc, counts );
		V_RETURN(hr);
		hr = m_pShader_RenderPoint->CompilePS_Solid( pd3dDevice, L"Shader/particle_render_point.hlsl", "ParticlePS");
		V_RETURN(hr);
	}

	{
		m_pShader_RenderWave = new HLSLclass();
		assert( m_pShader_RenderWave );

		D3D11_INPUT_ELEMENT_DESC mesh_layout_desc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		int counts = sizeof(mesh_layout_desc)/sizeof(D3D11_INPUT_ELEMENT_DESC);

		hr = m_pShader_RenderWave->CompileVSInputLayout( pd3dDevice, L"shader/particle_render_wave.hlsl", "ParticleVS", mesh_layout_desc, counts );
		V_RETURN(hr);
		hr = m_pShader_RenderWave->CompilePS_Solid( pd3dDevice, L"shader/particle_render_wave.hlsl", "ParticlePS");
		V_RETURN(hr);
	}

	//constan buffer
	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_DYNAMIC;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb_desc.MiscFlags = 0;    
	cb_desc.ByteWidth = PAD16( sizeof(Const_Per_Call_RenderPoint) );
	cb_desc.StructureByteStride = 0;
	hr = pd3dDevice->CreateBuffer(&cb_desc, NULL, &m_pPerCallCB_RenderPoint);
	assert(m_pPerCallCB_RenderPoint);

	cb_desc.ByteWidth = PAD16( sizeof(Const_Per_Call_RenderWave) );
	hr = pd3dDevice->CreateBuffer(&cb_desc, NULL, &m_pPerCallCB_RenderWave);
	assert(m_pPerCallCB_RenderWave);


	//sampler
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );



	sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0; 
	sampDesc.MaxAnisotropy = 1; 
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; 
	sampDesc.BorderColor[0] = 1.0f;
	sampDesc.BorderColor[1] = 1.0f;
	sampDesc.BorderColor[2] = 1.0f;
	sampDesc.BorderColor[3] = 1.0f;
	sampDesc.MinLOD = -FLT_MAX;
	sampDesc.MaxLOD = FLT_MAX;
	hr = pd3dDevice->CreateSamplerState( &sampDesc, &m_pTexSampler );
	assert( m_pTexSampler );


	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	

	//SRV
	m_pBasicWaveMap = new MyTexture();
    hr = m_pBasicWaveMap->InitResource(  pd3dDevice, "res/Test1.wt" );
	assert( m_pBasicWaveMap );

	//rander state
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
	hr = pd3dDevice->CreateRasterizerState(&ras_desc, &m_pRS);
	assert(m_pRS);



	//Blend
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory( &blendDesc, sizeof(blendDesc) );
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;//0x0f;

	hr = pd3dDevice->CreateBlendState(&blendDesc, &m_pAlpha_On );
	assert( m_pAlpha_On );

	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	hr = pd3dDevice->CreateBlendState(&blendDesc, &m_pAlpha_Off );
	assert( m_pAlpha_Off );


	//depth
	D3D11_DEPTH_STENCIL_DESC depthDesc;
	ZeroMemory( &depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC) );
	depthDesc.DepthEnable = FALSE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDesc.StencilEnable = FALSE;
	depthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = pd3dDevice->CreateDepthStencilState( &depthDesc, &m_pDepth_Off );
	assert( m_pDepth_Off );

	depthDesc.DepthEnable = TRUE;
	hr = pd3dDevice->CreateDepthStencilState( &depthDesc, &m_pDepth_On );
	assert( m_pDepth_On );

	return hr;
}

void ParticleRender::Release()
{
	SafeReleaseDelete( m_pRenderTexture );
	SafeReleaseDelete( m_pShader_RenderPoint );
	SafeReleaseDelete( m_pShader_RenderWave );
	SafeReleaseDelete( m_pWaveRect );
	SafeReleaseDelete( m_pBasicWaveMap );

	SAFE_RELEASE( m_pAlpha_On );
	SAFE_RELEASE( m_pAlpha_Off );
	SAFE_RELEASE( m_pDepth_On );
	SAFE_RELEASE( m_pDepth_Off );

	SAFE_RELEASE( m_pParticleVB );
	SAFE_RELEASE( m_pPerCallCB_RenderPoint );
	SAFE_RELEASE( m_pPerCallCB_RenderWave );
	SAFE_RELEASE( m_pRS );

	SAFE_RELEASE( m_pTexSampler );

	
}

HRESULT ParticleRender::ResizedSwapChain(ID3D11Device* pd3dDevice, int width, int height  )
{
	DXGI_FORMAT old_format = m_pRenderTexture->GetFormat();
	m_pRenderTexture->Release();
	return m_pRenderTexture->InitResource(pd3dDevice,width, height, old_format);
}

void ParticleRender::RenderPointToTexture(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, RenderParticle* pVB, size_t particleCounts, D3DXVECTOR2 cameraPos, D3DXVECTOR2 area   )
{
	ID3D11RenderTargetView* backbufferRTV =  DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* backbufferDSV =  DXUTGetD3D11DepthStencilView();
	m_pRenderTexture->SetRenderTarget( pd3dContext );
	m_pRenderTexture->ClearRenderTarget(pd3dContext, 0.0,0.0,0.0,1.0 );

	if( particleCounts != 0 )
	{
		// 设置blend因子
		float blendFactor[4];
		blendFactor[0] = 0.0f;
		blendFactor[1] = 0.0f;
		blendFactor[2] = 0.0f;
		blendFactor[3] = 0.0f;
		pd3dContext->OMSetBlendState( m_pAlpha_On, blendFactor, 0xffffffff );
		pd3dContext->OMSetDepthStencilState( m_pDepth_Off, 1);

		pd3dContext->RSSetState( m_pRS );

		m_Camera.SetViewParam( cameraPos.x, 100.0f, cameraPos.y );
		m_Camera.SetOrthoProjectionParam( area.x, area.y, 1.0f, 1000.0f);

		D3DXMATRIX matView = m_Camera.GetViewMatrix();
		D3DXMATRIX matProj = m_Camera.GetOrthoProjectionMatrix();
		D3DXMATRIX matWVP =  matView * matProj;

		pd3dContext->VSSetShader(m_pShader_RenderPoint->m_pVS, NULL, 0);
		pd3dContext->GSSetShader( NULL, NULL, 0 );
		pd3dContext->PSSetShader(m_pShader_RenderPoint->m_pPS_Solid, NULL, 0 );

		// IA 
		RenderPoint* ptrDate = new RenderPoint[ particleCounts ];
		for( size_t i=0; i != particleCounts; ++i )
		{
			ptrDate[i].pos = pVB[i].pos;
			ptrDate[i].color = pVB[i].amp;
		}

		D3D11_MAPPED_SUBRESOURCE map_vb_res;
		size_t temp_particle_counts = min( m_maxParticleCounts, particleCounts );
		HRESULT hr = pd3dContext->Map( m_pParticleVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_vb_res);
		assert( hr == S_OK );
		map_vb_res.DepthPitch = sizeof( RenderPoint )*temp_particle_counts;
		map_vb_res.RowPitch = map_vb_res.DepthPitch ;
		memcpy( map_vb_res.pData, ptrDate, sizeof( RenderPoint )*temp_particle_counts );
		pd3dContext->Unmap( m_pParticleVB, 0 );
		delete [] ptrDate;

		ID3D11Buffer* vbs[1] = { m_pParticleVB  };
		UINT strides[1] = { sizeof( RenderPoint ) };
		UINT offsets[1] = {0};
		pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
		pd3dContext->IASetInputLayout(m_pShader_RenderPoint->m_pLayout );

		//constant
		Const_Per_Call_RenderPoint call_consts;
		D3DXMatrixTranspose(&call_consts.matWorldViewProj, &matWVP);
		// Update constant buffer
		D3D11_MAPPED_SUBRESOURCE mapped_res;            
		pd3dContext->Map(m_pPerCallCB_RenderPoint, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		assert(mapped_res.pData);
		*(Const_Per_Call_RenderPoint*)mapped_res.pData = call_consts;
		pd3dContext->Unmap(m_pPerCallCB_RenderPoint, 0);

		ID3D11Buffer* cbs[1];
		cbs[0] = m_pPerCallCB_RenderPoint;
		pd3dContext->VSSetConstantBuffers(4, 1, cbs);

		//render
		pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		pd3dContext->Draw( temp_particle_counts, 0);

		//unbound
		vbs[0] = NULL;
		pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);

		//关闭alpha
		pd3dContext->OMSetBlendState(  m_pAlpha_Off, blendFactor, 0xffffffff );
		//打开Depth
		pd3dContext->OMSetDepthStencilState( m_pDepth_On, 1);
	}
	
	pd3dContext->OMSetRenderTargets(1, &backbufferRTV, backbufferDSV );
	pd3dContext->ClearDepthStencilView( backbufferDSV, D3D11_CLEAR_DEPTH,1.0 , 0 );

}



void ParticleRender::RenderWaveToTexture(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext ,float time ,RenderParticle* pVB,  size_t particleCounts,  D3DXVECTOR2 cameraPos, D3DXVECTOR2 area )
{
	ID3D11RenderTargetView* backbufferRTV =  DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* backbufferDSV =  DXUTGetD3D11DepthStencilView();
	
	m_pRenderTexture->SetRenderTarget( pd3dContext );
	m_pRenderTexture->ClearRenderTarget(pd3dContext, 0.0,0.0,0.0,1.0 );

	
	// 设置blend因子
	float blendFactor[4];
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	pd3dContext->OMSetBlendState( m_pAlpha_On, blendFactor, 0xffffffff );
	pd3dContext->OMSetDepthStencilState( m_pDepth_Off, 1);

	
	pd3dContext->RSSetState( m_pRS );

	if( particleCounts != 0 )
	{
		m_Camera.SetViewParam( cameraPos.x, 100.0f, cameraPos.y );
		m_Camera.SetOrthoProjectionParam( area.x, area.y, 1.0f, 1000.0f);

		D3DXMATRIX matView = m_Camera.GetViewMatrix();
		D3DXMATRIX matProj = m_Camera.GetOrthoProjectionMatrix();


		pd3dContext->VSSetShader(m_pShader_RenderWave->m_pVS, NULL, 0);
		pd3dContext->GSSetShader( NULL, NULL, 0 );
		pd3dContext->PSSetShader(m_pShader_RenderWave->m_pPS_Solid, NULL, 0 );

		// IA setup
		pd3dContext->IASetIndexBuffer(m_pWaveRect->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		ID3D11Buffer* vbs[1] = { m_pWaveRect->GetVertexBuffer()  };
		UINT strides[1] = {  m_pWaveRect->GetVertexSize()  };
		UINT offsets[1] = {0};
		pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
		pd3dContext->IASetInputLayout(m_pShader_RenderWave->m_pLayout );
		pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//texture in PS
		ID3D11ShaderResourceView*  ps_srvs[] = { m_pBasicWaveMap->GetSRV() };
		pd3dContext->PSSetShaderResources( 0, 1, ps_srvs );
		ID3D11SamplerState*      ps_samplers[] = { m_pTexSampler };
		pd3dContext->PSSetSamplers( 0, 1 , ps_samplers );

		for( size_t i=0; i !=particleCounts; ++i )
		{
			D3DXMATRIX matWorldTrans,matModelScale;
			D3DXVECTOR3 vecWroldTrans = pVB[i].pos;
			D3DXVECTOR3 vecModelScale = D3DXVECTOR3( pVB[i].size, 1, pVB[i].size );
			D3DXMATRIX matModelRotate;
			D3DXMatrixIdentity( &matModelRotate);

			matModelRotate._11 = pVB[i].dir.x;
			matModelRotate._13 = pVB[i].dir.y;
			matModelRotate._31 = -pVB[i].dir.y;
			matModelRotate._33 = pVB[i].dir.x;

			D3DXMatrixTranslation( &matWorldTrans,vecWroldTrans.x, vecWroldTrans.y, vecWroldTrans.z );
			D3DXMatrixScaling( &matModelScale, vecModelScale.x, vecModelScale.y, vecModelScale.z );

			

			static float fYaw,fPitch,fRoll;
			fYaw = fPitch = fRoll =0;
			D3DXQUATERNION qR;
			D3DXMATRIX matRot;
			D3DXMATRIX matTranlate1;
			D3DXMATRIX matTranlate2;
			D3DXMATRIX matModelWorld;

			fYaw = atan(pVB[i].dir.y/pVB[i].dir.x)/*+3.1415926/3*/;
			D3DXQuaternionRotationYawPitchRoll (&qR, fYaw, fPitch, fRoll);	
 			D3DXMatrixRotationQuaternion (&matRot, &qR);


			D3DXMatrixTranslation( &matTranlate1, pVB[i].pos.x, 0,pVB[i].pos.z);
			D3DXMatrixTranslation( &matTranlate2, -pVB[i].pos.x, 0,-pVB[i].pos.z);

			matModelWorld = matModelScale*matTranlate1*matRot*matTranlate2*matWorldTrans;

			D3DXMATRIX matWVP =  matModelWorld*matView * matProj;

			//constant
			Const_Per_Call_RenderWave call_consts;
			D3DXMatrixTranspose(&call_consts.matWorldViewProj, &matWVP);
			call_consts.amp = pVB[i].amp;

			// Update constant buffer
			D3D11_MAPPED_SUBRESOURCE mapped_res;            
			pd3dContext->Map(m_pPerCallCB_RenderWave, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
			assert(mapped_res.pData);
			*(Const_Per_Call_RenderWave*)mapped_res.pData = call_consts;
			pd3dContext->Unmap(m_pPerCallCB_RenderWave, 0);

			ID3D11Buffer* cbs[1];
			cbs[0] = m_pPerCallCB_RenderWave;
			pd3dContext->VSSetConstantBuffers(4, 1, cbs);
			pd3dContext->PSSetConstantBuffers(4, 1, cbs);

			//render
			pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			pd3dContext->DrawIndexed( m_pWaveRect->GetIndexCounts(), 0, 0);
		}
		//unbound
		ps_srvs[0] = NULL;
		pd3dContext->PSSetShaderResources( 0, 1, ps_srvs );

		vbs[0] = NULL;
		pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
	}

	//关闭alpha
	pd3dContext->OMSetBlendState(  m_pAlpha_Off, blendFactor, 0xffffffff );
	//打开Depth
	pd3dContext->OMSetDepthStencilState( m_pDepth_On, 1);

	pd3dContext->OMSetRenderTargets(1, &backbufferRTV, backbufferDSV );
	backbufferRTV =  DXUTGetD3D11RenderTargetView();
	pd3dContext->ClearDepthStencilView( backbufferDSV, D3D11_CLEAR_DEPTH,1.0 , 0 );
}