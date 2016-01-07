#include "DXUT.h"
#include "DXUT_2DElements.h"


int DXUT_2DElements::m_Mode = 0;

DXUT_2DElements::DXUT_2DElements(void)
{
	m_pTxtHelper = NULL;
}


DXUT_2DElements::~DXUT_2DElements(void)
{

}

HRESULT DXUT_2DElements::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	HRESULT hr = S_OK;
	ID3D11DeviceContext*  pd3dImemediateContext = DXUTGetD3D11DeviceContext();
	hr = m_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice,pd3dImemediateContext );
	V_RETURN(hr);

	m_pTxtHelper = new CDXUTTextHelper(pd3dDevice,pd3dImemediateContext,&m_DialogResourceManager,15);
	assert(m_pTxtHelper);

	m_Controls.Init( &m_DialogResourceManager );
	m_Controls.SetCallback( OnGUIEvent );
	int iY = 20;
	m_Controls.AddButton( IDC_FULLSCREEN, L"Window / Fullscreen", 32, iY, 200, 30 );
	m_Controls.AddButton( IDC_WIREFRAME, L"Solid / Wireframe", 32, iY += 40, 200, 30 );
	return hr;
}

HRESULT DXUT_2DElements::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	hr = m_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc);

	m_Controls.SetLocation( pBackBufferSurfaceDesc->Width - 220, 0 );
	m_Controls.SetSize( 220, 220 );
	return hr;
}

void DXUT_2DElements::OnD3D11DestroyDevice()
{
	m_DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	SAFE_DELETE( m_pTxtHelper );
}

void DXUT_2DElements::OnD3D11ReleasingSwapChain()
{
	m_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

void DXUT_2DElements::RenderText( size_t num)
{
	m_pTxtHelper->Begin();

	m_pTxtHelper->SetInsertionPos( 2, 0 );
	m_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );

	//FPS
	WCHAR buffer_1[256],buffer_2[256];
	swprintf_s( buffer_1, 256, L"FPS:%d", (int)DXUTGetFPS());
	m_pTxtHelper->DrawTextLine( buffer_1);

	swprintf_s( buffer_2, 256, L"The current amount of particle to be rendered:%d", (int)num);
	m_pTxtHelper->DrawTextLine( buffer_2);
	m_pTxtHelper->End();
}

void DXUT_2DElements::Render( float fElapsedTime, size_t num )
{
	m_Controls.OnRender(fElapsedTime);
	RenderText(num);
}

bool DXUT_2DElements::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return m_Controls.MsgProc(hWnd, uMsg,wParam,lParam);
}

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
	{
	case IDC_FULLSCREEN:
		DXUTToggleFullScreen();
		break;
	case IDC_WIREFRAME:
		DXUT_2DElements::m_Mode = (++DXUT_2DElements::m_Mode)%2;
		break;
	default:
		break;
	}
}