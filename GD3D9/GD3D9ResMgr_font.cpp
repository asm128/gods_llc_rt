//#define GPROFILE_ENABLED
//
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include <strsafe.h>


#include "GCore_printf.h"
#include "GDebugger_core.h"

using namespace god;

__GDEFINE_FREE_MANAGER_GINTERFACE_LIST_GELEMENTARY( Font, Font );
__GDEFINE_GET_GINTERFACE_BY_ID_GELEMENTARY( Font, Font );

GRDS(SXFont) _CGD3D9ResourceManager::getFontItem( GRDS(SXFont) pFontItem )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "getFontItem" );
	if( 0 == pFontItem )
		return 0;

	wchar_t buffer[256], buffer2[256]; 
	StringCchCopyW( buffer, 256, gget(pFontItem)->FontDesc.FaceName );
#pragma warning( disable : 4996 )
	_wcslwr_s( buffer );
#pragma warning( default : 4996 )
	D3DXFONT_DESCW* pFontDesc1, * pFontDesc2;

	GRDS(SXFont) pNewItem = 0; // store here the return value
	uint32_t i, n = m_lstManagerXFont.size(); // set some counters
	for( i=0; i<n; i++ )
	{
		pNewItem = m_lstManagerXFont.get_pointer()[i];
		//if( 0 == pNewItem )
		//	continue;
		pFontDesc1 = &gget(pFontItem)->FontDesc;
		pFontDesc2 = &gget(pNewItem)->FontDesc;
		StringCchCopyW( buffer2, 256, pFontDesc2->FaceName );
#pragma warning( disable : 4996 )
		_wcslwr_s( buffer2 );
#pragma warning( default : 4996 )
		if( 0 == wcscmp( buffer2, buffer ) 
			&& pFontDesc1->Italic			== pFontDesc2->Italic
			&& pFontDesc1->Height			== pFontDesc2->Height
			&& pFontDesc1->Weight			== pFontDesc2->Weight
			&& pFontDesc1->Width			== pFontDesc2->Width
			&& pFontDesc1->CharSet			== pFontDesc2->CharSet
			&& pFontDesc1->MipLevels		== pFontDesc2->MipLevels
			&& pFontDesc1->PitchAndFamily	== pFontDesc2->PitchAndFamily
			&& pFontDesc1->Quality			== pFontDesc2->Quality
			&& pFontDesc1->OutputPrecision	== pFontDesc2->OutputPrecision
			)
			break;
		else
			pNewItem = 0;
	}
	if( 0 == pNewItem )
	{	
		//ghdlr_XFont garbage(0, 0, XFont_Free_handler);
		gpod_ptr<SXFont> garbage;
		if( __createDXXFont( gget(pFontItem)) )
		{ // don't acquire if failed to create
			error_printf("__createDXXFont() FAILED!");
			garbage = pFontItem;
			return 0;
		}
		if( m_lstManagerXFont.push( pFontItem ) )
		{
			error_printf("Cannot insert to list! out of memory?");
			garbage = pFontItem;
			return 0;
		};
		pNewItem = pFontItem;
	}

	return gacquire( pNewItem );
};

// 
int32_t _CGD3D9ResourceManager::CreateFont(	int32_t nID, uint32_t nHeight, uint32_t nWidth, uint32_t nWeight, 
											const char* pFaceName, uint32_t nCodePage, bool bItalic, 
											GOSH(Font)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateFont" );
	if(0 == pFaceName)
		return G_INVALIDARGUMENT;
	if( nID <= 0 )
		return G_INVALIDID;

	GHPTR(Font) garbage;
	if( nID )
	{
		if( (garbage = (CGD3D9Font*)this->GetFont( nID )) != 0 )
		{// can't create duplicated ID
			return G_INVALIDID;
		}
	}
	else
		nID = getAvailableID( GIID_FONT );

	CGD3D9Font* pFont = new CGD3D9Font( this );
	wchar_t faceName[32] = {0};
	MultiByteToWideChar(nCodePage, MB_PRECOMPOSED, pFaceName, strlen(pFaceName), faceName, 32);
	if( pFont->internalInit( nID, faceName, nWidth, nHeight, nWeight, bItalic ) )
	{
		garbage = pFont;
		error_printf("Cannot initialize CGD3D9Font instance!");
		return G_FAIL;
	}
	if( 0 > m_lstFont.push( pFont ) )
	{
		garbage = pFont;
		error_printf("Cannot append CGD3D9Font instance to manager list! Resource won't be created.");
		return -1;
	};
	if( ppCreated )
	{
		garbage = *ppCreated;
		*ppCreated = gacquireInterface( pFont );
	}
	return S_OK;
};

// 
int32_t _CGD3D9ResourceManager::CreateFont(	uint32_t nHeight, uint32_t nWidth, uint32_t nWeight, 
											const char* pFaceName, uint32_t nCodePage, bool bItalic, 
											GOSH(Font)* ppCreated )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "CreateFont" );
	if( 0 == ppCreated ) 
		return G_INVALIDARGUMENT;

	if( 0 == pFaceName )
		return G_INVALIDARGUMENT;

	int32_t nID = getAvailableID( GIID_DEVICEBUFFER );

	GHPTR(Font) garbage; 
	CGD3D9Font* pFont = new CGD3D9Font( this );
	wchar_t faceName[32] = {0};
	MultiByteToWideChar(nCodePage, MB_PRECOMPOSED, pFaceName, strlen(pFaceName), faceName, 32);
	if( pFont->internalInit( nID, faceName, nWidth, nHeight, nWeight, bItalic ) )
	{
		garbage = pFont;
		return G_FAIL;
	}
	if( 0 > m_lstFont.push( pFont ) )
	{
		garbage = pFont;
		error_printf( "Failed to append font to manager list! Out of memory?" );
	};

	garbage = *ppCreated;
	*ppCreated = gacquireInterface( pFont );
	return S_OK;
};

int32_t _CGD3D9ResourceManager::__createDXXFont( SXFont* pFontData )
{
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__createDXFont" );
	D3DXFONT_DESCW* pDesc = &pFontData->FontDesc;
	if( D3D_OK != D3DXCreateFontW( m_pD3D9Device, pDesc->Height, pDesc->Width,	
		pDesc->Weight, pDesc->MipLevels, pDesc->Italic, pDesc->CharSet, 
		pDesc->OutputPrecision, pDesc->Quality, pDesc->PitchAndFamily, pDesc->FaceName, 
		&pFontData->pd3dXFont ) )
	{
		error_printf("Failed to create ID3DXFont instance.");
		return G_FAIL;
	}
	pFontData->pd3dXFont->GetDescW( pDesc );
	return G_OK;
}

