#include "GD3D9.h"
#include "GD3D9Device.h"

#include "GD3D9ResourceManager.h"


//#define GPROFILE_ENABLED

#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

void selectValidParameters( IDirect3D9* pd3d9, const D3DPRESENT_PARAMETERS* in_DesiredParameters, 
					   D3DPRESENT_PARAMETERS* out_pFullscreenParameters, D3DPRESENT_PARAMETERS* out_pWindowedParameters );

_CGD3D9Device::_CGD3D9Device( CGD3D9* pContext )
	//:m_DeviceData(0,0,device_Free_handler),
	//m_pVertexDeclaration(0,0,Decl_Free_handler),
	//m_pIndex(0,0,ManagerBuffer_Free_handler),
	//m_BlendState(0,0,BlendState_Free_handler),
	//m_RasterizerState(0,0,Rasterizer_Free_handler),
	//m_DepthStencilState(0,0,DepthStencil_Free_handler),
	//m_pVertexFunction(0,0,Shader_Free_handler),
	//m_pPixelFunction(0,0,Shader_Free_handler),
	//m_pShader(0,0,XEffect_Free_handler)
{
	GPROFILE_METHOD( "_CGD3D9Device", "_CGD3D9Device" );
	m_bRenderingPaused		= true;
	m_bMinimized			= false;
	m_bMaximized			= false;
	m_pContext				= pContext;
	m_nStreamCount			= 0;
	m_nIndexStreamCount		= 0;
	m_pWindowData			= 0;
	m_pHALDeviceData		= 0;
	m_nManagerCount			= 0;
	m_nManagerArraySize		= 0;
	m_lstpManager			= 0;
	m_nActiveWindow			= 0;
	m_pDefaultManager		= 0;
	__pfreeInterface			= greleaseInterface;

	__nRefCount				= 1; // 
	//int32_t i, n=256;
	//for( i=0; i<n; i++ )
	//{
	//	m_pIndexStream	[i].onFreeHandler( ManagerBuffer_Free_handler );
	//	m_pVertexStream	[i].onFreeHandler( ManagerBuffer_Free_handler );
	//}

	//m_pShader				= 0;
	//
	//m_pVertexFunction		= 0;
	//m_pPixelFunction		= 0;

	memset( &m_FullScreenPP, 0, sizeof( D3DPRESENT_PARAMETERS ) );
	memset( &m_WindowedPP, 0, sizeof( D3DPRESENT_PARAMETERS ) );

	m_hMainWindow			= 0;
	m_hChildWindowList		= 0;

	m_nUsedIDArraySize		= 0;
	m_pUsedID				= 0;

	m_ClearColor	= 0xFF442266;	//GBLACK;
	m_fClearDepth	= 1.0f;
	m_nClearStencil	= 0;
	m_DeviceType	= D3DDEVTYPE_HAL;
	m_nAdapter		= D3DADAPTER_DEFAULT;
	m_bLostDevice	= true;
	m_bRendering	= false;
	m_bFullScreen	= false;

	m_pCurrentRenderTarget	= 0;

#define __GINIT_POINTER_ARRAY(ClassName, Token)			\
 	m_n##Token##Count	=  m_n##Token##ArraySize = 0;	\
 	m_lst##Token		= 0;
	__GINIT_POINTER_ARRAY( IGVideoHandler*, VideoHandler );
#undef __GINIT_POINTER_ARRAY
};

_CGD3D9Device::~_CGD3D9Device( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "~_CGD3D9Device" );
	GHPTR(VideoContext) garbage = m_pDefaultManager;
	m_pDefaultManager = 0;
	garbage.free();
	__onLostDevice();
	__onDestroyDevice();

	while( m_nManagerCount )
	{
		garbage = m_lstpManager[--m_nManagerCount];
		m_lstpManager[m_nManagerCount] = 0;
	}
	if( m_lstpManager )
		free( m_lstpManager );

	if( m_hMainWindow )
		DestroyWindow( m_hMainWindow );

	if( m_lstVideoHandler ) 
	{ 
		free( m_lstVideoHandler ); 
		m_lstVideoHandler	= 0;
	}

	if( m_pUsedID )
		free( m_pUsedID );
}

int32_t _CGD3D9Device::internalInit( GRDS(SWindow) pWindowData, uint32_t nMinDepth, uint32_t nMinStencil )
{
	GPROFILE_METHOD( "_CGD3D9Device", "internalInit" );
	if( 0 == pWindowData )
		return G_FAIL;
	
	if( gget(pWindowData)->hWindowHandle == 0 )
		return G_FAIL;

	HRESULT hr;
	IDirect3D9* pd3d9	= m_pContext->getd3d9();
	CGD3D9Enumerator* pEnum = m_pContext->getEnumerator();
	D3DDISPLAYMODE* pDesktopMode = pEnum->getDesktopMode();

	gpod_ptr<SD3D9Device> DeviceData( m_DeviceData );
	DeviceData.create();

	m_bRendering	= false;
	m_bLostDevice	= true;
	m_ClearColor	= 0xFF332277; //4295A1;
	m_fClearDepth	= 1.0f;
	m_nClearStencil	= 0;
	DeviceData->hWindowHandle	= gget(pWindowData)->hWindowHandle;
	uint32_t i, nWindows		= gget(pWindowData)->nChildWindows;

	for( i=0; i< nWindows; i++ )
	{
		DeviceData->hChildWindows[i] = gget(gget(pWindowData)->ppChildWindows[i])->hWindowHandle;
	}
	
	hr = pd3d9->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pDesktopMode->Format, pDesktopMode->Format, true );
	if( hr < 0 )
	{
		MessageBoxW(NULL, L"Can't check device D3DDEVTYPE_HAL for D3DADAPTER_DEFAULT", L"RenderDevice->Init()", MB_OK | MB_ICONWARNING );
		DeviceData->DeviceType	= D3DDEVTYPE_SW;
	}
	else 
	{
		DeviceData->DeviceType = D3DDEVTYPE_HAL;
	}

	DeviceData->nAdapter = D3DADAPTER_DEFAULT;
	if( pd3d9->GetDeviceCaps( D3DADAPTER_DEFAULT, DeviceData->DeviceType, &DeviceData->Caps ) )
	{	
		error_printf("GetDeviceCaps() FAILED!");
		return G_FAIL;
	};

	DWORD dwBehaviorFlags = 0;
	if( DeviceData->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
		dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	// If pure device and HW T&L supported
	if( (DeviceData->Caps.DevCaps & D3DDEVCAPS_PUREDEVICE) 
		&& (dwBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) )
	{
		dwBehaviorFlags |= D3DCREATE_PUREDEVICE;
	}
	DeviceData->dwBehaviorFlags  = dwBehaviorFlags;
	//
	D3DPRESENT_PARAMETERS* pp1	= &DeviceData->PresentParameters,
		* ppWindowed = &m_WindowedPP, * ppFullScreen = &m_FullScreenPP;
	memset( &DeviceData->PresentParameters, 0, sizeof( D3DPRESENT_PARAMETERS ) );

	// Windowed settings
	ppFullScreen->Windowed				= !(ppWindowed->Windowed		= pp1->Windowed	= TRUE);
	ppWindowed->hDeviceWindow			= ppFullScreen->hDeviceWindow	= pp1->hDeviceWindow	= gget(pWindowData)->hWindowHandle;
	pp1->EnableAutoDepthStencil			= TRUE;

	D3DDISPLAYMODE dm;
	pd3d9->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &dm );
	dm.Format;

	pp1->SwapEffect						= D3DSWAPEFFECT_DISCARD;
	ppWindowed->BackBufferFormat		= pp1->BackBufferFormat = dm.Format;
	ppFullScreen->BackBufferFormat		= D3DFMT_X8R8G8B8;
	
	pp1->BackBufferCount				= 1;
	pp1->AutoDepthStencilFormat			= D3DFMT_D24S8;
	pp1->PresentationInterval			= D3DPRESENT_INTERVAL_IMMEDIATE;
//	pp1->PresentationInterval			= D3DPRESENT_INTERVAL_DEFAULT;
	pp1->BackBufferWidth				= gget(pWindowData)->nWidth;
	pp1->BackBufferHeight				= gget(pWindowData)->nHeight;
	pp1->Flags							= 0;
	pp1->MultiSampleQuality				= DEFAULT_QUALITY;
	pp1->MultiSampleType				= D3DMULTISAMPLE_NONE;
	
	selectValidParameters( pd3d9, pp1, ppFullScreen, ppWindowed );

	m_bFullScreen = false;
	if( __createDevice( DeviceData.get_address() ) )
	{
		error_printf("__createDevice() FAILED!");
		return G_FAIL;
	};

	m_DeviceData	= DeviceData;
	m_pWindowData	= gacquire( pWindowData );


	if( __onCreateDevice( DeviceData->pDevice ) )
	{
		MessageBox( NULL, L"__onCreateDevice failed", L"Device->internalInit()", MB_OK | MB_ICONERROR );
		return G_FAIL;
	}
	if( __onResetDevice() )
	{
		MessageBox( NULL, L"__onResetDevice failed", L"internalInit()", MB_OK | MB_ICONERROR );
		return G_FAIL;
	}

	if( CreateResourceManager( (GOSH(VideoContext)*)&m_pDefaultManager ) )
	{
		error_printf("Failed to create internal device default resource manager!");
		return G_FAIL;
	}
	return 0;
};


void selectValidDepthStencilFormat( IDirect3D9* pd3d9, const D3DPRESENT_PARAMETERS* in_DesiredParameters, D3DFORMAT* out_pFullscreenFormat );
void selectValidPixelFormat( IDirect3D9* pd3d9, const D3DPRESENT_PARAMETERS* in_DesiredParameters, D3DFORMAT* out_pFullscreenFormat );
void selectValidParameters( IDirect3D9* pd3d9, const D3DPRESENT_PARAMETERS* in_DesiredParameters, D3DPRESENT_PARAMETERS* out_pFullscreenParameters, D3DPRESENT_PARAMETERS* out_pWindowedParameters )
{
	out_pWindowedParameters->Windowed				= !(out_pFullscreenParameters->Windowed = FALSE);
	out_pWindowedParameters->BackBufferCount		= out_pFullscreenParameters->BackBufferCount			= in_DesiredParameters->BackBufferCount			;
	out_pWindowedParameters->SwapEffect				= out_pFullscreenParameters->SwapEffect					= in_DesiredParameters->SwapEffect				;	
	out_pWindowedParameters->EnableAutoDepthStencil	= out_pFullscreenParameters->EnableAutoDepthStencil		= in_DesiredParameters->EnableAutoDepthStencil	;	
	out_pWindowedParameters->AutoDepthStencilFormat	= out_pFullscreenParameters->AutoDepthStencilFormat		= in_DesiredParameters->AutoDepthStencilFormat	;	
	out_pWindowedParameters->PresentationInterval	= out_pFullscreenParameters->PresentationInterval		= in_DesiredParameters->PresentationInterval	;	
	out_pWindowedParameters->PresentationInterval	= out_pFullscreenParameters->PresentationInterval		= in_DesiredParameters->PresentationInterval	;	
	out_pWindowedParameters->BackBufferWidth		= out_pFullscreenParameters->BackBufferWidth			= in_DesiredParameters->BackBufferWidth			;
	out_pWindowedParameters->BackBufferHeight		= out_pFullscreenParameters->BackBufferHeight			= in_DesiredParameters->BackBufferHeight		;	
	out_pWindowedParameters->Flags					= out_pFullscreenParameters->Flags						= in_DesiredParameters->Flags					;	
	out_pWindowedParameters->MultiSampleQuality		= out_pFullscreenParameters->MultiSampleQuality			= in_DesiredParameters->MultiSampleQuality		;	
	out_pWindowedParameters->MultiSampleType		= out_pFullscreenParameters->MultiSampleType			= in_DesiredParameters->MultiSampleType			;

	selectValidPixelFormat( pd3d9, in_DesiredParameters, &out_pFullscreenParameters->BackBufferFormat );
	selectValidPixelFormat( pd3d9, in_DesiredParameters, &out_pWindowedParameters->BackBufferFormat );
	selectValidDepthStencilFormat( pd3d9, out_pFullscreenParameters, &out_pFullscreenParameters->AutoDepthStencilFormat );
	selectValidDepthStencilFormat( pd3d9, out_pWindowedParameters, &out_pWindowedParameters->AutoDepthStencilFormat );
};
void selectValidDepthStencilFormat( IDirect3D9* pd3d9, const D3DPRESENT_PARAMETERS* in_DesiredParameters, D3DFORMAT* out_pFullscreenFormat )
{
	if( D3D_OK 
		!= pd3d9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, 
		D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, in_DesiredParameters->AutoDepthStencilFormat
		) 
	) 
	{	// Invalid requested format? Check D3DFMT_D24S8.
		if( D3D_OK 
			!= pd3d9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, 
				D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8
			) 
		)
		{	// D3DFMT_D24S8 Invalid? Check D3DFMT_D24X8
			warning_printf("D3DFMT_D24S8 Depth stencil format unavailable.");
			if( D3D_OK != pd3d9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, 
				D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 
				) 
			)	 
			{	// D3DFMT_D24S8 Invalid? Select D3DFMT_D16
				warning_printf("D24X8 Depth stencil format unavailable. Setting to D16.");
				(*out_pFullscreenFormat)	= D3DFMT_D16;
			}
			else // D24X8 available, check depth stencil match.
			{
				info_printf("D24X8 Depth stencil format selected.");
				if( FAILED(pd3d9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, 
					in_DesiredParameters->BackBufferFormat, D3DFMT_D24S8 ) ) 
					)
				{
					warning_printf("D24X8 DepthStencil Buffer Format doesn't match for current display mode. Setting to D3DFMT_D16.");
					(*out_pFullscreenFormat)	= D3DFMT_D16;
				}
				else
				{
					(*out_pFullscreenFormat)	= D3DFMT_D24X8;
					info_printf("D3DFMT_D24X8 Selected.");
				}		
			}
		}
		else // D24S8 available, check depth stencil match.
		{
			if( FAILED(pd3d9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, in_DesiredParameters->BackBufferFormat, D3DFMT_D24S8)) )
			{
				warning_printf("DepthStencil Format doesn't match for fullscreen mode.");
				if( FAILED(pd3d9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, in_DesiredParameters->BackBufferFormat, D3DFMT_D24X8)) )
				{
					warning_printf("D24X8 DepthStencil Buffer Format doesn't match for fullscreen mode. Selecting D3DFMT_D16");
					(*out_pFullscreenFormat)	= D3DFMT_D16;
				}
				else
				{
					(*out_pFullscreenFormat)	= D3DFMT_D24X8;
					info_printf("D3DFMT_D24X8 Selected.");
				}		
			}
			else
			{
				(*out_pFullscreenFormat)	= D3DFMT_D24S8;
				info_printf("D3DFMT_D24S8 Selected.");
			}		
		}
	}
	else // Requested format is compatible with device format. Check depth stencil match.
	{
		if( FAILED(pd3d9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, in_DesiredParameters->BackBufferFormat, in_DesiredParameters->AutoDepthStencilFormat)) )
		{	// Desired format unavailable? Check D3DFMT_D24S8
			warning_printf("DepthStencil Format doesn't match for fullscreen mode.");
			if( FAILED(pd3d9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, in_DesiredParameters->BackBufferFormat, D3DFMT_D24S8)) )
			{	// D3DFMT_D24S8 unavailable? check D3DFMT_D24X8
				warning_printf("D24S8 DepthStencil Buffer Format doesn't match for fullscreen mode. Selecting D3DFMT_D24X8");
				if( FAILED(pd3d9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, in_DesiredParameters->BackBufferFormat, in_DesiredParameters->BackBufferFormat, D3DFMT_D24X8)) )
				{	// D3DFMT_D24X8 unavailable? Select D3DFMT_D16
					warning_printf("D24X8 DepthStencil Buffer Format doesn't match for fullscreen mode. Selecting D3DFMT_D16");
					(*out_pFullscreenFormat)	= D3DFMT_D16;
				}
				else
				{
				(*out_pFullscreenFormat)	= D3DFMT_D24X8;
					info_printf("D3DFMT_D24X8 format selected.");
				}
			}
			else
			{
				(*out_pFullscreenFormat)	= D3DFMT_D24S8;
				info_printf("D3DFMT_D24S8 format selected.");
			}
		}
		else
		{
			(*out_pFullscreenFormat)	= in_DesiredParameters->AutoDepthStencilFormat;
			info_printf("Desired depth/stencil format available and selected.");
		}
	}
};

void selectValidPixelFormat( IDirect3D9* pd3d9, const D3DPRESENT_PARAMETERS* in_DesiredParameters, D3DFORMAT* out_pFullscreenFormat )
{
};
