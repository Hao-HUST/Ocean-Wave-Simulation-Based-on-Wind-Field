//--------------------------------------------------------------------------------------
// File: DiaoyuIsland.cpp
//
// Empty starting point for new Direct3D 9 and/or Direct3D 11 applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "mystd.h"
#include "QuartTreeOcean.h"
#include "OpenSeaRender.h"
#include "DXUT_2DElements.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

CFirstPersonCamera          g_Camera;  
DXUT_2DElements             g_2DUI;
QuartTreeOcean*		        g_pOcean = NULL ;

OpenSeaRender*              g_pOpenSea = NULL; 


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
	HRESULT hr = S_OK;
	hr = g_2DUI.OnD3D11CreateDevice(pd3dDevice);
	V_RETURN(hr);

	D3DXVECTOR3 eyePos = D3DXVECTOR3( 0, 70.0f, -20.0f );
	D3DXVECTOR3 eyeLookAt = D3DXVECTOR3( 0, 50, 0 );
	g_Camera.SetViewParams(&eyePos, &eyeLookAt);
	g_Camera.SetRotateButtons( true, false, false );
	g_Camera.SetScalers(0.005f, 80.0f);

	g_pOcean = new QuartTreeOcean();
	hr = g_pOcean->InitResource(pd3dDevice,pBackBufferSurfaceDesc,14, 7 );
	V_RETURN(hr);

	g_pOpenSea = new OpenSeaRender( 8 );
	assert( g_pOpenSea );
	hr = g_pOpenSea->InitResource( pd3dDevice );
    V_RETURN(hr);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams(D3DX_PI/4, fAspectRatio, 1.0f, 20000.0f);

	hr = g_2DUI.OnD3D11ResizedSwapChain(pd3dDevice,pBackBufferSurfaceDesc);
	V_RETURN(hr);

	hr = g_pOcean->ResizedSwapChain(pd3dDevice,pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    return hr;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_Camera.FrameMove( fElapsedTime );
	
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    // Clear render target and the depth stencil 
    float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };

    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	g_pOcean->Update((float)fTime, (float)fElapsedTime);

	if( g_2DUI.GetMode() == 0  )
	{
		g_pOcean->RenderShaded(pd3dImmediateContext,(float)fTime, g_Camera );
	}
	else if(g_2DUI.GetMode() == 1  )
	{
		g_pOcean->RenderWireframe( pd3dImmediateContext, (float)fTime, g_Camera );

	}
	g_2DUI.Render(fElapsedTime,g_pOcean->GetNum());
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	g_2DUI.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	SafeReleaseDelete( g_pOpenSea );
	SafeReleaseDelete( g_pOcean );
	g_2DUI.OnD3D11DestroyDevice();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
	g_2DUI.MsgProc(hWnd, uMsg, wParam, lParam );
    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{   //_CrtSetBreakAlloc(192);
	_CrtDumpMemoryLeaks();
	_CrtSetBreakAlloc(192);
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	//_CrtSetBreakAlloc( 357 );
    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackMouse( OnMouse );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );



    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
	DXUTSetIsInGammaCorrectMode( false );

    // Perform any application-level initialization here

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"DiaoyuIsland" );

    // Only require 10-level hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 720 );
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}


