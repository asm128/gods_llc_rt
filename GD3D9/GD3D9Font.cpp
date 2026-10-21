
//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"
#include "GD3D9_function_macros.h"
#include <strsafe.h>

#include "GCore_printf.h"
#include "GDebugger_core.h"


using namespace god;

_CGD3D9Font::_CGD3D9Font( CGD3D9ResourceManager *pManager )
	//:m_FontData(0,0, XFont_Free_handler)
{
	GPROFILE_METHOD( "_CGD3D9Font", "_CGD3D9Font" );
	m_pManager			= pManager;
	m_nFontID			= 0;
	__pfreeInterface		= greleaseInterface;

	__nRefCount			= 0;
}
//
_CGD3D9Font::~_CGD3D9Font( void )
{
	GPROFILE_METHOD( "_CGD3D9Font", "~_CGD3D9Font" );
	if( m_pManager && m_nFontID )
		m_pManager->releaseResourceID( GIID_FONT, m_nFontID );
}
//
int32_t _CGD3D9Font::internalInit( int32_t nID, const wchar_t* pFacename, uint32_t nWidth, 
						uint32_t nHeight, uint32_t nWeight, bool bItalic )
{
	GPROFILE_METHOD( "_CGD3D9Font", "internalInit" );
	debug_wprintf( L"Initializing CGD3D9Font with ID=%i, pFacename(0x%p) = \"%s\".", nID, pFacename, pFacename );
	gpod_ptr<SXFont> FontItemRequest( m_FontData );
	FontItemRequest.create();
	D3DXFONT_DESC* pDesc = &FontItemRequest->FontDesc;
	pDesc->MipLevels		= 1;
	pDesc->OutputPrecision	= OUT_DEFAULT_PRECIS;
	pDesc->Quality			= DEFAULT_QUALITY;
	pDesc->CharSet			= DEFAULT_CHARSET;
	pDesc->PitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;
	pDesc->Width			= nWidth;
	pDesc->Height			= nHeight;
	pDesc->Weight			= nWeight;
	pDesc->Italic			= bItalic ? TRUE : FALSE;
	StringCchCopyW( pDesc->FaceName, 32, pFacename );
	
	m_FontData = m_pManager->getFontItem( FontItemRequest.get_pointer() );
	if( !m_FontData )
	{
		error_printf( "getFontItem() FAILED!" );
		return G_FAIL;
	}
	else
	{
		info_printf( "getFontItem() succeeded!" );
	}
	m_nFontID			= nID;
	__nRefCount			= 1;
	info_printf( "CGD3D9Font initialized!" );
	return 0;
}

//
uint32_t _CGD3D9Font::GetWidth( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetWidth" );
	if( !m_FontData )
		return 0;
	return m_FontData->FontDesc.Width;
};
//
uint32_t _CGD3D9Font::GetHeight( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetHeight" );
	if( !m_FontData )
		return 0;
	return m_FontData->FontDesc.Height;
};
//
uint32_t _CGD3D9Font::GetWeight( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetWeight" );
	if( !m_FontData )
		return 0;
	return m_FontData->FontDesc.Weight;
};

//
//const char* _CGD3D9Font::GetFacename( void ) const
//{
//	GPROFILE_METHOD( "_CGD3D9Font", "GetFacename" );
//	static char es[33] = {0};
//	static const char defa = '?';
//	if( !m_FontData )
//	{
//		es[0] = 0;
//		return es;
//	}
//	WideCharToMultiByte( CP_ACP, MB_PRECOMPOSED, m_FontData->FontDesc.FaceName, 32, es, 33, &defa, 0 );
//	return es;
//};

int32_t _CGD3D9Font::GetFacename( char* pFacename, uint32_t* nSize ) const
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetFacename" );
	if( 0 == nSize )
		return G_FAIL;
	if( 0 == pFacename )
	{
		*nSize = 32;
		return 0;
	}
	//const char* fn = &m_FontData->FontDesc.FaceName[0];
	//uint32_t sz = min( 32, (*nSize) );
	//for( uint32_t i=0; i<sz; i++ )
	//{
	//	pFacename[i] = fn[i];
	//}
	//static const char defa = '?';
	//BOOL b=false;
	//WideCharToMultiByte( CP_ACP, MB_PRECOMPOSED, m_FontData->FontDesc.FaceName, 32, pFacename, *nSize, &defa, &b );
	size_t n=0;
	wcstombs_s(&n, pFacename, *nSize, m_FontData->FontDesc.FaceName, 32);
	return 0;
};

bool _CGD3D9Font::GetItalic( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetItalic" );
	if( !m_FontData )
		return false;
	return m_FontData->FontDesc.Italic == TRUE;
}

//--------------------------------------------------------
int32_t _CGD3D9Font::GetID( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetID" );
	return m_nFontID;
};

GRESOURCE_TYPE _CGD3D9Font::GetResourceType( void ) const
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetResourceType" );
	return GRESOURCE_FONT;
};

void* _CGD3D9Font::GetPrivateData( void )
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetPrivateData" );
	if( !m_FontData )
		return 0;
	if( m_FontData->pd3dXFont )
		m_FontData->pd3dXFont->AddRef();
	return m_FontData->pd3dXFont;
};


int32_t _CGD3D9Font::Draw( const char* pText, int32_t nWCharCount, GRDS(SRectangle) lpQuad, bool bNormalized, uint32_t dwFormat, uint32_t Color, uint32_t nCodePage )
{
	GPROFILE_METHOD( "_CGD3D9Font", "Draw" );
	if( !m_FontData )
		return 0;

	ID3DXFont* pFont = m_FontData->pd3dXFont;
	if( 0 == pFont )
		return 0;

	uint32_t nWidth = m_pManager->GetWidth(0), 
		nHeight = m_pManager->GetHeight(0);
	
	RECT rc; 
	if( bNormalized )
	{
		rc.left		= (LONG)( nWidth* gget(lpQuad)->vOffset.x		);
		rc.top		= (LONG)( nHeight*gget(lpQuad)->vOffset.y		);
		rc.right	= (LONG)( rc.left+nWidth*gget(lpQuad)->vSize.x	);
		rc.bottom	= (LONG)( rc.top+nHeight*gget(lpQuad)->vSize.y	);
	}
	else
	{
		rc.left		= (LONG)( gget(lpQuad)->vOffset.x		);
		rc.top		= (LONG)( gget(lpQuad)->vOffset.y		);
		rc.right	= (LONG)( rc.left+gget(lpQuad)->vSize.x	);
		rc.bottom	= (LONG)( rc.top +gget(lpQuad)->vSize.y	);
	}

	nHeight = pFont->DrawTextA( 0, pText, nWCharCount, &rc, dwFormat, Color );
	return nHeight;
};

//------------------- IGInterface  
void* _CGD3D9Font::GetInterfacePointer( GINTERFACEID_TYPE InterfaceID )
{
	GPROFILE_METHOD( "_CGD3D9Font", "GetInterfacePointer" );
	switch( InterfaceID )
	{
	case GIID_CUSTOMINTERFACE:
	case GIID_FONT:
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
