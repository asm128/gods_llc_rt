//#define GPROFILE_ENABLED
//
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

GRDS(SRasterizerState) _CGD3D9ResourceManager::getRasterizerStateItem( GRDS(SRasterizerState) pRasterizerStateItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getRasterizerStateItem" );
	if( 0 == pRasterizerStateItem )
	{
		error_printf("getRasterizerStateItem() received NULL as _GMANAGER_RASTERIZERSTATE_DATA*!");
		return 0;
	}

	GRDS(SRasterizer) pDesc1 = gget(pRasterizerStateItem)->RasterizerData, 
		pDescOther;

	GRDS(SRasterizerState) pNewItem = 0; // store here the return value
	uint32_t i, n = m_lstManagerRasterizerState.size(); // set some counters
	for( i=0; i<n; i++ )
	{
		pNewItem = m_lstManagerRasterizerState.get_pointer()[i];
		if( 0 == pNewItem )
		{
			error_printf("CGD3D9ResourceManager::m_lstpRasterizerStateItem[%u] = NULL! This means the list wasn't managed properly as it should be a stack of non-NULL pointers!", i);
			continue;
		}
		pDescOther = gget(pNewItem)->RasterizerData;
		if(	0 == gcompare( pDesc1, pDescOther ) )
			break;  // found managed data with the same data? returning this one.
		else
			pNewItem = 0;
	}
	if( 0 == pNewItem )
	{
		if( __createDXRasterizerState( gget(pRasterizerStateItem) ) )
		{ // don't acquire if failed to create
			error_printf("__createDXRasterizerState() FAILED!");
			return 0;
		}
		if(  m_lstManagerRasterizerState.push( pRasterizerStateItem ) )
		{
			error_printf("Resource manager FAILED! when trying to append new rasterizer state item to list.\n"
				"This may occur if the system ran out of memory. We will try to continue as the resource may not live enough to require management.");
		};
		pNewItem = pRasterizerStateItem;
	}
	else
	{
		info_printf("Found _GMANAGER_RASTERIZERSTATE_DATA that matches requested data. Resource will be mapped instead of created.");
	}
	
	return gacquire( pNewItem );

};


int32_t _CGD3D9ResourceManager::__createDXRasterizerState( SRasterizerState* pRasterizerStateItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXRasterizerState" );
	if( 0 == pRasterizerStateItem )
	{
		error_printf( "__createDXRasterizerState() FAILED! GRDS(SMANAGER_RASTERIZERSTATE) is NULL!");
		return G_FAIL;
	}

//	HRESULT hr=NULL;
//	ID3D11DeviceContext* pContext = m_pDevice->getD3D11DeviceContext();
//	ID3D11Device* pDevice = m_pDevice->getD3D11Device();
//
//	_GRASTERIZER_DATA* pRasterizerData = pRasterizerStateItem->RasterizerData;
//	pRasterizerStateItem->d3dRasterizerDesc.AntialiasedLineEnable	= pRasterizerData->AntialiasingEnabled ? 1 : 0;
//	pRasterizerStateItem->d3dRasterizerDesc.CullMode				= GD3DCullModeFromGCullMode( pRasterizerData->CullMode );
//	pRasterizerStateItem->d3dRasterizerDesc.DepthBias				= pRasterizerData->DepthBias;
//	pRasterizerStateItem->d3dRasterizerDesc.DepthBiasClamp			= pRasterizerData->DepthBiasClamp;
//	pRasterizerStateItem->d3dRasterizerDesc.DepthClipEnable			= pRasterizerData->DepthClipEnable;
//	pRasterizerStateItem->d3dRasterizerDesc.FillMode				= GD3DFillModeFromGFillMode( pRasterizerData->FillMode );
//	pRasterizerStateItem->d3dRasterizerDesc.FrontCounterClockwise	= pRasterizerData->FrontCounterClockwise;
//	pRasterizerStateItem->d3dRasterizerDesc.MultisampleEnable		= pRasterizerData->MultisampleEnabled;
//	pRasterizerStateItem->d3dRasterizerDesc.ScissorEnable			= pRasterizerData->ScissorEnable;
//	pRasterizerStateItem->d3dRasterizerDesc.SlopeScaledDepthBias	= pRasterizerData->SlopeScaledDepthBias;
//	
//	if( D3D_OK != ( hr = pDevice->CreateRasterizerState(&pRasterizerStateItem->d3dRasterizerDesc, &pRasterizerStateItem->pD3DRasterizerState) ) ) 
//	{
//		error_printf("pDevice->CreateRasterizerState() FAILED! HRESULT=0x%X", (uint32_t)hr );
//		return G_FAIL;
//	}
	info_printf("This function is dummy as rasterizer states are handled different for D3D9.");
	return S_OK;
};

