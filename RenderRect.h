#pragma once
#include <D3DX11.h>
#include "mystd.h"

class RenderRect
{
public:
	struct MeshVertex
	{
		D3DXVECTOR3  Pos;
		D3DXVECTOR2  Tex;
	};

public:
	RenderRect( size_t u_pointsCounts, size_t v_pointsCounts );
	~RenderRect( );

private:
	RenderRect( const RenderRect& ){};

public:
	HRESULT InitResource(ID3D11Device*  pd3dDevice);
	void    Release();
	inline void    MoveTo( D3DXVECTOR3 pos ) { m_vTrans = pos; }
	inline void    SetScale( D3DXVECTOR3 sclae ) { m_vScale = sclae; }
	inline ID3D11Buffer*  GetIndexBuffer()  { return m_pMeshIB ;}
	inline ID3D11Buffer*  GetVertexBuffer()  { return m_pMeshVB; }
	inline int     GetVertexSize( ){ return sizeof(MeshVertex); }
	inline int     GetIndexCounts() { return m_IndexCounts;}

private:
	//U V 方向的细分点数
	 size_t m_UPointsCounts;
	 size_t m_VPointsCounts;
	 size_t m_VertexCounts;
	 size_t m_FaceCounts;
	 size_t m_IndexCounts;


	ID3D11Buffer* m_pMeshVB ;
	ID3D11Buffer* m_pMeshIB ;
	
public:
	D3DXVECTOR3   m_vTrans;
	D3DXVECTOR3   m_vScale;
};

