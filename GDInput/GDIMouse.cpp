#include "GDIMouse.h"
#include "GDInput.h"

//#define GPROFILE_ENABLED
#include "GDebugger_core.h"

CGDIMouse::CGDIMouse( CGDInput *pInput )
{
	GPROFILE_METHOD( "CGDIMouse", "CGDIMouse" );
	__nRefCount			= 1;
	__pfreeInterface		= greleaseInterface;
	
	m_nID				= 0;
	m_pContext			= pInput;
	m_pDriverHALData	= 0;
	ZeroMemory( &m_dims2, sizeof( DIMOUSESTATE2 ) );
	ZeroMemory( &m_dims2Prev, sizeof( DIMOUSESTATE2 ) );

	m_pDIM			= 0;
	m_hWnd			= NULL;

	m_bExclusive	= false;
	m_bForeground	= false;
	m_bImmediate	= true;

	m_bAcquired		= false;
}

CGDIMouse::~CGDIMouse( void )
{
	GPROFILE_METHOD( "CGDIMouse", "~CGDIMouse" );
	_Cleanup();
};

void CGDIMouse::_Cleanup( void )
{
	GPROFILE_METHOD( "CGDIMouse", "_Cleanup" );
	_UnacquireDirectInput();

	ZeroMemory( &m_dims2, sizeof( DIMOUSESTATE2 ) );
	ZeroMemory( &m_dims2Prev, sizeof( DIMOUSESTATE2 ) );

	if( m_pDIM )
		m_pDIM->Release();

	m_bExclusive	= false;
	m_bForeground	= false;
	m_bImmediate	= true;

	m_pDIM				= NULL;
	m_hWnd				= NULL;
};

INT32 CGDIMouse::Init( HWND hWnd )
{
	GPROFILE_METHOD( "CGDIMouse", "Init" );
	m_hWnd				= hWnd;
	if( _CreateDevice() )
	{
		_Cleanup();
		return G_FAIL;
	}
	// Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
	_SetCooperativeLevel();
	_AcquireDirectInput();
	return S_OK;
}

HRESULT CGDIMouse::_CreateDevice( void )
{
	GPROFILE_METHOD( "CGDIMouse", "_CreateDevice" );
	if( DI_OK != m_pContext->GetDirectInput()->CreateDevice( GUID_SysMouse, &m_pDIM, NULL ) )
		return G_FAIL;

	if( DI_OK != m_pDIM->SetDataFormat( &c_dfDIMouse2 ) )
        return G_FAIL;

	_SetCooperativeLevel();
	_AcquireDirectInput();
	return S_OK;
};

error_t CGDIMouse::Poll( FLOAT fElapsedTime )
{
	GPROFILE_METHOD( "CGDIMouse", "Poll" );
	if( !m_pDIM )
	{
		error_printf("IDirectInputDevice8* is NULL!");
		return -1;	// no device?
	}

	HRESULT hr;
	// Get the input's device state, and put the state in dims
	if( FAILED( hr = m_pDIM->GetDeviceState( sizeof( DIMOUSESTATE2 ), &m_dims2 ) ) )
	{
		ZeroMemory( &m_dims2, sizeof( m_dims2 ) );
		ZeroMemory( &m_dims2Prev, sizeof( DIMOUSESTATE2 ) );
		// DirectInput may be telling us that the input stream has been
		// interrupted.
		if( _AcquireDirectInput() )
			m_bAcquired = false;
    }
	else
		_HandleStates();

	return S_OK;
};

int32_t CGDIMouse::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( hWnd != m_hWnd || !m_hWnd )
		return 0;

	// Acquire or unacquire on window WM_ACTIVATE messages
	switch( uMsg )
	{
	case WM_ACTIVATE:
		if( WA_INACTIVE != wParam )
			_AcquireDirectInput();
		else 
			_UnacquireDirectInput();
		return 0;
	}
	return 0;
};

long CGDIMouse::_AcquireDirectInput( void )
{
	GPROFILE_METHOD( "CGDIMouse", "_AcquireDirectInput" );
	if( m_pDIM )
		if( DI_OK != m_pDIM->Acquire() )
			return -1;
		else
			m_bAcquired	= true;

	ZeroMemory( &m_dims2, sizeof( DIMOUSESTATE2 ) );
	ZeroMemory( &m_dims2Prev, sizeof( DIMOUSESTATE2 ) );

	return 0;
};

long CGDIMouse::_UnacquireDirectInput( void )
{
	GPROFILE_METHOD( "CGDIMouse", "_UnacquireDirectInput" );
	if( m_pDIM && m_bAcquired )
	{
		if( FAILED( m_pDIM->Unacquire() ) )
			return -1;
		else
			m_bAcquired		= false;
	}
	ZeroMemory( &m_dims2, sizeof( DIMOUSESTATE2 ) );
	ZeroMemory( &m_dims2Prev, sizeof( DIMOUSESTATE2 ) );

	return 0;
};

void CGDIMouse::SetImmediate( bool bImmediate )
{
	GPROFILE_METHOD( "CGDIMouse", "SetImmediate" );
	if( !bImmediate )
		MessageBox(	NULL, L"Buffered mode is not ready yet. Setting to immediate mode.",
					L"CodenamedDInput.dll", MB_OK | MB_ICONERROR 
				);
	m_bImmediate = true;
};

void CGDIMouse::SetForeground( bool bForeground )
{
	GPROFILE_METHOD( "CGDIMouse", "SetForeground" );
	if( m_bForeground && bForeground )
		return;

	_UnacquireDirectInput();
	m_bForeground = bForeground;
	// Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
	_SetCooperativeLevel();
	_AcquireDirectInput();
};

void CGDIMouse::SetExclusive( bool bExclusive )
{
	GPROFILE_METHOD( "CGDIMouse", "SetExclusive" );
	if( m_bExclusive == bExclusive )
		return;

	_UnacquireDirectInput();

	m_bExclusive = bExclusive;

	_SetCooperativeLevel();
	_AcquireDirectInput();
};

HRESULT CGDIMouse::_SetCooperativeLevel( void )
{
	GPROFILE_METHOD( "CGDIMouse", "_SetCooperativeLevel" );
	HRESULT hr;
	unsigned long dwCoopFlags;

	if( !m_hWnd  || !m_pDIM )
		return true;

	if( m_bExclusive )
        dwCoopFlags = DISCL_EXCLUSIVE;
    else
        dwCoopFlags = DISCL_NONEXCLUSIVE;

    if( m_bForeground )
        dwCoopFlags |= DISCL_FOREGROUND;
    else if( m_bExclusive )
	{
//		m_bForeground = true; // maybe the user will set nonexclusive after this call
        dwCoopFlags |= DISCL_FOREGROUND;
		MessageBox(	NULL, L"For security reasons, background exclusive mouse\n"
					L"access is not allowed. Setting to foreground mode" , 
					L"Mouse->SetCooperativeLevel()" , MB_OK | MB_ICONERROR 
				);
	}
	else
        dwCoopFlags |= DISCL_BACKGROUND;

    hr = m_pDIM->SetCooperativeLevel( m_hWnd, dwCoopFlags );
    
	if( hr == DIERR_UNSUPPORTED && !m_bForeground  && m_bExclusive  )
    {
		MessageBoxW( NULL, L"SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n"
                          L"For security reasons, background exclusive mouse\n"
                          L"access is not allowed." , L"Mouse", MB_OK | MB_ICONERROR );
		return true;
	}
	return false;

}

int32_t CGDIMouse::AddHandler( IGMouseHandler* pHandler )
{
	GPROFILE_METHOD( "CGDIMouse", "AddHandler" );
	if( 0 == pHandler )
		return -1;
	for( uint32_t iHandler=0, nHandlerCount = m_lstHandler.size(); iHandler < nHandlerCount; iHandler++ )
	{
		if( m_lstHandler[iHandler] == pHandler )
		{
			warning_printf("Cannot registered an already registered handler.");
			return 1;
		}
	}
	m_lstHandler.push( pHandler );
	return 0;
};

int32_t CGDIMouse::RemoveHandler( IGMouseHandler* pHandler )
{
	GPROFILE_METHOD( "CGDIMouse", "RemoveHandler" );
	if( 0 == pHandler )
	{
		error_printf("Cannot register a null mouse handler!");
		return -1;
	}
	for( uint32_t iHandler=0, nHandlerCount=m_lstHandler.size(); iHandler < nHandlerCount; iHandler++ )
		if( m_lstHandler[iHandler] == pHandler )
		{
			m_lstHandler.set_value(iHandler, m_lstHandler[--nHandlerCount] );
			m_lstHandler.pop(0);
			return 0;
		}
	warning_printf("Cannot unregister a handler that wasn't registered first.");
	return 1;
};

void CGDIMouse::_HandleStates( void )
{
	GPROFILE_METHOD( "CGDIMouse", "_HandleStates" );
	std::list<IGMouseHandler*>::iterator	iHandler;
	uint32_t j, nHandlerCount = m_lstHandler.size();
	for( uint32_t i = 0; i<8; i++ )
	{
		if( (m_dims2.rgbButtons[i] & 0x80) && !( m_dims2Prev.rgbButtons[i] & 0x80 ) )
		{
			for( j = 0; j < nHandlerCount; j++ )
				m_lstHandler[j]->OnMouseButtonDown( i );
		}
		else 
			if( !( m_dims2.rgbButtons[i] & 0x80 ) && (m_dims2Prev.rgbButtons[i] & 0x80) )
			{
				for( j = 0; j < nHandlerCount; j++ )
					m_lstHandler[j]->OnMouseButtonUp( i );
			}
		m_dims2Prev.rgbButtons[i] = m_dims2.rgbButtons[i];
	}
	if( m_dims2.lX || m_dims2.lY || m_dims2.lZ )
	{
		for( j = 0; j < nHandlerCount; j++ )
			m_lstHandler[j]->OnMouseMove( m_dims2.lX, m_dims2.lY, m_dims2.lZ );
	}
	m_dims2Prev.lX = m_dims2.lX;
	m_dims2Prev.lY = m_dims2.lY;
	m_dims2Prev.lZ = m_dims2.lZ;
}

void* CGDIMouse::GetInterfacePointer( GINTERFACEID_TYPE InterfaceType )
{ 
	GPROFILE_METHOD( "CGDIMouse", "GetInterfacePointer" );
	switch( InterfaceType )
	{
	case GIID_RESERVED:
	case GIID_CUSTOMINTERFACE:
		++ __nRefCount;
		return (CGDIMouse*)this;
	case GIID_MOUSEMANAGER:
		++ __nRefCount;
		return (GOSH(Mouse))this;
	default:
		return 0;
	}
}

bool CGDIMouse::GetProfilingEnabled( void ) const
{
	return god::genabledDebugger();
};

void CGDIMouse::SetProfilingEnabled( bool bEnabled )
{
	genableDebugger( bEnabled );
};
