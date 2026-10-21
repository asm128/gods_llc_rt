#include "GD3D9_core.h"
#include "GD3D9_resources.h"
#include "IGVideo.h"
#include "glist.h"
#include "garray.h"

#include "DXUT.h"
#include "DXUTgui.h"

namespace god
{
	typedef class _CGD3D9 CGD3D9;
	typedef class _CGD3D9Device CGD3D9Device;

	typedef class _CGD3D9ResourceManager : public __GOSH( VideoContext )
	{
	public:
		_CGD3D9ResourceManager( CGD3D9Device* pDevice );
		virtual ~_CGD3D9ResourceManager( void );

		// I won't be able to use the DXUT dialog manager for D3D11 implementation =(
		virtual CDXUTDialogResourceManager*	GetAyuditaDXUT( void );
		// returns device object without increasing reference counter
		IDirect3DDevice9*					getd3d9Device( void )
		{
			return m_pD3D9Device;
		};
		
		// returns an ID for the interface requested
		int32_t								getAvailableID( GINTERFACEID_TYPE ResourceType );
		int32_t								releaseResourceID( GINTERFACEID_TYPE ResourceType, id_t ID );
		
		virtual uint32_t					GetWidth( uint32_t nTarget );
		virtual uint32_t					GetHeight( uint32_t nTarget );
		virtual id_t						GetID( void ) const
		{
			return m_nID;
		}
		virtual void						setID( id_t newID )
		{
			m_nID = newID;
		}

		//------------- These are used to handle d3d device state changes
		error_t								onCreateDevice( IDirect3DDevice9* pD3DDevice );
		error_t								onResetDevice( void );
		void								onLostDevice( void );
		void								onDestroyDevice( void );

		GRDS(SManagerBuffer)				getBufferItem				( GRDS(SManagerBuffer)		ManagerItem);
		GRDS(SFunction)						getFunctionItem				( GRDS(SFunction)			ManagerItem);
		GRDS(SDecl)							getDeclItem					( GRDS(SDecl)				ManagerItem);
		GRDS(SImage)						getImageItem				( GRDS(SImage)				ManagerItem);
		GRDS(SRasterizerState)				getRasterizerStateItem		( GRDS(SRasterizerState)	ManagerItem);
		GRDS(SDepthStencilState)			getDepthStencilStateItem	( GRDS(SDepthStencilState)	ManagerItem);
		GRDS(SBlendState)					getBlendStateItem			( GRDS(SBlendState)			ManagerItem);
		GRDS(SEffect)						getEffectItem				( GRDS(SEffect)				ManagerItem);
		GRDS(SXFont)						getFontItem					( GRDS(SXFont)				ManagerItem);
		// ------------------- IGVideoContext
		virtual error_t						FreeUnused( void );
		virtual error_t						ClearResources( void );
		virtual GOSH(VideoDevice)			GetVideoManager( void );
		virtual error_t						CreateBuffer( id_t nID, GRESOURCE_MODE nAccessMode, GRDS(SBuffer) pBufferData, GOSH(VideoBuffer)* ppCreated );
		virtual error_t						CreateBuffer( GRESOURCE_MODE nAccessMode, GRDS(SBuffer) pBufferData, GOSH(VideoBuffer)* ppCreated );
		virtual error_t						CreateBuffer( id_t nID, GRESOURCE_MODE nAccessMode, 
														uint32_t nBufferSize, GDATA_TYPE nDataFormat, 
														uint32_t nElementCount, GRESOURCE_USAGE nUsage, 
														const void* pDataBuffer, GOSH(VideoBuffer)* ppCreated );
		virtual error_t						CreateBuffer( GRESOURCE_MODE nAccessMode, 
														uint32_t nBufferSize, GDATA_TYPE nDataFormat, 
														uint32_t nElementCount, GRESOURCE_USAGE nUsage, 
														const void* pDataBuffer, GOSH(VideoBuffer)* ppCreated );
		virtual error_t						CreateBufferFromFile( const char* szFilename, uint32_t nCodePage, GRESOURCE_MODE nAccessMode, GOSH(VideoBuffer)* pCreated );
		virtual error_t						CreateBufferFromFile( id_t nID, const char* szFilename, uint32_t nCodePage, GRESOURCE_MODE nAccessMode, GOSH(VideoBuffer)* pCreated );

		// The whole function stuff is very complex. 
		// Care must be taken in order to reduce process overhead and define redundant functions.
		virtual int32_t						CreateFunction( int32_t nID, GRDS(SBuffer) pBufferData, GOSH(VideoFunction)* ppCreated );
		//virtual int32_t					CompileAndCreateFunction( int32_t nID, _GBUFFER_DATA* pBufferData, GPROGRAMMINGLANGUAGE_ID nLanguage, const char* szEntryPoint, IGVideoFunction** ppCreated );
		//virtual int32_t					CompileAndCreateFunction( GPROGRAMMINGLANGUAGE_ID nLanguage, _GBUFFER_DATA* pBufferData, IGVideoFunction** ppCreated );
		//virtual int32_t					CreateFunction(	int32_t nID, GPROGRAMMINGLANGUAGE_ID nLanguage, uint32_t nBufferSize, 
		//													GDATA_TYPE nDataFormat, uint32_t nElementCount, GRESOURCE_USAGE nUsage, 
		//													const void* pDataBuffer, IGVideoFunction** ppCreated );
		//virtual int32_t					CreateFunction(	GPROGRAMMINGLANGUAGE_ID nLanguage, uint32_t nBufferSize, 
		//													GDATA_TYPE nDataFormat, uint32_t nElementCount, GRESOURCE_USAGE nUsage, 
		//													const void* pDataBuffer, IGVideoFunction** ppCreated );
		//virtual int32_t					CreateFunctionFromFile( int32_t ID, const wchar_t* wszFilename, GPROGRAMMINGLANGUAGE_ID nLanguage, GRESOURCE_USAGE Usage, const char* szEntryPoint, IGVideoFunction** ppCreated );
		//virtual int32_t					CreateFunctionFromFile( const wchar_t* wszFilename, GPROGRAMMINGLANGUAGE_ID nLanguage, GRESOURCE_USAGE Usage, const char* szEntryPoint, IGVideoFunction** ppCreated );
		//virtual int32_t					CompileAndCreateFunctionFromFile( int32_t ID, const wchar_t* wszFilename, GPROGRAMMINGLANGUAGE_ID nLanguage, const char* szEntryPoint, GRESOURCE_USAGE Usage, IGVideoFunction** ppCreated );
		//virtual int32_t					CompileAndCreateFunctionFromFile( const wchar_t* wszFilename, GPROGRAMMINGLANGUAGE_ID nLanguage, const char* szEntryPoint, GRESOURCE_USAGE Usage, IGVideoFunction** ppCreated );

#ifdef _TODO 
		virtual int32_t						CreateImage(	int32_t ID, const void* pData, uint32_t Width, uint32_t Height,
															GDATA_TYPE PixelFormat, GRESOURCE_MODE BufferMode, IGVideoImage** ppCreated );
		virtual int32_t						CreateImage( int32_t nID, IGVideoImage** ppCreated );
		virtual int32_t						CreateImage( IGVideoImage** ppCreated );
#endif
		virtual error_t						CreateImageFromMemory( uint32_t nWidth, uint32_t nHeight, GRDS(SBuffer) pBufferData, GRESOURCE_MODE nAccessMode, GOSH(VideoImage)* ppCreated );
		virtual error_t						CreateImageFromMemory( int32_t nID, uint32_t nWidth, uint32_t nHeight, GRDS(SBuffer) pBufferData, GRESOURCE_MODE nAccessMode, GOSH(VideoImage)* ppCreated );

		virtual error_t						CreateImageFromFile( bool in_b16Bit, uint32_t in_nAlphaKey, const wchar_t* wszFilename, bool* out_bIsAlpha, GOSH(VideoImage)* out_ppCreated );
		virtual error_t						CreateImageFromFile( int32_t nID, const wchar_t* wszFilename, GOSH(VideoImage)* ppCreated );
		virtual error_t						CreateImageFromFile( const wchar_t* wszFilename, GOSH(VideoImage)* ppCreated );

		virtual error_t						CreateImageFromFile( bool in_b16Bit, uint32_t in_nAlphaKey, const char* szFilename, uint32_t nCodePage, bool* out_bIsAlpha, GOSH(VideoImage)* out_ppCreated );
		virtual error_t						CreateImageFromFile( int32_t nID, const char* szFilename, uint32_t nCodePage, GOSH(VideoImage)* ppCreated );
		virtual error_t						CreateImageFromFile( const char* szFilename, uint32_t nCodePage, GOSH(VideoImage)* ppCreated );

		// Shaders
		virtual error_t						CreateProgramFromFile( int32_t ID, const wchar_t* pFilename, GOSH(VideoProgram)* ppCreated );
		virtual error_t						CreateProgramFromFile( const wchar_t* pFilename, GOSH(VideoProgram)* ppCreated );

		virtual error_t						CreateProgramFromFile( int32_t ID, const char* pFilename, uint32_t nCodePage, GOSH(VideoProgram)* ppCreated );
		virtual error_t						CreateProgramFromFile( const char* pFilename, uint32_t nCodePage, GOSH(VideoProgram)* ppCreated );

		// Fonts
		//virtual	error_t						CreateFont(	int32_t ID, uint32_t nHeight, uint32_t nWidth, uint32_t nWeight, const wchar_t* pFaceName, bool bItalic, GOSH(Font)* ppCreated );
		//virtual	error_t						CreateFont(	uint32_t nHeight, uint32_t nWidth, uint32_t nWeight, const wchar_t* pFaceName, bool bItalic, GOSH(Font)* ppCreated );
		
		virtual	error_t						CreateFont(	int32_t ID, uint32_t nHeight, uint32_t nWidth, uint32_t nWeight, const char* pFaceName, uint32_t nCodePage, bool bItalic, GOSH(Font)* ppCreated );
		virtual	error_t						CreateFont(	uint32_t nHeight, uint32_t nWidth, uint32_t nWeight, const char* pFaceName, uint32_t nCodePage, bool bItalic, GOSH(Font)* ppCreated );
		
		virtual GOSH(VideoBuffer)			GetBuffer( int32_t ID );
		virtual GOSH(VideoImage)			GetImage( int32_t ID );
		virtual GOSH(VideoFunction)			GetFunction( int32_t ID );
		virtual GOSH(VideoProgram)			GetProgram( int32_t ID );
		virtual GOSH(Font)					GetFont( int32_t ID );
		//------------------- IGResourceManager
		//virtual IGDeviceManager*			GetDevice( void );

		//------------------- IGInterface
		virtual void*						GetInterfacePointer( GINTERFACEID_TYPE InterfaceID );

	private:
		id_t								m_nID;
		//
		CGD3D9Device*						m_pDevice;
		IDirect3DDevice9*					m_pD3D9Device;
		int32_t								m_nUsedIDArraySize;
//		int32_t								m_nUsedIDCount;
		bool*								m_pUsedID;

		CDXUTDialogResourceManager			m_DXUTDialogManager;
#define __DECLARE_CGD3D9ARRAY_MEMBERS_V2( Token )				\
		gelementary_old<CGD3D9##Token*>			m_lst##Token;	\
		void __free##Token##List( void );
		//-----------------------------------------------
		__DECLARE_CGD3D9ARRAY_MEMBERS_V2( Function );
		__DECLARE_CGD3D9ARRAY_MEMBERS_V2( Image );
		__DECLARE_CGD3D9ARRAY_MEMBERS_V2( Buffer )
		__DECLARE_CGD3D9ARRAY_MEMBERS_V2( Font )
		__DECLARE_CGD3D9ARRAY_MEMBERS_V2( Program )
#undef __DECLARE_CGD3D9ARRAY_MEMBERS_V2

		// -----------------------
		// Append functions acquire an instance and stores its pointer in member list
#define __DECLARE_ITEMLIST_MEMBERS_NEW( tok, TOKEN )					\
		private:														\
			gpod_list<S##TOKEN>	m_lstManager##tok;						\
		public:															\
			error_t __createDX##tok( S##TOKEN* p##tok##Data );

			__DECLARE_ITEMLIST_MEMBERS_NEW(Image, Image)
			__DECLARE_ITEMLIST_MEMBERS_NEW(Buffer, ManagerBuffer)
			__DECLARE_ITEMLIST_MEMBERS_NEW(Decl, Decl)
			__DECLARE_ITEMLIST_MEMBERS_NEW(XFont, XFont)
			__DECLARE_ITEMLIST_MEMBERS_NEW(XProgram, Effect)
			__DECLARE_ITEMLIST_MEMBERS_NEW(Function, Function)
			////__DECLARE_ITEMLIST_MEMBERS_NEW(SwapChain, SWAPCHAIN ) 
			__DECLARE_ITEMLIST_MEMBERS_NEW(DepthStencilState, DepthStencilState)
			__DECLARE_ITEMLIST_MEMBERS_NEW(BlendState, BlendState)
			__DECLARE_ITEMLIST_MEMBERS_NEW(RasterizerState, RasterizerState)
#undef __DECLARE_ITEMLIST_MEMBERS_NEW

	public:
		error_t			__createDXImageFromFile		( SImage* pImageData );
		error_t			__createDXImageFromFile16	( SImage* pImageData );
		error_t			__createDXImageFromFile		( SImage* pImageData, bool b16Bit );
		error_t			__createDXImageFromBuffer	( SImage* pImageData );
	private:
		int32_t			__createDXIndexBuffer ( SManagerBuffer* pRef );
		int32_t			__createDXVertexBuffer( SManagerBuffer* pRef );

	}CGD3D9ResourceManager;
}