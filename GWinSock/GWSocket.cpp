//#define GPROFILE_ENABLED
#include "GWS3Connection.h"
#include "GCore_printf.h"
#include "GDebugger_core.h"

static const uint32_t g_PkgSize=sizeof(SBuffer);

CGWSocket::CGWSocket( void )
{
	GPROFILE_METHOD( "CGWSocket", "CGWSocket" );
	m_skSocket	= INVALID_SOCKET;
	m_bRunning	= false;
	m_pInbox	= NULL;
	m_Buffer	= NULL;
} // constructor
/*————————————————————————-*/

CGWSocket::~CGWSocket( void )
{
	GPROFILE_METHOD( "CGWSocket", "~CGWSocket" );
	if (IsRunning())
	{
		Disconnect();
		m_bRunning = false;
	}
	if (m_pInbox)
	{
		delete m_pInbox;
		m_pInbox = NULL;
	}
	if (m_Buffer)
	{
		free( m_Buffer );
		m_Buffer = NULL;
	}
	m_skSocket = INVALID_SOCKET;
} // destructor

int32_t CGWSocket::CreateSocket(void)
{
	GPROFILE_METHOD( "CGWSocket", "CreateSocket" );
	if (m_skSocket != INVALID_SOCKET) 
		Disconnect();

	m_skSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_skSocket==INVALID_SOCKET)
	{
		return G_FAIL;
	}

	m_pInbox = new GQueue();
	m_Buffer = (uint8_t*)malloc( sizeof( uint8_t )*65536 );
	if( 0 == m_Buffer )
	{
		error_printf("CGWSocket::CreateSocket(): Cannot allocate socket buffer! Out of memory?");
		throw(L"CGWSocket::CreateSocket(): Cannot allocate socket buffer! Out of memory?");
	}
	memset(m_Buffer,0,65536);
	return S_OK;
}

INT32 CGWSocket::Bind(int nPort)
{
	GPROFILE_METHOD( "CGWSocket", "Bind" );
	sockaddr_in saServerAddress;
	memset(&saServerAddress, 0, sizeof(sockaddr_in));
	saServerAddress.sin_family		= AF_INET;
	saServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	saServerAddress.sin_port		= htons(nPort);
	if( SOCKET_ERROR == 
		bind(m_skSocket, (sockaddr*)&saServerAddress, sizeof(sockaddr) ) )
	{
		Disconnect();
		return G_FAIL;
	}
	return S_OK;
} // Bind

INT32 CGWSocket::Listen(void)
{
	GPROFILE_METHOD( "CGWSocket", "Listen" );
	if (listen(m_skSocket, 32) != 0)
	return G_FAIL;
	m_bRunning = true;
	return S_OK;
} // Listen

INT32 CGWSocket::Accept( SOCKET *skToNewClient )
{
	GPROFILE_METHOD( "CGWSocket", "Accept" );
	sockaddr_in saClientAddress;
	int nClientSize = (int)sizeof(sockaddr_in);
	(*skToNewClient) = accept(m_skSocket, (sockaddr*) &saClientAddress,
		&nClientSize);
	if ( (*skToNewClient) == INVALID_SOCKET )
		return G_FAIL;
	return S_OK;
} // accept

INT32 CGWSocket::Connect(char *chServer, int nPort)
{
	GPROFILE_METHOD( "CGWSocket", "Connect" );
	sockaddr_in saServerAddress;
	hostent* pHost=NULL;
	// try to find the server
	memset(&saServerAddress,0,sizeof(sockaddr_in));
	saServerAddress.sin_port = htons(nPort);
	saServerAddress.sin_family = AF_INET;
	saServerAddress.sin_addr.s_addr=inet_addr(chServer);
	if (saServerAddress.sin_addr.s_addr==INADDR_NONE)
	{
		pHost = gethostbyname(chServer);
		if (pHost != NULL)
		{
			saServerAddress.sin_addr.s_addr = ((LPIN_ADDR)pHost->h_addr)->s_addr;
		}
		else
		return G_FAIL;
	}
	// connect to server address
	if (connect(m_skSocket, (sockaddr*)&saServerAddress, sizeof(sockaddr)) 
		== SOCKET_ERROR)
	{
		Disconnect();
		return G_FAIL;
	}
	m_bRunning = true;
	return S_OK;
} // Connect

void CGWSocket::Disconnect(void)
{
	GPROFILE_METHOD( "CGWSocket", "Disconnect" );
	if (m_skSocket != INVALID_SOCKET)
	{
		shutdown(m_skSocket, SB_BOTH);
		closesocket(m_skSocket);
		m_skSocket = INVALID_SOCKET;
	}
} // Disconnect

INT32 CGWSocket::Send(const char *pPkg, uint32_t nSize)
{
	GPROFILE_METHOD( "CGWSocket", "Send" );
	uint32_t nSent=0;
	uint32_t n=0;
	while (nSent < nSize)
	{
		n = send(m_skSocket, pPkg+nSent, nSize-nSent, 0);
		if (n==SOCKET_ERROR)
			return n;
		else 
			nSent += n;
	}
	return nSent;
} // Send
/*—————————————————————————-*/
int CGWSocket::Send( const char *pPkg, uint32_t nSize, SOCKET skReceiver )
{
	GPROFILE_METHOD( "CGWSocket", "Send" );
	uint32_t nSent=0;
	uint32_t n=0;
	while (nSent < nSize)
	{
		n = send(skReceiver, pPkg+nSent, nSize-nSent, 0);
		if (n==SOCKET_ERROR)
		return n;
		else nSent += n;
	}
	return nSent;
} // Send



INT32 CGWSocket::Receive(SOCKET sk) 
{
	GPROFILE_METHOD( "CGWSocket", "Receive" );
	HRESULT hr		= S_OK;
	//uint32_t nSize		= 65536;		// max bytes for each read call
	//uint32_t nBytesRead = 0;
	//uint32_t nOffset	= 0;			// Position in m_Buffer
	//uint32_t n			= 0;			// Remaining data
	//bool bDone		= false;		// 
	//GPACKET *pPkg	 = NULL;
	//uint32_t nPkgSizeTotal = 0;

	//// read up to 65.536 Bytes in each call
	//// loop until no more data is waiting
	//while (!bDone)
	//{
	//	nBytesRead = recv(sk, &m_Buffer[n], nSize-n, 0);
	//	if (nBytesRead == SOCKET_ERROR) 
	//	{
	//		int WSAError = WSAGetLastError();
	//		// ignore harmless messages
	//		if ( (WSAError != WSAEMSGSIZE) && (WSAError != WSAEWOULDBLOCK) )
	//		{
	//			hr = G_FAIL;
	//			bDone = true;
	//			break;
	//		}
	//	}
	//	// now we have nBytesRead bytes ii m_Buffer
	//	if (nBytesRead <= 0) 
	//		bDone = true;
	//	else
	//	{
	//		// take care of old data in the buffer
	//		nBytesRead += n;
	//		// loop until complete header is found
	//		while ( (nBytesRead-nOffset) > g_PkgSize )
	//		{
	//			// next bunch of data
	//			pPkg = (GPACKET*)&m_Buffer[nOffset];
	//			pPkg->pData = &m_Buffer[nOffset] + g_PkgSize;
	//			nPkgSizeTotal = g_PkgSize + pPkg->nSize;
	//			// did we get the whole package?
	//			if ( (nBytesRead-nOffset) >= (nPkgSizeTotal) )
	//			{
	//				m_pInbox->Enqueue(pPkg, nPkgSizeTotal);
	//				nOffset += nPkgSizeTotal;
	//			}
	//			// no back to recv()
	//			else 
	//			{
	//				// copy half package to the start of the buffer
	//				memcpy(m_Buffer, &m_Buffer[nOffset], nBytesRead-nOffset);
	//				n = nBytesRead-nOffset;
	//				break;
	//			}
	//		} // while
	//		// now we got all data waiting for us
	//		if (nBytesRead < nSize) bDone = true;
	//	}
	//} // while
	return hr;
} // Receive

//int g_PkgSize=sizeof(ZFXPACKAGE);
GRDS(SBuffer) CGWSocket::GetNextBuffer( void )
{
	GPROFILE_METHOD( "CGWSocket", "GetNextBuffer" );
	// anything at all?
	if (0 == m_pInbox->GetCount())
		return 0;
	// draw data into the buffer
	return m_pInbox->Front( true );
} // GetNextPkg