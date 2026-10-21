
#ifndef _GD3D9_FUNCTION_MACROS_H
#define _GD3D9_FUNCTION_MACROS_H

#define __GDEFINE_GET_GINTERFACE_BY_ID_GELEMENTARY( TOKEN, Token )	\
GOSH(TOKEN) _CGD3D9ResourceManager::Get##Token( int32_t ID )		\
{																	\
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "Get" #Token );		\
	GOSH(TOKEN) result=0;											\
	for( uint32_t i=0, n = m_lst##Token.size(); i < n; i++ )		\
	{																\
		result = m_lst##Token[i];									\
		if( result && result->GetID() == ID )						\
			return gacquireInterface( result );						\
	}																\
	return 0;														\
};
//

#define __GDEFINE_FREE_MANAGER_GINTERFACE_LIST_GELEMENTARY( TOKEN, Token )	\
	void _CGD3D9ResourceManager::__free##Token##List( void )				\
{																			\
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__free"#Token "List" );		\
	GHPTR(TOKEN) garbage;													\
	CGD3D9##Token* pstuff;													\
	while( m_lst##Token##.size() )											\
	{																		\
		m_lst##Token.pop(&pstuff);											\
		garbage = pstuff;													\
	}																		\
}


#define __GDEFINE_APPEND_MANAGER_GINTERFACE_FUNCTION( Token )											\
	int32_t _CGD3D9ResourceManager::__append##Token( CGD3D9##Token* p )									\
{																										\
	GPROFILE_METHOD( "_CGD3D9ResourceManager", "__append"#Token );										\
	if( 0 == p )																						\
	{																									\
		error_printf("Cannot append a null pointer! Parameter is NULL!");								\
		return -1;																						\
	}																									\
																										\
	if( gresizePointerArray( sizeof( CGD3D9##Token* ), m_n##Token##Count+1,								\
		&m_n##Token##ArraySize, (void**)&m_lstp##Token, m_n##Token##Count+m_n##Token##Count/2 ) )		\
	{																									\
		error_printf("Cannot resize array to store resource! Out of memory?");							\
		return -1;																						\
	}																									\
	m_lstp##Token[m_n##Token##Count]	= (CGD3D9##Token*)p->GetInterfacePointer(GIID_CUSTOMINTERFACE);	\
	m_n##Token##Count++;																				\
	return 0;																							\
};



#endif // _GD3D9_FUNCTION_MACROS_H