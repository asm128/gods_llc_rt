#include "GD3D9_core.h"
#include "IGVideo.h"

#ifndef _GD3D9_RESOURCES_H
#define _GD3D9_RESOURCES_H

namespace god
{
	typedef class _CGD3D9ResourceManager	CGD3D9ResourceManager;
	typedef class _CGD3D9Enumerator			CGD3D9Enumerator;
	typedef class _CGD3D9Device				CGD3D9Device;
	typedef class _CGD3D9					CGD3D9;

	//
	typedef class _CGD3D9Buffer : public __GOSH(VideoBuffer)
	{
	public:
		_CGD3D9Buffer( CGD3D9ResourceManager* pManager );
		virtual ~_CGD3D9Buffer( void );

		virtual int32_t					internalInit( int32_t nID, GRDS(SBuffer) BufferData, GRESOURCE_MODE ResourceMode );
		virtual int32_t					internalInit( int32_t nID, const char* szFilename, uint32_t nCodePage, GRESOURCE_MODE ResourceMode );
		virtual GRDS(SManagerBuffer)	getItemData( void )
		{
			return m_BufferData.acquire();
		};

		//------------------- GOSH(VideoBuffer)
		int32_t							Flush( void );
		//------------------- IGBuffer
		virtual GRDS(SBuffer)			GetBufferData( void ) const;
		virtual uint32_t				GetElementCount( void ) const;
		virtual GDATA_TYPE				GetDataFormat( void ) const;
		virtual uint32_t				GetRowSize( void ) const;
		virtual uint32_t				GetBufferSize( void ) const;
		virtual GRESOURCE_MODE			GetResourceMode( void ) const;
		virtual GRESOURCE_USAGE			GetUsage( void ) const;
		//------------------- IGDeviceResource
		virtual id_t					GetID( void ) const;
		//virtual GRESOURCE_TYPE			GetResourceType( void ) const;
		//------------------- IGInterface
		virtual void*					GetInterfacePointer( GINTERFACEID_TYPE InterfaceID );
	private:
		id_t							m_nBufferID;
		CGD3D9ResourceManager*			m_pManager;
		gpod_ptr<SManagerBuffer>			m_BufferData;
	} CGD3D9Buffer;

	// IGVideoFunction is for the CGD3D9Program class
	typedef class _CGD3D9Function : public __GOSH(VideoFunction)
	{
	public:
		_CGD3D9Function( CGD3D9ResourceManager* pManager );
		virtual ~_CGD3D9Function( void );
		
		virtual int32_t						internalInit( int32_t nID, const wchar_t* pFilename, GPROGRAMMINGLANGUAGE_ID nLanguage, 
			const char* szEntryPoint, GRESOURCE_USAGE nUsage );
		virtual int32_t						internalInit( int32_t nID, GRDS(SBuffer) Code, GPROGRAMMINGLANGUAGE_ID nLanguage, 
			const char* szEntryPoint );
		virtual GRDS(SFunction)				getItemData( void )
		{
			return m_FunctionData.acquire();
		};

		//------------------- IGVideoFunction
		//------------------- IGFunction
		virtual	int32_t						SetConstantBuffer( const char* pVarName, IGBuffer* pBuffer );
		virtual	int32_t						SetConstantValue( const char* pVarName, const void* pValue, uint32_t nSize );
		virtual	int32_t						SetConstantImage( const char* pVarName, GOSH(VideoImage) pImage );
		virtual GRDS(SBuffer)				GetFunctionData( void ); // ??
		virtual int32_t						GetVersion( void ) const;
		virtual GRESOURCE_USAGE				GetUsage( void ) const;
		//------------------- IGDeviceResource
		virtual int32_t						GetID( void ) const;
		virtual GRESOURCE_TYPE				GetResourceType( void ) const;
		virtual void*						GetPrivateData( void ); 
		//------------------- IGInterface
		virtual void*						GetInterfacePointer( GINTERFACEID_TYPE InterfaceID );
	private:
		id_t						m_ID;
		CGD3D9ResourceManager*		m_pManager;
		gpod_ptr<SFunction>				m_FunctionData;
	} CGD3D9Function;

	typedef class _CGD3D9Image : public __GOSH(VideoImage)
	{
	public:
		_CGD3D9Image( CGD3D9ResourceManager* pManager );
		virtual ~_CGD3D9Image( void );

		virtual int32_t							internalInit( int32_t nID, uint32_t nWidth, uint32_t nHeight, GRDS(SBuffer) pBufferData, GRESOURCE_MODE nAccessMode );
		virtual int32_t							internalInit( int32_t nID, const wchar_t* pFilename );
		virtual int32_t							internalInit( int32_t nID, bool b16Bit, uint32_t nAlphaKey, const wchar_t* pFilename );
		virtual GRDS(SImage)						getItemData( void )
		{
			return m_TextureData.acquire();
		};
		//------------------- IGVideoImage
		//virtual class GOSH(VideoContext)	GetManager( void );
		//------------------- IGImage
		virtual uint32_t						GetWidth( void ) const;
		virtual uint32_t						GetHeight( void ) const;
		virtual GRDS(SBuffer)					GetBufferData( uint32_t nIndex ) const;
		virtual GRESOURCE_USAGE					GetUsage( void ) const;
		//------------------- IGDeviceResource
		virtual int32_t							GetID( void ) const;
		virtual void*							GetPrivateData( void ); 
		//------------------- IGSmartPtr
		//------------------- IGInterface
		virtual void*							GetInterfacePointer( GINTERFACEID_TYPE InterfaceID );
	private:
		CGD3D9ResourceManager*					m_pManager;
		int32_t									m_nTextureID;
		gpod_ptr<SImage>							m_TextureData;
	} CGD3D9Image;

	typedef class _CGD3D9Program : public __GOSH(VideoProgram)
	{
	public:
		_CGD3D9Program( CGD3D9ResourceManager* pManager );
		virtual ~_CGD3D9Program( void );

		virtual int32_t					internalInit( int32_t nID, const wchar_t* pFilename );
		virtual GRDS(SEffect)			getItemData( void )
		{
			return m_EffectData.acquire();
		};
		//------------------- IGVideoFunction
		//------------------- IGProgram
		virtual	int32_t					SetConstantBuffer( const char* pVarName, GOSH(VideoBuffer) pBuffer );
		virtual	int32_t					SetConstantValue( const char* pVarName, const void* pValue, uint32_t nSize );
		virtual	int32_t					SetConstantImage( const char* pVarName, GOSH(VideoImage) pImage );
		virtual int32_t					SetActiveFunctionByName( const char* pFunctionName );
		//------------------- IGDeviceResource
		virtual int32_t					GetID( void ) const;
		virtual GRESOURCE_TYPE			GetResourceType( void ) const;
		virtual void*					GetPrivateData( void ); 
		//------------------- IGSmartPtr

		//------------------- IGInterface
		virtual void*				GetInterfacePointer( GINTERFACEID_TYPE InterfaceID );
	private:
		CGD3D9ResourceManager*	m_pManager;

		uint32_t				m_ID;
		gpod_ptr<SEffect>			m_EffectData;

	} CGD3D9Program;

	typedef class _CGD3D9Font : public __GOSH(Font)
	{
	public:
		_CGD3D9Font( CGD3D9ResourceManager* pManager );
		virtual ~_CGD3D9Font( void );

		virtual int32_t					internalInit( int32_t nID, const wchar_t* pFacename, uint32_t nWidth, uint32_t nHeight, uint32_t nWeight, bool bItalic );
		virtual GRDS(SXFont)				getItemData( void )
		{
			return m_FontData.acquire();
		};
		//------------------- IGFont
		// 
		virtual int32_t					Draw( const char* pText, int32_t nCharCount, GRDS(SRectangle) RectData, 
											bool bNormalized, uint32_t dwFormat=0, 
											uint32_t Color=0xFFFFFFFFL, uint32_t nCodePage=0
										);
		//virtual const char*				GetFacename( void ) const;
		virtual int32_t					GetFacename( char* pFacename, uint32_t* nSize ) const;
		virtual uint32_t				GetHeight( void ) const;
		virtual uint32_t				GetWidth( void ) const;
		virtual uint32_t				GetWeight( void ) const;
		virtual bool					GetItalic( void ) const;

		//------------------- IGDeviceResource
		virtual int32_t					GetID( void ) const;
		virtual GRESOURCE_TYPE			GetResourceType( void ) const;
		//virtual class GOSH(VideoContext)	GetManager( void );
		//virtual IGDeviceManager*		GetDevice( void );
		//virtual IGResourceManager*		GetResourceManager( void );
		virtual void*					GetPrivateData( void ); 
		//------------------- IGInterface
		virtual void*					GetInterfacePointer( GINTERFACEID_TYPE InterfaceID );
	private:
		CGD3D9ResourceManager*		m_pManager;
		int32_t						m_nFontID; // !!!
		gpod_ptr<SXFont>				m_FontData;

	} CGD3D9Font;
};

#endif // _GD3D9_RESOURCES_H