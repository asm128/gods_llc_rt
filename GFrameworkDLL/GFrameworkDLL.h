// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GFRAMEWORKDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GFRAMEWORKDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#include "GFramework.h"

#ifdef GFRAMEWORKDLL_EXPORTS
#define GFRAMEWORKDLL_API __declspec(dllexport)
#else
#define GFRAMEWORKDLL_API __declspec(dllimport)
#endif

namespace god
{
	class CGFramework : public IGFramework, public IGKeyboardHandler, public IGMouseHandler
	{
	public:
		CGFramework( void );
		virtual ~CGFramework( void );
		int32_t						Init( HWND hWnd );
		int32_t						Run();

		virtual error_t				ShutdownFramework( void );
		// Device manager accessors
		virtual void 				GetVideoManager			( GOSH(VideoDevice)*	);
		virtual void 				GetMouseManager			( GOSH(Mouse)*			);
		virtual void 				GetKeyboardManager		( GOSH(Keyboard)*		);
		virtual void 				GetPCManager			( GOSH(PPUDevice)*		);
		virtual void 				GetAudioManager			( GOSH(WavePlayer)*		);
		virtual void 				GetStorageManager		( GOSH(Storage)*		);
		// Resource mana			ger accessors
		virtual void 				GetVideoContext			( GOSH(VideoContext)*	);
		//
		virtual IGGame*				GetGame		( void );

		//--------
		virtual error_t				ToggleFullScreen		( void );

		virtual void				OnMouseMove( int32_t x, int32_t y, int32_t z ){};
		virtual void				OnMouseButtonUp( uint8_t ButtonIndex ){};
		virtual void				OnMouseButtonDown( uint8_t ButtonIndex ){};

		virtual void				OnKeyUp( GSCANCODE_TYPE Key ){};
		virtual void				OnKeyDown( GSCANCODE_TYPE Key ){};
	protected:	
		gpod_ptr<SWindow>				m_MainWindow;
#define GMAX_WINDOWS 32
		wchar_t						m_PCLibFilename[MAX_PATH];		// Parallel Computing DLL
		wchar_t						m_VideoLibFilename[MAX_PATH];	// Video DLL
		wchar_t						m_InputLibFilename[MAX_PATH];	// Input DLL
		wchar_t						m_AudioLibFilename[MAX_PATH];	// Audio/MM DLL
		wchar_t						m_NetworkLibFilename[MAX_PATH];	// Network DLL
		wchar_t						m_GameLibFilename[MAX_PATH];	// Game DLL
		wchar_t						m_StorageLibFilename[MAX_PATH];	// Storage DLL
#ifdef WIN32
		HINSTANCE					m_hInstance;
#endif
	private:
		GINTERFACESTATE_TYPE		m_cInterfaceState;
		// ----------------- Misc
		float						m_fLastTime;		// This is the last frame time
		bool						m_bNoDraw;
		// ----------------------------- Drivers ---------------------------------------
		// ----------------- Parallel compute
		IGPPU*						m_pPPU;			// Parallel processing memory context
		GHPTR(PPUDevice)			m_pPPUManager;	// Parallel processing unit manager
		// ----------------- Video
		IGVideo*					m_pGraphics;			// Graphics context
		GHPTR(VideoDevice)			m_pVideoManager;		// Video manager
		GHPTR(VideoContext)			m_pVideoContext;	// Video resource manager
		// ----------------- Input
		IGInput*					m_pInput;				// Input context
		GHPTR(Mouse)				m_pMouseManager;		// Mouse manager
		GHPTR(Keyboard) 			m_pKeyboardManager;		// Keyboard manager
		// ----------------- Audio
		IGAudio*					m_pAudio;				// Audio context
		GHPTR(WavePlayer)			m_pWavePlayer;			// Audio manager
		// ----------------- Network
		IGNetwork*					m_pNetwork;				// Network context
		GHPTR(NetworkManager)		m_pConnectionManager;	// Connection manager
		// ----------------- Storage
		GHPTR(Storage)				m_pStorageManager;
		// ----------------- Game
		IGGame*						m_pGame;

		// ------------------------------------------------------------------------------------------
		// Load the dlls of the engine.
		virtual int32_t					__loadLibraries( 
			const wchar_t* pComputingDLL, 
			const wchar_t* pInputDLL, 
			const wchar_t* pGraphicsDLL,
			const wchar_t* pAudioDLL, 
			const wchar_t* pNetworkDLL, 
			const wchar_t* pStorageDLL,
			const wchar_t* pGameDLL 
		);
		// Initialize IO devices. This function should be called after LoadLibraries and before any 
		// other initialization. 
		virtual int32_t					__initDevices( void );
	};
}

GFRAMEWORKDLL_API int gcreateFramework		( HWND hWnd );
GFRAMEWORKDLL_API int ginitFramework		( god::id_t frameworkId );
GFRAMEWORKDLL_API int gdestroyFramework		( god::id_t frameworkId );
GFRAMEWORKDLL_API int grunFramework			( god::id_t frameworkId );
GFRAMEWORKDLL_API int gshutdownFramework	( god::id_t frameworkId );

GFRAMEWORKDLL_API int gmouseMove			( god::id_t fId, int32_t x, int32_t y, int32_t z );
GFRAMEWORKDLL_API int gmouseButtonUp		( god::id_t fId, uint8_t ButtonIndex );
GFRAMEWORKDLL_API int gmouseButtonDown		( god::id_t fId, uint8_t ButtonIndex );
GFRAMEWORKDLL_API int gkeyUp				( god::id_t fId, god::GSCANCODE_TYPE Key );
GFRAMEWORKDLL_API int gkeyDown				( god::id_t fId, god::GSCANCODE_TYPE Key );


class GFRAMEWORKDLL_API CGFrameworkDLL
{
public:
	CGFrameworkDLL(void);
	// TODO: add your methods here.
};

extern GFRAMEWORKDLL_API int nGFrameworkDLL;

GFRAMEWORKDLL_API int create3DViewport(HWND hWnd);
GFRAMEWORKDLL_API god::IGFramework* getFramework(int hWnd);
