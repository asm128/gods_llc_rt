//#define GPROFILE_ENABLED
#include "GD3D9.h"
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"

#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

int32_t _CGD3D9Device::internalInit( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "internalInit" );
	uint32_t nWidth = GetSystemMetrics( SM_CXSCREEN ), 
		nHeight = GetSystemMetrics( SM_CYSCREEN );

	m_hMainWindow = CreateWindowW( L"GD3D9", L"Game Oriented Development Software Kit", 
							WS_POPUP, 0, 0, nWidth, nHeight,
							0, 0, m_pContext->getHInstance(), 0 );
	if( 0 == m_hMainWindow )
	{
		MessageBox(0, L"CreateWindow FAILED", 0, 0);	
		return 0;
	}

	HRESULT hr;
	IDirect3D9* pd3d9	= m_pContext->getd3d9();
	CGD3D9Enumerator* pEnum = m_pContext->getEnumerator();
	D3DDISPLAYMODE* pDesktopMode = pEnum->getDesktopMode();

	gpod_ptr<SD3D9Device> DeviceData;
	DeviceData.create();

	m_bRendering	= false;
	m_bLostDevice	= true;
	m_ClearColor	= 0xFF4295A1;
	m_fClearDepth	= 1.0f;
	m_nClearStencil	= 0;
	DeviceData->hWindowHandle	= m_hMainWindow;
	
	hr = pd3d9->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pDesktopMode->Format, pDesktopMode->Format, true );
	if( hr < 0 )
	{
		MessageBoxW(NULL, L"Can't check device D3DDEVTYPE_HAL for D3DADAPTER_DEFAULT", L"RenderDevice->Init()", MB_OK | MB_ICONWARNING );
		DeviceData->DeviceType	= D3DDEVTYPE_SW;
	}
	else 
	{
		DeviceData->DeviceType = D3DDEVTYPE_HAL;
	}

	DeviceData->nAdapter = D3DADAPTER_DEFAULT;
	if( pd3d9->GetDeviceCaps( D3DADAPTER_DEFAULT, DeviceData->DeviceType, &DeviceData->Caps ) )
	{	
		return G_FAIL;
	};

	DWORD dwBehaviorFlags = 0;
	if( DeviceData->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
		dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	// If pure device and HW T&L supported
	if( (DeviceData->Caps.DevCaps & D3DDEVCAPS_PUREDEVICE) 
		&& (dwBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) )
	{
		dwBehaviorFlags |= D3DCREATE_PUREDEVICE;
	}
	DeviceData->dwBehaviorFlags  = dwBehaviorFlags;
	//
	D3DPRESENT_PARAMETERS* pp1	= &DeviceData->PresentParameters,
		* ppWindowed = &m_WindowedPP, * ppFullScreen = &m_FullScreenPP;
	memset( &DeviceData->PresentParameters, 0, sizeof( D3DPRESENT_PARAMETERS ) );
	ppWindowed->BackBufferCount			= ppFullScreen->BackBufferCount			= pp1->BackBufferCount			= 1;
	ppWindowed->SwapEffect				= ppFullScreen->SwapEffect				= pp1->SwapEffect				= D3DSWAPEFFECT_DISCARD;
	ppWindowed->EnableAutoDepthStencil	= ppFullScreen->EnableAutoDepthStencil	= pp1->EnableAutoDepthStencil	= TRUE;
	ppWindowed->AutoDepthStencilFormat	= ppFullScreen->AutoDepthStencilFormat	= pp1->AutoDepthStencilFormat	= D3DFMT_D24S8;
	ppWindowed->PresentationInterval	= ppFullScreen->PresentationInterval	= pp1->PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;
//	ppWindowed->PresentationInterval	= ppFullScreen->PresentationInterval	= pp1->PresentationInterval		= D3DPRESENT_INTERVAL_DEFAULT;
	ppWindowed->BackBufferWidth			= ppFullScreen->BackBufferWidth			= pp1->BackBufferWidth			= nWidth;
	ppWindowed->BackBufferHeight		= ppFullScreen->BackBufferHeight		= pp1->BackBufferHeight			= nHeight;
	ppWindowed->Flags					= ppFullScreen->Flags					= pp1->Flags					= 0;
	ppWindowed->MultiSampleQuality		= ppFullScreen->MultiSampleQuality		= pp1->MultiSampleQuality		= DEFAULT_QUALITY;
	ppWindowed->MultiSampleType			= ppFullScreen->MultiSampleType			= pp1->MultiSampleType			= D3DMULTISAMPLE_NONE;
	// Windowed settings
	ppWindowed->BackBufferFormat		= pp1->BackBufferFormat = D3DFMT_UNKNOWN;
	ppFullScreen->BackBufferFormat		= D3DFMT_X8R8G8B8;
	ppWindowed->Windowed				= pp1->Windowed	= TRUE;
	ppFullScreen->Windowed				= FALSE;
	ppWindowed->hDeviceWindow			= ppFullScreen->hDeviceWindow = pp1->hDeviceWindow	= m_hMainWindow;

	m_bFullScreen = false;

	if( __createDevice( DeviceData.get_address() ) )
	{
		return G_FAIL;
	};

	m_DeviceData	= DeviceData;

	if( CreateResourceManager( (GOSH(VideoContext)*)&m_pDefaultManager ) )
		return G_FAIL;

	if( __onCreateDevice( DeviceData->pDevice ) )
	{
		MessageBox( NULL, L"Failed to allocate system resources for the device", L"Device->createDevice()", MB_OK | MB_ICONERROR );
		return G_FAIL;
	}
	if( __onResetDevice() )
	{
		MessageBox( NULL, L"OnResetDevice failed", L"Device->createDevice()", MB_OK | MB_ICONERROR );
		return G_FAIL;
	}
	return 0;
}


int32_t _CGD3D9Device::__createDevice( SD3D9Device* DeviceData )
{
	GPROFILE_METHOD( "_CGD3D9Device", "__createDevice" );
	HRESULT hr;
	IDirect3D9* pd3d9 = m_pContext->getd3d9();
	hr = pd3d9->CreateDevice( DeviceData->nAdapter, DeviceData->DeviceType, DeviceData->hWindowHandle,
		DeviceData->dwBehaviorFlags, &DeviceData->PresentParameters, &DeviceData->pDevice );
	if( FAILED(hr) )
		return G_FAIL;
	

	return S_OK;
};	

IDirect3DDevice9* _CGD3D9Device::getd3d9Device( void )
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "getd3d9Device" );
	if( !m_DeviceData )
		return 0;
	return m_DeviceData->pDevice;
}


CGD3D9* _CGD3D9Device::getContext( void )
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "getContext" );
	return m_pContext;
}

uint32_t _CGD3D9Device::GetTargetIndex( void )
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetTargetIndex" );
	return m_nActiveWindow;
}

GRDS(SWindow) _CGD3D9Device::GetTargetWindowData( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetTargetWindowData" );
	if( m_pWindowData == 0 )
		return 0;
	GRDS(SWindow) Result = 0;
	if( m_nActiveWindow == 0 )
		Result = m_pWindowData;
	else
		Result = m_pWindowData->ppChildWindows[m_nActiveWindow-1];

	return gacquire( Result );
}

// ------------------------------------------------------------------------------------------ 
// Let manager handle state changes
int32_t _CGD3D9Device::Update( float fElapsedTime )
{
	GPROFILE_METHOD( "_CGD3D9Device", "Update" );
	if( !m_DeviceData || 0 == m_pWindowData )
		return G_FAIL;

	if( m_bLostDevice 
		&& ( m_pWindowData->hWindowHandle != GetFocus() ) )
		// && ( FALSE != m_pDeviceData->PresentParameters.Windowed ) )
		return S_OK;

	if( m_bRenderingPaused )
		return S_OK;

	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;
	if( 0 == pDevice )
		return G_FAIL;

	// Get the state of the graphics device.
	HRESULT hr = pDevice->TestCooperativeLevel();

	if( hr == D3DERR_DEVICELOST )
	{
		m_bLostDevice = true;
		return S_OK;
	}
	else if( hr == D3DERR_DRIVERINTERNALERROR )
	{
		MessageBox( 0, L"D3DResourceManager->Update(): Internal Driver Error... Exiting", L"D3DResourceManager", 0 );
		PostQuitMessage(0);
		return G_FAIL;
	}
	CGD3D9ResourceManager* pMan;
	for( uint32_t iManager=0; iManager< m_nManagerCount;iManager++) 
	{
		if( !(pMan = m_lstpManager[iManager]) )
			continue;
		pMan->FreeUnused();
	}
	if( hr == D3DERR_DEVICENOTRESET )
	{
		__onLostDevice();
		if( pDevice->Reset( &m_DeviceData->PresentParameters ) )
		{
			error_printf( "IDirect3DDevice9::Reset() FAILED!" );
			return G_FAIL;
		}
		if( __onResetDevice() )
		{
			error_printf( "__onResetDevice() FAILED!" );
			return G_FAIL;
		}
	}

	return S_OK;
};


// ------------------------------------------------------------------------------------------ 
// Process window messages. 
int32_t _CGD3D9Device::MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam )
{
	for( uint32_t i=0; i< m_nManagerCount; i++ )
	{
		m_lstpManager[i]->GetAyuditaDXUT()->MsgProc( hWnd, uMsg, wParam, lParam );
	}

	if( !m_DeviceData || !m_pWindowData )
		return 0;
	
	gpod_ptr<SWindow> Window;

	if( hWnd != m_pWindowData->hWindowHandle )
	{
		for( uint32_t i=0; i< 32; i++ )
			if( m_DeviceData->hChildWindows[i] == hWnd )
			{	
				Window = gacquire( m_pWindowData->ppChildWindows[i] );
				break;
			}
	}
	else 
		Window = gacquire( m_pWindowData );

	if( 0 == Window ) // it's not for us
		return 0;
	
	IDirect3DDevice9* pDevice=m_DeviceData->pDevice;
	D3DPRESENT_PARAMETERS* pp = &m_DeviceData->PresentParameters;
	HRESULT hr=0;

	pp->hDeviceWindow = Window->hWindowHandle;
	// Is the application being minimized or maximized?
	static bool minOrMaxed = false;
	RECT clientRect;
	switch( uMsg )
	{
	case WM_ACTIVATE:
		if( WA_INACTIVE != wParam )
			m_bRenderingPaused = false;
		else if( FullScreenEnabled() )
  			m_bRenderingPaused = true;
		return 0;
	case WM_SIZE:
		//if(  0 == pDevice )
		//{
		//	pp->hDeviceWindow = m_pWindowData->hWindowHandle;
		//	return 0;
		//}
		//pp->BackBufferWidth  = LOWORD(lParam);
		//pp->BackBufferHeight = HIWORD(lParam);
		//if( pp->BackBufferWidth  <= 0 )
		//	pp->BackBufferWidth = 1;
		//if( pp->BackBufferHeight <= 0 )
		//	pp->BackBufferHeight = 1;
		//if( false == FullScreenEnabled() )
		//{
		//	pWindow->nWidth		= pp->BackBufferWidth;
		//	pWindow->nHeight	= pp->BackBufferHeight;
		//}

		if( wParam == SIZE_MINIMIZED )
		{
			m_bRenderingPaused = true;
			m_bMinimized = true;
			m_bMaximized = false;
			minOrMaxed	= true;
		}
		else if( wParam == SIZE_MAXIMIZED )
		{
			m_bRenderingPaused	= false;
			m_bMaximized		= true;
			m_bMinimized		= false;
			minOrMaxed			= true;
			///------------------------------------
			GetClientRect( hWnd, &clientRect );
			WINDOWPLACEMENT Placement = {0};
			GetWindowPlacement( Window->hWindowHandle, &Placement );
			
			pp->BackBufferWidth  = clientRect.right;
			pp->BackBufferHeight = clientRect.bottom;
			if( pp->BackBufferWidth  <= 0 )
				pp->BackBufferWidth = 1;
			if( pp->BackBufferHeight <= 0 )
				pp->BackBufferHeight = 1;
			if( false == FullScreenEnabled() )
			{
				Window->nWidth		= pp->BackBufferWidth;
				Window->nHeight		= pp->BackBufferHeight;
				Window->nPositionX	= Placement.rcNormalPosition.left;
				Window->nPositionY	= Placement.rcNormalPosition.top;
			}
			__onLostDevice();
			if( 0 > ( hr = pDevice->Reset( pp ) ) )
			{
				error_printf("IDirect3DDevice9::Reset() returned 0x%X!", hr);
				pp->hDeviceWindow = m_pWindowData->hWindowHandle;
				return 1;
			}
			__onResetDevice();
		}
		// Restored is any resize that is not a minimize or maximize.
		// For example, restoring the window to its default size
		// after a minimize or maximize, or from dragging the resize
		// bars.
		else if( wParam == SIZE_RESTORED )
		{
			///------------------------------------
			GetClientRect( hWnd, &clientRect );
			WINDOWPLACEMENT Placement = {0};
			GetWindowPlacement( Window->hWindowHandle, &Placement );
			
			pp->BackBufferWidth  = clientRect.right;
			pp->BackBufferHeight = clientRect.bottom;
			if( pp->BackBufferWidth  <= 0 )
				pp->BackBufferWidth = 1;
			if( pp->BackBufferHeight <= 0 )
				pp->BackBufferHeight = 1;
			if( false == FullScreenEnabled() )
			{
				Window->nWidth		= pp->BackBufferWidth;
				Window->nHeight	= pp->BackBufferHeight;
				Window->nPositionX	= Placement.rcNormalPosition.left;
				Window->nPositionY	= Placement.rcNormalPosition.top;
			}
			///---------------
			// Are we restoring from a mimimized or maximized state, 
			// and are in windowed mode?  Do not execute this code if 
			// we are restoring to full screen mode.
			if( minOrMaxed  && (false == FullScreenEnabled()) )
			{
				m_bMaximized = false;
				m_bMinimized = false;
				__onLostDevice();
				if( 0 > ( hr = pDevice->Reset( pp ) ) )
				{
					error_printf("IDirect3DDevice9::Reset() returned 0x%X!", hr);
					pp->hDeviceWindow = m_pWindowData->hWindowHandle;
					return 1;
				}

				__onResetDevice();
				m_bRenderingPaused = false;
			}
			else
			{
				// No, which implies the user is resizing by dragging
				// the resize bars.  However, we do not reset the device
				// here because as the user continuously drags the resize
				// bars, a stream of WM_SIZE messages is sent to the window,
				// and it would be pointless (and slow) to reset for each
				// WM_SIZE message received from dragging the resize bars.
				// So instead, we reset after the user is done resizing the
				// window and releases the resize bars, which sends a
				// WM_EXITSIZEMOVE message.
			}
			minOrMaxed = false;
		}
		return 0;


	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		GetClientRect( hWnd, &clientRect );
		WINDOWPLACEMENT Placement;
		GetWindowPlacement( Window->hWindowHandle, &Placement );
		
		pp->BackBufferWidth  = clientRect.right;
		pp->BackBufferHeight = clientRect.bottom;
		if( pp->BackBufferWidth  <= 0 )
			pp->BackBufferWidth = 1;
		if( pp->BackBufferHeight <= 0 )
			pp->BackBufferHeight = 1;
		//if( false == FullScreenEnabled() )
		//{
		//	pWindow->nWidth		= pp->BackBufferWidth;
		//	pWindow->nHeight	= pp->BackBufferHeight;
		//	pWindow->nPositionX	= Placement.rcNormalPosition.left;
		//	pWindow->nPositionY	= Placement.rcNormalPosition.top;
		//}
		if( pDevice )
		{
			__onLostDevice();
			if( 0 > ( hr = pDevice->Reset( pp ) ) )
			{
				error_printf("IDirect3DDevice9::Reset() returned 0x%X!", hr);
				pp->hDeviceWindow = m_pWindowData->hWindowHandle;
				return 1;
			}

			__onResetDevice();
		}
		m_bRenderingPaused = false;
		return 0;
	case WM_DESTROY:
		m_bRenderingPaused = true;
		pp->hDeviceWindow = 0;
		return 0;
	}

	pp->hDeviceWindow = m_pWindowData->hWindowHandle;

	return 0;
}; 


// ------------------------------------------------------------------------------------------ 
// Set the color to clear the backbuffer on BeginScene(). 
void	_CGD3D9Device::SetClearColorValue( const float* rgbaColor )		
{ 
	m_ClearColor = rgbaColor;		
};
void	_CGD3D9Device::SetClearDepthValue( float fDepth )				
{
	m_fClearDepth = fDepth;		
};
void	_CGD3D9Device::SetClearStencilValue( uint32_t nStencil )		
{ 
	m_nClearStencil = nStencil;	
};


// ------------------------------------------------------------------------------------------ 
// Returns the backbuffer height.  
uint32_t _CGD3D9Device::GetWidth( uint32_t nWindow )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetWidth" );
	if( !m_DeviceData || 0 == m_pWindowData )
		return 0;

	if( nWindow && nWindow >= m_pWindowData->nChildWindows )
		return 0;

	IDirect3DDevice9* pDevice = getd3d9Device();

	if( 0 == pDevice )
	{
		if( nWindow == 0 )
			return m_DeviceData->PresentParameters.BackBufferWidth;
		else
			return gget(m_pWindowData->ppChildWindows[nWindow-1])->nWidth;
	}
	IDirect3DSurface9* pTargetSurface;
	D3DSURFACE_DESC SurfaceDesc;
	pDevice->GetRenderTarget( nWindow, &pTargetSurface );
	if( pTargetSurface )
	{
		pTargetSurface->GetDesc( &SurfaceDesc );
		pTargetSurface->Release();
	}
	return SurfaceDesc.Width;
};  

// ------------------------------------------------------------------------------------------ 
// Returns the backbuffer width.
uint32_t _CGD3D9Device::GetHeight( uint32_t nWindow )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetHeight" );
	if( !m_DeviceData || 0 == m_pWindowData )
		return 0;

	if( nWindow && nWindow >= m_pWindowData->nChildWindows )
		return 0;

	IDirect3DDevice9* pDevice = getd3d9Device();

	if( 0 == pDevice )
	{
		if( nWindow == 0 )
			return m_DeviceData->PresentParameters.BackBufferHeight;
		else
			return gget(m_pWindowData->ppChildWindows[nWindow-1])->nHeight;
	}
	IDirect3DSurface9* pTargetSurface;
	D3DSURFACE_DESC SurfaceDesc;
	pDevice->GetRenderTarget( nWindow, &pTargetSurface );
	if( pTargetSurface )
	{
		pTargetSurface->GetDesc( &SurfaceDesc );
		pTargetSurface->Release();
	}
	return SurfaceDesc.Height;
};

// ------------------------------------------------------------------------------------------ 
// Clears the backbuffer with the specified color.  
int32_t _CGD3D9Device::Clear( bool bTarget, bool bDepth, bool bStencil )
{
	GPROFILE_METHOD( "_CGD3D9Device", "Clear" );
	HRESULT hr=NULL;
	LONG flags = 0;

	if( !m_DeviceData || 0 == m_DeviceData->pDevice ) 
		return G_FAIL;

	if( m_bLostDevice )
		return 0;

	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;

	if( bTarget ) flags = D3DCLEAR_TARGET;
	if( bDepth ) flags |= D3DCLEAR_ZBUFFER;
	if( bStencil ) flags |= D3DCLEAR_STENCIL;

	if( m_bRendering ) 
	{
		if( m_pShader.valid() && m_pShader->bActive )
		{
			m_pShader->pd3dXProgram->EndPass();
			m_pShader->pd3dXProgram->End();
			m_pShader->bActive = false;
		}
		if( hr = pDevice->EndScene() )
		{
			m_bRendering = false;
			return G_FAIL;
		}
		//m_pDeviceData->bRendering	= false;
	}

	hr = pDevice->Clear( 0, 0, flags, m_ClearColor, m_fClearDepth, m_nClearStencil );
	if( hr )
	{
		if( m_bRendering )
			pDevice->BeginScene();
		return hr;
	}
	if( m_bRendering )
		hr = pDevice->BeginScene();
	if( hr )
	{
		error_printf("%s", "Error clearing backbuffer!");
	}

	return hr;
};

int32_t _CGD3D9Device::ClearStencil( void )
{
	return Clear( false, false, true );
};

int32_t _CGD3D9Device::ClearDepth( void )
{
	return Clear( false, true, false );
};

int32_t _CGD3D9Device::ClearTarget( void )
{
	return Clear( true, false, false );
};

// ------------------------------------------------------------------------------------------ 
// Prepares the device for rendering. Every Drawing call must be in between BeginScene and 
// EndScene calls.    
int32_t _CGD3D9Device::StartRender( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "StartRender" );
	if( !m_DeviceData || !m_DeviceData->pDevice ) 
		return G_FAIL;

	if( true == m_bRendering ) 
	{
		error_printf( "Call to StartRender() with the device already rendering!" );
		EndRender();
		m_bRendering = false;
		return G_FAIL;
		
	}

	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;

	if( pDevice->BeginScene() ) 
	{
		error_printf( "IDirect3DDevice9::BeginScene() FAILED!" );
		return G_FAIL;
	}
	m_bRendering = true;
	return 0L;
};

// ------------------------------------------------------------------------------------------ 
// Finalize rendering and present backbuffer.
int32_t _CGD3D9Device::EndRender( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "EndRender" );
	if( !m_DeviceData || !m_DeviceData->pDevice ) 
		return G_FAIL;

	if( false == m_bRendering ) 
	{
		error_printf( "EndRender() called without the device rendering!" );
		return G_FAIL;
	}

	if( m_pShader.valid() && m_pShader->bActive )
	{
		m_pShader->pd3dXProgram->EndPass();
		m_pShader->pd3dXProgram->End();
		m_pShader->bActive	= false;
	}
	
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;
	HRESULT hr;
	if( hr = pDevice->EndScene() )
	{
		error_printf( "IDirect3DDevice9::EndScene() returned 0x%X!", hr );
		return G_FAIL; // who knows :P
	}

	m_bRendering = false;
	return S_OK;
}; 

// 1 Backbuffer each client window
//
int32_t _CGD3D9Device::PresentTarget( uint32_t nTargetIndex, GRDS(SRectangle) src_pQuad, GRDS(SRectangle) dst_pQuad )
{
	GPROFILE_METHOD( "_CGD3D9Device", "PresentTarget" );
	if( m_bRenderingPaused )
		return 0;

	if( !m_DeviceData || 0 == m_DeviceData->pDevice || 0 == m_pWindowData )
	{
		error_printf( "No available device or window data." );
		return G_FAIL;
	}
	if( m_bLostDevice )
		return 0;

	if( m_bRendering )
	{
		error_printf( "PresentWindow() called without calling EndRender() first." );
		return G_FAIL;
	}

	if( nTargetIndex && ( nTargetIndex-1 >= m_pWindowData->nChildWindows ) )
	{
		error_printf( "PresentWindow() called the wrong index (%i). Max index is %i.", nTargetIndex, m_pWindowData->nChildWindows );
		return G_INVALIDINDEX;
	}

	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;
	GRDS(SWindow) pWindowData;
	bool bChild = false;
	uint32_t nChildIndex = nTargetIndex-1;
	if( nTargetIndex )
	{
		pWindowData = m_pWindowData->ppChildWindows[nChildIndex];
		bChild = true;
	}
	else
		pWindowData = m_pWindowData;

	RECT rcsrc = {0}, rcdst = {0}, *prcsrc=0, *prcdst=0;
	
	// we need to transform quad from normalized screen coordinates (-1, -1) (1, 1) 
	// to pixel coordinates (0, 0, pixelcols, pixelrows)
	uint32_t nWidth=gget(pWindowData)->nWidth, nHeight=gget(pWindowData)->nHeight;
	uint32_t nHalfWidth=nWidth/2, nHalfHeight=nHeight/2;
#pragma warning( disable : 4244 )
	if( src_pQuad )
	{
		rcsrc.left		= gget(src_pQuad)->vOffset.x * nWidth+nHalfWidth;
		rcsrc.top		= gget(src_pQuad)->vOffset.y * nHeight+nHalfHeight;
		rcsrc.right		= gget(src_pQuad)->vSize.x * nWidth+nHalfWidth;
		rcsrc.bottom	= gget(src_pQuad)->vSize.y * nHeight+nHalfHeight;
		prcsrc = &rcsrc;
	}

	if( dst_pQuad )
	{
		rcdst.left		= gget(dst_pQuad)->vOffset.x * nWidth+nHalfWidth;
		rcdst.top		= gget(dst_pQuad)->vOffset.y * nHeight+nHalfHeight;
		rcdst.right		= gget(dst_pQuad)->vSize.x * nWidth+nHalfWidth;
		rcdst.bottom	= gget(dst_pQuad)->vSize.y * nHeight+nHalfHeight;
		prcdst = &rcdst;
	}
#pragma warning( default : 4244 )

	if( bChild )
	{
		if( 0 == m_DeviceData->pSwapChain[nChildIndex] )
		{
			error_printf( "FAILED Swap chain at location 0x%p with index %i is NULL. No backbuffer to swap.", m_DeviceData->pSwapChain, nChildIndex );
			return G_FAIL;
		}
		if( m_DeviceData->pSwapChain[nChildIndex]->Present( prcsrc, prcdst, gget(pWindowData)->hWindowHandle, 0, 0 ) )
		{
			error_printf( "FAILED Swap chain at location 0x%p with index %i failed to present backbuffer. No backbuffer to swap.", 
				m_DeviceData->pSwapChain, nChildIndex );
			return G_FAIL;
		}
	}
	else
	{
		if( m_pCurrentRenderTarget )
		{
			pDevice->SetRenderTarget( 0, m_pCurrentRenderTarget );
			m_pCurrentRenderTarget->Release();
			m_pCurrentRenderTarget = 0;
		}
		if( pDevice->Present( prcsrc, prcdst, 0, 0 ) )
		{
			error_printf( "FAILED IDirect3DDevice9::Present()" );
			return G_FAIL;
		}
	}

	return G_OK;
}; // 1 Backbuffer each client window



// ------------------------------------------------------------------------------------------ 
// Switchs the display mode to fullscreen.
int32_t _CGD3D9Device::ToggleFullScreen( bool bFullScreen ) //, int width, int height )
{
	GPROFILE_METHOD( "_CGD3D9Device", "ToggleFullScreen" );
	if( !m_DeviceData || 0 == m_DeviceData->pDevice || 0 == m_pWindowData )
		return G_FAIL;
	
	D3DPRESENT_PARAMETERS* pp = &m_DeviceData->PresentParameters;
	if( bFullScreen && (false == m_bFullScreen) )
	{
		int width  = GetSystemMetrics( SM_CXSCREEN );
		int height = GetSystemMetrics( SM_CYSCREEN );
		info_printf( "Attempting to set fullscreen from [%i x %i] mode. New backbuffer size is [%i x %i].", 
			pp->BackBufferWidth, pp->BackBufferHeight, width, height );
		pp->BackBufferWidth		= width;
		pp->BackBufferHeight	= height;
		pp->hDeviceWindow		= m_pWindowData->hWindowHandle;
		pp->Windowed			= FALSE;
		m_bFullScreen = true;
		SetWindowLongPtr( m_pWindowData->hWindowHandle, GWL_STYLE, WS_POPUP );
		SetWindowPos( m_pWindowData->hWindowHandle, HWND_TOP, 0, 0, width, height, SWP_NOZORDER | SWP_SHOWWINDOW );	
		SetTargetByIndex( 0 );
	}
	else if( (false == bFullScreen) && m_bFullScreen )
	{

		int width  = m_pWindowData->nWidth;
		int height = m_pWindowData->nHeight;

		info_printf( "Attempting to restore from fullscreen from [%i x %i] mode. New backbuffer size is [%i x %i].", 
			pp->BackBufferWidth, pp->BackBufferHeight, width, height );
		pp->BackBufferWidth		= width;
		pp->BackBufferHeight	= height;
		pp->Windowed			= TRUE;
		m_bFullScreen = false;

		if( m_pWindowData->dwStyle & WS_THICKFRAME )
		{
			width += GetSystemMetrics( SM_CXFRAME ) *2;
			height += GetSystemMetrics( SM_CYFRAME ) *2;
		}
		else
		{
			width += GetSystemMetrics( SM_CXFIXEDFRAME ) *2;
			height += GetSystemMetrics( SM_CYFIXEDFRAME ) *2;
		}

		SetWindowLongPtr( pp->hDeviceWindow, 
			GWL_STYLE, m_pWindowData->dwStyle );//	 WS_OVERLAPPED | WS_BORDER | WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX);
			//WS_OVERLAPPED | WS_THICKFRAME | WS_BORDER | WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX );
		SetWindowPos( pp->hDeviceWindow, HWND_NOTOPMOST, 
			m_pWindowData->nPositionX, m_pWindowData->nPositionY, 
			width, height + GetSystemMetrics(SM_CYCAPTION), SWP_SHOWWINDOW );
	}
	else
		return 0;

	__onLostDevice();
	if( m_DeviceData->pDevice->Reset( pp ) )
	{
		error_printf( "IDirect3DDevice9::Reset() FAILED!" );
		return G_FAIL;
	}
	else
	{
		info_printf( "IDirect3DDevice9::Reset() succeeded!" );
	}
	if( __onResetDevice() )
	{
		error_printf( "__onResetDevice() FAILED!" );
		return G_FAIL;
	}
	else
	{
		info_printf( "__onResetDevice() succeeded!" );
	}
	return S_OK;
}; 

// ------------------------------------------------------------------------------------------ 
// Returns true if the device is working in windowed mode.
bool _CGD3D9Device::FullScreenEnabled( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "FullScreenEnabled" );
	return 	m_bFullScreen;
 //	if( 0 == m_pDeviceData  )
	//	return G_FAIL;
	//return TRUE == m_pDeviceData->PresentParameters.Windowed;
};

// ------------------------------------------------------------------------------------------ 
// Sets render target window.
int32_t _CGD3D9Device::SetTargetByIndex( uint32_t nTargetIndex )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetTargetByIndex" );
	if( !m_DeviceData || 0 == m_pWindowData )
	{
		error_printf("Device data not initialized.");
		return G_FAIL;
	}

	if( nTargetIndex > m_pWindowData->nChildWindows )
	{
		error_printf( "Invalid render target index! Index requested: %u, maximum allowed: %u", nTargetIndex,  m_pWindowData->nChildWindows );
		return G_INVALIDINDEX;
	}
	
	//if( false == GetWindowed() )
	//	return S_OK;

	IDirect3DSurface9*	pBack = NULL;

	IDirect3DDevice9*	pDevice = getd3d9Device();

	//if( m_pDeviceData->bRendering ) 
	//{
	//	if( m_pShader && m_pShader->bActive )
	//	{
	//		m_pShader->pd3dProgramData->pEffect->EndPass();
	//		m_pShader->pd3dProgramData->pEffect->End();
	//		m_pShader->bActive = false;
	//	}
	//	if( pDevice->EndScene() )
	//		return G_FAIL;
	//}

	HRESULT hr=S_OK;
	if( nTargetIndex == 0 )
	{
		if( ( hr = pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBack ) ) < 0 )
		{
			error_printf("(IDirect3DDevice9*)->GetBackBuffer() returned HRESULT=0x%X.", hr);
			return G_FAIL;
		}
		if( ( hr = pDevice->SetRenderTarget(0, pBack) ) < 0 )
		{
			error_printf("(IDirect3DDevice9*)->SetRenderTarget() returned HRESULT=0x%X.", hr);
			pBack->Release();
			return G_FAIL;
		}
		pBack->Release();
		m_nActiveWindow	= nTargetIndex;

		//if( m_pDeviceData->bRendering ) 
		//{
		//	pDevice->BeginScene();
		//}

		return S_OK;
	}


	if( 0 == m_DeviceData->pSwapChain[nTargetIndex-1] )
	{
		m_nActiveWindow	= nTargetIndex;
		//if( m_pDeviceData->bRendering ) 
		//{
		//	pDevice->BeginScene();
		//}
		return S_OK;
	}
	// try to get the appropriate back buffer
	if( FAILED(m_DeviceData->pSwapChain[nTargetIndex-1]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBack) ) )
	{
		error_printf("(IDirect3DSwapChain9*)->GetBackBuffer() returned HRESULT=0x%X.", hr);
		return G_FAIL;
	}

	// and activate it for the device
	if( FAILED( pDevice->SetRenderTarget(0, pBack) ) )
	{
		pBack->Release();
		return G_FAIL;
	}
	pBack->Release();
	//if( m_pDeviceData->bRendering ) 
	//{
	//	pDevice->BeginScene();
	//}
	m_nActiveWindow	= nTargetIndex;
	return G_OK;
};

int32_t _CGD3D9Device::SetTargetImage( GOSH(VideoImage) pImage, uint32_t nTargetIndex )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetTargetImage" );
	if( !m_DeviceData || 0 == m_pWindowData )
	{
		error_printf("CGD3D9Device::SetTargetImage() was called when the device was not ready! Did you forget to initialize it?");
		return G_FAIL;
	}
	IDirect3DDevice9* pDevice = getd3d9Device();
	if( 0 == pImage )
	{
		if( m_pCurrentRenderTarget )
		{
			pDevice->SetRenderTarget( nTargetIndex, m_pCurrentRenderTarget );
			m_pCurrentRenderTarget->Release();
			m_pCurrentRenderTarget = 0;
		}
		//error_printf("CGD3D9Device::SetTargetImage() received NULL as parameter for target index=%u! (It should receive a valid IGVideoImage pointer!)", nTargetIndex );
		//return -1;
		return 0;
	}
	if( nTargetIndex > m_pWindowData->nChildWindows )
	{
		error_printf("CGD3D9Device::SetTargetImage() received an invalid rendertarget index as parameter! Invalid index=%u, Max index=%u", nTargetIndex, m_pWindowData->nChildWindows );
		return G_INVALIDINDEX;
	}	

	CGD3D9Image* pD3D9Image = (CGD3D9Image*)pImage->GetInterfacePointer( GIID_CUSTOMINTERFACE );
	// IDirect3DTexture9* pd3dTex = pD3D9Image->

	IDirect3DTexture9* pd3dTex = (IDirect3DTexture9*)pD3D9Image->GetPrivateData();
	greleaseInterface( &pImage ); // release here thus
	pD3D9Image = 0; // invalidate this
	if( 0 == pd3dTex )
	{
		error_printf("CGD3D9Device::SetTargetImage() FAILED because IGVideoImage->GetPrivateData() returned NULL instead of a valid IDirect3DTexture9 pointer!");
		return -1;
	}
	IDirect3DSurface9* pBack=NULL;
	pd3dTex->GetSurfaceLevel( 0, &pBack );
	
	if( m_pCurrentRenderTarget )
		m_pCurrentRenderTarget->Release();
	m_pCurrentRenderTarget = 0;
	pDevice->GetRenderTarget( 0, &m_pCurrentRenderTarget );
	pDevice->SetTexture( 0, 0 );

	HRESULT hr=0;
	if( 0 > ( hr = pDevice->SetRenderTarget(nTargetIndex, pBack) ) )
	{
		error_printf( "IDirect3DDevice9::SetRenderTarget() returned HRESULT=0x%X!", hr );
		pd3dTex->Release();
		pBack->Release();
		return G_FAIL;
	}
	pd3dTex->Release();
	pBack->Release();

	return 0;
};

error_t _CGD3D9Device::SetTargetArea( uint32_t nPositionX, uint32_t nPositionY, uint32_t nWidth, uint32_t nHeight )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetTargetArea" );
	m_TargetArea.left	= nPositionX;
	m_TargetArea.top	= nPositionY;
	m_TargetArea.right	= m_TargetArea.left	+nWidth;
	m_TargetArea.bottom	= m_TargetArea.top	+nHeight;
	IDirect3DDevice9* pDevice = getd3d9Device();
	error_t result=0;
	if( 0 == pDevice )
	{
		warning_printf("IDirect3DDevice9* is NULL at CGD3D9Device::SetTargetArea() call!");
	}
	else
	{
		if( result = pDevice->SetScissorRect( &m_TargetArea ) )
		{
			error_printf("IDirect3DDevice9::SetScissorRect() returned 0x%X!", result);
			return -1;
		};
	}
	return 0;
};


error_t _CGD3D9Device::SetViewportArea( uint32_t nPositionX, uint32_t nPositionY, uint32_t nWidth, uint32_t nHeight )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetViewportArea" );
	m_ViewportArea.X		= nPositionX;
	m_ViewportArea.Y		= nPositionY;
	m_ViewportArea.Width	= nWidth;
	m_ViewportArea.Height	= nHeight;
	m_ViewportArea.MinZ		= 0.0f;
	m_ViewportArea.MaxZ		= 1.0f;
	IDirect3DDevice9* pDevice = getd3d9Device();
	error_t result=0;
	if( 0 == pDevice )
	{
		warning_printf("IDirect3DDevice9* is NULL at CGD3D9Device::SetTargetArea() call!");
	}
	else
	{
		if( result = pDevice->SetViewport( &m_ViewportArea ) )
		{
			error_printf("IDirect3DDevice9::SetScissorRect() returned 0x%X!", result);
			return -1;
		};
	}
	return 0;
};


// ------------------------------------------------------------------------------------------
// int32_t _CGD3D9Device::CreateResourceManager( int32_t ID, GOSH(VideoContext)* ppCreated );
error_t _CGD3D9Device::CreateResourceManager( GOSH(VideoContext)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9Device", "CreateResourceManager" );
	if( 0 == ppCreated )
	{
		error_printf("_CGD3D9Device::CreateResourceManager(): \"GOSH(VideoContext)* ppCreated\" cannot be NULL!");
		return G_INVALIDARGUMENT;
	}
	CGD3D9ResourceManager* newManager;

	if( 0 > gresizePointerArray( sizeof( CGD3D9ResourceManager* ), m_nManagerCount+1, &m_nManagerArraySize, (void**)&m_lstpManager, m_nManagerArraySize+m_nManagerArraySize/2 ) )
	{
		error_printf("Cannot resize resource manager array! Out of memory?");
		return -1;
	};
	newManager = new CGD3D9ResourceManager(this);
	if( 0 == newManager ) 
	{
		error_printf("Cannot create CGD3D9ResourceManager instance! Out of memory?");
		return G_NOMEM;
	}
	newManager->setID( m_nManagerCount );
	m_lstpManager[m_nManagerCount] = newManager;
	m_nManagerCount++;
	if( getd3d9Device() )
	{
		newManager->onCreateDevice( m_DeviceData->pDevice );
		if( false == m_bLostDevice )
			newManager->onResetDevice();
	}
	GHPTR(VideoContext) garbage = *ppCreated;
	*ppCreated = gacquireInterface( newManager );
	return G_OK;
};

// Probably I should make this function part of the IGDriver interface?
IGVideo*_CGD3D9Device::GetContext( void )
{
	return m_pContext;
};

//------------------- IGDriver  
int32_t _CGD3D9Device::GetID( void ) const
{
	if ( 0 == m_pHALDeviceData )  
		return 0;
	return m_pHALDeviceData->nDeviceID;
};

uint32_t _CGD3D9Device::GetVersion( void ) const
{
	if ( 0 == m_pHALDeviceData )  
		return 0;
	return m_pHALDeviceData->nVersion;
};

GDRIVERFAMILY_ID _CGD3D9Device::GetFamilyID( void ) const
{
	if ( 0 == m_pHALDeviceData )  
		return 0;
	return m_pHALDeviceData->nFamilyID;
};

GRDS(SDevice) _CGD3D9Device::GetDeviceData( void )
{
	return gacquire( m_pHALDeviceData );
};

//
int32_t _CGD3D9Device::__onCreateDevice( IDirect3DDevice9* pD3DDevice )
{
	GPROFILE_METHOD( "_CGD3D9Device", "__onCreateDevice" );
	uint32_t i;

	CGD3D9ResourceManager* pMan;
	for(i=0; i< m_nManagerCount;i++) 
	{
		pMan = m_lstpManager[i];
		if( !pMan )
			continue;
		info_printf( "Calling onCreateDevice on resource manager at index %i.", i );
		if( pMan->onCreateDevice( pD3DDevice ) )
		{
			error_printf( "onCreateDevice() FAILED on resource manager at index %i.", i );
			return G_FAIL;
		}
	};

	for( uint32_t iHandler=0; iHandler<m_nVideoHandlerCount; iHandler++ )
	{
		//m_lstVideoHandler[iHandler]->OnDriverCreate( pD3DDevice );
	}

	Beep( 392+232*2, 50 ); // I used this because breakpoints here messed up the screen
	return G_OK;
};

int32_t _CGD3D9Device::__onResetDevice( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "__onResetDevice" );
	if( 0 == m_pWindowData || !m_DeviceData )
		return G_FAIL;

	uint32_t i, n = m_pWindowData->nChildWindows;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;
	if( 0 == pDevice )
		return G_FAIL;

	// create swap chains if needed
	if( n > 0 && (false == FullScreenEnabled()) )
	{
		for (uint32_t i=0; i<n; i++)
		{
			m_DeviceData->PresentParameters.hDeviceWindow		= gget(m_pWindowData->ppChildWindows[i])->hWindowHandle;
			m_DeviceData->PresentParameters.BackBufferHeight	= gget(m_pWindowData->ppChildWindows[i])->nHeight;
			m_DeviceData->PresentParameters.BackBufferWidth		= gget(m_pWindowData->ppChildWindows[i])->nWidth;
			if( pDevice->CreateAdditionalSwapChain( &m_DeviceData->PresentParameters, 
				&m_DeviceData->pSwapChain[i] ) )
			{ 
				error_printf( "Failed to create swap chain at index %i.", i );
				return G_FAIL; 
			}
			else
			{
				info_printf( "Swap chain at index %i created successfully.", i );
			}
		}
		m_DeviceData->PresentParameters.hDeviceWindow		= m_pWindowData->hWindowHandle;
		m_DeviceData->PresentParameters.BackBufferHeight	= m_pWindowData->nHeight;
		m_DeviceData->PresentParameters.BackBufferWidth	= m_pWindowData->nWidth;
	}

	CGD3D9ResourceManager* pMan;
	for(i=0; i< m_nManagerCount;i++) 
	{
		pMan = m_lstpManager[i];
		if( !pMan )
			continue;
		info_printf("Calling onResetDevice on resource manager at index=%i.", i);
		if( pMan->onResetDevice() )
		{
			error_printf( "Manager failed at onResetDevice(). Manager index=%i at address=0x%p.", i, m_lstpManager[i] );
			//return G_FAIL;
		}
	}

	for( i=0; i<m_nVideoHandlerCount; i++ )
	{
		//m_lstVideoHandler[i]->OnDriverReset( pDevice );
	}

	for( i=0; i<m_nVideoHandlerCount; i++ )
	{
		m_lstVideoHandler[i]->OnTargetResize( m_DeviceData->PresentParameters.BackBufferWidth, m_DeviceData->PresentParameters.BackBufferHeight, 0 );
	}

	m_bLostDevice = false;
	m_bRenderingPaused	= false;

	Beep( 494+232*2, 50 ); // I used this because breakpoints messed up the debug
	return G_OK;
};

void _CGD3D9Device::__onLostDevice( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "__onLostDevice" );
	if( !m_DeviceData || 0 == m_pWindowData )
		return;
	uint32_t i, n;
	// create swap chains if needed
	info_printf( "CGD3D9Device::__onLostDevice() called." );

	for( i=0; i<m_nVideoHandlerCount; i++ )
	{
		//m_lstVideoHandler[i]->OnDriverLost();
	}
	
	n = m_pWindowData->nChildWindows;
	if( n > 0 )
	{
		for (uint32_t i=0; i<n; i++)
		{
			if( m_DeviceData->pSwapChain[i] )
			{
				info_printf( "Released swap chain at index %i.", i );
				m_DeviceData->pSwapChain[i]->Release();
				m_DeviceData->pSwapChain[i] = 0;
			}
		}
	};
	if( m_pShader.valid() )
	{
		if( m_pShader->bActive )
		{
			info_printf( "Flushing shader pipeline." );
			m_pShader->pd3dXProgram->EndPass();
			m_pShader->pd3dXProgram->End();
			m_pShader->bActive = false;
		}
		m_pShader.free();
	}
	if( m_pVertexFunction.valid() )
	{
		m_pVertexFunction->bActive = false;
		m_pVertexFunction.free();
	}
	if( m_pPixelFunction.valid() )
	{
		m_pPixelFunction->bActive = false;
		m_pPixelFunction.free();
	}
	if( m_pIndex.valid() )
	{
		m_pIndex->bActive = false;
		m_pIndex.free();
	}
	if( m_pVertexDeclaration.valid() )
	{
		m_pVertexDeclaration->bActive = false;
		m_pVertexDeclaration.free();
	}
	n = 256;
	for( i=0; i<n; i++ )
	{
		if( m_pVertexStream[i].valid() )
		{
			m_pVertexStream[i]->bActive = false;
			m_pVertexStream[i].free();
		}
		if( m_pIndexStream[i].valid() )
		{
			m_pIndexStream[i]->bActive = false;
			m_pIndexStream[i].free();
		}
	}
	if( m_pCurrentRenderTarget )
	{
		m_DeviceData->pDevice->SetRenderTarget( 0, m_pCurrentRenderTarget );
		m_pCurrentRenderTarget->Release();
		m_pCurrentRenderTarget = 0;
	}

	CGD3D9ResourceManager* pMan;
	for(i=0; i< m_nManagerCount;i++) 
	{
		pMan = m_lstpManager[i];
		if( !pMan )
			continue;
		info_printf( "Calling onLostDevice() on resource manager at index %i.", i );
		pMan->onLostDevice();
	}
	m_bLostDevice = true;
	m_bRenderingPaused	= true;

	Beep( 330+232, 50 ); // I used these because breakpoints messed up the debug
	return;
}

void _CGD3D9Device::__onDestroyDevice( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "__onDestroyDevice" );

	for( uint32_t iHandler=0; iHandler<m_nVideoHandlerCount; iHandler++ )
	{
		//m_lstVideoHandler[iHandler]->OnDriverDestroy();
	}
	
	if( m_pCurrentRenderTarget )
	{
		m_pCurrentRenderTarget->Release();
		m_pCurrentRenderTarget = 0;
	}
	CGD3D9ResourceManager* pMan;
	for(uint32_t i=0; i< m_nManagerCount;i++) 
	{
		pMan = m_lstpManager[i];
		if( !pMan )
			continue;
		info_printf( "Calling onDestroyDevice() on resource manager at index %i.", i );
		pMan->onDestroyDevice();
	}

	Beep( 294+232, 50 ); // I used these because breakpoints messed up the debug
	return;
};

#define ID_ARRAY_SIZE_BLOCK 100
#define MAKE_ID_FROM_INDEX( Index ) (-(Index+1))

int32_t _CGD3D9Device::getAvailableID( GINTERFACEID_TYPE ResourceType )
{
	GPROFILE_METHOD( "_CGD3D9Device", "getAvailableID" );
	if( 0 == m_nUsedIDArraySize )
	{// we use an array and use the (index+1)*-1 formula to make a valid ID
		m_pUsedID		= (bool*)malloc( sizeof( bool )*ID_ARRAY_SIZE_BLOCK );
		memset( m_pUsedID, 0, sizeof(bool)*ID_ARRAY_SIZE_BLOCK );
		m_pUsedID[0]	= true;
		m_nUsedIDArraySize	= ID_ARRAY_SIZE_BLOCK;
//		m_nUsedIDCount	= 1;
		return MAKE_ID_FROM_INDEX( 0 );
	}
	int32_t i, n=m_nUsedIDArraySize;
	for( i=0; i<n; i++ )
	{
		if( m_pUsedID[i] )
			continue;
		m_pUsedID[i] = true;
		return MAKE_ID_FROM_INDEX( i );
	}
	// if we got this far, it means there's no space in the array so we need to create more space
	uint32_t newsize = m_nUsedIDArraySize+ID_ARRAY_SIZE_BLOCK;
	bool* OldArray = m_pUsedID,* NewArray = (bool*)malloc( sizeof( bool )*newsize );
	memset( NewArray, 1, sizeof( bool )*m_nUsedIDArraySize ); 
	memset( &NewArray[m_nUsedIDArraySize], 0, sizeof( bool )*ID_ARRAY_SIZE_BLOCK ); 
	NewArray[m_nUsedIDArraySize] = true;
	m_nUsedIDArraySize	= newsize;
	m_pUsedID = NewArray;
	free( OldArray );
	return MAKE_ID_FROM_INDEX(m_nUsedIDArraySize);
};

error_t _CGD3D9Device::RegisterHandler( IGVideoHandler* pHandler )
{
	error_t result = 0;
	if( result = gresizePointerArray( sizeof(IGVideoHandler*), m_nVideoHandlerCount+1, &m_nVideoHandlerArraySize, (void**)&m_lstVideoHandler, 10 ) )
	{
		error_printf("Cannot get new array to store video handler! Out of memory?");
		return -1;
	}
	for( uint32_t i=0; i<m_nVideoHandlerCount; i++ )
	{
		if( m_lstVideoHandler[i] == pHandler )
		{
			warning_printf("Cannot register a video handler that is registered already.");
			return 1;
		}
	}

	m_lstVideoHandler[m_nVideoHandlerCount] = pHandler;
	m_nVideoHandlerCount++;
	return 0;
};

error_t _CGD3D9Device::UnregisterHandler( IGVideoHandler* pHandler )
{
	for( uint32_t i=0; i<m_nVideoHandlerCount; i++ )
	{
		if( m_lstVideoHandler[i] == pHandler )
		{
			m_lstVideoHandler[i] = m_lstVideoHandler[--m_nVideoHandlerCount];
			return 0;
		}
	}
	warning_printf("Cannot unregister a video handler that is not registered!");
	return 1;
};

error_t _CGD3D9Device::SetTargetSize( uint32_t nWidth, uint32_t nHeight )
{
	///------------------------------------
	D3DPRESENT_PARAMETERS* pp = &m_DeviceData->PresentParameters;
	pp->BackBufferWidth  = nWidth;//clientRect.right;
	pp->BackBufferHeight = nHeight;//clientRect.bottom;
	if( pp->BackBufferWidth  <= 0 )
		pp->BackBufferWidth = 1;
	if( pp->BackBufferHeight <= 0 )
		pp->BackBufferHeight = 1;

	IDirect3DDevice9* pDevice=m_DeviceData->pDevice;
	if( !pDevice )
		return -1;

	__onLostDevice();
	HRESULT hr;
	if( 0 > ( hr = pDevice->Reset( pp ) ) )
	{
		error_printf("IDirect3DDevice9::Reset() returned 0x%X!", hr);
		pp->hDeviceWindow = m_pWindowData->hWindowHandle;
		return 1;
	}
	return 	__onResetDevice();
};

error_t _CGD3D9Device::SetTargetFormat( GDATA_TYPE DataFormat )
{
	///------------------------------------
	IDirect3DDevice9* pDevice=m_DeviceData->pDevice;
	D3DPRESENT_PARAMETERS* pp = &m_DeviceData->PresentParameters;
	D3DFORMAT oldFormat = pp->BackBufferFormat;
	pp->BackBufferFormat = GD3DFMTFromGDataType( GUSAGE_RENDERTARGET, DataFormat );//clientRect.right;

	__onLostDevice();
	HRESULT hr;
	if( 0 > ( hr = pDevice->Reset( pp ) ) )
	{
		error_printf("IDirect3DDevice9::Reset() returned 0x%X!", hr);
		pp->BackBufferFormat = oldFormat;

		__onLostDevice();
		HRESULT hr;
		if( 0 > ( hr = pDevice->Reset( pp ) ) )
		{
			error_printf("IDirect3DDevice9::Reset() returned 0x%X!", hr);
			return -1;
		}
	}

	if( 0 > __onResetDevice() )
	{
		error_printf("Failed to handle device reset!");
		return -1;
	}
	return 0;
};


GDATA_TYPE _CGD3D9Device::GetTargetFormat( void )
{
	///------------------------------------
	if( !m_DeviceData )
		return GDATATYPE_UNKNOWN;
	return GDataTypeFromD3DFMT( m_DeviceData->PresentParameters.BackBufferFormat, 0 );//clientRect.right;
}

bool _CGD3D9Device::GetProfilingEnabled( void ) const
{
	return god::genabledDebugger();
};

void _CGD3D9Device::SetProfilingEnabled( bool bEnabled )
{
	genableDebugger( bEnabled );
};

// Returns a pointer to the underlying system object, such as IDirect3DDevice9 instance in this case
void* _CGD3D9Device::GetPrivateData( void )
{
	if( !m_DeviceData ) 
		return 0;
	if( m_DeviceData->pDevice )
		m_DeviceData->pDevice->AddRef();
	return m_DeviceData->pDevice;
}; 		//

// Returns a IGInterface pointer depending on the IID requested
void* _CGD3D9Device::GetInterfacePointer( GINTERFACEID_TYPE InterfaceID )

{
	switch( InterfaceID )
	{
	case GIID_CUSTOMINTERFACE:
	case GIID_VIDEOMANAGER:
		++__nRefCount;
		return this;
	default:
		return 0;
	}
};

