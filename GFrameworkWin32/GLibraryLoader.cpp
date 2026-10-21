






/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2009-2013: Pablo Ariel Zorrilla Cepeda
#include "GLibraryLoader.h"

#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

CGLibraryLoader::CGLibraryLoader( void )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	GPROFILE_METHOD( "CGLibraryLoader", "CGLibraryLoader" );
	// Initialize member pointers
	m_pVideoContext			= NULL;
	m_pInputContext			= NULL;
	m_pNetworkContext		= NULL;
	m_pPPUContext		= NULL;
	m_pGameContext			= NULL;
	m_pAudioContext			= NULL;

	//
	m_hVideoDLL				= NULL;
	m_hInputDLL				= NULL;
	m_hStorageDLL			= NULL;
	m_hPPUDLL			= NULL;
	m_hNetworkDLL			= NULL;
	m_hGameDLL				= NULL;
	m_hAudioDLL				= NULL;

	_pCreateGVideo			= NULL;
	_pReleaseGVideo			= NULL;

	_pCreateGInput			= NULL;
	_pReleaseGInput			= NULL;

	_pCreateGNetwork		= NULL;
	_pReleaseGNetwork		= NULL;

	_pCreateGAudio			= NULL;
	_pReleaseGAudio			= NULL;

	_pCreateGPPU		= NULL;
	_pReleaseGPPU		= NULL;

	_pCreateGGame			= NULL;
	_pReleaseGGame			= NULL;

	_pCreateStorage			= NULL;
	_pFreeStorage			= NULL;

};

CGLibraryLoader::~CGLibraryLoader( void )
{
	//GPROFILE_METHOD( "CGLibraryLoader", "~CGLibraryLoader" ); This call may lead to leaks caused by the static initialization order fiasco
	FreeLibrary( m_hGameDLL			);
	FreeLibrary( m_hVideoDLL		);
	FreeLibrary( m_hInputDLL		);
	FreeLibrary( m_hPPUDLL	);
	FreeLibrary( m_hAudioDLL		);
	FreeLibrary( m_hNetworkDLL		);
	FreeLibrary( m_hStorageDLL		);
};

// This function loads the video dll module and stores its handle and exported procedures, then creates the main context object which will provide us with the available devices 
int32_t CGLibraryLoader::CreateVideoContext( const wchar_t *wszDllName, IGVideo **ppContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "CreateVideoContext" );
	if( 0 == m_hVideoDLL )
	{
		m_hVideoDLL = LoadLibraryW( wszDllName );
		if( !m_hVideoDLL ) 
		{
			 MessageBoxW( NULL, L"DLL load failed.", wszDllName, MB_OK | MB_ICONERROR );
			 return G_FAIL; // Cannot load dll
		}
	}

	if ( !_pCreateGVideo ) 
	{
		// function pointer to dll's 'CreateRenderDevice' function
		_pCreateGVideo = ( CREATEGVIDEO )
							GetProcAddress( m_hVideoDLL, "CreateGVideo" );
		if ( !_pCreateGVideo )
		{
			error_wprintf(L"Cannot find CreateGVideo() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}
	if( !_pReleaseGVideo )
	{
		_pReleaseGVideo = ( RELEASEGVIDEO )
							GetProcAddress( m_hVideoDLL, "ReleaseGVideo" );
		if( !_pReleaseGVideo )
		{
			error_wprintf(L"Cannot find ReleaseGVideo() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}

	if( _pCreateGVideo( m_hVideoDLL, &m_pVideoContext ) )
	{
		error_printf("Failed to create video context.");
		return G_FAIL;
	}
	*ppContext = m_pVideoContext;			// Set the pointer to member var 
	return 0;
}

int32_t CGLibraryLoader::DestroyVideoContext( IGVideo* pContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "DestroyVideoContext" );
	if( !pContext )
		return E_FAIL;
	HRESULT hr = _pReleaseGVideo( pContext );

	if( hr != 0 )
		MessageBox( NULL, L"Unknown Error!", L"CGVideo", MB_OK | MB_ICONWARNING );

	if( pContext == m_pVideoContext && pContext )
	{
		m_pVideoContext	= NULL;
		_pCreateGVideo	= NULL;
		_pReleaseGVideo	= NULL;
		return 0;
	}

	error_printf("Invalid address received as parameter for DestroyVideoContext().\nThis could indicate a heap corruption or unproper handling of the object pointer.");
	return G_FAIL;
}

int32_t CGLibraryLoader::CreateInputContext( const wchar_t *wszDllName, IGInput **ppContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "CreateInputContext" );
	if( 0 == wszDllName )
	{
		error_printf("Library name not specified for input device drivers.");
		return G_FAIL;
	}

	if( 0 == m_hInputDLL )
	{
		m_hInputDLL = LoadLibraryW( wszDllName );
		if( !m_hInputDLL ) 
		{
			error_wprintf( L"Failed to load input device drivers: %s.", wszDllName );
			MessageBoxW( NULL, wszDllName, L"DLL load failed.", MB_OK | MB_ICONERROR );
			return G_FAIL; // Cannot load dll
		}
	}

	if ( !_pCreateGInput ) 
	{
		// function pointer to dll's 'CreateRenderDevice' function
		_pCreateGInput = ( CREATEGINPUT )
							GetProcAddress( m_hInputDLL, "CreateGInput" );
		if ( !_pCreateGInput )
		{
			error_wprintf(L"Cannot find CreateGInput() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}
	if( !_pReleaseGInput )
	{
		_pReleaseGInput = ( RELEASEGINPUT )
							GetProcAddress( m_hInputDLL, "ReleaseGInput" );
		if( !_pReleaseGInput )
		{
			error_wprintf(L"Cannot find ReleaseGInput() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}

	if( _pCreateGInput( &m_pInputContext ) )
	{
		error_printf("Cannot create input device context.");
		return E_FAIL;
	}
	*ppContext = m_pInputContext;			// Set the pointer to member var 
	return 0;
}

int32_t CGLibraryLoader::DestroyInputContext( IGInput* pContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "DestroyInputContext" );
	if( !pContext )
		return E_FAIL;
	HRESULT hr =_pReleaseGInput( pContext );

	if( hr != 0 )
		MessageBox( NULL, L"Unknown Error!", L"GInput", MB_OK | MB_ICONWARNING );

	if( pContext == m_pInputContext && pContext )
	{
		m_pInputContext	= NULL;
		_pCreateGInput	= NULL;
		_pReleaseGInput	= NULL;
		return 0;
	}
	error_printf("Invalid address received as parameter for DestroyInputContext().\nThis could indicate a heap corruption or unproper handling of the object pointer.");
	return G_FAIL;
}


int32_t CGLibraryLoader::CreateAudioContext( const wchar_t *wszDllName, IGAudio **ppContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "CreateAudioContext" );
	if( 0 == wszDllName )
	{
		error_printf("Library name not specified for audio dll.");
		return G_FAIL;
	}

	if( 0 == m_hAudioDLL )
	{
		m_hAudioDLL = LoadLibraryW( wszDllName );
		if( !m_hAudioDLL ) 
		{
			error_wprintf( L"Failed to load audio dll: %s.", wszDllName );
			MessageBoxW( NULL, L"DLL load failed.", wszDllName, MB_OK | MB_ICONERROR );
			return G_FAIL; // Cannot load dll
		}
	}
	if ( !_pCreateGAudio ) 
	{
		// function pointer to dll's 'CreateRenderDevice' function
		_pCreateGAudio = ( CREATEGAUDIO )
							GetProcAddress( m_hAudioDLL, "CreateGAudio" );
		if ( !_pCreateGAudio )
		{
			error_wprintf(L"Cannot find CreateGAudio() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}
	if( !_pReleaseGAudio )
	{
		_pReleaseGAudio = ( RELEASEGAUDIO )
							GetProcAddress( m_hAudioDLL, "ReleaseGAudio" );
		if( !_pReleaseGAudio )
		{
			error_wprintf(L"Cannot find ReleaseGAudio() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}

	if( _pCreateGAudio( m_hAudioDLL, &m_pAudioContext ) )
	{
		error_printf("Cannot create audio context\n.");
		return G_FAIL;
	}
	*ppContext = m_pAudioContext;			// Set the pointer to member var 
	return 0;
}

int32_t CGLibraryLoader::DestroyAudioContext( IGAudio* pContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "DestroyAudioContext" );
	if( !pContext )
		return E_FAIL;
	HRESULT hr = _pReleaseGAudio( pContext );

	if( hr != 0 )
		MessageBox( NULL, L"Unknown Error!", L"GMultiAudio", MB_OK | MB_ICONWARNING );

	if( pContext && pContext == m_pAudioContext )
	{
		m_pAudioContext		= NULL;
		_pCreateGAudio		= NULL;
		_pReleaseGAudio		= NULL;
		return 0;
	}
	error_printf("Invalid address received as parameter for DestroyAudioContext().\nThis could indicate a heap corruption or unproper handling of the object pointer.");
	return G_FAIL;
}



int32_t CGLibraryLoader::CreateNetworkContext( const wchar_t *wszDllName, IGNetwork **ppContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "CreateNetworkContext" );
	if( 0 == wszDllName )
	{
		error_printf("Dynamic link library name not specified for network context.");
		return G_FAIL;
	}

	if( 0 == m_hNetworkDLL )
	{
		m_hNetworkDLL = LoadLibraryW( wszDllName );
		if( !m_hNetworkDLL ) 
		{
			MessageBoxW( NULL, wszDllName, L"DLL load failed.", MB_OK | MB_ICONERROR );
			error_wprintf( L"Cannot load network dll: %s.", wszDllName );
			return G_FAIL; // Cannot load dll
		}
	}
	if ( !_pCreateGNetwork ) 
	{
		// function pointer to dll's 'CreateRenderDevice' function
		_pCreateGNetwork = ( CREATEGNETWORK )
							GetProcAddress( m_hNetworkDLL, "CreateGNetwork" );
		if ( !_pCreateGNetwork )
		{
			error_wprintf(L"Cannot find CreateGNetwork() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}
	if( !_pReleaseGNetwork )
	{
		_pReleaseGNetwork = ( RELEASEGNETWORK )
							GetProcAddress( m_hNetworkDLL, "ReleaseGNetwork" );
		if( !_pReleaseGNetwork )
		{
			error_wprintf(L"Cannot find ReleaseGNetwork() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}

	if( _pCreateGNetwork( m_hNetworkDLL, ppContext ) )
	{
		error_printf( "Cannot create network context." );
		return G_FAIL;
	}
	m_pNetworkContext = *ppContext;
	return 0;
}

int32_t CGLibraryLoader::DestroyNetworkContext( IGNetwork* pContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "DestroyNetworkContext" );
	if( !pContext )
		return E_FAIL;
	HRESULT hr = _pReleaseGNetwork( pContext );

	if( hr != 0 )
		MessageBox( NULL, L"Unknown Error!", L"GNetwork", MB_OK | MB_ICONWARNING );

	if( pContext && pContext == m_pNetworkContext )
	{
		m_pNetworkContext		= NULL;
		_pCreateGNetwork		= NULL;
		_pReleaseGNetwork		= NULL;
		return 0;
	}

	error_printf("Invalid address received as parameter for DestroyNetworkContext().\nThis could indicate a heap corruption or unproper handling of the object pointer.");
	return G_FAIL;
}

int32_t CGLibraryLoader::CreatePPUContext( const wchar_t *wszDllName, IGPPU **ppContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "CreatePPUContext" );
	if( 0 == wszDllName )
	{
		error_printf("Dynamic link library name not specified for parallel computing context.");
		return G_FAIL;
	}

	if( 0 == m_hPPUDLL )
	{
		m_hPPUDLL = LoadLibraryW( wszDllName );
		if( !m_hPPUDLL ) 
		{
			MessageBoxW( NULL, wszDllName, L"DLL load failed.", MB_OK | MB_ICONERROR );
			error_wprintf(L"Cannot load dll: %s", wszDllName);
			return G_FAIL; // Cannot load dll
		}
	}

	if ( !_pCreateGPPU ) 
	{
		// function pointer to dll's 'CreateRenderDevice' function
		_pCreateGPPU = ( CREATEGPARALLELC )
							GetProcAddress( m_hPPUDLL, "CreateGPPU" );
		if ( !_pCreateGPPU )
		{
			error_wprintf(L"Cannot find CreateGPPU() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}
	if( !_pReleaseGPPU )
	{
		_pReleaseGPPU = ( RELEASEGPARALLELC )
							GetProcAddress( m_hPPUDLL, "ReleaseGPPU" );
		if( !_pReleaseGPPU )
		{
			error_wprintf(L"Cannot find ReleaseGPPU() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}


	if( _pCreateGPPU( m_hPPUDLL, ppContext ) )
	{
		error_printf("Cannot load parallel computing library.");
		return G_FAIL; // failed?
	}
	m_pPPUContext = *ppContext;			// Set the pointer to member var 
	return 0;
}

int32_t CGLibraryLoader::DestroyPPUContext( IGPPU* pContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "DestroyPPUContext" );
	if( !pContext )
		return E_FAIL;
	HRESULT hr = _pReleaseGPPU( pContext );

	if( hr != 0 )
		MessageBox( NULL, L"Unknown Error!", L"CGPPU", MB_OK | MB_ICONWARNING );

	if( pContext && pContext == m_pPPUContext )
	{
		m_pPPUContext 	= NULL;
		_pCreateGPPU		= NULL;
		_pReleaseGPPU		= NULL;
		return 0;
	}

	error_printf("Invalid address received as parameter for DestroyPPUContext().\nThis could indicate a heap corruption or unproper handling of the object pointer.");
	return G_FAIL;
}


int32_t CGLibraryLoader::CreateGameContext( const wchar_t *wszDllName, IGGame **ppContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "CreateGameContext" );
	if( 0 == wszDllName )
	{
		error_printf("Dynamic link library name not specified for parallel computing context.");
		return G_FAIL;
	}

	if( 0 == m_hGameDLL )
	{
		m_hGameDLL = LoadLibraryW( wszDllName );
		if( !m_hGameDLL ) 
		{
			MessageBoxW( NULL, wszDllName, L"DLL load failed.", MB_OK | MB_ICONERROR );
			error_wprintf(L"Cannot load dll: %s", wszDllName);
			return G_FAIL; // Cannot load dll
		}
	}

	if ( !_pCreateGGame ) 
	{
		// function pointer to dll's 'CreateRenderDevice' function
		_pCreateGGame = ( CREATEGGAME )
							GetProcAddress( m_hGameDLL, "CreateGGame" );
		if ( !_pCreateGGame )
		{
			error_wprintf(L"Cannot find CreateGGame() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}
	if( !_pReleaseGGame )
	{
		_pReleaseGGame = ( RELEASEGGAME )
							GetProcAddress( m_hGameDLL, "ReleaseGGame" );
		if( !_pReleaseGGame )
		{
			error_wprintf(L"Cannot find ReleaseGGame() exported function in %s.", wszDllName);
			return G_FAIL; 
		}
	}

	if( _pCreateGGame( m_hGameDLL, ppContext ) )
	{
		error_printf("Cannot load game dll.");
		return G_FAIL; // failed?
	}
	m_pGameContext = *ppContext;			// Set the pointer to member var 
	return 0;
}

int32_t CGLibraryLoader::DestroyGameContext( IGGame* pContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "DestroyGameContext" );
	if( !pContext )
		return E_FAIL;
	HRESULT hr = _pReleaseGGame( pContext );

	if( hr != 0 )
		MessageBox( NULL, L"Unknown Error!", L"CGGame", MB_OK | MB_ICONWARNING );

	if( pContext && pContext == m_pGameContext )
	{
		m_pGameContext 	= NULL;
		_pCreateGGame		= NULL;
		_pReleaseGGame		= NULL;
		return 0;
	}

	error_printf("Invalid address received as parameter for DestroyGameContext().\nThis could indicate a heap corruption or unproper handling of the object pointer.");
	return G_FAIL;
}


int32_t CGLibraryLoader::CreateStorage( const wchar_t *wszDllName, GOSH(Storage)*ppContext )
{
	GPROFILE_METHOD( "CGLibraryLoader", "CreateStorage" );
	if( 0 == wszDllName )
	{
		error_printf("Dynamic link library name not specified for parallel computing context.");
		return G_FAIL;
	}

	if( 0 == m_hStorageDLL )
	{
		m_hStorageDLL = LoadLibraryW( wszDllName );
		if( !m_hStorageDLL ) 
		{
			MessageBoxW( NULL, wszDllName, L"DLL load failed.", MB_OK | MB_ICONERROR );
			error_wprintf(L"Cannot load dll: %s", wszDllName);
			return G_FAIL; // Cannot load dll
		}
	}

	if ( !_pCreateStorage ) 
	{
		// function pointer to dll's 'CreateRenderDevice' function
		_pCreateStorage = ( GCREATEINTERFACE_STORAGE )
							GetProcAddress( m_hStorageDLL, "_gcreateInterface" );
		if ( !_pCreateStorage )
		{
			error_wprintf(L"Cannot find _gcreateInterface() function in %s.", wszDllName);
			return G_FAIL; 
		}
	}
	if( !_pFreeStorage )
	{
		_pFreeStorage = ( GFREEINTERFACE_STORAGE )
							GetProcAddress( m_hStorageDLL, "_greleaseInterface" );
		if( !_pFreeStorage )
		{
			error_wprintf(L"Cannot find _greleaseInterface() function in %s.", wszDllName);
			return G_FAIL; 
		}
	}

	if( _pCreateStorage( m_hStorageDLL, ppContext ) )
	{
		error_printf("Cannot load storage dll.");
		return G_FAIL; // failed?
	}
	return 0;
}
//
//int32_t CGLibraryLoader::DestroyStorage( IStprage* pContext )
//{
//	GPROFILE_METHOD( "CGLibraryLoader", "DestroyStorage" );
//	if( !pContext )
//		return E_FAIL;
//	HRESULT hr = _pReleaseStprage( pContext );
//
//	if( hr != 0 )
//		MessageBox( NULL, L"Unknown Error!", L"CStprage", MB_OK | MB_ICONWARNING );
//
//	if( pContext && pContext == m_pStorage )
//	{
//		m_pStorage 	= NULL;
//		_pCreateStprage		= NULL;
//		_pReleaseStprage		= NULL;
//		return 0;
//	}
//
//	error_printf("Invalid address received as parameter for DestroyStorage().\nThis could indicate a heap corruption or unproper handling of the object pointer.");
//	return G_FAIL;
//}