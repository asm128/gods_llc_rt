
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"


//#define GPROFILE_ENABLED
#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

GRDS(SDecl) _CGD3D9ResourceManager::getDeclItem( GRDS(SDecl) pDeclItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getDeclItem" );
	if( 0 == pDeclItem )
		return 0;
	GRDS(SDecl) pNewItem = 0; // store here the return value

	D3DVERTEXELEMENT9* pDecl=0, * pDecl2=0;
	pDecl = &gget(pDeclItem)->VertexElement[0];
	uint32_t nElementCount  = gget(pDeclItem)->nElementCount;

	uint32_t i, j, n = m_lstManagerDecl.size(); // set some counters
	for( i=0; i<n; i++ )
	{
		pNewItem = m_lstManagerDecl.get_pointer()[i];

		//if( 0 == pNewItem )
		//	continue;

		pDecl2 = &gget(pNewItem)->VertexElement[0];
		j=0;
		// Did we reach D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}?
		while( ( pDecl[j].Stream != 0xff ) && 
			( pDecl2[j].Stream != 0xff ) && (j < nElementCount))
		{
			if( pDecl[j].Stream	!= pDecl2[j].Stream ||
				pDecl[j].Method	!= pDecl2[j].Method ||
				pDecl[j].Offset	!= pDecl2[j].Offset ||
				pDecl[j].Type	!= pDecl2[j].Type ||
				pDecl[j].Usage	!= pDecl2[j].Usage ||
				pDecl[j].UsageIndex	!= pDecl2[j].UsageIndex 
				)
			{
				pNewItem = 0;
				break;
			}
			j++;
			if( 0 == pNewItem )
				continue;
		}
		// if we reached the end of the existing decl, continue loop
		if( pDecl[j].Stream != 0xff || pDecl2[j].Stream != 0xff  )
		{
			//&&  ( (*iRef)->eVertexElements[i].Stream == 0xff ) )
			pNewItem = 0;
			continue; 
		}
		if( 0 != pNewItem )
			break;
	}
	if( 0 == pNewItem )
	{
		if( __createDXDecl( gget(pDeclItem) ) )
		{ // don't acquire if failed to create
			return 0;
		}
		if( m_lstManagerDecl.push( pDeclItem ) )
		{
			error_printf("Cannot append to list! Out of memory?");
			grelease( &pDeclItem );
			return 0;
		};
		pNewItem = pDeclItem;
	}
	else
	{
	}
	
	return gacquire( pNewItem );

};

int32_t _CGD3D9ResourceManager::__createDXDecl( SDecl* pDeclItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXDecl" );
	HRESULT hr=NULL;
	if( 0 == m_pD3D9Device )
	{
		error_printf("NULL is not a valid device! Forgot to initialize properly?");
		return -1;
	}
	if( D3D_OK != 
		( hr = m_pD3D9Device->CreateVertexDeclaration( pDeclItem->VertexElement, &pDeclItem->pd3dDecl ) )
		) 
	{ 
		return G_FAIL; 
	}
	return S_OK;
};

