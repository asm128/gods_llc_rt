/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2009: Pablo Ariel Zorrilla Cepeda
//
// Description: Base framework implementing IGFramework interface.
//
// Creation Date: 2009/12/16
//
#include "IGFramework.h"
#include "IGPrivate.h"

#ifndef _GFRAMEWORK_H
#define _GFRAMEWORK_H

namespace god
{
	typedef class _CGFramework : public IGFramework, public IGKeyboardHandler
	{
	public:
		static _CGFramework*			g_pFramework; // for the msgproc -_-"

		_CGFramework( void );
		virtual ~_CGFramework( void );

		// Initialize the framework. You can call this function at the beginning of main()
		virtual int32_t							Init( HINSTANCE hInstance, 
			const wchar_t* wszWndCaption, 
			const wchar_t* pGameDLL, 
			const wchar_t* pInputDLL, 
			const wchar_t* pGraphicsDLL, 
			const wchar_t* pNetworkDLL,
			const wchar_t* pAudioDLL, 
			const wchar_t* pStorageDLL, 
			const wchar_t* pComputingDLL, 
			uint32_t nWidth=640, uint32_t nHeight=480, uint32_t nPositionX=10, uint32_t nPositionY=10 );
		virtual int32_t							Init( HWND hWnd, 
			const wchar_t* wszWndCaption, 
			const wchar_t* pGameDLL, 
			const wchar_t* pInputDLL, 
			const wchar_t* pGraphicsDLL, 
			const wchar_t* pNetworkDLL,
			const wchar_t* pAudioDLL, 
			const wchar_t* pStorageDLL, 
			const wchar_t* pComputingDLL );
		// function: Run()
		// This function should be called after the Init() function and it shows the window and 
		// executes the main loop
		virtual int32_t							Run( void );
		//virtual int32_t							RunFrame( void );
		// This function destroys the main window. Destroying the main window shuts down the framework.
		virtual int32_t							ShutdownFramework( void );
		// This function should be able to be called whenever required in the custom app.
		virtual int32_t							ToggleFullScreen( void );
		
		// ------------------ Some accessors
		virtual const wchar_t*					GetWindowCaption( void );
		virtual GRDS(SWindow)					GetWindowData( void );

		virtual GOSH(VideoDevice)				GetVideoManager( void );
		virtual GOSH(VideoContext)				GetVideoContext( void );
		virtual GOSH(Mouse)						GetMouseManager( void );
		virtual GOSH(Keyboard)					GetKeyboardManager( void );
		virtual GOSH(PPUDevice)					GetPCManager( void );
		virtual GOSH(WavePlayer)				GetAudioManager( void );
		virtual GOSH(Storage)					GetStorageManager( void );
		// Device manager accessors
		virtual void 	GetVideoManager			( GOSH(VideoDevice)* );
		virtual void 	GetMouseManager			( GOSH(Mouse)* );
		virtual void 	GetKeyboardManager		( GOSH(Keyboard)* );
		virtual void 	GetPCManager			( GOSH(PPUDevice)* );
		virtual void 	GetAudioManager			( GOSH(WavePlayer)* );
		virtual void 	GetStorageManager		( GOSH(Storage)* );
		// Resource manager accessors
		virtual void 	GetVideoContext			( GOSH(VideoContext)* );

		// ------------------------------------
		// Accessors
		virtual IGGame*					GetGame( void );
		
		// ------------------------------------
		virtual void*					GetInterfacePointer( GINTERFACEID_TYPE pType );
		virtual GINTERFACE_STATE		GetInterfaceState( void ) const;
		// Windows specific functions
#ifdef WIN32
		virtual LRESULT					MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );
		virtual HWND					GetWindowHandle( void );
#endif
	protected:
		// ------------------------------------------------------------------------------------------
		// Initialize custom app
		virtual int32_t					InitApplication( void )=0;
		//----------- Begin of main loop function calls --------------
		// Read and process user input.
		virtual int32_t					ProcessUserInput( float fElapsedTime )=0;
		// Message processor for the custom app.
		virtual int32_t					ApplicationMsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam )=0;
		// Update function for custom app.
		virtual int32_t					UpdateApplication( float fElapsedTime )=0;
		// Draw everything. 
		virtual int32_t					DrawWindow( uint32_t nIndex, float fElapsedTime )=0;
		// Play sounds.
		virtual int32_t					PlaySounds( float fElapsedTime )=0;
		//----------- End of main loop function calls --------------

		// ------------------------------------------------------------------------------------------
		// This function will be called when the main window is requested to close.
		virtual int32_t					ShutdownApplication( void )=0;

	protected:	
		gpod_ptr<SWindow>					m_MainWindow;
#define GMAX_WINDOWS 32
		wchar_t							m_PCLibFilename[MAX_PATH];		// Parallel Computing DLL
		wchar_t							m_VideoLibFilename[MAX_PATH];	// Video DLL
		wchar_t							m_InputLibFilename[MAX_PATH];	// Input DLL
		wchar_t							m_AudioLibFilename[MAX_PATH];	// Audio/MM DLL
		wchar_t							m_NetworkLibFilename[MAX_PATH];	// Network DLL
		wchar_t							m_GameLibFilename[MAX_PATH];		// Game DLL
		wchar_t							m_StorageLibFilename[MAX_PATH];		// Game DLL
#ifdef WIN32
		HINSTANCE						m_hInstance;
#endif

	private:
		GINTERFACESTATE_TYPE			m_cInterfaceState;
		// ----------------- Misc
		float							m_fLastTime;		// This is the last frame time
		bool							m_bNoDraw;
		// ----------------------------- Drivers ---------------------------------------
		// ----------------- Parallel compute
		IGPPU*							m_pPPU;			// Parallel processing memory context
		GHPTR(PPUDevice)				m_pPPUManager;	// Parallel processing unit manager
		// ----------------- Video
		IGVideo*						m_pGraphics;			// Graphics context
		GHPTR(VideoDevice)				m_pVideoManager;		// Video manager
		GHPTR(VideoContext)				m_pVideoContext;	// Video resource manager
		// ----------------- Input
		IGInput*						m_pInput;				// Input context
		GHPTR(Mouse)					m_pMouseManager;		// Mouse manager
		GHPTR(Keyboard) 				m_pKeyboardManager;		// Keyboard manager
		// ----------------- Audio
		IGAudio*						m_pAudio;				// Audio context
		GHPTR(WavePlayer)				m_pWavePlayer;			// Audio manager
		// ----------------- Network
		IGNetwork*						m_pNetwork;				// Network context
		GHPTR(NetworkManager)			m_pConnectionManager;	// Connection manager
		// ----------------- Storage
		GHPTR(Storage)					m_pStorageManager;
		// ----------------- Game
		IGGame*							m_pGame;

		// ------------------------------------------------------------------------------------------
		// Load the dlls of the engine.
		virtual int32_t					__loadLibraries( 
			const wchar_t* pComputingDLL, 
			const wchar_t* pInputDLL, 
			const wchar_t* pGraphicsDLL,
			const wchar_t* pAudioDLL, 
			const wchar_t* pNetworkDLL, 
			const wchar_t* pStorageDLL,
			const wchar_t* pGameDLL );
		// Initialize IO devices. This function should be called after LoadLibraries and before any 
		// other initialization. 
		virtual int32_t					__initDevices( void );
		// Call update subfunctions UpdateContexts and UpdateGame.
		virtual int32_t					__updateFramework( float fElapsedTime );
		virtual int32_t					__drawFramework( float fElapsedTime );
		// Update general engine stuff.
		virtual int32_t					__updateDevices( float fElapsedTime );
		virtual int32_t					__resizeWindows( void );
		// Cleanup what's been initialized in this base app
		virtual int32_t					Cleanup( void );
	} CGFrameworkWin32;

}// namespace

#endif // _GFRAMEWORK_H
