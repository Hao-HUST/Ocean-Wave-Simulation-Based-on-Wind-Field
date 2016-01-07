#include "DXUT.h"
#include "QuartTreeOcean.h"
#include "mystd.h"
#include <stdlib.h>
#include "MyTexture.h"

QuartTree::QuartTree(void)
{
	m_accuracy = 0.3f; 
	m_minCoverage = 32.0f;
}


QuartTree::~QuartTree(void)
{
}

QuartTree::qrt_node::qrt_node()
{
	ZeroMemory( this, sizeof( qrt_node ) );
	lod = 0xffffffff;
	sub_left_top = sub_right_top = sub_left_bottom = sub_right_bottom = -1;
};

void QuartTree::BuildRenderList(const CBaseCamera& camera )
{
	m_VisibleAreaList.clear();
	m_rootNode.sub_left_bottom = -1;
	m_rootNode.sub_left_top = -1;
	m_rootNode.sub_right_bottom = -1;
	m_rootNode.sub_right_top = -1;
	recur_BuildRenderList( camera, m_rootNode );
}


int QuartTree::recur_BuildRenderList(const CBaseCamera& camera, qrt_node& node)
{
	if( IsInViewArea( camera, node ) == false  )
		return -1;

	int lod = DetemineLod( camera, node );
	if( lod == -1  )
	{
		qrt_node left_top, right_top, left_bottom, right_bottom;
		left_top.left_bottom_pos = node.left_bottom_pos + D3DXVECTOR2(0,0.5f)*node.side_length;
		right_top.left_bottom_pos = node.left_bottom_pos + D3DXVECTOR2(0.5f, 0.5f)*node.side_length;
		left_bottom.left_bottom_pos = node.left_bottom_pos + D3DXVECTOR2(0,0)*node.side_length;
		right_bottom.left_bottom_pos = node.left_bottom_pos + D3DXVECTOR2(0.5, 0)*node.side_length;

		left_top.side_length = right_top.side_length = left_bottom.side_length = right_bottom.side_length = node.side_length*0.5f;
		
		node.sub_left_top = recur_BuildRenderList( camera, left_top );
		node.sub_right_top = recur_BuildRenderList( camera, right_top );
		node.sub_left_bottom = recur_BuildRenderList( camera, left_bottom );
		node.sub_right_bottom = recur_BuildRenderList( camera, right_bottom );

		if( node.IsLeafNode() )
		{
			 return -1;
		}
		
	}
	else
	{
		node.lod = lod;
	}

	int pos = (int)m_VisibleAreaList.size();
	m_VisibleAreaList.push_back( node );
	return pos;
}

bool QuartTree::qrt_node::IsLeafNode()
{
	if( sub_left_bottom == -1 && sub_right_bottom == -1 && sub_left_top == -1 && sub_right_top ==-1 )
		return true;
	else
		return false;
}

void QuartTree::InitResource( size_t maxGridDimExp, float totalSideLength , D3DXVECTOR2 centerPos /* = D3DXVECTOR2 */)
{
	m_MaxGridDimExp = maxGridDimExp;
	m_rootNode.left_bottom_pos = centerPos + D3DXVECTOR2( -totalSideLength*0.5f, -totalSideLength*0.5f );
	m_rootNode.side_length = totalSideLength;

	m_VisibleAreaList.clear();
}

void QuartTree::Release()
{
	m_VisibleAreaList.clear();
}

bool QuartTree::qrt_node::ContainsPoint( D3DXVECTOR2 point )
{
	if( point.x>= left_bottom_pos.x && point.y >= left_bottom_pos.y
	 && point.x<=left_bottom_pos.x+ side_length && point.y <= left_bottom_pos.y + side_length  )
		return true;
	else
		return false;
}

int QuartTree::SearchLeaf( D3DXVECTOR2 point )
{
	size_t size = m_VisibleAreaList.size();
	if( size == 0 || m_VisibleAreaList[size-1].ContainsPoint(point) == false )
		return -1;
	else
		return resur_SearchLeaf( size-1, point );
}

int QuartTree::resur_SearchLeaf( int pos , D3DXVECTOR2 point )
{
	qrt_node tempNode = m_VisibleAreaList[pos];
	if( tempNode.IsLeafNode() )
	{
		return pos;
	}
	else
	{
		if( tempNode.sub_left_top != -1 && m_VisibleAreaList[tempNode.sub_left_top].ContainsPoint(point) )
			return resur_SearchLeaf( tempNode.sub_left_top, point );
		else if(  tempNode.sub_right_top != -1 && m_VisibleAreaList[tempNode.sub_right_top].ContainsPoint(point) )
			return resur_SearchLeaf( tempNode.sub_right_top, point );
		else if(  tempNode.sub_left_bottom != -1 && m_VisibleAreaList[tempNode.sub_left_bottom].ContainsPoint(point) )
			return resur_SearchLeaf( tempNode.sub_left_bottom, point );
		else if(  tempNode.sub_right_bottom != -1 && m_VisibleAreaList[tempNode.sub_right_bottom].ContainsPoint(point) )
			return resur_SearchLeaf( tempNode.sub_right_bottom, point );
		else
			return -1;
	}
}

QuartTree::PatternType QuartTree::GetPatternType( const qrt_node& node )
{
	PatternType pt;
	pt.lod = node.lod;

	D3DXVECTOR2 pInTop,pInBottom,pInLeft,pInRight;
	float deta = min( 1.0f, 0.01f*node.side_length );
	pInTop.x = pInBottom.x = node.left_bottom_pos.x + 0.5f*node.side_length;
	pInTop.y = node.left_bottom_pos.y + node.side_length + deta;
	pInBottom.y =  node.left_bottom_pos.y - deta;

	pInLeft.y = pInRight.y =  node.left_bottom_pos.y + 0.5f*node.side_length;
	pInLeft.x = node.left_bottom_pos.x - deta;
	pInRight.x = node.left_bottom_pos.x + node.side_length + deta;

	D3DXVECTOR2 poins[] = { pInTop, pInBottom, pInLeft,pInRight};
	int rst[4];
	for( int i=0; i !=4 ; ++i )
	{
		int location = SearchLeaf( poins[i] );
		if( location == -1)
			rst[i] = 0;
		else
		{
			qrt_node adj_node = m_VisibleAreaList[ location ];
			float resolution = adj_node.side_length/node.side_length*powf(2.0f, (float)adj_node.lod - node.lod );
			if( resolution >3.9f )
				rst[i] = 2;
			else if( resolution > 1.9f )
				rst[i] = 1;
			else
				rst[i]= 0;
		}
	}

	pt.t_degree = rst[0];
	pt.b_degree = rst[1];
	pt.l_degree = rst[2];
	pt.r_degree = rst[3];

	return pt;
}

bool    QuartTree::IsInViewArea( const CBaseCamera& camera, const qrt_node& node )
{
	D3DXVECTOR4 connerPoint[8]; 
	D3DXMATRIX matView = *camera.GetViewMatrix(); 
	D3DXVECTOR4 point[8];
	float offset = 200;
	point[0]= D3DXVECTOR4( node.left_bottom_pos.x , 0, node.left_bottom_pos.y, 1 );
	D3DXVec4Transform( connerPoint, &point[0], &matView );
	point[1] =  D3DXVECTOR4( node.left_bottom_pos.x+node.side_length , 0, node.left_bottom_pos.y, 1 );
	D3DXVec4Transform( connerPoint+1, &point[1], &matView );
	point[2] =  D3DXVECTOR4( node.left_bottom_pos.x , 0, node.left_bottom_pos.y+node.side_length, 1 );
	D3DXVec4Transform( connerPoint+2, &point[2], &matView );
	point[3] =  D3DXVECTOR4( node.left_bottom_pos.x+node.side_length , 0, node.left_bottom_pos.y+node.side_length, 1 );
	D3DXVec4Transform( connerPoint+3, &point[3], &matView );
	point[4]= D3DXVECTOR4( node.left_bottom_pos.x , offset, node.left_bottom_pos.y, 1 );
	D3DXVec4Transform( connerPoint+4, &point[4], &matView );
	point[5] =  D3DXVECTOR4( node.left_bottom_pos.x+node.side_length , offset, node.left_bottom_pos.y, 1 );
	D3DXVec4Transform( connerPoint+5, &point[5], &matView );
	point[6] =  D3DXVECTOR4( node.left_bottom_pos.x , offset, node.left_bottom_pos.y+node.side_length, 1 );
	D3DXVec4Transform( connerPoint+6, &point[6], &matView );
	point[7] =  D3DXVECTOR4( node.left_bottom_pos.x+node.side_length , offset, node.left_bottom_pos.y+node.side_length, 1 );
	D3DXVec4Transform( connerPoint+7, &point[7], &matView );

	D3DXMATRIX matProj = *camera.GetProjMatrix();

	float a = atan( 1/matProj(0,0) );
	D3DXVECTOR4 left_plane( cosf(a), 0 , sinf(a), 0 );
	D3DXVECTOR4 right_plane = left_plane; right_plane.x *= -1.0f;

	a = atan( 1/matProj(1,1) );
	D3DXVECTOR4 bottom_plane(0, cosf(a), sinf(a), 0 );
	D3DXVECTOR4 top_plane = bottom_plane; top_plane.y *= -1.0f;

	float z_min = camera.GetNearClip();
	float z_max  = camera.GetFarClip();
	D3DXVECTOR4 back_plane( 0, 0, 1, -z_min );
	D3DXVECTOR4 front_plane( 0,0, -1, z_max );

	float ret[8][6];
	D3DXVECTOR4 plane[]={ top_plane, bottom_plane , left_plane, right_plane, front_plane, back_plane };
	for( int i=0; i<8; ++i )
	{
		for( int j=0; j<6; ++j )
		{
			ret[i][j] = D3DXVec4Dot( &plane[j], &connerPoint[i] );
		}
	}

	bool rst = true;
	for( int i=0; i<6; ++i )
	{
		if(   ret[0][i]<0 &&  ret[1][i]<0 && ret[2][i]<0 && ret[3][i]<0
			&&ret[4][i]<0 &&  ret[5][i]<0 && ret[6][i]<0 && ret[7][i]<0)
		{
			rst = false;
			break;
		}
	}


	return rst;
	
}


int  QuartTree::DetemineLod( const CBaseCamera& camera, const qrt_node& node )
	//返回[-1, maxLod]，-1表示还要继续分解
{
	float max_level = (float)(1<<(m_MaxGridDimExp));
	const static float sample_pos[16][2] =
	{
		{0, 0},
		{0, 1},
		{1, 0},
		{1, 1},
		{0.5f, 0.333f},
		{0.25f, 0.667f},
		{0.75f, 0.111f},
		{0.125f, 0.444f},
		{0.625f, 0.778f},
		{0.375f, 0.222f},
		{0.875f, 0.556f},
		{0.0625f, 0.889f},
		{0.5625f, 0.037f},
		{0.3125f, 0.37f},
		{0.8125f, 0.704f},
		{0.1875f, 0.148f},
	};

	D3DXMATRIX matProj = *camera.GetProjMatrix();
	D3DXVECTOR3 eye_point = *camera.GetEyePt();
	eye_point = D3DXVECTOR3(eye_point.x,eye_point.y, eye_point.z);

	float min_distance = (float)((size_t) -1);	
	for (int i = 0; i < 16; i++)
	{
		D3DXVECTOR3 test_point( node.left_bottom_pos.x + sample_pos[i][0]*node.side_length, 0, node.left_bottom_pos.y+sample_pos[i][1]*node.side_length);
		D3DXVECTOR3 eye_vec = test_point - eye_point;
		float dist = D3DXVec3Length(&eye_vec);
		if( dist< min_distance )
			min_distance = dist;
	}

	float grid_len_world = node.side_length / max_level;
	float area_world = grid_len_world * grid_len_world;// * abs(eye_point.z) / sqrt(nearest_sqr_dist);
	float max_area_proj = area_world * matProj(0, 0) * matProj(1, 1) / (min_distance * min_distance);

	UINT num_vps = 1;
	D3D11_VIEWPORT vp;
	DXUTGetD3D11DeviceContext()->RSGetViewports(&num_vps, &vp);
	float screen_area = vp.Height* vp.Width;
	float pixel_coverage = max_area_proj * screen_area * 0.25f;

	if (pixel_coverage > m_minCoverage && node.side_length > max_level*m_accuracy)
		return -1;
	else
	{
		 		int rst = 0;
		 		while( pixel_coverage< m_minCoverage && rst< (int)m_MaxGridDimExp - 2 )
		 		{
		 			pixel_coverage*=4.0f;
		 			rst+=1;
		 		}
		 		return rst;
	}
}

int  QuartTree::DetemineLod_DIY( const CBaseCamera& camera, const qrt_node& node  )
{
	float max_level = (float)(1<<(m_MaxGridDimExp));
	 
	D3DXMATRIX matView = *camera.GetViewMatrix();
	D3DXMATRIX matProj = *camera.GetProjMatrix();
	 	
	D3DXMATRIX matWVP = matView*matProj;
	D3DXVECTOR4 pos_lb = D3DXVECTOR4( node.left_bottom_pos.x, 0, node.left_bottom_pos.y, 1);
	D3DXVECTOR4 pos_rb = pos_lb; pos_rb.x+=node.side_length;
	D3DXVECTOR4 pos_lt = pos_lb; pos_lt.z+=node.side_length;
	D3DXVECTOR4 pos_rt = pos_lt; pos_rt.x+=node.side_length;
	 
	D3DXVECTOR4 point[4];
	D3DXVec4Transform( &point[0], &pos_lb, &matWVP);
	D3DXVec4Transform( &point[1], &pos_rb, &matWVP);
	D3DXVec4Transform( &point[2], &pos_lt, &matWVP);
	D3DXVec4Transform( &point[3], &pos_rt, &matWVP);

	D3DXVECTOR3 ndc[4];
	for( int i=0; i<4; ++i )
	{
	 	point[i]/=point[i].w;
	 	ndc[i].x = max(-1, min(1, point[i].x ) );
	 	ndc[i].y = max(-1, min(1, point[i].y ) );
	 	ndc[i].z = 1;
	}
	 
	D3DXVECTOR3 edge_l,edge_r,edge_t,edge_b;
	edge_l = ndc[2] - ndc[0];
	edge_r = ndc[3] - ndc[1];
	edge_t = ndc[3] - ndc[2];
	edge_b = ndc[1] - ndc[0];
	  
	D3DXVECTOR3 tri_lb, tri_rt ;
	D3DXVec3Cross( &tri_lb, &edge_l, &edge_b);
	D3DXVec3Cross( &tri_rt, &edge_r, &edge_t);
	float area_clip = ( D3DXVec3Length( &tri_lb ) + D3DXVec3Length( &tri_rt ) )/2.0f;
	float ratio = area_clip/4.0f;
	 
	for( int i=0; i<4; ++i )
	{
	 	ndc[i].x = point[i].x;
	 	ndc[i].y = point[i].y;
	 	ndc[i].z = 1;
	}
	 
	edge_l = ndc[2] - ndc[0];
	edge_r = ndc[3] - ndc[1];
	edge_t = ndc[3] - ndc[2];
	edge_b = ndc[1] - ndc[0];
	 
	D3DXVec3Cross( &tri_lb, &edge_l, &edge_b);
	D3DXVec3Cross( &tri_rt, &edge_r, &edge_t);
	float area_noClip = ( D3DXVec3Length( &tri_lb ) + D3DXVec3Length( &tri_rt ) )/2.0f;
	float tri_ratio = area_noClip/max_level/max_level;
	 
	 	
	if( (node.side_length > max_level) && ( tri_ratio > 0.0001 && ratio>0.1 ) )
	{
	 	return  -1;
	}
	else
	{
	 	int rst = 0;
	 	while( tri_ratio< 0.001 && rst< (int)m_MaxGridDimExp - 2 )
	 	{
	 		tri_ratio*=4.0f;
	 		rst+=1;
	 	}
	 	return rst;
	}
	 	
}


LodMesh::LodMesh()
{
	m_pIndexBuffer = NULL;
	m_pVertexBuffer = NULL;
	m_pPattern = NULL;
}

LodMesh::~LodMesh()
{
}

HRESULT LodMesh::InitResource(ID3D11Device* pd3dDevice, size_t maxGridDimExp)
{
	if( m_maxGridDimExp <2 )
		return S_FALSE;
	
	assert( maxGridDimExp <= 10 ); //不希望太大

	HRESULT hr = S_OK;
	m_maxGridDimExp = maxGridDimExp;
	m_maxLod = maxGridDimExp - 2;
	m_maxLevel = 1<<maxGridDimExp;

	//VertexBuffer
	size_t max_level = 1<<m_maxGridDimExp;
	size_t width = max_level+1;
	size_t height = max_level+1;
	size_t vertexCounts = width*height;
	MeshVertex*  pV = new MeshVertex[vertexCounts];
	assert( pV );
	size_t index = 0;
	for( size_t r=0; r< height; ++r )
	{
		for( size_t c=0; c < width; ++c )
		{
			pV[ index ].x = float(c);
			pV[ index ].y = 0;
			pV[ index ].z = float(r); 
			++index;
		}
	}

	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = vertexCounts * sizeof(MeshVertex);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags =  D3D11_BIND_VERTEX_BUFFER ;
	vb_desc.CPUAccessFlags = 0;
	vb_desc.MiscFlags = 0;
	vb_desc.StructureByteStride = sizeof(MeshVertex);

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = pV;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	SAFE_RELEASE(m_pVertexBuffer); 
	hr = pd3dDevice->CreateBuffer(&vb_desc, &init_data, &m_pVertexBuffer);
	assert(m_pVertexBuffer);

	SAFE_DELETE_ARRAY(pV);

	

	//index buf
	size_t indexCounts = CaculateIndexBufSize( maxGridDimExp );
	DWORD* pI = new DWORD[indexCounts];
	assert( pI );
	m_pPattern = new PatchRenderParam [m_maxLod+1][3][3][3][3];
	size_t offset = 0;

	for( size_t lod = 0; lod <= m_maxLod; ++lod  )
	{
		size_t  GridDimExp= maxGridDimExp - lod;

		for( size_t top_detaDimExp = 0; top_detaDimExp < 3; ++top_detaDimExp )
		{
			for( size_t bottom_detaDimExp =0; bottom_detaDimExp < 3; ++bottom_detaDimExp  )
			{
				for( size_t left_detaDimExp = 0; left_detaDimExp < 3; ++left_detaDimExp )
				{
					for( size_t right_detaDimExp = 0; right_detaDimExp < 3; ++right_detaDimExp )
					{
						PatchRenderParam& pattern = m_pPattern[lod][top_detaDimExp][bottom_detaDimExp][left_detaDimExp][right_detaDimExp];
						pattern.innerMesh_indexStartLocation = offset;
						pattern.innerMesh_indexCounts = InitInerMeshIndexBuffer( pI+offset, GridDimExp, top_detaDimExp, bottom_detaDimExp, left_detaDimExp, right_detaDimExp);
						offset += pattern.innerMesh_indexCounts;

						pattern.boundaryMesh_indexStartLoaction = offset;
						pattern.boundaryMesh_indexCounts = InitBoundaryMeshIndexBuffer( pI+offset, GridDimExp, top_detaDimExp, bottom_detaDimExp, left_detaDimExp, right_detaDimExp);
						offset += pattern.boundaryMesh_indexCounts;
					}
				}
			}
		}
	}

	assert(offset == indexCounts);
	

	D3D11_BUFFER_DESC ib_desc;
	ib_desc.ByteWidth = indexCounts * sizeof( DWORD );
	ib_desc.Usage = D3D11_USAGE_IMMUTABLE;
	ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib_desc.CPUAccessFlags = 0;
	ib_desc.MiscFlags = 0;
	ib_desc.StructureByteStride = sizeof(DWORD);

	init_data.pSysMem = pI;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	SAFE_RELEASE(m_pIndexBuffer);
	hr = pd3dDevice->CreateBuffer(&ib_desc, &init_data, &m_pIndexBuffer);
	assert(m_pIndexBuffer);

	SAFE_DELETE_ARRAY( pI );


	return S_OK;
}


size_t LodMesh::InitInerMeshIndexBuffer( DWORD* pIndexBuf, size_t GridDimExp, size_t t_degree, size_t b_degree, size_t l_degree, size_t r_degree )
{
	if( GridDimExp < 2 )
		return 0;

	size_t level = 1<<GridDimExp;

	RECT inner_rect;
	inner_rect.bottom = b_degree == 0 ? 0 : 1;
	inner_rect.top    = t_degree == 0 ? level : level -1;
	inner_rect.left   = l_degree == 0 ? 0 : 1;
	inner_rect.right  = r_degree == 0 ? level : level -1;



	size_t counts = 0;
	bool   rever = false;
	for( int r = inner_rect.bottom; r < inner_rect.top ; ++r ) //height次
	{
		if( rever == false )
		{
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r+1,inner_rect.left );
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r,inner_rect.left);
			

			for( int c =inner_rect.left+ 1; c <= inner_rect.right; ++c ) //width次
			{
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+1, c );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r, c );
				
			}		
		}
		else
		{
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r+1,inner_rect.right );
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r,inner_rect.right );
			
			
			
			for( int c = (inner_rect.right-1); c>= inner_rect.left ; --c ) //width次
			{
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+1, c );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r, c );
				
			}	
		}
		rever = !rever;
	}

	return counts;
}

size_t LodMesh::InitBoundaryMeshIndexBuffer( DWORD* pIndexBuf, size_t GridDimExp, size_t t_degree, size_t b_degree, size_t l_degree, size_t r_degree )
{
	if( GridDimExp < 2 )
		return 0;

	size_t level = 1<<GridDimExp;
	size_t counts = 0;
	
	size_t width = level;  //实际有width+1个点
	size_t height = level;
	//top
	if( t_degree )
	{
		size_t step = 1<<t_degree;

		size_t r = width - 1; //密集的行号
		for( size_t c = 0; c < width; c+=step )
		{
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r+1, c );
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r+1, c+step );
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r, c+step/2 );
			

			for( size_t i=0; i < step/2; ++i )
			{
				if( l_degree > 0 && (c+i) == 0 )
					continue;

				pIndexBuf[counts++ ] = Mesh_2D_RC( r+1, c );
				pIndexBuf[counts++ ] = Mesh_2D_RC( r , c+i );
				pIndexBuf[counts++ ] = Mesh_2D_RC( r , c+i+1 );
				
				
			}

			for( size_t i = step; i > step/2; --i  )
			{
				if( r_degree>0 && (c+i) == width )
					continue;

				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+1, c+step );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r , c+i-1 );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r , c+i );
				
			}
		}
	}

	//bottom
	if( b_degree )
	{
		size_t step = 1<<b_degree;

		size_t r = 1; //密集的行号
		for( size_t c = 0; c < width; c+=step )
		{
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r-1, c );
			
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r, c+step/2 );
			
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r-1, c+step );
			for( size_t i=0; i < step/2; ++i )
			{
				if( l_degree > 0 && (c+i) == 0 )
					continue;

				pIndexBuf[counts++ ] = Mesh_2D_RC( r-1, c );
				pIndexBuf[counts++ ] = Mesh_2D_RC( r , c+i+1 );
				pIndexBuf[counts++ ] = Mesh_2D_RC( r , c+i );
				
			}

			for( size_t i = step; i > step/2; --i  )
			{
				if( r_degree>0 && (c+i) == width )
					continue;

				pIndexBuf[ counts++ ] = Mesh_2D_RC( r-1, c+step );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r , c+i );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r , c+i-1 );
			}
		}
	}
	//left
	if( l_degree )
	{
		size_t step = 1<<l_degree;

		size_t c= 1; //密集的列号
		for( size_t r = 0; r < height ; r+=step )
		{
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r,        c-1 );
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r+step/2, c );
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r+step,   c-1 );

			for( size_t i=0; i < step/2; ++i )
			{
				if( b_degree>0 && (r+i) == 0 )
					continue;

				pIndexBuf[counts++ ] = Mesh_2D_RC( r,    c-1 );
				pIndexBuf[counts++ ] = Mesh_2D_RC( r+i+1,c );
				pIndexBuf[counts++ ] = Mesh_2D_RC( r+i , c );
				
			}


			for( size_t i = step; i > step/2; --i  )
			{
				if( t_degree>0 && (r+i) == height )
					continue;

				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+step, c-1 );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+i ,   c );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+i-1 , c );
			}
		}
		
	}
	//right
	if( r_degree )
	{
		size_t step = 1<<r_degree;

		size_t c= width-1; //密集的列号
		for( size_t r = 0; r < height ; r+=step )
		{
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r,        c+1 );
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r+step/2, c );
			pIndexBuf[ counts++ ] = Mesh_2D_RC( r+step,   c+1 );

			for( size_t i=0; i < step/2; ++i )
			{
				if( b_degree>0 && (r+i) == 0 )
					continue;

				pIndexBuf[counts++ ] = Mesh_2D_RC( r,    c+1 );
				pIndexBuf[counts++ ] = Mesh_2D_RC( r+i , c );
				pIndexBuf[counts++ ] = Mesh_2D_RC( r+i+1,c );
			}


			for( size_t i = step; i > step/2; --i  )
			{
				if( t_degree>0 && (r+i) == height )
					continue;

				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+step, c+1 );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+i ,   c );
				pIndexBuf[ counts++ ] = Mesh_2D_RC( r+i-1 , c );
			}
		}

	}

	return counts;
}

size_t LodMesh::CaculateIndexBufSize( size_t maxGridDimExp )
{
	assert( maxGridDimExp <= 10 );
	//计算规则见indexCounts.py
	DWORD IndexSizeArray[] = {0, 0, 4284, 18828, 69444, 254412, 956916, 3689820, 14464836, 57249324,227753748};
	return IndexSizeArray[maxGridDimExp];
}

void LodMesh::Release()
{
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pIndexBuffer );
	SAFE_DELETE_ARRAY( m_pPattern );
}

QuartTreeOcean::QuartTreeOcean()
{
	m_pPerCallCB = NULL;
	m_pPerFrameCB = NULL;
	m_pPerAppCB = NULL;

	m_pRasterState_WireFrame = NULL;
	m_pRasterState_Solid = NULL;
	m_pOceanShader = NULL;
	
	m_pSampleLinearWrap = NULL;
	m_pPerlinNoiseMap = NULL;

	m_pFresenlSRV = NULL;

	m_pOpenWave1= NULL;
};

QuartTreeOcean::~QuartTreeOcean()
{

}

HRESULT QuartTreeOcean::InitResource(ID3D11Device* pd3dDevice,const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, size_t totalSideLengthExp,  size_t maxGridDimExp ,D3DXVECTOR2 centerPos /* = D3DXVECTOR2 */)
{
	assert( pd3dDevice );

	HRESULT hr = S_OK;
	float totalSideLength = (float)(1<<totalSideLengthExp);
	QTpart.InitResource( maxGridDimExp, totalSideLength , centerPos);
	hr = LMpart.InitResource( pd3dDevice, maxGridDimExp );


	m_pOpenWave1 = new WaveParticle();
	assert( m_pOpenWave1 );
	hr = m_pOpenWave1->InitResource_1(pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 256*4, 256*4);

	
	m_pOceanShader = new HLSLclass();
	assert( m_pOceanShader );
	D3D11_INPUT_ELEMENT_DESC mesh_layout_desc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	int counts = sizeof(mesh_layout_desc)/sizeof(D3D11_INPUT_ELEMENT_DESC);

	hr = m_pOceanShader->CompileVSInputLayout( pd3dDevice, L"shader/Ocean.hlsl", "OceanVS", mesh_layout_desc, counts );
	V_RETURN(hr);
	hr = m_pOceanShader->CompilePS_Solid( pd3dDevice, L"shader/Ocean.hlsl", "OceanPS_Solid");
	V_RETURN(hr);
	hr = m_pOceanShader->CompilePS_Wireframe( pd3dDevice, L"shader/Ocean.hlsl", "OceanPS_WireFrame");
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
	cb_desc.ByteWidth = PAD16( sizeof(Const_Per_Frame) );
	hr = pd3dDevice->CreateBuffer(&cb_desc, NULL, &m_pPerFrameCB);
	assert(m_pPerFrameCB);
	cb_desc.ByteWidth = PAD16( sizeof(Const_Per_App) );
	hr = pd3dDevice->CreateBuffer(&cb_desc, NULL, &m_pPerAppCB);
	assert(m_pPerAppCB);

	//rander state
	D3D11_RASTERIZER_DESC ras_desc;
	ras_desc.FillMode = D3D11_FILL_WIREFRAME; 
	ras_desc.CullMode = D3D11_CULL_NONE; 
	ras_desc.FrontCounterClockwise = FALSE; 
	ras_desc.DepthBias = 0;
	ras_desc.SlopeScaledDepthBias = 0.0f;
	ras_desc.DepthBiasClamp = 0.0f;
	ras_desc.DepthClipEnable= TRUE;
	ras_desc.ScissorEnable = FALSE;
	ras_desc.MultisampleEnable = TRUE;
	ras_desc.AntialiasedLineEnable = FALSE;

	hr = pd3dDevice->CreateRasterizerState(&ras_desc, &m_pRasterState_WireFrame);
	assert(m_pRasterState_WireFrame);

	ras_desc.FillMode = D3D11_FILL_SOLID; 
	hr = pd3dDevice->CreateRasterizerState(&ras_desc, &m_pRasterState_Solid);
	assert(m_pRasterState_Solid);

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

	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = pd3dDevice->CreateSamplerState( &sampDesc, &m_pSampleLinearWrap );
	assert( m_pSampleLinearWrap );



	//texture
	hr = CreateFresenlMap(pd3dDevice);
	V_RETURN(hr);


	m_pPerlinNoiseMap = new MyTexture();
	assert( m_pPerlinNoiseMap );
	hr = m_pPerlinNoiseMap->InitResource(pd3dDevice, "res/perlin_noise.hg" );
	V_RETURN(hr);


	m_perlinScale =  D3DXVECTOR3( 150.0f, 8.0f/1.5, 150.0f );
	m_matPerlinTex  = D3DXMATRIX(1.0f/m_perlinScale.x, 0, 0, 0,
		0,1,0,0,
		0,0,-1.0f/m_perlinScale.z, 0,	
		0,0,0,1);

	ID3D11DeviceContext* pd3dContext = DXUTGetD3D11DeviceContext();
	SetShaderConstant(pd3dContext);


	return hr;	
}


void SunSky(float angle, D3DXVECTOR4& sunDir, D3DXVECTOR4& sunColor,D3DXVECTOR4& skyColor )
{

	float a = 1.f;
	float b = 1.f;
	float SunAngle = (1.f-angle/180.f)*D3DX_PI;
	sunDir = D3DXVECTOR4( -a*cosf( SunAngle ), -b*sinf( SunAngle), 0, 0 );
	sunColor = D3DXVECTOR4(1.0, 1.0, 0.6, 1.0);
	skyColor = D3DXVECTOR4(0.38,0.44,0.56,1.0);
}

void QuartTreeOcean::SetShaderConstant( ID3D11DeviceContext* pd3dContext )
{
	assert( pd3dContext );
	Const_Per_App App_consts;
	D3DXMatrixTranspose(&App_consts.matPerlinTex, &m_matPerlinTex);

	App_consts.vec4PerlinScale = D3DXVECTOR4( m_perlinScale.x/m_pPerlinNoiseMap->m_width, m_perlinScale.y, m_perlinScale.z/m_pPerlinNoiseMap->m_height, 1 );
	

	App_consts.vec4SunDir = D3DXVECTOR4( 0.936016f, -0.343206f, 0.0780013f,1.0);
	
	App_consts.vec4SkyColor = D3DXVECTOR4(0.38,0.45,0.56,1.0);

	App_consts.vec4SunColor = D3DXVECTOR4(0.92, 0.6, 0.02, 1.0);
	
	App_consts.oceanDeep = 50;
	App_consts.vec4WaterColor = D3DXVECTOR4(powf(0.333, App_consts.oceanDeep/7),powf(0.333, App_consts.oceanDeep/30),powf(0.333, App_consts.oceanDeep/70),1.0);

	App_consts.vec4PerlinOctave = D3DXVECTOR4(1.12f, 0.59f, 0.23f,1);
	App_consts.vec4PerlinOctaveAmp = D3DXVECTOR4( 0.2f,0.3f,0.5f,1)/2;

	
	D3D11_MAPPED_SUBRESOURCE mapped_res;            
	pd3dContext->Map(m_pPerAppCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	assert(mapped_res.pData);
	*(Const_Per_App*)mapped_res.pData = App_consts;
	pd3dContext->Unmap(m_pPerAppCB, 0);
}

HRESULT QuartTreeOcean::CreateFresenlMap( ID3D11Device* pd3dDevice )
{
	HRESULT hr = S_OK;
	//fresenl map
	const int fresenl_map_size = 256;
	float* pFrenselArray = new float[ fresenl_map_size ];
	for( int i=0; i<fresenl_map_size; ++i )
	{
		float cos_a = ((float)i)/(fresenl_map_size);
		//1.33是水的反射参数
		pFrenselArray[i] = (D3DXFresnelTerm( cos_a, 1.33 ));
	}

	D3D11_TEXTURE1D_DESC desc;
	ZeroMemory(&desc, sizeof(desc) );
	desc.Width = fresenl_map_size;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.MiscFlags = 0;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = pFrenselArray;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	ID3D11Texture1D* pFresenlTexture = NULL;
	pd3dDevice->CreateTexture1D( &desc, &init_data, &pFresenlTexture );
	assert( pFresenlTexture );

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srv_desc.Texture1D.MipLevels = 1;
	srv_desc.Texture1D.MostDetailedMip = 0;
	hr = pd3dDevice->CreateShaderResourceView( pFresenlTexture,&srv_desc, &m_pFresenlSRV );
	assert( m_pFresenlSRV );

	SAFE_RELEASE( pFresenlTexture );
	SAFE_DELETE_ARRAY( pFrenselArray );

	return S_OK;
}

void QuartTreeOcean::Release()
{
	SAFE_RELEASE( m_pPerCallCB );
	SAFE_RELEASE( m_pPerFrameCB );
	SAFE_RELEASE( m_pPerAppCB );

	SAFE_RELEASE( m_pRasterState_WireFrame );
	SAFE_RELEASE( m_pRasterState_Solid );

	SAFE_RELEASE( m_pSampleLinearBorder );
	SAFE_RELEASE( m_pSampleLinearWrap );

	SAFE_RELEASE( m_pFresenlSRV );
	

	SafeReleaseDelete( m_pPerlinNoiseMap );
	SafeReleaseDelete( m_pOceanShader );

	QTpart.Release();
	LMpart.Release();


	SafeReleaseDelete( m_pOpenWave1 );

}



void QuartTreeOcean::RenderPatchs(  ID3D11DeviceContext* pd3dContext, float time,  const CBaseCamera&  camera )
{
	QTpart.BuildRenderList( camera );
	// VS & PS
	pd3dContext->VSSetShader(m_pOceanShader->m_pVS, NULL, 0);
	pd3dContext->GSSetShader( NULL, NULL, 0 );
	
	//Const Buffer
	D3DXMATRIX matView = *camera.GetViewMatrix();
	D3DXMATRIX matProj = *camera.GetProjMatrix();
	D3DXMATRIX matVP =  matView * matProj;

	m_PerlinTexMovement = 0.6*D3DXVECTOR2(1.0,0.0)*time*m_matPerlinTex._11;
	Const_Per_Frame frame_consts;
	D3DXMatrixTranspose(&frame_consts.matViewProj, &matVP);
	frame_consts.vec3EyePos = *camera.GetEyePt();
	frame_consts.time  = time;
	frame_consts.vec2PerlinMovement = m_PerlinTexMovement;
	D3DXMATRIX matOpenWaveTex1 =  m_pOpenWave1->GetMatrix();
	D3DXMatrixTranspose(&frame_consts.matOpenWaveTex1,&matOpenWaveTex1);
	frame_consts.vec4OpenWaveScale1 = m_pOpenWave1->GetDimScale();


	D3D11_MAPPED_SUBRESOURCE mapped_res;            
	pd3dContext->Map(m_pPerFrameCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	assert(mapped_res.pData);
	*(Const_Per_Frame*)mapped_res.pData = frame_consts;
	pd3dContext->Unmap(m_pPerFrameCB, 0);


	ID3D11Buffer* cbs[2] = { m_pPerAppCB, m_pPerFrameCB};
	pd3dContext->VSSetConstantBuffers(0, 2, cbs);
	pd3dContext->PSSetConstantBuffers(0, 2, cbs);

	//IA 
	pd3dContext->IASetIndexBuffer( LMpart.m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	ID3D11Buffer* vbs[1] = { LMpart.m_pVertexBuffer  };
	UINT strides[1] = { sizeof( LodMesh::MeshVertex) };
	UINT offsets[1] = {0};
	pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
	pd3dContext->IASetInputLayout(m_pOceanShader->m_pLayout );

	//texture and sampler
	ID3D11ShaderResourceView*  vs_srvs[] = { m_pPerlinNoiseMap->GetSRV(), m_pOpenWave1->GetHeightMapSRV()};
	int vs_srvs_counts = sizeof( vs_srvs )/sizeof( ID3D11ShaderResourceView* );
	pd3dContext->VSSetShaderResources( 0, vs_srvs_counts, vs_srvs );
	ID3D11SamplerState*      vs_samplers[] = { m_pSampleLinearBorder, m_pSampleLinearWrap };
	pd3dContext->VSSetSamplers( 0, 2 , vs_samplers );


	for( std::vector<QuartTree::qrt_node>::iterator iter = QTpart.m_VisibleAreaList.begin(); 
		iter != QTpart.m_VisibleAreaList.end(); ++iter)
	{
		QuartTree::qrt_node tempNode = *iter;
		if( tempNode.IsLeafNode() == false )
			continue;

		float scale = tempNode.side_length/powf( 2.0f, (float)QTpart.m_MaxGridDimExp - tempNode.lod );
		D3DXMATRIX matModelTrans,matModelScale;
		D3DXVECTOR3 vecModelTrans = D3DXVECTOR3( tempNode.left_bottom_pos.x, 0, tempNode.left_bottom_pos.y );
		D3DXVECTOR3 vecModelScale = D3DXVECTOR3( scale, 1.0f, scale );
		D3DXMatrixTranslation( &matModelTrans,vecModelTrans.x, vecModelTrans.y, vecModelTrans.z );
		D3DXMatrixScaling( &matModelScale, vecModelScale.x, vecModelScale.y, vecModelScale.z );
		D3DXMATRIX matWorld = matModelScale*matModelTrans;

		//constant
		Const_Per_Call call_consts;
		D3DXMatrixTranspose(&call_consts.matWorld, &matWorld);

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

		QuartTree::PatternType pt = QTpart.GetPatternType( tempNode );
		LodMesh::PatchRenderParam patch_render_param = LMpart.m_pPattern[pt.lod][ pt.t_degree ][ pt.b_degree][ pt.l_degree][ pt.r_degree ];

		if( patch_render_param.innerMesh_indexCounts > 0)
		{
			pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
			pd3dContext->DrawIndexed( patch_render_param.innerMesh_indexCounts, patch_render_param.innerMesh_indexStartLocation, 0 );
		}

		if( patch_render_param.boundaryMesh_indexCounts > 0)
		{
			pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			pd3dContext->DrawIndexed( patch_render_param.boundaryMesh_indexCounts, patch_render_param.boundaryMesh_indexStartLoaction, 0 );
		}
	}

	//unbound
	vbs[0] = NULL;
	pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);

	ZeroMemory( vs_srvs, sizeof(ID3D11ShaderResourceView*)*vs_srvs_counts );
	pd3dContext->VSSetShaderResources( 0, vs_srvs_counts, vs_srvs );
}

void QuartTreeOcean::RenderWireframe(  ID3D11DeviceContext* pd3dContext, float time,  const CBaseCamera&  camera )
{
		pd3dContext->RSSetState( m_pRasterState_WireFrame );
		pd3dContext->PSSetShader(m_pOceanShader->m_pPS_Wireframe, NULL, 0);

		RenderPatchs(pd3dContext, time,	camera);
		pd3dContext->RSSetState( m_pRasterState_Solid );

}

void   QuartTreeOcean::RenderShaded( ID3D11DeviceContext* pd3dContext, float time,  const CBaseCamera&  camera )
{
	pd3dContext->RSSetState( m_pRasterState_Solid );
	pd3dContext->PSSetShader(m_pOceanShader->m_pPS_Solid, NULL, 0);

	ID3D11ShaderResourceView*  ps_srvs[] = { m_pPerlinNoiseMap->GetSRV(),m_pOpenWave1->GetHeightMapSRV(), m_pOpenWave1->GetGradientMapSRV(), m_pFresenlSRV };
	int ps_srvs_counts = sizeof( ps_srvs )/sizeof(ID3D11ShaderResourceView*);
	pd3dContext->PSSetShaderResources( 0,ps_srvs_counts, ps_srvs );
	ID3D11SamplerState*      ps_samplers[] = { m_pSampleLinearBorder, m_pSampleLinearWrap };
	pd3dContext->PSSetSamplers( 0, 2 , ps_samplers );

	RenderPatchs(pd3dContext, time,	camera);

	ZeroMemory( ps_srvs, sizeof( ps_srvs) );
	pd3dContext->PSSetShaderResources( 0, ps_srvs_counts, ps_srvs );


}


HRESULT QuartTreeOcean::ResizedSwapChain(ID3D11Device* pd3dDevice,  int backbuf_width, int backbuf_height )
{
	HRESULT hr = S_OK;
	hr = m_pOpenWave1->ResizedSwapChain(pd3dDevice,backbuf_width,backbuf_height);
	V_RETURN(hr);
	return hr;
}


void    QuartTreeOcean::Update( float time, float deatTime )
{
	m_pOpenWave1->Update(time, deatTime);

	num = (m_pOpenWave1->GetParticleCounts());
}