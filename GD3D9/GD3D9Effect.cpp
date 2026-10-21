//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include <strsafe.h>


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;


_CGD3D9Program::_CGD3D9Program( CGD3D9ResourceManager *pManager )
	//:m_EffectData(0,0,XEffect_Free_handler)
{
	GPROFILE_METHOD( "_CGD3D9Program", "_CGD3D9Program" );
	m_pManager		= pManager;
	__pfreeInterface	= greleaseInterface;

	m_ID			= 0;
	__nRefCount		= 1;
}


_CGD3D9Program::~_CGD3D9Program( void )
{
	GPROFILE_METHOD( "_CGD3D9Program", "~_CGD3D9Program" );
	if( m_pManager && m_ID )
		m_pManager->releaseResourceID( GIID_DEVICEPROGRAM, m_ID );
}

int32_t _CGD3D9Program::internalInit( int32_t nID, const wchar_t* pFilename )
{
	GPROFILE_METHOD( "_CGD3D9Program", "internalInit" );
	debug_wprintf( L"Initializing CGD3D9Program with ID=%i, pFilename(0x%p) = \"%s\".", nID, pFilename, pFilename );
	m_ID = nID;
	gpod_ptr<SEffect> EffectItem( m_EffectData );
	EffectItem.create();
	gcreate(&EffectItem->FileName);
	size_t fnlen = gget(EffectItem->FileName)->WCharCount = (uint16_t)wcslen(pFilename); // 
	StringCchCopyW(gget(EffectItem->FileName)->WText, fnlen + 1, pFilename);
	_wcslwr_s(	   gget(EffectItem->FileName)->WText);
	BOOL b = false;
	memset(		   gget(EffectItem->FileName)->Text, 0, sizeof(gget(EffectItem->FileName)->Text));
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, gget(EffectItem->FileName)->WText, sizeof(gget(EffectItem->FileName)->WText)/sizeof(wchar_t), gget(EffectItem->FileName)->Text, sizeof(gget(EffectItem->FileName)->Text), "?", &b);
	DWORD lerr = GetLastError();
	HRESULT hr = HRESULT_FROM_WIN32(lerr);

	gget(EffectItem->FileName)->CharCount = (uint16_t)strlen(gget(EffectItem->FileName)->Text);

	m_EffectData = m_pManager->getEffectItem( EffectItem.get_pointer() );
	if( !m_EffectData )
	{
		error_printf( "getEffectItem() returned NULL!" );
		return G_FAIL;
	}
	debug_wprintf( L"Initialized CGCD3D9Effect with ID=%i, pFilename(0x%p) = \"%s\".", nID, pFilename, pFilename );
	return 0;
}

// IGProgram
int32_t _CGD3D9Program::SetConstantBuffer( const char* pVarName, GOSH(VideoBuffer) pBuffer )
{
	GPROFILE_METHOD( "_CGD3D9Program", "SetConstantBuffer" );
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

int32_t _CGD3D9Program::SetConstantValue( const char* pVarName, const void* pValue, uint32_t nSize )
{
	GPROFILE_METHOD( "_CGD3D9Program", "SetConstantValue" );
	if( !m_EffectData )
	{
		error_printf( "Attempt to set a value without the data initialized!");
		return G_FAIL;
	}
	if( 0 == m_EffectData->pd3dXProgram )
	{
		error_printf( "Attempt to set a value without a ID3DXEffect instance available!");
		return G_FAIL;
	}
	D3DXHANDLE ParamHandle = m_EffectData->pd3dXProgram->GetParameterByName( 0, pVarName );
	if( 0 == ParamHandle )
	{
		error_printf("FAILED Attempt to set a variable to an unexisting parameter: %s!", pVarName);
		return G_FAIL;
	}
	HRESULT hr = m_EffectData->pd3dXProgram->SetValue( ParamHandle, pValue, nSize );
	if( hr ) 
	{
		error_printf( "ID3DXEffect::SetValue() returned HRESULT=0x%X. Parameter name: %s", hr, pVarName );
	}
	return hr;
};

int32_t _CGD3D9Program::SetConstantImage( const char* pVarName, GOSH(VideoImage) pImage )
{
	GPROFILE_METHOD( "_CGD3D9Program", "SetConstantImage" );
	if( 0 == pImage )
	{
		error_printf( "cannot set NULL as a constant image!" );
		return G_FAIL;
	}
	if( !m_EffectData )
	{
		error_printf( "Cannot set a constant image to a shader not properly initialized!" );
		return G_FAIL;
	}
	if( 0 == m_EffectData->pd3dXProgram )
	{
		error_printf( "Cannot set a constant image to a shader without proper ID3DXEffect instance initialized!" );
		return G_FAIL;
	}
	CGD3D9Image* pTex = (CGD3D9Image*)pImage->GetInterfacePointer( GIID_CUSTOMINTERFACE );
	if( 0 == pTex )
	{
		error_printf("FAILED Trying to retrieve a proper CGD3D9Image pointer!");
		return G_FAIL;
	}
	HRESULT hr = 0;
	IDirect3DTexture9* pd3dTex = (IDirect3DTexture9*)pTex->GetPrivateData();
	GHPTR(VideoImage) garbage=pTex;
	if( 0 == pd3dTex )
	{
		error_printf( "Cannot retrieve IDirect3DTexture9 pointer from CGD3D9Image instance!" );
		return G_FAIL;
	}
	D3DXHANDLE ParamHandle = m_EffectData->pd3dXProgram->GetParameterByName( 0, pVarName );
	if( 0 == ParamHandle )
	{
		error_printf( "Cannot set a constant image to an unexisting parameter! (%s).", pVarName );
		pd3dTex->Release();
		return G_FAIL;
	}
	hr = m_EffectData->pd3dXProgram->SetTexture( ParamHandle, pd3dTex );
	if( hr )
	{
		error_printf( "ID3DXEffect::SetTexture() returned HRESULT=0x%X.", (int)hr );
	}
	pd3dTex->Release();
	return hr;
};

int32_t _CGD3D9Program::SetActiveFunctionByName( const char* pFunctionName )
{
	GPROFILE_METHOD( "_CGD3D9Program", "SetActiveFunctionByName" );
	if( 0 == pFunctionName )
	{
		error_printf( "CGD3D9Program FAILED Trying to set NULL as a function!" );
		return G_FAIL;
	}
	if( !m_EffectData )
	{
		error_printf( "CGD3D9Program FAILED Trying to set a function on an uninitialized instance!" );
		return G_FAIL;
	}
	if( 0 == m_EffectData->pd3dXProgram )
	{
		error_printf( "CGD3D9Program FAILED Trying to set a function on an effect without proper ID3DXEffect instance!" );
		return G_FAIL;
	}
	
	ID3DXEffect* pd3dProgram = m_EffectData->pd3dXProgram;

	D3DXHANDLE ParamHandle = pd3dProgram->GetTechniqueByName( pFunctionName );
	if( 0 == ParamHandle )
	{
		error_printf( "CGD3D9Program FAILED Trying to retrieve a function! pFunctionName=%s", pFunctionName );
		return G_FAIL;
	}

	if( m_EffectData->ActiveTechHandle == ParamHandle )
		return S_OK;

	if( m_EffectData->bActive )
	{
		pd3dProgram->EndPass();
		pd3dProgram->End();
		m_EffectData->bActive = false;
	}

	if( D3D_OK == pd3dProgram->SetTechnique( ParamHandle ) )
		m_EffectData->ActiveTechHandle = ParamHandle;
	else
	{
		error_printf( "Failed to set active technique on ID3DXEffect instance at address 0x%p.", pd3dProgram );
		m_EffectData->ActiveTechHandle = 0;
		return G_FAIL;
	}
	return 0;
};

int32_t _CGD3D9Program::GetID( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Program", "GetID" );
	//if( m_pEffectData && m_pEffectData->pProgramData )
	//	return m_pEffectData->pProgramData->nProgramID;
	return m_ID;
};

GRESOURCE_TYPE _CGD3D9Program::GetResourceType( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Program", "GetResourceType" );
	return GRESOURCE_PROGRAM;
};

void* _CGD3D9Program::GetPrivateData( void )
{
	GPROFILE_METHOD( "_CGD3D9Program", "GetPrivateData" );
	if( !m_EffectData )
		return 0;
	if( m_EffectData->pd3dXProgram )
		m_EffectData->pd3dXProgram->AddRef();
	return m_EffectData->pd3dXProgram;
};

//------------------- IGInterface  
void* _CGD3D9Program::GetInterfacePointer( GINTERFACEID_TYPE InterfaceID )

{
	GPROFILE_METHOD( "_CGD3D9Program", "GetInterfacePointer" );
	switch( InterfaceID )
	{
	case GIID_CUSTOMINTERFACE:
	case GIID_DEVICEPROGRAM:
		__nRefCount++;
		return this;
	default:
		error_printf( "Failed to retrieve interface from interface id!\n"
			"InterfaceID		: 0x%X-%X.\n"
			"Instance address	: 0x%p."
			, (int)((InterfaceID & (0xFFFFFFFFLL<<32)) >> 32)	// msb?
			, (int)(InterfaceID & 0xFFFFFFFF)					// lsb?
			, this
			);
		return 0;
	}
};
