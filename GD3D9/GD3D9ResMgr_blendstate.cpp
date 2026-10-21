
//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

GRDS(SBlendState) _CGD3D9ResourceManager::getBlendStateItem( GRDS(SBlendState) pBlendStateItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getBlendStateItem" );
	if( 0 == pBlendStateItem )
	{
		error_printf("getBlendStateItem() received NULL as _GMANAGER_BLENDSTATE_DATA*!");
		return 0;
	}

	GRDS(SBlend) pBlendDesc1 = gget(pBlendStateItem)->BlendData, 
		pBlendDescOther;

	GRDS(SBlendState) pNewItem = 0; // store here the return value
	uint32_t i, n = m_lstManagerBlendState.size(); // set some counters
	for( i=0; i<n; i++ )
	{
		pNewItem = m_lstManagerBlendState.get_pointer()[i];
		if( 0 == pNewItem )
		{
			error_printf("_GMANAGER_BLENDSTATE_DATA* is NULL at m_lstpBlendStateItem[i=%u]!", i);
			continue;
		}
		pBlendDescOther = gget(pNewItem)->BlendData;
		if( 0 == gcompare( pBlendDesc1, pBlendDescOther ) ) // gcompareData() performs both pointer comparison and memcmp()
		{
			render_printf( "_GMANAGER_BLENDSTATE_DATA Match!" );
			break;
		}
		else
		{
			pNewItem = 0;
		}
	}
	if( 0 == pNewItem )
	{
		if( __createDXBlendState( gget(pBlendStateItem) ) )
		{ // don't acquire if failed to create
			error_printf("__createDXBlendState() FAILED!");
			return 0;
		}
		if( m_lstManagerBlendState.push( pBlendStateItem ) )
		{
			error_printf("Resource manager FAILED! when trying to append new blend state item to list.\n"
				"This may occur if the system ran out of memory. We will try to continue as the resource may not live enough to require management.");
		};
		pNewItem = pBlendStateItem;
	}
	else
	{
		info_printf("Found _GMANAGER_BLENDSTATE_DATA that matches requested data. Resource will be mapped instead of created.");
	}
	
	return gacquire( pNewItem );

};

int32_t _CGD3D9ResourceManager::__createDXBlendState( SBlendState* pBlendStateItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXBlendState" );
	if( 0 == pBlendStateItem )
	{
		error_printf( "__createDXBlendState() FAILED! _GMANAGER_BLENDSTATE_DATA* is NULL!");
		return G_FAIL;
	}

//	HRESULT hr=NULL;
//	ID3D11DeviceContext* pContext = m_pDevice->getD3D11DeviceContext();
//	ID3D11Device* pDevice = m_pDevice->getD3D11Device();
//
//	GBLEND_DATA* pBlendData = pBlendStateItem->BlendData;
//
//	pBlendStateItem->d3dBlendDesc.AlphaToCoverageEnable					= pBlendData->AlphaToCoverageEnable;
//	pBlendStateItem->d3dBlendDesc.IndependentBlendEnable				= pBlendData->IndependentBlendEnable;
//	pBlendStateItem->d3dBlendDesc.RenderTarget[0].BlendEnable			= pBlendData->BlendEnable;
//	pBlendStateItem->d3dBlendDesc.RenderTarget[0].BlendOp				= GD3DBlendOpFromGArithmeticOperator( pBlendData->BlendOp );
//	pBlendStateItem->d3dBlendDesc.RenderTarget[0].BlendOpAlpha			= GD3DBlendOpFromGArithmeticOperator( pBlendData->BlendOpAlpha );
//	pBlendStateItem->d3dBlendDesc.RenderTarget[0].DestBlend				= GD3DBlendFromGInterpolationValue( pBlendData->DestBlend );
//	pBlendStateItem->d3dBlendDesc.RenderTarget[0].DestBlendAlpha		= GD3DBlendFromGInterpolationValue( pBlendData->DestBlendAlpha );
//	pBlendStateItem->d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask	= GD3DBlendFromGInterpolationValue( pBlendData->RenderTargetWriteMask );
//	pBlendStateItem->d3dBlendDesc.RenderTarget[0].SrcBlend				= GD3DBlendFromGInterpolationValue( pBlendData->SrcBlend );
//	pBlendStateItem->d3dBlendDesc.RenderTarget[0].SrcBlendAlpha			= GD3DBlendFromGInterpolationValue( pBlendData->SrcBlendAlpha );
//
//	if( D3D_OK != ( hr = pDevice->CreateBlendState( &pBlendStateItem->d3dBlendDesc, &pBlendStateItem->pD3DBlendState) ) ) 
//	{
//		error_printf("pDevice->CreateBlendState() FAILED! HRESULT=0x%X", (uint32_t)hr );
//		return G_FAIL;
//	}
	info_printf("This function is dummy as blend states are handled different for D3D9.");
	return S_OK;
};
