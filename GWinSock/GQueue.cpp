//#define GPROFILE_ENABLED
#include "GWS3Connection.h"
#include "GCore_printf.h"
#include "GDebugger_core.h"

GQueueElem::GQueueElem(const uint8_t *pData, uint32_t nSize)
{
	GPROFILE_METHOD( "GQueueElem", "GQueueElem" );
	m_pData = NULL;
	m_pNext = NULL;
	m_pData = (uint8_t*)malloc( sizeof( uint8_t )*nSize );
	if( 0 == m_pData )
	{
		error_printf("malloc() FAILED! Out of memory?");
		throw("malloc() FAILED! Out of memory?");
	}
	m_nSize = nSize;
	for( uint32_t i=0; i<nSize; i++)
		m_pData[i] = pData[i];
//	memcpy(m_pData, pData, nSize);
} // constructor
/*————————————————————————-*/

GQueueElem::~GQueueElem(void)
{
	GPROFILE_METHOD( "GQueueElem", "~GQueueElem" );
	if (m_pData)
	{
		delete [] m_pData;
		m_pData = NULL;
	}
	m_pNext = NULL;
} // destructor

// init pointers
GQueue::GQueue(void) 
{
	GPROFILE_METHOD( "GQueue", "GQueue" );
	m_pTail = NULL;
	m_pHead = NULL;
	m_Count = 0;
   }// constructor
/*----------------------------------------------------------------*/

// free all queued elements and reset members
GQueue::~GQueue(void) 
{
	GPROFILE_METHOD( "GQueue", "~GQueue" );
	while (m_pHead) 
		Dequeue();
	m_pTail = NULL;
	m_pHead = NULL;
	m_Count = 0;
}// destructor

void GQueue::Enqueue(const void *pData, unsigned int nSize)
{
	GPROFILE_METHOD( "GQueue", "Enqueue" );
	GQueueElem *pNew = new GQueueElem( (const BYTE*)pData, nSize);
	// this is the first element
	if (m_Count == 0)
	{
		m_pHead = pNew;
		m_pTail = pNew;
	}
	// there is already one element
	else if (m_Count == 1)
	{
		m_pHead->m_pNext = pNew;
		m_pTail = pNew;
	}
	else
	{
		m_pTail->m_pNext = pNew;
		m_pTail = pNew;
	}
	m_Count++;
} // Enqueue

void GQueue::Dequeue(void)
{
	GPROFILE_METHOD( "GQueue", "Dequeue" );
	GQueueElem *pTemp;
	// already empty
	if (m_Count == 0) return;
	// one element left
	else if (m_Count == 1)
	{
		delete m_pHead;
		m_pHead = NULL;
		m_pTail = NULL;
	}
	else
	{
		pTemp = m_pHead;
		m_pHead = m_pHead->m_pNext;
		delete pTemp;
	}
	m_Count--;
} // Dequeue

GRDS(SBuffer) GQueue::Front( bool bDequeue ) // void *pData, BOOL bDequeue)
{
	GPROFILE_METHOD( "GQueue", "Front" );
	//gacquire( m_pHead );
	//return m_pHead;
	//if( bDequeue )
	//	Dequeue();
	return 0;
};

uint32_t GQueue::GetCount(void) 
{ 
	GPROFILE_METHOD( "GQueue", "GetCount" );
	return m_Count; 
}

uint32_t GQueue::GetFrontSize(void)
{
	GPROFILE_METHOD( "GQueue", "GetFrontSize" );
	if (m_pHead) 
		return m_pHead->m_nSize;
	else return 0;
}