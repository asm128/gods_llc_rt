//#define GPROFILE_ENABLED
#include "GD3D9.h"
#include "GD3D9Device.h"

#if (defined( _DEBUG ) || defined( DEBUG )) && defined( WIN32 )
#include <crtdbg.h>
#endif

#include "GCore_printf.h"
#include "GDebugger_core.h"


using namespace god;

LRESULT __stdcall MainWndProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );

CGD3D9* g_pVideo = 0;

int32_t	CreateGVideo( HMODULE hModule, IGVideo** ppInterface )
{
	GPROFILE_FUNCTION();
	if( 0 == ppInterface || 0 == hModule )
		return G_INVALIDARGUMENT;
	if( g_pVideo )
	{
		warning_printf( "Trying to create new context when there is one created already. Returning existent..." );
		(*ppInterface) = g_pVideo;
		return 0;
	}
	CGD3D9* pVideo = new CGD3D9(hModule);
#if (defined( _DEBUG ) || defined( DEBUG )) && defined( WIN32 )
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	if( 0 == pVideo )
	{
		error_printf("Not enough memory!");
		return G_NOMEM;
	}

	if( pVideo->internalInit() )
	{
		error_printf( "Failed to initialize video context interface." );
		delete pVideo;
		return G_FAIL;
	};
	g_pVideo = pVideo;
	*ppInterface = pVideo;
	info_printf( "Video context created successfully." );
	return S_OK;
};

int32_t	ReleaseGVideo( IGVideo *pInterface )
{
	GPROFILE_FUNCTION();
	if( (0 == pInterface) || (pInterface != g_pVideo) )
	{
		error_printf( "ReleaseGVideo was called with invalid parameters." );
		return G_INVALIDARGUMENT;
	}
	delete g_pVideo;
	g_pVideo = 0;
	info_printf( "IGVideo context interface was destroyed successfully." );
	return 0;

	error_printf( "The object returned a non-zero reference count meaning that some objects were not properly released." );
	return G_FAIL;
};


_CGD3D9::_CGD3D9( HMODULE hModule )
	:m_DummyMethod( this, "_CGD3D9", "_CGD3D9" )
{
	m_hModule			= hModule;
	m_pD3D9				= 0;
	m_pEnumerator		= 0;

	m_hMainWindow		= 0;
	m_bClassRegistered	= false;
	m_hInstance			= 0;
 
}

_CGD3D9::~_CGD3D9( void )
{
	GPROFILE_METHOD( "_CGD3D9", "~_CGD3D9" );
	if( m_pEnumerator )
		delete m_pEnumerator;
 
	if( m_pD3D9 )
		m_pD3D9->Release();

	if( m_hMainWindow )
		DestroyWindow( m_hMainWindow );

	if( m_bClassRegistered )
		UnregisterClassA( "GD3D9", m_hInstance );
}

int32_t _CGD3D9::internalInit( void )
{
	GPROFILE_METHOD( "_CGD3D9", "internalInit" );
	m_pD3D9 = Direct3DCreate9( D3D_SDK_VERSION ); // D3D Context
	if( !m_pD3D9 )
	{	
		error_printf("Direct3DCreate9( D3D_SDK_VERSION ) FAILED!");
		return G_FAIL;
	}
	else
	{
		info_printf("D3D9 Context created successfully.");
	}
	if( 0 == m_pEnumerator )
	{
		m_pEnumerator = new CGD3D9Enumerator( this );
		if( m_pEnumerator->internalInit( 640, 480, 4, 12 ) )
		{
			error_printf("FAILED to create D3D9 device enumerator!");
			delete m_pEnumerator;
			m_pEnumerator = 0;
			return G_FAIL;
		}
	}
	return G_OK;
}

int32_t _CGD3D9::CreateVideoManager( HINSTANCE hInstance, GRDS(SWindow) pWindowData, uint32_t nMinDepth, 
								  uint32_t nMinStencil, IGTrueVideoHandler** ppDevice )
{
	GPROFILE_METHOD( "_CGD3D9", "CreateVideoManager" );
	if( 0 == ppDevice )
	{
		error_printf("_CGD3D9::CreateVideoManager(): IGVideoManager** ppDevice cannot be NULL!");
		return -1;
	};

	if( false == m_bClassRegistered )
	{
		WNDCLASS* wc = &m_WindowClass;
		wc->style				= NULL; // CS_HREDRAW | CS_VREDRAW;
		wc->lpfnWndProc			= MainWndProc; 
		wc->cbClsExtra			= 0;
		wc->cbWndExtra			= 0;
		wc->hInstance			= hInstance;
		wc->hIcon				= LoadIcon(0, IDI_APPLICATION);
		wc->hCursor				= LoadCursor(0, IDC_ARROW);
		wc->hbrBackground		= CreateSolidBrush(GetSysColor(COLOR_3DFACE));
		wc->lpszMenuName		= 0;
		wc->lpszClassName		= L"GD3D9";
		if( !RegisterClass(wc) )
		{
			MessageBox(0, L"RegisterClass FAILED", 0, 0);
			return G_FAIL;
		}
	}
	if( 0 == m_hMainWindow )
	{
		m_hMainWindow = CreateWindow( L"GD3D9", L"IGVideo", WS_POPUP, 0, 0, 
								GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ),
								0, 0, hInstance, 0 ); 

		if( 0 == m_hMainWindow )
		{
			MessageBox(0, L"CreateWindow FAILED", 0, 0);	
			return G_FAIL;
		}
	}

	CGD3D9Device* pDevice = new CGD3D9Device( this );
	if( 0 == pDevice )
	{
		info_printf("FAILED to construct new CGD3D9Device()! Out of memory?" );
		return G_FAIL;
	}
	if( 0 != pDevice->internalInit( pWindowData, nMinDepth, nMinStencil ) )
	{
		info_printf("FAILED to initialize video manager! Out of memory?" );
		delete pDevice;
		return G_FAIL;
	}

	GHPTR(VideoDevice) garbage = *ppDevice;
	*ppDevice = pDevice; 
	return G_OK;
};

CGD3D9Enumerator* _CGD3D9::getEnumerator( void )
{
	return m_pEnumerator;
};

IDirect3D9* _CGD3D9::getd3d9( void )
{
	return m_pD3D9;
}

LRESULT _CGD3D9::MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam )
{
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

LRESULT __stdcall MainWndProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam )
{
	if( 0 == g_pVideo )
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	return (LRESULT)g_pVideo->MsgProc( hWnd, uMsg, wParam, lParam );
};

void* _CGD3D9::GetPrivateData( void )
{
	if( 0 == m_pD3D9 ) 
		return 0;
	if( m_pD3D9 )
		m_pD3D9->AddRef();
	return m_pD3D9;
}; 
