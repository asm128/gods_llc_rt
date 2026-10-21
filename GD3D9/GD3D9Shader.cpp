//#define GPROFILE_ENABLED
//
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include <strsafe.h>


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

_CGD3D9Function::_CGD3D9Function( CGD3D9ResourceManager* pManager )
	//:m_FunctionData(0,0,Shader_Free_handler)
{
	GPROFILE_METHOD( "_CGD3D9Function", "_CGD3D9Function" );
	m_pManager			= pManager;
//	m_pFunctionData		= 0;
	m_ID				= 0;
	__pfreeInterface		= greleaseInterface;
	__nRefCount = 1;
}


_CGD3D9Function::~_CGD3D9Function( void )
{
	GPROFILE_METHOD( "_CGD3D9Function", "~_CGD3D9Function" );
	if( m_pManager && m_ID )
		m_pManager->releaseResourceID( GIID_DEVICEFUNCTION, m_ID );
}

int32_t _CGD3D9Function::internalInit( int32_t nID, const wchar_t* pFilename, GPROGRAMMINGLANGUAGE_ID nLanguage, const char* szEntryPoint, GRESOURCE_USAGE nUsage )
{ // we could move this functionality to the manager function CompileAndCreateFunctionFromFile()
	GPROFILE_METHOD( "_CGD3D9Function", "internalInit" );
	gpod_ptr<SFunction> FunctionItem( m_FunctionData );
	FunctionItem.create();
	gcreate(&FunctionItem->FileName);
	size_t fnlen = gget(FunctionItem->FileName)->WCharCount = (uint16_t)wcslen(pFilename); // 
	StringCchCopyW(gget(FunctionItem->FileName)->WText, fnlen + 1, pFilename);
	_wcslwr_s(gget(FunctionItem->FileName)->WText);
	BOOL b = false;
	memset(gget(FunctionItem->FileName)->Text, 0, sizeof(gget(FunctionItem->FileName)->Text));
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, gget(FunctionItem->FileName)->WText, MAX_PATH, gget(FunctionItem->FileName)->Text, MAX_PATH, "?", &b);
	gget(FunctionItem->FileName)->CharCount = (uint16_t)strlen(gget(FunctionItem->FileName)->Text);


	m_FunctionData = m_pManager->getFunctionItem( FunctionItem.get_pointer() );
	if( !m_FunctionData )
	{
		error_wprintf(L"_CGD3D9Function::init(): CGD3D9ResourceManager::getFunctionItem() returned NULL!\n"
			L"Parameters: \n"
			L"int32_t nID						:%i\n"
			L"wchar_t* pFilename (Address)		:0x%p\n"
			L"wchar_t* pFilename (Data)			:%s\n"
			L"GPROGRAMMINGLANGUAGE_ID nLanguage	:%i"
			, nID
			, pFilename
			, (0 != pFilename) ? pFilename : L""
			, (int)nLanguage
			);
		error_printf(
			"const char* szEntryPoint (Address)	:%p\n"
			"const char* szEntryPoint (Data)	:%s\n"
			"GRESOURCE_USAGE nUsage				:0x%X"
			, szEntryPoint
			, (0 != szEntryPoint) ? szEntryPoint : ""
			, (int)nUsage
			);
		return G_FAIL;
	}
	m_ID = nID;
	return 0;
}

int32_t _CGD3D9Function::internalInit( int32_t nID, GRDS(SBuffer) Code, GPROGRAMMINGLANGUAGE_ID nLanguage, const char* szEntryPoint )
{
	GPROFILE_METHOD( "_CGD3D9Function", "internalInit" );
	gpod_ptr<SFunction> FunctionItem( m_FunctionData );
	FunctionItem.create();
	FunctionItem->SourceCode = gacquire( Code );

	m_FunctionData = m_pManager->getFunctionItem( FunctionItem.get_pointer() );
	if( !m_FunctionData )
	{
		error_printf("_CGD3D9ResourceManager::getFunctionItem( _GMANAGER_FUNCTION_DATA* ) returned NULL!");
		return G_FAIL;
	}
	m_ID	= nID;
	return 0;
}

int32_t _CGD3D9Function::SetConstantBuffer( const char* pVarName, IGBuffer* pBuffer )
{
	GPROFILE_METHOD( "_CGD3D9Function", "SetConstantBuffer" );
	if( 0 == pVarName || 0 == pBuffer )
	{
		if( pVarName )
		{
			error_printf( "Invalid parameters calling CGD3D9Program::SetConstantBuffer(). VarName(0x%p)=%s. IGBuffer*=0x%p", pVarName, pVarName, pBuffer );
		}
		else
		{
			error_printf( "Invalid parameters calling CGD3D9Program::SetConstantBuffer(). VarName=NULL. IGBuffer*=0x%p", pBuffer );
		}
		//printInfoString( pBuffer );
		return G_FAIL;
	}
	error_printf("_CGD3D9Program::SetConstantBuffer() is still not implemented!!");
	return G_FAIL;
};

int32_t _CGD3D9Function::SetConstantValue( const char* pVarName, const void* pValue, uint32_t nSize )
{
	GPROFILE_METHOD( "_CGD3D9Function", "SetConstantValue" );
	if( !m_FunctionData )
	{
		error_printf( "_CGD3D9Program::SetConstantValue(): object not properly initialized!" );
		return G_FAIL;
	}
	if( 0 == pVarName )
	{
		error_printf( "_CGD3D9Program::SetConstantValue(): Name == NULL!" );
		return G_FAIL;
	}
	if( 0 == m_FunctionData->pd3dConstantTable )
	{
		error_printf( "_CGD3D9Program::SetConstantValue(): No constant table! Why would this ever happen?" );
		return G_FAIL;
	}
	D3DXHANDLE ParamHandle = m_FunctionData->pd3dConstantTable->GetConstantByName( 0, pVarName );
	if( 0 == ParamHandle )
	{
		error_printf( "ID3DXConstantTable::GetConstantByName(): returned NULL! Unexisting parameter? (%s).", pVarName );
		return G_FAIL;
	}
	HRESULT hr = m_FunctionData->pd3dConstantTable->SetValue( m_pManager->getd3d9Device(), ParamHandle, pValue, nSize );
	if( hr ) 
	{
		error_printf( "ID3DXConstantTable::SetValue() returned HRESULT=0x%X. Parameter \"%s\" not set.", hr, pVarName );
	}
	return hr;
};

int32_t _CGD3D9Function::SetConstantImage( const char* pVarName, GOSH(VideoImage) pImage )
{
	GPROFILE_METHOD( "_CGD3D9Function", "SetConstantImage" );
	if( !m_FunctionData )
	{
		error_printf( "_CGD3D9Program::SetConstantImage(): object not properly initialized!" );
		return G_FAIL;
	}
	if( 0 == pImage )
	{
		error_printf( "_CGD3D9Program::SetConstantImage(): IGImage* == NULL!" );
		return G_FAIL;
	}
	if( 0 == m_FunctionData->pd3dConstantTable	)
	{
		error_printf( "_CGD3D9Program::SetConstantImage(): No constant table! Why would this ever happen?" );
		return G_FAIL;
	}
	D3DXHANDLE ParamHandle = m_FunctionData->pd3dConstantTable->GetConstantByName( 0, pVarName );
	if( 0 == ParamHandle )
	{
		error_printf( "ID3DXConstantTable::GetConstantByName() returned NULL! Unexisting parameter? (%s).", pVarName );
		return G_FAIL;
	}

	uint32_t nSamplerIndex = m_FunctionData->pd3dConstantTable->GetSamplerIndex( ParamHandle );
	IDirect3DDevice9* pd3dDevice = m_pManager->getd3d9Device();

	HRESULT hr = 0;
	CGD3D9Image* pTex = (CGD3D9Image*)pImage->GetInterfacePointer( GIID_CUSTOMINTERFACE );
	if( 0 == pTex )
	{
		error_printf("_CGD3D9Program::SetConstantImage(): Cannot retrieve CGD3D9Image object!");
		return G_FAIL;
	}

	IDirect3DTexture9* pd3dTex = (IDirect3DTexture9*)pTex->getItemData();
	GHPTR(VideoImage) garbage = pTex;
	if( 0 == pd3dTex )
	{
		error_printf( "_CGD3D9Program::SetConstantImage(): cannot retrieve IDirect3DTexture9 interface from CGD3D9Image instance! Invalid object?" );
		return G_FAIL;
	}
	if( hr = pd3dDevice->SetTexture( nSamplerIndex, pd3dTex ) )
	{	
		error_printf("IDirect3DDevice9::SetTexture() returned HRESULT=0x%X! Sampler index = %u, IDirect3DTexture9* = 0x%p", hr, nSamplerIndex, pd3dTex);  
	};
	pd3dTex->Release();
	return hr;
};

int32_t _CGD3D9Function::GetID( void ) const
{
	//GPROFILE_METHOD( "_CGD3D9Function", "GetID" );
	return m_ID;
};

GRESOURCE_TYPE _CGD3D9Function::GetResourceType( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Function", "GetResourceType" );
	return GRESOURCE_FUNCTION;
};

GRDS(SBuffer) _CGD3D9Function::GetFunctionData( void )
{
	GPROFILE_METHOD( "_CGD3D9Function", "GetFunctionData" );
	return gacquire( m_FunctionData->SourceCode );
};

int32_t _CGD3D9Function::GetVersion( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Function", "GetVersion" );
	//if( m_pFunctionData && m_pFunctionData->pFunctionData )
	//	return m_pFunctionData->pFunctionData->nVersion;
	return 0;
};

//		virtual char*					GetIdentifierString( void ) const;
GRESOURCE_USAGE _CGD3D9Function::GetUsage( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Function", "GetUsage" );
	if( !m_FunctionData || !m_FunctionData->SourceCode )
		return GUSAGE_UNKNOWN;
	return gget(m_FunctionData->SourceCode)->Usage;
};

void* _CGD3D9Function::GetPrivateData( void )
{
	GPROFILE_METHOD( "_CGD3D9Function", "GetPrivateData" );
	if( !m_FunctionData ) 
		return 0;
	if( m_FunctionData->pd3dPixelShader )
	{
		m_FunctionData->pd3dPixelShader->AddRef();
		return m_FunctionData->pd3dPixelShader;
	}
	else if( m_FunctionData->pd3dVertexShader )
	{
		m_FunctionData->pd3dVertexShader->AddRef();
		return m_FunctionData->pd3dVertexShader;
	}
	return 0;
};


//------------------- IGInterface  
void* _CGD3D9Function::GetInterfacePointer( GINTERFACEID_TYPE InterfaceID )

{
	GPROFILE_METHOD( "_CGD3D9Function", "GetInterfacePointer" );
	switch( InterfaceID )
	{
	case GIID_CUSTOMINTERFACE:
	case GIID_DEVICEFUNCTION:
		__nRefCount++;
		return this;
	default:
		error_printf( "FAILED When trying to retrieve interface!\n"
			"InterfaceID		: 0x%X-%X.\n"
			"Instance address	: 0x%p."
			, (int)((InterfaceID & (0xFFFFFFFFLL<<32)) >> 32)
			, (int)(InterfaceID & 0xFFFFFFFF)
			, this
			);
		return 0;
	}
};
