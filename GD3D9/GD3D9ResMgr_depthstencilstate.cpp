
//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include "GCore_printf.h"
#include "GDebugger_core.h"


using namespace god;

GRDS(SDepthStencilState) _CGD3D9ResourceManager::getDepthStencilStateItem( GRDS(SDepthStencilState) pDepthStencilStateItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getDepthStencilStateItem" );
	if( 0 == pDepthStencilStateItem )
	{
		error_printf("getDepthStencilStateItem() FAILED! Parameter is NULL!");
		return 0;
	}

	GRDS(SStencil) pStencilData = gget(pDepthStencilStateItem)->StencilData, pStencilDataOther;
	GRDS(SDepth) pDepthData		= gget(pDepthStencilStateItem)->DepthData, pDepthDataOther;

	GRDS(SDepthStencilState)  pNewItem = 0; // store here the return value
	uint32_t i, n = m_lstManagerDepthStencilState.size(); // set some counters
	for( i=0; i<n; i++ )
	{
		pNewItem = m_lstManagerDepthStencilState.get_pointer()[i];
		
		if( 0 == pNewItem ) // this should never happen!
		{
			error_printf("m_lstpDepthStencilStateItem[%u] = NULL! This means the list wasn't managed properly as it should be a stack of non-NULL pointers!", i);
			continue;
		}
		
		pStencilDataOther	= gget(pNewItem)->StencilData;
		pDepthDataOther		= gget(pNewItem)->DepthData;
		
		if( pDepthStencilStateItem == pNewItem || (pDepthData == pDepthDataOther && pStencilData == pStencilDataOther) )
			break; // we found a managed data that matches the requested one. Returning this one.
		

		if(	(0 == gcompare( pDepthData, pDepthDataOther ))
			&& (0 == gcompare( pStencilData, pStencilDataOther )) )
			break; // check if both depth and stencil data matches. Return this item if they do (so we don't create twice the same data).
		else
			pNewItem = 0;
	}
	if( 0 == pNewItem )
	{
		if( __createDXDepthStencilState( gget(pDepthStencilStateItem) ) )
		{ // don't acquire if failed to create
			error_printf("__createDXDepthStencilState() FAILED!");
			return 0;
		}
		if( m_lstManagerDepthStencilState.push( pDepthStencilStateItem ) )
		{ // failing to append to manager list means that the life of the resource will be determined by the behaviour of it's holder.
			error_printf("Resource manager FAILED! when trying to append new depth stencil state item to list.\n"
				"This may occur if the system ran out of memory. We will try to continue as the resource may not live enough to require management.");
			grelease( &pDepthStencilStateItem );
			return 0;
		}
		pNewItem = pDepthStencilStateItem;
	}
	else
	{
		info_printf("Found _GMANAGER_DEPTHSTENCILSTATE_DATA that matches requested data. Resource will be mapped instead of created.");
	}
	return gacquire( pNewItem );

};

int32_t _CGD3D9ResourceManager::__createDXDepthStencilState( SDepthStencilState* pDepthStencilStateItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXDepthStencilState" );
	if( 0 == pDepthStencilStateItem )
	{
		error_printf( "__createDXDepthStencilState() FAILED! parameter is NULL!");
		return G_FAIL;
	}

	info_printf("This function is dummy because depth stencil state is handled different for D3D9. Look at D3D11 implementation for more information on this.");
	return S_OK;
};

