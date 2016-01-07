//ÎÄ×ÖºÍ¿Ø¼þ

#pragma once
#include "DXUT.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "DXUTcamera.h"

enum Control_ID
{
	IDC_FULLSCREEN = 1000,       
	IDC_WIREFRAME,               
};

class DXUT_2DElements  
{
public:
	DXUT_2DElements(void);
	~DXUT_2DElements(void);

	HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice );
	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void    OnD3D11DestroyDevice();
	void    OnD3D11ReleasingSwapChain();
	bool    MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void    Render( float fElapsedTime ,size_t num );
	

public:
	static int                  m_Mode ;                
	static inline  int          GetMode(){return m_Mode;} 

private:
	CDXUTDialogResourceManager  m_DialogResourceManager;    // manager for shared resources of dialogs
	CDXUTTextHelper*            m_pTxtHelper ;              // 2D text
	CDXUTDialog                 m_Controls;                  // dialog for button controls

	void     RenderText( size_t num );

};



void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );