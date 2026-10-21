#include "IGNetwork.h"
#include <windows.h>

#ifndef _GWS3HOOKS_H
#define _GWS3HOOKS_H

using namespace god;

typedef class _CGWS3 CGWS3;

typedef struct GCLIENT_TYPE
{
	SOCKET		skToClient;
	uint32_t	nID;
} GCLIENT;

class GQueueElem
{
public:
	GQueueElem(const uint8_t *pData, uint32_t nSize);
	~GQueueElem(void);
	GQueueElem	*m_pNext;
	uint8_t		*m_pData;
	uint32_t	m_nSize;
};

class GQueue
{
private:
	GQueueElem *m_pHead;
	GQueueElem *m_pTail;
	uint32_t m_Count;
public:
	GQueue(void);
	~GQueue(void);
	
	void Dequeue(void);
	void Enqueue(const void *pData, uint32_t nSize);
	GRDS(SBuffer) Front( bool bDequeue ); // void *pData, ;
	uint32_t GetCount(void);
	uint32_t GetFrontSize(void);
};

class CGWSocket
{
public:
	CGWSocket( void );
	~CGWSocket( void );

	bool		IsRunning(void) 
	{ return m_bRunning; }
	// socket MISC functions
	int32_t		CreateSocket(void);
	int32_t		Bind(int nPort);
	int32_t		Listen(void);
	int32_t		Accept(SOCKET *skToNewClient);
	int32_t		Connect(char *chServer, int nPort);
	void		Disconnect(void);
	// send and receive data
	int32_t		Send(const char*,UINT);
	int32_t		Send(const char*,UINT,SOCKET );
	int32_t		Receive( SOCKET sk );
	// get information
	SOCKET		GetSocket( void ) 
	{ return m_skSocket; }
	// information about the inbox
	bool		Empty( void ) 
	{ return (m_pInbox->GetCount() == 0); }
	//uint32_t			GetNextPacketSize( void ) 
	//{ return (m_pInbox->GetFrontSize()); }
	GRDS(SBuffer)	GetNextBuffer( void );
private:
	GQueue		*m_pInbox;
	SOCKET		m_skSocket;
	uint8_t		*m_Buffer;
	bool		m_bRunning;
}; // class

typedef class _CGWS3Connection : public __GOSH(NetworkConnection)
{
public:
	_CGWS3Connection( void );
	virtual ~_CGWS3Connection( void ); // not virtual, we have to call this destructor!
	virtual error_t			internalInit( id_t nID, GCOMMUNICATION_MODE Mode, uint16_t Port, char *IP, uint32_t nSize);

	virtual bool			IsConnected( void ) const;
	virtual error_t			PushBuffer( GRDS(SBuffer) BufferData );
	virtual GRDS(SBuffer)	PopBuffer( void );
	virtual uint32_t		GetBufferCount(void) const;
	void*					GetInterfacePointer( GINTERFACEID_TYPE InterfaceType );

	id_t					GetID( void ) const { return m_nID; };
	int32_t					MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );
private:
	id_t					m_nID;

	HWND					m_hMainWnd;
	bool					m_bRunning;
	GCOMMUNICATION_MODE		m_CommunicationMode;
	CGWSocket				*m_pSockObj;
	GCLIENT					m_Clients[256];
	uint8_t					m_ClientCount;
	uint32_t				m_ClientID;
	uint8_t					*m_Buffer;
	uint32_t				m_nPort;	// port number
	char					m_pIP[256]; // IP address
	uint32_t				m_nMaxSize; // buffer size

	// initializing Socket Object
	int32_t					CreateServer(CGWSocket **ppSkObject);
	int32_t					CreateClient(CGWSocket **ppSkObject);

	// message processing
	int32_t					OnAccept(void);
	int32_t					OnReceive(SOCKET skReceiving);
	int32_t					OnDisconnect(SOCKET skDisconnecting);

	void					_Cleanup( void );
} CGWS3Hooks, *LPGWS3HOOKS;

#endif // _GWS3HOOKS_H