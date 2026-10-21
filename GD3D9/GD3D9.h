// File:D3D9.h
//
// Copyright © 2011: Pablo Ariel Zorrilla Cepeda
// 
// Any use or distribution of this code is strictly prohibited
//
#include "GD3D9_core.h"
#include "IGVideo.h"
#include "IGPrivate.h"

//#define GPROFILE_ENABLED
#include "GDebugger_core.h"

namespace god
{
	typedef class _CGD3D9 CGD3D9;
	typedef class _CGD3D9Device CGD3D9Device;

	// enumeration stuff
	typedef class _CGD3D9Enumerator
	{
	public:
		_CGD3D9Enumerator( CGD3D9* pContext );
		~_CGD3D9Enumerator( void ); 

		int32_t				internalInit( int nMinWidth, int nMinHeight, int nMinDepth, int nMinStencil );
		uint32_t			getAdapterCount( void );
		GD3D9ADAPTER_DATA*	getAdapterData( void );
		D3DDISPLAYMODE*		getDesktopMode( void )
		{
			return &m_DesktopMode;
		};
   private:
		CGD3D9*					m_pContext;
		// 
		D3DDISPLAYMODE			m_DesktopMode;			// current desktop display mode
		uint32_t				m_nFormatCount;			// total adapter formats possible
		D3DFORMAT				m_nAdapterFormat[10];	// list of possible adapter formats
		uint32_t				m_nMinWidth;			// minimum screen with
		uint32_t				m_nMinHeight;			// minimum screen height
		uint32_t				m_nMinBits;				// minimum backbuffer bits
		uint32_t				m_nMinDepth;			// minimum depth bits
		uint32_t				m_nMinStencil;			// minimum stencil bits
		BYTE					m_nAdapterCount;
		GD3D9ADAPTER_DATA		m_AdapterData[20];
		//
		uint32_t				m_nSelectedAdapter;
		uint32_t				m_nSelectedDevice;
		uint32_t				m_nSelectedCombo;
		//
		int32_t					__enumAdapters( void );
		int32_t					__enumDevices( GD3D9ADAPTER_DATA& );
		int32_t					__enumCombos( GD3D9DEVICEINFO_DATA& );
		uint32_t				__getBits( D3DFORMAT );
		int32_t					__confirmDevice( D3DCAPS9*, DWORD, D3DFORMAT );
		bool					__confirmDepthFmt( GD3D9COMBO_DATA* );
   } CGD3D9Enumerator; // class

	//
	typedef class _CGD3D9 : public IGVideo
	{
	public:
		_CGD3D9( HMODULE hModule );
		virtual ~_CGD3D9( void );

		virtual int32_t				internalInit( void );
		virtual IDirect3D9*			getd3d9( void );
		virtual	HINSTANCE			getHInstance( void ){return m_hInstance; };
		virtual	HWND				getWindowHandle( void ){return m_hMainWindow; };
		virtual CGD3D9Enumerator*	getEnumerator( void );
		virtual LRESULT				MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );

		//------------------- IGVideo 
		//virtual int32_t				CreateVideoManager( HWND hWnd, IGVideoManager ** ppDevice );
		//virtual int32_t				CreateVideoManager( HINSTANCE hInstance, GRDS(SWINDOW) pWindowData, uint32_t nMinDepth, uint32_t nMinStencil, GOSH(VideoDevice)* ppDevice );
		virtual int32_t				CreateVideoManager( HINSTANCE hInstance, GRDS(SWindow) pWindowData, uint32_t nMinDepth, uint32_t nMinStencil, IGTrueVideoHandler** ppDevice );
		virtual void*				GetPrivateData( void );
		//------------------- IGInterface
	private:
		CGDbgMethod					m_DummyMethod;
		HWND						m_hMainWindow;
		WNDCLASS					m_WindowClass;
		HINSTANCE					m_hInstance;
		bool						m_bClassRegistered; // this is set to true if the wndclass is already registered

		CGD3D9Enumerator*			m_pEnumerator;

		//						
		//uint32_t					m_nDeviceCount;
		//uint32_t					m_nDeviceArraySize;
		//CGD3D9Device**				m_ppDeviceList;	
		// DIRECT3D VARIABLES
		IDirect3D9*					m_pD3D9;
		HMODULE						m_hModule;
	} CGD3D9;


	//// ----------------------- HELPER FUNCTIONS
	//int32_t GHCreateVideoBuffer( GOSH(VideoContext) pResourceManager, 
	//	int32_t nID, uint32_t nBufferSize, GRESOURCE_USAGE Usage, GRESOURCE_MODE ResourceMode, 
	//	GDATA_TYPE nElementType, uint32_t nElementCount, const void* pElementData, 
	//	GOSH(VideoBuffer)* ppCreated );
	//int32_t GHCreateVideoBuffer( GOSH(VideoContext) pResourceManager, 
	//	uint32_t nBufferSize, GRESOURCE_USAGE Usage, GRESOURCE_MODE ResourceMode, 
	//	GDATA_TYPE nElementType, uint32_t nElementCount, 
	//	const void* pElementData, GOSH(VideoBuffer)* ppCreated );

	//int32_t GHCreateVideoBuffer( GOSH(VideoContext) pResourceManager, int32_t nID, 
	//	GRESOURCE_MODE ResourceMode, _GBUFFER_DATA* pBufferData, GOSH(VideoBuffer)* ppCreated );
	//int32_t GHCreateVideoBuffer( GOSH(VideoContext) pResourceManager, GRESOURCE_MODE ResourceMode, 
	//	_GBUFFER_DATA* pBufferData, GOSH(VideoBuffer)* ppCreated );

	//int32_t GHCreateBuffer( GOSH(VideoContext) pResourceManager );
}; 