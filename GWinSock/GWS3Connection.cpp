#include <WinSock2.h>

#include "GWS3Connection.h"
//#define GPROFILE_ENABLED
#include "GDebugger_core.h"

#include <strsafe.h>

// init and release
error_t _CGWS3Connection::internalInit( id_t nID, GCOMMUNICATION_MODE nMode, USHORT Port, CHAR *IP, uint32_t nMaxBufferSize )
{
	GPROFILE_METHOD( "_CGWS3Device", "Init" );

	m_pSockObj			= NULL;

	ZeroMemory( &m_pIP[0], sizeof( CHAR )*256 ); // IP address
	ZeroMemory( &m_Clients[0], sizeof( GCLIENT )*256 ); //this too

	m_nMaxSize			= nMaxBufferSize; // buffer size

	m_CommunicationMode	= nMode; // should be inited at init
	m_nPort				= Port;
	m_ClientID			= (GCOMMMODE_SERVER == nMode) ? 0 : 1; // 0 reserved for server 
	m_ClientCount		= 0;
	m_Buffer			= (uint8_t*)malloc( sizeof( uint8_t )*m_nMaxSize );
	m_bRunning			= false;

	uint32_t				nRes;
	WSADATA				wsaData;
	uint32_t				nEvents=0;
	WORD				wVersion;
	if (IP) 
		StringCchCopyA(m_pIP, 256, IP);
	wVersion			= MAKEWORD(2,0);
	if ( (nRes = WSAStartup(wVersion, &wsaData)) != 0)
		return G_FAIL;
	// create master socket object listening
	if (m_CommunicationMode==GCOMMMODE_SERVER)
	{
		if ( FAILED( CreateServer(&m_pSockObj) ) )
		return G_FAIL;
	}
	// create socket object as client
	else if (m_CommunicationMode==GCOMMMODE_CLIENT)
	{
		if( 0 == m_pIP[0] )
			StringCchCopyA(m_pIP, 256, "LOCALHOST");

		if( FAILED( CreateClient(&m_pSockObj) ) )
			return G_FAIL;
	}
	else 
		return E_INVALIDARG;

	m_nID		= nID;
	m_bRunning	= true;
	return S_OK;
};

int32_t _CGWS3Connection::CreateServer( CGWSocket **ppSkObject )
{
	GPROFILE_METHOD( "_CGWS3Device", "CreateServer" );
	uint32_t nEvents=0;
	(*ppSkObject) = new CGWSocket();
	if (!(*ppSkObject)) // lol no RAM? xD
		return G_FAIL;

	// 1. Step: create a socket object
	if (FAILED( (*ppSkObject)->CreateSocket() ))
		return G_FAIL;
	// 2. Step: bind to port
	if (FAILED( (*ppSkObject)->Bind(m_nPort) ))
		return G_FAIL;
	// 3. Step: set to listening guard
	if (FAILED( (*ppSkObject)->Listen() ))
		return G_FAIL;
	nEvents |= FD_READ | FD_WRITE | FD_CONNECT | FD_ACCEPT | FD_CLOSE;
	// 4. Step: set Windows notification
	if ( WSAAsyncSelect((*ppSkObject)->GetSocket(), m_hMainWnd, WM_GNETSERVER,
		nEvents) == SOCKET_ERROR)
	{
		m_pSockObj->Disconnect();
		return G_FAIL;
	}
	// set all clients as invalid
	for (int i=0; i<256; i++)
	{
		m_Clients[i].skToClient = INVALID_SOCKET;
		m_Clients[i].nID = 0;
	}
	return S_OK;
} // CreateServer

int32_t _CGWS3Connection::CreateClient(CGWSocket **ppSkObject)
{
	GPROFILE_METHOD( "_CGWS3Device", "CreateClient" );
	uint32_t nEvents=0;
	(*ppSkObject) = new CGWSocket();
	if (!(*ppSkObject))
		return G_FAIL;
	// 1. Step: create socket object
	if (FAILED( (*ppSkObject)->CreateSocket() ))
		return G_FAIL;
	if (m_pIP == NULL)
		gethostname(m_pIP, 10);
	// 2. Step: try to connect
	if (FAILED( (*ppSkObject)->Connect(m_pIP, m_nPort) ))
		return G_FAIL;
	nEvents |= FD_READ | FD_CLOSE;
	// 3. Step: set Windows notification
	if (WSAAsyncSelect( (*ppSkObject)->GetSocket(), m_hMainWnd, WM_GNETCLIENT,
		nEvents) == SOCKET_ERROR) 
	{
		m_pSockObj->Disconnect();
		return G_FAIL;
	}
	return S_OK;
} // Create Client
//

//
bool _CGWS3Connection::IsConnected(void) const
{
	GPROFILE_METHOD( "_CGWS3Device", "IsConnected" );
	return m_bRunning;
};

// 
int32_t _CGWS3Connection::MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam )
{
	//GPROFILE_METHOD( "_CGWS3Device", "MsgProc" );
	WORD wEvent, wError;
	wError = HIWORD(lParam);
	wEvent = LOWORD(lParam);
	// evaluate which event occurred
	switch (wEvent) 
	{
	// new client is accepted
	case FD_CONNECT: 
		break;
	// new clients is knocking
	case FD_ACCEPT: 
		{ 
			return OnAccept(); 
		} 
		break;
	// there is data to be received
	case FD_READ: 
		{ 
			return OnReceive(wParam); 
		} 
		break;
	// a socket is closing
	case FD_CLOSE: 
		{ 
			return OnDisconnect(wParam); 
		}
		break;
	// after sending data
	case FD_WRITE: 
		break;
	}
	return S_OK;
};

// ---------------------------
// sending and receiving
//
error_t _CGWS3Connection::PushBuffer( GRDS(SBuffer) pPacket )
{
	GPROFILE_METHOD( "_CGWS3Device", "PushBuffer" );
	uint32_t g_PkgSize = pPacket->__kDataSize;

	uint32_t nBytes=0;
	uint32_t nSize = g_PkgSize + gget( pPacket )->nElementCount*GTYPEID_TOTALBYTES(gget( pPacket )->DataFormat);
		//pPacket->nSizeInBytes;
	HRESULT hr=S_OK; 
	if (nSize > m_nMaxSize)
		return E_OUTOFMEMORY;

	//if( m_CommMode == GCOMMMODE_CLIENT )
	//{
	//	nBytes = m_pSockObj->Send((char*)pPacket->pDataBuffer, nSize);
	//	if ( (nBytes==SOCKET_ERROR) || (nBytes<nSize) )
	//		return G_FAIL;
	//}
	//else
	//{
	//	if (nSize > m_nMaxSize) 
	//		return E_OUTOFMEMORY;
	//	for (uint32_t i=0; i<m_ClCount; i++)
	//	{
	//		if (m_Clients[i].skToClient != INVALID_SOCKET)
	//		{
	//			nBytes = m_pSockObj->Send((char*)pPacket->pDataBuffer, nSize, m_Clients[i].skToClient);
	//			if ( (nBytes==SOCKET_ERROR) || (nBytes<nSize) )
	//				hr = G_FAIL;
	//		}
	//	}
	//}
	return hr;
};
// 
//error_t _CGWS3Device::Send(const GPACKET* pPacket)
//{
//	uint32_t g_PkgSize=sizeof(GPACKET);
//
//	INT hr=S_OK;
//	uint32_t nBytes=0;
//	uint32_t nSize=g_PkgSize+pPacket->nSize;
//
//	if (m_Mode != GNETWORKMODE_SERVER)
//		return G_FAIL;
//
//	if (nSize > m_nMaxSize) 
//		return E_OUTOFMEMORY;
//
//	// data serialization
//	memcpy(m_Buffer, pPacket, g_PkgSize);
//	memcpy(m_Buffer+g_PkgSize, pPacket->pData, pPacket->nSize);
//	for (uint32_t i=0; i<m_ClCount; i++)
//	{
//		if (m_Clients[i].skToClient != INVALID_SOCKET)
//		{
//			nBytes = m_pSockObj->Send(m_Buffer, nSize, m_Clients[i].skToClient);
//			if ( (nBytes==SOCKET_ERROR) || (nBytes<nSize) )
//			hr = G_FAIL;
//		}
//	}
//	return hr;
//};


INT32 _CGWS3Connection::OnAccept( void )
{
	GPROFILE_METHOD( "_CGWS3Device", "OnAccept" );
	//uint32_t g_PkgSize=sizeof(_GBUFFER_DATA);

	//int nSize=0, nBytes=0, i=m_ClCount;
	//if (m_ClCount >= 255) 
	//	return E_OUTOFMEMORY;
	//if ( FAILED( m_pSockObj->Accept( &(m_Clients[i].skToClient) ) ) )
	//	return G_FAIL;

	//// SEND THE ID TO THE NEW CLIENT:
	//GPACKET *pPkg	= (GPACKET*)m_Buffer;
	//pPkg->pData		= &m_Buffer[g_PkgSize];
	//pPkg->nSize		= sizeof(UINT);
	//pPkg->TypeID	= 0; // ID Msg
	//pPkg->nSender	= 0; // Server
	//memcpy(pPkg->pData, &m_ClID, sizeof(UINT));

	//// increase counter
	//m_Clients[i].nID = m_ClID;
	//m_ClCount++;
	//m_ClID++;
	//nSize = g_PkgSize + pPkg->nSize;
	//nBytes = m_pSockObj->Send(m_Buffer, nSize, m_Clients[i].skToClient);
	//if ( (nBytes==SOCKET_ERROR) || (nBytes<nSize) )
	//	return G_FAIL;
	//// INFORM ALL OTHER CLIENTS ABOUT THE NEW GUY IN TOWN
	//pPkg->TypeID = 1;
	//SendToClients(pPkg);

	return S_OK;
} // OnAccept

INT32 _CGWS3Connection::OnDisconnect(SOCKET skDisconnecting)
{
	GPROFILE_METHOD( "_CGWS3Device", "OnDisconnect" );
	//uint32_t g_PkgSize=sizeof(GPACKET);

	//GPACKET Packet;
	//UCHAR i=0;
	//if (skDisconnecting==INVALID_SOCKET) 
	//	return G_FAIL;
	//if (m_Mode==GNETWORKMODE_SERVER)
	//{
	//	// delete from list
	//	for (i=0; i<m_ClCount; i++)
	//	{
	//		if (m_Clients[i].skToClient == skDisconnecting)
	//			break;
	//	}
	//	if (i>=m_ClCount)
	//		return G_FAIL;

	//	// close the socket
	//	shutdown(m_Clients[i].skToClient,0x02);
	//	closesocket(m_Clients[i].skToClient);
	//	m_Clients[i].skToClient = INVALID_SOCKET;

	//	// TELL THE OTHERS SAY GOODBYE
	//	Packet.pData	= &m_Buffer[g_PkgSize];
	//	Packet.nSize	= sizeof(UINT);
	//	Packet.TypeID	= 2; // ID disconnecting Message
	//	Packet.nSender	= 0; // Server
	//	memcpy( Packet.pData, &m_Clients[i].nID, sizeof(UINT) );
	//	SendToClients(&Packet);

	//	// copy last entr to free positon
	//	memcpy(&m_Clients[i], &m_Clients[m_ClCount-1], sizeof(GCLIENT));
	//	m_ClCount--;
	//}
	//else
	//{
	//	shutdown(m_pSockObj->GetSocket(),0x02);
	//	closesocket(m_pSockObj->GetSocket());
	//}
	return S_OK;
} // OnDisconnect

error_t _CGWS3Connection::OnReceive(SOCKET skReceiving)
{
	GPROFILE_METHOD( "_CGWS3Device", "OnReceive" );
	if (m_bRunning)
		return m_pSockObj->Receive(skReceiving);
	else
		return 1;
} // OnReceive

void* _CGWS3Connection::GetInterfacePointer( GINTERFACEID_TYPE InterfaceType )
{ 
	GPROFILE_METHOD( "_CGWS3", "GetInterfacePointer" );
	switch( InterfaceType )
	{
	case GIID_RESERVED:
	case GIID_CUSTOMINTERFACE:
		return (_CGWS3*)this;
	case GIID_DEVICEFUNCTION:
		return (GOSH(NetworkConnection))this;
	default:
		return 0;
	}
}

