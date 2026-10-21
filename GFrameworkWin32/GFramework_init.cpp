/// File:GFramework_init.cpp
///
/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2011: Pablo Ariel Zorrilla Cepeda
///
#include "GFramework.h"
#include "GLibraryLoader.h"
#include <strsafe.h>

#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

CGFrameworkWin32* CGFrameworkWin32::g_pFramework = NULL;

// Message processor wrapper function
LRESULT __stdcall MainWndProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );

CGLibraryLoader		g_LibraryLoader;

// Message processor wrapper function
LRESULT __stdcall MainWndProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam )
{
//	GPROFILE_METHOD( 0, 0, "MainWndProc" );
	if( 0 == CGFrameworkWin32::g_pFramework )
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	return CGFrameworkWin32::g_pFramework->MsgProc( hWnd, uMsg, wParam, lParam );
};

_CGFramework::_CGFramework( void )
{
	GPROFILE_METHOD( "_CGFramework", "_CGFramework" );
// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

//	m_dwEntityType			= ;
	m_bNoDraw				= false;

	m_pPPU					= NULL;	// PPU resource context.
	m_pInput				= NULL;	// Input context object.
	m_pGraphics				= NULL;	// Graphics context.
	m_pAudio				= NULL; // Audio context
	m_pNetwork				= NULL;
	m_pStorageManager		= NULL;
	m_pGame					= NULL;
	m_MainWindow			= 0;
	m_fLastTime				= 0.0f;

	if( g_pFramework )
		MessageBox( NULL, L"Global framework pointer is not empty!", L"Error", MB_OK );

	g_pFramework	= this;
	m_cInterfaceState	= GISTATE_UNUSED;
};

_CGFramework::~_CGFramework( void )
{
	GPROFILE_METHOD( "_CGFramework", "~_CGFramework" );
	g_pFramework			= NULL; // set the global pointer to null!
	Cleanup();
};

int32_t _CGFramework::Cleanup( void )
{
	GPROFILE_METHOD( "_CGFramework", "Cleanup" );
	m_bNoDraw				= true;

	if( m_pGame )
	{
		g_LibraryLoader.DestroyGameContext( m_pGame );
	};

	m_pVideoContext			= NULL;
	m_pVideoManager			= NULL;
	m_pWavePlayer			= NULL;	// Keyboard device.
	m_pMouseManager			= NULL;	// Mouse device.
	m_pKeyboardManager		= NULL;
	m_pConnectionManager	= NULL;
	m_pPPUManager			= NULL;

	if( m_pGraphics )
	{
		if( g_LibraryLoader.DestroyVideoContext( m_pGraphics ) )
		{
			error_printf( "Graphics context returned a nonzero reference count, meaning that some objects were not properly released." );
		}
		else
		{
			info_printf( "Graphics context properly released." );
		}
	}
	if( m_pInput )
	{
		if( g_LibraryLoader.DestroyInputContext( m_pInput ) )
		{
			error_printf( "Input context returned a nonzero reference count, meaning that some objects were not properly released." );
		}
		else
		{
			info_printf( "Input context properly released." );
		}
	}
	if( m_pAudio )
	{
		if( g_LibraryLoader.DestroyAudioContext( m_pAudio ) )
		{
			error_printf( "Multimedia context returned a nonzero reference count, meaning that some objects were not properly released." );
		}
		else
		{
			info_printf( "Multimedia context properly released." );
		}
	}
	if( m_pNetwork )
	{
		if( g_LibraryLoader.DestroyNetworkContext( m_pNetwork ) )
		{
			error_printf( "DestroyNetworkContext() failed!" );
		}
		else
		{
			info_printf( "Network context properly released." );
		}
	}
	if( m_pPPU )
	{
		if( g_LibraryLoader.DestroyPPUContext( m_pPPU ) )
		{
			error_printf( "DestroyPPUContext() failed!" );
		}
		else
		{
			info_printf( "Parallel computing context properly released." );
		}
	}

	if( m_hInstance )
	{
		UnregisterClassW( L"CGFramework", m_hInstance );
		info_printf( "\"CGFramework\" WNDCLASS unregistered." );
	}
	m_pPPU					= NULL;
	m_pGraphics				= NULL;	// Graphics context.
	m_pInput				= NULL;	// Input context object.
	m_pAudio				= NULL;	// Input context object.
	m_pNetwork				= NULL;
	m_pGame					= NULL;
	info_printf( "CGFramework Cleanup complete." );
	return 0;
};


//
// - function Init( HINSTANCE, wchar_t* )
// Create the application window and call initialization subfunctions: LoadLibraries, InitDevices, 
// InitResourceManagers, InitApplication
//
int32_t _CGFramework::Init( HINSTANCE hInstance, 
			const wchar_t* wszWndCaption, 
			const wchar_t* pGameDLL, 
			const wchar_t* pInputDLL, 
			const wchar_t* pGraphicsDLL, 
			const wchar_t* pNetworkDLL,
			const wchar_t* pAudioDLL, 
			const wchar_t* pStorageDLL, 
			const wchar_t* pComputingDLL, 
			uint32_t nWidth, uint32_t nHeight, uint32_t nPositionX, uint32_t nPositionY )
{
	GPROFILE_METHOD( "_CGFramework", "Init" );
	m_hInstance	= hInstance;

	gpod_ptr<SWindow> MainWindowData;

	// Default to a window with a client area rectangle of width x height.
	RECT r = { (LONG)nPositionX, (LONG)nPositionY, (LONG)nWidth, (LONG)nHeight };
	MainWindowData->nPositionX		= r.left;
	MainWindowData->nPositionY		= r.top;
	MainWindowData->nWidth			= r.right;
	MainWindowData->nHeight		= r.bottom;

	info_printf( "Window position: x=%i. y=%i\n"
		"Window size: x=%i. y=%i"
		, MainWindowData->nPositionX
		, MainWindowData->nPositionY
		, MainWindowData->nWidth
		, MainWindowData->nHeight );

	uint32_t i =0;
	while( true )
	{
		MainWindowData->wszCaption[i] = wszWndCaption[i];
		if(L'\0' == wszWndCaption[i])
			break;
		if( 64 == i )
		{
			MainWindowData->wszCaption[i] = L'\0';
			break;
		}
		i++;
	}

	WNDCLASS windowClass = {};
	WNDCLASS* wc = &windowClass;
	wc->style				= NULL; // CS_HREDRAW | CS_VREDRAW;
	wc->lpfnWndProc			= MainWndProc; 
	wc->cbClsExtra			= 0;
	wc->cbWndExtra			= 0;
	wc->hInstance			= hInstance;
	wc->hIcon				= LoadIcon(0, IDI_APPLICATION);
	wc->hCursor				= LoadCursor(0, IDC_ARROW);
	wc->hbrBackground		= CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	wc->lpszMenuName		= 0;
	wc->lpszClassName		= L"CGFramework";
	MainWindowData->dwStyle	= WS_OVERLAPPED | WS_THICKFRAME | WS_BORDER | WS_MAXIMIZEBOX |
								WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX;
	if( !RegisterClass(wc) )
	{
		error_printf( "Failed to register WNDCLASS \"CGFramework\"." );
		MessageBox(0, L"RegisterClass FAILED", 0, 0);
		return G_FAIL;
	}
	else
	{
		info_printf( " \"CGFramework\" WNDCLASS registered successfully." );
	}
	MainWindowData->WindowClass = windowClass;

	MainWindowData->hWindowHandle = CreateWindow(	L"CGFramework", wszWndCaption, 
								MainWindowData->dwStyle, MainWindowData->nPositionX, MainWindowData->nPositionY, 
								r.right + GetSystemMetrics( SM_CXFRAME )*2, 
								r.bottom + GetSystemMetrics( SM_CYFRAME ) *2
								+ GetSystemMetrics( SM_CYCAPTION ),
								0, 0, hInstance, 0
							); 

	if( 0 == MainWindowData->hWindowHandle )
	{
		error_printf( "CreateWindow FAILED!" );
		MessageBox(0, L"CreateWindow FAILED", 0, 0);
		ShutdownFramework();
		return G_FAIL;
	}

	// we don't make any more changes to these
	m_MainWindow = MainWindowData;

	error_t err = 0; // error
	// Load engine shared libraries.
   	if( err = __loadLibraries( pComputingDLL, pInputDLL, pGraphicsDLL, pAudioDLL, pNetworkDLL, pStorageDLL, pGameDLL ) )
	{
		error_printf( "Failed to load extern modules." );
		ShutdownFramework();
		return err;
	}
	else
	{
		info_printf( "__loadLibraries() succeeded." );
	}

	// Initialize devices ( Input, Renderer, Sound, Network and anything else ).
	if( err = __initDevices() )
	{
		error_printf( "Failed to initialize devices." );
		ShutdownFramework();
		return err;
	}
	else
	{
		info_printf( "__initDevices() succeeded." );
	}

	if( m_pVideoManager )
	{
		if( err = m_pVideoManager->CreateResourceManager( &m_pVideoContext ) )
		{
			error_printf( "Failed to create default resource manager." );
			return err;
		}
		else
		{
			info_printf( "Default resource manager created successfully." );
		}
		/* Let's get rid of renderers in framework!
		if( err = m_MeshRenderer.InitMeshRenderer( presman ) )
		{
			error_printf( "Cannot initialize default mesh renderer." );
			MessageBox(0, L"Cannot initialize mesh renderer", 0, 0);
			ShutdownFramework();
			return err;
		}
		else
		{
			m_MeshRenderer.SetDefaultCamera(0);
			info_printf( "Default mesh renderer initialized successfully." );
		}
		if( err = m_SpriteRenderer.InitSpriteRenderer( presman ) )
		{
			error_printf( "Cannot initialize default sprite renderer." );
			MessageBox(0, L"Cannot initialize sprite renderer", 0, 0);
			ShutdownFramework();
			return err;
		}
		else
		{
			m_SpriteRenderer.SetDefaultCamera(0);
			info_printf( "Default sprite renderer initialized successfully." );
		}
*/
	}

	m_cInterfaceState = GISTATE_NORMAL;

	//if( err = m_pGame->InitGame( this ) )
	//{
	//	error_printf( "Failed to initialize game object. InitGame() returned 0x%X", err );
	//	ShutdownFramework();
	//	m_cInterfaceState = GISTATE_UNUSED;
	//	return err;
	//}
	//else
	//{
	//	info_printf( "Game object created successfully." );
	//}
	if( InitApplication() )
	{
		error_printf( "InitApplication() FAILED!" );
		MessageBox(0, L"InitApplication() FAILED!", 0, 0);
		ShutdownApplication();
		m_cInterfaceState = GISTATE_UNUSED;
		return G_FAIL;
	}
	else
	{
		info_printf( "InitApplication() succeeded!" );
	}
	return S_OK;
};

//
// - function Init( HINSTANCE, wchar_t* )
// Create the application window and call initialization subfunctions: LoadLibraries, InitDevices, 
// InitResourceManagers, InitApplication
//
int32_t _CGFramework::Init( HWND hWnd, 
			const wchar_t* wszWndCaption, 
			const wchar_t* pGameDLL, 
			const wchar_t* pInputDLL, 
			const wchar_t* pGraphicsDLL, 
			const wchar_t* pNetworkDLL,
			const wchar_t* pAudioDLL, 
			const wchar_t* pStorageDLL, 
			const wchar_t* pComputingDLL )
{
	GPROFILE_METHOD( "_CGFramework", "Init" );
	m_hInstance	= GetModuleHandle(NULL);

	gpod_ptr<SWindow> MainWindowData;

	// Default to a window with a client area rectangle of width x height.
	RECT r = {0};
	GetWindowRect(hWnd, &r);
	MainWindowData->nPositionX	= r.left								+ 2; //GetSystemMetrics(SM_CYDLGFRAME);
	MainWindowData->nPositionY	= r.top									+ 2; //GetSystemMetrics(SM_CXDLGFRAME);
	MainWindowData->nWidth		= r.right - MainWindowData->nPositionX	- 2; //GetSystemMetrics(SM_CYDLGFRAME);
	MainWindowData->nHeight		= r.bottom - MainWindowData->nPositionY - 2; //GetSystemMetrics(SM_CXDLGFRAME);

	info_printf( "Window position: x=%i. y=%i\n"
		"Window size: x=%i. y=%i"
		, MainWindowData->nPositionX
		, MainWindowData->nPositionY
		, MainWindowData->nWidth
		, MainWindowData->nHeight );

	uint32_t i =0;
	while( true )
	{
		MainWindowData->wszCaption[i] = wszWndCaption[i];
		if(L'\0' == wszWndCaption[i])
			break;
		if( 64 == i )
		{
			MainWindowData->wszCaption[i] = L'\0';
			break;
		}
		i++;
	}

	WNDCLASS windowClass = {};
	WNDCLASS* wc = &windowClass;
	wc->style				= CS_HREDRAW | CS_VREDRAW;
	wc->lpfnWndProc			= MainWndProc; 
	wc->cbClsExtra			= 0;
	wc->cbWndExtra			= 0;
	wc->hInstance			= m_hInstance;
	wc->hIcon				= LoadIcon(0, IDI_APPLICATION);
	wc->hCursor				= LoadCursor(0, IDC_ARROW);
	wc->hbrBackground		= CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	wc->lpszMenuName		= 0;
	wc->lpszClassName		= L"CGFramework";
	static wchar_t className[256] = {0};
	GetClassNameW( hWnd, className, 256 );
	GetClassInfo(m_hInstance, className, wc );

	WINDOWINFO wndInfo={};
	GetWindowInfo(hWnd,&wndInfo);
	wc->style = wndInfo.dwStyle;
	MainWindowData->WindowClass = windowClass;

	//if( !RegisterClass(wc) )
	//{
	//	error_printf( "Failed to register WNDCLASS \"CGFramework\"." );
	//	MessageBox(0, L"RegisterClass FAILED", 0, 0);
	//	return G_FAIL;
	//}
	//else
	//{
	//	info_printf( " \"CGFramework\" WNDCLASS registered successfully." );
	//}

	MainWindowData->hWindowHandle = hWnd; 

	if( 0 == MainWindowData->hWindowHandle )
	{
		error_printf( "CreateWindow FAILED!" );
		MessageBox(0, L"CreateWindow FAILED", 0, 0);
		ShutdownFramework();
		return G_FAIL;
	}

	// we don't make any more changes to these
	MainWindowData->dwStyle	= WS_OVERLAPPED | WS_THICKFRAME | WS_BORDER | WS_MAXIMIZEBOX |
								WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX;
	m_MainWindow = MainWindowData;

	error_t err = 0; // error
	// Load engine shared libraries.
   	if( err = __loadLibraries( pComputingDLL, pInputDLL, pGraphicsDLL, pAudioDLL, pNetworkDLL, pStorageDLL, pGameDLL ) )
	{
		error_printf( "Failed to load extern modules." );
		ShutdownFramework();
		return err;
	}
	else
	{
		info_printf( "__loadLibraries() succeeded." );
	}

	// Initialize devices ( Input, Renderer, Sound, Network and anything else ).
	if( err = __initDevices() )
	{
		error_printf( "Failed to initialize devices." );
		ShutdownFramework();
		return err;
	}
	else
	{
		info_printf( "__initDevices() succeeded." );
	}

	if( m_pVideoManager )
	{
		if( err = m_pVideoManager->CreateResourceManager( &m_pVideoContext ) )
		{
			error_printf( "Failed to create default resource manager." );
			return err;
		}
		else
		{
			info_printf( "Default resource manager created successfully." );
		}
		/* Let's get rid of renderers in framework!
		if( err = m_MeshRenderer.InitMeshRenderer( presman ) )
		{
			error_printf( "Cannot initialize default mesh renderer." );
			MessageBox(0, L"Cannot initialize mesh renderer", 0, 0);
			ShutdownFramework();
			return err;
		}
		else
		{
			m_MeshRenderer.SetDefaultCamera(0);
			info_printf( "Default mesh renderer initialized successfully." );
		}
		if( err = m_SpriteRenderer.InitSpriteRenderer( presman ) )
		{
			error_printf( "Cannot initialize default sprite renderer." );
			MessageBox(0, L"Cannot initialize sprite renderer", 0, 0);
			ShutdownFramework();
			return err;
		}
		else
		{
			m_SpriteRenderer.SetDefaultCamera(0);
			info_printf( "Default sprite renderer initialized successfully." );
		}
*/
	}

	m_cInterfaceState = GISTATE_NORMAL;

	//if( err = m_pGame->InitGame( this ) )
	//{
	//	error_printf( "Failed to initialize game object. InitGame() returned 0x%X", err );
	//	ShutdownFramework();
	//	m_cInterfaceState = GISTATE_UNUSED;
	//	return err;
	//}
	//else
	//{
	//	info_printf( "Game object created successfully." );
	//}
	if( InitApplication() )
	{
		error_printf( "InitApplication() FAILED!" );
		MessageBox(0, L"InitApplication() FAILED!", 0, 0);
		ShutdownApplication();
		m_cInterfaceState = GISTATE_UNUSED;
		return G_FAIL;
	}
	else
	{
		info_printf( "InitApplication() succeeded!" );
	}
	return S_OK;
};

int32_t _CGFramework::__loadLibraries( const wchar_t* pPPUDLL, 
								  const wchar_t* pInputDLL, 
								  const wchar_t* pGraphicsDLL, 
								  const wchar_t* pAudioDLL, 
								  const wchar_t* pNetworkDLL,
								  const wchar_t* pStorageDLL,
								  const wchar_t* pGameDLL
								  )
{
	GPROFILE_METHOD( "_CGFramework", "__loadLibraries" );
	// 
	if( pPPUDLL && g_LibraryLoader.CreatePPUContext( pPPUDLL, &m_pPPU ) )
	{
		error_wprintf(L"Failed to load parallel computing library (%s).", pPPUDLL );
		MessageBoxW( NULL, L"Failed to load parallel computing library.", pPPUDLL, MB_OK | MB_ICONWARNING );
		//return G_FAIL;
		m_PCLibFilename[0] = 0;
	}
	else if( pPPUDLL )
	{
		debug_wprintf(L"%s loaded.", pPPUDLL);
		StringCchCopyW( m_PCLibFilename, 256, pPPUDLL );
	}
	//
	if( pInputDLL && g_LibraryLoader.CreateInputContext( pInputDLL, &m_pInput ) )
	{
		error_wprintf(L"Failed to load user input library (%s).", pInputDLL );
		MessageBoxW( NULL, L"Failed to load input library.", pInputDLL, MB_OK | MB_ICONERROR );
		m_InputLibFilename[0] = 0;
		return G_FAIL;
	}
	else if( pInputDLL )
	{
		debug_wprintf(L"%s loaded.", pInputDLL);
		StringCchCopyW( m_InputLibFilename, 256, pInputDLL );
	}
	// 
	if( pGraphicsDLL && g_LibraryLoader.CreateVideoContext( pGraphicsDLL, &m_pGraphics ) )
	{
		error_wprintf(L"Failed to load video device library (%s).", pGraphicsDLL);
		MessageBoxW( NULL, L"Failed to load 3D graphics library.", pGraphicsDLL, MB_OK | MB_ICONERROR );
		m_VideoLibFilename[0] = 0;
		return G_FAIL;
	}
	else if( pGraphicsDLL )
	{
		debug_wprintf(L"%s loaded.", pGraphicsDLL);
		StringCchCopyW( m_VideoLibFilename, 256, pGraphicsDLL );
	}
	//
	if( pAudioDLL && g_LibraryLoader.CreateAudioContext( pAudioDLL, &m_pAudio ) )
	{
		error_wprintf(L"Failed to load media device library (%s).", pAudioDLL);
		MessageBoxW( NULL, L"Failed to load audio library.", pAudioDLL, MB_OK | MB_ICONERROR );
		m_AudioLibFilename[0] = 0;
		return G_FAIL;
	}
	else if( pAudioDLL )
	{
		debug_wprintf(L"%s loaded.", pAudioDLL);
		StringCchCopyW( m_AudioLibFilename, 256, pAudioDLL );
	}

	if( pNetworkDLL && g_LibraryLoader.CreateNetworkContext( pNetworkDLL, &m_pNetwork ) )
	{
		error_wprintf(L"Failed to load network device library (%s).", pNetworkDLL);
		MessageBoxW( NULL, L"Failed to load network library.", pNetworkDLL, MB_OK | MB_ICONERROR );
		m_NetworkLibFilename[0] = 0;
		return G_FAIL;
	}
	else if( pNetworkDLL )
	{
		debug_wprintf(L"%s loaded.", pNetworkDLL);
		StringCchCopyW( m_NetworkLibFilename, 256, pNetworkDLL );
	}

	if( pGameDLL && g_LibraryLoader.CreateGameContext( pGameDLL, &m_pGame ) )
	{
		error_wprintf(L"Failed to load game library (%s).", pGameDLL);
		MessageBoxW( NULL, L"Failed to load game library.", pGameDLL, MB_OK | MB_ICONERROR );
		m_GameLibFilename[0] = 0;
		return G_FAIL;
	}
	else if( pGameDLL )
	{
		debug_wprintf(L"%s loaded.", pGameDLL);
		StringCchCopyW( m_GameLibFilename, 256, pGameDLL );
	}

	if( pStorageDLL && g_LibraryLoader.CreateStorage( pStorageDLL, &m_pStorageManager ) )
	{
		error_wprintf(L"Failed to load storage library (%s).", pStorageDLL);
		MessageBoxW( NULL, L"Failed to load storage library.", pStorageDLL, MB_OK | MB_ICONERROR );
		m_StorageLibFilename[0] = 0;
		return G_FAIL;
	}
	else if( pStorageDLL )
	{
		debug_wprintf(L"%s loaded.", pStorageDLL);
		StringCchCopyW( m_StorageLibFilename, 256, pStorageDLL );
	}

	return S_OK;
};


int32_t _CGFramework::__initDevices( void )
{
	GPROFILE_METHOD( "_CGFramework", "__initDevices" );
	if( 0 == m_MainWindow )
		return G_FAIL;

	// Create devices
	int32_t r;
	IGTruePPUHandler* newPPU=0;
	if( ( 0 != m_pPPU ) && 
		( r = m_pPPU->CreatePPUManager( &newPPU, 0 ) ) 
	)
	{
		error_printf("->CreatePPUManager() FAILED! error: 0x%X", r);
		return r;
	}
	else
	{
		m_pPPUManager = newPPU;
	}

	// Create input managers
	IGTrueKeyboardHandler* newKeyb = 0;
	if( r = m_pInput->CreateKeyboardManager( &newKeyb, m_MainWindow->hWindowHandle ) )
	{
		error_printf("IGInput::CreateKeyboardManager() returned 0x%X", r);
		return r;
	}
	else
	{
		m_pKeyboardManager = newKeyb;
	}
	IGTrueMouseHandler* newMous = 0;
	if( r = m_pInput->CreateMouseManager( &newMous, m_MainWindow->hWindowHandle ) )
	{
		error_printf("IGInput::CreateMouseManager() returned 0x%X", r);
		return r;
	}
	else
	{
		m_pMouseManager = newMous;
	}

	m_pKeyboardManager->AddHandler( this );
	m_pKeyboardManager->SetForeground( true );
	m_pMouseManager->SetForeground( true );
	
	IGTrueVideoHandler* newVid = 0;
	if( r = m_pGraphics->CreateVideoManager( m_hInstance, m_MainWindow, 24, 8, &newVid ) )
	{
		error_printf("IGVideo::CreateVideoManager() returned 0x%X", r);
		return r;
	}
	else
	{
		info_printf( "Video manager created successfully." );
		m_pVideoManager = newVid;
	}
#ifdef _TODO
	if( r = m_pAudio->CreateWavePlayer( &m_pWavePlayer, m_MainWindow->hWindowHandle ) )
	{
		error_printf( "Failed to create wave player." );
		return r; 
	}
	else
	{
		info_printf( "Wave player created successfully." );
	}
	if( r = m_pNetwork->CreateDevice( &m_pNetworkDevice ) )
	{
		error_printf( "Failed to create network device." );
		return r;
	}
	else
	{
		info_printf( "Network device created successfully." );
	}
#endif

	return S_OK;
};

int32_t _CGFramework::ShutdownFramework( void )
{
	GPROFILE_METHOD( "_CGFramework", "ShutdownFramework" );
	info_printf( "Shutting down framework." );

/* Let's get rid of renderers in framework!
	m_MeshRenderer.ShutdownRenderer();
	m_SpriteRenderer.ShutdownRenderer();
*/
	m_bNoDraw			= true;
	g_pFramework		= NULL;

	if( m_MainWindow && m_MainWindow->hWindowHandle )
	{
		info_printf( "Destroying main window." );
		DestroyWindow( m_MainWindow->hWindowHandle );
		m_MainWindow->hWindowHandle = 0;
	}
	m_cInterfaceState	= GISTATE_UNUSED;
	return S_OK;
};
