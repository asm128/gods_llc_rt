#include "IGStorage.h"
#include <Windows.h>

#ifndef _GSTORAGEWIN32_H
#define _GSTORAGEWIN32_H

namespace god
{
	// 	Windows implementation of the storage manager
	// 
	class CGStorageWin32 : public __GOSH(Storage)
	{
	public:
		CGStorageWin32( HMODULE hModule );
		virtual ~CGStorageWin32( void );

		virtual error_t				init( void );

		virtual error_t				GetDirectoryExecution	( char* out_szPath, uint32_t* inout_nBufferSize );
		virtual error_t				GetDirectorySettings	( char* out_szPath, uint32_t* inout_nBufferSize );
		virtual error_t				GetDirectoryResources	( char* out_szPath, uint32_t* inout_nBufferSize );
		virtual error_t				GetDirectorySavegame	( char* out_szPath, uint32_t* inout_nBufferSize );
		virtual error_t				GetDirectoryScreenshots	( char* out_szPath, uint32_t* inout_nBufferSize );
		virtual error_t				GetDirectoryMailbox		( char* out_szPath, uint32_t* inout_nBufferSize );
		virtual error_t				GetDirectoryAvatars		( char* out_szPath, uint32_t* inout_nBufferSize );
		virtual error_t				GetDirectorySecured		( char* out_szPath, uint32_t* inout_nBufferSize );

		virtual const char*			GetDirectoryExecution	( void );
		virtual const char*			GetDirectorySettings	( void );
		virtual const char*			GetDirectoryResources	( void );
		virtual const char*			GetDirectorySavegame	( void );
		virtual const char*			GetDirectoryScreenshots	( void );
		virtual const char*			GetDirectoryMailbox		( void );
		virtual const char*			GetDirectoryAvatars		( void );
		virtual const char*			GetDirectorySecured		( void );

		virtual error_t				RelocateDirectoryExecution( const char* in_wszPath, uint32_t in_nCharCount, bool bMoveContents );
		virtual error_t				RelocateDirectorySettings( const char* in_wszPath, uint32_t in_nCharCount, bool bMoveContents );
		virtual error_t				RelocateDirectoryResources( const char* in_wszPath, uint32_t in_nCharCount, bool bMoveContents );
		virtual error_t				RelocateDirectorySavegame( const char* in_wszPath, uint32_t in_nCharCount, bool bMoveContents );
		virtual error_t				RelocateDirectoryScreenshots( const char* in_wszPath, uint32_t in_nCharCount, bool bMoveContents );
		virtual error_t				RelocateDirectoryMailbox( const char* in_wszPath, uint32_t in_nCharCount, bool bMoveContents );
		virtual error_t				RelocateDirectoryAvatars( const char* in_wszPath, uint32_t in_nCharCount, bool bMoveContents );
		virtual error_t				RelocateDirectorySecured( const char* in_wszPath, uint32_t in_nCharCount, bool bMoveContents );

		virtual id_t				GetID( void ) const;

		// This function queries for an interface 
		// it will probably stay for the c++ implementation only until I find a better workaround on this
		virtual void*				GetInterfacePointer( GINTERFACEID_TYPE InterfaceID );

	private:
		HMODULE				m_hDllModule;
		wchar_t				m_wszCustomFolderName		[MAX_PATH+4];
		char				m_szCustomFolderName		[MAX_PATH+4];
		wchar_t				m_wszDirectoryExecution		[MAX_PATH+4],
							m_wszDirectorySettings		[MAX_PATH+4],
							m_wszDirectorySavegame		[MAX_PATH+4],
							m_wszDirectoryMailbox		[MAX_PATH+4],
							m_wszDirectoryResources		[MAX_PATH+4],
							m_wszDirectoryScreenshots	[MAX_PATH+4],
							m_wszDirectoryAvatars		[MAX_PATH+4],
							m_wszDirectorySecured		[MAX_PATH+4];
		char				m_szDirectoryExecution		[MAX_PATH+4],
							m_szDirectorySettings		[MAX_PATH+4],
							m_szDirectorySavegame		[MAX_PATH+4],
							m_szDirectoryMailbox		[MAX_PATH+4],
							m_szDirectoryResources		[MAX_PATH+4],
							m_szDirectoryScreenshots	[MAX_PATH+4],
							m_szDirectoryAvatars		[MAX_PATH+4],
							m_szDirectorySecured		[MAX_PATH+4];
	private:
		void				ClearPaths( void );
	};
};

#endif // _GSTORAGEWIN32_H