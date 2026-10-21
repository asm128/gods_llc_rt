// File:D3D9ResMgr_effect.cpp
//
// Copyright © 2011: Pablo Ariel Zorrilla Cepeda
// 
// Any use or distribution of this code is strictly prohibited
//
//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include <strsafe.h>

#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

void _CGD3D9ResourceManager::__freeProgramList( void )
{															
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__freeProgramList" );
	GHPTR(VideoProgram)	garbage=0;
	CGD3D9Program* pgarbage;
	while( m_lstProgram.size() )
	{
		m_lstProgram.pop(&pgarbage);
		garbage = pgarbage;
	}	
}
__GDEFINE_GET_GINTERFACE_BY_ID_GELEMENTARY( VideoProgram, Program );

// Monster Shaders	  
int32_t _CGD3D9ResourceManager::CreateProgramFromFile( int32_t nID, const wchar_t* pFilename, GOSH(VideoProgram)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateProgramFromFile" );
	if(0 == pFilename)
	{
		error_printf("CreateProgramFromFile() FAILED trying to create Video Program from NULL filename. The ID requested was %i.", nID );
		return G_INVALIDARGUMENT;
	}
	if( nID <= 0 )
	{
		error_wprintf(L"CreateProgramFromFile() FAILED trying to create Video Program with invalid ID (<=0). The ID requested was %i. The file asked for was: %s", nID, pFilename );
		return G_INVALIDID;
	}

	CGD3D9Program* pEffect=0;

	GHPTR(VideoProgram) garbage;
	if( nID )
	{
		if(0 != (garbage = pEffect = (CGD3D9Program*)this->GetProgram( nID ) ) )
		{// can't create duplicated ID
			error_wprintf(L"CreateProgramFromFile() FAILED trying to create Video Program with duplicated ID! The ID requested was %i. The file asked for was: %s", nID, pFilename );
			return G_INVALIDID;
		}
	}
	else
		nID = getAvailableID( GIID_DEVICEPROGRAM );

	if( 0 == pEffect )
	{
		pEffect = new CGD3D9Program( this );
		if( pEffect->internalInit( nID, pFilename ) )
		{
			error_wprintf( L"CGD3D9ResourceManager FAILED to initialize CGD3D9Program instance for video program with ID=%i and filename=%s", nID, pFilename );
			delete pEffect;
			return G_FAIL;
		}
		debug_wprintf( L"CGD3D9Program instance initialized successfully! for program with ID=%i from filename=%s", nID, pFilename);
		if( 0 > m_lstProgram.push( pEffect ) )
		{
			garbage = pEffect;
			error_printf("Failed to create video program.");
			return -1;
		};
	}
	else
	{
		if( pEffect->internalInit( nID, pFilename ) )
		{
			error_wprintf( L"CGD3D9ResourceManager FAILED to reinitialize CGD3D9Program instance for video program with ID=%i and filename=%s", nID, pFilename );
			return G_FAIL;
		}
	}
	debug_wprintf(L"Video program created successfully from filename=%s", pFilename);
	if( ppCreated )
	{
		garbage = *ppCreated;
		*ppCreated = gacquireInterface( pEffect );
	}
	return S_OK;
};

// 
int32_t _CGD3D9ResourceManager::CreateProgramFromFile( const wchar_t* pFilename, GOSH(VideoProgram)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateProgramFromFile" );
	if( !ppCreated ) 
	{
		if(pFilename)
		{
			error_wprintf(L"FAILED to create video program from filename \"%s\". Output pointer is NULL.", pFilename );
		}
		else
		{
			error_printf("FAILED to create video program with automatic ID. Filename and output pointer are NULL." );
		}
		return G_INVALIDARGUMENT;
	}

	if( 0 == pFilename )
	{
		error_printf( "FAILED to create video program with automatic ID. Input filename is NULL." );
		return G_INVALIDARGUMENT;
	}

	CGD3D9Program* pEffect = new CGD3D9Program( this );
	if( pEffect->internalInit( getAvailableID( GIID_DEVICEPROGRAM ), pFilename ) )
	{
		error_wprintf( L"Cannot initialize CGD3D9Program instance for video program with filename=%s", pFilename );
		delete pEffect;
		return G_FAIL;
	}
	if( 0 > m_lstProgram.push( pEffect ) )
	{
		delete( pEffect );
		error_printf("Out of memory?");
		return -1;
	};

	debug_wprintf(L"Video program created successfully from filename=%s", pFilename);
	GHPTR(VideoProgram) old = *ppCreated;
	*ppCreated = gacquireInterface( pEffect );
	return S_OK;
};

error_t _CGD3D9ResourceManager::CreateProgramFromFile( int32_t ID, const char* pFilename, uint32_t nCodePage, GOSH(VideoProgram)* ppCreated )
{
	wchar_t buffer[300]= {0};
	MultiByteToWideChar( nCodePage, MB_COMPOSITE, pFilename, strlen(pFilename), buffer, 300 );
	return this->CreateProgramFromFile( ID, buffer, ppCreated );
	return 0;
};

error_t _CGD3D9ResourceManager::CreateProgramFromFile( const char* pFilename, uint32_t nCodePage, GOSH(VideoProgram)* ppCreated )
{
	wchar_t buffer[300]= {0};
	MultiByteToWideChar( nCodePage, MB_COMPOSITE, pFilename, strlen(pFilename), buffer, 300 );
	return this->CreateProgramFromFile( buffer, ppCreated );
	return 0;
};

// ------------------ 
GRDS(SEffect) _CGD3D9ResourceManager::getEffectItem( GRDS(SEffect) pEffectItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getEffectItem" );
	if( 0 == pEffectItem )
		return 0;

#ifdef _WIN32 // crap to compare case-insensitive names (waste of cycles)
	wchar_t buffer[256], buffer2[256]; 
	if( gget(pEffectItem)->FileName )
		StringCchCopyW( buffer, 256, gget(gget(pEffectItem)->FileName)->WText );
	else
		buffer[0] = 0;
	_wcslwr_s( buffer );
#endif

	GRDS(SEffect) pNewItem = 0; // store here the return value
	uint32_t i, n = m_lstManagerXProgram.size(); // set some counters
	for( i=0; i<n; i++ )
	{
		pNewItem = m_lstManagerXProgram.get_pointer()[i];
		if( 0 == pNewItem )
			continue;
#ifdef _WIN32
		if( gget(pNewItem)->FileName )
			StringCchCopyW( buffer2, 256, gget(gget(pNewItem)->FileName)->WText );
		else
		{
			buffer2[0] = L'?'; buffer2[1] = 0;
		}
		_wcslwr_s( buffer2 );
		if( 0 == wcscmp( buffer2, buffer ) )
		{
#else
		if( 0 == wcscmp( pNewItem->pFileData->wszName, pEffectItem->pFileData->wszName ) )
		{
#endif
			info_printf("Found already loaded effect with same name. Attempt to recycle...");
			printInfoString( gget(pNewItem)->FileName );
			break;
		}
		else
		{
			pNewItem = 0;
		}
	}
	if( 0 == pNewItem )
	{
		if( __createDXXProgram( gget(pEffectItem) ) )
		{ // don't acquire if failed to create
			error_printf("__createDXProgram() FAILED!");
			return 0;
		}
		info_printf("__createDXProgram() succeeded!");
		if( m_lstManagerXProgram.push( pEffectItem ) )
		{ // don't acquire if failed to create
			error_printf("_CGD3D9ResourceManager::__appendProgramItem() FAILED!");
		}
		pNewItem = pEffectItem;
	}
	else
	{
	}
	info_printf("Succeeded creating/retrieving managed data for video program.");
	
	return gacquire( pNewItem );
};
//
int32_t _CGD3D9ResourceManager::__createDXXProgram( SEffect* pEffectData )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXProgram" );

	DWORD dwFlags = 0;
	ID3DXBuffer* pd3dxBuffer=0;
	D3DXEFFECT_DESC* pDesc = &pEffectData->EffectDesc;
#if ( defined( _DEBUG ) ||  defined( DEBUG ) )
	dwFlags = D3DXSHADER_DEBUG;
#else
	dwFlags = D3DXSHADER_OPTIMIZATION_LEVEL3;
#endif
	HRESULT hr = D3DXCreateEffectFromFileW( m_pD3D9Device, 
		gget(pEffectData->FileName)->WText, 0, 0, dwFlags, 0, 
		&pEffectData->pd3dXProgram, &pd3dxBuffer );
	if( D3D_OK != hr )
	{
		if( 0 != pd3dxBuffer )
		{
			error_printf( "Failed to load effect file: \"%s\"", gget(pEffectData->FileName)->Text );
			error_printf( "D3DXCreateEffectFromFileW() returned HRESULT=0x%X", (uint32_t)hr );
			MessageBoxA( NULL, (char*)pd3dxBuffer->GetBufferPointer(), "Failed to load effect file", MB_ICONERROR ); 
			error_printf( "%s", (char*)pd3dxBuffer->GetBufferPointer() );
			//error_printf( "Reasons:\n%s", (char*)pd3dxBuffer->GetBufferPointer() );
			pd3dxBuffer->Release();
		}
		else
		{
			error_printf( "Failed to load effect file: %s. D3DXCreateEffectFromFileW() returned HRESULT=0x%X", gget(pEffectData->FileName)->Text, (uint32_t)hr );
			MessageBoxW( NULL, gget(pEffectData->FileName)->WText, L"Failed to load effect file", MB_ICONERROR ); 
		}
		return G_FAIL;
	}
	if( 0 != pd3dxBuffer )
		pd3dxBuffer->Release();

	info_printf("Succeeded loading d3d effect from file.");
	pEffectData->pd3dXProgram->GetDesc( pDesc );
	return G_OK;
}

