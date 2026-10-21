#include "GStorageWin32.h"
#include <Windows.h>
#include <Shlobj.h>
#include <crtdbg.h>

//#define GPROFILE_ENABLED
#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

CGStorageWin32* g_pStorage	= 0;

error_t _gcreateInterface( HMODULE hModule, god::GOSH(Storage)* ppInterface )
{
	GPROFILE_FUNCTION();
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	if( 0 == ppInterface || 0 == hModule )
	{
		error_printf("Parameter is NULL!");
		return G_INVALIDARGUMENT;
	}
	if( g_pStorage )
	{
		warning_printf( "Storage manager already created. Returning existent..." );
		(*ppInterface) = gacquireInterface( g_pStorage );
		return 0;
	}
	CGStorageWin32* pStorage = new CGStorageWin32(hModule);

	if( pStorage->init() )
	{
		error_printf( "Failed to initialize video context interface." );
		delete pStorage;
		return G_FAIL;
	};
	GHPTR(Storage) garbage = *ppInterface;
	*ppInterface = g_pStorage = pStorage;
	info_printf( "Storage context created successfully." );
	return S_OK;
};

void _greleaseInterface( god::GOSH(Storage)* pInterface )
{
	if( 0 == (*pInterface) )
		return;
	if( 0 == (*pInterface)->__nRefCount )
	{
		error_printf("Reference count error detected while releasing GOSH(Storage)!");
		_CrtDbgBreak();
		g_pStorage = 0;
		delete(*pInterface);
		return;
	}
	if(0 == --(*pInterface)->__nRefCount)
	{
		g_pStorage = 0;
		delete(*pInterface);
	}
	(*pInterface) = 0;
};

CGStorageWin32::CGStorageWin32( HMODULE hDllModule )
{
	GPROFILE_METHOD( "CGStorageWin32", "CGStorageWin32" );
	__nRefCount		= 1;
	__pfreeInterface	= greleaseInterface;
	m_hDllModule	= hDllModule;
	// should be set to null on init
	ClearPaths();
};

CGStorageWin32::~CGStorageWin32( void )
{
	GPROFILE_METHOD( "CGStorageWin32", "~CGStorageWin32" );
};

int32_t CGStorageWin32::init( void )
{
	GPROFILE_METHOD( "CGStorageWin32", "init" );
	
	ClearPaths();	// This sets the buffers to 0

	// TODO: bInitDefaultPaths should be set to false if saved settings are found
	bool bInitDefaultPaths = true;

	if( bInitDefaultPaths )
	{
		HRESULT hr;
		GetCurrentDirectoryW( 261, &m_wszDirectoryExecution[0] );
		memcpy( m_wszDirectoryResources, m_wszDirectoryExecution, sizeof(wchar_t)*(MAX_PATH+1) );
		wcscat_s( m_wszDirectoryResources, L"\\data" );


		wchar_t* buffer = 0;
		hr = SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, 0, &buffer );
		if( buffer )
			wcscpy_s( m_wszDirectorySettings, &buffer[0] ); 
		memcpy( &m_wszDirectoryMailbox[0],		&m_wszDirectorySettings[0], sizeof(wchar_t)*(MAX_PATH+1) ); 
		memcpy( &m_wszDirectorySecured[0],		&m_wszDirectorySettings[0], sizeof(wchar_t)*(MAX_PATH+1) ); 
		memcpy( &m_wszDirectoryAvatars[0],		&m_wszDirectorySettings[0], sizeof(wchar_t)*(MAX_PATH+1) ); 

		wcscat_s( m_wszDirectorySettings,	L"\\GOD\\Settings\\" );
		wcscat_s( m_wszDirectoryMailbox,	L"\\GOD\\Mailbox\\" );
		wcscat_s( m_wszDirectorySecured,	L"\\GOD\\Secured\\" );
		wcscat_s( m_wszDirectoryAvatars,	L"\\GOD\\Avatars\\" );

		// Get Windows saved games folder
		hr = SHGetKnownFolderPath( FOLDERID_SavedGames, 0, 0, &buffer );
		if( buffer )
			wcscpy_s( m_wszDirectorySavegame, buffer ); 
		wcscat_s( m_wszDirectorySavegame, L"\\GOD\\" );

		// Get Windows screenshots folder
		hr = SHGetKnownFolderPath( FOLDERID_Screenshots, 0, 0, &buffer );
		if( buffer )
			wcscpy_s( m_wszDirectoryScreenshots, buffer ); 
		else
		{
			hr = SHGetKnownFolderPath( FOLDERID_Pictures, 0, 0, &buffer );
			if( buffer )
				wcscpy_s( m_wszDirectoryScreenshots, buffer ); 
		}

	}
	else
	{
		error_printf("Not implemented!");
		throw("");
	}
	return 0;
}

error_t CGStorageWin32::GetDirectoryExecution( char* out_szPath, uint32_t* inout_nBufferSize )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryExecution" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::GetDirectorySettings( char* out_szPath, uint32_t* inout_nBufferSize )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetDirectorySettings" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::GetDirectoryResources( char* out_szPath, uint32_t* inout_nBufferSizeInWords )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryResources" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::GetDirectorySavegame( char* out_szPath, uint32_t* inout_nBufferSizeInWords )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetDirectorySavegame" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::GetDirectoryScreenshots( char* out_szPath, uint32_t* inout_nBufferSizeInWords )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryScreenshots" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::GetDirectoryMailbox( char* out_szPath, uint32_t* inout_nBufferSizeInWords )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryMailbox" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::GetDirectoryAvatars( char* out_szPath, uint32_t* inout_nBufferSizeInWords )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryAvatars" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::GetDirectorySecured( char* out_szPath, uint32_t* inout_nBufferSizeInWords )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetDirectorySecured" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::RelocateDirectoryExecution( const char* in_szPath, uint32_t in_nCharCount, bool bMoveContents )
{
	GPROFILE_METHOD( "CGStorageWin32", "RelocateDirectoryExecution" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::RelocateDirectorySettings( const char* in_szPath, uint32_t in_nCharCount, bool bMoveContents )
{
	GPROFILE_METHOD( "CGStorageWin32", "RelocateDirectorySettings" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::RelocateDirectoryResources( const char* in_szPath, uint32_t in_nCharCount, bool bMoveContents )
{
	GPROFILE_METHOD( "CGStorageWin32", "RelocateDirectoryResources" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::RelocateDirectorySavegame( const char* in_szPath, uint32_t in_nCharCount, bool bMoveContents )
{
	GPROFILE_METHOD( "CGStorageWin32", "RelocateDirectorySavegame" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::RelocateDirectoryScreenshots( const char* in_szPath, uint32_t in_nCharCount, bool bMoveContents )
{
	GPROFILE_METHOD( "CGStorageWin32", "RelocateDirectoryScreenshots" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::RelocateDirectoryMailbox( const char* in_szPath, uint32_t in_nCharCount, bool bMoveContents )
{
	GPROFILE_METHOD( "CGStorageWin32", "RelocateDirectoryMailbox" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::RelocateDirectoryAvatars( const char* in_szPath, uint32_t in_nCharCount, bool bMoveContents )
{
	GPROFILE_METHOD( "CGStorageWin32", "RelocateDirectoryAvatars" );
	error_printf("Not implemented!");
	return G_FAIL;
};

error_t CGStorageWin32::RelocateDirectorySecured( const char* in_szPath, uint32_t in_nCharCount, bool bMoveContents )
{
	GPROFILE_METHOD( "CGStorageWin32", "RelocateDirectorySecured" );
	error_printf("Not implemented!");
	return G_FAIL;
};

const char* CGStorageWin32::GetDirectoryExecution	( void ) { GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryExecution" )		; return m_szDirectoryExecution		; };
const char* CGStorageWin32::GetDirectorySettings	( void ) { GPROFILE_METHOD( "CGStorageWin32", "GetDirectorySettings" )		; return m_szDirectorySettings		; };
const char* CGStorageWin32::GetDirectoryResources	( void ) { GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryResources" )		; return m_szDirectoryResources		; };
const char* CGStorageWin32::GetDirectorySavegame	( void ) { GPROFILE_METHOD( "CGStorageWin32", "GetDirectorySavegame" )		; return m_szDirectorySavegame		; };
const char* CGStorageWin32::GetDirectoryScreenshots	( void ) { GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryScreenshots" )	; return m_szDirectoryScreenshots	; };
const char* CGStorageWin32::GetDirectoryMailbox		( void ) { GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryMailbox" )		; return m_szDirectoryMailbox		; };
const char* CGStorageWin32::GetDirectoryAvatars		( void ) { GPROFILE_METHOD( "CGStorageWin32", "GetDirectoryAvatars" )		; return m_szDirectoryAvatars		; };
const char* CGStorageWin32::GetDirectorySecured		( void ) { GPROFILE_METHOD( "CGStorageWin32", "GetDirectorySecured" )		; return m_szDirectorySecured		; };


void* CGStorageWin32::GetInterfacePointer( GINTERFACEID_TYPE InterfaceID )
{
	GPROFILE_METHOD( "CGStorageWin32", "GetInterfacePointer" );
	switch( InterfaceID )
	{
	case GIID_CUSTOMINTERFACE:
		return (CGStorageWin32*)this;
	case GIID_STORAGEMANAGER:
		return (GOSH(Storage))this;
	}
	error_printf("Invalid interface requested! 0x%I64X", (uint64_t)InterfaceID );
	return 0;
};

id_t CGStorageWin32::GetID( void ) const
{
	GPROFILE_METHOD( "CGStorageWin32", "GetID" );
	error_printf("Not implemented.");
	return 0;
};

void CGStorageWin32::ClearPaths( void )
{
	memset( &m_wszDirectoryExecution	[0], 0, sizeof(wchar_t)*(MAX_PATH+1) );
	memset( &m_wszDirectorySettings		[0], 0, sizeof(wchar_t)*(MAX_PATH+1) );
	memset( &m_wszDirectorySavegame		[0], 0, sizeof(wchar_t)*(MAX_PATH+1) );
	memset( &m_wszDirectoryMailbox		[0], 0, sizeof(wchar_t)*(MAX_PATH+1) );

	memset( &m_wszDirectoryResources	[0], 0, sizeof(wchar_t)*(MAX_PATH+1) );	
	memset( &m_wszDirectoryScreenshots	[0], 0, sizeof(wchar_t)*(MAX_PATH+1) );
	memset( &m_wszDirectoryAvatars		[0], 0, sizeof(wchar_t)*(MAX_PATH+1) );	
	memset( &m_wszDirectorySecured		[0], 0, sizeof(wchar_t)*(MAX_PATH+1) );

	memset( &m_szDirectoryExecution		[0], 0, sizeof(char)*(MAX_PATH+1) );
	memset( &m_szDirectorySettings		[0], 0, sizeof(char)*(MAX_PATH+1) );
	memset( &m_szDirectorySavegame		[0], 0, sizeof(char)*(MAX_PATH+1) );
	memset( &m_szDirectoryMailbox		[0], 0, sizeof(char)*(MAX_PATH+1) );

	memset( &m_szDirectoryResources		[0], 0, sizeof(char)*(MAX_PATH+1) );	
	memset( &m_szDirectoryScreenshots	[0], 0, sizeof(char)*(MAX_PATH+1) );
	memset( &m_szDirectoryAvatars		[0], 0, sizeof(char)*(MAX_PATH+1) );	
	memset( &m_szDirectorySecured		[0], 0, sizeof(char)*(MAX_PATH+1) );
};
