






/// File:GFramework_main.cpp
///
/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2011: Pablo Ariel Zorrilla Cepeda
///
#include "GFramework.h"

#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

float g_ToggleFullScreenDelay = 0;

int32_t CGFrameworkWin32::Run( void )
{
	GPROFILE_METHOD( "CGFramework", "Run" );
	MSG msg;
	msg.message = WM_NULL;

	// This is to calculate the time between frames (a timer)
	__int64			CountsPerSecond		= 0;
	__int64			PrevTimeStamp		= 0;
	__int64			CurrentTimeStamp	= 0;
	double			SecondsPerCount		= 0.0; 
	//__int64			MicrosecondsPerCount	= 0; 
	//__int64			NanosecondsPerCount	= 0; 
	double			MillisecondsPerCount	= 0; 

	QueryPerformanceFrequency( ( LARGE_INTEGER* )&CountsPerSecond );
	MillisecondsPerCount = (1.0*1000 / (CountsPerSecond));
	//MicrosecondsPerCount = (1.0*1000000 / CountsPerSecond);
	//NanosecondsPerCount = (1.0*1000000000 / CountsPerSecond) ;
	SecondsPerCount = (1.0 / (CountsPerSecond));
	QueryPerformanceCounter( ( LARGE_INTEGER* )&PrevTimeStamp );

	info_printf( "Counts per second                 : %i\n"
				"Milliseconds per count (1.0*1000 / CountsPerSecond): %.10f"
				,
				(int)CountsPerSecond, 
				//(double)(SecondsPerCount*1000.0) );
				MillisecondsPerCount );

	info_printf( "Showing window and starting loop..." );
	ShowWindow( m_MainWindow->hWindowHandle, SW_SHOW );
	UpdateWindow( m_MainWindow->hWindowHandle );
	int32_t result = 0;
	while( msg.message != WM_QUIT )
	{
		while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
//		Sleep( 1 );
		// Calculate time from last cycle
		QueryPerformanceCounter( ( LARGE_INTEGER* ) &CurrentTimeStamp );
		//m_nLastTime =  ( CurrentTimeStamp - PrevTimeStamp )* MicrosecondsPerCount;
		//m_nLastTime =  ( CurrentTimeStamp - PrevTimeStamp )* NanosecondsPerCount;
		m_fLastTime =  (float)(( CurrentTimeStamp - PrevTimeStamp )* SecondsPerCount);
		PrevTimeStamp = CurrentTimeStamp;

		if( m_fLastTime < 0.000001f )
		{
			info_printf( "Last frame time recorded was smaller than 0.000001f (%f).", m_fLastTime );
			//info_printf( "Last frame time recorded was smaller than 0.000001f (%f). Clamped at 0.000001f.", m_fLastTime );
			//m_fLastTime = 0.000001f ;
		}
		else if( m_fLastTime > 0.150f )
		{
			info_printf( "Last frame time recorded was longer than 0.150f (%f).", m_fLastTime );
			//info_printf( "Last frame time recorded was longer than 0.150f (%f). Clamping at 0.150f.", m_fLastTime );
			//m_fLastTime = 0.150f;
		}

		if( !g_pFramework )
		{
			info_printf( "g_pFramework is NULL. Exiting game loop." );
			break;
		}
		if(GISTATE_UNUSED & GetInterfaceState())// || !g_pFramework )
		{
			info_printf( "CGFramework was shut down. Attempting to terminate game..." );
			if( g_pFramework )
			{
				//ShutdownFramework();
				info_printf( "Calling ShutdownApplication()." );
				ShutdownApplication();
			}
			Sleep( 2 );
			continue;
		}

		// Update all
		result = __updateFramework( m_fLastTime );
		if( result )
		{
			error_printf( "Failed to update framework. Attemping to shut down game..." );
			Sleep( 2 );
			ShutdownApplication();
//			ShutdownFramework();
			continue;
		};
		if(GISTATE_UNUSED & GetInterfaceState())// || !g_pFramework )
		{
			info_printf( "CGFramework was shut down. Attempting to terminate game..." );
			if( g_pFramework )
			{
				//ShutdownFramework();
				info_printf( "Calling ShutdownApplication()." );
				ShutdownApplication();
			}
			Sleep( 2 );
			continue;
		}
		result = PlaySounds( m_fLastTime );
		if( result )
		{
			error_printf( "Failed to play sounds. Attemping to shut down game..." );
			Sleep( 2 );
			ShutdownApplication();
//			ShutdownFramework();
			continue;
		};

		// 
		// Don't draw because window is minimized/away from user's view 
		// or the application is not able to draw -why?-.
		if( m_bNoDraw )
		{
			Sleep( 2 ); // Leave processing time for the cpu (5 ms will be enough)
            continue;
		}

		// Draw all
//		DrawGame( m_fLastTime );
		result = __drawFramework( m_fLastTime );
		if( result )
		{
			error_printf( "__drawGame() FAILED!" );
			Sleep( 2 );
//			ShutdownFramework();
//			continue;
		};
	}
	info_printf( "Game loop terminated." );
	return result;
};

// Update 
int32_t CGFrameworkWin32::__updateFramework( float fElapsedTime )
{
	GPROFILE_METHOD( "CGFramework", "__updateFramework" );
	error_t result = 0;
	if( result = __updateDevices( fElapsedTime ) )
	{
		error_printf( "__updateDevices() returned 0x%X!", result );
		return E_FAIL; // This shouldn't fail either unless a programming error
	}

	g_ToggleFullScreenDelay -= fElapsedTime;

	if( 0 == (GetInterfaceState() & GISTATE_UNUSED) 
		&& (result = ProcessUserInput( fElapsedTime )) )
	{
		error_printf( "ProcessUserInput() FAILED!" );
		return E_FAIL; // This function shouldn't be able to fail 
	}

//	if( IsPaused() ) // this should be implemented inside UpdateGame
//		return 0;	// If paused, don't update the world

	if( 0 == (GetInterfaceState() & GISTATE_UNUSED) 
		&& (result = UpdateApplication( fElapsedTime )) )
	{
		error_printf( "UpdateGame FAILED!" );
		return E_FAIL;	// This should never happen either. 
	}

	return S_OK;
};

int32_t CGFrameworkWin32::__updateDevices( float fElapsedTime )
{
	GPROFILE_METHOD( "CGFramework", "__updateDevices" );
	if( GISTATE_UNUSED == (GetInterfaceState() & GISTATE_UNUSED) )
		return 0;

	error_t result = 0;
	GHPTR(VideoDevice) garbageVideo;
	GHPTR(Mouse) garbageMouse;
	GHPTR(Keyboard) garbageKeyboard;
	GHPTR(PPUDevice) garbagePPU;
	GHPTR(WavePlayer) garbageWave;
	GHPTR(NetworkManager) garbageNet;
	if( m_pVideoManager )
	{
		IGTrueVideoHandler* pVid = (IGTrueVideoHandler*)m_pVideoManager->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageVideo = pVid) && (result = pVid->Update( fElapsedTime )) )
		{
			error_printf( "IGVideoManager->Update() returned 0x%X.", result );
			return E_FAIL;
		}
		if( GISTATE_UNUSED == (GetInterfaceState() & GISTATE_UNUSED) )
			return 0;
	}

	if( m_pKeyboardManager )
	{
		IGTrueKeyboardHandler* Keyb = (IGTrueKeyboardHandler*)m_pKeyboardManager->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		// We don't want to handle input anymore if the framework was shut down
		if( (garbageKeyboard = Keyb) && (result = Keyb->Poll( fElapsedTime )) )
		{
			error_printf( "Failed trying to update keyboard manager." );
			return E_FAIL;
		}
		if( GISTATE_UNUSED == (GetInterfaceState() & GISTATE_UNUSED) )
			return 0;
	}
	if( m_pMouseManager )
	{
		IGTrueMouseHandler* Mous = (IGTrueMouseHandler*)m_pMouseManager->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageMouse = Mous) && (result = Mous->Poll( fElapsedTime )) )
		{
			error_printf( "Failed trying to update mouse manager." );
			return E_FAIL;
		}
		if( GISTATE_UNUSED == (GetInterfaceState() & GISTATE_UNUSED) )
			return 0;
	}
	if( m_pConnectionManager )
	{
		IGTrueNetworkHandler* Netw = (IGTrueNetworkHandler*)m_pConnectionManager->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageNet = Netw) && (result = Netw->Update( fElapsedTime )) )
		{
			error_printf( "Failed trying to update mouse manager." );
			return E_FAIL;
		}
		if( GISTATE_UNUSED == (GetInterfaceState() & GISTATE_UNUSED) )
			return 0;
	}

	if( m_pWavePlayer )
	{
		IGTrueAudioHandler* Audi = (IGTrueAudioHandler*)m_pWavePlayer->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageWave = Audi) && (result = Audi->Update( fElapsedTime )) )
		{
			error_printf( "Failed trying to update mouse manager." );
			return E_FAIL;
		}
		if( GISTATE_UNUSED == (GetInterfaceState() & GISTATE_UNUSED) )
			return 0;
	}
	if( GISTATE_UNUSED == (GetInterfaceState() & GISTATE_UNUSED) )
		return 0;

	return 0;
};

// Update 
int32_t CGFrameworkWin32::ToggleFullScreen( void )
{
	GPROFILE_METHOD( "CGFramework", "ToggleFullScreen" );
	if( 0 != (GetInterfaceState() & GISTATE_UNUSED) )
	{
		error_printf( "ToggleFullScreen() was called when the framework wasn't initialized yet." );
		return S_OK;
	}

	if( g_ToggleFullScreenDelay > 0 )
	{
		return S_OK;
	}
	g_ToggleFullScreenDelay = 1.0f;
	if( 0 == m_pVideoManager )
	{
		warning_printf( "No video manager set when trying to toggle full screen!." );
		return E_FAIL;
	}

	if( false == m_pVideoManager->FullScreenEnabled() )
	{
		info_printf( "Enabling full screen mode." );
		if( m_pVideoManager->ToggleFullScreen( true ) )
		{
			error_printf( "Failed to enable full screen mode." );
			return E_FAIL;
		}
		else
		{
			if( __resizeWindows() )
			{
				error_printf( "__resizeWindows() FAILED!" );
			}
			info_printf( "Full screen mode enabled successfully." );
		}
	}
	else
	{
		info_printf( "Disabling full screen mode." );
		if( m_pVideoManager->ToggleFullScreen( false ) )
		{
			error_printf( "Failed to disable full screen mode." );
			return E_FAIL;
		}
		else
		{
			if( __resizeWindows() )
			{
				error_printf( "__resizeWindows() FAILED!" );
			}
			info_printf( "Full screen mode disabled successfully." );
		}

	}
	return S_OK;
};

LRESULT CGFrameworkWin32::MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam )
{
	//GPROFILE_METHOD( "CGFramework", "MsgProc" );
	// Is the application in a minimized or maximized state?
	static bool minOrMaxed = false;
	GHPTR(VideoDevice) garbageVideo;
	GHPTR(Mouse) garbageMouse;
	GHPTR(Keyboard) garbageKeyboard;
	GHPTR(PPUDevice) garbagePPU;
	GHPTR(WavePlayer) garbageWave;
	GHPTR(NetworkManager) garbageNet;
	error_t result=0;
	if( m_pVideoManager )
	{
		IGTrueVideoHandler* pVid = (IGTrueVideoHandler*)m_pVideoManager->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageVideo = pVid) && (result = pVid->MsgProc( hWnd, uMsg, wParam, lParam )) )
		{
			error_printf( "VideoManager::MsgProc returned error value. Terminating..." );
			ShutdownApplication();
			//ShutdownFramework();
			return 1;
		}
	}
	if( m_pKeyboardManager )
	{
		IGTrueKeyboardHandler* pKeyb = (IGTrueKeyboardHandler*)m_pKeyboardManager->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageKeyboard = pKeyb) && (result = pKeyb->MsgProc( hWnd, uMsg, wParam, lParam )) )
		{
			error_printf( "KeyboardManager::MsgProc returned error value. Terminating..." );
			ShutdownApplication();
			//ShutdownFramework();
			return 1;
		}
	}
	if( m_pMouseManager )
	{
		IGTrueMouseHandler* pMous = (IGTrueMouseHandler*)m_pMouseManager->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageMouse = pMous ) && (result = pMous->MsgProc( hWnd, uMsg, wParam, lParam )) )
		{
			error_printf( "MouseManager::MsgProc returned error value. Terminating..." );
			ShutdownApplication();
			//ShutdownFramework();
			return 1;
		}
	}
	if( m_pWavePlayer )
	{
		IGTrueAudioHandler* pWave = (IGTrueAudioHandler*)m_pWavePlayer->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageWave = pWave) && (result = pWave->MsgProc( hWnd, uMsg, wParam, lParam )) )
		{
			error_printf( "MouseManager::MsgProc returned error value. Terminating..." );
			ShutdownApplication();
			//ShutdownFramework();
			return 1;
		}
	}

	if( m_pConnectionManager )
	{
		IGTrueNetworkHandler* pNet = (IGTrueNetworkHandler*)m_pConnectionManager->GetInterfacePointer( GIID_CUSTOMINTERFACE );
		if( (garbageNet = pNet) && (result = pNet->MsgProc( hWnd, uMsg, wParam, lParam )) )
		{
			error_printf( "MouseManager::MsgProc returned error value. Terminating..." );
			ShutdownApplication();
			//ShutdownFramework();
			return 1;
		}
	}
	switch( uMsg )
	{
	// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		info_printf( "Received WM_DESTROY message. Posting WM_QUIT..." );
		PostQuitMessage(0);
		return 0;
	case WM_QUIT:
		info_printf( "Received WM_QUIT message. return DefWindowProc( hWnd=%p, uMsg=%u, wParam=%Iu, lParam=%Id )...", hWnd, uMsg, wParam, lParam );
		break;
	case WM_SIZE:
		if( wParam == SIZE_MINIMIZED )
		{
			info_printf("Received SIZE_MINIMIZED. Resizing window elements...");
			minOrMaxed	= true;
			m_bNoDraw	= true;
			break;
		}
		else if( wParam == SIZE_MAXIMIZED )
		{
			info_printf("Received SIZE_MAXIMIZED. Resizing window elements...");
			if( __resizeWindows() )
			{
				error_printf( "__resizeWindows() returned an error." );
			}
			else
			{
				info_printf( "__resizeWindows() succeeded." );
			}
			minOrMaxed	= true;
			m_bNoDraw	= false;
			break;
		}
		else if( wParam == SIZE_RESTORED )
		{
			info_printf("Received SIZE_RESTORED. Resizing window elements...");
			if( __resizeWindows() )
			{
				error_printf( "__resizeWindows() returned an error." );
			}
			else
			{
				info_printf( "__resizeWindows() succeeded." );
			}
			minOrMaxed	= true;
			m_bNoDraw	= false;
			break;
		}
		else 
		{
			info_printf("Received unknown WM_SIZE message.");
			//if( __resizeWindows() )
			//	error_printf( "__resizeWindows() returned an error." );
			//else
			//	info_printf( "__resizeWindows() succeeded." );
			minOrMaxed	= false;
			m_bNoDraw	= false;
		}

		break;
	case WM_SHOWWINDOW:
	case WM_CREATE:
	case WM_EXITSIZEMOVE:
		m_bNoDraw	= false;
		info_printf("Received WM_EXITSIZEMOVE message.");
		if( __resizeWindows() )
		{
			warning_printf( "__resizeWindows() returned an error." );
		}
		else
		{
			info_printf( "__resizeWindows() succeeded." );
		}
		break;
	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);
		break;
	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 320;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;
	case WM_SYSKEYDOWN:
		return 0;
	case WM_CLOSE:
		info_printf( "Received WM_CLOSE message. Calling ShutdownApplication()..." );
		ShutdownApplication();
		return 0;
	};
	if( ApplicationMsgProc( hWnd, uMsg, wParam, lParam ) )
	{
		error_printf( "ApplicationMsgProc( %p, %u, %Iu, %Id ) FAILED!", hWnd, uMsg, wParam, lParam );
		return 0;
	}
	//if( m_pGame && m_pGame->MsgProc( hWnd, uMsg, wParam, lParam ) )
	//{
	//	error_printf( "IGGame::MsgProc( %i, %i, %i, %i ) returned 0x%X!", (int)hWnd, (int)uMsg, (int)wParam, (int)lParam, result );
	//	return 0;
	//}
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
};

HWND CGFrameworkWin32::GetWindowHandle( void )
{
	GPROFILE_METHOD( "CGFramework", "GetWindowHandle" );
	if( 0 == m_MainWindow )
		return 0;
	return m_MainWindow->hWindowHandle;
};

static const wchar_t emptywchar[1] = { 0 };

const wchar_t* CGFrameworkWin32::GetWindowCaption( void )
{
	GPROFILE_METHOD( "CGFramework", "GetWindowCaption" );
	if( 0 == m_MainWindow )
		return emptywchar;
	return m_MainWindow->wszCaption;
};

GRDS(SWindow) _CGFramework::GetWindowData( void )
{
	GPROFILE_METHOD( "CGFramework", "GetWindowData" );
	return gacquire( m_MainWindow );
};


GINTERFACE_STATE _CGFramework::GetInterfaceState( void ) const
{
	GPROFILE_METHOD( "CGFramework", "GetInterfaceState" );
	return m_cInterfaceState;
};

void* _CGFramework::GetInterfacePointer( GINTERFACEID_TYPE pType )
{
	GPROFILE_METHOD( "CGFramework", "GetInterfacePointer" );
	switch( pType )
	{
	case GIID_CUSTOMINTERFACE:
	case GIID_FRAMEWORK:
		return this;
	}
	return 0;
};


// Draw everything.
int32_t _CGFramework::__drawFramework( float fElapsedTime )
{
	GPROFILE_METHOD( "CGFramework", "__drawFramework" );
	int32_t err=0;

	// Get a pointer to the video objects
	GHPTR(VideoDevice) pDevice = GetVideoManager();

	pDevice->SetTargetByIndex( 0 );
	if( pDevice->Clear( true, true, true ) )
	{
		error_printf( "Failed when attempting to clear render target." );
		return G_FAIL;
	}

	if( pDevice->StartRender() )
	{
		error_printf( "StartRender() FAILED!" );
		return G_FAIL;
	}

	if( DrawWindow( 0, fElapsedTime ) )
	{
		if( pDevice->EndRender() )
		{
			error_printf( "EndRender() FAILED!" );
			return G_FAIL;
		}
		error_printf( "Application DrawWindow() FAILED!" );
		return G_FAIL;
	}
	if( pDevice->EndRender() )
	{
		error_printf( "EndRender() FAILED!" );
		return G_FAIL;
	}
	
	if( 0 > pDevice->PresentTarget( 0, 0, 0 ) )
	{
		error_printf( "FAILED to present render target." );
		return G_FAIL;
	}

	return err;
};


int32_t _CGFramework::__resizeWindows( void )
{
	GPROFILE_METHOD( "CGFramework", "__resizeWindows" );
	RECT rc;
	if( 0 == m_MainWindow || 0 == m_MainWindow->hWindowHandle )
	{
		warning_printf("__resizeWindows() was called when there was no window handle available.");
		return -1;
	}

	gpod_ptr<SWindow> newData;
	GetClientRect( m_MainWindow->hWindowHandle, &rc );
	gclone( &newData, m_MainWindow );
	newData->nWidth = rc.right-rc.left;
	newData->nHeight = rc.bottom-rc.top;

	m_MainWindow = newData;

	return 0;
};

void _CGFramework::GetVideoManager		( GOSH(VideoDevice)*	crap ) { GHPTR(VideoDevice)		garbage = *crap; *crap = gacquireInterface( m_pVideoManager		); };
void _CGFramework::GetMouseManager		( GOSH(Mouse)*			crap ) { GHPTR(Mouse)			garbage = *crap; *crap = gacquireInterface( m_pMouseManager		); };
void _CGFramework::GetKeyboardManager	( GOSH(Keyboard)*		crap ) { GHPTR(Keyboard)		garbage = *crap; *crap = gacquireInterface( m_pKeyboardManager	); };
void _CGFramework::GetPCManager			( GOSH(PPUDevice)*		crap ) { GHPTR(PPUDevice)		garbage = *crap; *crap = gacquireInterface( m_pPPUManager		); };
void _CGFramework::GetAudioManager		( GOSH(WavePlayer)*		crap ) { GHPTR(WavePlayer)		garbage = *crap; *crap = gacquireInterface( m_pWavePlayer		); };
void _CGFramework::GetStorageManager	( GOSH(Storage)*		crap ) { GHPTR(Storage)			garbage = *crap; *crap = gacquireInterface( m_pStorageManager	); };
void _CGFramework::GetVideoContext		( GOSH(VideoContext)*	crap ) { GHPTR(VideoContext)	garbage = *crap; *crap = gacquireInterface( m_pVideoContext		); };

// ------------------------------------
// Accessors
GOSH(Storage) _CGFramework::GetStorageManager( void )
{
	//GPROFILE_METHOD( "CGFramework", "GetStorageManager" );
	return gacquireInterface( m_pStorageManager );
};


GOSH(VideoDevice) _CGFramework::GetVideoManager( void )
{ 
	//GPROFILE_METHOD( "CGFramework", "GetVideoManager" );
	return gacquireInterface( m_pVideoManager );
};

GOSH(VideoContext) _CGFramework::GetVideoContext( void )
{ 
	//GPROFILE_METHOD( "CGFramework", "GetVideoContext" );
	return gacquireInterface( m_pVideoContext );
};

GOSH(Mouse) _CGFramework::GetMouseManager( void )
{ 
	//GPROFILE_METHOD( "CGFramework", "GetMouseManager" );
	return gacquireInterface( m_pMouseManager );
};

GOSH(Keyboard) _CGFramework::GetKeyboardManager( void )
{ 
	//GPROFILE_METHOD( "CGFramework", "GetKeyboardManager" );
	return gacquireInterface( m_pKeyboardManager ); 
};

GOSH(PPUDevice) _CGFramework::GetPCManager( void )
{ 
	//GPROFILE_METHOD( "CGFramework", "GetPCManager" );
	return gacquireInterface( m_pPPUManager );
};

GOSH(WavePlayer) _CGFramework::GetAudioManager( void )
{ 
	//GPROFILE_METHOD( "CGFramework", "GetAudioManager" );
	return gacquireInterface( m_pWavePlayer ); 
};

IGGame* _CGFramework::GetGame( void )
{
	//GPROFILE_METHOD( "CGFramework", "GetGame" );
	return m_pGame;
};

