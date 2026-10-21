#include "GDIKeyboard.h"
#include "GDInput.h"

//#define GPROFILE_ENABLED
#include "GDebugger_core.h"

CGDIKeyboard::CGDIKeyboard( CGDInput *pInput )
{
	GPROFILE_METHOD( "CGDIKeyboard", "CGDIKeyboard" );
	__nRefCount				= 1;

	m_pContext				= pInput;

	ZeroMemory(	m_diks, 256 );
	ZeroMemory(	m_diksPrev, 256 );
	m_pDIK					= 0;
	m_hWnd					= 0;

	m_bExclusive			= false;
	m_bForeground			= false;
	m_bImmediate			= true;
	m_bDisableWindowsKey	= false;
	__pfreeInterface			= greleaseInterface;

	m_bAcquired				= false;
};

CGDIKeyboard::~CGDIKeyboard( void )
{
	GPROFILE_METHOD( "CGDIKeyboard", "~CGDIKeyboard" );
	_Cleanup();
};

void CGDIKeyboard::_Cleanup( void )
{
	GPROFILE_METHOD( "CGDIKeyboard", "_Cleanup" );
	_UnacquireDirectInput();

	ZeroMemory(	m_diks, 256 );
	ZeroMemory(	m_diksPrev, 256 );

	if( m_pDIK )	m_pDIK->Release();

	m_pDIK	= NULL;
	m_hWnd	= NULL;
};

INT32 CGDIKeyboard::Init( HWND hWnd )
{
	GPROFILE_METHOD( "CGDIKeyboard", "Init" );
	m_hWnd = hWnd;
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
};

HRESULT CGDIKeyboard::_CreateDevice( void )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "_CreateDevice" );
   // Obtain an interface to the system keyboard device.
	// Obtain an interface to the system Mouse device.
	if( S_OK != m_pContext->GetDirectInput()->CreateDevice( GUID_SysKeyboard, &m_pDIK, NULL ) ) 
		return G_FAIL;

	// Set the data format to "Keyboard format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing an array
    // of 256 bytes to IDirectInputDevice::GetDeviceState.
	if( S_OK != m_pDIK->SetDataFormat( &c_dfDIKeyboard ) )
        return G_FAIL;

	return S_OK;
};

INT32 CGDIKeyboard::Poll( FLOAT fElapsedTime )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "Poll" );
	if( !m_pDIK )
		return G_FAIL;	//

	HRESULT hr;

	// Get the input's device state, and put the state in dims
	hr = m_pDIK->GetDeviceState( 256, m_diks );

	if( FAILED( hr ) )
    {
		ZeroMemory( m_diks, 256 );
		ZeroMemory(	m_diksPrev, 256 );
		// DirectInput may be telling us that the input stream has been
		// interrupted. We just re-acquire and try again.
		if( _AcquireDirectInput() )
			m_bAcquired = false;
	}
	else
		_HandleStates();

	return S_OK;
};

int32_t CGDIKeyboard::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "MsgProc" );
	if( hWnd != m_hWnd )
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

HRESULT CGDIKeyboard::_AcquireDirectInput( void )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "_AcquireDirectInput" );
	if( m_pDIK )
		if( S_OK != m_pDIK->Acquire() )
			return G_FAIL;
		else
			m_bAcquired	= true;

	ZeroMemory( m_diks, 256 );
	ZeroMemory(	m_diksPrev, 256 );

	return S_OK;
};

long CGDIKeyboard::_UnacquireDirectInput( void )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "_UnacquireDirectInput" );
	if( m_pDIK && m_bAcquired )
		if( FAILED( m_pDIK->Unacquire() ) )
			return G_FAIL;
		else
			m_bAcquired	= false;

	ZeroMemory( m_diks, 256 );
	ZeroMemory(	m_diksPrev, 256 );

	return S_OK;
};

void CGDIKeyboard::SetImmediate( bool bImmediate )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "SetImmediate" );
	if( !bImmediate )
		MessageBox(	NULL, L"Buffered mode is not ready yet. Setting to immediate mode.",
					L"CKeyboard->SetImmediate( bool )", MB_OK | MB_ICONERROR 
				);
	m_bImmediate = true;
};

void CGDIKeyboard::SetForeground( bool bForeground )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "SetForeground" );
	if( m_bForeground == bForeground )
		return;

	_UnacquireDirectInput();

	m_bForeground = bForeground;

	// Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
	_SetCooperativeLevel();
	_AcquireDirectInput();
};

void CGDIKeyboard::DisableWindowsKey( bool bDisable )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "DisableWindowsKey" );
	if( m_bDisableWindowsKey == bDisable )
		return;

	_UnacquireDirectInput();

	m_bDisableWindowsKey = bDisable;
	// Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
	_SetCooperativeLevel();
	_AcquireDirectInput();
};

void CGDIKeyboard::SetExclusive( bool bExclusive )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "SetExclusive" );
	if( m_bExclusive == bExclusive )
		return;
	_UnacquireDirectInput();

	m_bExclusive = bExclusive;

	_SetCooperativeLevel();
	_AcquireDirectInput();
};

HRESULT CGDIKeyboard::_SetCooperativeLevel( void )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "_SetCooperativeLevel" );
	HRESULT hr;
	unsigned long dwCoopFlags;

	if( !m_hWnd  || !m_pDIK )
		return G_FAIL;

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
		MessageBox(	NULL, L"For security reasons, background exclusive Keyboard"
					L"access is not allowed. Setting to foreground mode" , 
					L"GDInput.dll" , MB_OK | MB_ICONERROR 
				);
	}
	else
        dwCoopFlags |= DISCL_BACKGROUND;

    if( m_bDisableWindowsKey && !m_bExclusive && m_bForeground )
        dwCoopFlags |= DISCL_NOWINKEY;

    hr = m_pDIK->SetCooperativeLevel( m_hWnd, dwCoopFlags );
    
	if( hr == DIERR_UNSUPPORTED && !m_bForeground  && m_bExclusive  )
    {
		MessageBoxW(	NULL, L"SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n"
						L"For security reasons, background exclusive Keyboard\n"
						L"access is not allowed." , L"Keyboard", MB_OK | MB_ICONERROR 
					);
		return G_FAIL;
	}

	return S_OK;
}

INT32 CGDIKeyboard::AddHandler( IGKeyboardHandler* pHandler )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "AddHandler" );
	if( 0 == pHandler )
		return -1;
	for( uint32_t iHandler=0, nHandlerCount = m_lstHandler.size(); iHandler< nHandlerCount; iHandler++ )
	{
		if( m_lstHandler[iHandler] == pHandler )
		{
			warning_printf("Keyboard handler already added to device list.");
			return 1; // already added
		}
	}
	m_lstHandler.push( pHandler );
	return 0;
};

INT32 CGDIKeyboard::RemoveHandler( IGKeyboardHandler* pHandler )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "RemoveHandler" );
	if( 0 == pHandler )
	{
		error_printf("Cannot register a null keyboard handler!");
		return -1;
	}
	for( uint32_t iHandler=0, nHandlerCount=m_lstHandler.size(); iHandler < nHandlerCount; iHandler++ )
		if( m_lstHandler[iHandler] == pHandler )
		{
			m_lstHandler.set_value(iHandler, m_lstHandler[--nHandlerCount] );
			m_lstHandler.pop(0);
			return 0;
		}
	return 1;
};

void CGDIKeyboard::_HandleStates( void )
{
 	GPROFILE_METHOD( "CGDIKeyboard", "_HandleStates" );
	uint32_t j, nHandlerCount = m_lstHandler.size();
	for( uint32_t i = 0; i<256; i++ )
	{
		switch( m_diks[i] )
		{
		case 0x80:
			if( !( m_diksPrev[i] & 0x80 ) )
			{
				for( j = 0; j < nHandlerCount; j++ )
					m_lstHandler[j]->OnKeyDown( i );
			}
			break;
		case 0:
			if( m_diksPrev[i] & 0x80 )
			{
				for( j = 0; j < nHandlerCount; j++ )
					m_lstHandler[j]->OnKeyUp( i );
			}
		}
		m_diksPrev[i] = m_diks[i];
	}
};

void* CGDIKeyboard::GetInterfacePointer( GINTERFACEID_TYPE InterfaceType )
{ 
 	GPROFILE_METHOD( "CGDIKeyboard", "GetInterfacePointer" );
	switch( InterfaceType )
	{
	case GIID_RESERVED:
	case GIID_CUSTOMINTERFACE:
		++ __nRefCount;
		return (CGDIKeyboard*)this;
	case GIID_KEYBOARDMANAGER:
		++ __nRefCount;
		return (GOSH(Keyboard))this;
	default:
		return 0;
	}
}

bool CGDIKeyboard::GetProfilingEnabled( void ) const
{
	return god::genabledDebugger();
};

void CGDIKeyboard::SetProfilingEnabled( bool bEnabled )
{
	genableDebugger( bEnabled );
};
