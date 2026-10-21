//#define GPROFILE_ENABLED
//
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include <strsafe.h>


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

//__GDEFINE_GET_DEVICEMANAGER_FUNCTION( Image );
//__GDEFINE_GET_RESOURCEMANAGER_FUNCTION( Image );
//__GDEFINE_GET_VIDEORESOURCEMANAGER_FUNCTION( Image );

_CGD3D9Image::_CGD3D9Image( CGD3D9ResourceManager *pManager )
	//:m_TextureData(0,0,Texture_Free_handler)
{
	GPROFILE_METHOD( "_CGD3D9Image", "_CGD3D9Image" );
	m_pManager			= pManager;
	m_nTextureID		= 0;

	__pfreeInterface		= greleaseInterface;
	__nRefCount			= 1;
}

_CGD3D9Image::~_CGD3D9Image( void )
{
	GPROFILE_METHOD( "_CGD3D9Image", "~_CGD3D9Image" );
	if( m_pManager && m_nTextureID )
		m_pManager->releaseResourceID( GIID_DEVICEIMAGE, m_nTextureID );
}

int32_t _CGD3D9Image::internalInit( int32_t nID, uint32_t nWidth, uint32_t nHeight, GRDS(SBuffer) BufferData, GRESOURCE_MODE nAccessMode )
{
	GPROFILE_METHOD( "_CGD3D9Image", "internalInit" );
	m_nTextureID = nID;
	info_printf( "Initializing CGD3D9Image with ID=%i. Size =[%i x %i]. Requested Access Mode is %i", nID, nWidth, nHeight, nAccessMode );
	printInfoString( BufferData );
	gpod_ptr<SImage> ImageItem(m_TextureData);
	ImageItem.create();
	
	gpod_ptr<SBuffer> bufferData = gacquire( BufferData );
	bufferData.clone(bufferData);
	ImageItem->BufferData	= bufferData.acquire();
	ImageItem->bActive		= false;
	ImageItem->FileName		= 0;
	ImageItem->nLevels		= 1;

	D3DSURFACE_DESC* pSurfaceDesc = &ImageItem->PrimarySurfaceDesc;
	if( BufferData )
		pSurfaceDesc->Format	= GD3DFMTFromGDataType( gget(BufferData)->Usage, gget(BufferData)->DataFormat ); // D3DFMT_A8R8G8B8;
	else
		pSurfaceDesc->Format	= D3DFMT_A8R8G8B8;
	pSurfaceDesc->Height				= nHeight;
	pSurfaceDesc->Width					= nWidth;
	pSurfaceDesc->MultiSampleQuality	= DEFAULT_QUALITY;
	pSurfaceDesc->MultiSampleType		= D3DMULTISAMPLE_NONE;
	pSurfaceDesc->Pool					= D3DPOOL_DEFAULT;
	pSurfaceDesc->Usage					= 0;
	pSurfaceDesc->Type					= D3DRTYPE_SURFACE;
	if( GRESMODE_HOST_READ != (GRESMODE_HOST_READ & nAccessMode) )
	{
		//pSurfaceDesc->Usage	= D3DUSAGE_WRITEONLY;
		pSurfaceDesc->Pool	= D3DPOOL_DEFAULT;
		info_printf("Setting pool as D3DPOOL_DEFAULT.");
	}
	else 
	{
		pSurfaceDesc->Pool	= D3DPOOL_MANAGED;
		info_printf("Setting pool as D3DPOOL_MANAGED.");
	}

	if( GRESMODE_HOST_WRITE == (GRESMODE_HOST_WRITE & nAccessMode) )
	{
		info_printf("Setting usage as D3DUSAGE_DYNAMIC.");
		pSurfaceDesc->Usage	= D3DUSAGE_DYNAMIC;
	}
	if( GRESMODE_DEVICE_READ != (GRESMODE_DEVICE_READ & nAccessMode) 
		&& GRESMODE_DEVICE_WRITE != (GRESMODE_DEVICE_WRITE & nAccessMode) 
		)
	{
		info_printf("Setting pool as D3DPOOL_SYSTEMMEM.");
		pSurfaceDesc->Pool	= D3DPOOL_SYSTEMMEM;
	}
	else if( GRESMODE_DEVICE_WRITE == ( GRESMODE_DEVICE_WRITE & nAccessMode ) )
	{
		pSurfaceDesc->Pool	= D3DPOOL_DEFAULT;
		pSurfaceDesc->Usage	= D3DUSAGE_RENDERTARGET;
	}
//
	m_TextureData = m_pManager->getImageItem( ImageItem.get_pointer() );
	if( !m_TextureData )
	{
		error_printf("CGD3D9VideoContext::getTextureItem() FAILED!");
		return G_FAIL;
	}
	else
	{
		info_printf("getTextureItem() succeeded!");
	}
	return 0;
}

int32_t _CGD3D9Image::internalInit( int32_t nID, const wchar_t* pFilename )
{
	GPROFILE_METHOD( "_CGD3D9Image", "internalInit" );
	debug_wprintf( L"Initializing CGD3D9Image with ID=%i, pFilename(0x%p) = \"%s\".", nID, pFilename, pFilename );
	gpod_ptr<SImage> ImageItem;
	ImageItem.create();
	gcreate( &ImageItem->FileName );
	size_t fnlen =  gget(ImageItem->FileName)->WCharCount = (uint16_t)wcslen( pFilename ); // 
	StringCchCopyW( gget(ImageItem->FileName)->WText, fnlen+1, pFilename );
	_wcslwr_s( gget(ImageItem->FileName)->WText );
	BOOL b=false;
	memset(gget( ImageItem->FileName)->Text, 0, sizeof(gget( ImageItem->FileName)->Text));
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, gget(ImageItem->FileName)->WText, MAX_PATH, gget(ImageItem->FileName)->Text, MAX_PATH, "?", &b );
	gget(ImageItem->FileName)->CharCount = (uint16_t)strlen(gget(ImageItem->FileName)->Text);

	ImageItem->b16Bit									= false;
	ImageItem->nAlphaKey								= 0;
	ImageItem->nLevels									= 1;
	ImageItem->PrimarySurfaceDesc.Pool					= D3DPOOL_DEFAULT;
	ImageItem->PrimarySurfaceDesc.MultiSampleQuality	= DEFAULT_QUALITY;
	ImageItem->PrimarySurfaceDesc.MultiSampleType		= D3DMULTISAMPLE_NONE;
	m_TextureData = m_pManager->getImageItem( ImageItem.get_pointer() );
	if( !m_TextureData )
	{
		error_printf("CGD3D9VideoContext::getTextureItem() FAILED!");
		m_pManager->releaseResourceID( GIID_DEVICEIMAGE, nID );
		return G_FAIL;
	}
	else
	{
		info_printf("getTextureItem() succeeded!");
	}

	m_nTextureID = nID;
	return 0;
}

int32_t _CGD3D9Image::internalInit( int32_t nID, bool b16Bit, uint32_t nAlphaKey, const wchar_t* pFilename )
{
	GPROFILE_METHOD( "_CGD3D9Image", "internalInit" );
	debug_wprintf( L"Initializing CGD3D9Image with ID=%i, pFilename(0x%p) = \"%s\".", nID, pFilename, pFilename );
	gpod_ptr<SImage> ImageItem;
	ImageItem.create();
	gcreate( &ImageItem->FileName );
	size_t fnlen = gget(ImageItem->FileName)->WCharCount = (uint16_t)wcslen( pFilename ); // 
	StringCchCopyW( gget( ImageItem->FileName)->WText, fnlen+1, pFilename );
	_wcslwr_s( gget(ImageItem->FileName)->WText );
	BOOL b=false;
	memset(gget(ImageItem->FileName)->Text, 0, sizeof(gget(ImageItem->FileName)->Text));
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, gget(ImageItem->FileName)->WText, MAX_PATH, gget(ImageItem->FileName)->Text, MAX_PATH, "?", &b );
	gget(ImageItem->FileName)->CharCount = (uint16_t)strlen(gget(ImageItem->FileName)->Text);

	ImageItem->b16Bit		= b16Bit;
	ImageItem->nAlphaKey	= nAlphaKey;
	ImageItem->nLevels		= 1;
	ImageItem->PrimarySurfaceDesc.Pool	= D3DPOOL_DEFAULT;
	ImageItem->PrimarySurfaceDesc.MultiSampleQuality	= DEFAULT_QUALITY;
	ImageItem->PrimarySurfaceDesc.MultiSampleType	= D3DMULTISAMPLE_NONE;
	m_TextureData = m_pManager->getImageItem( ImageItem.get_pointer() );
	if( !m_TextureData )
	{
		error_printf("getImageItem() returned a null texture!",);
		m_pManager->releaseResourceID( GIID_DEVICEIMAGE, nID );
		return G_FAIL;
	}

	m_nTextureID = nID;
	return 0;
};

uint32_t _CGD3D9Image::GetWidth( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Image", "GetWidth" );
	if( !m_TextureData )
		return 0;
	return m_TextureData->PrimarySurfaceDesc.Width;
};

uint32_t _CGD3D9Image::GetHeight( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Image", "GetHeight" );
	if( !m_TextureData )
		return 0;
	return m_TextureData->PrimarySurfaceDesc.Height;
};

GRDS(SBuffer) _CGD3D9Image::GetBufferData( uint32_t nIndex ) const
{
	GPROFILE_METHOD( "_CGD3D9Image", "GetBuffer" );
	if( !m_TextureData )
		return 0;

	if(0 == m_TextureData->BufferData)
	{
		error_printf("m_TextureData->BufferData is null which could mean a failure in the load system.");
		switch(m_TextureData->PrimarySurfaceDesc.Format)
		{
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			break;
		case D3DFMT_R5G6B5:
			break;
		default:
			error_printf("Unsupported software buffer format.");
			break;
		};
	}
	else
		return gacquire(m_TextureData->BufferData);

	return 0;
};

GRESOURCE_USAGE _CGD3D9Image::GetUsage( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Image", "GetUsage" );
	return GUSAGE_TEXEL;
};

int32_t _CGD3D9Image::GetID( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Image", "GetID" );
	return m_nTextureID;
};

void* _CGD3D9Image::GetPrivateData( void )
{
	GPROFILE_METHOD( "_CGD3D9Image", "GetPrivateData" );
	if( m_TextureData.valid() )
	{
		if( m_TextureData->pd3dImage )
			m_TextureData->pd3dImage->AddRef();
		return m_TextureData->pd3dImage;
	}
	return 0;
};

//------------------- IGInterface  
void* _CGD3D9Image::GetInterfacePointer( GINTERFACEID_TYPE InterfaceID )
{
	GPROFILE_METHOD( "_CGD3D9Image", "GetInterfacePointer" );
	switch( InterfaceID )
	{
	case GIID_CUSTOMINTERFACE:
	case GIID_DEVICEIMAGE:
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
