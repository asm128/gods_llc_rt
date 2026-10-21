//#define GPROFILE_ENABLED
#include "GD3D9ResourceManager.h"
#include "GD3D9Device.h"


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

// List			= 0;\


_CGD3D9ResourceManager::_CGD3D9ResourceManager( _CGD3D9Device* pDevice )
	//:m_lstManagerBuffer(0, 0, ManagerBuffer_Free_handler),			// initialize list handlers.
	//m_lstManagerImage(0, 0, Texture_Free_handler),					// initialize list handlers.
	//m_lstManagerBlendState(0, 0, BlendState_Free_handler),			// initialize list handlers.
	//m_lstManagerDepthStencilState(0, 0, DepthStencil_Free_handler),	// initialize list handlers.
	//m_lstManagerRasterizerState(0, 0, Rasterizer_Free_handler),		// initialize list handlers.
	//m_lstManagerXProgram(0, 0, XEffect_Free_handler),				// initialize list handlers.
	//m_lstManagerXFont(0, 0, XFont_Free_handler),					// initialize list handlers.
	//m_lstManagerDecl(0, 0, Decl_Free_handler),						// initialize list handlers.
	//m_lstManagerFunction(0, 0, Shader_Free_handler)					// initialize list handlers.
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "_CGD3D9ResourceManager" );
	m_pDevice				= pDevice;
	m_pD3D9Device			= 0;
	m_nUsedIDArraySize		= 0;
	m_pUsedID				= 0;
	m_nID					= 0;
	__pfreeInterface			= greleaseInterface;

	// 
	__nRefCount				= 1; // Init with refcount 1
	//_CrtSetBreakAlloc(245);
};

_CGD3D9ResourceManager::~_CGD3D9ResourceManager( void )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "~_CGD3D9ResourceManager" );
	onLostDevice();
	onDestroyDevice();
	__freeFontList();
	__freeProgramList();
	__freeFunctionList();
	__freeImageList();
	__freeBufferList();

	if( m_pUsedID )
		free( m_pUsedID );
	
	if( m_pD3D9Device )
		m_pD3D9Device->Release();

#undef __GRELEASE_MANAGER_ITEM_ARRAY
};

#define ID_ARRAY_SIZE_BLOCK 100
#define MAKE_ID_FROM_INDEX( Index ) (-(Index+1))

int32_t	_CGD3D9ResourceManager::releaseResourceID( GINTERFACEID_TYPE ResourceType, id_t ID )
{
	if( ID >= 0 )
		return 0;
	id_t index = (-ID)-1;
	m_pUsedID[index] = false;
	return 0;
};

int32_t _CGD3D9ResourceManager::getAvailableID( GINTERFACEID_TYPE ResourceType )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getAvailableID" );
	if( 0 == m_nUsedIDArraySize || 0 == m_pUsedID )
	{// we use an array and use the (index+1)*-1 formula to make a valid ID
		m_pUsedID		= (bool*)malloc( sizeof( bool )*ID_ARRAY_SIZE_BLOCK );
		memset( m_pUsedID, 0, sizeof(bool)*ID_ARRAY_SIZE_BLOCK );
		m_pUsedID[0]	= true;
		m_nUsedIDArraySize	= ID_ARRAY_SIZE_BLOCK;
//		m_nUsedIDCount	= 1;
		return MAKE_ID_FROM_INDEX( 0 );
	}
	int32_t i, n=m_nUsedIDArraySize;
	for( i=0; i<n; i++ )
	{
		if( m_pUsedID[i] )
			continue;
		m_pUsedID[i] = true;
		return MAKE_ID_FROM_INDEX( i );
	}
	// if we got this far, it means there's no space in the array so we need to create more space
	int32_t newsize = m_nUsedIDArraySize+ID_ARRAY_SIZE_BLOCK, oldSize=m_nUsedIDArraySize;
	bool* OldArray = m_pUsedID,* NewArray = (bool*)malloc( sizeof( bool )*newsize );
	memset( NewArray, true, sizeof( bool )*oldSize ); 
	memset( &NewArray[oldSize], false, sizeof( bool )*ID_ARRAY_SIZE_BLOCK ); 
	NewArray[oldSize] = true;
	m_nUsedIDArraySize	= newsize;
	m_pUsedID = NewArray;
	free( OldArray );
	return MAKE_ID_FROM_INDEX(oldSize);
};

int32_t _CGD3D9ResourceManager::onCreateDevice( IDirect3DDevice9* pD3DDevice )
{ // here we only initialize static resources, leave dynamic for the "onResetDevice" event.
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "onCreateDevice" );
	if( m_pD3D9Device )
	{
		error_printf("CGD3D9ResourceManager::onCreateDevice() was called when there was a device already set!");
		return -1;
		//m_pD3D9Device->Release();
	}
	if( 0 == pD3DDevice )
	{
		return -1;
		error_printf("IDirect3DDevice9* is NULL!");
	}
	HRESULT hr=0;
	if( 0 > (hr = pD3DDevice->QueryInterface( IID_IDirect3DDevice9, (void**)&m_pD3D9Device )) )
	{
		error_printf("QueryInterface( IID_IDirect3DDevice9 ) returned 0x%X! Cannot acquire device!", hr);
		return -1;
	};


	uint32_t i, n;
	n=m_lstManagerBuffer.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerBuffer[i] )
		{	
			if( m_lstManagerBuffer[i]->BufferDesc.Pool != D3DPOOL_DEFAULT )
			{
				__createDXBuffer( m_lstManagerBuffer[i] );
			}
		}
	}
	n = m_lstManagerFunction.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerFunction[i] )
		{	
			__createDXFunction( m_lstManagerFunction[i] );
		}
	}

	n = m_lstManagerImage.size();
	for( i=0; i < n; i++ )
	{
		if( m_lstManagerImage[i] )
		{	
			if( m_lstManagerImage[i]->PrimarySurfaceDesc.Pool != D3DPOOL_DEFAULT )
			{
				__createDXImage( m_lstManagerImage[i] );
			}
		}
	}

	n = m_lstManagerXProgram.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerXProgram[i] )
		{	
			__createDXXProgram( m_lstManagerXProgram[i] );
		}
	}
	n = m_lstManagerXFont.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerXFont[i] )
		{	
			__createDXXFont( m_lstManagerXFont[i] );
		}
	}

	m_DXUTDialogManager.OnD3D9CreateDevice( pD3DDevice );


	return S_OK;
};

int32_t _CGD3D9ResourceManager::onResetDevice( void )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "onResetDevice" );
	uint32_t i, n;
	n = m_lstManagerBuffer.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerBuffer[i] )
		{	
			if( m_lstManagerBuffer[i]->BufferDesc.Pool == D3DPOOL_DEFAULT )
			{
				__createDXBuffer( m_lstManagerBuffer[i] );
			}
		}
	}

	n = m_lstManagerImage.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerImage[i] )
		{	
			if( m_lstManagerImage[i]->PrimarySurfaceDesc.Pool == D3DPOOL_DEFAULT )
			{
				__createDXImage( m_lstManagerImage[i] );
			}
		}
	}
	n = m_lstManagerXProgram.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerXProgram[i] && m_lstManagerXProgram[i]->pd3dXProgram )
		{	
			m_lstManagerXProgram[i]->pd3dXProgram->OnResetDevice();
		}
	}
	n=m_lstManagerXFont.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerXFont[i] && m_lstManagerXFont[i]->pd3dXFont )
		{	
			m_lstManagerXFont[i]->pd3dXFont->OnResetDevice();
		}
	}
	m_DXUTDialogManager.OnD3D9ResetDevice();
	return S_OK;
};

void _CGD3D9ResourceManager::onLostDevice( void )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "onLostDevice" );
	uint32_t i, n;
	n=m_lstManagerXFont.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerXFont[i] &&  m_lstManagerXFont[i]->pd3dXFont )
		{	
			m_lstManagerXFont[i]->pd3dXFont->OnLostDevice();
		}
	}
	n = m_lstManagerXProgram.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerXProgram[i] && m_lstManagerXProgram[i]->pd3dXProgram )
		{	
			m_lstManagerXProgram[i]->pd3dXProgram->OnLostDevice();
			m_lstManagerXProgram[i]->ActiveTechHandle	= 0;
			m_lstManagerXProgram[i]->bActive = false;
		}
	}
	n = m_lstManagerImage.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerImage[i] && m_lstManagerImage[i]->pd3dImage )
		{	
			if( m_lstManagerImage[i]->PrimarySurfaceDesc.Pool == D3DPOOL_DEFAULT )
			{
				m_lstManagerImage[i]->pd3dImage->Release();
				//m_lstManagerImage[i]->bActive = false;
				m_lstManagerImage[i]->pd3dImage = 0;
			}
		}
	}
	n = m_lstManagerBuffer.size();
	for( i=0; i< n; i++ )
	{
		if( m_lstManagerBuffer[i] && m_lstManagerBuffer[i]->pd3dBuffer )
		{	
			if( m_lstManagerBuffer[i]->BufferDesc.Pool == D3DPOOL_DEFAULT )
			{
				m_lstManagerBuffer[i]->pd3dBuffer->Release();
				m_lstManagerBuffer[i]->pd3dBuffer	= 0;
			}
		}
	}
	m_DXUTDialogManager.OnD3D9LostDevice();
};

void _CGD3D9ResourceManager::onDestroyDevice( void )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "onDestroyDevice" );
	uint32_t i, n;

#define __RELEASE_RES_D3D_ARRAY_NEW( tok )				\
n = m_lstManager##tok.size();							\
for( i=0; i < n; i++ )									\
{														\
	if( m_lstManager##tok[i]->pd3d##tok## )				\
	{													\
		m_lstManager##tok[i]->pd3d##tok##->Release();	\
		m_lstManager##tok[i]->pd3d##tok## = 0;			\
	}													\
}

	__RELEASE_RES_D3D_ARRAY_NEW( Buffer )	
	__RELEASE_RES_D3D_ARRAY_NEW( Image )	
	__RELEASE_RES_D3D_ARRAY_NEW( XProgram )	
	__RELEASE_RES_D3D_ARRAY_NEW( XFont )	
	__RELEASE_RES_D3D_ARRAY_NEW( Decl )	

	n = m_lstManagerFunction.size();
	for( i=0; i < n; i++ )
	{
		if( m_lstManagerFunction[i]->pd3dPixelShader )
		{
			m_lstManagerFunction[i]->pd3dPixelShader->Release();
			m_lstManagerFunction[i]->pd3dPixelShader = 0;
		}
		if( m_lstManagerFunction[i]->pd3dVertexShader )
		{
			m_lstManagerFunction[i]->pd3dVertexShader->Release();
			m_lstManagerFunction[i]->pd3dVertexShader = 0;
		}
		if( m_lstManagerFunction[i]->pd3dConstantTable )
		{
			m_lstManagerFunction[i]->pd3dConstantTable->Release();
			m_lstManagerFunction[i]->pd3dConstantTable = 0;
		}
	}

	m_DXUTDialogManager.OnD3D9DestroyDevice();
	if( m_pD3D9Device )
	{
		m_pD3D9Device->Release();
		m_pD3D9Device = 0;
	}
};


// ------------------- IGVideoContext
int32_t _CGD3D9ResourceManager::ClearResources( void )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "ClearResources" );
	__freeFontList();
	__freeProgramList();
	__freeImageList();
	__freeFunctionList();
	__freeBufferList();

	return G_OK;
};

error_t _CGD3D9ResourceManager::FreeUnused( void )
{
	uint32_t size, i=0;												
	GHPTR(VideoBuffer) garbage;
	GOSH(VideoBuffer) pBuffer;
	while( i < (size = m_lstBuffer.size()) )	
	{
		if( (pBuffer = m_lstBuffer.get_pointer()[i])->__nRefCount == 1 )
		{
			garbage = pBuffer;
			m_lstBuffer.set_value( i, m_lstBuffer[m_lstBuffer.size()-1] );
			m_lstBuffer.resize(size-1);
			continue;
		}
		i++;
	}

	i=0;
	GHPTR(VideoImage) garbageImage;
	GOSH(VideoImage) pImage;
	while( i < (size = m_lstImage.size()) )	
	{
		if( (pImage = m_lstImage[i])->__nRefCount == 1 )
		{
			garbageImage = pImage;
			m_lstImage.set_value( i, m_lstImage[m_lstImage.size()-1] );
			m_lstImage.resize(size-1);
			continue;
		}
		i++;
	}

	i=0;
	while( i < (size = m_lstManagerBuffer.size()) )	
	{
		if( (m_lstManagerBuffer.get_pointer()[i])->__nRefCount == 1 )
		{
			m_lstManagerBuffer.set( m_lstManagerBuffer.get_pointer()[size-1], i );
			m_lstManagerBuffer.resize(size-1);
			continue;
		}
		i++;
	}

	i=0;
	while( i < (size = m_lstManagerImage.size()) )
	{
		if( (m_lstManagerImage.get_pointer()[i])->__nRefCount == 1 )
		{
			m_lstManagerImage.set( m_lstManagerImage.get_pointer()[m_lstManagerImage.size()-1], i );
			m_lstManagerImage.resize(size-1);
			continue;
		}
		i++;
	}

	return 0;
};

CDXUTDialogResourceManager* _CGD3D9ResourceManager::GetAyuditaDXUT( void )
{
	return &m_DXUTDialogManager;
}

GOSH(VideoDevice) _CGD3D9ResourceManager::GetVideoManager( void )
{
	return gacquireInterface( m_pDevice );
};

void* _CGD3D9ResourceManager::GetInterfacePointer( GINTERFACEID_TYPE InterfaceID )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "GetInterfacePointer" );
	switch( InterfaceID )
	{
	case GIID_CUSTOMINTERFACE:
	case GIID_RESOURCEMANAGER:
		++__nRefCount;
		return this;
	default:
		return 0;
	}
};


// ------------------------------------------------------------------------------------------ 
// Returns the backbuffer height.  
uint32_t _CGD3D9ResourceManager::GetWidth( uint32_t nWindow )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetWidth" );
	IDirect3DDevice9* pDevice;
	if( 0 == (pDevice = m_pD3D9Device) )
	{
		error_printf("No direct3d9 device!");
		return 320;
	}

	IDirect3DSurface9* pTargetSurface;
	D3DSURFACE_DESC SurfaceDesc;
	pDevice->GetRenderTarget( nWindow, &pTargetSurface );
	if( pTargetSurface )
	{
		pTargetSurface->GetDesc( &SurfaceDesc );
		pTargetSurface->Release();
	}
	else
	{
		error_printf("Invalid surface index! %u", nWindow);
		return 320;
	}
	return SurfaceDesc.Width;
};  

// ------------------------------------------------------------------------------------------ 
// Returns the backbuffer width.
uint32_t _CGD3D9ResourceManager::GetHeight( uint32_t nWindow )
{
	GPROFILE_METHOD( "_CGD3D9Device", "GetHeight" );
	IDirect3DDevice9* pDevice;
	if( 0 == (pDevice = m_pD3D9Device) )
	{
		error_printf("No direct3d9 device!");
		return 200;
	}

	IDirect3DSurface9* pTargetSurface;
	D3DSURFACE_DESC SurfaceDesc;
	pDevice->GetRenderTarget( nWindow, &pTargetSurface );
	if( pTargetSurface )
	{
		pTargetSurface->GetDesc( &SurfaceDesc );
		pTargetSurface->Release();
	}
	else
	{
		error_printf("Invalid surface index! %u", nWindow);
		return 200;
	}
	return SurfaceDesc.Height;

};

