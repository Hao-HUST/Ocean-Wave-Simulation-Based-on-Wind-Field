#include "DXUT.h"
#include "MeshFactory.h"

inline DWORD IndexAtCR(int c, int r, int meshDim ) { return (r)*(meshDim+1) + (c); }

LODMesh::LODMesh( int level )
{
	m_Level = level;
	m_MeshDim = 1<<level;
	m_VertexNum = (m_MeshDim+1)*(m_MeshDim+1);
	m_FaceNum  = 2*m_MeshDim*m_MeshDim;
	m_IndexNum = 3*m_FaceNum;

	m_pMeshIB = NULL;
	m_pMeshVB = NULL;
	m_ModelTrans = D3DXVECTOR3(-0.5f,0,-0.5f);

}

LODMesh::~LODMesh()
{
	Release();
}

HRESULT LODMesh::Init(ID3D11Device* pd3dDevice)
{
	HRESULT hr = S_OK;
	MeshVertex* pV = new MeshVertex[m_VertexNum];
	assert(pV);

	int r, c;
	for (r = 0; r <= m_MeshDim; r++)
	{
		for (c = 0; c <= m_MeshDim; c++)
		{
			pV[r * (m_MeshDim + 1) + c].Pos.x= (float)c/m_MeshDim;
			pV[r * (m_MeshDim + 1) + c].Pos.y = 0;
			pV[r * (m_MeshDim + 1) + c].Pos.z = (float)r/m_MeshDim;

		}
	}

	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = m_VertexNum * sizeof(MeshVertex);
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


	DWORD* index_array = new DWORD[m_IndexNum];
	assert(index_array);

	DWORD* pIndex = index_array; 
	for( DWORD r=0; r<(DWORD)m_MeshDim ; ++r)
	{
		for( DWORD c=0; c< (DWORD)m_MeshDim; ++c)
		{		
			*pIndex++ = IndexAtCR( c,r , m_MeshDim );
			*pIndex++ = IndexAtCR( c,r+1 , m_MeshDim );
			*pIndex++ = IndexAtCR( c+1,r , m_MeshDim );


			*pIndex++ = IndexAtCR( c+1,r , m_MeshDim );
			*pIndex++ = IndexAtCR( c,r+1 , m_MeshDim );	
			*pIndex++ = IndexAtCR( c+1,r+1 , m_MeshDim );

		}
	}

	D3D11_BUFFER_DESC ib_desc;
	ib_desc.ByteWidth = m_IndexNum * sizeof( DWORD );
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

void LODMesh::Release()
{
	SAFE_RELEASE( m_pMeshIB );
	SAFE_RELEASE( m_pMeshVB );
};



MeshFactory::MeshFactory(void)
{
	MeshStorage.clear();
}


MeshFactory::~MeshFactory(void)
{
	Release();
}

LODMesh* MeshFactory::GetLevelMesh( int level )
{
	assert( level >=0 );
	assert( level<= 10 );

	//先检查需要的LOD网格是否存在
	std::map<int,LODMesh*>::iterator iter = MeshStorage.find( level  );

	if( iter != MeshStorage.end() ) //找到了
	{
		return (iter->second);
	}
	else
	{
		LODMesh* pNewMesh = new LODMesh( level );
		ID3D11Device* pd3dDeivce = DXUTGetD3D11Device();
		assert(pd3dDeivce);
		pNewMesh->Init(pd3dDeivce);

		MeshStorage.insert( std::pair<int,LODMesh*>( level, pNewMesh ) );
		return MeshStorage[level];
	}
	
}

void MeshFactory::Release()
{
	for( std::map<int, LODMesh*>::iterator iter = MeshStorage.begin();
		iter != MeshStorage.end(); iter++ )
	{
		iter->second->Release();
		SAFE_DELETE( iter->second)
	}
	MeshStorage.clear();
}

