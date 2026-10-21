//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

GRDS(SBlend) _CGD3D9Device::GetBlendData( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetBlendData" );
	if( !m_BlendState )
		return 0;
	return gacquire( m_BlendState->BlendData );
};

GRDS(SRasterizer) _CGD3D9Device::GetRasterizerData( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetRasterizerData" );
	if( !m_RasterizerState )
		return 0;
	return gacquire( m_RasterizerState->RasterizerData );
};

GRDS(SDepth) _CGD3D9Device::GetDepthData( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetDepthData" );
	if( !m_DepthStencilState )
		return 0;
	return gacquire( m_DepthStencilState->DepthData );
};

GRDS(SStencil) _CGD3D9Device::GetStencilData( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetStencilData" );
	if( !m_DepthStencilState )
		return 0;
	return gacquire( m_DepthStencilState->StencilData );
}

int32_t _CGD3D9Device::SetBlendData( GRDS(SBlend) pBlendData )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetBlendData" );
	if( (m_BlendState.valid() && pBlendData == m_BlendState->BlendData) 
		|| (pBlendData == 0 && !m_BlendState) )
	{// if the same state is already applied, just return 0.
		return 0;
	}
	gpod_ptr<SBlendState> pBlendItem, pNewBlend;
	pBlendItem.create();
	pBlendItem->BlendData = gacquire( pBlendData ); // assign to new structure for management

	pNewBlend		= m_pDefaultManager->getBlendStateItem( pBlendItem.get_pointer() );
	if( !pNewBlend )
	{
		error_printf("FAILED to create blend state item! CGD3D9ResourceManager::getDepthStencilStateItem() returned NULL!");
		return -1;
	}

	if( pNewBlend == m_BlendState )
		return 0; // the state is already applied, nothing to do.

	pBlendItem		= m_BlendState;	// store old pointer to release later
	m_BlendState	= pNewBlend;		// assign the managed pointer
	if( pBlendItem.valid() )
		pBlendItem->bActive = false;

	return 0;
};

int32_t _CGD3D9Device::SetRasterizerData( GRDS(SRasterizer) pRasterizerData )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetRasterizerData" );
	if( (m_RasterizerState.valid() && pRasterizerData == m_RasterizerState->RasterizerData) 
		|| (pRasterizerData == 0 && !m_RasterizerState) )
	{// if the same state is already applied, just return 0.
		return 0;
	}


	gpod_ptr<SRasterizerState> pRasterizerItem, pNewRasterizer;
	pRasterizerItem.create();
	gacquire( pRasterizerData ); // acquire to assign to new structure for management
	pRasterizerItem->RasterizerData = pRasterizerData;

	pNewRasterizer		= m_pDefaultManager->getRasterizerStateItem( pRasterizerItem.get_pointer() ); // request initialized data
	if( !pNewRasterizer )
	{
		error_printf("FAILED to create rasterizer state item! CGD3D9ResourceManager::getDepthStencilStateItem() returned NULL!");
		return -1;
	}
	if( m_RasterizerState == pNewRasterizer )
		return 0; // this state is already set

	pRasterizerItem		= m_RasterizerState;	// store old pointer to release later
	m_RasterizerState	= pNewRasterizer;		// assign the managed pointer
	if( pRasterizerItem.valid() )
		pRasterizerItem->bActive = false;
	return 0;
};

int32_t _CGD3D9Device::SetDepthData( GRDS(SDepth) pDepthData )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetDepthData" );
	if( (m_DepthStencilState.valid() && pDepthData == m_DepthStencilState->DepthData) 
		|| (pDepthData == 0 && !m_DepthStencilState) )
	{// if the same state is already applied, just return 0.
		return 0;
	}


	gpod_ptr<SDepthStencilState> pDepthStencilItem, pNewDepthStencil;
	pDepthStencilItem.create();
	pDepthStencilItem->DepthData = gacquire( pDepthData ); // assign to new structure for management

	// get active stencil state and clone to new desc
	if( m_DepthStencilState.valid() )
		pDepthStencilItem->StencilData = gacquire( m_DepthStencilState->StencilData );

	if( 0 == pDepthStencilItem->StencilData )
	{
		gcreate( &pDepthStencilItem->StencilData );
		gget(pDepthStencilItem->StencilData)->StencilEnable			= false;
		gget(pDepthStencilItem->StencilData)->DepthFailOperation	= GOPERATIONTYPE_KEEP;
		gget(pDepthStencilItem->StencilData)->FailOperation			= GOPERATIONTYPE_KEEP;
		gget(pDepthStencilItem->StencilData)->PassOperation			= GOPERATIONTYPE_REPLACE;
		gget(pDepthStencilItem->StencilData)->Operator				= GOPERATOR_ALWAYS;
		gget(pDepthStencilItem->StencilData)->ReadMask				= 0xFF;
		gget(pDepthStencilItem->StencilData)->WriteMask				= 0xFF;
	}

	pNewDepthStencil				= m_pDefaultManager->getDepthStencilStateItem( pDepthStencilItem.get_pointer() );
	if( !pNewDepthStencil )
	{
		error_printf("FAILED to create depth stencil state item! CGD3D9ResourceManager::getDepthStencilStateItem() returned NULL!");
		return -1;
	}
	if( m_DepthStencilState == pNewDepthStencil )
		return 0; // this state is already set
	pDepthStencilItem	= m_DepthStencilState;	// store old pointer to release later
	m_DepthStencilState	= pNewDepthStencil;		// assign the managed pointer
	if( pDepthStencilItem.valid() )
		pDepthStencilItem->bActive = false;

	return 0;
};

int32_t _CGD3D9Device::SetStencilData( GRDS(SStencil) pStencilData )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetStencilData" );
	if( (m_DepthStencilState.valid() && pStencilData == m_DepthStencilState->StencilData) 
		|| (pStencilData == 0 && !m_DepthStencilState) )
	{// if the same state is already applied, just return 0.
		return 0;
	}


	gpod_ptr<SDepthStencilState> pDepthStencilItem, pNewDepthStencil;
	pDepthStencilItem.create();
	pDepthStencilItem->StencilData = gacquire( pStencilData ); // assign to new structure for management

	// get active stencil state and clone to new desc
	if( m_DepthStencilState.valid() )
		pDepthStencilItem->StencilData = gacquire( m_DepthStencilState->StencilData );

	if( 0 == pDepthStencilItem->DepthData )
	{
		gcreate( &pDepthStencilItem->DepthData );
		gget(pDepthStencilItem->DepthData)->WriteEnable		= true;
		gget(pDepthStencilItem->DepthData)->Operator		= GOPERATOR_ALWAYS;
		gget(pDepthStencilItem->DepthData)->WriteMask		= 1; // ?
	}

	pNewDepthStencil				= m_pDefaultManager->getDepthStencilStateItem( pDepthStencilItem.get_pointer() );
	if( !pNewDepthStencil )
	{
		error_printf("FAILED to create depth stencil state item! CGD3D9ResourceManager::getDepthStencilStateItem() returned NULL!");
		return -1;
	}

	if( m_DepthStencilState == pNewDepthStencil )
		return 0; // this state is already set

	pDepthStencilItem			= m_DepthStencilState;	// store old pointer to release later
	m_DepthStencilState		= pNewDepthStencil;		// assign the managed pointer
	if( pDepthStencilItem.valid() )
		pDepthStencilItem->bActive = false;
	return 0;
};

int32_t _CGD3D9Device::SetImageWrapMode( uint32_t nUsageIndex, GWRAP_MODE Mode )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetImageWrapMode" );
	if( !m_DeviceData )
		return G_NOTREADY;

	if( 0 == m_DeviceData->pDevice )
		return G_NOTAVAILABLE;

	uint8_t d3dMode = 0;
	if( Mode & GWRAPMODE_U )
		d3dMode |= D3DWRAP_U;
	if( Mode & GWRAPMODE_V )
		d3dMode |= D3DWRAP_V;
	if( Mode & GWRAPMODE_W )
		d3dMode |= D3DWRAP_W;
	if( Mode & GWRAPMODE_3 )
		d3dMode |= D3DWRAPCOORD_3;

	uint8_t rsindex = ( nUsageIndex<8 ) ? ( D3DRS_WRAP0+nUsageIndex ) : ( D3DRS_WRAP8+(nUsageIndex-8) );
	return m_DeviceData->pDevice->SetRenderState( (D3DRENDERSTATETYPE)rsindex, d3dMode );
};

GWRAP_MODE _CGD3D9Device::GetImageWrapMode( uint32_t nUsageIndex ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetImageWrapMode" );
	if( nUsageIndex > 15 
		|| !m_DeviceData )
		return GWRAPMODE_DEFAULT;
	GWRAP_MODE result = 0;
	BYTE rsindex = ( nUsageIndex<8 ) ? ( D3DRS_WRAP0+nUsageIndex ) : ( D3DRS_WRAP8-8+nUsageIndex );
	BYTE state = m_DeviceData->nRenderState[rsindex]; 
	if( 0 != ( state & D3DWRAP_U ) ) result |= GWRAPMODE_U;
	if( 0 != ( state & D3DWRAP_V ) ) result |= GWRAPMODE_V;
	if( 0 != ( state & D3DWRAP_W ) ) result |= GWRAPMODE_W;
	if( 0 != ( state & D3DWRAPCOORD_3 ) ) result |= GWRAPMODE_3;
	return result;
};

// Sets alpha mode.
int32_t _CGD3D9Device::EnableTargetAreaTest( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "EnableTargetAreaTest" );
	if( !m_DeviceData )
	{
		warning_printf("function called without the device properly initialized!");
		return G_NOTREADY;
	}
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
	{
		warning_printf("No current D3D9 device available to enable scissor test!");
		return G_NOTAVAILABLE;
	}
	int err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE ) ) )
	{
		error_printf("Cannot enable scissor test!");
		return err;
	}
	m_DeviceData->nRenderState[D3DRS_SCISSORTESTENABLE] = TRUE;
	return err;
};

error_t _CGD3D9Device::DisableTargetAreaTest( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "DisableTargetAreaTest" );
	if( !m_DeviceData )
	{
		warning_printf("function called without the device properly initialized!");
		return G_NOTREADY;
	}
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
	{
		warning_printf("No current D3D9 device available to disable scissor test!");
		return G_NOTAVAILABLE;
	}
	HRESULT err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE ) ) )
	{
		error_printf("Cannot disable scissor test!");
		return err;
	}
	m_DeviceData->nRenderState[D3DRS_SCISSORTESTENABLE] = FALSE;
	return err;
};

// Sets alpha mode.
int32_t _CGD3D9Device::EnableAlphaBlend( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "EnableAlphaBlend" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_ALPHABLENDENABLE] = TRUE;
	return err;
};

int32_t _CGD3D9Device::DisableAlphaBlend( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "DisableAlphaBlend" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_ALPHABLENDENABLE] = FALSE;
	return err;
};

int32_t _CGD3D9Device::EnableAlphaTest( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "EnableAlphaTest" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_ALPHATESTENABLE] = TRUE;
	return err;
}; 

int32_t _CGD3D9Device::DisableAlphaTest( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "DisableAlphaTest" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_ALPHATESTENABLE] = FALSE;
	return err;
};

int32_t _CGD3D9Device::SetAlphaReferenceValue( uint32_t nRef )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetAlphaReferenceValue" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	if( nRef > 255 )
		return G_UNSUPPORTED;
	int err;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ALPHAREF, nRef ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_ALPHAREF] = nRef;
	return err;
};

int32_t _CGD3D9Device::SetAlphaBlendOperator( GLOGICAL_OPERATOR nOperator )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetAlphaBlendOperator" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;
	D3DCMPFUNC reset;
	switch( nOperator )
	{
	case GOPERATOR_EQUAL: reset = D3DCMP_EQUAL;
		break;
	case GOPERATOR_GREATEREQUAL: reset = D3DCMP_GREATEREQUAL;
		break;
	case GOPERATOR_LESSEQUAL: reset = D3DCMP_LESSEQUAL;
		break;
	case GOPERATOR_NEVER: reset = D3DCMP_NEVER;
		break;
	case GOPERATOR_ALWAYS: reset = D3DCMP_ALWAYS;
		break;
	case GOPERATOR_LESS: reset = D3DCMP_LESS;
		break;
	case GOPERATOR_GREATER: reset = D3DCMP_GREATER;
		break;
	default:
		return G_INVALIDARGUMENT;
	}
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ALPHAFUNC, reset ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_ALPHAFUNC] = reset;
	return err;
};

int32_t _CGD3D9Device::SetBlendOperator( GLOGICAL_OPERATOR nOperator )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetBlendOperator" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;
	D3DBLENDOP reset;
	switch( nOperator )
	{
	case GOPERATOR_ADD: reset = D3DBLENDOP_ADD;
		break;
	case GOPERATOR_SUBSTRACT: reset = D3DBLENDOP_SUBTRACT;
		break;
	case GOPERATOR_LESS: reset = D3DBLENDOP_MIN;
		break;
	case GOPERATOR_GREATER: reset = D3DBLENDOP_MAX;
		break;
	case GOPERATOR_REVSUBSTRACT: reset = D3DBLENDOP_REVSUBTRACT;
		break;
	default:
		return G_INVALIDARGUMENT;
	}
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_BLENDOP, reset ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_BLENDOP] = reset;
	return err;
}

int32_t _CGD3D9Device::SetSrcBlendValue( GINTERPOLATION_VALUE nValue )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetSrcBlendValue" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;

	D3DBLEND reset;
	switch( nValue )
	{
	case GINTERPOLATION_0: reset = D3DBLEND_ZERO;
		break;
	case GINTERPOLATION_1: reset = D3DBLEND_ONE;
		break;
	case GINTERPOLATION_SOURCECOLOR: reset = D3DBLEND_SRCCOLOR;
		break;
	case GINTERPOLATION_INVSOURCECOLOR: reset = D3DBLEND_INVSRCCOLOR;
		break;
    case GINTERPOLATION_DESTCOLOR: reset = D3DBLEND_DESTCOLOR;
		break;
    case GINTERPOLATION_INVDESTCOLOR: reset = D3DBLEND_INVDESTCOLOR;
		break;
	case GINTERPOLATION_SOURCEALPHA: reset = D3DBLEND_SRCALPHA;
		break;
	case GINTERPOLATION_INVSOURCEALPHA: reset = D3DBLEND_INVSRCALPHA;
		break;
    case GINTERPOLATION_SOURCEALPHASAT: reset = D3DBLEND_SRCALPHASAT;
		break;
    case GINTERPOLATION_BOTHSOURCEALPHA: reset = D3DBLEND_BOTHSRCALPHA;
		break;
    case GINTERPOLATION_BOTHINVSOURCEALPHA: reset = D3DBLEND_BOTHINVSRCALPHA;
		break;
	case GINTERPOLATION_DESTALPHA: reset = D3DBLEND_DESTALPHA;
		break;
    case GINTERPOLATION_INVDESTALPHA: reset = D3DBLEND_INVDESTALPHA;
		break;
	default:
		return G_INVALIDARGUMENT;
	}
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_SRCBLEND, reset ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_SRCBLEND] = reset;
	return err;

};  

int32_t _CGD3D9Device::SetDstBlendValue( GINTERPOLATION_VALUE nValue )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetDstBlendValue" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;

	D3DBLEND reset;
	switch( nValue )
	{
	case GINTERPOLATION_0:
		reset = D3DBLEND_ZERO;
		break;
	case GINTERPOLATION_1:
		reset = D3DBLEND_ONE;
		break;
	case GINTERPOLATION_SOURCECOLOR:
		reset = D3DBLEND_SRCCOLOR;
		break;
	case GINTERPOLATION_INVSOURCECOLOR:
		reset = D3DBLEND_INVSRCCOLOR;
		break;
    case GINTERPOLATION_DESTCOLOR:
		reset = D3DBLEND_DESTCOLOR;
		break;
    case GINTERPOLATION_INVDESTCOLOR:
		reset = D3DBLEND_INVDESTCOLOR;
		break;
	case GINTERPOLATION_SOURCEALPHA:
		reset = D3DBLEND_SRCALPHA;
		break;
	case GINTERPOLATION_INVSOURCEALPHA:
		reset = D3DBLEND_INVSRCALPHA;
		break;
    case GINTERPOLATION_SOURCEALPHASAT:
		reset = D3DBLEND_SRCALPHASAT;
		break;
    case GINTERPOLATION_BOTHSOURCEALPHA:
		reset = D3DBLEND_BOTHSRCALPHA;
		break;
    case GINTERPOLATION_BOTHINVSOURCEALPHA:
		reset = D3DBLEND_BOTHINVSRCALPHA;
		break;
	case GINTERPOLATION_DESTALPHA:
		reset = D3DBLEND_DESTALPHA;
		break;
    case GINTERPOLATION_INVDESTALPHA:
		reset = D3DBLEND_INVDESTALPHA;
		break;
	default:
		return G_INVALIDARGUMENT;
	}
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_DESTBLEND, reset ) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_DESTBLEND] = reset;
	return err;

};  

// Stencil buffer
int32_t _CGD3D9Device::EnableZBuffer( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "EnableZBuffer" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ZENABLE, TRUE ) ) )
	{
		error_printf("Failed to enable Z buffer!");
		return err;
	}
	m_DeviceData->nRenderState[D3DRS_ZENABLE] = TRUE;
	return err;
};

int32_t _CGD3D9Device::DisableZBuffer( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "DisableZBuffer" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	int err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ZENABLE, FALSE ) ) )
	{
		error_printf("Failed to disable Z buffer!");
		return err;
	}
	m_DeviceData->nRenderState[D3DRS_ZENABLE] = FALSE;
	return err;
}; 

int32_t _CGD3D9Device::EnableZWrite( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "EnableZWrite" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	HRESULT err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) ) )
	{
		error_printf("Failed to enable Z write!");
		return err;
	}
	m_DeviceData->nRenderState[D3DRS_ZWRITEENABLE] = TRUE;
	return 0;
}; 

int32_t _CGD3D9Device::DisableZWrite( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "DisableZWrite" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	HRESULT err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE ) ) )
	{
		error_printf("Failed to disable Z write!");
		return err;
	}
	m_DeviceData->nRenderState[D3DRS_ZWRITEENABLE] = FALSE;
	return 0;
};

int32_t _CGD3D9Device::EnableStencilBuffer( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "EnableStencilBuffer" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	HRESULT err = 0;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE ) ) )
	{
		error_printf("Failed to enable stencil!");
		return err;
	}
	m_DeviceData->nRenderState[D3DRS_STENCILENABLE] = TRUE;
	return 0;
};

int32_t _CGD3D9Device::DisableStencilBuffer( void )
{
	GPROFILE_METHOD( "_CGD3D9Device", "DisableStencilBuffer" );
	if( !m_DeviceData )
		return G_NOTREADY;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	
	if( 0 == pDevice )
		return G_NOTAVAILABLE;
	HRESULT err;
	if( 0 > ( err = pDevice->SetRenderState( D3DRS_STENCILENABLE, FALSE ) ) )
	{
		error_printf("Failed to disable stencil!");
		return err;
	}
	m_DeviceData->nRenderState[D3DRS_STENCILENABLE] = FALSE;
	return 0;
};

int32_t _CGD3D9Device::SetStencilBufferData( const GSTENCIL1_DATA* pStencilData )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetStencilBufferData" );
	if( !m_DeviceData )
		return G_NOTREADY;

	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;	

	if( 0 == pDevice )
		return G_NOTAVAILABLE;

	int err=0;

	switch( pStencilData->CmpOperator )
	{
	case GOPERATOR_ALWAYS:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILFUNC] = D3DCMP_ALWAYS;
		break;
	case GOPERATOR_NEVER:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NEVER) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILFUNC] = D3DCMP_NEVER;
		break;
	case GOPERATOR_EQUAL:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILFUNC] = D3DCMP_EQUAL;
		break;
	default:
		return G_INVALIDARGUMENT;
	}
	if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILREF, pStencilData->dwRef) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_STENCILREF] = pStencilData->dwRef;
	if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILMASK, pStencilData->dwMask) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_STENCILFUNC] = pStencilData->dwMask;
	if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, pStencilData->dwWriteMask) ) )
		return err;
	m_DeviceData->nRenderState[D3DRS_STENCILFUNC] = pStencilData->dwWriteMask;

	switch( pStencilData->ZFail )
	{
	case GOPERATIONTYPE_KEEP:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILZFAIL] = D3DSTENCILOP_KEEP;
		break;
	case GOPERATIONTYPE_REPLACE:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILZFAIL] = D3DSTENCILOP_REPLACE;
		break;
	}
	switch( pStencilData->Fail )
	{
	case GOPERATIONTYPE_KEEP:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILFAIL] = D3DSTENCILOP_KEEP;
		break;
	case GOPERATIONTYPE_REPLACE:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_REPLACE) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILFAIL] = D3DSTENCILOP_REPLACE;
		break;
	}
	switch( pStencilData->Pass )
	{
	case GOPERATIONTYPE_KEEP:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILPASS] = D3DSTENCILOP_KEEP;
		break;
	case GOPERATIONTYPE_REPLACE:
		if( 0 > ( err = pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE) ) )
			return err;
		m_DeviceData->nRenderState[D3DRS_STENCILPASS] = D3DSTENCILOP_REPLACE;
		break;
	}
	return err;
};

// 
int32_t _CGD3D9Device::SetBackfaceCulling( GCULL_MODE nMode )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetBackfaceCulling" );
	if( !m_DeviceData )
		return G_NOTREADY;

	if( 0 == m_DeviceData->pDevice )
		return G_NOTAVAILABLE;

	int32_t err = 0;
	switch( nMode )
	{
	default:
		return G_INVALIDARGUMENT;
	case GCULLMODE_NONE:
		err = m_DeviceData->pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_CULLMODE] = D3DCULL_NONE;
		break;
	case GCULLMODE_CCW:
		err = m_DeviceData->pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_CULLMODE] = D3DCULL_CCW;
		break;
	case GCULLMODE_CW:
		err = m_DeviceData->pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_CULLMODE] = D3DCULL_CW;
		break;
	};
	return err;
};
// 
error_t _CGD3D9Device::SetFillMode( GFILL_MODE nMode )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetBackfaceCulling" );
	if( !m_DeviceData )
		return G_NOTREADY;

	if( 0 == m_DeviceData->pDevice )
		return G_NOTAVAILABLE;

	int32_t err = 0;
	switch( nMode )
	{
	default:
		return G_INVALIDARGUMENT;
	case GFILLMODE_SOLID:
		err = m_DeviceData->pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_FILLMODE] = D3DFILL_SOLID;
		break;
	case GFILLMODE_WIREFRAME:
		err = m_DeviceData->pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_FILLMODE] = D3DFILL_WIREFRAME;
		break;
	case GFILLMODE_POINT:
		err = m_DeviceData->pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_POINT );
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_FILLMODE] = D3DFILL_POINT;
		break;
	};
	return err;
};
int32_t _CGD3D9Device::SetDepthBufferMode( GRESOURCE_MODE nMode )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetDepthBufferMode" );
	if( !m_DeviceData )
		return G_NOTREADY;

	if( 0 == m_DeviceData->pDevice )
		return G_NOTAVAILABLE;

	int32_t err = 0, res;
	IDirect3DDevice9* pDevice = m_DeviceData->pDevice;
	if( 0 == nMode || 
		( GRESMODE_DEVICE_READ != (nMode & GRESMODE_DEVICE_READ)
			&& GRESMODE_DEVICE_WRITE != (nMode & GRESMODE_DEVICE_WRITE)
		) 
	) 
	{
		res |= err = pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_ZENABLE] = D3DZB_FALSE;
		res |= err = pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_ZWRITEENABLE] = FALSE;
		return err;
	}
	if( GRESMODE_DEVICE_READ == (nMode & GRESMODE_DEVICE_READ) )
	{
		res |= err = pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_ZENABLE] = D3DZB_TRUE; // que caca es esto de d3dzb-true es cualquiera desde mi punto de vista
	};
	if( GRESMODE_DEVICE_WRITE == (nMode & GRESMODE_DEVICE_WRITE) ) //|| (nMode & GRESMODE_HOST_WRITE) )
	{
		res |= err = pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_ZWRITEENABLE] = TRUE; // que caca es esto de d3dzb-true es cualquiera desde mi punto de vista
	}
	else
	{
		res |= err = pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		if( 0 == err )
			m_DeviceData->nRenderState[D3DRS_ZWRITEENABLE] = FALSE; // que caca es esto de d3dzb-true es cualquiera desde mi punto de vista
	}

	return res;
}; 

bool _CGD3D9Device::GetAlphaBlendEnabled( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetAlphaBlendEnabled" );
	if( !m_DeviceData )
		return false;
	return ( TRUE == m_DeviceData->nRenderState[D3DRS_ALPHABLENDENABLE] );
}; 

bool _CGD3D9Device::GetAlphaTestEnabled( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetAlphaTestEnabled" );
	if( !m_DeviceData )
		return false;
	return ( TRUE == m_DeviceData->nRenderState[D3DRS_ALPHATESTENABLE] );
};  

uint32_t _CGD3D9Device::GetAlphaReferenceValue( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetAlphaReferenceValue" );
	if( !m_DeviceData )
		return false;
	return m_DeviceData->nRenderState[D3DRS_ALPHAREF];
};

GLOGICAL_OPERATOR _CGD3D9Device::GetBlendOperator( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetBlendOperator" );
	if( !m_DeviceData )
		return GOPERATOR_ALWAYS;
	GLOGICAL_OPERATOR reset;
	switch( m_DeviceData->nRenderState[D3DRS_BLENDOP] )
	{
	default:
	case D3DCMP_ALWAYS: reset = GOPERATOR_ALWAYS;
		break;
	case D3DCMP_NEVER: reset = GOPERATOR_NEVER;
		break;
	case D3DCMP_EQUAL: reset = GOPERATOR_EQUAL;
		break;
	case D3DCMP_LESS: reset = GOPERATOR_LESS;
		break;
	case D3DCMP_GREATER: reset = GOPERATOR_GREATER;
		break;
	case D3DCMP_GREATEREQUAL: reset = GOPERATOR_GREATEREQUAL;
		break;
	case D3DCMP_LESSEQUAL: reset = GOPERATOR_LESSEQUAL;
		break;
	}
	return reset;
};  

GINTERPOLATION_VALUE _CGD3D9Device::GetSrcBlendValue( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetSrcBlendValue" );
	if( !m_DeviceData )
		return 0;
	GINTERPOLATION_VALUE reset=0;
	switch( m_DeviceData->nRenderState[D3DRS_SRCBLEND] )
	{
	default:
	case D3DBLEND_ZERO:
		reset = GINTERPOLATION_0;
		break;
	case D3DBLEND_ONE:
		reset = GINTERPOLATION_1;
		break;
	case D3DBLEND_SRCCOLOR:
		reset = GINTERPOLATION_SOURCECOLOR;
		break;
	case D3DBLEND_INVSRCCOLOR:
		reset = GINTERPOLATION_INVSOURCECOLOR;
		break;
    case D3DBLEND_DESTCOLOR:
		reset = GINTERPOLATION_DESTCOLOR;
		break;
    case D3DBLEND_INVDESTCOLOR:
		reset = GINTERPOLATION_INVDESTCOLOR;
		break;
	case D3DBLEND_SRCALPHA:
		reset = GINTERPOLATION_SOURCEALPHA;
		break;
	case D3DBLEND_INVSRCALPHA:
		reset = GINTERPOLATION_INVSOURCEALPHA;
		break;
    case D3DBLEND_SRCALPHASAT:
		reset = GINTERPOLATION_SOURCEALPHASAT;
		break;
    case D3DBLEND_BOTHSRCALPHA:
		reset = GINTERPOLATION_BOTHSOURCEALPHA;
		break;
    case D3DBLEND_BOTHINVSRCALPHA:
		reset = GINTERPOLATION_BOTHINVSOURCEALPHA;
		break;
	case D3DBLEND_DESTALPHA:
		reset = GINTERPOLATION_DESTALPHA;
		break;
    case D3DBLEND_INVDESTALPHA:
		reset = GINTERPOLATION_INVDESTALPHA;
		break;
	}
	return reset;
};

GINTERPOLATION_VALUE _CGD3D9Device::GetDstBlendValue( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetDstBlendValue" );
	if( !m_DeviceData )
		return 0;
	GINTERPOLATION_VALUE reset=0;
	switch( m_DeviceData->nRenderState[D3DRS_DESTBLEND] )
	{
	default:
	case D3DBLEND_ZERO:
		reset = GINTERPOLATION_0;
		break;
	case D3DBLEND_ONE:
		reset = GINTERPOLATION_1;
		break;
	case D3DBLEND_SRCCOLOR:
		reset = GINTERPOLATION_SOURCECOLOR;
		break;
	case D3DBLEND_INVSRCCOLOR:
		reset = GINTERPOLATION_INVSOURCECOLOR;
		break;
    case D3DBLEND_DESTCOLOR:
		reset = GINTERPOLATION_DESTCOLOR;
		break;
    case D3DBLEND_INVDESTCOLOR:
		reset = GINTERPOLATION_INVDESTCOLOR;
		break;
	case D3DBLEND_SRCALPHA:
		reset = GINTERPOLATION_SOURCEALPHA;
		break;
	case D3DBLEND_INVSRCALPHA:
		reset = GINTERPOLATION_INVSOURCEALPHA;
		break;
    case D3DBLEND_SRCALPHASAT:
		reset = GINTERPOLATION_SOURCEALPHASAT;
		break;
    case D3DBLEND_BOTHSRCALPHA:
		reset = GINTERPOLATION_BOTHSOURCEALPHA;
		break;
    case D3DBLEND_BOTHINVSRCALPHA:
		reset = GINTERPOLATION_BOTHINVSOURCEALPHA;
		break;
	case D3DBLEND_DESTALPHA:
		reset = GINTERPOLATION_DESTALPHA;
		break;
    case D3DBLEND_INVDESTALPHA:
		reset = GINTERPOLATION_INVDESTALPHA;
		break;
	}
	return reset;
};

GRESOURCE_MODE _CGD3D9Device::GetDepthBufferMode( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetDepthBufferMode" );
	if( !m_DeviceData )
		return 0;
	GRESOURCE_MODE result = 0;
	if( TRUE == m_DeviceData->nRenderState[D3DRS_ZENABLE] )
		result |= GRESMODE_DEVICE_READ;
	if( TRUE == m_DeviceData->nRenderState[D3DRS_ZWRITEENABLE] )
		result |= GRESMODE_DEVICE_WRITE;
	return result;
};

GCULL_MODE _CGD3D9Device::GetBackfaceCulling(  void ) const
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetBackfaceCulling" );
	if( !m_DeviceData )
		return GCULLMODE_CCW;
	switch( m_DeviceData->nRenderState[D3DRS_CULLMODE] )
	{
	default:
	case D3DCULL_CCW:
		return GCULLMODE_CCW;
	case D3DCULL_CW:
		return GCULLMODE_CW;
	case D3DCULL_NONE:
		return GCULLMODE_NONE;
	}
};  

