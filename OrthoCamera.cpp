#include "DXUT.h"
#include "OrthoCamera.h"
#include <D3DX10.h>


OrthoCamera::OrthoCamera()
{
	SetViewParam( 0, 10, 0 );
	SetOrthoProjectionParam( 10, 10);
};

OrthoCamera::~OrthoCamera()
{

};