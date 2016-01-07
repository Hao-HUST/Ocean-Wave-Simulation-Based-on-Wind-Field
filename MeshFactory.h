#pragma once
#include <D3DX11.h>
#include <map>



class LODMesh
{
public:
	struct MeshVertex
	{
		D3DXVECTOR3  Pos;
	};

public:
	int m_Level;             
	int m_MeshDim;           
	int m_VertexNum;         
	int m_IndexNum;         
	int m_FaceNum;          

	ID3D11Buffer* m_pMeshVB ;
	ID3D11Buffer* m_pMeshIB ;
	D3DXVECTOR3  m_ModelTrans;  

public:
	LODMesh( int lod );
	~LODMesh();

	inline ID3D11Buffer* GetIndexBuffer() {return m_pMeshIB;}
	inline ID3D11Buffer* GetVertexBuffer() { return m_pMeshVB; }
	inline int           GetIndexNum() { return m_IndexNum; }
	inline int           GetVertexSize() { return sizeof(MeshVertex); }

	HRESULT Init(ID3D11Device*  pd3dDevice);
	void    Release();
};

class MeshFactory
{
public:
	MeshFactory(void);
	~MeshFactory(void);

	LODMesh* GetLevelMesh( int level );
	void     Release();

private:
	std::map<int, LODMesh*> MeshStorage;
};

