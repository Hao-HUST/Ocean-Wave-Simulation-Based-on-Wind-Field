#include "DXUT.h"
#include "RenderRect.h"


RenderRect::RenderRect( size_t u_pointsCounts, size_t v_pointsCounts )
	:m_UPointsCounts( max(2, u_pointsCounts) )
	,m_VPointsCounts( max(2, v_pointsCounts) )
	,m_VertexCounts( m_UPointsCounts*m_VPointsCounts )
	,m_FaceCounts( (m_UPointsCounts-1)*(m_VPointsCounts-1)*2 )
	,m_IndexCounts( m_FaceCounts*3 )
{
	m_pMeshVB = NULL;
	m_pMeshIB = NULL;
	m_vTrans = D3DXVECTOR3( 0,0,0);
	m_vScale = D3DXVECTOR3( 1.0f,1.0f,1.0f);
}


RenderRect::~RenderRect(void)
{
}

HRESULT RenderRect::InitResource(ID3D11Device*  pd3dDevice)
{
	assert( pd3dDevice != NULL );

	HRESULT hr = S_OK;
	size_t  UDim = m_UPointsCounts - 1;
	size_t  VDim = m_VPointsCounts - 1;

	MeshVertex* pV = new MeshVertex[m_VertexCounts];
	assert(pV);

	size_t u, v;
	for( v=0; v<m_VPointsCounts; ++v )
	{
		for( u=0; u<m_UPointsCounts; ++u )
		{
			int i =  v*m_UPointsCounts+ u;
			pV[ i ].Tex.x = ((float)u)/UDim;
			pV[ i ].Tex.y = ((float)v)/VDim;

			pV[ i ].Pos.x = pV[ i ].Tex.x  - 0.5f;
			pV[ i ].Pos.y = 0;
			pV[ i ].Pos.z = 0.5f - pV[ i ].Tex.y;
		}
	}

	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = m_VertexCounts * sizeof(MeshVertex);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags =  D3D11_BIND_VERTEX_BUFFER ;
	vb_desc.CPUAccessFlags = 0;
	vb_desc.MiscFlags = 0;
	vb_desc.StructureByteStride = sizeof(MeshVertex);

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = pV;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	SAFE_RELEASE(m_pMeshVB); 

	hr = pd3dDevice->CreateBuffer(&vb_desc, &init_data, &m_pMeshVB);
	assert(m_pMeshVB);

	SAFE_DELETE_ARRAY(pV);

	//Index buffer


	DWORD* index_array = new DWORD[m_IndexCounts];
	assert(index_array);

	DWORD* pIndex = index_array; 
	for(  v=0; v<VDim ; ++v ) 
	{
		for( u=0; u<UDim; ++u )
		{		
			*pIndex++ = (v)*m_UPointsCounts + (u);
			*pIndex++ = (v)*m_UPointsCounts + (u+1);
			*pIndex++ = (v+1)*m_UPointsCounts + (u);

			*pIndex++ = (v)*m_UPointsCounts + (u+1);
			*pIndex++ = (v+1)*m_UPointsCounts + (u+1);
			*pIndex++ = (v+1)*m_UPointsCounts + (u);
			
		}
	}

	D3D11_BUFFER_DESC ib_desc;
	ib_desc.ByteWidth = m_IndexCounts * sizeof( DWORD );
	ib_desc.Usage = D3D11_USAGE_IMMUTABLE;
	ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib_desc.CPUAccessFlags = 0;
	ib_desc.MiscFlags = 0;
	ib_desc.StructureByteStride = sizeof(DWORD);

	init_data.pSysMem = index_array;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	SAFE_RELEASE(m_pMeshIB);
	hr = pd3dDevice->CreateBuffer(&ib_desc, &init_data, &m_pMeshIB);
	assert(m_pMeshIB);

	SAFE_DELETE_ARRAY(index_array);

	return hr;
}

void    RenderRect::Release()
{
	SAFE_RELEASE( m_pMeshIB );
	SAFE_RELEASE( m_pMeshVB );
}