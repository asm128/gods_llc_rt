// File:D3D9ResMgr_buffer.cpp
//
// Copyright © 2011: Pablo Ariel Zorrilla Cepeda
// 
// Any use or distribution of this code is strictly prohibited
//

//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

void _CGD3D9ResourceManager::__freeBufferList( void )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__freeBufferList" );
	GHPTR(VideoBuffer)	garbage=0;
	CGD3D9Buffer* pgarbage;
	while( m_lstBuffer.size() )
	{
		m_lstBuffer.pop(&pgarbage);
		garbage = pgarbage;
	}						
}

__GDEFINE_GET_GINTERFACE_BY_ID_GELEMENTARY( VideoBuffer, Buffer );

int32_t _CGD3D9ResourceManager::CreateBuffer( int32_t nID, GRESOURCE_MODE nAccessMode, GRDS(SBuffer) BufferData, GOSH(VideoBuffer)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateBuffer" );
	if( nID <= 0 )
	{
		error_printf("Tried to create buffer with invalid ID=%i.", nID);
		return G_INVALIDID; // 
	}
	
	CGD3D9Buffer* pBuffer=0;
	if( nID )
	{
		if( pBuffer = (CGD3D9Buffer*)GetBuffer( nID ) )
		{
			error_printf("Tried to create buffer with already registered ID=%i.", nID);
			GHPTR(VideoBuffer) old=pBuffer;
			return G_INVALIDID;
		}
	}
	else
		nID = getAvailableID( GIID_DEVICEBUFFER );

	if( 0 == pBuffer )
	{
		pBuffer = new CGD3D9Buffer( this );
		if( 0 == pBuffer )
		{
			error_printf("Out of memory!");
			return -1;
		}
		if( pBuffer->internalInit( nID, BufferData, nAccessMode ) )
		{
			error_printf("when trying to initialize new CGD3D9Buffer instance! with ID=%i", nID);
			printInfoString(BufferData);
			delete pBuffer;
			return G_FAIL;
		}
		m_lstBuffer.push( pBuffer );
	}
	else
		if( pBuffer->internalInit( nID, BufferData, nAccessMode ) )
		{
			error_printf("Cannot reinitialize CGD3D9Buffer instance! with ID=%i", nID);
			return G_FAIL;
		}
	
	if( ppCreated )
	{
		GHPTR(VideoBuffer) garbage = *ppCreated;
		*ppCreated = gacquireInterface( pBuffer );
	}

	return S_OK;
};

int32_t _CGD3D9ResourceManager::CreateBuffer( int32_t nID, GRESOURCE_MODE nAccessMode, uint32_t nBufferSize, 
													GDATA_TYPE nDataFormat, uint32_t nElementCount, 
													GRESOURCE_USAGE nUsage, const void* pDataBuffer, 
													GOSH(VideoBuffer)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateBuffer" );
	error_t result=0;

	gpod_ptr<SBuffer> BufferData;
	result = gcreateBuffer( nDataFormat, nUsage, nElementCount, &BufferData );
	if( result )
	{
		error_printf("Cannot create buffer! Out of memory? error=0x%X", result);
		return -1;
	}
	uint32_t nSizeToCopy, nElementSize = GTYPEID_TOTALBYTES( nDataFormat );
	nSizeToCopy = min( nElementSize*nElementCount, nBufferSize );
	memcpy( BufferData->pByteArray, pDataBuffer, nSizeToCopy );

	if( result = this->CreateBuffer( nID, GRESMODE_DEVICE_READ, BufferData, ppCreated ) )
	{
		error_printf("Failed!");
		return result;
	}
	return result;
};

int32_t _CGD3D9ResourceManager::CreateBuffer( GRESOURCE_MODE nAccessMode, uint32_t nBufferSize, 
											GDATA_TYPE nDataFormat, uint32_t nElementCount, 
											GRESOURCE_USAGE nUsage, const void* pDataBuffer, 
											GOSH(VideoBuffer)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateBuffer" );
	if( 0 == ppCreated ) 
		return G_INVALIDARGUMENT;

	error_t result=0;
	gpod_ptr<SBuffer> pBufferData=0;
	result = gcreateBuffer( nDataFormat, nUsage, nElementCount, &pBufferData );
	if( result )
	{
		error_printf("Cannot create buffer! Out of memory? error=0x%X", result);
		return -1;
	}
	uint32_t nSizeToCopy, nElementSize = GTYPEID_TOTALBYTES( nDataFormat );
	nSizeToCopy = min( nElementSize*nElementCount, nBufferSize );
	memcpy( pBufferData->pByteArray, pDataBuffer, nSizeToCopy );

	if( result = this->CreateBuffer( GRESMODE_DEVICE_READ, pBufferData, ppCreated ) )
	{
		error_printf("CreateBuffer() FAILED!");
		return result;
	}
	return result;
};

// RREVENGE BUFFERSS!!
int32_t _CGD3D9ResourceManager::CreateBuffer( GRESOURCE_MODE nAccessMode, GRDS(SBuffer) BufferData, GOSH(VideoBuffer)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateBuffer" );
	if( 0 == ppCreated ) 
	{
		error_printf("Output pointer is null!");
		return G_INVALIDARGUMENT;
	}

	int32_t nID = getAvailableID( GIID_DEVICEBUFFER );

	CGD3D9Buffer* pBuffer = 0;
	pBuffer = new CGD3D9Buffer( this );
	if( pBuffer->internalInit( nID, BufferData, nAccessMode ) )
	{
		error_printf("when trying to initialize new CGD3D9Buffer instance! with ID=%i.", nID);
		delete( pBuffer );
		return G_FAIL;
	}
	if( 0 > m_lstBuffer.push( pBuffer ) )
	{
		delete( pBuffer );
		error_printf("Cannot append buffer! Out of memory?");
		return -1;
	};

	GHPTR(VideoBuffer) garbage = *ppCreated;
	*ppCreated = gacquireInterface( pBuffer );
	return S_OK;
};

error_t _CGD3D9ResourceManager::CreateBufferFromFile( const char* szFilename, uint32_t nCodePage, GRESOURCE_MODE nAccessMode, GOSH(VideoBuffer)* pCreated )
{
	return 0;
};

error_t _CGD3D9ResourceManager::CreateBufferFromFile( id_t nID, const char* szFilename, uint32_t nCodePage, GRESOURCE_MODE nAccessMode, GOSH(VideoBuffer)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromFile" );

	if(0 == szFilename)
	{
		error_printf("Invalid arguments! filename is NULL!");
		return G_INVALIDARGUMENT;
	}
	if( nID <= 0 )
	{
		error_printf("Invalid ID! ID=%i.", nID);
		return G_INVALIDID;
	}

	CGD3D9Buffer* pBuffer=0;

	GHPTR(VideoBuffer) garbage;
	if( nID )
	{
		if( garbage = (CGD3D9Buffer*)this->GetImage( nID ) )
		{	// can't create duplicated ID
			error_printf("Duplicated ID=%d! The file asked for was: %s", nID, szFilename );
			return G_INVALIDID;
		}
	}
	else
		nID = getAvailableID( GIID_DEVICEIMAGE );

	pBuffer = new CGD3D9Buffer( this );
	//if( pBuffer->internalInit( nID, szFilename, nAccessMode ) )
	//{
	//	garbage = pBuffer;
	//	error_wprintf(L"FAILED ResourceManager trying to initialize new CGD3D9Image instance from filename=%s. Requested ID=%i.", szFilename, nID );
	//	return G_FAIL;
	//}
	info_printf("CGD3D9Image::init() succeeded! from filename=%s. Requested ID=%i.", szFilename, nID );
	if( 0 > m_lstBuffer.push( pBuffer ) )
	{ // if we fail to append we must be out of memory
		garbage = pBuffer;
		error_printf("Cannot append CGD3D9ResourceManager::__appendBuffer() FAILED! Object won't be created.");
		return G_NOMEM;
	}
	if( ppCreated )
	{
		GHPTR(VideoBuffer) garbage = *ppCreated;
		*ppCreated = gacquireInterface( pBuffer );
	}
	return 0;
};

//
GRDS(SManagerBuffer) _CGD3D9ResourceManager::getBufferItem( GRDS(SManagerBuffer) pBufferItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getBufferItem" );
	if( 0 == pBufferItem || (0 == gget(pBufferItem)->BufferData) )
		return 0;
	GRDS(SManagerBuffer) pNewItem = 0; // store here the return value

	D3DVERTEXBUFFER_DESC* pBufferDesc1, * pBufferDesc2;
	uint32_t i, n = m_lstManagerBuffer.size(); // set some counters
	for( i=0; i<n; i++ )
	{
		pNewItem = m_lstManagerBuffer.get_pointer()[i];

		//if( 0 == pNewItem )
		//	continue;

		pBufferDesc1 = &gget(pBufferItem)->BufferDesc;
		pBufferDesc2 = &gget(pNewItem)->BufferDesc;

		if( pBufferDesc1->Format	== pBufferDesc2->Format
			&& pBufferDesc1->Pool	== pBufferDesc2->Pool
			&& pBufferDesc1->Size	<= pBufferDesc2->Size
			&& pBufferDesc1->Type	== pBufferDesc2->Type
			&& pBufferDesc1->Usage	== pBufferDesc2->Usage
			//&& (0 == (pBufferDesc1->Usage & D3DUSAGE_WRITEONLY))
			&& pNewItem->__nRefCount == 1
			)// I'm wondering if this is gonna work
			break;
		else
			pNewItem = 0;
	}
	if( 0 == pNewItem )
	{
		if( __createDXBuffer( gget(pBufferItem) ) )
		{ // don't acquire if failed to create
			error_printf("__createDXBuffer() FAILED!");
			return 0;
		}
		if( m_lstManagerBuffer.push( pBufferItem ) )
		{
			error_printf("Failed to insert buffer to list! Out of memory?");
			gpod_ptr<SManagerBuffer> garbage( pBufferItem );
			return 0;
		};
		pNewItem = pBufferItem;
	}
	else
	{
		info_printf("Existing item used.");
	}
	
	return gacquire( pNewItem );
};

//
int32_t _CGD3D9ResourceManager::__createDXBuffer( SManagerBuffer* pBufferItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXBuffer" );
	HRESULT hr=NULL;
	GRESOURCE_MODE BufferMode	= pBufferItem->ResourceMode; 
	GRESOURCE_USAGE Usage		= gget(pBufferItem->BufferData)->Usage;
	uint32_t nSize				= gget(pBufferItem->BufferData)->nElementCount*GTYPEID_TOTALBYTES(gget(pBufferItem->BufferData)->DataFormat);//pBufferItem->BufferData->nSizeInBytes;
	uint32_t nElementSize		= GTYPEID_ELEMENTSIZE( gget(pBufferItem->BufferData)->DataFormat );
	D3DVERTEXBUFFER_DESC*	pVertexDesc		= NULL;
	D3DVERTEXBUFFER_DESC*	pIndexDesc		= NULL;
	D3DSURFACE_DESC*		pSurfaceDesc	= NULL;
	switch( Usage )
	{
	case GUSAGE_INDEX:
		pIndexDesc = &pBufferItem->BufferDesc;
		pIndexDesc->Pool	= D3DPOOL_DEFAULT;
		pIndexDesc->Size	= nSize;
		pIndexDesc->Type	= D3DRTYPE_INDEXBUFFER;
		pIndexDesc->Format	= (nElementSize > 16) ? D3DFMT_INDEX32 : D3DFMT_INDEX16;
		pIndexDesc->Usage	= 0;
		if( GRESMODE_HOST_READ != (GRESMODE_HOST_READ & BufferMode) )
		{
			pIndexDesc->Usage	= D3DUSAGE_WRITEONLY;
			pIndexDesc->Pool	= D3DPOOL_DEFAULT;
		}
		else
			pIndexDesc->Pool	= D3DPOOL_MANAGED;

		if( GRESMODE_HOST_WRITE == (GRESMODE_HOST_WRITE & BufferMode) )
		{
			pIndexDesc->Usage	|= D3DUSAGE_DYNAMIC;
		}
		if( GRESMODE_DEVICE_READ != (GRESMODE_DEVICE_READ & BufferMode) 
			&& GRESMODE_DEVICE_WRITE != (GRESMODE_DEVICE_WRITE & BufferMode) )
		{
			pIndexDesc->Pool	= D3DPOOL_SYSTEMMEM;
		}

		if( __createDXIndexBuffer( pBufferItem ) )
		{
			error_printf("__createDXIndexBuffer() FAILED!");
			return G_FAIL;
		}
		else
		{
			info_printf("__createDXIndexBuffer() succeeded!");
		}

		break;
	case GUSAGE_UNKNOWN:
//			break;
	case GUSAGE_TEXCOORD:
	case GUSAGE_BLENDWEIGHT:
	case GUSAGE_BLENDINDICES:
	case GUSAGE_VERTEX:
	case GUSAGE_POSITION:
	case GUSAGE_VERTEXCOLOR:
	case GUSAGE_NORMAL:
	case GUSAGE_TANGENT:
	case GUSAGE_BITANGENT:
		pVertexDesc = &pBufferItem->BufferDesc;
		pVertexDesc->FVF	= 0;
		pVertexDesc->Pool	= D3DPOOL_DEFAULT;
		pVertexDesc->Size	= nSize;
		pVertexDesc->Type	= D3DRTYPE_VERTEXBUFFER;
		pVertexDesc->Format	= D3DFMT_UNKNOWN;
		pVertexDesc->Usage	= 0;
		if( GRESMODE_HOST_READ != (GRESMODE_HOST_READ & BufferMode) )
		{
			pVertexDesc->Usage	= D3DUSAGE_WRITEONLY;
			pVertexDesc->Pool	= D3DPOOL_DEFAULT;
		}
		else 
			pVertexDesc->Pool	= D3DPOOL_MANAGED;

		if( GRESMODE_HOST_WRITE == (GRESMODE_HOST_WRITE & BufferMode) )
		{
			pVertexDesc->Usage	|= D3DUSAGE_DYNAMIC;
		}
		if( GRESMODE_DEVICE_READ != (GRESMODE_DEVICE_READ & BufferMode) 
			&& GRESMODE_DEVICE_WRITE != (GRESMODE_DEVICE_WRITE & BufferMode) )
		{
			pVertexDesc->Pool	= D3DPOOL_SYSTEMMEM;
		}

		// Create buffer finally (if no desc then no buffer)
		if( __createDXVertexBuffer( pBufferItem ) )
		{
			error_printf("__createDXVertexBuffer() FAILED!");
			return G_FAIL;
		}
		else
		{
			info_printf("__createDXVertexBuffer() succeeded!");
		}
		break; // exit switch
	case GUSAGE_TEXEL:
	case GUSAGE_DEPTH:
	case GUSAGE_RENDERTARGET:
		return -1;// these should be surface buffers
	case GUSAGE_STENCIL:
		return -1;
	case GUSAGE_DEPTHSTENCIL:
		return -1;
	default:
		return G_INVALIDARGUMENT;
	}
	return S_OK;
}


int32_t _CGD3D9ResourceManager::__createDXIndexBuffer( SManagerBuffer* pBufferItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXIndexBuffer" );
	HRESULT hr;
	IDirect3DIndexBuffer9* pd3dIndexBuffer	= NULL;
	D3DINDEXBUFFER_DESC IBDesc; 
	IBDesc.Format	= pBufferItem->BufferDesc.Format;
	IBDesc.Type		= pBufferItem->BufferDesc.Type;
	IBDesc.Pool		= pBufferItem->BufferDesc.Pool;
	IBDesc.Size		= pBufferItem->BufferDesc.Size;
	IBDesc.Usage	= pBufferItem->BufferDesc.Usage;

	D3DINDEXBUFFER_DESC* pDesc = &IBDesc;

	IDirect3DDevice9* pDevice = m_pD3D9Device;
	if( 0 == pDevice )
	{
		error_printf("IDirect3DDevice9* is NULL! this shouldn't happen!");
		return -1;
	}

	if( D3D_OK != (hr = pDevice->CreateIndexBuffer( pDesc->Size,
			pDesc->Usage, pDesc->Format, pDesc->Pool, &pd3dIndexBuffer, 0 ) ) )
	{
		error_printf("IDirect3DDevice9::CreateIndexBuffer failed! Size = %u, Usage = %u, Format = %u, Pool = %u"
			, (uint32_t)pDesc->Size
			, (uint32_t)pDesc->Usage
			, (uint32_t)pDesc->Format
			, (uint32_t)pDesc->Pool
		);
		return G_FAIL;
	}

	if (gget(pBufferItem->BufferData)->nElementCount*GTYPEID_TOTALBYTES(gget(pBufferItem->BufferData)->DataFormat))
	{
		uint32_t max;
		BYTE* data = (BYTE*)gget(pBufferItem->BufferData)->pByteArray;
		switch( gget(pBufferItem->BufferData)->DataFormat )
		{
		case GDATATYPE_UINT32:
			uint32_t* ui;
			if( pd3dIndexBuffer->Lock( 0, 0, (void**)&ui, D3DLOCK_NOSYSLOCK ) )
			{
				pd3dIndexBuffer->Release();
				return G_FAIL;
			}
			max = IBDesc.Size/sizeof(uint32_t);
			for( uint32_t i=0; i < max; i++ ) 
				ui[i] = ((uint32_t*)data)[i];
			if( pd3dIndexBuffer->Unlock() )
			{
				pd3dIndexBuffer->Release();
				return G_FAIL;
			}
			break;
		case GDATATYPE_UINT16:
			uint16_t* us;
			if( pd3dIndexBuffer->Lock( 0, 0, (void**)&us, D3DLOCK_NOSYSLOCK ) )
			{
				pd3dIndexBuffer->Release();
				return G_FAIL;
			}
			max = IBDesc.Size/sizeof(uint16_t);
			for( uint32_t i=0; i < max; i++ ) 
				us[i] = ((uint16_t*)data)[i];
			if( pd3dIndexBuffer->Unlock() )
			{
				pd3dIndexBuffer->Release();
				return G_FAIL;
			}
			break;
		default:
			error_printf("Unsuported index buffer format. Supported formats: GDATATYPE_UINT16 and GDATATYPE_UINT32. Format received: 0x%X", gget(pBufferItem->BufferData)->DataFormat );
			pd3dIndexBuffer->Release();
			return G_FAIL;
		}
	}

	pd3dIndexBuffer->GetDesc( pDesc );
	pBufferItem->pd3dBuffer = pd3dIndexBuffer;
	pBufferItem->BufferDesc.Format	= pDesc->Format;
	pBufferItem->BufferDesc.Usage	= pDesc->Usage;
	pBufferItem->BufferDesc.Type	= pDesc->Type;
	pBufferItem->BufferDesc.Size	= pDesc->Size;
	pBufferItem->BufferDesc.Pool	= pDesc->Pool;
	pBufferItem->BufferDesc.FVF		= 0;
	return S_OK;
};


int32_t _CGD3D9ResourceManager::__createDXVertexBuffer( SManagerBuffer* pBufferItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXVertexBuffer" );
	IDirect3DVertexBuffer9* pBuffer	= NULL;
	D3DVERTEXBUFFER_DESC* pDesc		= &pBufferItem->BufferDesc;
	HRESULT hr;

	uint32_t nSize = gget(pBufferItem->BufferData)->nElementCount*GTYPEID_TOTALBYTES(gget(pBufferItem->BufferData)->DataFormat);
		//pBufferItem->BufferData->nSizeInBytes;

	IDirect3DDevice9* pDevice = m_pD3D9Device;

	if( D3D_OK !=
		(hr = pDevice->CreateVertexBuffer( nSize, pDesc->Usage, 0, pDesc->Pool, &pBuffer, 0 )) 
	)
	{
		return G_FAIL;
	}
	GDATA_TYPE nDataFormat = gget(pBufferItem->BufferData)->DataFormat;
	if( nSize )
	{
		uint32_t nVertexBytes = GTYPEID_TOTALBYTES( nDataFormat );
		uint32_t nSubElementBytes = GTYPEID_ELEMENTBYTES( nDataFormat ),
			nElementCount = GTYPEID_ELEMENTCOUNT( nDataFormat );

		uint32_t max = nSize/nSubElementBytes;
		BYTE* data = (BYTE*)gget(pBufferItem->BufferData)->pByteArray;
		BYTE* d3dPtr=0;
		switch( nSubElementBytes )
		{
		case 2:
			if( pBuffer->Lock( 0, 0, (void**)&d3dPtr, D3DLOCK_NOSYSLOCK ) )
			{
				pBuffer->Release();
				return G_FAIL;
			}
			for( uint32_t i=0; i < max; i++ ) 
				((WORD*)d3dPtr)[i] = ((WORD*)data)[i];
			if( pBuffer->Unlock() )
			{
				pBuffer->Release();
				return G_FAIL;
			}
			break;
		case 4:
			if( pBuffer->Lock( 0, 0, (void**)&d3dPtr, D3DLOCK_NOSYSLOCK ) )
			{
				pBuffer->Release();
				return G_FAIL;
			}
			for( uint32_t i=0; i < max; i++ ) 
				((DWORD*)d3dPtr)[i] = ((DWORD*)data)[i];
			if( pBuffer->Unlock() )
			{
				pBuffer->Release();
				return G_FAIL;
			}
			break;
		default:
			if( pBuffer->Lock( 0, 0, (void**)&d3dPtr, 0 ) )
			{
				pBuffer->Release();
				return G_FAIL;
			}
			for( uint32_t i=0; i < nSize; i++ ) 
				d3dPtr[i] = data[i];
			if( pBuffer->Unlock() )
			{
				pBuffer->Release();
				return G_FAIL;
			}
		};
	}

	pBuffer->GetDesc( pDesc );
	pBufferItem->pd3dBuffer = pBuffer;
	return S_OK;
};

