#include "GWS3Connection.h"
#include "IGNetwork.h"
#include "IGPrivate.h"

#ifndef _GWS3DEVICE_H
#define _GWS3DEVICE_H

typedef class _CGWS3NetworkManager : public IGTrueNetworkHandler
{
public: 
	_CGWS3NetworkManager( _CGWS3* pContext );
	virtual ~_CGWS3NetworkManager( void );

	virtual error_t			internalInit( HWND hWnd, id_t nID );
	//
	virtual error_t			CreateConnection( GCOMMUNICATION_MODE Mode, uint16_t Port, char *IP, uint32_t nMaxBufferSize, GOSH(NetworkConnection)* out_pCreated );
	virtual bool			IsConnected( GOSH(NetworkConnection) ) const;
	virtual error_t			PushBuffer( GOSH(NetworkConnection) Connection, GRDS(SBuffer) BufferData );
	virtual GRDS(SBuffer)	PopBuffer( GOSH(NetworkConnection) Connection );
	virtual uint32_t		GetBufferCount( GOSH(NetworkConnection) Connection ) const;

	error_t					Update( float fElapsedTime );
	id_t					GetID( void ) const { return m_nID; };
	void*					GetInterfacePointer( GINTERFACEID_TYPE InterfaceType );
	int32_t					MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam ){return 0;};

private:
	id_t					m_nID;
	HWND					m_hMainWnd;
	CGWS3*					m_pContext;

	// Functions to handle device state changes
	void						_Cleanup( void );
} CGWS3NetworkManager;// class CGWS3Device;

#endif // _GWS3DEVICE_H