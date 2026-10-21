//#define GPROFILE_ENABLED
#include "GD3D9_core.h"

#include "GCore_printf.h"
#include "GDebugger_core.h"
#include "glist.h"
#include "GCore_function_macros.h"

//using namespace god;

GDEFINE_POD_CUSTOMCLEANUP( god, SD3D9Device			, god::device_Free_handler				);
GDEFINE_POD_CUSTOMCLEANUP( god, SManagerBuffer		, god::ManagerBuffer_Free_handler		);
GDEFINE_POD_CUSTOMCLEANUP( god, SDecl				, god::Decl_Free_handler				);
GDEFINE_POD_CUSTOMCLEANUP( god, SFunction			, god::Shader_Free_handler				);
GDEFINE_POD_CUSTOMCLEANUP( god, SImage				, god::Texture_Free_handler				);
GDEFINE_POD_CUSTOMCLEANUP( god, SXFont				, god::XFont_Free_handler				);
GDEFINE_POD_CUSTOMCLEANUP( god, SEffect				, god::XEffect_Free_handler				);
GDEFINE_POD_CUSTOMCLEANUP( god, SBlendState			, god::BlendState_Free_handler			);
GDEFINE_POD_CUSTOMCLEANUP( god, SDepthStencilState	, god::DepthStencil_Free_handler		);
GDEFINE_POD_CUSTOMCLEANUP( god, SRasterizerState	, god::Rasterizer_Free_handler			);
//GDEFINE_PRINTINFOSTRING( god, SD3D9Device			, "IDirect3DDevice9*: 0x%p", in_CoreInstance->pDevice );
//GDEFINE_PRINTINFOSTRING( god, SManagerBuffer		, "IUnknown: 0x%p", (void*)in_CoreInstance->pd3dBuffer		);
//GDEFINE_PRINTINFOSTRING( god, SDecl					, "IUnknown: 0x%p", (void*)in_CoreInstance->pd3dDecl		);
//GDEFINE_PRINTINFOSTRING( god, SFunction				, "IUnknown: 0x%p", (void*)(in_CoreInstance->pd3dPixelShader ? (IUnknown*)in_CoreInstance->pd3dPixelShader : (IUnknown*)in_CoreInstance->pd3dVertexShader));
//GDEFINE_PRINTINFOSTRING( god, SImage				, "IUnknown: 0x%p", (void*)in_CoreInstance->pd3dImage		);
//GDEFINE_PRINTINFOSTRING( god, SEffect				, "IUnknown: 0x%p", (void*)in_CoreInstance->pd3dXProgram	);
//GDEFINE_PRINTINFOSTRING( god, SXFont				, "IUnknown: 0x%p", (void*)in_CoreInstance->pd3dXFont		);
//GDEFINE_PRINTINFOSTRING( god, SBlendState			, "IUnknown: 0x%p", (void*)0 );
//GDEFINE_PRINTINFOSTRING( god, SDepthStencilState	, "IUnknown: 0x%p", (void*)0 );
//GDEFINE_PRINTINFOSTRING( god, SRasterizerState		, "IUnknown: 0x%p", (void*)0 );


//--------------------------------------------------- HELPER FUNCTIONS
god::GDATA_TYPE god::GTypeFromD3DDeclType( D3DDECLTYPE DeclType )
{
	GPROFILE_FUNCTION();
	GDATA_TYPE nElementType;
	switch( DeclType )
	{
	case D3DDECLTYPE_DEC3N: // these we will probably never use
		nElementType = GTYPEID_MAKE( true, true, false, false, 3, 10 );
		break;
	case D3DDECLTYPE_UDEC3:
		nElementType = GTYPEID_MAKE( false, false, false, false, 3, 10 );
		break;
	case D3DDECLTYPE_UBYTE4N:
		nElementType = GDATATYPE_UINT8_NORM_4;
		break;
	case D3DDECLTYPE_UBYTE4:
		nElementType = GDATATYPE_UINT8_4;
		break;
	case D3DDECLTYPE_D3DCOLOR:
		nElementType = GDATATYPE_UINT8_NORM_4;
		break;
	case D3DDECLTYPE_SHORT2:
		nElementType = GDATATYPE_UINT16_2;
		break;
	case D3DDECLTYPE_SHORT2N:
		nElementType = GDATATYPE_UINT16_NORM_2;
		break;
	case D3DDECLTYPE_SHORT4:
		nElementType = GDATATYPE_UINT16_4;
		break;
	case D3DDECLTYPE_SHORT4N:
		nElementType = GDATATYPE_UINT16_NORM_4;
		break;
	case D3DDECLTYPE_FLOAT16_2:
		nElementType = GDATATYPE_FLOAT16_2;
		break;
	case D3DDECLTYPE_FLOAT16_4:
		nElementType = GDATATYPE_FLOAT16_4;
		break;
	case D3DDECLTYPE_FLOAT1:
		nElementType = GDATATYPE_FLOAT32;
		break;
	case D3DDECLTYPE_FLOAT2:
		nElementType = GDATATYPE_FLOAT32_2;
		break;
	case D3DDECLTYPE_FLOAT3:
		nElementType = GDATATYPE_FLOAT32_3;
		break;
	case D3DDECLTYPE_FLOAT4:
		nElementType = GDATATYPE_FLOAT32_4;
		break;
	default:
//	case D3DDECLTYPE_UNUSED:
		nElementType = GDATATYPE_UNKNOWN;
	}
	return nElementType;
};


god::GRESOURCE_USAGE god::GUsageFromD3DDeclUsage( D3DDECLUSAGE DeclUsage )
{
	GPROFILE_FUNCTION();
	uint32_t nUsage;
	switch( DeclUsage )
	{
	case D3DDECLUSAGE_COLOR:
		nUsage = GUSAGE_VERTEXCOLOR;
		break;
	case D3DDECLUSAGE_NORMAL:
		nUsage = GUSAGE_NORMAL;
		break;
	case D3DDECLUSAGE_BLENDINDICES:
		nUsage = GUSAGE_BLENDINDICES;
		break;
	case D3DDECLUSAGE_BLENDWEIGHT:
		nUsage = GUSAGE_BLENDWEIGHT;
		break;
	case D3DDECLUSAGE_POSITION:
		nUsage = GUSAGE_VERTEX;
		break;
	case D3DDECLUSAGE_TANGENT:
		nUsage = GUSAGE_TANGENT;
		break;
	case D3DDECLUSAGE_TEXCOORD:
		nUsage = GUSAGE_TEXCOORD;
		break;
	case D3DDECLUSAGE_DEPTH:
		nUsage = GUSAGE_DEPTH;
		break;
	case D3DDECLUSAGE_BINORMAL:
		nUsage = GUSAGE_BITANGENT;
		break;
	case D3DDECLUSAGE_FOG:
//		break;
	case D3DDECLUSAGE_TESSFACTOR:
//		break;
	case D3DDECLUSAGE_PSIZE:
//		break;
	case D3DDECLUSAGE_SAMPLE:
//		break;
	case D3DDECLUSAGE_POSITIONT:
//		break;
	default:
	//case D3DDE:
		nUsage = GUSAGE_UNKNOWN;
		break;
	}
	return nUsage;
}

god::GDATA_TYPE god::GDataTypeFromD3DFMT( D3DFORMAT d3dFormat, GRESOURCE_USAGE Usage )
{
	GPROFILE_FUNCTION();
	GDATA_TYPE retval		= GDATATYPE_UNKNOWN;
	const char *pSelectedFormat = "D3DFMT_UNKNOWN";
	
	switch( d3dFormat )
	{
	case D3DFMT_A8: 
		pSelectedFormat = "D3DFMT_A8";
		retval			= GDATATYPE_UINT8;
		break;
	case D3DFMT_R5G6B5:
		pSelectedFormat = "D3DFMT_R5G6B5";
		retval			= GDATATYPE_UINT8_2;
		break;
	case D3DFMT_D15S1: 
		pSelectedFormat = "D3DFMT_D15S1";
		retval = GDATATYPE_UINT8_2;
		break;
	case D3DFMT_A4R4G4B4: 
		pSelectedFormat = "D3DFMT_A4R4G4B4";
		retval = GDATATYPE_UINT8_2;
		break;
	case D3DFMT_A1R5G5B5: 
		pSelectedFormat = "D3DFMT_A1R5G5B5";
		retval = GDATATYPE_UINT8_2;
		break;
	case D3DFMT_X8R8G8B8: 
		pSelectedFormat = "D3DFMT_X8R8G8B8";
		retval = GDATATYPE_UINT8_3;
		break;
	case D3DFMT_A8R8G8B8: 
		pSelectedFormat = "D3DFMT_A8R8G8B8";
		retval = GDATATYPE_UINT8_4;
		break;
	case D3DFMT_G16R16: 
		pSelectedFormat = "D3DFMT_G16R16";
		retval = GDATATYPE_UINT16_2;
		break;
	case D3DFMT_A16B16G16R16: 
		pSelectedFormat = "D3DFMT_A16B16G16R16";
		retval = GDATATYPE_UINT16_4;
		break;
	case D3DFMT_G32R32F: 
		pSelectedFormat = "D3DFMT_G32R32F";
		retval = GDATATYPE_UINT32_2;
		break;
	default:
		error_printf("Unrecognized format!");
		pSelectedFormat = "Unrecognized format!";
	case D3DFMT_UNKNOWN: 
		retval = GDATATYPE_UNKNOWN;
		break;
	}
	info_printf( "%s selected from D3DFORMAT=%s.", god::getStringFromDataType(retval), pSelectedFormat );

	return retval;
};

D3DFORMAT god::GD3DFMTFromGDataType( GRESOURCE_USAGE GUsage, GDATA_TYPE GDataType )
{
	GPROFILE_FUNCTION();
	uint32_t nElementSize	= GTYPEID_ELEMENTBYTES(GDataType);
	uint32_t nElementCount	= GTYPEID_ELEMENTCOUNT(GDataType);
	D3DFORMAT retval = D3DFMT_UNKNOWN;
	const char* pSelectedText;
	switch( nElementSize )
	{
	case 1:
		switch( nElementCount )
		{
		case 1:
			if( GUsage == GUSAGE_STENCIL )
			{
				retval = D3DFMT_S8_LOCKABLE;
				pSelectedText = "D3DFMT_S8_LOCKABLE";
			}
			else if( GUsage == GUSAGE_ALPHA )
			{
				retval = D3DFMT_A8;
				pSelectedText = "D3DFMT_A8";
			}
			else 
			{
				retval = D3DFMT_L8;
				pSelectedText = "D3DFMT_L8";
			}
			break;
		case 2:
			if( GUsage == GUSAGE_DEPTHSTENCIL )
			{
				retval = D3DFMT_D15S1;
				pSelectedText = "D3DFMT_D15S1";
			}
			else if( GUsage == GUSAGE_DEPTH )
			{
				retval = D3DFMT_D16;
				pSelectedText = "D3DFMT_D16";
			}
			else if( GUsage == GUSAGE_TEXEL )
			{
				retval = D3DFMT_A4R4G4B4;
				pSelectedText = "D3DFMT_A4R4G4B4";
			}
			else if( GUsage == GUSAGE_RENDERTARGET )
			{
				retval = D3DFMT_A1R5G5B5;
				pSelectedText = "D3DFMT_A1R5G5B5";
			}
			else
			{
				retval = D3DFMT_A1R5G5B5;
				pSelectedText = "D3DFMT_A1R5G5B5";
			}
			break;
		case 3:
			if( GUsage == GUSAGE_DEPTH
				|| GUsage == GUSAGE_DEPTHSTENCIL )
			{
				retval = D3DFMT_D24X8;
				pSelectedText = "D3DFMT_D24X8";
			}
			else
			{
				retval = D3DFMT_X8R8G8B8;
				pSelectedText = "D3DFMT_X8R8G8B8";
			}
			break;
		case 4:
			if( GUsage == GUSAGE_DEPTHSTENCIL )
			{
				retval = D3DFMT_D24S8;
				pSelectedText = "D3DFMT_D24S8";
			}
			else
			{
				retval = D3DFMT_A8R8G8B8;
				pSelectedText = "D3DFMT_A8R8G8B8";
			}
			break;
		default:
			retval = D3DFMT_UNKNOWN;
			pSelectedText = "D3DFMT_UNKNOWN";
			break;
		}
		break;
	case 2:
		switch( nElementCount )
		{
		case 1:
			retval = D3DFMT_R5G6B5;
			pSelectedText = "D3DFMT_R5G6B5";
			break;
		case 2:
			retval = D3DFMT_G16R16;
			pSelectedText = "D3DFMT_G16R16";
			break;
		//case 3:			 ??
		//	retval = D3DFMT_;??
		//	pSelectedText = "";
		//	break;
		case 4:
			retval = D3DFMT_A16B16G16R16;
			pSelectedText = "D3DFMT_A16B16G16R16";
			break;
		default:
			retval = D3DFMT_UNKNOWN;
			pSelectedText = "D3DFMT_UNKNOWN";
			break;
		}
		break;
	case 4:
		switch( nElementCount )
		{
		case 1:
			retval = D3DFMT_A8R8G8B8;
			pSelectedText = "D3DFMT_A8R8G8B8";
			break;
		case 2:
			retval = D3DFMT_G32R32F;
			pSelectedText = "D3DFMT_G32R32F";
			break;
		//case 3:
		//	retval = D3DFMT_;
		//	break;
		case 4:
			retval = D3DFMT_A32B32G32R32F;
			break;
		default:
			retval = D3DFMT_UNKNOWN;
			pSelectedText = "D3DFMT_UNKNOWN";
			break;
		}
		break;
	default:
		retval = D3DFMT_UNKNOWN;
		pSelectedText = "D3DFMT_UNKNOWN";
		break;
	}
	info_printf( "%s selected from GDATA_TYPE=%s and GRESOURCE_USAGE=0x%X.", pSelectedText, getStringFromDataType(GDataType), GUsage );
	
	return retval;
};

