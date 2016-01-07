#pragma once
#include <vector>
#include <DXUTcamera.h>
#include <D3DX11.h>
#include "HLSLclass.h"
#include "MyTexture.h"
#include "OpenSeaWave.h"


#define PAD16(n) (((n)+15)/16*16)

class QuartTree
{
public:
	struct qrt_node
	{
		D3DXVECTOR2 left_bottom_pos;
		float       side_length;
		size_t      lod;

		int          sub_left_top;
		int          sub_right_top;
		int          sub_left_bottom;
		int          sub_right_bottom;

		qrt_node();
		bool    IsLeafNode();
		bool    ContainsPoint( D3DXVECTOR2 point );
	};

	struct PatternType
	{
		size_t lod;
		size_t t_degree;
		size_t b_degree;
		size_t l_degree;
		size_t r_degree;
	};


	QuartTree(void);
	~QuartTree(void);

public:
	qrt_node               m_rootNode;
	std::vector<qrt_node>  m_VisibleAreaList;

 	size_t m_MaxGridDimExp;       
	float  m_accuracy;             
	float  m_minCoverage;          

	void     InitResource( size_t maxGridDimExp, float totalSideLength , D3DXVECTOR2 centerPos = D3DXVECTOR2(0,0) );
	void     Release();
	void     BuildRenderList(const CBaseCamera& camera );
	PatternType  GetPatternType( const qrt_node& node );

private:
	int     recur_BuildRenderList( const CBaseCamera& camera, qrt_node& node);
	int		resur_SearchLeaf( int pos , D3DXVECTOR2 point );

	bool    IsInViewArea( const CBaseCamera& camera, const qrt_node& node );
	int     DetemineLod( const CBaseCamera& camera, const qrt_node& node );
	int     DetemineLod_DIY( const CBaseCamera& camera, const qrt_node& node  );

	int     SearchLeaf( D3DXVECTOR2 point );

};


class LodMesh
{
public:
	typedef D3DXVECTOR3 MeshVertex;

	struct PatchRenderParam
	{
		size_t innerMesh_indexCounts;
		size_t innerMesh_indexStartLocation;

		size_t boundaryMesh_indexCounts;
		size_t boundaryMesh_indexStartLoaction;
	};

	PatchRenderParam(* m_pPattern)[3][3][3][3];   

	ID3D11Buffer*      m_pVertexBuffer;
	ID3D11Buffer*      m_pIndexBuffer;
	ID3D11InputLayout* m_pMeshLayout;

	LodMesh();
	~LodMesh();
  

	size_t        m_maxGridDimExp;
	static const size_t  m_minGridDimExp = 2; 

	size_t        m_maxLod;           
	static const size_t  m_minLod = 0;

	size_t        m_maxLevel;           //2^m_maxGridDimExp

	HRESULT InitResource( ID3D11Device* pd3dDevice, size_t maxGridDimExp);
	void    Release();

private:
	inline size_t   Mesh_2D_RC( size_t r, size_t c){ return r*(m_maxLevel+1)+c;}
	static size_t   CaculateIndexBufSize( size_t maxGridDimExp );

	size_t InitInerMeshIndexBuffer( DWORD* pIndexBuf, size_t  GridDimExp, size_t t_degree, size_t b_degree, size_t l_degree, size_t r_degree );//返回Index数
	size_t InitBoundaryMeshIndexBuffer(  DWORD* pIndexBuf, size_t  GridDimExp, size_t t_degree, size_t b_degree, size_t l_degree, size_t r_degree  );//返回Index数
};


class QuartTreeOcean
{
public:
	struct Const_Per_App
	{
		D3DXMATRIX  matPerlinTex;
		D3DXVECTOR4 vec4PerlinScale;
		D3DXVECTOR4 vec4SunDir;
		D3DXVECTOR4 vec4SunColor;
		D3DXVECTOR4 vec4WaterColor;
		D3DXVECTOR4 vec4SkyColor;
		D3DXVECTOR4 vec4PerlinOctave;     
		D3DXVECTOR4 vec4PerlinOctaveAmp;
		float       oceanDeep;
	};

	struct Const_Per_Frame
	{
		D3DXMATRIX  matViewProj;
		D3DXMATRIX  matOpenWaveTex1;
		D3DXVECTOR4 vec4OpenWaveScale1;

		D3DXVECTOR3 vec3EyePos;
		float       time;
		D3DXVECTOR2 vec2PerlinMovement;
	};

	struct Const_Per_Call
	{
		D3DXMATRIX	matWorld;
	};

	QuartTreeOcean();
	~QuartTreeOcean();

	HRESULT InitResource( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, size_t totalSideLengthExp,  size_t maxGridDimExp ,D3DXVECTOR2 centerPos = D3DXVECTOR2(0,0) );
	HRESULT ResizedSwapChain(ID3D11Device* pd3dDevice,  int backbuf_width, int backbuf_height );
	void    Release();
	void    RenderWireframe(  ID3D11DeviceContext* pd3dContext, float time,  const CBaseCamera&  camera );
	void    RenderShaded( ID3D11DeviceContext* pd3dContext, float time,  const CBaseCamera&  camera );
	void    Update( float time, float deatTime );

	size_t  GetNum()  {return num;}
private:
	void   RenderPatchs( ID3D11DeviceContext* pd3dContext, float time,  const CBaseCamera&  camera );
	void   SetShaderConstant( ID3D11DeviceContext* pd3dContext );
	HRESULT CreateFresenlMap( ID3D11Device* pd3dDevice );

public:
	QuartTree  QTpart;
	LodMesh    LMpart;

	HLSLclass*    m_pOceanShader;
	ID3D11Buffer* m_pPerCallCB;
	ID3D11Buffer* m_pPerFrameCB;
	ID3D11Buffer* m_pPerAppCB;

	ID3D11RasterizerState*  m_pRasterState_WireFrame;
	ID3D11RasterizerState*  m_pRasterState_Solid;


	ID3D11SamplerState*  m_pSampleLinearBorder;
	ID3D11SamplerState*  m_pSampleLinearWrap;

	MyTexture*  m_pPerlinNoiseMap;


	ID3D11ShaderResourceView*  m_pFresenlSRV;
	

	D3DXVECTOR2 m_PerlinTexMovement;
	D3DXVECTOR3 m_perlinScale;
	D3DXMATRIX m_matPerlinTex;


	OpenSeaWave* m_pOpenWave1;

	size_t       num;
};

