//#define GPROFILE_ENABLED
#include "GD3D9Device.h"
#include "GD3D9ResourceManager.h"


#include "GCore_printf.h"
#include "GDebugger_core.h"


using namespace god;

#ifdef _TODO // TODO remains to do
// ------------------------------------------------------------------------------------------ 
// Return owner context 
IGVideoFunction* _CGD3D9Device::GetShader( void );  
GOSH(VideoBuffer) _CGD3D9Device::GetSourceBuffer( uint32_t nStream ); 
GOSH(VideoBuffer) _CGD3D9Device::GetIndices( void );
//// Render
#endif // _TODO

// ------------------------------------------------------------------------------------------
int32_t _CGD3D9Device::SetProgram( GOSH(VideoProgram) pShader )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetProgram" );
	if( 0 == pShader )
		return G_INVALIDARGUMENT;
	_CGD3D9Program* pEffect = (_CGD3D9Program*)pShader->GetInterfacePointer( GIID_CUSTOMINTERFACE ); 
	
	gpod_ptr<SEffect> temp(pEffect->getItemData());
	GHPTR(VideoProgram) garbage = pEffect;
	pEffect = 0;

	if( m_pShader == temp )
		return S_OK;

	gpod_ptr<SEffect> old(m_pShader);
	m_pShader = temp;
	if( old.valid() && old->bActive )
	{
		old->pd3dXProgram->EndPass();
		old->pd3dXProgram->End();
		old->bActive = false;
	}
	return S_OK;
};

int32_t _CGD3D9Device::SetStreamCount( uint32_t nStreamsToRender )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetStreamCount" );
	if( nStreamsToRender > 255 )
		return G_INVALIDARGUMENT;
	if( m_nStreamCount != nStreamsToRender )
		m_nStreamCount = nStreamsToRender;
	return S_OK;
};

int32_t _CGD3D9Device::SetSourceBuffer( uint32_t nStream, uint32_t nOffset, GOSH(VideoBuffer) pBuffer )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetSourceBuffer" );
	if( 0 == pBuffer )
		return G_INVALIDARGUMENT;
	CGD3D9Buffer* pd3dBuffer = (CGD3D9Buffer*)pBuffer->GetInterfacePointer( GIID_CUSTOMINTERFACE );
	if( nStream > 255 )
		return G_INVALIDINDEX;

	gpod_ptr<SManagerBuffer> temp(pd3dBuffer->getItemData());
	GHPTR(VideoBuffer) garbage = pd3dBuffer;
	pd3dBuffer = 0;

	if( m_pVertexStream[nStream] == temp )
		return S_OK;
	
	gpod_ptr<SManagerBuffer> old(m_pVertexStream[nStream] );
	m_pVertexStream[nStream] = temp;
	if( old != 0 )
		old->bActive = false;
	return S_OK;
};

int32_t _CGD3D9Device::SetIndices( uint32_t nOffset, GOSH(VideoBuffer) pBuffer )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetIndices" );
	if( 0 == pBuffer )
		return G_FAIL;
	
	CGD3D9Buffer* pd3dBuffer = (CGD3D9Buffer*)pBuffer->GetInterfacePointer( GIID_CUSTOMINTERFACE );
	gpod_ptr<SManagerBuffer> temp(pd3dBuffer->getItemData());
	GHPTR(VideoBuffer) garbage = pd3dBuffer;
	pd3dBuffer = 0;

	if( m_pIndex == temp )
		return S_OK;

	gpod_ptr<SManagerBuffer> old(m_pIndex);
	m_pIndex			= temp;
	if( old != 0 )
		old->bActive	= false;
	return S_OK;
};

int32_t _CGD3D9Device::SetFunction( GOSH(VideoFunction) pFunction )
{
	GPROFILE_METHOD( "_CGD3D9Device", "SetFunction" );
//	we could optimize this function to simply store the CGD3D9Function pointer -and probably we should- as data may be not stored properly this way....
//	ON THE OTHER HAND, it would be more difficult the resourcemanager management as the manager of the CGD3D9Function object
//	could be released and CGD3D9Device would not notice, which would lead to either a memory leak or underperformance from resources that are released "just in case", 
//	by making every input to be released when any resource manager gets inactive.
	if( 0 == pFunction )
	{
		error_printf("_CGD3D9Device::SetFunction(): Cannot set NULL as video function!");
		return G_FAIL;
	}
	
	CGD3D9Function* pImplFunction = (CGD3D9Function*)pFunction->GetInterfacePointer( GIID_CUSTOMINTERFACE );
	gpod_ptr<SFunction> temp = pImplFunction->getItemData(), old;

	GHPTR(VideoFunction) garbage=pImplFunction;
	pImplFunction = 0;

	switch( gget(temp->SourceCode)->Usage )
	{
	case GUSAGE_VERTEXSHADER:
		if( m_pVertexFunction == temp ) // already set the same function
			return S_OK;
		old = m_pVertexFunction;
		m_pVertexFunction.acquire(temp);
		break;
	case GUSAGE_PIXELSHADER:
		if( m_pPixelFunction == temp ) // already set the same function
			return S_OK;
		old = m_pPixelFunction;
		m_pPixelFunction.acquire(temp);
		break;
	default:
		error_printf( "Unsupported GRESOURCE_USAGE value! _GDEVICEFUNCTION_DATA::ProgramCode->Usage == %u", gget(temp->SourceCode)->Usage );
		return G_FAIL;
	}
	if(old)
		old->bActive	= false;
	return S_OK;
};


// This function is terrible
int32_t _CGD3D9Device::Render( GPRIMITIVE_TYPE PrimitiveType, uint32_t nPrimitiveCount,
									uint32_t nStartVertex, uint32_t nVertexCount, // uint32_t nVertexSlice,
									uint32_t nStartIndex // , IGVideoBuffer* pIndices
								)
{
	GPROFILE_METHOD( "_CGD3D9Device", "Render" );
	IDirect3DDevice9* pDevice = getd3d9Device();
	if( 0 == pDevice )
	{
		error_printf("_CGD3D9Device::Render(): IDirect3DDevice9* == NULL!");
		return G_NOTREADY;
	}

	// check if all buffers and funtions are properly set
	if( !m_pIndex )
	{
		error_printf("_CGD3D9Device::Render() was called with no IGVideoBuffer set as index buffer.");
		return -1;
	}

	uint32_t i, n=m_nStreamCount;
	for( i=0; i<n; i++ )
		if( !m_pVertexStream[i] )
		{
			error_printf("_CGD3D9Device::Render(): Stream index %u is NULL! Stream count requested for render = %u", i, n );
			return -1;
		}
	D3DPRIMITIVETYPE ptPrimType;
	switch( PrimitiveType )
	{ // select the underlying d3d primitive type
	case GPRIMITIVE_POINT:
		ptPrimType = D3DPT_POINTLIST;
		break;
	case GPRIMITIVE_LINE:
		ptPrimType = D3DPT_LINELIST;
		break;
	case GPRIMITIVE_TRIANGLE:
		ptPrimType = D3DPT_TRIANGLELIST;
		break;
	case GPRIMITIVE_LINESTRIP:
		ptPrimType = D3DPT_LINESTRIP;
		break;
	case GPRIMITIVE_TRIANGLESTRIP:
		ptPrimType = D3DPT_TRIANGLESTRIP;
		break;
//	case GPRIMITIVE_TRIANGLEFAN:
//		ptPrimType = D3DPT_TRIANGLEFAN; - unsupported in d3d11 plus we never used it
//		break;
	default:
		error_printf( "Invalid GPRIMITIVE_TYPE value! GPRIMITIVE_TYPE=%i", (int)PrimitiveType );
		return G_FAIL; // unrecognized type
	}

	// ERRGHH I MUST REMOVE THIS D3D FX FRAMEWORK ASAP
	HRESULT hr=S_OK;
	
	if( m_pVertexFunction.valid() && m_pPixelFunction.valid() )
	{
		if( m_pShader.valid() && m_pShader->bActive )
		{
			m_pShader->pd3dXProgram->EndPass();
			m_pShader->pd3dXProgram->End();
			m_pShader->bActive	= false;
		}
		if( false == m_pVertexFunction->bActive )
		{ /* something about setting up the constant buffer */ 
			if( hr = pDevice->SetVertexShader( m_pVertexFunction->pd3dVertexShader ) )
			{
				error_printf("IDirect3DDevice9::SetVertexShader() returned HRESULT=0x%X.", (int)hr);
				return -1;
			};
		}
		else
		{ /* something about updating the d3d constant buffer */ }

		if( false == m_pPixelFunction->bActive )
		{ /* something about setting up the constant buffer */ 
			if( hr = pDevice->SetPixelShader( m_pPixelFunction->pd3dPixelShader ) )
			{
				error_printf("IDirect3DDevice9::SetPixelShader() returned HRESULT=0x%X.", (int)hr);
				return -1;
			};
		}
		else
		{ /* something about updating the d3d constant buffer */ }
	}
	else if( m_pShader.valid() )
	{
		if( false == m_pShader->bActive )
		{
			uint32_t nPasses;
			m_pShader->pd3dXProgram->Begin( &nPasses, D3DXFX_DONOTSAVESTATE );
			m_pShader->pd3dXProgram->BeginPass( 0 );
			m_pShader->bActive = true;
		}
		else
		{
			m_pShader->pd3dXProgram->CommitChanges();
		}
	}

	IDirect3DVertexBuffer9* pd3dVB = 0;
	IDirect3DIndexBuffer9* pd3dIB = 0;
	if( false == m_pIndex->bActive )
	{
		if( hr = m_pIndex->pd3dBuffer->QueryInterface( IID_IDirect3DIndexBuffer9, (void**)&pd3dIB ) )
		{
			error_printf("IDirect3DVertexBuffer9::QueryInterface() returned HRESULT=0x%X!", (int)hr);
			return G_FAIL;
		}
		if( hr = pDevice->SetIndices( pd3dIB ) )
		{
			error_printf("IDirect3DDevice9::SetIndices() returned HRESULT=0x%X!"
				"Parameters: IDirect3DIndexBuffer9* = 0x%p"
				, hr
				, pd3dIB 
				);
			pd3dIB->Release();
			return G_FAIL;
		}
		pd3dIB->Release();
		m_pIndex->bActive = true;

	}
	for( i=0; i<n; i++ )
	{
		if( false == m_pVertexStream[i]->bActive )
		{
			if( hr = m_pVertexStream[i]->pd3dBuffer->QueryInterface( IID_IDirect3DVertexBuffer9, (void**)&pd3dVB ) )
			{
				error_printf("IDirect3DVertexBuffer9::QueryInterface() returned HRESULT=0x%X!", hr);
				return G_FAIL;
			}
			int sz1 = GTYPEID_ELEMENTBYTES( gget(m_pVertexStream[i]->BufferData)->DataFormat ),
				sz2 = GTYPEID_ELEMENTCOUNT( gget(m_pVertexStream[i]->BufferData)->DataFormat );
			int sz3 = sz1*sz2;
			if( hr = pDevice->SetStreamSource( i, pd3dVB, 0, sz3 ) )
			{
				pd3dVB->Release();
				error_printf("IDirect3DDevice9::SetStreamSource() returned HRESULT=0x%X!\n"
					"Parameters: Stream index = %u, IDirect3DVertexBuffer9* = 0x%p, ByteOffset = %u, Stride size = %u"
					, hr
					, i, pd3dVB, 0, sz3
					);
				return G_FAIL;
			};
			pd3dVB->Release();
			m_pVertexStream[i]->bActive = true;
		}
	}
	
	//ghdlr_Decl newDecl(0, 0, Decl_Free_handler), activeDecl(0, 0, Decl_Free_handler);
	gpod_ptr<SDecl> newDecl, activeDecl;
	newDecl.create();
	newDecl->nElementCount = n;
	memset( &newDecl->VertexElement[0], 0, sizeof( D3DVERTEXELEMENT9 )*64 );
	D3DVERTEXELEMENT9* elems = &newDecl->VertexElement[0], *otherelem=0;
	uint32_t normidx, vertidx, texcidx, tangidx, colidx, blendindicesidx, blendweightidx;
	normidx = vertidx = texcidx = tangidx = colidx = blendindicesidx = blendweightidx = 0;
	i=0;
	while( i < n )
	{
		
		otherelem = &gget(m_pVertexStream[i]->pDeclData)->VertexElement[0];
		elems[i].Offset		= 0; // offset is always zero because each stream starts from the beginning
		elems[i].Type		= otherelem->Type;
		elems[i].Method		= otherelem->Method;
		elems[i].Usage		= otherelem->Usage;
		switch( elems[i].Usage )
		{
		case D3DDECLUSAGE_POSITION:
			elems[i].UsageIndex	= vertidx;
			vertidx++;
			break;
		case D3DDECLUSAGE_NORMAL:
			elems[i].UsageIndex	= normidx;
			normidx++;
			break;
		case D3DDECLUSAGE_TEXCOORD:
			elems[i].UsageIndex	= texcidx;
			texcidx++;
			break;
		case D3DDECLUSAGE_BLENDINDICES:
			elems[i].UsageIndex	= blendindicesidx;
			blendindicesidx++;
			break;
		case D3DDECLUSAGE_BLENDWEIGHT:
			elems[i].UsageIndex	= blendweightidx;
			blendweightidx++;
			break;
		case D3DDECLUSAGE_TANGENT:
			elems[i].UsageIndex	= tangidx;
			tangidx++;
			break;
		case D3DDECLUSAGE_COLOR:
			elems[i].UsageIndex	= colidx;
			colidx++;
			break;
		default:
			return G_FAIL;
		}
		elems[i].Stream		= i;
		i++;
	}
	elems[i].Offset		= 0;
	elems[i].Type		= D3DDECLTYPE_UNUSED;
	elems[i].Method		= 0;
	elems[i].Usage		= 0;
	elems[i].UsageIndex	= 0;
	elems[i].Stream		= 0xFF;
	
	activeDecl = m_pDefaultManager->getDeclItem( newDecl.get_pointer() );
	if( !activeDecl )
	{ // why would this ever happen? bad formed data?
		error_printf("Cannot get a proper vertex declaration item! getDeclItem() returned NULL!");
		return G_FAIL;
	}
	if( activeDecl != m_pVertexDeclaration )
	{
		if( m_pVertexDeclaration.get_pointer() )
			m_pVertexDeclaration->bActive = false;
		m_pVertexDeclaration = activeDecl;
		if( hr = pDevice->SetVertexDeclaration( m_pVertexDeclaration->pd3dDecl ) )
		{ // this will hardly happen
			error_printf("IDirect3DDevice9::SetVertexDeclaration() returned HRESULT=0x%X", hr);
			return G_FAIL;
		}
		activeDecl->bActive = true;
	}
	else 
	{
		if( false == m_pVertexDeclaration->bActive )
		{
			if( hr = pDevice->SetVertexDeclaration( m_pVertexDeclaration->pd3dDecl ) )
			{ // this will hardly happen
				error_printf("IDirect3DDevice9::SetVertexDeclaration() returned HRESULT=0x%X", hr);
				return G_FAIL;
			}
			m_pVertexDeclaration->bActive = true;
		}
	}
	//pDevice->SetScissorRect
	pDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	if( (!m_pIndex) && ( ptPrimType == D3DPT_POINTLIST ) )
	{
		if( hr = pDevice->DrawPrimitive( ptPrimType, nStartVertex, nPrimitiveCount ) )
		{ //
			error_printf( "IDirect3DDevice9::DrawPrimitive() FAILED! HRESULT=0x%X", hr );
			return G_FAIL;
		}
	}
	else
	{
		if( D3D_OK != (hr = pDevice->DrawIndexedPrimitive( ptPrimType, 0, 
			nStartVertex, nVertexCount, nStartIndex, nPrimitiveCount ) ) )
		{ //
			error_printf( "IDirect3DDevice9::DrawIndexedPrimitive() FAILED! HRESULT=0x%X", hr );
			return G_FAIL;
		}
	}

	return G_OK;
};