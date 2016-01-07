#pragma once
#include <D3DX11.h>


//正交投影摄像机，专用于从高空向下垂直正投影
class OrthoCamera
{
public:
	OrthoCamera();
	~OrthoCamera();

private:
	//视点的位置
	float m_eyePosX;
	float m_eyePosY;
	float m_eyePosZ;

	//投影窗口的大小
	float m_viewVolumeWidth;
	float m_viewVolumeHeight;

	//取景的深度范围
	float m_near;
	float m_far;


private:
	D3DXMATRIX m_mView;              // View matrix 
	D3DXMATRIX m_mProj;              // Projection matrix

private:
	inline void BuildViewMatrix()
	{
		D3DXVECTOR3 vecEye( m_eyePosX, m_eyePosY, m_eyePosZ );
		D3DXVECTOR3 vecAt( m_eyePosX, m_eyePosY - 10, m_eyePosZ );
		D3DXVECTOR3 vecUp( 0,0, 1 );
		D3DXMatrixLookAtLH( &m_mView, &vecEye, &vecAt, &vecUp );
	}

	inline void BuidOrhoProjectionMatrix()
	{
		D3DXMatrixOrthoLH( &m_mProj, m_viewVolumeWidth, m_viewVolumeHeight, m_near, m_far );
	}

public:
	inline void SetViewParam( float x, float y, float z )
	{
		m_eyePosX = x;
		m_eyePosY = y;
		m_eyePosZ = z;

		BuildViewMatrix();
	}

	inline void SetOrthoProjectionParam( float w = 10.0f, float h = 10.0f,  float n = 1.0f, float f = 1000.0f)
	{

		m_viewVolumeWidth = w;
		m_viewVolumeHeight = h;

		m_near = n;
		m_far = f;

		BuidOrhoProjectionMatrix();
	}

	inline D3DXVECTOR3 GetEyePos()
	{
		return D3DXVECTOR3( m_eyePosX, m_eyePosY, m_eyePosZ );
	}

	inline D3DXVECTOR4 GetOrthoProjectionParam()
	{
		return D3DXVECTOR4( m_viewVolumeWidth, m_viewVolumeHeight, m_near, m_far );
	}

	inline D3DXMATRIX& GetViewMatrix() { return m_mView ;}

	inline D3DXMATRIX& GetOrthoProjectionMatrix() {return m_mProj ;}
 };