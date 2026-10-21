#include "GWS3.h"

//#define GPROFILE_ENABLED
#include "GDebugger_core.h"

int32_t CreateGNetwork( HMODULE hModule, IGNetwork** ppInterface )
{
	GPROFILE_FUNCTION();
	CGWS3* pNetwork = new CGWS3(hModule);
	if( pNetwork->Init() )
	{
		delete pNetwork;
		return G_FAIL;
	};
	*ppInterface = pNetwork;
	return S_OK;
};

INT32 ReleaseGNetwork( IGNetwork *pInterface )
{
	GPROFILE_FUNCTION();
	if( 0 == pInterface )
	{
		error_printf("Parameter is NULL!");
		return -1;
	}
	delete pInterface;
	return S_OK;
#ifdef _DEBUG
	MessageBox( NULL, L"The object returned a non-zero reference count "
				L"meaning that some objects were not properly released.",
				L"GWS3", MB_OK | MB_ICONWARNING );
#endif
	return G_FAIL;
};

_CGWS3::_CGWS3( HMODULE hModule )
{
	GPROFILE_METHOD( "_CGWS3", "_CGWS3" );
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	m_hModule		= hModule;

	m_nDeviceCount = m_nDeviceArraySize = 0;
	m_pDeviceList = 0;
};

_CGWS3::~_CGWS3( void )
{
	GPROFILE_METHOD( "_CGWS3", "~_CGWS3" );
	GHPTR(NetworkManager) garbage;
	while( m_nDeviceCount )
		garbage = m_pDeviceList[--m_nDeviceCount];
	if( m_pDeviceList )
		free( m_pDeviceList );
};

int32_t _CGWS3::Init( void )
{
	GPROFILE_METHOD( "_CGWS3", "Init" );
	return S_OK;
};

int32_t _CGWS3::CreateConnectionManager( HWND hWnd, IGTrueNetworkHandler** ppDevice )
{
	GPROFILE_METHOD( "_CGWS3", "CreateConnectionManager" );
	if( 0 == ppDevice )
		return G_INVALIDARGUMENT;
	CGWS3NetworkManager* pDevice = new CGWS3NetworkManager(this);
	if( 0 == pDevice )
		return G_NOMEM;
	static id_t count = 0;
	count++;
	if( 0 > pDevice->internalInit( hWnd, count ) )
	{
		error_printf("Cannot initialize network manager!");
		return -1;
	}

	if( 0 > gresizePointerArray( sizeof(CGWS3NetworkManager*), m_nDeviceCount+1, &m_nDeviceArraySize, (void**)&m_pDeviceList, m_nDeviceArraySize+m_nDeviceArraySize/2 ) )
	{
		error_printf("");
		return -1;
	};

	m_pDeviceList[m_nDeviceCount] = pDevice;
	m_nDeviceCount++;

	return S_OK;
};
