//#define GPROFILE_ENABLED
#include "GD3D9.h"

#include "GDebugger_core.h"

using namespace god;

static int __cdecl SortModesCallback(const void* arg1, const void* arg2);

// some common backbuffer formats
const uint32_t g_nFormats_B=2;
const D3DFORMAT g_fmtBackBuffer[] = {	D3DFMT_A8R8G8B8 
										, D3DFMT_X8R8G8B8 
										//, D3DFMT_R8G8B8   
										//, D3DFMT_R5G6B5
										//, D3DFMT_A1R5G5B5 
										//, D3DFMT_X1R5G5B5
										//, D3DFMT_X4R4G4B4 
										//, D3DFMT_A4R4G4B4
										// , D3DFMT_A2B10G10R10 
									};


// some good multisample modes, list best ones last
const uint32_t g_nMS = 8;
const D3DMULTISAMPLE_TYPE g_msType[] = {	D3DMULTISAMPLE_NONE,
											D3DMULTISAMPLE_2_SAMPLES,
											D3DMULTISAMPLE_3_SAMPLES,
											D3DMULTISAMPLE_4_SAMPLES,
											D3DMULTISAMPLE_5_SAMPLES,
											D3DMULTISAMPLE_6_SAMPLES,
											D3DMULTISAMPLE_7_SAMPLES,
											D3DMULTISAMPLE_8_SAMPLES
										};

_CGD3D9Enumerator::_CGD3D9Enumerator( CGD3D9* pContext )
{
	GPROFILE_METHOD( "_CGD3D9Enumerator", "_CGD3D9Enumerator" );
	m_pContext = pContext;
	// nothing yet
	m_nAdapterCount = 0;

	memset( m_AdapterData, 0, sizeof(GD3D9ADAPTER_DATA)*20 );
	// set misc values
	m_nMinWidth  = 640;
	m_nMinHeight = 480;
	m_nMinBits   = 8;
	
	m_nSelectedAdapter	= 0;
	m_nSelectedDevice	= 0;
	m_nSelectedCombo	= 0;

   // set allowed bpp formats
	m_nAdapterFormat[0]		= D3DFMT_R5G6B5;		// 16 bit
	m_nAdapterFormat[1]		= D3DFMT_R8G8B8;		// 24 bit
	m_nAdapterFormat[2]		= D3DFMT_X1R5G5B5;		// 16 bit (15+mask)
	m_nAdapterFormat[3]		= D3DFMT_X4R4G4B4;		// 16 bit (12+mask)
	m_nAdapterFormat[4]		= D3DFMT_X8R8G8B8;		// 32 bit (24+mask)
	m_nAdapterFormat[5]		= D3DFMT_A1R5G5B5;		// 16 bit alpha
	m_nAdapterFormat[6]		= D3DFMT_A4R4G4B4;		// 16 bit alpha
	m_nAdapterFormat[7]		= D3DFMT_A8R8G8B8;		// 32 bit
	m_nAdapterFormat[8]		= D3DFMT_A2B10G10R10;	// 32 bit alpha
	m_nAdapterFormat[9]		= D3DFMT_R3G3B2;		// 8 bit (no alpha)
	m_nFormatCount			= 3;
}

_CGD3D9Enumerator::~_CGD3D9Enumerator( void )
{
	GPROFILE_METHOD( "_CGD3D9Enumerator", "~_CGD3D9Enumerator" );
}

int32_t _CGD3D9Enumerator::internalInit( int32_t nMinWidth, int32_t nMinHeight, int32_t nMinDepth, int32_t nMinStencil )
{	
	GPROFILE_METHOD( "_CGD3D9Enumerator", "internalInit" );
	if( nMinWidth < 320 || nMinHeight < 200 )
		return G_FAIL;

	HRESULT hr;
	m_nMinDepth		= nMinDepth;
	m_nMinStencil	= nMinStencil;
	// set misc values
	m_nMinWidth  = nMinWidth;
	m_nMinHeight = nMinHeight;
	m_nMinBits   = 8;

	if( 0 == m_pContext->getd3d9() )
		return G_FAIL;

	// get current adapter's display mode
	if( FAILED( hr = m_pContext->getd3d9()->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &m_DesktopMode ) ) ) 
		return hr; //
	if( FAILED( hr = __enumAdapters() ) )
		return hr;
	return G_OK;
}

int32_t _CGD3D9Enumerator::__enumAdapters( void )
{
	GPROFILE_METHOD( "_CGD3D9Enumerator", "__enumAdapters" );
	HRESULT hr;
	IDirect3D9* pd3d9 = m_pContext->getd3d9();

	D3DDISPLAYMODE DisplayMode;
	uint32_t nAdapterCount = pd3d9->GetAdapterCount();
	uint32_t nAdapter, nFormat, nMode;
	uint32_t nModeCount=0;
	for( nAdapter=0; nAdapter< nAdapterCount; nAdapter++ )
	{
		m_AdapterData[m_nAdapterCount].nAdapter		= nAdapter;
		m_AdapterData[m_nAdapterCount].nModeCount	= 0;	
		// get adapter information struct
		pd3d9->GetAdapterIdentifier(nAdapter, 0, &m_AdapterData[m_nAdapterCount].d3dAdapterIdentifier);

		// loop through allowed bpp formats
		for (nFormat=0; nFormat<m_nFormatCount; nFormat++) 
		{
			// get available display modes for this adapter/bpp format
			nModeCount = pd3d9->GetAdapterModeCount( nAdapter, m_nAdapterFormat[nFormat] );

			for (nMode=0; nMode<nModeCount; nMode++) 
			{
				// get number of available display modes
				pd3d9->EnumAdapterModes( nAdapter, m_nAdapterFormat[nFormat], nMode, &DisplayMode );

				// don't use less than we asked for
				if( DisplayMode.Width < m_nMinWidth  
					|| DisplayMode.Height < m_nMinHeight 
					|| __getBits(DisplayMode.Format) < m_nMinBits )
				{
					continue;
				}
				else // else save this mode
				{
					m_AdapterData[m_nAdapterCount].d3dDisplayMode[m_AdapterData[m_nAdapterCount].nModeCount] = DisplayMode;
					m_AdapterData[m_nAdapterCount].nModeCount++;
				}
				
			} // for [modes]
		} // for [formats]
		// we got the data, now sort it
		qsort(m_AdapterData[m_nAdapterCount].d3dDisplayMode, m_AdapterData[m_nAdapterCount].nModeCount,
			sizeof(D3DDISPLAYMODE), SortModesCallback);

		if( FAILED( hr = __enumDevices(m_AdapterData[m_nAdapterCount]) ) ) 
			return hr;

		// keep this one if any device is found
		if( m_AdapterData[m_nAdapterCount].nDeviceCount > 0 )
			m_nAdapterCount++;
	} // for [adapters]

	return G_OK;
} // __enumAdapters

int32_t _CGD3D9Enumerator::__enumDevices(GD3D9ADAPTER_DATA &AdapterData) 
{
	GPROFILE_METHOD( "_CGD3D9Enumerator", "__enumDevices" );
	GD3D9DEVICEINFO_DATA *pDeviceInfoData=0;
	HRESULT hr;

	IDirect3D9* pd3d9 = m_pContext->getd3d9();
	const D3DDEVTYPE d3dDevTypes[] =  { D3DDEVTYPE_HAL, D3DDEVTYPE_REF, D3DDEVTYPE_SW };

	AdapterData.nDeviceCount = 0;

	for (uint32_t nDevice=0; nDevice<2; nDevice++) 
	{
		pDeviceInfoData = &AdapterData.DeviceInfoData[AdapterData.nDeviceCount];

		pDeviceInfoData->nComboCount	= 0;
		pDeviceInfoData->nAdapter	 = AdapterData.nAdapter;
		pDeviceInfoData->d3dDevType  = d3dDevTypes[nDevice];

		// will fail if unsupported devtype
		if( FAILED( pd3d9->GetDeviceCaps( pDeviceInfoData->nAdapter, pDeviceInfoData->d3dDevType, 
			&pDeviceInfoData->d3dCaps )) )
			continue;

		// this should not fail
		if( FAILED(hr=__enumCombos(*pDeviceInfoData)) )
			return hr;
		
		// at least one supported combo?
		if( pDeviceInfoData->nComboCount <= 0 )
			continue;

		// keep valid device
		AdapterData.nDeviceCount++;
	} // for

	return G_OK;
} // __enumDevices

int32_t _CGD3D9Enumerator::__enumCombos( GD3D9DEVICEINFO_DATA &DeviceInfoData ) 
{
	GPROFILE_METHOD( "_CGD3D9Enumerator", "__enumCombos" );
	GD3D9COMBO_DATA* pComboData;
	bool bWindowed;
	bool bFmtCheck=false;

	DeviceInfoData.nComboCount = 0;

	IDirect3D9* pd3d9 = m_pContext->getd3d9();

	uint32_t nFormatA, nFormatB, n;
	// all adapter Formats
	for( nFormatA=0; nFormatA<m_nFormatCount; nFormatA++ ) 
	{
		// for all allowed backbuffer formats
		for( nFormatB=0; nFormatB < g_nFormats_B; nFormatB++) 
		{
			// check windowed and fullscreen
			for( n=0; n<2; n++) 
			{
				if( n==0 )
					bWindowed=true;
				else 
					bWindowed=false;

				// valid combo?
				if( FAILED( pd3d9->CheckDeviceType( DeviceInfoData.nAdapter, DeviceInfoData.d3dDevType, 
													m_nAdapterFormat[nFormatA], g_fmtBackBuffer[nFormatB], 
													bWindowed )))
					continue;

				// at this point we have a valid combo that is supported
				// by the system. still need to find depth/stencil fmt
				// and best vertex processing type. 
				pComboData = &DeviceInfoData.ComboData[DeviceInfoData.nComboCount];
				pComboData->nAdapter		= DeviceInfoData.nAdapter;
				pComboData->d3dDevType		= DeviceInfoData.d3dDevType;
				pComboData->bWindowed		= bWindowed;
				pComboData->fmtBackBuffer	= g_fmtBackBuffer[nFormatB];
				pComboData->fmtAdapter		= m_nAdapterFormat[nFormatA];

				// SELECT DEPTH/STENCIL FORMAT

				 // select depth fmt & stencil buffer
				if( m_nMinStencil )
				{
					pComboData->fmtDepthStencil = D3DFMT_D24S8;
					bFmtCheck = __confirmDepthFmt(pComboData);
				  
					if( !bFmtCheck )
					{
						pComboData->fmtDepthStencil = D3DFMT_D24X4S4;
						bFmtCheck = __confirmDepthFmt(pComboData);
					}
					if( !bFmtCheck ) 
					{
						pComboData->fmtDepthStencil = D3DFMT_D15S1;
						bFmtCheck = __confirmDepthFmt(pComboData);
					}
				}
				// or without stencil buffer
				else 
				{
					if( m_nMinDepth > 24 )
					{
						pComboData->fmtDepthStencil = D3DFMT_D32;
						bFmtCheck = __confirmDepthFmt(pComboData);
					}
					if( !bFmtCheck && (m_nMinDepth > 16) )
					{
						pComboData->fmtDepthStencil = D3DFMT_D24X8;
						bFmtCheck = __confirmDepthFmt(pComboData);
					}
					else 
					{
						pComboData->fmtDepthStencil = D3DFMT_D16;
						bFmtCheck = __confirmDepthFmt(pComboData);
					}
				}

				// no suitable fmt => ignore this combo
				if( !bFmtCheck )
					continue;

				// SELECT VERTEX PROCESSING

				// we have hardware T&L
				if(( DeviceInfoData.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0) 
				{
					// 1.case: pure device
					if( ((DeviceInfoData.d3dCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)!=0) 
						&& ( __confirmDevice(&DeviceInfoData.d3dCaps, D3DCREATE_PUREDEVICE, 
												pComboData->fmtBackBuffer)==G_OK ) )
					{
						pComboData->dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING 
												 | D3DCREATE_PUREDEVICE;
					}
					// 2.case: hardware device
					else if( __confirmDevice(&DeviceInfoData.d3dCaps, D3DCREATE_HARDWARE_VERTEXPROCESSING, 
												  pComboData->fmtBackBuffer)==G_OK ) 
					{
						pComboData->dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
					}
					// 3.case: mixed sw/hw
					else if( __confirmDevice(&DeviceInfoData.d3dCaps, D3DCREATE_MIXED_VERTEXPROCESSING, 
												  pComboData->fmtBackBuffer)==G_OK ) 
					{
						pComboData->dwBehavior = D3DCREATE_MIXED_VERTEXPROCESSING;
					}
				} // if [HW]
				else // 4.case: must be sw processing 
				{
					if( __confirmDevice(&DeviceInfoData.d3dCaps, D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
											pComboData->fmtBackBuffer)==G_OK) 
					{
						pComboData->dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
					}
				}
				// SELECT MULTISAMPLE TYPE
				DWORD msQuality;
				pComboData->msType = D3DMULTISAMPLE_NONE;
				for( uint8_t nMS=g_nMS-1; nMS>0; nMS-- )
				{
					if( SUCCEEDED( pd3d9->CheckDeviceMultiSampleType(
								pComboData->nAdapter, 
								pComboData->d3dDevType, 
								pComboData->fmtBackBuffer, 
								pComboData->bWindowed,
								g_msType[nMS], &msQuality ) ) )
					{
						pComboData->msType = g_msType[nMS];
						break;
					}
				} // for [multisample]

				// NOW WE GOT EVERYTHING WE NEED AND KEEP THIS COMBO
				DeviceInfoData.nComboCount++;
			} // for [windowed/fullscreen]
		} // for [BackbufferFormats]
	} // for [AdapterFormats]
	return G_OK;
}

static int __cdecl SortModesCallback(const void* arg1, const void* arg2) 
{
	GPROFILE_FUNCTION();
	D3DDISPLAYMODE* pdm1 = (D3DDISPLAYMODE*)arg1;
	D3DDISPLAYMODE* pdm2 = (D3DDISPLAYMODE*)arg2;

	if (pdm1->Width > pdm2->Width)
		return 1;
	if (pdm1->Width < pdm2->Width)
		return -1;
	if (pdm1->Height > pdm2->Height)
		return 1;
	if (pdm1->Height < pdm2->Height)
		return -1;
	if (pdm1->Format > pdm2->Format)
		return 1;
	if (pdm1->Format < pdm2->Format)
		return -1;
	if (pdm1->RefreshRate > pdm2->RefreshRate)
		return 1;
	if (pdm1->RefreshRate < pdm2->RefreshRate)
		return -1;
	return 0;
} // SortModesCallback


bool _CGD3D9Enumerator::__confirmDepthFmt(GD3D9COMBO_DATA *pComboData) 
{
	GPROFILE_METHOD( "_CGD3D9Enumerator", "__confirmDepthFmt" );
	IDirect3D9* pd3d9 = m_pContext->getd3d9();
	// compatible with adapter?
	if( FAILED( pd3d9->CheckDeviceFormat(	pComboData->nAdapter, pComboData->d3dDevType, 
											pComboData->fmtAdapter, D3DUSAGE_DEPTHSTENCIL, 
											D3DRTYPE_SURFACE, pComboData->fmtDepthStencil)) )
		return false;
	// compatible with backbuffer format?
	if( FAILED( pd3d9->CheckDepthStencilMatch(	pComboData->nAdapter, pComboData->d3dDevType, 
												pComboData->fmtAdapter, pComboData->fmtBackBuffer, 
												pComboData->fmtDepthStencil ) ) )
		return false;
	return true;
} // ConfirmDepthFmt
/*----------------------------------------------------------------*/


int32_t _CGD3D9Enumerator::__confirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT fmtBackbuffer) 
{
	GPROFILE_METHOD( "_CGD3D9Enumerator", "__confirmDevice" );
	if ( (dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING) 
		|| (dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING) ) 
	{
		// alphablending from texture pixels supported
		if( 0 == (pCaps->TextureCaps & D3DPTEXTURECAPS_ALPHA) )
			return G_FAIL;
		if( pCaps->VertexShaderVersion < D3DVS_VERSION(1,0) )
			return G_FAIL;
	}
	return G_OK;
} // ConfirmDevice

uint32_t _CGD3D9Enumerator::__getBits(D3DFORMAT fmt) 
{
	GPROFILE_METHOD( "_CGD3D9Enumerator", "__getBits" );
	switch(fmt) 
	{
	case D3DFMT_A2B10G10R10: 
		return 32;
	case D3DFMT_R8G8B8: 
		return 24;
	case D3DFMT_A8R8G8B8: 
		return 24;
	case D3DFMT_X8R8G8B8:	 
		return 24;
	case D3DFMT_R5G6B5: 
		return 16;
	case D3DFMT_X1R5G5B5:
		return 15;
	case D3DFMT_A1R5G5B5:
		return 15;
	case D3DFMT_X4R4G4B4:
		return 12;
	case D3DFMT_A4R4G4B4:
		return 12;
	case D3DFMT_R3G3B2:
		return 8;
	case D3DFMT_A8R3G3B2:
		return 8;
	default:
		return 0;
	}
} // GetBits