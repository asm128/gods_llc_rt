#include "GWS3Manager.h"
#include "GWS3.h"

//#define GPROFILE_ENABLED
#include "GDebugger_core.h"

_CGWS3NetworkManager::_CGWS3NetworkManager( _CGWS3* pContext )
{
	GPROFILE_METHOD( "_CGWS3Device", "_CGWS3Device" );
	__nRefCount		= 1;
	__pfreeInterface	= greleaseInterface;
	m_pContext		= pContext;
	m_hMainWnd		= 0;
}

_CGWS3NetworkManager::~_CGWS3NetworkManager( void )
{
	GPROFILE_METHOD( "_CGWS3Device", "~_CGWS3Device" );
	_Cleanup();
	m_pContext	= NULL;
};

void _CGWS3NetworkManager::_Cleanup( void )
{
	GPROFILE_METHOD( "_CGWS3Device", "_Cleanup" );
};

error_t _CGWS3NetworkManager::internalInit( HWND hWnd, id_t nID )
{
	if( 0 == hWnd )
	{
		error_printf("HWND is NULL!");
		return -1;
	}
	m_hMainWnd	= hWnd; // window handle
	m_nID		= nID;
	return 0;
}

error_t _CGWS3NetworkManager::CreateConnection( GCOMMUNICATION_MODE nMode, USHORT Port, CHAR *IP, uint32_t nMaxBufferSize, GOSH(NetworkConnection)* out_ppCreated )
{
	return 0;
};

error_t _CGWS3NetworkManager::Update( float fElapsedTime )
{
	//for( uint32_t iConn=0; iConn<m_n
	return 0;
}

error_t _CGWS3NetworkManager::PushBuffer( GOSH(NetworkConnection) pConnection, GRDS(SBuffer) in_Buffer )
{
	GPROFILE_METHOD( "_CGWS3Device", "GetNextBuffer" );
	return pConnection->PushBuffer(in_Buffer);
};

GRDS(SBuffer) _CGWS3NetworkManager::PopBuffer( GOSH(NetworkConnection) pConnection )
{
	GPROFILE_METHOD( "_CGWS3Device", "GetNextBuffer" );
	return pConnection->PopBuffer();
};
//-----------------------------
// information about the inbox
//
uint32_t _CGWS3NetworkManager::GetBufferCount(GOSH(NetworkConnection) stuff ) const
{
	GPROFILE_METHOD( "_CGWS3Device", "GetBufferCount" );
	return stuff ->GetBufferCount();
};
//
bool _CGWS3NetworkManager::IsConnected(GOSH(NetworkConnection) stuff ) const
{
	GPROFILE_METHOD( "_CGWS3Device", "IsConnected" );
	return stuff ->IsConnected();
};

void* _CGWS3NetworkManager::GetInterfacePointer( GINTERFACEID_TYPE InterfaceType )
{ 
	GPROFILE_METHOD( "_CGWS3", "GetInterfacePointer" );
	switch( InterfaceType )
	{
	case GIID_RESERVED:
	case GIID_CUSTOMINTERFACE:
		return (_CGWS3*)this;
	case GIID_NETWORKMANAGER:
		return (GOSH(NetworkManager))this;
	default:
		return 0;
	}
}



