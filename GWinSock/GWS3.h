#include "IGNetwork.h"
//#include "GMath.h"
#include "GWS3Manager.h"

#ifndef _GWS3_H
#define _GWS3_H

typedef class _CGWS3 : public IGNetwork
{
public:
	_CGWS3( HMODULE hModule );
	virtual ~_CGWS3( void );
	
	int32_t					Init( void );
	virtual error_t			CreateConnectionManager( HWND Window, IGTrueNetworkHandler** ppDevice );
private:
	HMODULE					m_hModule;
	uint32_t				m_nDeviceCount, m_nDeviceArraySize;
	CGWS3NetworkManager**	m_pDeviceList;
} CGWS3, *LPGWS3;


#endif // _GWS_H