//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"

#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

_CGD3D9Buffer::_CGD3D9Buffer( CGD3D9ResourceManager *pManager )
	//:m_BufferData(0,0, ManagerBuffer_Free_handler)
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "_CGD3D9Buffer" );
	m_pManager			= pManager;
	__pfreeInterface		= greleaseInterface;

	__nRefCount			= 0;
	m_nBufferID			= 0;
}

_CGD3D9Buffer::~_CGD3D9Buffer( void )
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "~_CGD3D9Buffer" );
	if( m_pManager && m_nBufferID )
		m_pManager->releaseResourceID( GRESOURCE_BUFFER, m_nBufferID );
}

error_t _CGD3D9Buffer::internalInit( int32_t nID, const char* szFilename, uint32_t nCodePage, GRESOURCE_MODE ResourceMode )
{
	return 0;
};

//
error_t _CGD3D9Buffer::internalInit( int32_t nID, GRDS(SBuffer) _BufferData, GRESOURCE_MODE nAccessMode )
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "internalInit" );
	info_printf( "Attempting to create buffer with ID=%i from BufferHAL at address %p.", (int)nID, _BufferData );
	gpod_ptr<SManagerBuffer> BufferItem( m_BufferData );
	//ghdlr_Decl DeclItem(0,0,Decl_Free_handler);
	gpod_ptr<SDecl> DeclItem;
	BufferItem.create(); // Create new manager item

	BufferItem->BufferData	= gacquire( _BufferData );
	const SBuffer* BufferData = gget(_BufferData);
	BufferItem->bActive		= false;
	// Assign the HAL buffer
	BufferItem->BufferDesc.Format	= D3DFMT_VERTEXDATA;
	BufferItem->BufferDesc.FVF		= 0;
	BufferItem->BufferDesc.Pool		= D3DPOOL_DEFAULT;
	BufferItem->BufferDesc.Size		= BufferData->nElementCount*GTYPEID_TOTALBYTES(BufferData->DataFormat);
	//BufferItem->BufferDesc.Size = BufferData->nSizeInBytes;
	BufferItem->BufferDesc.Type		= D3DRTYPE_VERTEXBUFFER;
	BufferItem->BufferDesc.Usage	= 0;
	BufferItem->ResourceMode		= nAccessMode;
	// 
	m_BufferData = m_pManager->getBufferItem( BufferItem.get_pointer() ); 
	if( !m_BufferData )
	{
		error_printf( "getBufferItem( pBufferItem=%p ) returned NULL when trying to create buffer with ID=%i. FAILED!", (void*)BufferItem.get_pointer(), (int)nID );
		return G_FAIL;
	}
	else
	{
		info_printf( "getBufferItem() succeeded creating buffer with ID=%i.", nID );
	}
	if( m_BufferData->BufferData != BufferItem->BufferData )
	{
		grelease( &BufferItem->BufferData );
	}
	if( BufferData->Usage == GUSAGE_INDEX )
	{
		info_printf( "GUSAGE_INDEX. Index buffer with ID=%i created OK.", nID );
		__nRefCount			= 1;
		m_nBufferID			= nID;
		return S_OK;
	}
	if( 0 == m_BufferData->pDeclData )
	{

		DeclItem.create();

		DeclItem->bActive	= false;
		DeclItem->nElementCount = 1;
		D3DVERTEXELEMENT9* Decl = &DeclItem->VertexElement[0];
		memset( Decl, 0, sizeof(D3DVERTEXELEMENT9)*MAX_FVF_DECL_SIZE  );
		Decl->Method		= D3DDECLMETHOD_DEFAULT;
		Decl->Offset		= 0;
		Decl->Stream		= 0;
		Decl->UsageIndex	= 0;
		// ----------------------------------------------------------
		// following code applies for vertex buffers only
		switch( BufferData->Usage )
		{
		case GUSAGE_POSITION:
			info_printf( "case GUSAGE_POSITION." );
			Decl->Usage	= D3DDECLUSAGE_POSITION;
			break;
		case GUSAGE_VERTEX:
			info_printf( "case GUSAGE_VERTEX." );
			Decl->Usage	= D3DDECLUSAGE_POSITION;
			break;
		case GUSAGE_VERTEXCOLOR:
			info_printf( "case GUSAGE_VERTEXCOLOR." );
			Decl->Usage	= D3DDECLUSAGE_COLOR;
			break;
		case GUSAGE_NORMAL:
			info_printf( "case GUSAGE_NORMAL." );
			Decl->Usage	= D3DDECLUSAGE_NORMAL;
			break;
		case GUSAGE_TEXCOORD:
			info_printf( "case GUSAGE_TEXCOORD." );
			Decl->Usage	= D3DDECLUSAGE_TEXCOORD;
			break;
		case GUSAGE_BLENDINDICES:
			info_printf( "case GUSAGE_BLENDINDICES." );
			Decl->Usage	= D3DDECLUSAGE_BLENDINDICES;
			break;
		case GUSAGE_BLENDWEIGHT:
			info_printf( "case GUSAGE_BLENDWEIGHT." );
			Decl->Usage	= D3DDECLUSAGE_BLENDWEIGHT;
			break;
		case GUSAGE_TANGENT:
			info_printf( "case GUSAGE_TANGENT." );
			Decl->Usage	= D3DDECLUSAGE_TANGENT;
			break;

		default:
			error_printf( "Unsupported buffer usage (%i) found when trying to create vertex buffer.", (int)BufferData->Usage );
			return G_FAIL;
		}
		switch( BufferData->DataFormat )
		{
		case GDATATYPE_FLOAT32:
			info_printf( "case GDATATYPE_FLOAT32." );
			Decl->Type	= D3DDECLTYPE_FLOAT1;
			break;
		case GDATATYPE_FLOAT32_2:
			info_printf( "case GDATATYPE_FLOAT32_2." );
			Decl->Type	= D3DDECLTYPE_FLOAT2;
			break;
		case GDATATYPE_FLOAT32_3:
			info_printf( "case GDATATYPE_FLOAT32_3." );
			Decl->Type	= D3DDECLTYPE_FLOAT3;
			break;
		case GDATATYPE_FLOAT32_4:
			info_printf( "case GDATATYPE_FLOAT32_4." );
			Decl->Type	= D3DDECLTYPE_FLOAT4;
			break;
		case GDATATYPE_UINT8_4:
			info_printf( "case GDATATYPE_UINT8_4." );
			if( D3DDECLUSAGE_COLOR == Decl->Usage )
			{
				info_printf( "Setting as D3DDECLTYPE_D3DCOLOR." );
				Decl->Type	= D3DDECLTYPE_D3DCOLOR;
			}
			else
			{
				info_printf( "Setting as D3DDECLTYPE_UBYTE4." );
				Decl->Type	= D3DDECLTYPE_UBYTE4;
			}
			break;
		case GDATATYPE_UINT32:
			info_printf( "case GDATATYPE_UINT32." );
			info_printf( "Setting as D3DDECLTYPE_D3DCOLOR." );
			Decl->Type	= D3DDECLTYPE_D3DCOLOR;
			break;
		default:
			error_printf( "Unsupported data format (%i) found when trying to create vertex buffer.", (int)BufferData->DataFormat );
			return G_FAIL;
		}
		Decl = &DeclItem->VertexElement[1];
		Decl->Stream		= 0xFF;
		Decl->Method		= 0;
		Decl->Offset		= 0;
		Decl->Usage			= 0;
		Decl->UsageIndex	= 0;
		Decl->Type			= D3DDECLTYPE_UNUSED;

		m_BufferData->pDeclData = m_pManager->getDeclItem( DeclItem.get_pointer() );
		if( 0 == m_BufferData->pDeclData )
		{
			error_printf("Failed to create vertex declaration for managed buffer at address %p.\nBuffer ID = %i", (void*)m_BufferData.get_pointer() , nID);
			printInfoString( m_BufferData->BufferData );
			return G_FAIL;
		}
	}
	__nRefCount			= 1;
	m_nBufferID			= nID;
	info_printf( "Video buffer with ID=%i created successfully.", nID );
	return S_OK;
}

//------------------- IGBuffer
GRDS(SBuffer) _CGD3D9Buffer::GetBufferData( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetBufferData" );
	if( !m_BufferData || 0 == m_BufferData->BufferData )
		return 0;
	return gacquire( m_BufferData->BufferData );
};

//
uint32_t _CGD3D9Buffer::GetElementCount( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetElementCount" );
	if( !m_BufferData )
		return 0;
	return gget(m_BufferData->BufferData)->nElementCount;
};

//
GDATA_TYPE _CGD3D9Buffer::GetDataFormat( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetDataFormat" );
	if( !m_BufferData )
		return 0;
	return gget(m_BufferData->BufferData)->DataFormat;
};

//uint32_t _CGD3D9Buffer::GetRowCount( void ) const
//{
//	if( m_pBufferData )
//		return m_pBufferData->pBufferHALData->pBufferData->;
//	return 0;
//};

uint32_t _CGD3D9Buffer::GetRowSize( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetRowSize" );
	if( !m_BufferData )
		return 0;
	return gget(m_BufferData->BufferData)->nColumnCount;
};

//
uint32_t _CGD3D9Buffer::GetBufferSize( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetBufferSize" );
	if( !m_BufferData )
		return 0;
	return gget(m_BufferData->BufferData)->nElementCount*GTYPEID_TOTALBYTES(gget(m_BufferData->BufferData)->DataFormat);
		//m_BufferData->BufferData->nSizeInBytes;
};

//
GRESOURCE_MODE _CGD3D9Buffer::GetResourceMode( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetResourceMode" );
	if( !m_BufferData )
		return GRESMODE_DEFAULT;
	return m_BufferData->ResourceMode;
};

//
GRESOURCE_USAGE _CGD3D9Buffer::GetUsage( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetUsage" );
	if( !m_BufferData )
		return GUSAGE_UNKNOWN;
	return gget(m_BufferData->BufferData)->Usage;
};

int32_t _CGD3D9Buffer::Flush( void )
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "Flush" );
	if( !m_BufferData )
		return 0;
	if( m_BufferData->pd3dBuffer )
	{
		m_BufferData->bActive	= false;
		m_BufferData->pd3dBuffer->Release();
		if( m_pManager->__createDXBuffer( m_BufferData.get_address() ) )
		{
			error_printf( "Failed __createDXBuffer() call when trying to flush buffer changes.");
			return -1;
		}
	}
	return 0;
}

//
int32_t _CGD3D9Buffer::GetID( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetID" );
	return m_nBufferID;
};

//------------------- IGInterface  
void* _CGD3D9Buffer::GetInterfacePointer( GINTERFACEID_TYPE InterfaceID )
{
	GPROFILE_METHOD( "_CGD3D9Buffer", "GetInterfacePointer" );
	switch( InterfaceID )
	{
	case GIID_CUSTOMINTERFACE:
	case GIID_DEVICEBUFFER:
		__nRefCount++;
		return this;
	default:
		return 0;
	}
};
