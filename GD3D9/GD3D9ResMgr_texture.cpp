//#define GPROFILE_ENABLED
//
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"

#include <string>
#include <strsafe.h>


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

__GDEFINE_FREE_MANAGER_GINTERFACE_LIST_GELEMENTARY( VideoImage, Image );
__GDEFINE_GET_GINTERFACE_BY_ID_GELEMENTARY( VideoImage, Image );

#define equal_color( r, r1, g, g1, b, b1, epsilon ) \
	( ((abs((int)(r - r1)) < epsilon ) \
	&& (abs((int)(g - g1)) < epsilon ) \
	&& (abs((int)(b - b1)) < epsilon ) ) \
	? true : false )

int32_t _CGD3D9ResourceManager::CreateImageFromMemory( int32_t nID, uint32_t nWidth, uint32_t nHeight,
													  GRDS(SBuffer) pBufferData, GRESOURCE_MODE nAccessMode, 
													  GOSH(VideoImage)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromMemory" );

	if( 0 == nWidth || 0 == nHeight )
	{
		error_printf( "Cannot create a zero-length image! nWidth=%u, nHeight=%u", nWidth, nHeight );
		return G_INVALIDARGUMENT;
	}
	else if( 0 == pBufferData )
	{
		error_printf("_CGD3D9ResourceManager::CreateImageFromMemory(): Invalid parameters! GRDS(SBuffer) is NULL!");
		return G_INVALIDARGUMENT;
	}
	else if( nID <= 0 )
	{
		error_printf("Cannot create GOSH(VideoImage) with invalid ID (<=0).\n"
			"The ID requested was %i for GRDS(SBuffer) at address 0x%p", nID, pBufferData );
		return G_INVALIDID;
	}

	CGD3D9Image* pTexture=0;
	GHPTR(VideoImage) garbage;
	if( nID )
	{
		if( 0 != (garbage = (CGD3D9Image*)this->GetImage( nID )) )
		{	// can't create duplicated ID
			error_wprintf(L"CreateImageFromMemory() FAILED trying to create Video Program with duplicated ID! The ID requested was %i.\n" 
				L"for _GBUFFER_DATA at address: 0x%p", nID, pBufferData );
			return G_INVALIDID;
		}
	}
	else
		nID = getAvailableID( GIID_DEVICEIMAGE );

	pTexture = new CGD3D9Image( this );
	if( pTexture->internalInit( nID, nWidth, nHeight, pBufferData, nAccessMode ) )
	{
		garbage = pTexture;
		error_printf("FAILED to initialize new CGD3D9Image instance! Size=[%i x %i]. AccessMode requested=%i", 
			nWidth, nHeight, nAccessMode );
		return G_FAIL;
	}
	info_printf("CGD3D9Image instance initialized successfully!");
	if( 0 > m_lstImage.push( pTexture ) )
	{
		garbage = pTexture;
		error_printf("CGD3D9ResourceManager::__appendImage() FAILED! Out of memory? Image object won't be created.");
		return G_NOMEM;
	}
	if( ppCreated )
	{
		garbage = *ppCreated;
		*ppCreated = gacquireInterface( pTexture );
	}
	return S_OK;
};

int32_t _CGD3D9ResourceManager::CreateImageFromMemory( uint32_t nWidth, uint32_t nHeight,
													  GRDS(SBuffer) pBufferData, GRESOURCE_MODE nAccessMode, 
													  GOSH(VideoImage)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromMemory" );
	if( !ppCreated )
	{
		error_printf("FAILED invalid argument on CreateImageFromMemory() IGVideoImage** is NULL!");
		return G_INVALIDARGUMENT;
	}
	else if( 0 == pBufferData )
	{
		error_printf("_CGD3D9ResourceManager::CreateImageFromMemory(): Invalid parameters! Buffer is NULL!");
		return G_INVALIDARGUMENT;
	}

	CGD3D9Image* pTexture = new CGD3D9Image( this );
	GHPTR(VideoImage) garbage;
	if( pTexture->internalInit( getAvailableID( GIID_DEVICEIMAGE ), nWidth, nHeight, pBufferData, nAccessMode ) )
	{
		garbage = pTexture;
		error_printf("Cannot initialize new CGD3D9Image instance! Size=[%i x %i]. AccessMode requested=%i", nWidth, nHeight, nAccessMode );
		return G_FAIL;
	}
	info_printf("CGD3D9Image instance initialized successfully!");
	if( 0 > m_lstImage.push( pTexture ) )
	{
		garbage = pTexture;
		error_printf("CGD3D9ResourceManager::__appendImage() FAILED! Out of memory? Image object won't be created.");
		return G_NOMEM;
	}
	garbage = *ppCreated;
	*ppCreated = gacquireInterface( pTexture );
	return S_OK;
};

// Monster images
int32_t _CGD3D9ResourceManager::CreateImageFromFile( int32_t nID, const wchar_t* pFilename, GOSH(VideoImage)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromFile" );

	if(0 == pFilename)
	{
		error_printf("Invalid arguments! filename is NULL!");
		return G_INVALIDARGUMENT;
	}
	if( nID <= 0 )
	{
		error_printf("Invalid ID! ID=%i.", nID);
		return G_INVALIDID;
	}

	CGD3D9Image* pTexture=0;

	GHPTR(VideoImage) garbage;
	if( nID )
	{
		if( garbage = (CGD3D9Image*)this->GetImage( nID ) )
		{	// can't create duplicated ID
			error_wprintf(L"Duplicated ID=%d! The file asked for was: %s", nID, pFilename );
			return G_INVALIDID;
		}
	}
	else
		nID = getAvailableID( GIID_DEVICEIMAGE );

	pTexture = new CGD3D9Image( this );
	if( pTexture->internalInit( nID, pFilename ) )
	{
		garbage = pTexture;
		error_wprintf(L"FAILED ResourceManager trying to initialize new CGD3D9Image instance from filename=%s. Requested ID=%i.", pFilename, nID );
		return G_FAIL;
	}
	debug_wprintf(L"CGD3D9Image::init() succeeded! from filename=%s. Requested ID=%i.", pFilename, nID );
	if( 0 > m_lstImage.push( pTexture ) )
	{ // if we fail to append we must be out of memory
		garbage = pTexture;
		error_printf("Cannot append CGD3D9ResourceManager::__appendImage() FAILED! Object won't be created.");
		return G_NOMEM;
	}
	if( ppCreated )
	{
		GHPTR(VideoImage) garbage = *ppCreated;
		*ppCreated = gacquireInterface( pTexture );
	}
	return S_OK;
};

// Monster IMAGES!
int32_t _CGD3D9ResourceManager::CreateImageFromFile( const wchar_t* pFilename, GOSH(VideoImage)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromFile" );

	if( 0 == ppCreated ) 
	{
		error_wprintf(L"CreateImageFromFile() received a NULL pointer as output! Filename=%s.", pFilename );
		return G_INVALIDARGUMENT;
	}
	if( 0 == pFilename )
	{
		error_printf("CreateImageFromFile() FAILED trying to create Video Program from NULL filename.");
		return G_INVALIDARGUMENT;
	}

	uint32_t nID = getAvailableID( GIID_DEVICEIMAGE );
	debug_wprintf(L"Attempting to create image from filename=%s. ID=%i.", pFilename, nID );
	CGD3D9Image* pTexture = new CGD3D9Image( this );
	if( 0 == pTexture )
	{
		error_wprintf(L"CGD3D9ResourceManager cannot allocate new CGD3D9Image instance! Out of memory?" );
		return G_FAIL;
	}
	GHPTR(VideoImage) garbage;
	if( pTexture->internalInit( nID, pFilename ) )
	{
		garbage = pTexture;
		error_wprintf(L"CGD3D9ResourceManager cannot initialize new CGD3D9Image instance from filename=%s. Requested ID=%i.", pFilename, nID );
		return G_FAIL;
	}
	if( 0 > m_lstImage.push( pTexture ) )
	{
		garbage = pTexture;
		error_wprintf(L"CGD3D9ResourceManager failed to append a CGD3D9Image instance to manager list!" );
		return G_NOMEM;
	};

	debug_wprintf(L"CGD3DTexture instance initialized successfully! ID=%i, Filename=%s", pTexture->GetID(), pFilename );
	garbage = *ppCreated;
	*ppCreated = gacquireInterface( pTexture );
	return S_OK;
};

int32_t _CGD3D9ResourceManager::CreateImageFromFile( bool in_b16Bit, uint32_t in_nAlphaKey, 
			const wchar_t* wszFilename, bool* out_bIsAlpha, GOSH(VideoImage)* out_ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromFile" );

	if( 0 == wszFilename )
	{
		error_wprintf(L"FAILED attempt to load image from file! Filename is NULL!");
		return G_FAIL;
	}
	if( !out_ppCreated ) 
	{
		error_wprintf( L"FAILED attempt to load image from file! Output pointer is NULL! for filename=%s", wszFilename );
		return G_INVALIDARGUMENT;
	}

	uint32_t nID=getAvailableID( GIID_DEVICEIMAGE );
	debug_wprintf(L"ResourceManager attempting to create image from data:\nFilename=%s.\nID=%i.\n16Bit=%i.\nAlphaKey=0x%X.", 
		wszFilename, (int)nID, (int)in_b16Bit, (int)in_nAlphaKey );

	CGD3D9Image* pTexture = new CGD3D9Image( this );
	GHPTR(VideoImage) garbage;
	if( pTexture->internalInit( nID, in_b16Bit, in_nAlphaKey, wszFilename ) )
	{
		garbage = pTexture;
		error_wprintf(L"FAILED ResourceManager trying to initialize a new CGD3D9Image instance from filename=%s. Requested ID=%i.", wszFilename, nID );
		return G_FAIL;
	}
	if( out_bIsAlpha )
	{
		gpod_ptr<SImage> pTData = pTexture->getItemData();
		if( pTData->PrimarySurfaceDesc.Format == D3DFMT_R5G6B5 )
			*out_bIsAlpha = false;
		else
			*out_bIsAlpha = true;
	}
	if( 0 > m_lstImage.push( pTexture ) )
	{ // if we fail to append we must be out of memory
		garbage = pTexture;
		error_printf("Cannot append CGD3D9Image object to management list. Object won't be created.");
		return G_NOMEM;
	}

	debug_wprintf(L"ResourceManager successfully created image from data:\nFilename=%s.\nID=%i.\n16Bit=%i.\nAlphaKey=0x%X.", 
		wszFilename, (int)nID, (int)in_b16Bit, (int)in_nAlphaKey );

	garbage = *out_ppCreated;
	*out_ppCreated = gacquireInterface( pTexture );
	return S_OK;
}

// Monster IMAGES!!
int32_t _CGD3D9ResourceManager::CreateImageFromFile( int32_t ID, const char* pFilename, 
												  uint32_t nCodePage, GOSH(VideoImage)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromFile" );
	wchar_t wcFilename[1024] = {0};
	MultiByteToWideChar( nCodePage, MB_PRECOMPOSED, pFilename, (int)strlen( pFilename ), wcFilename, 1024 ); 
	return CreateImageFromFile( ID, wcFilename, ppCreated );
};

int32_t _CGD3D9ResourceManager::CreateImageFromFile( const char* pFilename, 
												  uint32_t nCodePage, GOSH(VideoImage)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromFile" );
	wchar_t wcFilename[1024] = {0};
	MultiByteToWideChar( nCodePage, MB_PRECOMPOSED, pFilename, (int)strlen( pFilename ), wcFilename, 1024 ); 
	return CreateImageFromFile( wcFilename, ppCreated );
};

int32_t _CGD3D9ResourceManager::CreateImageFromFile( bool b16Bit, uint32_t nAlphaKey, 
												  const char* pFilename, 
												  uint32_t nCodePage, bool* out_bIsAlpha, GOSH(VideoImage)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateImageFromFile" );
	wchar_t wcFilename[1024];
	ZeroMemory( wcFilename, sizeof( wchar_t )*1024 );

	MultiByteToWideChar( nCodePage, MB_PRECOMPOSED, pFilename, (int)strlen( pFilename ), wcFilename, 1024 ); 
	return CreateImageFromFile( b16Bit, nAlphaKey, wcFilename, out_bIsAlpha, ppCreated );
};

GRDS(SImage) _CGD3D9ResourceManager::getImageItem( GRDS(SImage) pImageItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getImageItem" );
	if( 0 == pImageItem )
		return 0;
	GRDS(SImage) pNewItem = 0; // store here the return value

	uint32_t i, n = m_lstManagerImage.size();
	if( 0 != gget(pImageItem)->BufferData )
	{
		for( i=0; i<n; i++ )
		{
			pNewItem = m_lstManagerImage.get_pointer()[i];
			if( pNewItem == pImageItem )
				break;
			if( gget(pNewItem)->BufferData == gget(pImageItem)->BufferData )
			{
				if(    gget(pNewItem)->PrimarySurfaceDesc.Pool					== gget(pImageItem)->PrimarySurfaceDesc.Pool 
					&& gget(pNewItem)->PrimarySurfaceDesc.Width					== gget(pImageItem)->PrimarySurfaceDesc.Width 
					&& gget(pNewItem)->PrimarySurfaceDesc.Height				== gget(pImageItem)->PrimarySurfaceDesc.Height
					&& gget(pNewItem)->PrimarySurfaceDesc.Usage					== gget(pImageItem)->PrimarySurfaceDesc.Usage
					&& gget(pNewItem)->PrimarySurfaceDesc.Format				== gget(pImageItem)->PrimarySurfaceDesc.Format
					&& gget(pNewItem)->PrimarySurfaceDesc.MultiSampleQuality	== gget(pImageItem)->PrimarySurfaceDesc.MultiSampleQuality
					&& gget(pNewItem)->PrimarySurfaceDesc.MultiSampleType		== gget(pImageItem)->PrimarySurfaceDesc.MultiSampleType )
				{
					break;
				}
				else 
					pNewItem = 0;
			}
			else
				pNewItem = 0;
		}
	}

	if( 0 == pNewItem && gget(pImageItem)->FileName )
	{
		wchar_t buffer[1024], buffer2[1024]; 
		StringCchCopyW( buffer, 1024, gget(gget(pImageItem)->FileName)->WText );

		for( i=0; i<n; i++ )
		{
			pNewItem = m_lstManagerImage.get_pointer()[i];
			if( 0 == pNewItem )
			{
				error_printf("CGD3D9ResourceManager::m_lstpImageItem got screwed.");
				continue;
			}
#ifdef _WIN32
			if( gget(pNewItem)->FileName )
				StringCchCopyW( buffer2, 1024, gget(gget(pNewItem)->FileName)->WText );
			else
			{
				buffer2[0] = L'?'; buffer2[1] = 0;
			}

			_wcslwr_s( buffer2 );
			if( 0 == wcscmp( buffer2, buffer ) )
#else
			if( 0 == wcscmp( pNewItem->pFileData->wszName, pImageItem->pFileData->wszName ) )
#endif
				break;
			else
				pNewItem = 0;
		}
	}
	if( 0 == pNewItem )
	{
		if( __createDXImage( gget(pImageItem) ) )
		{ // don't acquire if failed to create
			error_printf("__createDXImage( pImageItem ) FAILED!");
			return 0;
		}
		if( m_lstManagerImage.push( pImageItem ) )
		{
			error_printf("__appendImageItem() FAILED!");
			grelease( &pImageItem );
		}
		pNewItem = pImageItem;
	}
	else
	{
	}
	return gacquire( pNewItem );
	/// ADD HERE THE pImageItem pointer to the m_ppImageItem list
};

//
int32_t _CGD3D9ResourceManager::__createDXImage( SImage* pTextureData )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXImage" );
	pTextureData->PrimarySurfaceDesc.Pool	= D3DPOOL_DEFAULT;

	if( pTextureData->BufferData )
		return __createDXImageFromBuffer( pTextureData );
	else
		return __createDXImageFromFile( pTextureData );
}

error_t _CGD3D9ResourceManager::__createDXImageFromFile( SImage* pTextureData )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXImageFromFile" );
	HRESULT hr=0;
	IDirect3DSurface9*	pSurface = 0;
	IDirect3DDevice9*	pDevice = m_pD3D9Device;
	D3DSURFACE_DESC*	pDesc = &pTextureData->PrimarySurfaceDesc;
	IDirect3DTexture9*	pSysTex = 0, * pSysTex2 = 0;
	pTextureData->PrimarySurfaceDesc.Pool	= D3DPOOL_DEFAULT;
	//wchar_t bakName[1024];
	//if( pTextureData->FileData->wszName[0] == L'\\' )
	//	StringCchPrintf( bakName, 1024, L"D:%s", pTextureData->FileData->wszName );
	//else
	//	StringCchPrintf( bakName, 1024, L"D:\\%s", pTextureData->FileData->wszName );
	//if( 0 == CopyFileW( pTextureData->FileData->wszName, bakName, true ) )
	//{
	//	gcreateDirectoryRecursiveW( bakName, (int32_t)wcslen( pTextureData->FileData->wszName ) );
	//	//error_printf("created directory for failed bak");
	//};
		
	static int nBytesCounter = 0;
	if( false == pTextureData->b16Bit )
	{
		info_printf("Loading texture as 32 bit.");

		return this->__createDXImageFromFile( pTextureData, false );
		//if( D3D_OK != D3DXCreateTextureFromFileExW(pDevice,
		//		pTextureData->FileName->WText, 
		//		D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,  //aa D3DX_DEFAULT, D3DX_DEFAULT, 
		//		pTextureData->nLevels,		// mip levels
		//		NULL,				// usage
		//		D3DFMT_A8R8G8B8,    // texture color format
		//		D3DPOOL_DEFAULT,	// memory class
		//		D3DX_DEFAULT,		// filter
		//		D3DX_DEFAULT,		// mip filter
		//		0xFFFF00FF,			// color key 
		//		NULL,				// source info
		//		NULL,				// pallette
		//		&pTextureData->pd3dImage ) )
		//{
		//	//MessageBoxW( NULL, pTextureData->FileName->WText, L"Failed to load texture file", MB_ICONERROR ); 
		//	return G_FAIL;
		//}
		//
		//pTextureData->pd3dImage->GetSurfaceLevel( 0, &pSurface );
		//if( pSurface )
		//{
		//	pSurface->GetDesc( pDesc );
		//	pSurface->Release();
		//}
		//grelease( &pTextureData->BufferData );
		//return 0;
	}

	if( 0 == pTextureData->nAlphaKey )
	{
		info_printf("Loading texture as 16 bit.");
		return this->__createDXImageFromFile( pTextureData, true );
		//pTextureData->PrimarySurfaceDesc.Format = D3DFMT_R5G6B5;
		//pTextureData->PrimarySurfaceDesc.Pool	= D3DPOOL_DEFAULT;
		//if( D3D_OK != D3DXCreateTextureFromFileExW(pDevice,
		//		pTextureData->FileName->WText, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, //D3DX_DEFAULT, D3DX_DEFAULT, 
		//		pTextureData->nLevels,		// mip levels
		//		NULL,				// usage
		//		pTextureData->PrimarySurfaceDesc.Format,		// texture color format
		//		pTextureData->PrimarySurfaceDesc.Pool,	// memory class
		//		D3DX_DEFAULT,		// filter
		//		D3DX_DEFAULT,		// mip filter
		//		0,					// color key 
		//		NULL,				// source info
		//		NULL,				// pallette
		//		&pTextureData->pd3dImage ) )
		//{
		//	//MessageBoxW( NULL, pTextureData->FileName->WText, L"Failed to load texture file", MB_ICONERROR ); 
 		//	return G_FAIL;
		//}
		//pTextureData->pd3dImage->GetSurfaceLevel( 0, &pSurface );
		//if( pSurface )
		//{
		//	pSurface->GetDesc( pDesc );
		//	pSurface->Release();
		//}
		//grelease( &pTextureData->BufferData );
		//return 0;
	}

	info_printf("Loading 16 bit texture with alpha key.");
	if( D3D_OK != D3DXCreateTextureFromFileExW(pDevice,
			gget(pTextureData->FileName)->WText, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, //D3DX_DEFAULT, D3DX_DEFAULT, 
			pTextureData->nLevels,		// mip levels
			NULL,				// usage
			D3DFMT_R5G6B5,		// texture color format
			D3DPOOL_SYSTEMMEM,	// memory class
			D3DX_FILTER_NONE,		// filter
			D3DX_FILTER_NONE,		// mip filter
			0,					// color key 
			NULL,				// source info
			NULL,				// pallette
			&pSysTex ) )
	{
		error_printf("D3DXCreateTextureFromFileExW() returned 0x%X!", hr);
		//MessageBoxW( NULL, pTextureData->FileName->WText, L"Failed to load texture file", MB_ICONERROR ); 
		return G_FAIL;
	}
	
	pSysTex->GetSurfaceLevel( 0, &pSurface );
	pSurface->GetDesc( pDesc );
	pSurface->Release();

	gpod_ptr<SBuffer> newBuffer;
	uint32_t i, x, y, nSize=pDesc->Width*pDesc->Height, nIndex=0;
	if( 0 > gcreateBuffer( GDATATYPE_UINT8_4, GUSAGE_TEXEL, nSize, &newBuffer ) )
	{
		error_printf("gcreateBuffer()");
		return G_FAIL;
	};
	newBuffer->nColumnCount = pDesc->Width;
	bool bIsAlpha=false;
	RECT rc = { 0, 0, (LONG)pDesc->Width, (LONG)pDesc->Height };
	D3DLOCKED_RECT LockedRect;
	if( hr = pSysTex->LockRect( 0, &LockedRect, &rc, 0 ) )
		return -1;
	uint32_t LineWidth = LockedRect.Pitch >> 1;
	DWORD origcolor, ourcolor = 0xffFFFFFF;
	nBytesCounter += LockedRect.Pitch*pDesc->Height;
	info_printf( "%u Texture bytes and counting!", nBytesCounter );
	for( i=0; i<nSize; i++ )
	{ // check if our texture matches the alpha key in any pixel
		x = i%pDesc->Width;
		y = i/pDesc->Width;
		nIndex = (y*LineWidth)+x;
		origcolor = ((uint16_t*)LockedRect.pBits)[nIndex];
		ourcolor = D3DCOLOR_ARGB( 0xff, 
			int(((origcolor & 0xF800) >>11)/((float)0x1f)*255), 
			int(((origcolor & 0x7e0) >>5)/((float)0x3f)*255), 
			int((origcolor & 0x1F)/((float)0x1f)*255) );
		if( equal_color( ((ourcolor & 0xFF0000)>>16), ((pTextureData->nAlphaKey & 0xFF0000)>>16),
			((ourcolor & 0xFF00)>>8), ((pTextureData->nAlphaKey & 0xFF00)>>8),
			(ourcolor & 0xFF), (pTextureData->nAlphaKey & 0xFF), 5 ) )
		//if( (ourcolor & 0xFFFFFF) == (pTextureData->nAlphaKey & 0xFFFFFF) )
		{ 
			bIsAlpha = true;
			break;
		};
	}
	pSysTex->UnlockRect( 0 );

	if( bIsAlpha )
	{
		D3DLOCKED_RECT LockedRect2;
		pDesc->Format = D3DFMT_A8R8G8B8;
		if( hr = pDevice->CreateTexture( pDesc->Width, pDesc->Height, pTextureData->nLevels, pDesc->Usage,
			pDesc->Format, D3DPOOL_SYSTEMMEM, &pSysTex2, 0 ) )
		{
			error_printf("IDirect3DDevice9::CreateTexture() returned 0x%X!", hr);
			if( hr == D3DERR_INVALIDCALL )
			{ // ?? cannot remember what is this about
				// MessageBoxW( 0, L"Confirmed", L"Debug", 0 ); ??
			}
			return hr;
		}
		if( hr = pSysTex->LockRect( 0, &LockedRect, &rc, 0 ) )
			return -1;
		if( hr = pSysTex2->LockRect( 0, &LockedRect2, &rc, 0 ) )
			return -1;
		uint32_t i, x, y, nSize=pDesc->Width*pDesc->Height, nIndex=0, nIndex2=0;
		void* pPixelsSrc=0, * pPixelsDst=0;
		uint32_t LineWidth2 = LockedRect2.Pitch >> 2;
		for( i=0; i<nSize; i++ )
		{ // check if our texture matches the alpha key in any pixel
			x = i%pDesc->Width;
			y = i/pDesc->Width;
			nIndex = (y*LineWidth)+x;
			nIndex2 = (y*LineWidth2)+x;
			origcolor = ((uint16_t*)LockedRect.pBits)[nIndex];
			ourcolor = D3DCOLOR_ARGB( 0xff, int(((origcolor & 0xF800) >>11)/((float)0x1f)*255), 
				int(((origcolor & 0x7e0) >>5)/((float)0x3f)*255), 
				int((origcolor & 0x1F)/((float)0x1f)*255) );
			if( equal_color( ((ourcolor & 0xFF0000)>>16), ((pTextureData->nAlphaKey & 0xFF0000)>>16),
				((ourcolor & 0xFF00)>>8), ((pTextureData->nAlphaKey & 0xFF00)>>8),
				(ourcolor & 0xFF), (pTextureData->nAlphaKey & 0xFF), 5 ) )
			//if( (ourcolor & 0xFFFFFF) == (pTextureData->nAlphaKey & 0xFFFFFF) )
			{
				((uint32_t*)LockedRect2.pBits)[nIndex2] = 0;//0x00FFFFFF;
			}
			else
			{
				((uint32_t*)LockedRect2.pBits)[nIndex2] = ourcolor;
				((uint32_t*)newBuffer->pByteArray)[i] = ourcolor;
			}
		}
		pSysTex2->UnlockRect( 0 );
		pSysTex->UnlockRect( 0 );
		pSysTex->Release();
		pSysTex = pSysTex2; pSysTex2 = 0;
	}
	pDesc->Pool = D3DPOOL_DEFAULT;
	if( hr = pDevice->CreateTexture( pDesc->Width, pDesc->Height, pTextureData->nLevels, pDesc->Usage,
		pDesc->Format, pDesc->Pool, &pTextureData->pd3dImage, 0 ) )
	{
		error_printf("IDirect3DDevice9::CreateTexture() returned 0x%X!", hr);
		if( hr == D3DERR_INVALIDCALL )
		{ // again ?
			// MessageBoxW( 0, L"Confirmed", L"Debug", 0 );
		}
		return hr;
	}
	if( 0 > (hr = pDevice->UpdateTexture( pSysTex, pTextureData->pd3dImage )) )
	{
		error_printf("IDirect3DDevice9::UpdateTexture() returned 0x%X!", hr);
		if( pSysTex )
			pSysTex->Release();
		return hr;
	}
	if( pSysTex )
		pSysTex->Release();
	return G_OK;
}

error_t _CGD3D9ResourceManager::__createDXImageFromBuffer( SImage* pTextureData )
{	
	HRESULT hr = 0;
	D3DSURFACE_DESC* pDesc = &pTextureData->PrimarySurfaceDesc;
	IDirect3DDevice9* pDevice = m_pD3D9Device;
	IDirect3DTexture9* pSysTex = 0;//, * pSysTex2 = 0;

	if( hr = pDevice->CreateTexture( pDesc->Width, pDesc->Height, pTextureData->nLevels, 0, pDesc->Format, D3DPOOL_SYSTEMMEM, &pSysTex, 0 ) )
	{
		error_printf("IDirect3DDevice9::CreateTexture() FAILED!\n"
			"Size: [%i x %i]\n"
			"Mipmap Levels: %i\n"
			"Usage: 0\n"
			"Format: 0x%X\n"
			"Pool: D3DPOOL_SYSTEMMEM"
			,
			pDesc->Width,
			pDesc->Height,
			pTextureData->nLevels,
			(int)pDesc->Format
			);
		error_printf("IDirect3DDevice9::CreateTexture() FAILED! HRESULT=0x%X", hr);
		return hr;
	}
	RECT rc = { 0, 0, (LONG)pDesc->Width, (LONG)pDesc->Height };
	D3DLOCKED_RECT LockedRect;
	if( hr = pSysTex->LockRect( 0, &LockedRect, &rc, 0 ) )
		return -1;
	uint32_t i, x, y, nSize=pDesc->Width*pDesc->Height, nIndex=0;

	uint32_t nElementCount	= GTYPEID_ELEMENTCOUNT	(gget(pTextureData->BufferData)->DataFormat),
		nElementBytes		= GTYPEID_ELEMENTBYTES	(gget(pTextureData->BufferData)->DataFormat),
		nTypeBytes			= GTYPEID_TOTALBYTES	(gget(pTextureData->BufferData)->DataFormat);

	void* pPixels = gget(pTextureData->BufferData)->pByteArray;
	uint32_t LineWidth = LockedRect.Pitch;
	if( nTypeBytes == 2 ) 
		LineWidth = LockedRect.Pitch >> 1;
	else if( nTypeBytes == 4 ) 
		LineWidth = LockedRect.Pitch >> 2;

	for( i=0; i<nSize; i++ )
	{
		x = i%pDesc->Width;
		y = i/pDesc->Width;
		nIndex = (y*LineWidth)+x;
		if( nTypeBytes == 1 )
		{
			((uint8_t*)LockedRect.pBits)[nIndex] = ((uint8_t*)pPixels)[i];
		}
		else if( nTypeBytes == 2 )
		{
			((uint16_t*)LockedRect.pBits)[nIndex] = ((uint16_t*)pPixels)[i];
		}
		else if( nTypeBytes == 4 )
		{
			((uint32_t*)LockedRect.pBits)[nIndex] = ((uint32_t*)pPixels)[i];
		}
	}
	pSysTex->UnlockRect( 0 );

	if( hr = pDevice->CreateTexture( pDesc->Width, pDesc->Height, pTextureData->nLevels, pDesc->Usage,
		pDesc->Format, pDesc->Pool, &pTextureData->pd3dImage, 0 ) )
	{
		pSysTex->Release();
		error_printf("Failed to create d3d texture to store image!");
		return hr;
	}
	if( hr = pDevice->UpdateTexture( pSysTex, pTextureData->pd3dImage ) )
	{
		pSysTex->Release();
		return hr;
	}
	pSysTex->Release();
	return 0;
}

error_t _CGD3D9ResourceManager::__createDXImageFromFile16( SImage* pTextureData )
{
	error_printf("TODO: __createDXImageFromFile16()");
	return -1;
}

error_t _CGD3D9ResourceManager::__createDXImageFromFile( SImage* pTextureData, bool b16Bit )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXImageFromFile32" );
		
	static int nBytesCounter = 0;
	HRESULT hr = 0;
	IDirect3DDevice9* pDevice = m_pD3D9Device;
	IDirect3DTexture9* pSysTex = 0, *pFinalTex=0, *pOldTex=0;
	IDirect3DSurface9* pSurface = 0;
	D3DSURFACE_DESC* psDesc = &pTextureData->PrimarySurfaceDesc;
	if( D3D_OK != D3DXCreateTextureFromFileExW(pDevice,
			gget(pTextureData->FileName)->WText, 
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, //aa D3DX_DEFAULT, D3DX_DEFAULT, 
			pTextureData->nLevels,						// mip levels
			NULL,										// usage
			(b16Bit ? D3DFMT_R5G6B5 : D3DFMT_A8R8G8B8),	// texture color format
			D3DPOOL_SYSTEMMEM,							// memory class
			D3DX_DEFAULT,								// filter
			D3DX_DEFAULT,								// mip filter
			0x00000000,									// color key 
			NULL,										// source info
			NULL,										// pallette
			&pSysTex ) )
	{
		//MessageBoxW( NULL, pTextureData->FileName->WText, L"Failed to load texture file", MB_ICONERROR ); 
		error_wprintf(L"D3DXCreateTextureFromFileExW() failed to load texture file: %s", gget(pTextureData->FileName)->WText);
		return G_FAIL;
	}

	pSysTex->GetSurfaceLevel( 0, &pSurface );
	if( pSurface )
	{
		pSurface->GetDesc( psDesc );
		pSurface->Release(); pSurface = 0;
	}

	GDATA_TYPE bufferdt = GDataTypeFromD3DFMT(psDesc->Format, GUSAGE_TEXEL);

	gpod_ptr<SBuffer> newBuffer, oldBuffer;
	if( 0 > gcreateBuffer(bufferdt, GUSAGE_TEXEL, psDesc->Width*psDesc->Height, &newBuffer) )
	{
		error_printf("gcreateBuffer");
		SAFE_RELEASE(pSysTex)
		return -1;
	}
	newBuffer->nColumnCount = psDesc->Width;

	RECT rc = { 0, 0, (LONG)psDesc->Width, (LONG)psDesc->Height };
	D3DLOCKED_RECT LockedRect;
	if( hr = pSysTex->LockRect( 0, &LockedRect, &rc, 0 ) )
		return -1;
	uint32_t i, x, y, nSize=psDesc->Width*psDesc->Height, nIndex=0;

	uint32_t nElementCount = GTYPEID_ELEMENTCOUNT(newBuffer->DataFormat),
		nElementBytes = GTYPEID_ELEMENTBYTES(newBuffer->DataFormat),
		nTypeBytes = GTYPEID_TOTALBYTES(newBuffer->DataFormat);

	//void* pPixels = pTextureData->BufferData->pByteArray;
	uint32_t LineWidth = LockedRect.Pitch;
	if( nTypeBytes == 2 ) 
		LineWidth = LockedRect.Pitch >> 1;
	else if( nTypeBytes == 4 ) 
		LineWidth = LockedRect.Pitch >> 2;

	void* pPixelsTarget	= newBuffer->pByteArray;
	void* pPixelsSource	= LockedRect.pBits;

	//for( i=0; i<nSize; i++ )
	//{
	//	x = i%psDesc->Width; y = i/psDesc->Width;
	//	nIndex = (y*LineWidth)+x;
	//	switch( nTypeBytes )
	//	{
	//	case 1:
	//		((uint8_t*)pPixelsTarget)[nIndex] = ((uint8_t*)pPixelsSource)[i];
	//	case 2:
	//		((uint16_t*)pPixelsTarget)[nIndex] = ((uint16_t*)pPixelsSource)[i];
	//	case 4:
	//		((uint32_t*)pPixelsTarget)[nIndex] = ((uint32_t*)pPixelsSource)[i];
	//	}
	//}

	switch( nTypeBytes )
	{
	case 1:
		for( i=0; i<nSize; i++ )
		{
			x = i%psDesc->Width; y = i/psDesc->Width;
			nIndex = (y*LineWidth)+x;
			((uint8_t*)pPixelsTarget)[nIndex] = ((uint8_t*)pPixelsSource)[i];
		}
		break;
	case 2:
		for( i=0; i<nSize; i++ )
		{
			x = i%psDesc->Width; y = i/psDesc->Width;
			nIndex = (y*LineWidth)+x;
			((uint16_t*)pPixelsTarget)[nIndex] = ((uint16_t*)pPixelsSource)[i];
		}
		break;
	case 4:
		for( i=0; i<nSize; i++ )
		{
			x = i%psDesc->Width; y = i/psDesc->Width;
			nIndex = (y*LineWidth)+x;
			((uint32_t*)pPixelsTarget)[nIndex] = ((uint32_t*)pPixelsSource)[i];
		}
		break;
	};

	pSysTex->UnlockRect( 0 );
	psDesc->Pool = D3DPOOL_DEFAULT;
	if( hr = pDevice->CreateTexture( psDesc->Width, psDesc->Height, pTextureData->nLevels, psDesc->Usage,
		psDesc->Format, psDesc->Pool, &pFinalTex, 0 ) )
	{
		pSysTex->Release();
		error_printf("Failed to create d3d texture to store image!");
		return hr;
	}
	if( hr = pDevice->UpdateTexture( pSysTex, pFinalTex ) )
	{
		error_printf("Failed to update texture!");
		pSysTex->Release();
		return hr;
	}

	oldBuffer = pTextureData->BufferData;
	pTextureData->BufferData = gacquire( newBuffer );

	pOldTex = pTextureData->pd3dImage;
	pTextureData->pd3dImage = pFinalTex;
	if( pOldTex ) pOldTex->Release();
	SAFE_RELEASE(pSysTex)
	return 0;
}
