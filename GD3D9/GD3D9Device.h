#include "GD3D9_core.h"
#include "GD3D9_resources.h"
#include "IGVideo.h"
#include "IGPrivate.h"

namespace god
{
	typedef class _CGD3D9 CGD3D9;
	typedef class _CGD3D9ResourceManager CGD3D9ResourceManager;

	typedef class _CGD3D9Device : public IGTrueVideoHandler//_GOSH(VideoDevice)
	{
	public:
		_CGD3D9Device( CGD3D9* pContext );
		virtual ~_CGD3D9Device( void );
		//
		virtual int32_t							internalInit( void );
		virtual int32_t							internalInit( GRDS(SWindow) pWindowData, uint32_t nMinDepth, uint32_t nMinStencil );
		virtual CGD3D9*							getContext( void );
		virtual D3DPRESENT_PARAMETERS*			getd3d9PP( void )
		{
			if( !m_DeviceData )
				return 0;
			return &m_DeviceData->PresentParameters; 
		};
		int32_t									getAvailableID( GINTERFACEID_TYPE ResourceType );
		bool									GetDeviceLost( void )
		{
			return m_bLostDevice;
		}

		// ------------------------------------------------------------------------------------------
		// Let manager handle state changes
		virtual int32_t							Update( float fElapsedTime );

		// ------------------------------------------------------------------------------------------
		// Process window messages.
		virtual int32_t							MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );

		// ------------------------------------------------------------------------------------------
		// Set inputs
		virtual int32_t							SetFunction( GOSH(VideoFunction) pFunction );
		virtual int32_t							SetProgram( GOSH(VideoProgram) pProgram );
		virtual int32_t							SetStreamCount( uint32_t nStreamsToRender );
		virtual int32_t							SetSourceBuffer( uint32_t nStream, uint32_t nOffset, GOSH(VideoBuffer) pBuffer );
		virtual int32_t							SetIndices( uint32_t nOffset, GOSH(VideoBuffer) pBuffer );
		virtual bool							GetProfilingEnabled( void ) const;
		virtual void							SetProfilingEnabled( bool bEnabled );
		// ------------------------------------------------------------------------------------------
		// Render					
	#ifdef _TODO
		virtual int32_t							Render(		GPRIMITIVE_TYPE PrimitiveType,
														_GBUFFER_DATA* pVertexData, 
														uint32_t nStartVertex, uint32_t nVertexCount,
														_GBUFFER_DATA* pIndexData, 
														uint32_t nStartIndex, uint32_t nIndexCount
													);
		virtual int32_t							Render(		GPRIMITIVE_TYPE PrimitiveType,
														GGEOMETRYBUFFER_DATA* pGeometryBufferData 
													);
	#endif
		virtual int32_t							Render( GPRIMITIVE_TYPE PrimitiveType, uint32_t nPrimitiveCount,
													uint32_t nStartVertex, uint32_t nVertexCount, // uint32_t nVertexSlice,
													uint32_t nStartIndex // , IGVideoBuffer* pIndices
												);
		virtual uint32_t						GetWidth( uint32_t nTarget );
		virtual uint32_t						GetHeight( uint32_t nTarget );
		virtual GDATA_TYPE 						GetTargetFormat( void );
		virtual void							SetClearColorValue( const float* rgbaColor );
		virtual void							SetClearDepthValue( float fDepth );
		virtual void							SetClearStencilValue( uint32_t nStencil );
		virtual int32_t							ClearTarget( void );
		virtual int32_t							ClearDepth( void );
		virtual int32_t							ClearStencil( void );
		virtual int32_t							Clear( bool bColor, bool bDepth, bool bStencil );

		virtual	int32_t							StartRender( void );
		virtual	int32_t							EndRender( void );
		virtual int32_t							ToggleFullScreen( bool bFullScreen );
		virtual bool							FullScreenEnabled( void ) const;
		virtual int32_t							SetTargetByIndex( uint32_t nTargetIndex );
		virtual int32_t							SetTargetImage( GOSH(VideoImage) pImage, uint32_t nTargetIndex );
		virtual uint32_t 						GetTargetIndex( void );
		virtual GRDS(SWindow)					GetTargetWindowData( void );
		virtual int32_t							PresentTarget( uint32_t nTargetIndex, GRDS(SRectangle) src_pQuad, GRDS(SRectangle) dst_pQuad ); // 1 Backbuffer each client window
		virtual error_t							SetTargetArea( uint32_t nPositionX, uint32_t nPositionY, uint32_t nWidth, uint32_t nHeight );
		virtual error_t							SetViewportArea( uint32_t nPositionX, uint32_t nPositionY, uint32_t nWidth, uint32_t nHeight );
		virtual error_t							SetTargetSize( uint32_t nWidth, uint32_t nHeight );
		virtual error_t							SetTargetFormat( GDATA_TYPE DataFormat );

		// ------------------------------------------------------------------- 
		// Renderer states 
		virtual int32_t							SetBlendData		( GRDS(SBlend) pBlendData );
		virtual int32_t							SetRasterizerData	( GRDS(SRasterizer) pRasterizerData );
		virtual int32_t							SetDepthData		( GRDS(SDepth) pDepthData );
		virtual int32_t							SetStencilData		( GRDS(SStencil) pStencilData );

		virtual GRDS(SBlend)					GetBlendData( void );
		virtual GRDS(SRasterizer)				GetRasterizerData( void );
		virtual GRDS(SDepth)					GetDepthData( void );
		virtual GRDS(SStencil)					GetStencilData( void );

		// ------------------------------------------------------------------- 
		// Obsolete renderstate accessors
		virtual error_t							EnableTargetAreaTest( void );
		virtual error_t							DisableTargetAreaTest( void );
		virtual int32_t							EnableAlphaBlend( void );
		virtual int32_t							DisableAlphaBlend( void );
		virtual int32_t							EnableAlphaTest( void );
		virtual int32_t							DisableAlphaTest( void );
		virtual int32_t							SetAlphaReferenceValue( uint32_t nRef );
		virtual int32_t							SetAlphaBlendOperator( GLOGICAL_OPERATOR Condition );
		virtual int32_t							SetBlendOperator( GLOGICAL_OPERATOR Operator );
		virtual int32_t							SetSrcBlendValue( GINTERPOLATION_VALUE Mode );
		virtual int32_t							SetDstBlendValue( GINTERPOLATION_VALUE Mode );
		virtual int32_t							EnableZBuffer( void );
		virtual int32_t							DisableZBuffer( void );
		virtual int32_t							EnableZWrite( void );
		virtual int32_t							DisableZWrite( void );
		virtual int32_t							EnableStencilBuffer( void );
		virtual int32_t							DisableStencilBuffer( void );
		virtual int32_t							SetStencilBufferData( const GSTENCIL1_DATA* pStencilData );
		virtual int32_t							SetBackfaceCulling( GCULL_MODE Mode );
		virtual int32_t							SetFillMode( GFILL_MODE Mode );
		virtual int32_t							SetDepthBufferMode( GRESOURCE_MODE Mode );
		virtual bool							GetAlphaBlendEnabled( void ) const;
		virtual bool							GetAlphaTestEnabled( void ) const;
		virtual uint32_t						GetAlphaReferenceValue( void ) const;
		virtual GLOGICAL_OPERATOR				GetBlendOperator( void ) const;
		virtual GINTERPOLATION_VALUE			GetSrcBlendValue( void ) const;
		virtual GINTERPOLATION_VALUE			GetDstBlendValue( void ) const;
		virtual GCULL_MODE						GetBackfaceCulling(  void ) const;
		virtual GRESOURCE_MODE					GetDepthBufferMode( void ) const;
		virtual int32_t							SetImageWrapMode( uint32_t nUsageIndex, GWRAP_MODE Mode );
		virtual GWRAP_MODE 						GetImageWrapMode( uint32_t nUsageIndex ) const;

		// ------------------------------------------------------------------------------------------
		virtual int32_t							CreateResourceManager( GOSH(VideoContext)* ppCreated );
		
		// Probably I should make this function part of the IGDriver interface?
		virtual IGVideo*						GetContext( void );

		//------------------- IGVideoHandler stuff
		virtual error_t							RegisterHandler( IGVideoHandler* pHandler );
		virtual error_t							UnregisterHandler( IGVideoHandler* pHandler );
		
		//------------------- IGDriver
		virtual int32_t							GetID( void ) const;
		virtual uint32_t						GetVersion( void ) const;
		virtual GDRIVERFAMILY_ID				GetFamilyID( void ) const;
		virtual GRDS(SDevice)					GetDeviceData( void );
		virtual void*							GetPrivateData( void ); 		// 
		
		//------------------- IGInterface
		virtual void*							GetInterfacePointer( GINTERFACEID_TYPE InterfaceID );
	private:
		RECT									m_TargetArea;
		D3DVIEWPORT9							m_ViewportArea;
		WNDCLASS								m_WindowClass;
		HWND									m_hMainWindow;
		HWND*									m_hChildWindowList;
		IDirect3DSurface9*						m_pCurrentRenderTarget;

		gpod_ptr<SBlendState>					m_BlendState;
		gpod_ptr<SRasterizerState>				m_RasterizerState;
		gpod_ptr<SDepthStencilState>			m_DepthStencilState;

		GCOLOR									m_ClearColor;
		float									m_fClearDepth;
		uint32_t								m_nClearStencil;
		D3DDEVTYPE								m_DeviceType;
		uint32_t								m_nAdapter;
		bool									m_bLostDevice;
		bool									m_bRendering;

		bool									m_bFullScreen;
		D3DPRESENT_PARAMETERS					m_FullScreenPP;
		D3DPRESENT_PARAMETERS					m_WindowedPP;
		CGD3D9*									m_pContext;
		uint32_t								m_nManagerCount;
		uint32_t								m_nManagerArraySize;
		CGD3D9ResourceManager**					m_lstpManager;	
		CGD3D9ResourceManager*					m_pDefaultManager;

		// --------------------------------
		gpod_ptr<SD3D9Device>						m_DeviceData;
		gpod_ptr<SDevice>							m_pHALDeviceData;
		gpod_ptr<SWindow>							m_pWindowData;
		uint32_t								m_nActiveWindow;
		bool									m_bRenderingPaused;
		bool									m_bMinimized;
		bool									m_bMaximized;

		// video handlers
		#define __GDECLARE_POINTER_ARRAY(ClassName, Token)					\
 			unsigned int		m_n##Token##Count, m_n##Token##ArraySize;	\
 			##ClassName##**		m_lst##Token##; 
			__GDECLARE_POINTER_ARRAY( IGVideoHandler, VideoHandler );
		#undef __GDECLARE_POINTER_ARRAY


		uint32_t								m_nStreamCount;
		uint32_t								m_nIndexStreamCount;
		gpod_ptr<SManagerBuffer>				m_pIndexStream[256];
		gpod_ptr<SManagerBuffer>				m_pVertexStream[256];
		gpod_ptr<SManagerBuffer>				m_pIndex;
		gpod_ptr<SFunction>					m_pVertexFunction;
		gpod_ptr<SFunction>					m_pPixelFunction;
		//ghdlr_Effect							m_pShader;
		//ghdlr_Decl								m_pVertexDeclaration;
		gpod_ptr<SEffect>							m_pShader;
		gpod_ptr<SDecl>								m_pVertexDeclaration;

		int32_t									m_nUsedIDArraySize;
		bool*									m_pUsedID;
		// --------------------------------
		IDirect3DDevice9*						getd3d9Device( void );
		int32_t									__createDevice( SD3D9Device* pDeviceData );
		int32_t									__onCreateDevice( IDirect3DDevice9* pDevice );
		int32_t									__onResetDevice( void );
		void									__onLostDevice( void );
		void									__onDestroyDevice( void );
	} CGD3D9Device;
}