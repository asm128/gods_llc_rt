// File:D3D9_core.h
//
// Copyright © 2011: Pablo Ariel Zorrilla Cepeda
// 
// Any use or distribution of this code is strictly prohibited
//
#if defined( DEBUG ) || defined( _DEBUG )
#define D3D_DEBUG_INFO
#endif

#include <d3dx9.h>
//#define DEBUG_PRINTF_ENABLED
//#define ERROR_PRINTF_ENABLED
//#define WARNING_PRINTF_ENABLED

#include "GMemory_core.h"
#include "GText_core.h"
#ifdef CreateFont
#undef CreateFont
#endif
#include "GDrivers_core.h"
#include "GGraphics_core.h"

#ifndef _GD3D9RESOURCEMANAGER_CORE_H
#define _GD3D9RESOURCEMANAGER_CORE_H

namespace god
{

#define SAFE_RELEASE(p) if( p ){ p->Release(); p=0; }
	// one for each AdapterFormat-BackbufferFormat-WindowMode
	// (windowed or fullscreen) combination that is valid
	typedef struct _GD3D9COMBO_DATA
	{
		uint32_t						nAdapter;			// Adapter ID we should find a way to fuck these redundances off
		D3DDEVTYPE						d3dDevType;			// HAL, SW, REF
		bool							bWindowed;			// windowed mode 
		D3DFORMAT						fmtAdapter;			// pixelbuffer
		D3DFORMAT						fmtBackBuffer;		// backbuffer
		D3DFORMAT						fmtDepthStencil;	// z/stencil format
		DWORD							dwBehavior;			// vertex processing
		D3DMULTISAMPLE_TYPE				msType;				// multisample type
	} GD3D9COMBO_DATA;
	/*----------------------------------------------------------------*/
	// up to three for each adapter
	typedef struct _GD3D9DEVICEINFO_DATA
	{
		uint32_t						nAdapter;		// Adapter ID
		D3DDEVTYPE						d3dDevType;		// HAL, SW, REF
		D3DCAPS9						d3dCaps;		// capabilites
		uint32_t						nComboCount;	// number of combos
		GD3D9COMBO_DATA					ComboData[80];	// combo
	} GD3D9DEVICEINFO_DATA;

	/*----------------------------------------------------------------*/
	typedef struct _GD3D9ADAPTER_DATA
	{
		D3DADAPTER_IDENTIFIER9			d3dAdapterIdentifier;
		uint32_t						nAdapter;	// Adapter ID
		uint32_t						nModeCount;			
		D3DDISPLAYMODE					d3dDisplayMode[150];
		uint32_t						nDeviceCount;		
		GD3D9DEVICEINFO_DATA			DeviceInfoData[3];	
	} GD3D9ADAPTER_DATA;

	//
	struct SD3D9Device
	{
		HWND							hWindowHandle;
		HWND							hChildWindows[32];
		IDirect3DSwapChain9*			pSwapChain[32];
		D3DPRESENT_PARAMETERS			PresentParameters;
		D3DCAPS9						Caps;
		int32_t							nRenderState[256]; // store here render states (thank god!)
		D3DDEVTYPE						DeviceType;
		uint32_t						nAdapter;
		DWORD							dwBehaviorFlags;
		IDirect3DDevice9*				pDevice;
	};
	GDECLARE_POD(SD3D9Device);
	static void device_Free_handler(SD3D9Device* stuff)
	{
		SAFE_RELEASE(stuff->pDevice);
		for (uint32_t i = 0; i<32; i++)
			SAFE_RELEASE(stuff->pSwapChain[i]);
	};

	struct SBlendState
	{
		bool							bActive;
		GRDS(SBlend)					BlendData;
	};
	GDECLARE_POD(SBlendState);
	static void BlendState_Free_handler(SBlendState* stuff)
	{
		grelease(&(stuff)->BlendData);
	};

	struct SRasterizerState
	{
		bool							bActive;
		GRDS(SRasterizer)				RasterizerData;
	};
	GDECLARE_POD(SRasterizerState);
	static void Rasterizer_Free_handler(SRasterizerState* stuff)
	{
		grelease(&stuff->RasterizerData);
	};


	struct SDepthStencilState
	{
		bool							bActive;
		GRDS(SStencil)					StencilData;
		GRDS(SDepth)					DepthData;
	};
	GDECLARE_POD(SDepthStencilState);
	static void DepthStencil_Free_handler(SDepthStencilState* stuff)
	{
		grelease(&stuff->StencilData);
		grelease(&stuff->DepthData);
	};
	// these structures bind engine data to the actual OS API data
	struct SDecl
	{
		bool							bActive;
		uint32_t						nElementCount;
		D3DVERTEXELEMENT9				VertexElement[MAX_FVF_DECL_SIZE];
		IDirect3DVertexDeclaration9*	pd3dDecl;
	};
	GDECLARE_POD(SDecl);
	static void Decl_Free_handler(SDecl* stuff)
	{
		SAFE_RELEASE((stuff)->pd3dDecl);
	};

	//
	struct SManagerBuffer
	{
		bool							bActive;
		GRDS(STextMedium)				FileName;
		D3DVERTEXBUFFER_DESC			BufferDesc;
		IDirect3DResource9*				pd3dBuffer;
		GRESOURCE_MODE					ResourceMode;
		GRDS(SBuffer)					BufferData;
		GRDS(SDecl)						pDeclData;
	};
	GDECLARE_POD(SManagerBuffer);
	static void ManagerBuffer_Free_handler(SManagerBuffer* stuff)
	{
		SAFE_RELEASE(stuff->pd3dBuffer);
		grelease(&stuff->BufferData);
		grelease(&stuff->pDeclData);
	};
	//
	struct SFunction
	{
		bool							bActive;
		GRDS(STextMedium)				FileName;
		GRDS(SBuffer)					SourceCode;
		wchar_t							szFilename;
		IDirect3DPixelShader9*			pd3dPixelShader;
		IDirect3DVertexShader9*			pd3dVertexShader;
		ID3DXConstantTable*				pd3dConstantTable;
	};
	GDECLARE_POD(SFunction);
	static void Shader_Free_handler(SFunction* stuff)
	{
		SAFE_RELEASE(stuff->pd3dPixelShader);
		SAFE_RELEASE(stuff->pd3dVertexShader);
		grelease(&stuff->SourceCode);
		grelease(&stuff->FileName);
	};

	//
	struct SImage
	{
		bool							bActive;
		bool							b16Bit;
		uint32_t						nAlphaKey;
		GRDS(STextMedium)				FileName;
		D3DSURFACE_DESC					PrimarySurfaceDesc;
		uint32_t						nLevels;
		IDirect3DTexture9*				pd3dImage;
		GRDS(SBuffer)					BufferData;
	};
	GDECLARE_POD(SImage);
	static void Texture_Free_handler(SImage* stuff)
	{
		SAFE_RELEASE(stuff->pd3dImage);
		grelease(&stuff->BufferData);
		grelease(&stuff->FileName);
	};
	//
	struct SEffect
	{
		bool							bActive;
		GRDS(STextMedium)				FileName;
		D3DXEFFECT_DESC					EffectDesc;
		D3DXHANDLE						ActiveTechHandle;
		ID3DXEffect*					pd3dXProgram;
//		_GDEVICEPROGRAM_DATA*			pProgramData;
	};
	GDECLARE_POD(SEffect);
	static void XEffect_Free_handler(SEffect* stuff)
	{
		SAFE_RELEASE((stuff)->pd3dXProgram);
		grelease(&(stuff)->FileName);
	};
	//
	struct SXFont
	{
		bool							bActive;
		D3DXFONT_DESCW					FontDesc;
		ID3DXFont*						pd3dXFont;
	};
	GDECLARE_POD(SXFont);
	static void XFont_Free_handler(SXFont* stuff)
	{
		SAFE_RELEASE((stuff)->pd3dXFont);
	};

	GDATA_TYPE GTypeFromD3DDeclType( D3DDECLTYPE DeclType );
	GRESOURCE_USAGE GUsageFromD3DDeclUsage( D3DDECLUSAGE DeclUsage );
	D3DFORMAT GD3DFMTFromGDataType( GRESOURCE_USAGE GUsage, GDATA_TYPE GDataType );
	GDATA_TYPE GDataTypeFromD3DFMT( D3DFORMAT d3dFormat, GRESOURCE_USAGE Usage );




}
#endif // _GD3D9RESOURCEMANAGER_CORE_H