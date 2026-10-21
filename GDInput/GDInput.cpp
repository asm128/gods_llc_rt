#include "GDInput.h"

//#define GPROFILE_ENABLED
#include "GCore_printf.h"
#include "GDebugger_core.h"

INT32 CreateGInput( IGInput** ppInterface )
{
	GPROFILE_FUNCTION();
	CGDInput* pInput = new CGDInput();
	if( 0 == pInput )
	{
		error_printf("Not enough memory!");
		return G_NOMEM;
	}
	int32_t r = 0;
	if( r = pInput->Init() )
	{
		error_printf("(CGDInput*)->Init() returned %i!", r);
		delete pInput;
		return G_FAIL;
	}
	*ppInterface = pInput;
	return r;
};

INT32 ReleaseGInput( IGInput *pInterface )
{
	GPROFILE_FUNCTION();
	if( !pInterface )
		return G_FAIL;
	delete pInterface;
	return S_OK;
}

CGDInput::CGDInput( void )
{
	GPROFILE_METHOD( "CGDInput", "CGDInput" );
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	m_pDI		= NULL;
}
CGDInput::~CGDInput( void )
{
	GPROFILE_METHOD( "CGDInput", "~CGDInput" );
	_Cleanup();
};

int32_t CGDInput::Init( void )
{
	GPROFILE_METHOD( "CGDInput", "Init" );
	if( S_OK != DirectInput8Create(	GetModuleHandleW( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8W, 
		( void** )&m_pDI, NULL ) )
	{
		MessageBox( NULL, L"Cannot create DirectInput context", L"GDInput", MB_OK | MB_ICONERROR );
		_Cleanup();
		return G_FAIL;
	};
	return S_OK;
};

// Inherited from INaturalInput interface.
int32_t CGDInput::Update( float fElapsedTime )
{
	GPROFILE_METHOD( "CGDInput", "Update" );

	return S_OK;
};

int32_t CGDInput::CreateKeyboardManager( IGTrueKeyboardHandler** ppCreated, HWND hWnd )
{
	GPROFILE_METHOD( "CGDInput", "CreateKeyboardManager" );
	if( !m_pDI )
		return G_FAIL;
	CGDIKeyboard* pDevice = new CGDIKeyboard( this );
	GHPTR(Keyboard) garbage;
	if( pDevice->Init( hWnd ) )
	{
		garbage = pDevice;
		return G_FAIL;
	}
	//m_lstKeyboard.push_back( pDevice );
	garbage = *ppCreated;
	*ppCreated = pDevice;
	return S_OK;
};

INT32 CGDInput::CreateMouseManager( IGTrueMouseHandler** ppCreated, HWND hWnd )
{
	GPROFILE_METHOD( "CGDInput", "CreateMouseManager" );
	if( !m_pDI )
		return G_FAIL;
	CGDIMouse* pDevice = new CGDIMouse( this );
	GHPTR(Mouse) garbage;
	if( pDevice->Init( hWnd ) )
	{
		garbage = pDevice;
		return G_FAIL;
	}
	//m_lstMouse.push_back( pDevice );
	garbage = *ppCreated;
	*ppCreated = pDevice;
	return S_OK;
};

void CGDInput::_Cleanup( void )
{
	GPROFILE_METHOD( "CGDInput", "_Cleanup" );
	GHPTR(Keyboard) garbage;
	CGDIKeyboard* pK;
	while( m_lstKeyboard.size() )
	{
		m_lstKeyboard.pop(&pK);
		garbage = pK;
	}
	garbage.free();

	GHPTR(Mouse) garbage2;
	CGDIMouse* pM;
	while( m_lstMouse.size() )
	{
		m_lstMouse.pop(&pM);
		garbage2 = pM;
	}
	garbage2.free();
	if( m_pDI )
		m_pDI->Release();

	m_pDI = NULL;

};