//#define GPROFILE_ENABLED
//
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include <strsafe.h>


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

__GDEFINE_FREE_MANAGER_GINTERFACE_LIST_GELEMENTARY( VideoFunction, Function );
__GDEFINE_GET_GINTERFACE_BY_ID_GELEMENTARY( VideoFunction, Function );

// We may need to modify parameters for this function for D3D11 implementation, but this will do for now
int32_t _CGD3D9ResourceManager::CreateFunction( int32_t nID, GRDS(SBuffer) pBufferData, GOSH(VideoFunction)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateFunction" );

	if( 0 == pBufferData )
	{
		error_printf("Invalid parameters! GRDS(SBuffer) is NULL!");
		return G_INVALIDARGUMENT;
	}
	else if( nID <= 0 )
	{
		error_printf("Cannot create IGVideoFunction with invalid ID (<=0).\n"
			"The ID requested was %i for buffer at address 0x%p", nID, gget(pBufferData) );
		return G_INVALIDID;
	}

	_CGD3D9Function* pFunction=0;
	GHPTR(VideoFunction) garbage;
	if( nID )
	{
		if( 0 != (garbage = pFunction = (CGD3D9Function*)this->GetFunction( nID )) )
		{	// can't create duplicated ID
			error_wprintf(L"Cannot create IGVideoFunction with duplicated ID! The ID requested was %i.\n" 
				L"for GRDS(SBuffer) at address: 0x%p", nID, pBufferData );
			return G_INVALIDID;
		}
	}
	else
		nID = getAvailableID( GIID_DEVICEIMAGE );

	pFunction = new CGD3D9Function( this );
	if( pFunction->internalInit( nID, pBufferData, 0, 0 ) )
	{
		garbage = pFunction;
		error_printf("Cannot initialize CGD3D9Function instance!");
		return G_FAIL;
	}
	info_printf("CGD3D9Function instance initialized successfully!");
	if( 0 > m_lstFunction.push( pFunction ) )
	{
		garbage = pFunction;
		error_printf("Cannot append image to management list! Out of memory?");
		return G_NOMEM;
	}

	garbage = *ppCreated;
	*ppCreated = gacquireInterface( pFunction );
	return S_OK;
};


GRDS(SFunction) _CGD3D9ResourceManager::getFunctionItem( GRDS(SFunction) pFunctionItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getFunctionItem" );
	if( 0 == pFunctionItem )
	{
		return 0;
	}
	GRDS(SFunction) pNewItem=0; // store here the return value

	wchar_t buffer2[256]; // , buffer3[256]; 
	if( gget(pFunctionItem)->FileName )
		StringCchCopyW( buffer2, 256, gget(gget(pFunctionItem)->FileName)->WText );
	else
		buffer2[0] = 0;

	uint32_t i, n = m_lstManagerFunction.size(); // set some counters
	for( i=0; i<n; i++ )
	{ // Look for this shader into our list in order to find out if it's already loaded and skip creation if duplicated.
		pNewItem = m_lstManagerFunction.get_pointer()[i];
		if( 0 == pNewItem )
		{
			error_printf("GRDS(SFunction) is NULL at m_lstpFunctionItem[i=%u]!", i);
			continue;
		}
		if( gget(pNewItem)->FileName )
		{ // try to match by file name
			if( 0 == wcscmp( buffer2, gget(gget(pNewItem)->FileName)->WText ) )
				break;
			else 
				pNewItem = 0;
		}
		else
		{
			//if( pNewItem->
		}
	}
	if( 0 == pNewItem )
	{
		if( __createDXFunction( gget(pFunctionItem) ) )
		{ // don't acquire if failed to create
			return 0;
		}
		if( m_lstManagerFunction.push( pFunctionItem ) )
		{
			error_printf("CGD3D9ResourceManager::__appendFunctionItem() FAILED! Out of memory?");
			grelease( &pFunctionItem );
		};
		pNewItem = pFunctionItem;
	}
	else
	{
	}
	return gacquire( pNewItem );
};

int32_t _CGD3D9ResourceManager::__createDXFunction( SFunction* pShaderItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXFunction" );
	if( 0 == pShaderItem )
	{
		error_printf("_CGD3D9ResourceManager::__createDXFunction() received NULL as _GMANAGER_FUNCTION_DATA*!");
		return -1;
	}
	
	IDirect3DDevice9* pd3dDevice = m_pD3D9Device;
	if( 0 == pd3dDevice )
	{
		error_printf("(IDirect3DDevice9* == NULL)! If this happens it means we're not syncing device states properly.");
		return -1;
	}
	HRESULT hr=0;
	error_t result=0;
	if( pShaderItem->SourceCode ) // if no program code we try to load from file
	{ // Try to read cached data from memory, compiled or not.
		ID3DXBuffer* compiledCode = 0;
		ID3DXBuffer* errorMsg = 0;
		GRDS(SBuffer) sourceData = pShaderItem->SourceCode;
		if( 0 == sourceData )
		{
			error_printf("_GMANAGER_FUNCTION_DATA::ProgramCode == NULL! Cannot create shader with no input code data!");
			return -1;
		}
		GRDS(SBuffer) BinaryData=0;
//		switch( functionData->nLanguage )
//		{
//		case GLANGUAGE_HLSL:
//			if( 0 > 
//				( hr = D3DXCompileShader( (char*)sourceData->pByteArray, sourceData->nSizeInBytes, 0, 0, functionData->szIdentifierName, 
//					0, 0, &compiledCode, &errorMsg, &pShaderItem->pd3dConstantTable ) ) )
//			{
//				error_printf("_CGD3D9ResourceManager::__createDXFunction(): D3DXCompileShader() FAILED!\n"
//					"Cannot compile shader from HLSL code!\n%s\nHRESULT = 0x%X", (char*)errorMsg->GetBufferPointer(), hr );
//				return -1;	
//			}
//			if( 0 > 
//				( result = gcreateBuffer( GDATATYPE_UINT8, sourceData->Usage, compiledCode->GetBufferSize(), &BinaryData ) ) 
//			)
//			{
//				error_printf("_CGD3D9ResourceManager::__createDXFunction(): Cannot compile shader from HLSL code!\n%s\nHRESULT = 0x%X", (char*)errorMsg->GetBufferPointer(), hr );
//				return -1;	
//			};
//			memcpy( BinaryData->pByteArray, compiledCode->GetBufferPointer(), compiledCode->GetBufferSize() );
//			// this is very wrong, the whole functionData should be discarded and replaced with a new one with the updated values
//			functionData->nLanguage = GLANGUAGE_BINARY;
//			grelease( &functionData->ProgramCode );
//			functionData->ProgramCode = BinaryData;
//			goto create_binary_switch;
//			break;
//		case GLANGUAGE_ASM_D3DSM:
//			break; // how to compile shader assembly?
//		case GLANGUAGE_BINARY:
//create_binary_switch:
//			if( GUSAGE_PIXELSHADER == functionData->ProgramCode->Usage )
//			{
//				if( hr = pd3dDevice->CreatePixelShader( (DWORD*)functionData->ProgramCode->pByteArray, &pShaderItem->pd3dPixelShader ) )
//				{
//					error_printf("Cannot create shader! HRESULT=0x%X", hr);
//					return -1;
//				};
//			}
//			else if( GUSAGE_VERTEXSHADER == functionData->ProgramCode->Usage )
//			{
//				if( hr = pd3dDevice->CreateVertexShader( (DWORD*)functionData->ProgramCode->pByteArray, &pShaderItem->pd3dVertexShader ) )
//				{
//					error_printf("Cannot create shader! HRESULT=0x%X", hr);
//					return -1;
//				};
//			}
//		default:
//			error_printf("Unsupported shader language used? GDEVICE_FUNCTION_DATA::nLanguage == 0x%X", functionData->nLanguage);
//			break;
//		}
	}
	else if( pShaderItem->FileName )
	{

	}
	else
	{
	}
	
	return G_FAIL;
};
